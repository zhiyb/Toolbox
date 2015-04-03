#ifndef UART_H
#define UART_H

#include <inttypes.h>

void initUART(void);

int receiveChar(void);
uint16_t receiveData(uint8_t *data, uint16_t count);

void poolSending(void);
void sendChar(char c);
void sendData(uint8_t *buffer, uint16_t length);
void sendValue(uint32_t value, uint8_t bytes);
void sendString(const char *string);
void sendString_P(const char *string);

#endif
