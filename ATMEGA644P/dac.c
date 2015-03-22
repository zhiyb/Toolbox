#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <instructions.h>
#include "dac.h"
#include "uart.h"
#include "ctrl.h"

// In <avr/cpufunc.h>, but WinAVR doesn't have the file
#define _NOP() __asm__ __volatile__("nop")

static volatile uint8_t dac_data;

void initDAC(void)
{
	dac_data = 0;

	// Port initialisation
	DDRD |= DAC_LOAD | DAC_DATA | DAC_CLK;
	PORTD |= DAC_LOAD | DAC_DATA | DAC_CLK;
	// Configure UART1 in SPI mode
	// Enable transmit only
	UCSR1B = _BV(TXEN1);
	// Master SPI mode, MSB first, data setup at rising edge
	UCSR1C = _BV(UMSEL11) | _BV(UMSEL10) | _BV(UCPHA1);
	// BAUD = FOSC / (2 * (UBRR1 + 1))
	UBRR1H = 0;
	// Clock frequency max. 1MHz
	UBRR1L = F_CPU / 1000000 / 2 - 1;
	// Clear transmit complete flag
	UCSR1A |= _BV(TXC1);
}

static inline void setDAC(uint8_t ch, uint8_t data)
{
	UDR1 = (ch << 1) + 1;	// RNG = 1 for gain of 2x from ref
	dac_data = data;
	// Enable data register empty interrupt
	UCSR1B |= _BV(UDRE1);
	return;
}

static inline uint8_t getDAC(void)
{
	return dac_data;
}

// USART1 Data register empty interrupt
ISR(USART1_UDRE_vect)
{
	UDR1 = dac_data;
	// Disable data register empty interrupt
	UCSR1B &= ~_BV(UDRE1);
	// Enable transmit complete interrupt
	UCSR1B |= _BV(TXC1);
}

// USART1 Transmit complete interrupt
ISR(USART1_TX_vect, ISR_NOBLOCK)
{
	PORTD &= ~DAC_LOAD;	// Lowing DAC_LOAD to load
	//_NOP();		// tW(LOAD) min. 250ns
	// Disable transmit complete interrupt
	UCSR1B &= ~_BV(TXC1);
	PORTD |= DAC_LOAD;
}

void ctrlDACControllerGenerate(void)
{
	sendChar(CMD_CONTROLLER);
	sendChar(CTRL_DAC_ID);
	sendString_P(PSTR("DAC TLV5620"));

	sendChar(0);
	ctrlByteType(CTRL_DAC_VALUE_TYPE, CTRL_DAC_VALUE_MIN, CTRL_DAC_VALUE_MAX, getDAC());
	sendString_P(PSTR("Channel 0"));

	sendChar(1);
	ctrlByteType(CTRL_DAC_VALUE_TYPE, CTRL_DAC_VALUE_MIN, CTRL_DAC_VALUE_MAX, getDAC());
	sendString_P(PSTR("Channel 1"));

	sendChar(2);
	ctrlByteType(CTRL_DAC_VALUE_TYPE, CTRL_DAC_VALUE_MIN, CTRL_DAC_VALUE_MAX, getDAC());
	sendString_P(PSTR("Channel 2"));

	sendChar(3);
	ctrlByteType(CTRL_DAC_VALUE_TYPE, CTRL_DAC_VALUE_MIN, CTRL_DAC_VALUE_MAX, getDAC());
	sendString_P(PSTR("Channel 3"));
	sendChar(INVALID_ID);
}

void ctrlDACController(void)
{
	uint8_t channel;
loop:
	if ((channel = receiveChar()) == INVALID_ID)
		return;
	uint8_t value;
	receiveData((uint8_t *)&value, CTRL_DAC_VALUE_BYTES);
	setDAC(channel, value);
	goto loop;
}
