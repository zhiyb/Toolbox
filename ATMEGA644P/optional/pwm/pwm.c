#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <instructions.h>
#include <uart.h>
#include <ctrl.h>
#include "pwm.h"
#include "timer1.h"

// PWM settings
#define PWM_SET_TOP	0
#define PWM_SET_OCA	1
#define PWM_SET_OCB	2

void initPWM(void)
{
	// Port initialisation
	DDRD |= PWM_OCA | PWM_OCB;
	PORTD |= PWM_OCA | PWM_OCB;

	// Timer 1 initialisation
	initTimer1();
}

static inline void setPWMTop(uint16_t data)
{
	timer1Top = data;
}

static inline void setPWMOCA(uint16_t data)
{
	timer1OCA = data;
}

static inline void setPWMOCB(uint16_t data)
{
	timer1OCB = data;
}

static inline uint16_t getPWMTop(void)
{
	return timer1Top;
}

static inline uint16_t getPWMOCA(void)
{
	return timer1OCA;
}

static inline uint16_t getPWMOCB(void)
{
	return timer1OCB;
}

void ctrlPWMControllerGenerate(void)
{
	sendChar(CMD_CONTROLLER);
	sendChar(CTRL_PWM_ID);
	sendString_P(PSTR("PWM Timer1"));

	sendChar(PWM_SET_TOP);
	ctrlByteType(CTRL_PWM_VALUE_TYPE, CTRL_PWM_VALUE_MIN, CTRL_PWM_VALUE_MAX, getPWMTop());
	sendString_P(PSTR("Top"));

	sendChar(PWM_SET_TOP);
	sendChar(CTRL_NEW_COLUMN);

	sendChar(PWM_SET_OCA);
	ctrlByteType(CTRL_PWM_VALUE_TYPE, CTRL_PWM_VALUE_MIN, CTRL_PWM_VALUE_MAX, getPWMOCA());
	sendString_P(PSTR("Channel A"));

	sendChar(PWM_SET_OCA);
	sendChar(CTRL_NEW_COLUMN);

	sendChar(PWM_SET_OCB);
	ctrlByteType(CTRL_PWM_VALUE_TYPE, CTRL_PWM_VALUE_MIN, CTRL_PWM_VALUE_MAX, getPWMOCB());
	sendString_P(PSTR("Channel B"));

	sendChar(INVALID_ID);
}

void ctrlPWMController(void)
{
	uint8_t channel;
loop:
	if ((channel = receiveChar()) == INVALID_ID)
		return;
	uint16_t value;
	receiveData((uint8_t *)&value, CTRL_PWM_VALUE_BYTES);
	switch (channel) {
	case PWM_SET_TOP:
		setPWMTop(value);
		break;
	case PWM_SET_OCA:
		setPWMOCA(value);
		break;
	case PWM_SET_OCB:
		setPWMOCB(value);
		break;
	}
	goto loop;
}
