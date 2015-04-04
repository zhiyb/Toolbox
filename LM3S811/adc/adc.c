#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <inc/hw_adc.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>
#include <driverlib/adc.h>
#include <driverlib/sysctl.h>
#include <driverlib/debug.h>
#include <inttypes.h>
#include <instructions.h>
#include "handles.h"
#include "timer0.h"
#include "ctrl.h"
#include "uart.h"
#include "adc.h"

static const char * const channelName[] = {"CH_0", "CH_1", "CH_2", "CH_3", "DCH_0-1", "DCH_2-3", "TEMP"};
static const uint8_t channels[] = {ADC_CTL_CH0, ADC_CTL_CH1, ADC_CTL_CH2, ADC_CTL_CH3, \
				   ADC_CTL_D | ADC_CTL_CH0, ADC_CTL_D | ADC_CTL_CH1, ADC_CTL_TS};

typedef uint16_t adcData_t;

static uint8_t adcBuffer[ADC_ALIGN_BYTES + ADC_PREPEND_BYTES + ADC_BUFFER_SIZE];
uint8_t *adcTxBuffer;
static adcData_t *adcBufferStart, *adcBufferCurrent, *adcBufferEnd;
uint16_t adcTxBufferLength;
volatile uint16_t adcTxBufferRequest;

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

//*****************************************************************************
//
// These defines are used by the ADC driver to simplify access to the ADC
// sequencer's registers.
//
//*****************************************************************************
#define ADC_SEQ                 (ADC_O_SSMUX0)
#define ADC_SEQ_STEP            (ADC_O_SSMUX1 - ADC_O_SSMUX0)
#define ADC_SSMUX               (ADC_O_SSMUX0 - ADC_O_SSMUX0)
#define ADC_SSEMUX              (ADC_O_SSEMUX0 - ADC_O_SSMUX0)
#define ADC_SSCTL               (ADC_O_SSCTL0 - ADC_O_SSMUX0)
#define ADC_SSFIFO              (ADC_O_SSFIFO0 - ADC_O_SSMUX0)
#define ADC_SSFSTAT             (ADC_O_SSFSTAT0 - ADC_O_SSMUX0)
#define ADC_SSOP                (ADC_O_SSOP0 - ADC_O_SSMUX0)
#define ADC_SSDC                (ADC_O_SSDC0 - ADC_O_SSMUX0)

// Store in adcData_t *pulBuffer
long ADCSeqDataGet(unsigned long ulBase, unsigned long ulSequenceNum,
		   adcData_t *pulBuffer)
{
    unsigned long ulCount;

    //
    // Check the arguments.
    //
    ASSERT((ulBase == ADC) || (ulBase == ADC1_BASE));
    ASSERT(ulSequenceNum < 4);

    //
    // Get the offset of the sequence to be read.
    //
    ulBase += ADC_SEQ + (ADC_SEQ_STEP * ulSequenceNum);

    //
    // Read samples from the FIFO until it is empty.
    //
    ulCount = 0;
    while(!(HWREG(ulBase + ADC_SSFSTAT) & ADC_SSFSTAT0_EMPTY) && (ulCount < 8))
    {
	//
	// Read the FIFO and copy it to the destination.
	//
	*pulBuffer++ = HWREG(ulBase + ADC_SSFIFO);

	//
	// Increment the count of samples read.
	//
	ulCount++;
    }

    //
    // Return the number of samples read.
    //
    return(ulCount);
}

unsigned long ADCSeqDataGetNum(unsigned long ulBase, unsigned long ulSequenceNum, unsigned long ulNum, adcData_t *pulBuffer)
{
	unsigned long ulCount = 0;
	ulBase += ADC_SEQ + (ADC_SEQ_STEP * ulSequenceNum);
	while (ulCount != ulNum && !(HWREG(ulBase + ADC_SSFSTAT) & ADC_SSFSTAT0_EMPTY)) {
		*pulBuffer++ = HWREG(ulBase + ADC_SSFIFO);
		ulCount++;
	}
	return ulCount;
}

void initADC(void)
{
	// Initialise data structure
	adcBufferCurrent = (adcData_t *)adcBuffer;

	// ADC init
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC);
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_500KSPS);
	// ulBase, ulSequenceNum, ulTrigger, ulPriority
	ADCSequenceConfigure(ADC, 0, ADC_TRIGGER_TIMER, 0);
	configureADC();
	// ulBase, ulSequenceNum, ulStep, ulConfig
	ADCHardwareOversampleConfigure(ADC, 0);
	ADCSequenceEnable(ADC, 0);
	ADCIntEnable(ADC, 0);
	IntEnable(INT_ADC0SS0);

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
		adcBufferCurrent = adcBufferStart = (adcData_t *)(adcTxBuffer + ADC_SCAN_PREPEND_BYTES);
		adcBufferEnd = adcBufferStart + channelCount;
	} else {
		adcBuffer[ADC_ALIGN_BYTES + 0] = CMD_ANALOGDATA;
		adcBuffer[ADC_ALIGN_BYTES + 1] = CTRL_ADC_ID;
		adcBuffer[ADC_ALIGN_BYTES + 2] = CTRL_FRAME;
		*(uint32_t *)(adcBuffer + ADC_ALIGN_BYTES + 3) = 0;
		adcTxBufferLength = ADC_PREPEND_BYTES + adcBufferCount * CTRL_ADC_BYTES;
		adcTxBuffer = adcBuffer + ADC_ALIGN_BYTES;
		adcBufferCurrent = adcBufferStart = (adcData_t *)(adcTxBuffer + ADC_PREPEND_BYTES);
		adcBufferEnd = adcBufferStart + adcBufferCount;
	}
	adcData_t adc[8];
	while (ADCSeqDataGet(ADC, 0, adc));
	resumeADC();
}

static inline void stopADC(void)
{
	pauseADC();
	// FIXME: Proper delay required here
	volatile unsigned long i = SYS_CLK / 1000;
	while (i--);
	adcTxBufferRequest = 0;
}

static void configureADC(void)
{
	uint8_t mask = 0x01, i, last = 0;
	channelCount = 0;
	ADCSequenceDisable(ADC, 0);
	for (i = 0; i != CTRL_ADC_CHANNELS; i++) {
		if (channelEnabled & mask)
			ADCSequenceStepConfigure(ADC, 0, channelCount++, channels[(last = i)]);
		mask <<= 1;
	}
	if (!channelCount)
		return;
	ADCSequenceStepConfigure(ADC, 0, channelCount - 1, channels[last] | ADC_CTL_IE | ADC_CTL_END);
	ADCSequenceEnable(ADC, 0);
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
	sendChar(CMD_ANALOG);			// Analog waveform (ADC) customised controller
	sendChar(CTRL_ADC_ID);			// ID
	sendString("ADC");			// Name
	sendChar(CTRL_ADC_RESOLUTION);		// Result resolution (bits)
	sendValue(CTRL_ADC_SCAN_FREQUENCY, 4);	// Scan mode maximum transfer frequency
	sendValue(CTRL_ADC_MAX_FREQUENCY, 4);	// Maximum sampling frequency
	sendChar(CTRL_ADC_CHANNELS);		// Channels
	uint8_t i;
	for (i = 0; i < CTRL_ADC_CHANNELS; i++) {
		sendChar(i);			// Channel ID
		sendString(channelName[i]);
		if (i == CTRL_ADC_CHANNELS - 1) {	// Temperature sensor
			sendValue(floatToRawUInt32(-225.f), 4);
			sendValue(floatToRawUInt32(147.5), 4);
		} else {
			sendValue(floatToRawUInt32(CTRL_ADC_REF), 4);
			sendValue(floatToRawUInt32(CTRL_ADC_OFFSET), 4);
		}
	}
	sendValue(channelEnabled, CTRL_ADC_CHANNELS_BYTES);
	sendValue(ADC_BUFFER_SIZE / CTRL_ADC_BYTES, 4);	// ADC conversion buffer size
	ctrlTimer0ControllerGenerate();	// ADC trigger timer information
}

void ADCIntHandler(void)
{
	ADCIntClear(ADC, 0);
	if (ADCSeqDataGetNum(ADC, 0, channelCount, adcBufferCurrent) != channelCount)
		return;

	adcBufferCurrent = adcBufferCurrent + channelCount == adcBufferEnd ? adcBufferStart : adcBufferCurrent + channelCount;

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
	adcBufferCurrent = adcBufferStart;
	resumeADC();
}
