#ifndef UART_H
#define UART_H

#define UART0_TX	_BV(1)
#define UART0_RX	_BV(0)

#include <inttypes.h>

void initUART(void);

int receiveChar(void);
uint32_t receiveData(uint8_t *data, uint32_t count);

void pollSending(void);
void sendChar(char c);
void sendData(uint8_t *buffer, uint32_t length);
void sendValue(uint32_t value, uint8_t bytes);
void sendString(const char *string);

#endif
