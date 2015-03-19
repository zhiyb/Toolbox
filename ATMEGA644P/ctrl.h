#ifndef CTRL_H
#define CTRL_H

#include <inttypes.h>

extern volatile uint8_t pause;

void ctrlRootLoop(void);
void ctrlDeviceInfo(void);

#endif
