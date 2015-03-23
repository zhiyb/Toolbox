#include <avr/io.h>
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
	sendString(DEVICE_NAME);
#ifdef ENABLE_DAC
	ctrlDACControllerGenerate();
#endif
	ctrlADCControllerGenerate();
}

void ctrlRootLoop(void)
{
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
