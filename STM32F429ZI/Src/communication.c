#include <string.h>
#include "stm32f4xx_hal.h"
#include "communication.h"
#include "handles.h"

static HAL_StatusTypeDef UART_WaitOnFlagUntilTimeout(UART_HandleTypeDef *huart, uint32_t Flag, FlagStatus Status, uint32_t Timeout);
void clearOverrun(UART_HandleTypeDef *huart);

static HAL_StatusTypeDef UART_WaitOnFlagUntilTimeout(UART_HandleTypeDef *huart, uint32_t Flag, FlagStatus Status, uint32_t Timeout)
{
	uint32_t tickstart = 0;

	/* Get tick */
	tickstart = HAL_GetTick();

	/* Wait until flag is set */
	if(Status == RESET)
	{
		while(__HAL_UART_GET_FLAG(huart, Flag) == RESET)
		{
			/* Check for the Timeout */
			if(Timeout != HAL_MAX_DELAY)
			{
				if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
				{
					/* Disable TXE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts for the interrupt process */
					__HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
					__HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
					__HAL_UART_DISABLE_IT(huart, UART_IT_PE);
					__HAL_UART_DISABLE_IT(huart, UART_IT_ERR);

					huart->State= HAL_UART_STATE_READY;

					/* Process Unlocked */
					__HAL_UNLOCK(huart);

					return HAL_TIMEOUT;
				}
			}
		}
	}
	else
	{
		while(__HAL_UART_GET_FLAG(huart, Flag) != RESET)
		{
			/* Check for the Timeout */
			if(Timeout != HAL_MAX_DELAY)
			{
				if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
				{
					/* Disable TXE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts for the interrupt process */
					__HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
					__HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
					__HAL_UART_DISABLE_IT(huart, UART_IT_PE);
					__HAL_UART_DISABLE_IT(huart, UART_IT_ERR);

					huart->State= HAL_UART_STATE_READY;

					/* Process Unlocked */
					__HAL_UNLOCK(huart);

					return HAL_TIMEOUT;
				}
			}
		}
	}
	return HAL_OK;
}

/*void resync(void)
{
	while (HAL_DMA_Abort(HUART.hdmarx) != HAL_OK);
	do
		sendChar(PKG_ACK);
	while (receiveChar(-1) != PKG_ACK);
	//uartRecv = 0;
	//HUART.hdmarx->Instance->CR &= DMA_SxCR_CT;
	while (HAL_DMAEx_MultiBufferStart(HUART.hdmarx, (uint32_t)&HUART.Instance->DR, (uint32_t)uartBuffer[0], (uint32_t)uartBuffer[1], PKG_SIZE) != HAL_OK);
}*/

void clearOverrun(UART_HandleTypeDef *huart)
{
	if(__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) == SET)
		__HAL_UART_CLEAR_OREFLAG(huart);
}

int receiveChar(uint32_t timeout)
{
	clearOverrun(&HUART);
	if(UART_WaitOnFlagUntilTimeout(&HUART, UART_FLAG_RXNE, RESET, timeout) != HAL_OK)
		return -1;
	if(HUART.Init.Parity == UART_PARITY_NONE)
		return (char)(HUART.Instance->DR & (uint8_t)0x00FF);
	else
		return (char)(HUART.Instance->DR & (uint8_t)0x007F);
}

uint32_t receiveData(uint8_t *data, uint32_t count, uint32_t timeout)
{
	int c, i;
	for (i = 0; i < count && (c = receiveChar(timeout)) != -1; i++)
		*data++ = c;
	return i;
}

/*uint8_t *receive(void)
{
	uint8_t *buffer = 0;
ignore:
	if (uartRecv == 0) {
		while ((HUART.hdmarx->Instance->CR & DMA_SxCR_CT) == 0);
		buffer = uartBuffer[0];
	} else {
		while ((HUART.hdmarx->Instance->CR & DMA_SxCR_CT) != 0);
		buffer = uartBuffer[1];
	}
	uartRecv = !uartRecv;
	if (*(uint16_t *)&buffer[PKG_SIZE - 2] == PKG_SYNC)
		return buffer;
	char buff[32];
	sprintf(buff, "0x%02X, 0x%02X\r\n", buffer[PKG_SIZE - 2], buffer[PKG_SIZE - 1]);
	sendString(buff);
	pollSending();
	if (buffer[PKG_SIZE - 1] == CMD_RESYNC)
		resync();
	goto ignore;
}*/

void pollSending(void)
{
	while (HUART.hdmatx->State != HAL_DMA_STATE_READY_MEM0 && HUART.hdmatx->State != HAL_DMA_STATE_READY_MEM1 && HUART.hdmatx->State != HAL_DMA_STATE_READY);
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
	while (HAL_DMA_Start_IT(HUART.hdmatx, (uint32_t)buffer, (uint32_t)&HUART.Instance->DR, length) != HAL_OK);
}

/*void send(uint8_t *buffer)
{
	sendData(buffer, PKG_SIZE);
}*/

void sendString(const char *string)
{
	sendData((uint8_t *)string, strlen(string) + 1);
}
