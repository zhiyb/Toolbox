#include <instructions.h>
#include "stm32f4xx_hal.h"
#include "uart.h"
#include "dac.h"
#include "info.h"
#include "handles.h"
#include "ctrl.h"

static const char * const channelName[] = {"Channel 1", "Channel 2"};
static const uint8_t channels[] = {DAC_CHANNEL_1, DAC_CHANNEL_2};

void initDAC(void)
{
	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 1024);
	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 3096);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
}

void ctrlDACControllerGenerate(void)
{
	sendChar(CMD_CONTROLLER);
	sendChar(CTRL_DAC_ID);
	sendString("Internal DAC");

	uint8_t i;
	for (i = 0; i < CTRL_DAC_CHANNELS; i++) {
		uint8_t id = channels[i];
		sendChar(id);
		ctrlByteType(CTRL_DAC_VALUE_TYPE, CTRL_DAC_VALUE_MIN, CTRL_DAC_VALUE_MAX, HAL_DAC_GetValue(&hdac, id));
		sendString(channelName[i]);

		// New column
		if (i != CTRL_DAC_CHANNELS - 1) {
			sendChar(id);
			sendChar(CTRL_NEW_COLUMN);
		}
	}
	sendChar(INVALID_ID);
	poolSending();
}

void ctrlDACController(void)
{
	uint8_t channel;
loop:
	if ((channel = receiveChar()) == INVALID_ID)
		return;
	uint16_t value;
	receiveData((uint8_t *)&value, CTRL_DAC_VALUE_BYTES);
	HAL_DAC_SetValue(&hdac, channel, DAC_ALIGN_12B_R, value);
	goto loop;
}
