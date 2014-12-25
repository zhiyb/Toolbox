#ifndef HANDLES
#define HANDLES

#include "stm32f4xx_hal.h"

#define HUART		huart8

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern DAC_HandleTypeDef hdac;

extern RNG_HandleTypeDef hrng;

extern SD_HandleTypeDef hsd;
extern HAL_SD_CardInfoTypedef SDCardInfo;

extern UART_HandleTypeDef huart8;
extern DMA_HandleTypeDef hdma_uart8_tx;

#endif // HANDLES
