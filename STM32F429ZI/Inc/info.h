#ifndef INFO_H
#define INFO_H

#include <instructions.h>

#define DEVICE_NAME	"STM32F429ZI"
#define SYS_CLK		180000000
#define APB1_CLK	(SYS_CLK / 4)
#define APB1_TIMER_CLK	(APB1_CLK * 2)
#define APB2_CLK	(SYS_CLK / 2)
#define APB2_TIMER_CLK	(APB2_CLK * 2)
#define BAUD		UART_BAUD

#endif // INFO_H
