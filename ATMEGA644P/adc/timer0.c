#include <avr/io.h>
#include <instructions.h>
#include "timer0.h"
#include "uart.h"
#include "info.h"

void initTimer0(void)
{
	// CTC mode
	TCCR0A = _BV(WGM01);
	stopTimer0();
}

void startTimer0(void)
{
	PORTB |= _BV(7);
	// Prescaler 64
	TCCR0B = _BV(CS01) | _BV(CS00);
}

void stopTimer0(void)
{
	TCCR0B = 0;
	TCNT0 = 0;
	PORTB &= ~_BV(7);
}

static inline void setTimer0(const uint8_t v)
{
	OCR0A = v;
}

void ctrlTimer0Controller(void)
{
	uint8_t value;
loop:
	switch (receiveChar()) {
	case CTRL_START:
		if (receiveChar())
			startTimer0();
		else
			stopTimer0();
		break;
	case CTRL_SET:
		receiveData((uint8_t *)&value, TIMER0_BYTES);
		setTimer0(value);
		break;
	case INVALID_ID:
	default:
		return;
	}
	goto loop;
}

void ctrlTimer0ControllerGenerate(void)
{
	sendChar(CMD_TIMER);			// Timer information
	sendChar(TIMER0_ID);			// ID
	sendChar(TIMER0_RESOLUTION);		// Resolution (bits)
	sendValue(TIMER0_FREQUENCY, 4);		// Frequency
}
