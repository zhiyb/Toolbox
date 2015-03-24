#ifndef PWM_H
#define PWM_H

#define CTRL_PWM_ID		20
#define CTRL_PWM_CHANNELS	2
#define CTRL_PWM_VALUE_BYTES	CTRL_BYTE2
#define CTRL_PWM_VALUE_TYPE	CTRL_PWM_VALUE_BYTES
#define CTRL_PWM_VALUE_MIN	0x0000
#define CTRL_PWM_VALUE_MAX	0xFFFF

#include <inttypes.h>

// PWM using timer 1, PORTB
#define PWM_DDR	DDRB
#define PWM_OCA	_BV(1)
#define PWM_OCB	_BV(2)

void initPWM(void);
void ctrlPWMControllerGenerate(void);
void ctrlPWMController(void);

#endif
