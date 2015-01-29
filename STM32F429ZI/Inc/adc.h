#ifndef ADC_H
#define ADC_H

#include <inttypes.h>
#include "info.h"

#define CTRL_ADC_ID			0
#define CTRL_ADC_CHANNELS		8
#define CTRL_ADC_RESOLUTION		12

void initADC(void);
void resetADC(void);
void ctrlADCController(void);
void ctrlADCControllerGenerate(void);

#endif // ADC_H
