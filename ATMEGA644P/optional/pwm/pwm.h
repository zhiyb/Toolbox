#ifndef PWM_H
#define PWM_H

#define CTRL_PWM_ID		20
#define CTRL_PWM_CHANNELS	2
#define CTRL_PWM_VALUE_BYTES	CTRL_BYTE2
#define CTRL_PWM_VALUE_TYPE	CTRL_PWM_VALUE_BYTES
#define CTRL_PWM_VALUE_MIN	0x0000
#define CTRL_PWM_VALUE_MAX	0xFFFF

#include <inttypes.h>

// PWM using timer 1, PORTD
#define PWM_DDR	DDRD
#define PWM_OCA	_BV(5)
#define PWM_OCB	_BV(4)

void initPWM(void);
void ctrlPWMControllerGenerate(void);
void ctrlPWMController(void);

#endif
