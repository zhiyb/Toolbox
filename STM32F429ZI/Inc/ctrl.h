#ifndef CTRL_H
#define CTRL_H

#include <inttypes.h>

extern volatile uint8_t pause;

void ctrlRootLoop(void);
void ctrlByteType(uint8_t type, uint32_t min, uint32_t max, uint32_t value);

#endif // CTRL_H
