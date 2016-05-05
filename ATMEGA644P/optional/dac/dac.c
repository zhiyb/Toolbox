#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <instructions.h>
#include <uart.h>
#include "dac.h"
#include "ctrl.h"

// In <avr/cpufunc.h>, but WinAVR doesn't have the file
#define _NOP() __asm__ __volatile__("nop")

const static PROGMEM char ch0[] = "Channel 0";
const static PROGMEM char ch1[] = "Channel 1";
const static PROGMEM char ch2[] = "Channel 2";
const static PROGMEM char ch3[] = "Channel 3";
const static PROGMEM PGM_P const channelName[] = {ch0, ch1, ch2, ch3};

static volatile uint8_t dacReady;
static uint8_t dac_data;
static uint8_t dacData[CTRL_DAC_CHANNELS];

static void setDAC(uint8_t ch, uint8_t data);

void initDAC(void)
{
#ifdef DAC_USE_PORTD
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
	//UCSR1A |= _BV(TXC1);
#else
	// Port initialisation
	DDRB |= DAC_LOAD | DAC_DATA | DAC_CLK;
	PORTB |= DAC_LOAD | DAC_DATA | DAC_CLK;
	// Use native SPI
	// Master SPI mode, MSB first, data setup at rising edge
	// Clock frequency max. 1MHz
	SPCR = _BV(MSTR) | _BV(CPHA) | _BV(SPR0);
	SPSR = _BV(SPI2X);
	SPCR |= _BV(SPE);
	// Clear transmit complete flag
#endif

	dacReady = 2;
	uint8_t i;
	for (i = 0; i < CTRL_DAC_CHANNELS; i++)
		setDAC(i, 0);
}

static void setDAC(uint8_t ch, uint8_t data)
{
	while (dacReady != 2);
	// RNG = 1 for gain of 2x from ref
#ifdef DAC_USE_PORTD
	UDR1 = (ch << 1) + 1;
#else
	SPDR = (ch << 1) + 1;
#endif
	dacReady = 0;
	dac_data = dacData[ch] = data;

#ifdef DAC_USE_PORTD
	// Enable data register empty interrupt
	UCSR1B |= _BV(UDRIE1);
#else
	// Enable SPI interrupt
	SPCR |= _BV(SPIE);
#endif
}

#ifdef DAC_USE_PORTD
// USART1 Data register empty interrupt
ISR(USART1_UDRE_vect)
{
	UDR1 = dac_data;
	UCSR1A |= _BV(TXC1);
	// Enable transmit complete, disable data register empty
	UCSR1B = (UCSR1B & ~_BV(UDRIE1)) | _BV(TXCIE1);
}

// USART1 Transmit complete interrupt
ISR(USART1_TX_vect, ISR_NOBLOCK)
{
	PORTD &= ~DAC_LOAD;	// Lowing DAC_LOAD to load
	//_NOP();		// tW(LOAD) min. 250ns
	// Disable transmit complete interrupt
	UCSR1B &= ~_BV(TXCIE1);
	dacReady = 2;
	PORTD |= DAC_LOAD;
}
#else
// SPI Transmit complete interrupt
ISR(SPI_STC_vect, ISR_NOBLOCK)
{
	if (dacReady == 0) {
		SPDR = dac_data;
		dacReady = 1;
		return;
	}

	PORTB &= ~DAC_LOAD;	// Lowing DAC_LOAD to load
	//_NOP();		// tW(LOAD) min. 250ns
	// Disable SPI interrupt
	SPCR &= ~_BV(SPIE);
	dacReady = 2;
	PORTB |= DAC_LOAD;
}
#endif

void ctrlDACControllerGenerate(void)
{
	sendChar(CMD_CONTROLLER);
	sendChar(CTRL_DAC_ID);
	sendString_P(PSTR("DAC TLV5620"));

	uint8_t i;
	for (i = 0; i < CTRL_DAC_CHANNELS; i++) {
		sendChar(i);
		ctrlByteType(CTRL_DAC_VALUE_TYPE, CTRL_DAC_VALUE_MIN, CTRL_DAC_VALUE_MAX, dacData[i]);
		sendString_P((PGM_P)pgm_read_word(channelName + i));

		// New column
		if (i != CTRL_DAC_CHANNELS - 1) {
			sendChar(i);
			sendChar(CTRL_NEW_COLUMN);
		}
	}
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
	setDAC(channel % CTRL_DAC_CHANNELS, value);
	goto loop;
}
