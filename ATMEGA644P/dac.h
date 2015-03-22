#ifndef DAC_H
#define DAC_H

#ifdef __cplusplus
extern "C" {
#endif

#define CTRL_DAC_ID		10
#define CTRL_DAC_VALUE_BYTES	CTRL_BYTE1
#define CTRL_DAC_VALUE_TYPE	CTRL_DAC_VALUE_BYTES
#define CTRL_DAC_VALUE_MIN	0x00
#define CTRL_DAC_VALUE_MAX	0xFF

#include <inttypes.h>

// DAC chip TLV5620 uses PORTD with UART1
// DAC_LDAC set to LOW by hardware
#define DAC_LOAD	(1 << 2)
#define DAC_DATA	(1 << 3)
#define DAC_CLK		(1 << 4)

void initDAC(void);
void ctrlDACControllerGenerate(void);
void ctrlDACController(void);

#ifdef __cplusplus
}
#endif

#endif
