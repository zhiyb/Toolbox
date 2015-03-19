#include <avr/io.h>
#include <avr/interrupt.h>
#include <instructions.h>
#include <string.h>
#include "uart.h"

static volatile uint8_t cnt, data;

void initUART(void)
{
	// Initialise UART0_TX
	#include <util/setbaud.h>
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0A = USE_2X << U2X0;
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);

	// Clear flags
	UCSR0A |= _BV(TXC0);
	UDR0;

	// Interrupt
	UCSR0B |= _BV(RXCIE0);
}

void pollSending(void)
{
	while (!(UCSR0A & _BV(TXC0)));
}

int receiveChar(void)
{
	while (!cnt);
	cnt = 0;
	return data;
}

uint32_t receiveData(uint8_t *data, uint32_t count)
{
	uint8_t i = count;
	while (count--)
		*data++ = receiveChar();
	return i;
}

void sendChar(char c)
{
	while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = c;
}

ISR(USART0_RX_vect)
{
	data = UDR0;
	cnt++;
}

void sendData(uint8_t *buffer, uint32_t length)
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
	sendData((uint8_t *)string, strlen(string) + 1);
}
