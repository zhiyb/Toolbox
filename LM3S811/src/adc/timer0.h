#ifndef TIMER0_H
#define TIMER0_H

#include "info.h"

#define TIMER0_ID		0
#define TIMER0_FREQUENCY	SYS_CLK
#define TIMER0_RESOLUTION	32
#define TIMER0_BYTES		BYTES(TIMER0_RESOLUTION)

void initTimer0(void);
void startTimer0(void);
void stopTimer0(void);
void ctrlTimer0Controller(void);
void ctrlTimer0ControllerGenerate(void);

#endif
