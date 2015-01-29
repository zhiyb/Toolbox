#ifndef HANDLES_H
#define HANDLES_H

#include "stm32f4xx_hal.h"

#define HUART		huart8

#define ADC_HW		(&hadc1)
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern DAC_HandleTypeDef hdac;

extern RNG_HandleTypeDef hrng;

#define ADC_TIMER	(&htim5)
extern TIM_HandleTypeDef htim5;		// ADC Timer

//extern SD_HandleTypeDef hsd;
//extern HAL_SD_CardInfoTypedef SDCardInfo;

extern UART_HandleTypeDef huart8;
extern DMA_HandleTypeDef hdma_uart8_tx;

void reset(void);

#endif // HANDLES
