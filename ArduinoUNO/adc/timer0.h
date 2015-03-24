#ifndef TIMER0_H
#define TIMER0_H

#define TIMER0_ID		0
#define TIMER0_FREQUENCY	(SYS_CLK / 64)
#define TIMER0_RESOLUTION	8
#define TIMER0_BYTES		1

void initTimer0(void);
void startTimer0(void);
void stopTimer0(void);
void ctrlTimer0Controller(void);
void ctrlTimer0ControllerGenerate(void);

#endif
