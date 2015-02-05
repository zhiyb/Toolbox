#include <string.h>
#include "stm32f4xx_hal.h"
#include "communication.h"
#include "handles.h"

#define UART_RX_BUFFER_SIZE	64
static uint8_t buffer[UART_RX_BUFFER_SIZE];
static uint32_t position = 0;

void clearOverrun(UART_HandleTypeDef *huart)
{
	if(__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) == SET)
		__HAL_UART_CLEAR_OREFLAG(huart);
}

int receiveChar(uint32_t timeout)
{
	uint32_t tickstart = 0;

	/* Get tick */
	tickstart = HAL_GetTick();

	while (UART_RX_BUFFER_SIZE - HUART->hdmarx->Instance->NDTR == position)
		if (timeout != HAL_MAX_DELAY)
			if (timeout == 0 || (HAL_GetTick() - tickstart) > timeout)
				return -1;

	uint8_t data = buffer[position++];
	if (position == UART_RX_BUFFER_SIZE)
		position = 0;
	return data;
}

uint32_t receiveData(uint8_t *data, uint32_t count, uint32_t timeout)
{
	int c, i;
	for (i = 0; i < count && (c = receiveChar(timeout)) != -1; i++)
		*data++ = c;
	return i;
}

void pollSending(void)
{
	while (HUART->hdmatx->State != HAL_DMA_STATE_READY_MEM0 && HUART->hdmatx->State != HAL_DMA_STATE_READY_MEM1 && HUART->hdmatx->State != HAL_DMA_STATE_READY);
}

void sendChar(char c)
{
	sendData((uint8_t *)&c, 1);
}

void sendValue(uint32_t value, uint8_t bytes)
{
	sendData((uint8_t *)&value, bytes);
}

void sendData(uint8_t *buffer, uint32_t length)
{
	pollSending();
	while (HAL_DMA_Start_IT(HUART->hdmatx, (uint32_t)buffer, (uint32_t)&HUART->Instance->DR, length) != HAL_OK);
}

void sendString(const char *string)
{
	sendData((uint8_t *)string, strlen(string) + 1);
}

void initUART()
{
	HAL_UART_Receive_DMA(HUART, buffer, UART_RX_BUFFER_SIZE);
	HUART->Instance->CR3 |= USART_CR3_DMAT;
	sendString(__DATE__ ", " __TIME__ " | Hello, world!\r\n");
}
