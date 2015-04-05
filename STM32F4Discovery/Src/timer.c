#include <inttypes.h>
#include <instructions.h>
#include "timer.h"
#include "handles.h"
#include "uart.h"
#include "info.h"

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

static inline void setTimer(TIM_HandleTypeDef *handle, const uint32_t v)
{
	handle->Init.Period = v;
	HAL_TIM_Base_Init(handle);
}

void ctrlTimerController(void)
{
	TIM_HandleTypeDef* handle = TIMER_HANDLE(receiveChar());
	if (!handle)
		return;
	uint32_t value;
loop:
	switch (receiveChar()) {
	case CTRL_START:
		if (receiveChar())
			startTimer(handle);
		else
			stopTimer(handle);
		break;
	case CTRL_SET:
		receiveData((uint8_t *)&value, TIMER_BYTES(handle));
		setTimer(handle, value);
		break;
	case INVALID_ID:
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
	poolSending();
}
