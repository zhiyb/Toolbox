#ifndef ADC_H
#define ADC_H

#include <inttypes.h>
#include "info.h"

#define CTRL_ADC_ID		0
#define CTRL_ADC_CHANNELS	8
#define CTRL_ADC_CHANNELS_BYTES	((CTRL_ADC_CHANNELS + 7) / 8)
#define CTRL_ADC_BYTES		2
#define CTRL_ADC_RESOLUTION	12
#define CTRL_ADC_SCAN_FREQUENCY	(BAUD / 10 / CTRL_ADC_BYTES / 200)
#define CTRL_ADC_REF		3.3f
#define CTRL_ADC_OFFSET		0.f

extern uint8_t adcTxBuffer[CTRL_ADC_CHANNELS * CTRL_ADC_BYTES + 3];
extern volatile uint8_t adcTxBufferRequest, adcTxBufferLength;

void initADC(void);
void resetADC(void);
void ctrlADCController(const uint8_t id);
void ctrlADCControllerGenerate(void);

#endif // ADC_H
