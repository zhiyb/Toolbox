#include <inttypes.h>
#include <instructions.h>
#include "stm32f4xx_hal.h"
#include "handles.h"
#include "timer.h"
#include "ctrl.h"
#include "uart.h"
#include "adc.h"
#include "info.h"

static const char * const channelName[] = {"CH_3", "CH_6", "CH_8", "CH_9", "CH_10", \
					   "CH_TEMP", "CH_VBAT", "CH_VREFINT"};
static const uint8_t channels[] = {ADC_CHANNEL_3, ADC_CHANNEL_6, ADC_CHANNEL_8, ADC_CHANNEL_9, ADC_CHANNEL_10,
				   ADC_CHANNEL_TEMPSENSOR, ADC_CHANNEL_VBAT, ADC_CHANNEL_VREFINT};

typedef uint16_t adcData_t;

static uint8_t adcBuffer[ADC_ALIGN_BYTES + ADC_PREPEND_BYTES + ADC_BUFFER_SIZE];
uint8_t *adcTxBuffer;
uint16_t adcTxBufferLength;
volatile uint16_t adcTxBufferRequest;

static uint16_t adcBufferCount;	// Buffered conversions count
static uint16_t channelEnabled = 0xFFFF;
static uint8_t channelCount = CTRL_ADC_CHANNELS, scanMode = 1;

static inline void stopADC(void);
static void configureADC(void);

static inline uint32_t floatToRawUInt32(float x)
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
	adcTxBufferRequest = 0;

	// ADC init
	initTimer(ADC_TIMER);

	resetADC();
	configureADC();
}

void resetADC(void)
{
	stopADC();
}

static inline void pauseADC(void)
{
	HAL_TIM_OC_Stop(ADC_TIMER, ADC_TIMER_CHANNEL);
}

static inline void resumeADC(void)
{
	HAL_TIM_OC_Start(ADC_TIMER, ADC_TIMER_CHANNEL);
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
		while (HAL_ADC_Start_DMA(ADC_HW, (uint32_t *)(adcTxBuffer + ADC_SCAN_PREPEND_BYTES), channelCount) != HAL_OK);
	} else {
		adcBuffer[ADC_ALIGN_BYTES + 0] = CMD_ANALOGDATA;
		adcBuffer[ADC_ALIGN_BYTES + 1] = CTRL_ADC_ID;
		adcBuffer[ADC_ALIGN_BYTES + 2] = CTRL_FRAME;
		*(uint32_t *)(adcBuffer + ADC_ALIGN_BYTES + 3) = 0;
		adcTxBufferLength = ADC_PREPEND_BYTES + adcBufferCount * CTRL_ADC_BYTES;
		adcTxBuffer = adcBuffer + ADC_ALIGN_BYTES;
		while (HAL_ADC_Start_DMA(ADC_HW, (uint32_t *)(adcTxBuffer + ADC_PREPEND_BYTES), adcBufferCount) != HAL_OK);
		__HAL_ADC_ENABLE_IT(ADC_HW, ADC_IT_EOC);
		__HAL_DMA_DISABLE_IT(ADC_HW->DMA_Handle, DMA_IT_TC);
	}
	resumeADC();
}

static void stopADC(void)
{
	pauseADC();
	HAL_ADC_Stop_DMA(ADC_HW);
	// FIXME: Proper delay required here
	volatile unsigned long i = SYS_CLK / 1000;
	while (i--);
	adcTxBufferRequest = 0;
}

static void configureADC(void)
{
	uint16_t mask = 0x01;
	uint8_t i;
	channelCount = 0;
	for (i = 0; i < CTRL_ADC_CHANNELS; i++) {
		if (channelEnabled & mask) {
			ADC_ChannelConfTypeDef sConfig;
			sConfig.Channel = channels[i];
			sConfig.Rank = channelCount + 1;
			sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
			HAL_ADC_ConfigChannel(ADC_HW, &sConfig);
			channelCount++;
		}
		mask <<= 1;
	}
	if (!channelCount)
		return;
	ADC_HW->Init.NbrOfConversion = channelCount;
	HAL_ADC_Init(ADC_HW);
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
		sendValue(floatToRawUInt32(CTRL_ADC_REF), 4);
		sendValue(floatToRawUInt32(CTRL_ADC_OFFSET), 4);
	}
	sendValue(channelEnabled, CTRL_ADC_CHANNELS_BYTES);
	sendValue(ADC_BUFFER_SIZE / CTRL_ADC_BYTES, 4);	// ADC conversion buffer size
	ctrlTimerControllerGenerate(ADC_TIMER);	// ADC trigger timer information
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (scanMode) {
		if (pause)
			adcTxBufferRequest++;
		else
			sendData(adcTxBuffer, adcTxBufferLength);
		return;
	}

	if (pause)
		return;
	// Not ready for send
	if (ADC_HW->DMA_Handle->Instance->NDTR != adcBufferCount)
		return;

	pauseADC();
	sendData(adcTxBuffer, adcTxBufferLength);
	poolSending();
	resumeADC();
}
