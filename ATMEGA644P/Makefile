# Modified by zhiyb (Yubo Zhi)

PRG	= ATMEGA644P
SRC	= main.c uart.c ctrl.c adc.c timer0.c
LIB	= 
LIBDIR	= 
VERSION	= 0.0.1

PRGER	= usbasp
MCU_TARGET	= atmega644p
OPTIMIZE	= -Os

DEFS	+= -DF_CPU=12000000
LIBS	+= 
INCS	+= -I../Inc

EFUSE	= 0xFF
HFUSE	= 0x9C
LFUSE	= 0xE7

LOG	= ~/.make_avr.log

include Makefile.inc
