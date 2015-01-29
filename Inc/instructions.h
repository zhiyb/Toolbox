#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

//#define PKG_SIZE	32
//#define PKG_SYNC	(uint16_t)0xAA55
#define FW_VERSION	1
#define INVALID_ID	((uint8_t)-1)

#define CMD_NOP		'\x00'
#define CMD_RESET	'R'
#define CMD_PAUSE	'P'
#define CMD_CONTROLLER	'C'
#define CMD_TIMER	'T'
#define CMD_ANALOGWAVE	'W'
#define CMD_DIGITALWAVE	'w'
#define CMD_INFO	'I'
#define CMD_ACK		'A'
#define CMD_END		-1

#define CTRL_TOGGLE	0
#define CTRL_BYTE1	1
#define CTRL_BYTE2	2
#define CTRL_BYTE3	3
#define CTRL_BYTE4	4
#define CTRL_BUTTON	5
#define CTRL_RADIO	6
#define CTRL_CHECKBOX	7
#define CTRL_SELECTOR	8
#define CTRL_SELECTED	9
#define CTRL_READONLY	0x80

#endif
