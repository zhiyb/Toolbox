#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <instructions.h>
#include <string.h>
#include "uart.h"
#include "info.h"
#include "handles.h"

// Device specific
void initUART(void)
{
	// UART GPIO init
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// UART init
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	UARTEnable(UART_BASE);
	UARTFIFOEnable(UART_BASE);
	UARTConfigSetExpClk(UART_BASE, SysCtlClockGet(), BAUD, \
			    UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
	sendString(__DATE__ ", " __TIME__ " | Hello, world!\r\n");
}

void poolSending(void)
{
	while (UARTBusy(UART_BASE));
}

int receiveChar(void)
{
	return UARTCharGet(UART_BASE);
}

void sendChar(char c)
{
	UARTCharPut(UART_BASE, c);
}

// General
uint16_t receiveData(uint8_t *data, uint16_t count)
{
	uint8_t i = count;
	while (count--)
		*data++ = receiveChar();
	return i;
}

void sendData(uint8_t *buffer, uint16_t length)
{
	while (length--)
		sendChar(*buffer++);
}

void sendValue(uint32_t value, uint8_t bytes)
{
	sendData((uint8_t *)&value, bytes);
}

void sendString(const char *string)
{
	char c;
	do
		sendChar(c = *string++);
	while (c != '\0');
}
