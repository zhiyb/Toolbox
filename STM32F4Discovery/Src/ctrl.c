#include <string.h>
#include <instructions.h>
#include "adc.h"
#include "dac.h"
#include "timer.h"
#include "handles.h"
#include "ctrl.h"
#include "uart.h"
#include "info.h"

#define SEND_ADC_REQUEST

volatile uint8_t pause = 1;

void ctrlByteType(uint8_t type, uint32_t min, uint32_t max, uint32_t value)
{
	uint8_t bytes = type & 0x07;
	sendChar(type);
	sendValue(min, bytes);
	sendValue(max, bytes);
	sendValue(value, bytes);
	poolSending();
}

static void ctrlDeviceInfo(void)
{
	sendChar(CMD_INFO);
	sendValue(FW_VERSION, 4);
	sendString(DEVICE_NAME);
	ctrlDACControllerGenerate();
	ctrlADCControllerGenerate();
}

void ctrlRootLoop(void)
{
#ifdef SEND_ADC_REQUEST
	uint8_t req = adcTxBufferRequest;
#endif
	adcTxBufferRequest = 0;
#ifdef SEND_ADC_REQUEST
	while (req--) {
		pause = 1;
		sendData(adcTxBuffer, adcTxBufferLength);
	}
#endif
	pause = 0;
	switch (receiveChar()) {
	case CMD_RESET:
		pause = 1;
		sendChar(CMD_ACK);
		reset();
		break;
	case CMD_ANALOG:
		pause = 1;
		sendChar(CMD_ACK);
		ctrlADCController(receiveChar());
		break;
	case CMD_TIMER:
		sendChar(CMD_ACK);
		ctrlTimerController();
		break;
	case CMD_CONTROLLER:
		sendChar(CMD_ACK);
		switch (receiveChar()) {
		case CTRL_DAC_ID:
			ctrlDACController();
			break;
		}
		break;
	case CMD_INFO:
		pause = 1;
		sendChar(CMD_ACK);
		ctrlDeviceInfo();
	case INVALID_ID:
		break;
	}
}
