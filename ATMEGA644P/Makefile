# Modified by zhiyb (Yubo Zhi)

TRG	= ATMEGA644P
SRC	= main.c uart.c ctrl.c
INCDIRS	= ../inc .
SUBDIRS	= adc
OPTDIR	= optional

PRGER		= usbasp
MCU_TARGET	= atmega644p
MCU_FREQ	= 12000000
OPTLEVEL	= s

# Enable 8 bits, 4 channel DAC chip TLV5620
ENABLE_DAC	= y
ifeq ($(ENABLE_DAC), y)
DAC_USE_PORTD	= n
endif

# Enable PWM, using timer 1 with PORTD, conflict with DAC(TLV5620)
ifneq ($(ENABLE_DAC), y)
ENABLE_PWM	= y
endif

CONFS_D	+= ENABLE_DAC ENABLE_PWM

ifeq ($(ENABLE_DAC), y)
SUBDIRS	+= $(OPTDIR)/dac
#DEFS	+= -DENABLE_DAC
endif

ifeq ($(ENABLE_PWM), y)
SUBDIRS	+= $(OPTDIR)/pwm
#DEFS	+= -DENABLE_PWM
endif

EFUSE	= 0xFF
HFUSE	= 0x9C
LFUSE	= 0xE7

LOG	= ~/.make_avr.log

include ../Makefile_AVR.defs
