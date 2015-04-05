#ifndef DAC_H
#define DAC_H

#define CTRL_DAC_ID		10
#define CTRL_DAC_CHANNELS	2
#define CTRL_DAC_VALUE_BYTES	CTRL_BYTE2
#define CTRL_DAC_VALUE_TYPE	CTRL_DAC_VALUE_BYTES
//#define CTRL_DAC_VALUE_MIN	0x00E0
//#define CTRL_DAC_VALUE_MAX	0x0F1C
#define CTRL_DAC_VALUE_MIN	0x0000
#define CTRL_DAC_VALUE_MAX	0x0FFF

#include <inttypes.h>

void initDAC(void);
void ctrlDACControllerGenerate(void);
void ctrlDACController(void);

#endif
