#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <driverlib/sysctl.h>
#include <driverlib/pwm.h>
#include <driverlib/gpio.h>
#include <inttypes.h>
#include "timer0.h"
#include "uart.h"
#include "ctrl.h"
#include "adc.h"
#ifdef ENABLE_PWM
#include "pwm.h"
#endif

void init(void)
{
	// Clock init
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_6MHZ);

	initUART();
	initTimer0();
	initADC();

#ifdef ENABLE_PWM
	initPWM();
	// PWM init
	SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, SYS_CLK / 5000);
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, SYS_CLK / 5000 / 2);
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, SYS_CLK / 5000 / 4);
	PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT | PWM_OUT_1_BIT, true);
	PWMGenEnable(PWM0_BASE, PWM_GEN_0);
#endif

	//HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_3);
}

void reset(void)
{
	resetADC();
}

int main(void)
{
	init();

	for (;;)
		ctrlRootLoop();
	return 1;
}
