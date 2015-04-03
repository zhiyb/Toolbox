#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>
#include <driverlib/sysctl.h>
#include <inttypes.h>
#include <instructions.h>
#include "timer0.h"
#include "uart.h"
#include "info.h"

static uint32_t timerV;

void initTimer0(void)
{
	// Timer init
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
	TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
	TimerControlStall(TIMER0_BASE, TIMER_A, true);
	TimerLoadSet(TIMER0_BASE, TIMER_A, SYS_CLK);
	stopTimer0();
}

void startTimer0(void)
{
	TimerEnable(TIMER0_BASE, TIMER_A);
}

void stopTimer0(void)
{
	TimerDisable(TIMER0_BASE, TIMER_A);
}

static inline void setTimer0(const uint32_t v)
{
	timerV = v;
	TimerLoadSet(TIMER0_BASE, TIMER_A, v);
}

void ctrlTimer0Controller(void)
{
	uint32_t value;
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
