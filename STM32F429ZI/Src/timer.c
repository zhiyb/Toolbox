#include "timer.h"
#include "handles.h"
#include "communication.h"

void initTimer(TIM_HandleTypeDef* handle)
{
	stopTimer(handle);
}

void startTimer(TIM_HandleTypeDef *handle)
{
	HAL_TIM_Base_Start(handle);
}

void stopTimer(TIM_HandleTypeDef *handle)
{
	HAL_TIM_Base_Stop(handle);
}

void ctrlTimerController()
{
	TIM_HandleTypeDef* handle = TIMER_HANDLE(receiveChar(-1));
	if (!handle)
		return;
	uint32_t value;
loop:
	switch (receiveChar(-1)) {
	case CTRL_START:
		if (receiveChar(-1))
			startTimer(handle);
		else
			stopTimer(handle);
		break;
	case CTRL_SET:
		receiveData((uint8_t *)&value, TIMER_BYTES(handle), -1);
		//HAL_TIM_Base_DeInit(handle);
		handle->Init.Period = value;
		HAL_TIM_Base_Init(handle);
		break;
	default:
		return;
	}
	goto loop;
}

void ctrlTimerControllerGenerate(TIM_HandleTypeDef* handle)
{
	sendChar(CMD_TIMER);				// Timer information
	sendChar(TIMER_ID(handle));			// ID
	if (TIMER_ID(handle) == INVALID_ID)
		return;
	sendChar(TIMER_RESOLUTION(handle));		// Resolution (bits)
	sendValue(TIMER_FREQUENCY(handle), 4);		// Frequency
}
