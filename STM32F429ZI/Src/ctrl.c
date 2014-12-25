#include <stdio.h>
#include <string.h>
#include <instructions.h>
#include "ctrl.h"
#include "communication.h"
#include "adc.h"
#include "info.h"

volatile uint8_t pause;

void ctrlDeviceInfo(void)
{
	sendChar(CMD_INFO);
	sendString(DEVICE_NAME);
}

void ctrlRootLoop(void)
{
	char buff[64];
	for (;;) {
		pause = 0;
		switch (receiveChar(-1)) {
		case CMD_RESET:
			pause = 1;
			sendChar(CMD_ACK);
			break;
		case CMD_PAUSE:
			pause = 1;
			sendChar(CMD_ACK);
			break;
		case CMD_CONF:
			pause = 1;
			sendChar(CMD_ACK);
			break;
		case CMD_INFO:
			pause = 1;
			sendChar(CMD_ACK);
			ctrlDeviceInfo();
			break;
		default:
			sprintf(buff, "ADC[0]: %.3f\t ADC[1]: %.3f\r\n", (float)adc[0] / 4095.0 * 3.3, (float)adc[1] / 4095.0 * 3.3);
			sendString(buff);
			pollSending();
			sprintf(buff, "ADC[0]: %u\t ADC[1]: %u\r\n", adc[0], adc[1]);
			sendString(buff);
			pollSending();
		}
	}
}
