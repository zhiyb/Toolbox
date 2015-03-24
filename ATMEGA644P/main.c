#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer0.h"
#include "uart.h"
#include "ctrl.h"
#include "adc.h"
#ifdef ENABLE_DAC
#include "dac.h"
#endif
#ifdef ENABLE_PWM
#include "pwm.h"
#endif

void init(void)
{
	DDRB |= _BV(7);
	PORTB |= _BV(7);
	sei();
	initUART();
	initTimer0();
	initADC();
#ifdef ENABLE_DAC
	initDAC();
#endif
#ifdef ENABLE_PWM
	initPWM();
#endif
}

void reset(void)
{
	resetADC();
	stopTimer0();
}

int main(void)
{
	init();

	for (;;)
		ctrlRootLoop();
	return 1;
}
