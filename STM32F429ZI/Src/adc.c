#include "stm32f4xx_hal.h"
#include "adc.h"
#include "info.h"
#include "ctrl.h"
#include "handles.h"
#include "communication.h"
#include "timer.h"

uint16_t result[CTRL_ADC_CHANNELS] = {0};
uint16_t channelEnabled = 0;
uint8_t channelCount = 0;

void startADC(void)
{
	if (!channelCount)
		return;
	while (HAL_ADC_Start_DMA(ADC_HW, (uint32_t *)result, channelCount) != HAL_OK);
	startTimer(ADC_TIMER);
}

void stopADC(void)
{
	HAL_ADC_Stop_DMA(ADC_HW);
	stopTimer(ADC_TIMER);
}

void configureADC(void)
{
	const static uint32_t channels[CTRL_ADC_CHANNELS] = {
		ADC_CHANNEL_0, ADC_CHANNEL_3, ADC_CHANNEL_6, ADC_CHANNEL_8, ADC_CHANNEL_9,
		ADC_CHANNEL_TEMPSENSOR, ADC_CHANNEL_VBAT, ADC_CHANNEL_VREFINT,
	};
	stopADC();
	HAL_ADC_DeInit(ADC_HW);
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
	initTimer(ADC_TIMER);
	//configureADC();
	stopADC();
	//startADC();
}

void resetADC(void)
{
	stopADC();
}

void ctrlADCController(void)
{
}

void ctrlADCControllerGenerate(void)
{
	const static char* channels[CTRL_ADC_CHANNELS] = {
		"ADC_CHANNEL_0", "ADC_CHANNEL_3", "ADC_CHANNEL_6", "ADC_CHANNEL_8", "ADC_CHANNEL_9",
		"ADC_CHANNEL_TEMPSENSOR", "ADC_CHANNEL_VBAT", "ADC_CHANNEL_VREFINT",
	};
	sendChar(CMD_ANALOGWAVE);		// Analog wave controller generate
	sendChar(CTRL_ADC_ID);			// ID
	sendString("ADC");			// Name
	sendChar(CTRL_ADC_RESOLUTION);		// Result resolution (bits)
	// Analog waveform (ADC) customised controller
	sendChar(CTRL_ADC_CHANNELS);		// Channels
	uint8_t i;
	for (i = 0; i < CTRL_ADC_CHANNELS; i++) {
		sendChar(i);			// Channel ID
		sendString(channels[i]);	// Channel name
	}
	ctrlTimerControllerGenerate(ADC_TIMER);	// ADC trigger timer information
	sendChar(CMD_END);			// End settings
}
