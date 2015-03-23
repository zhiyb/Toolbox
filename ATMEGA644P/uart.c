#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <instructions.h>
#include <string.h>
#include "uart.h"
#include "info.h"

static struct rxBuffer_t {
	uint8_t *read;
	uint8_t * volatile write;
	uint8_t buffer[UART_BUFFER_SIZE];
} rx;

void initUART(void)
{
	// Initialise data structure
	rx.write = rx.read = rx.buffer;

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

void poolSending(void)
{
	while (!(UCSR0A & _BV(TXC0)));
}

ISR(USART0_RX_vect)
{
	uint8_t *ptr = rx.write;
	*ptr = UDR0;
	rx.write = ptr == rx.buffer + UART_BUFFER_SIZE - 1 ? rx.buffer : ptr + 1;
}

int receiveChar(void)
{
	while (rx.read == rx.write);
	uint8_t data = *rx.read;
	rx.read = rx.read == rx.buffer + UART_BUFFER_SIZE - 1 ? rx.buffer : rx.read + 1;
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
	char c;
	do
		sendChar(c = *string++);
	while (c != '\0');
}

void sendString_P(const char *string)
{
	char c;
	do
		sendChar(c = pgm_read_byte(string++));
	while (c != '\0');
}
