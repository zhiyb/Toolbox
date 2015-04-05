#ifndef TIMER_H
#define TIMER_H

#include <inttypes.h>
#include "handles.h"
#include "info.h"
#include "stm32f4xx_hal.h"

#define TIMER_ID(handle)		((handle) == ADC_TIMER ? 0 : INVALID_ID)
#define TIMER_FREQUENCY(handle)		((handle) == ADC_TIMER ? APB1_TIMER_CLK : 0)
#define TIMER_RESOLUTION(handle)	((handle) == ADC_TIMER ? 32 : 0)
#define TIMER_BYTES(handle)		((handle) == ADC_TIMER ? 4 : 0)
#define TIMER_HANDLE(id)		((id) == 0 ? ADC_TIMER : 0)

void initTimer(TIM_HandleTypeDef* handle);
void startTimer(TIM_HandleTypeDef* handle);
void stopTimer(TIM_HandleTypeDef* handle);
void ctrlTimerController();
void ctrlTimerControllerGenerate(TIM_HandleTypeDef* handle);

#endif // TIMER_H

