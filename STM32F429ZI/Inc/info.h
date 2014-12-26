#ifndef INFO_H
#define INFO_H

#include <instructions.h>

#define DEVICE_NAME	"STM32F429ZI"

#define CTRL_DAC1_ID		10
#define CTRL_DAC2_ID		11
#define CTRL_DAC_VALUE_BYTES	CTRL_BYTE2
#define CTRL_DAC_VALUE_TYPE	CTRL_DAC_VALUE_BYTES
#define CTRL_DAC_VALUE_MIN	0x00E0
#define CTRL_DAC_VALUE_MAX	0x0F1C

#endif // INFO_H
