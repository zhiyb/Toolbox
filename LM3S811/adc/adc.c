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
volatile uint8_t adcIgnore = 0;

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

long ADCSeqDataGetNum(unsigned long ulBase, unsigned long ulSequenceNum, unsigned long ulNum, adcData_t *pulBuffer)
{
	unsigned long ulCount = 0;
	ulBase += ADC_SEQ + (ADC_SEQ_STEP * ulSequenceNum);
	while (!(HWREG(ulBase + ADC_SSFSTAT) & ADC_SSFSTAT0_EMPTY) && ulCount != ulNum) {
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
	//ADCSequenceEnable(ADC0_BASE, 0);
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
	ADCIntClear(ADC, 0);
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
	//pauseADC();
	stopADC();

	// Device specific
	//adcData_t adc[CTRL_ADC_CHANNELS];
	//ADCSeqDataGet(ADC, 0, adc);
	adcIgnore = 2;
	ADCSequenceEnable(ADC, 0);

	resumeADC();
}

static inline void stopADC(void)
{
	pauseADC();
	adcData_t adc[CTRL_ADC_CHANNELS];
	ADCSequenceDisable(ADC, 0);
	ADCSeqDataGet(ADC, 0, adc);
	ADCIntClear(ADC, 0);
	adcTxBufferRequest = 0;
}

static void configureADC(void)
{
	uint8_t mask = 0x01, i, last = 0;
	channelCount = 0;
	for (i = 0; i != CTRL_ADC_CHANNELS; i++) {
		if (channelEnabled & mask)
			ADCSequenceStepConfigure(ADC, 0, channelCount++, channels[(last = i)]);
		mask <<= 1;
	}
	if (!channelCount)
		return;
	ADCSequenceStepConfigure(ADC, 0, channelCount - 1, channels[last] | ADC_CTL_IE | ADC_CTL_END);
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
#if 0
	if (adcIgnore) {
		// Unexpected data exist
		adcData_t adc[CTRL_ADC_CHANNELS];
		ADCSeqDataGet(ADC, 0, adc);
		adcIgnore--;
		return;
	}
#endif

	unsigned long cnt;
	cnt = ADCSeqDataGetNum(ADC, 0, channelCount, adcBufferCurrent);
	//cnt = ADCSeqDataGet(ADC0_BASE, 0, adcBufferCurrent);
	ADCIntClear(ADC, 0);

	// Incorrect data length
	if (cnt != channelCount)
		return;

	adcBufferCurrent = adcBufferCurrent + cnt == adcBufferEnd ? adcBufferStart : adcBufferCurrent + cnt;

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





// Original ******************************************
#if 0
static uint32_t channelEnabled = 0xFFFFFFFF, channelCount = LIM_ADC_CHANNELS;

static struct buffer_t {
	uint8_t enabled;
	unsigned long current, length;
	uint16_t buffer[LIM_BUFFER_SIZE / 2];
} buffer;

static struct trigger_t {
	enum TriggerType {RisingEdge = 0, FallingEdge = 1} type;
	enum TriggerStatuses {New, Before, Waiting, After} status;
	uint8_t enabled, channel;
	uint16_t level, previousData;
	unsigned long preLength, postLength, count;
} trigger;

// *************************** ADC ***************************

void adcSequenceUpdate(void);
void timerDisable(void);
void timerEnable(void);
void timerSet(const unsigned long load);

void adcLimitsPut(void)
{
	UARTCharPut(UART0_BASE, LIM_ADC_CHANNELS);
	UARTCharPut(UART0_BASE, LIM_ADC_BITS);
	UARTLongPut(UART0_BASE, LIM_ADC_MAXSPEED);
	UARTLongPut(UART0_BASE, LIM_ADC_RTSPEED);
	UARTCharPut(UART0_BASE, LIM_ADC_REFERENCE_I);
	UARTCharPut(UART0_BASE, LIM_ADC_REFERENCE_F);
}

void adcConfigure(void)
{
	switch (UARTCharGet(UART0_BASE)) {
	case COM_LIMITS:
		adcLimitsPut();
		UARTCharPut(UART0_BASE, COM_ACK);
		break;
	case COM_SET:
		channelEnabled = UARTLongGet(UART0_BASE);
		buffer.enabled = UARTCharGet(UART0_BASE);
		adcSequenceUpdate();
		UARTCharPut(UART0_BASE, COM_ACK);
		break;
	case COM_START:
		UARTCharPut(UART0_BASE, COM_ACK);
		ADCIntClear(ADC0_BASE, 0);
		ADCSequenceEnable(ADC0_BASE, 0);
		//ADCIntEnable(ADC0_BASE, 0);
		break;
	case COM_STOP:
		//ADCIntDisable(ADC0_BASE, 0);
		ADCSequenceDisable(ADC0_BASE, 0);
		UARTCharPut(UART0_BASE, COM_ACK);
		break;
	}
}

static unsigned long resolveSequence(const unsigned long channel)
{
	unsigned long mask = 0x00000001, ch = 0, seq = 0;
	for (ch = 0; ch != channel && ch != LIM_ADC_CHANNELS; ch++) {
		if (channelEnabled & mask)
			seq++;
		mask <<= 1;
	}
	return seq;
}

// *************************** Trigger ***************************

static inline enum TriggerStatuses triggerHandle(const unsigned long data)
{
	switch (trigger.status) {
	case New:
		trigger.status = trigger.preLength <= 1 ? Waiting : Before;
		trigger.count = 1;
		break;
	case Before:
		if (++trigger.count >= trigger.preLength) {
			trigger.status = Waiting;
			trigger.count = 1;
		}
		break;
	case Waiting:
		if ((trigger.type == RisingEdge && trigger.previousData <= trigger.level && data >= trigger.level && trigger.previousData < data) || \
		    (trigger.type == FallingEdge && trigger.previousData >= trigger.level && data <= trigger.level && trigger.previousData > data))
			trigger.status = trigger.postLength <= 1 ? New : After;
		break;
	case After:
		if (++trigger.count >= trigger.postLength)
			trigger.status = New;
	}
	trigger.previousData = data;
	return trigger.status;
}

void adcTriggerInit(void)
{
	adcTriggerReset();
}

void adcTriggerReset(void)
{
	trigger.status = New;
}

void adcTriggerConfigure(void)
{
	switch (UARTCharGet(UART0_BASE)) {
	case COM_SET:
		trigger.channel = UARTCharGet(UART0_BASE);
		trigger.type = UARTCharGet(UART0_BASE);
		trigger.level = UARTLongGet(UART0_BASE);
		trigger.preLength = UARTLongGet(UART0_BASE);
		trigger.postLength = UARTLongGet(UART0_BASE);
		trigger.channel = resolveSequence(trigger.channel);
		UARTCharPut(UART0_BASE, COM_ACK);
		break;
	case COM_START:
		UARTCharPut(UART0_BASE, COM_ACK);
		trigger.enabled = 1;
		break;
	case COM_STOP:
		trigger.enabled = 0;
		UARTCharPut(UART0_BASE, COM_ACK);
		break;
	}
}

// *************************** Buffer ***************************

void adcBufferInit(void)
{
	buffer.enabled = 0;
	buffer.length = 0;
	adcBufferReset();
	//bufferInit();
}

void adcBufferReset(void)
{
	buffer.current = 0;
}

void adcBufferConfigure(void)
{
	switch (UARTCharGet(UART0_BASE)) {
	case COM_LIMITS:
		UARTLongPut(UART0_BASE, LIM_BUFFER_SIZE);
		UARTCharPut(UART0_BASE, COM_ACK);
		break;
	case COM_SET:
		adcBufferReset();
		//bufferSetLength(UARTLongGet(UART0_BASE));
		buffer.length = UARTLongGet(UART0_BASE) / 2;
		UARTCharPut(UART0_BASE, COM_ACK);
		break;
	}
}

// *************************** Interrupt ***************************

/*#define	ADC_SEQ			(ADC_O_SSMUX0)
#define	ADC_SEQ_STEP		(ADC_O_SSMUX1 -	ADC_O_SSMUX0)
#define	ADC_SSMUX		(ADC_O_SSMUX0 -	ADC_O_SSMUX0)
#define	ADC_SSEMUX		(ADC_O_SSEMUX0 - ADC_O_SSMUX0)
#define	ADC_SSCTL		(ADC_O_SSCTL0 -	ADC_O_SSMUX0)
#define	ADC_SSFIFO		(ADC_O_SSFIFO0 - ADC_O_SSMUX0)
#define	ADC_SSFSTAT		(ADC_O_SSFSTAT0	- ADC_O_SSMUX0)
#define	ADC_SSOP		(ADC_O_SSOP0 - ADC_O_SSMUX0)
#define	ADC_SSDC		(ADC_O_SSDC0 - ADC_O_SSMUX0)
long ADCSequenceDataGetNum(unsigned long ulBase, unsigned long ulSequenceNum, unsigned long ulNum, unsigned long *pulBuffer)
{
	unsigned long ulCount = 0;
	ulBase += ADC_SEQ + (ADC_SEQ_STEP * ulSequenceNum);
	while (!(HWREG(ulBase + ADC_SSFSTAT) & ADC_SSFSTAT0_EMPTY) && (ulCount < 8)) {
		*pulBuffer++ = HWREG(ulBase + ADC_SSFIFO);
		ulCount++;
	}
	return ulCount;
}*/

void ADCIntHandler(void)
{
	unsigned long adc[8], cnt, i;
	//cnt = ADCSequenceDataGetNum(ADC0_BASE, 0, channelCount, adc);
	cnt = ADCSequenceDataGet(ADC0_BASE, 0, adc);
	ADCIntClear(ADC0_BASE, 0);

//	if (cnt != channelCount)
//		return;

	if (!buffer.enabled) {
		UARTCharPut(UART0_BASE, COM_ADC_DATA);
		for (i = 0; i != cnt; i++)
			UARTShortPut(UART0_BASE, adc[i]);
		return;
	}

	for (i = 0; i != cnt; i++) {
		buffer.buffer[buffer.current] = adc[i];
		if (++buffer.current == buffer.length)
			buffer.current = 0;
	}

	if (trigger.enabled) {
		if (triggerHandle(adc[trigger.channel]) != New)
			return;
	} else if (buffer.current != 0)
			return;

	// Output
	ADCSequenceDisable(ADC0_BASE, 0);
	timerDisable();
	UARTCharPut(UART0_BASE, COM_ADC_FRAME);
	for (i = 0; i != buffer.length; i++) {
		//UARTShortPut(UART0_BASE, *(short *)bufferRead(sizeof(short)));
		UARTShortPut(UART0_BASE, buffer.buffer[buffer.current]);
		if (++buffer.current == buffer.length)
			buffer.current = 0;
	//} while (bufferPosition() != 0);
	}
	buffer.current = 0;
	ADCSequenceDataGet(ADC0_BASE, 0, adc);
	ADCIntClear(ADC0_BASE, 0);
	ADCSequenceEnable(ADC0_BASE, 0);
	timerEnable();
}
#endif
