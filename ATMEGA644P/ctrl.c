#include <instructions.h>
#include "handles.h"
#include "timer0.h"
#include "ctrl.h"
#include "uart.h"
#include "info.h"
#include "adc.h"

volatile uint8_t pause;

void ctrlDeviceInfo(void)
{
	sendChar(CMD_INFO);
	sendValue(FW_VERSION, 4);
	sendString(DEVICE_NAME);
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
	case CMD_INFO:
		pause = 1;
		sendChar(CMD_ACK);
		ctrlDeviceInfo();
	case INVALID_ID:
		break;
	}
}
