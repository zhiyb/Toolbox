#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <inttypes.h>

void pollSending(void);
void sendData(uint8_t *buffer, uint32_t length);
//void send(uint8_t *buffer);
void sendString(const char *string);
//void resync(void);
//uint8_t *receive(void);
int receiveChar(uint32_t timeout);
uint32_t receiveData(uint8_t *data, uint32_t count, uint32_t timeout);
void sendChar(char c);
void sendValue(uint32_t value, uint8_t bytes);

#endif // COMMUNICATION_H
