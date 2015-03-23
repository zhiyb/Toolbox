# Modified by zhiyb (Yubo Zhi)

PRG	= ATMEGA644P
SRC	= main.c uart.c ctrl.c
SUBDIRS	= adc
OPTDIR	= optional
LIB	= 
LIBDIR	= 
VERSION	= 0.0.1

PRGER	= usbasp
MCU_TARGET	= atmega644p
OPTIMIZE	= -Os

# Enable 8 bits, 4 channel DAC chip TLV5620
ENABLE_DAC	= no
# Enable PWM, using timer 1 with PORTD, conflict with DAC(TLV5620)
ifneq ($(ENABLE_DAC), yes)
ENABLE_PWM	= yes
endif

ifeq ($(ENABLE_DAC), yes)
SUBDIRS	+= $(OPTDIR)/dac
DEFS	+= -DENABLE_DAC
endif

ifeq ($(ENABLE_PWM), yes)
SUBDIRS	+= $(OPTDIR)/pwm
DEFS	+= -DENABLE_PWM
endif

DEFS	+= -DF_CPU=12000000
LIBS	+= 
INCS	+= -I. -I../Inc

EFUSE	= 0xFF
HFUSE	= 0x9C
LFUSE	= 0xE7

LOG	= ~/.make_avr.log

include $(SUBDIRS:%=%/Makefile)
include Makefile.inc
