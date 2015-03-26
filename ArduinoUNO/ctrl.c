#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <instructions.h>
#include "handles.h"
#include "timer0.h"
#include "ctrl.h"
#include "uart.h"
#include "info.h"
#include "adc.h"
#ifdef ENABLE_DAC
#include "dac.h"
#endif
#ifdef ENABLE_PWM
#include "pwm.h"
#endif

#define SEND_ADC_REQUEST

volatile uint8_t pause = 1;

void ctrlByteType(uint8_t type, uint32_t min, uint32_t max, uint32_t value)
{
	uint8_t bytes = type & 0x07;
	sendChar(type);
	sendValue(min, bytes);
	sendValue(max, bytes);
	sendValue(value, bytes);
}

static void ctrlDeviceInfo(void)
{
	sendChar(CMD_INFO);
	sendValue(FW_VERSION, 4);
	sendString_P(PSTR(DEVICE_NAME));
#ifdef ENABLE_DAC
	ctrlDACControllerGenerate();
#endif
#ifdef ENABLE_PWM
	ctrlPWMControllerGenerate();
#endif
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
		switch (receiveChar()) {
		case TIMER0_ID:
			ctrlTimer0Controller();
			break;
		}
		break;
	case CMD_CONTROLLER:
		sendChar(CMD_ACK);
		switch (receiveChar()) {
#ifdef ENABLE_DAC
		case CTRL_DAC_ID:
			ctrlDACController();
			break;
#endif
#ifdef ENABLE_PWM
		case CTRL_PWM_ID:
			ctrlPWMController();
			break;
#endif
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