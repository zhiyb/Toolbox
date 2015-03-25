#include "stm32f4xx_hal.h"
#include "adc.h"
#include "info.h"
#include "ctrl.h"
#include "handles.h"
#include "communication.h"
#include "timer.h"

uint8_t adcBuffer[ADC_ALIGN_BYTES + ADC_PREPEND_BYTES + ADC_BUFFER_SIZE] = {0, CMD_ANALOGDATA, CTRL_ADC_ID, 0};
uint8_t *adcTxBuffer;
volatile uint32_t adcTxBufferLength;
volatile uint8_t adcTxBufferRequest = 0;

static uint32_t bufferCount;	// Buffered conversions count
static uint16_t channelEnabled = 0xFFFF;
static uint8_t channelCount = CTRL_ADC_CHANNELS, scanMode = 1;

uint32_t floatToRawUInt32(float x)
{
	union {
		float f;	// assuming 32-bit IEEE 754 single-precision
		uint32_t i;	// assuming 32-bit unsigned int
	} u;
	u.f = x;
	return u.i;
}

static void startADC(void)
{
	if (!channelCount)
		return;
	if (scanMode) {
		adcBuffer[ADC_SCAN_ALIGN_BYTES + 0] = CMD_ANALOGDATA;
		adcBuffer[ADC_SCAN_ALIGN_BYTES + 1] = CTRL_ADC_ID;
		adcBuffer[ADC_SCAN_ALIGN_BYTES + 2] = CTRL_DATA;
		adcTxBufferLength = ADC_SCAN_PREPEND_BYTES + channelCount * CTRL_ADC_BYTES;
		adcTxBuffer = adcBuffer + ADC_SCAN_ALIGN_BYTES;
		while (HAL_ADC_Start_DMA(ADC_HW, (uint32_t *)(adcBuffer + ADC_SCAN_ALIGN_BYTES + ADC_SCAN_PREPEND_BYTES), channelCount) != HAL_OK);
	} else {
		adcBuffer[ADC_ALIGN_BYTES + 0] = CMD_ANALOGDATA;
		adcBuffer[ADC_ALIGN_BYTES + 1] = CTRL_ADC_ID;
		adcBuffer[ADC_ALIGN_BYTES + 2] = CTRL_FRAME;
		*(uint32_t *)(adcBuffer + ADC_ALIGN_BYTES + 3) = 0;
		adcTxBufferLength = ADC_PREPEND_BYTES + bufferCount * CTRL_ADC_BYTES;
		adcTxBuffer = adcBuffer + ADC_ALIGN_BYTES;
		while (HAL_ADC_Start_DMA(ADC_HW, (uint32_t *)(adcBuffer + ADC_ALIGN_BYTES + ADC_PREPEND_BYTES), bufferCount) != HAL_OK);
		__HAL_ADC_ENABLE_IT(ADC_HW, ADC_IT_EOC);
		__HAL_DMA_DISABLE_IT(ADC_HW->DMA_Handle, DMA_IT_TC);
	}
	//startTimer(ADC_TIMER);
	HAL_TIM_OC_Start(ADC_TIMER, ADC_TIMER_CHANNEL);
}

static void stopADC(void)
{
	HAL_ADC_Stop_DMA(ADC_HW);
	//stopTimer(ADC_TIMER);
	HAL_TIM_OC_Stop(ADC_TIMER, ADC_TIMER_CHANNEL);
	adcTxBufferRequest = 0;
}

static void configureADC(void)
{
	const static uint32_t channels[CTRL_ADC_CHANNELS] = {
		ADC_CHANNEL_3, ADC_CHANNEL_6, ADC_CHANNEL_8, ADC_CHANNEL_9, ADC_CHANNEL_10,
		ADC_CHANNEL_TEMPSENSOR, ADC_CHANNEL_VBAT, ADC_CHANNEL_VREFINT,
	};
	//stopADC();
	//HAL_ADC_DeInit(ADC_HW);
	channelCount = 0;
	uint16_t mask = 0x01;
	uint8_t i;
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
	//startADC();
}

void initADC(void)
{
	stopADC();
	initTimer(ADC_TIMER);
	configureADC();
	//startADC();
}

void resetADC(void)
{
	stopADC();
}

void ctrlADCController(const uint8_t id)
{
loop:
	switch (receiveChar(-1)) {
	case CTRL_START:
		if (receiveChar(-1))
			startADC();
		else
			stopADC();
		break;
	case CTRL_SET:
		receiveData((uint8_t *)&channelEnabled, CTRL_ADC_CHANNELS_BYTES, -1);
		configureADC();
		break;
	case CTRL_DATA:
		scanMode = receiveChar(-1) == CTRL_DATA;
		stopADC();
		break;
	case CTRL_FRAME:
		receiveData((uint8_t *)&bufferCount, 4, -1);
		bufferCount *= channelCount;
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
		"CH_3", "CH_6", "CH_8", "CH_9", "CH_10",
		"CH_TEMPSENSOR", "CH_VBAT", "CH_VREFINT",
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
	ctrlTimerControllerGenerate(ADC_TIMER);	// ADC trigger timer information
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (scanMode) {
		if (pause)
			adcTxBufferRequest++;
		else
			sendData(adcTxBuffer, adcTxBufferLength);
	} else {
		// Frame mode, can just skip data frames
		if (pause)
			return;
		// Not ready for send
		if (ADC_HW->DMA_Handle->Instance->NDTR != bufferCount)
			return;
		HAL_TIM_OC_Stop(ADC_TIMER, ADC_TIMER_CHANNEL);
		sendData(adcTxBuffer, adcTxBufferLength);
		pollSending();
		HAL_TIM_OC_Start(ADC_TIMER, ADC_TIMER_CHANNEL);
	}
}
