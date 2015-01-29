#ifndef TIMER_H
#define TIMER_H

#include <inttypes.h>
#include "info.h"
#include "stm32f4xx_hal.h"

void initTimer(TIM_HandleTypeDef* handle);
void startTimer(TIM_HandleTypeDef* handle);
void stopTimer(TIM_HandleTypeDef* handle);
void ctrlTimerController();
void ctrlTimerControllerGenerate(TIM_HandleTypeDef* handle);

#endif // TIMER_H

