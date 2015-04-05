#include <instructions.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "uart.h"
#include "info.h"
#include "handles.h"

static uint8_t buffer[UART_RX_BUFFER_SIZE];
static uint8_t position = 0;

// Device specific
void initUART(void)
{
	HUART->Init.BaudRate = BAUD;
	HAL_UART_Init(HUART);
	HAL_UART_Receive_DMA(HUART, buffer, UART_RX_BUFFER_SIZE);
	HUART->Instance->CR3 |= USART_CR3_DMAT;
	sendString(DEVICE_NAME "\r\n");
	sendString(__DATE__ ", " __TIME__ " | Hello, world!\r\n");
	poolSending();
}

#if 0
static inline void clearOverrun(UART_HandleTypeDef *huart)
{
	if(__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) == SET)
		__HAL_UART_CLEAR_OREFLAG(huart);
}
#endif

void poolSending(void)
{
	//while (!(HUART->Instance->SR & UART_FLAG_TC));
	//HUART->Instance->SR &= ~UART_FLAG_TC;
	//while (HUART->hdmatx->State != HAL_DMA_STATE_READY_MEM0 && HUART->hdmatx->State != HAL_DMA_STATE_READY_MEM1 && HUART->hdmatx->State != HAL_DMA_STATE_READY);
}

char receiveChar(void)
{
	while (UART_RX_BUFFER_SIZE - HUART->hdmarx->Instance->NDTR == position);
	while (HUART->Instance->SR & UART_FLAG_RXNE);

	char data = buffer[position];
	position = position + 1 == UART_RX_BUFFER_SIZE ? 0 : position + 1;
	return data;
}

void sendData(uint8_t *buffer, uint32_t length)
{
	//poolSending();
	while (length--)
		sendChar(*buffer++);
	//while (HAL_DMA_Start_IT(HUART->hdmatx, (uint32_t)buffer, (uint32_t)&HUART->Instance->DR, length) != HAL_OK);
}

void sendChar(char c)
{
	//sendData((uint8_t *)&c, 1);
	poolSending();
	HUART->Instance->SR;
	HUART->Instance->DR = c;
	while (!(HUART->Instance->SR & UART_FLAG_TC));
}

// General
void receiveData(uint8_t *data, uint32_t count)
{
	while (count--)
		*data++ = receiveChar();
}

void sendValue(uint32_t value, uint8_t bytes)
{
	sendData((uint8_t *)&value, bytes);
}

void sendString(const char *string)
{
	sendData((uint8_t *)string, strlen(string) + 1);
}
