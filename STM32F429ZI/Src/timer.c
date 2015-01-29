#include "timer.h"
#include "handles.h"
#include "communication.h"

#define TIMER_ID(handle)		(handle == ADC_TIMER ? 0 : INVALID_ID)
#define TIMER_FREQUENCY(handle)		(handle == ADC_TIMER ? SYS_CLK : 0)
#define TIMER_RESOLUTION(handle)	(handle == ADC_TIMER ? 32 : 0)
#define TIMER_HANDLE(id)		(id == 0 ? ADC_TIMER : 0)

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
