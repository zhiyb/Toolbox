#ifndef DAC_H
#define DAC_H

#include <inttypes.h>

#define CTRL_DAC1_ID		10
#define CTRL_DAC2_ID		11
#define CTRL_DAC_VALUE_BYTES	CTRL_BYTE2
#define CTRL_DAC_VALUE_TYPE	CTRL_DAC_VALUE_BYTES
#define CTRL_DAC_VALUE_MIN	0x00E0
#define CTRL_DAC_VALUE_MAX	0x0F1C

void initDAC(void);
void ctrlDACControllerGenerate(void);
void ctrlDACController(const uint8_t id);

#endif // DAC_H
