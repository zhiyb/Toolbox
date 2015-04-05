#ifndef ADC_H
#define ADC_H

#include <inttypes.h>
#include "info.h"

#define ADC_BUFFER_SIZE		(32 * 1024)
// Frame mode:	Align, CMD, ID, Data type (frame), Data start position
#define ADC_ALIGN_BYTES		1
#define ADC_PREPEND_BYTES	7
// Scan mode:	Align, CMD, ID, Data type (data), Data
#define ADC_SCAN_ALIGN_BYTES	1
#define ADC_SCAN_PREPEND_BYTES	3

#define CTRL_ADC_ID		0
#define CTRL_ADC_CHANNELS	8
#define CTRL_ADC_CHANNELS_BYTES	BYTES(CTRL_ADC_CHANNELS)
#define CTRL_ADC_RESOLUTION	12
#define CTRL_ADC_BYTES		BYTES(CTRL_ADC_RESOLUTION)
#define CTRL_ADC_MAX_FREQUENCY	(APB2_CLK / (3 + 12 + 1))
#define CTRL_ADC_SCAN_FREQUENCY	(BAUD / 10 / (CTRL_ADC_BYTES + ADC_SCAN_PREPEND_BYTES + 1))
#if (CTRL_ADC_SCAN_FREQUENCY > CTRL_ADC_MAX_FREQUENCY)
#undef CTRL_ADC_SCAN_FREQUENCY
#define CTRL_ADC_SCAN_FREQUENCY	CTRL_ADC_MAX_FREQUENCY
#endif
#define CTRL_ADC_REF		3.3f
#define CTRL_ADC_OFFSET		0.f

extern uint8_t *adcTxBuffer;
extern uint16_t adcTxBufferLength;
extern volatile uint16_t adcTxBufferRequest;

void initADC(void);
void resetADC(void);
void ctrlADCController(const uint8_t id);
void ctrlADCControllerGenerate(void);

#endif // ADC_H
