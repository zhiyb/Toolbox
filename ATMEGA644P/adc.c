#include <avr/io.h>
#include <avr/interrupt.h>
#include <instructions.h>
#include "timer0.h"
#include "ctrl.h"
#include "uart.h"
#include "adc.h"

uint8_t adcBuffer[ADC_ALIGN_BYTES + ADC_PREPEND_BYTES + ADC_BUFFER_SIZE];
uint8_t *adcTxBuffer;
uint8_t *adcBufferStart, *adcBufferCurrent, *adcBufferEnd;
volatile uint16_t adcTxBufferLength;
volatile uint8_t adcTxBufferRequest;

static uint32_t adcBufferCount;	// Buffered conversions count
static uint8_t channelEnabled = 0xFF;
static uint8_t channelCount = CTRL_ADC_CHANNELS, scanMode = 1;

static uint32_t floatToRawUInt32(float x)
{
	union {
		float f;	// assuming 32-bit IEEE 754 single-precision
		uint32_t i;	// assuming 32-bit unsigned int
	} u;
	u.f = x;
	return u.i;
}

void initADC(void)
{
	ADMUX = _BV(ADLAR) | 0;
	// Prescaler 64
	ADCSRA = _BV(ADATE) | _BV(ADIF) | _BV(ADIE) | 3;
	ADCSRB = _BV(ADTS1) | _BV(ADTS0);
	DIDR0 |= _BV(ADC0D);
	ADCSRA |= _BV(ADEN) | _BV(ADSC);
}

void resetADC(void)
{
}

void startADC(void)
{
	if (!channelCount)
		return;
	if (scanMode) {
		adcBuffer[ADC_SCAN_ALIGN_BYTES + 0] = CMD_ANALOGDATA;
		adcBuffer[ADC_SCAN_ALIGN_BYTES + 1] = CTRL_ADC_ID;
		adcBuffer[ADC_SCAN_ALIGN_BYTES + 2] = CTRL_DATA;
		adcTxBufferLength = ADC_SCAN_PREPEND_BYTES + channelCount * CTRL_ADC_BYTES;
		adcTxBuffer = adcBuffer + ADC_SCAN_ALIGN_BYTES;
		adcBufferCurrent = adcBufferStart = adcTxBuffer + ADC_SCAN_PREPEND_BYTES;
		adcBufferEnd = adcBufferStart + channelCount * CTRL_ADC_BYTES;
	} else {
		adcBuffer[ADC_ALIGN_BYTES + 0] = CMD_ANALOGDATA;
		adcBuffer[ADC_ALIGN_BYTES + 1] = CTRL_ADC_ID;
		adcBuffer[ADC_ALIGN_BYTES + 2] = CTRL_FRAME;
		*(uint32_t *)(adcBuffer + ADC_ALIGN_BYTES + 3) = 0;
		adcTxBufferLength = ADC_PREPEND_BYTES + adcBufferCount * CTRL_ADC_BYTES;
		adcTxBuffer = adcBuffer + ADC_ALIGN_BYTES;
		adcBufferCurrent = adcBufferStart = adcTxBuffer + ADC_PREPEND_BYTES;
		adcBufferEnd = adcBufferStart + adcBufferCount * CTRL_ADC_BYTES;
	}
	PORTB |= _BV(7);
	startTimer0();
	TIFR0 |= _BV(OCF0A);
}

void stopADC(void)
{
	PORTB &= ~_BV(7);
	stopTimer0();
}

void configureADC(void)
{
	stopADC();
}

void ctrlADCController(const uint8_t id)
{
loop:
	switch (receiveChar()) {
	case CTRL_START:
		if (receiveChar())
			startADC();
		else
			stopADC();
		break;
	case CTRL_SET:
		receiveData((uint8_t *)&channelEnabled, CTRL_ADC_CHANNELS_BYTES);
		configureADC();
		break;
	case CTRL_DATA:
		scanMode = receiveChar() == CTRL_DATA;
		adcBufferCount = channelCount;
		stopADC();
		break;
	case CTRL_FRAME:
		receiveData((uint8_t *)&adcBufferCount, 4);
		adcBufferCount *= channelCount;
		break;
	case INVALID_ID:
	default:
		return;
	}
	goto loop;
}

void ctrlADCControllerGenerate(void)
{
	const static char* channels[CTRL_ADC_CHANNELS] = {
		"CH_1",
	};
	sendChar(CMD_ANALOG);		// Analog waveform (ADC) customised controller
	sendChar(CTRL_ADC_ID);			// ID
	sendString("ADC");			// Name
	sendChar(CTRL_ADC_RESOLUTION);		// Result resolution (bits)
	sendValue(CTRL_ADC_SCAN_FREQUENCY, 4);	// Scan mode maximum transfer frequency
	sendValue(CTRL_ADC_MAX_FREQUENCY, 4);	// Maximum sampling frequency
	sendChar(CTRL_ADC_CHANNELS);		// Channels
	uint8_t i;
	for (i = 0; i < CTRL_ADC_CHANNELS; i++) {
		sendChar(i);			// Channel ID
		sendString(channels[i]);	// Channel name
		sendValue(floatToRawUInt32(CTRL_ADC_REF), 4);
		sendValue(floatToRawUInt32(CTRL_ADC_OFFSET), 4);
	}
	sendValue(channelEnabled, CTRL_ADC_CHANNELS_BYTES);
	sendValue(ADC_BUFFER_SIZE / CTRL_ADC_BYTES, 4);	// ADC conversion buffer size
	ctrlTimer0ControllerGenerate();	// ADC trigger timer information
	sendChar(CMD_END);			// End settings
}

ISR(ADC_vect, ISR_NOBLOCK)
{
	TIFR0 |= _BV(OCF0A);
	*adcBufferCurrent++ = ADCH;
	if (adcBufferCurrent == adcBufferEnd)
		adcBufferCurrent = adcBufferStart;

	if (scanMode) {
		if (pause)
			adcTxBufferRequest++;
		else
			sendData(adcTxBuffer, adcTxBufferLength);
		return;
	}

	if (pause)
		return;
	if (adcBufferCurrent != adcBufferStart)
		return;
	stopADC();
	sendData(adcTxBuffer, adcTxBufferLength);
	startADC();
}
