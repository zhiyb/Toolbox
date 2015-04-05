#ifndef UART_H
#define UART_H

#define UART_RX_BUFFER_SIZE	64

#include <inttypes.h>

void initUART(void);

char receiveChar(void);
void receiveData(uint8_t *data, uint32_t count);

void poolSending(void);
void sendChar(char c);
void sendData(uint8_t *buffer, uint32_t length);
void sendValue(uint32_t value, uint8_t bytes);
void sendString(const char *string);

#endif
