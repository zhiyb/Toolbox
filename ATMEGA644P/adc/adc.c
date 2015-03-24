#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <instructions.h>
#include "timer0.h"
#include "ctrl.h"
#include "uart.h"
#include "adc.h"

const static PROGMEM char ch0[] = "CH_0";
const static PROGMEM char ch1[] = "CH_1";
const static PROGMEM char ch2[] = "CH_2";
const static PROGMEM char ch3[] = "CH_3";
const static PROGMEM char ch4[] = "CH_4";
const static PROGMEM char ch5[] = "CH_5";
const static PROGMEM char ch6[] = "CH_6";
const static PROGMEM char ch7[] = "CH_7";
const static PROGMEM PGM_P const channelName[] = {ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7};
const static PROGMEM uint8_t channels[] = {0, 1, 2, 3, 4, 5, 6, 7};

static uint8_t channelSequence[CTRL_ADC_CHANNELS + 1];
static uint8_t *chSeqCurrent;

static uint8_t adcBuffer[ADC_ALIGN_BYTES + ADC_PREPEND_BYTES + ADC_BUFFER_SIZE];
uint8_t *adcTxBuffer;
static uint8_t *adcBufferStart, *adcBufferCurrent, *adcBufferEnd;
uint16_t adcTxBufferLength;
volatile uint8_t adcTxBufferRequest;

static uint16_t adcBufferCount;	// Buffered conversions count
static uint8_t channelEnabled = 0xFF;
static uint8_t channelCount = CTRL_ADC_CHANNELS, scanMode = 1;

static inline void stopADC(void);
static void configureADC(void);

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
	// Initialise data structure
	configureADC();

	ADMUX = _BV(ADLAR) | 0;
	// Prescaler 64
	ADCSRA = _BV(ADATE) | _BV(ADIF) | _BV(ADIE) | 6;
	ADCSRB = _BV(ADTS1) | _BV(ADTS0);
	DIDR0 = 0xFF;
	ADCSRA |= _BV(ADEN);

	resetADC();
}

void resetADC(void)
{
	stopADC();
}

static inline void pauseADC(void)
{
	stopTimer0();
}

static inline void resumeADC(void)
{
	TIFR0 |= _BV(OCF0A);
	startTimer0();
}

static void startADC(void)
{
	adcTxBufferRequest = 0;
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
	pauseADC();
	while (ADCSRA & _BV(ADSC));
	ADCSRA |= _BV(ADIE);
	resumeADC();
}

static inline void stopADC(void)
{
	pauseADC();
	ADCSRA &= ~(_BV(ADSC) | _BV(ADIE));
	adcTxBufferRequest = 0;
}

static void configureADC(void)
{
	uint8_t mask = 0x01, i;
	channelCount = 0;
	chSeqCurrent = channelSequence;
	for (i = 0; i < CTRL_ADC_CHANNELS; i++) {
		if (channelEnabled & mask) {
			*chSeqCurrent++ = pgm_read_byte(channels + i);
			channelCount++;
		}
		mask <<= 1;
	}
	*chSeqCurrent = INVALID_ID;
	chSeqCurrent = channelSequence;
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
		scanMode = receiveChar();
		break;
	case CTRL_FRAME: {
		uint32_t cnt;
		receiveData((uint8_t *)&cnt, 4);
		adcBufferCount = cnt * channelCount;
		break;
	}
	case INVALID_ID:
	default:
		return;
	}
	goto loop;
}

void ctrlADCControllerGenerate(void)
{
	sendChar(CMD_ANALOG);		// Analog waveform (ADC) customised controller
	sendChar(CTRL_ADC_ID);			// ID
	sendString_P(PSTR("ADC"));		// Name
	sendChar(CTRL_ADC_RESOLUTION);		// Result resolution (bits)
	sendValue(CTRL_ADC_SCAN_FREQUENCY, 4);	// Scan mode maximum transfer frequency
	sendValue(CTRL_ADC_MAX_FREQUENCY, 4);	// Maximum sampling frequency
	sendChar(CTRL_ADC_CHANNELS);		// Channels
	uint8_t i;
	for (i = 0; i < CTRL_ADC_CHANNELS; i++) {
		sendChar(i);			// Channel ID
		sendString_P((PGM_P)pgm_read_word(channelName + i));
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
	*adcBufferCurrent = ADCH;
	adcBufferCurrent = adcBufferCurrent == adcBufferEnd - 1 ? adcBufferStart : adcBufferCurrent + 1;

	uint8_t *ptr = chSeqCurrent + 1;
	if (*ptr != INVALID_ID) {
		// Change ADC channel
		ADMUX = _BV(ADLAR) | *ptr;
		ADCSRA |= _BV(ADSC);
		chSeqCurrent = ptr;
		return;
	}

	chSeqCurrent = channelSequence;
	ADMUX = _BV(ADLAR) | *chSeqCurrent;

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

	pauseADC();
	sendData(adcTxBuffer, adcTxBufferLength);
	poolSending();
	resumeADC();
}
