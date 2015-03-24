#ifndef ADC_H
#define ADC_H

#include <inttypes.h>
#include "info.h"

#define CTRL_ADC_ID		0
#define CTRL_ADC_CHANNELS	4
#define CTRL_ADC_CHANNELS_BYTES	1
#define CTRL_ADC_RESOLUTION	8
#define CTRL_ADC_BYTES		1
#define CTRL_ADC_MAX_FREQUENCY	(SYS_CLK / 64 / (13 + 1))
#define CTRL_ADC_SCAN_FREQUENCY	(BAUD / 10 / (CTRL_ADC_BYTES + 4))
#if (CTRL_ADC_SCAN_FREQUENCY > CTRL_ADC_MAX_FREQUENCY)
#undef CTRL_ADC_SCAN_FREQUENCY
#define CTRL_ADC_SCAN_FREQUENCY	CTRL_ADC_MAX_FREQUENCY
#endif
#define CTRL_ADC_REF		3.3f
#define CTRL_ADC_OFFSET		0.f

#define ADC_BUFFER_SIZE		(3 * 1024)
// Frame mode:	Align, CMD, ID, Data start position, Data
#define ADC_ALIGN_BYTES		0
#define ADC_PREPEND_BYTES	7
// Scan mode:	Align, CMD, ID, Data
#define ADC_SCAN_ALIGN_BYTES	0
#define ADC_SCAN_PREPEND_BYTES	3

/*extern uint8_t adcBuffer[ADC_ALIGN_BYTES + ADC_PREPEND_BYTES + ADC_BUFFER_SIZE];
extern uint8_t *adcTxBuffer;
extern volatile uint16_t adcTxBufferLength;
extern volatile uint8_t adcTxBufferRequest;*/

void initADC(void);
void resetADC(void);
void ctrlADCController(const uint8_t id);
void ctrlADCControllerGenerate(void);

#endif // ADC_H
