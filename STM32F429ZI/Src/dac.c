#include <instructions.h>
#include "stm32f4xx_hal.h"
#include "communication.h"
#include "dac.h"
#include "info.h"
#include "handles.h"
#include "ctrl.h"

void ctrlDACControllerGenerate(void)
{
	sendChar(CMD_CONTROLLER);	// Operation: Controller generate
	sendChar(CTRL_DAC1_ID);		// Controller ID
	sendString("DAC Channel 1");	// Controller name
	sendChar(DAC_CHANNEL_1);	// Settings ID
	ctrlByteType(CTRL_DAC_VALUE_TYPE, CTRL_DAC_VALUE_MIN, CTRL_DAC_VALUE_MAX, HAL_DAC_GetValue(&hdac, DAC_CHANNEL_1));
	sendString("Value");		// Settings name
	//sendChar(-1);
	sendChar(CMD_END);		// End settings

	sendChar(CMD_CONTROLLER);
	sendChar(CTRL_DAC2_ID);
	sendString("DAC Channel 2");
	sendChar(DAC_CHANNEL_2);
	ctrlByteType(CTRL_DAC_VALUE_TYPE, CTRL_DAC_VALUE_MIN, CTRL_DAC_VALUE_MAX, HAL_DAC_GetValue(&hdac, DAC_CHANNEL_2));
	sendString("Value");
	//sendChar(-1);
	sendChar(CMD_END);
}

void ctrlDACController(const uint8_t id)
{
	uint8_t channel = receiveChar(-1);
	uint16_t value;
	receiveData((uint8_t *)&value, CTRL_DAC_VALUE_BYTES, -1);
	HAL_DAC_SetValue(&hdac, channel, DAC_ALIGN_12B_R, value);
}
