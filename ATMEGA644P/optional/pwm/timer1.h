#ifndef TIMER1_H
#define TIMER1_H

#include <inttypes.h>

#define timer1Top	ICR1
#define timer1OCA	OCR1A
#define timer1OCB	OCR1B

void initTimer1(void);

#endif
