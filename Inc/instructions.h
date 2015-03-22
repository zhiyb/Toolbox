#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

//#define PKG_SIZE	32
//#define PKG_SYNC	(uint16_t)0xAA55
#define FW_VERSION	0
#define INVALID_ID	__UINT8_MAX__
#define BAUD		1500000

#define CMD_NOP			'\x00'
#define CMD_RESET		'R'
//#define CMD_PAUSE		'P'
#define CMD_CONTROLLER		'C'
#define CMD_CONTROLLERDATA	'c'
#define CMD_TIMER		'T'
#define CMD_ANALOG		'W'
#define CMD_ANALOGDATA		'D'
#define CMD_DIGITAL		'w'
#define CMD_DIGITALDATA		'd'
#define CMD_INFO		'I'
#define CMD_ACK			'A'
#define CMD_END			__UINT8_MAX__

#define CTRL_START	0	// 0: stop, 1: start
#define CTRL_SET	1	// Set value to component
#define CTRL_DATA	2	// Data / Set data format
#define CTRL_FRAME	3	// Data frame / Set frame length

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
#define CTRL_NEW_COLUMN	10
#define CTRL_READONLY	0x80

#endif
