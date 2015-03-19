#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer0.h"
#include "uart.h"
#include "ctrl.h"
#include "adc.h"

void init(void)
{
	DDRB |= _BV(7);
	PORTB |= _BV(7);
	initUART();
	initTimer0();
	initADC();
}

void reset(void)
{
	stopTimer0();
}

int main(void)
{
	init();
	sei();

	for (;;)
		ctrlRootLoop();
	return 1;
}
