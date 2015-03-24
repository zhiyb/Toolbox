#include <avr/io.h>
#include "timer1.h"

void initTimer1(void)
{
	// PWM, Phase and frequency corrent, top: ICR1
	TCCR1A = _BV(COM1A1) | _BV(COM1B1);
	TCCR1B = _BV(WGM13);
	TCCR1C = 0;
	TCNT1 = 0;
	timer1OCA = 0;
	timer1OCB = 0;
	timer1Top = 0;
	TIMSK1 = 0;
	TCCR1B |= _BV(CS10);
}
