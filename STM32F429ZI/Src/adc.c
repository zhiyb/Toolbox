#include "stm32f4xx_hal.h"
#include "adc.h"
#include "info.h"
#include "ctrl.h"
#include "handles.h"
#include "communication.h"
#include "timer.h"

uint8_t adcTxBuffer[CTRL_ADC_CHANNELS * CTRL_ADC_BYTES + 3] = {CMD_ANALOGDATA, CTRL_ADC_ID, 0};
volatile uint8_t adcTxBufferRequest = 0, adcTxBufferLength = 0;

uint16_t result[CTRL_ADC_CHANNELS] = {0};
uint16_t channelEnabled = 0xFFFF;
uint8_t channelCount = CTRL_ADC_CHANNELS;

void startADC(void)
{
	if (!channelCount)
		return;
	while (HAL_ADC_Start_DMA(ADC_HW, (uint32_t *)result, channelCount) != HAL_OK);
	//startTimer(ADC_TIMER);
	HAL_TIM_OC_Start(ADC_TIMER, ADC_TIMER_CHANNEL);
}

void stopADC(void)
{
	HAL_ADC_Stop_DMA(ADC_HW);
	//stopTimer(ADC_TIMER);
	HAL_TIM_OC_Stop(ADC_TIMER, ADC_TIMER_CHANNEL);
	adcTxBufferRequest = 0;
}

void configureADC(void)
{
	const static uint32_t channels[CTRL_ADC_CHANNELS] = {
		ADC_CHANNEL_3, ADC_CHANNEL_6, ADC_CHANNEL_8, ADC_CHANNEL_9, ADC_CHANNEL_10,
		ADC_CHANNEL_TEMPSENSOR, ADC_CHANNEL_VBAT, ADC_CHANNEL_VREFINT,
	};
	stopADC();
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
	default:
		return;
	}
	goto loop;
}

void ctrlADCControllerGenerate(void)
{
	const static char* channels[CTRL_ADC_CHANNELS] = {
		"CHANNEL_3", "CHANNEL_6", "CHANNEL_8", "CHANNEL_9", "CHANNEL_10",
		"CHANNEL_TEMPSENSOR", "CHANNEL_VBAT", "CHANNEL_VREFINT",
	};
	sendChar(CMD_ANALOG);		// Analog waveform (ADC) customised controller
	sendChar(CTRL_ADC_ID);			// ID
	sendString("ADC");			// Name
	sendChar(CTRL_ADC_RESOLUTION);		// Result resolution (bits)
	sendValue(CTRL_ADC_SCAN_FREQUENCY, 4);	// Scan mode maximum transfer frequency
	sendChar(CTRL_ADC_CHANNELS);		// Channels
	uint8_t i;
	for (i = 0; i < CTRL_ADC_CHANNELS; i++) {
		sendChar(i);			// Channel ID
		sendString(channels[i]);	// Channel name
	}
	ctrlTimerControllerGenerate(ADC_TIMER);	// ADC trigger timer information
	sendChar(CMD_END);			// End settings
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	uint8_t i;
	adcTxBuffer[2] = CTRL_DATA;
	adcTxBufferLength = 3 + channelCount * 2;
	for (i = 0; i < channelCount; i++)
		*(uint16_t *)&adcTxBuffer[3 + i * 2] = result[i];
	if (!pause)
		sendData(adcTxBuffer, adcTxBufferLength);
	else
		adcTxBufferRequest++;
}
