# Modified by zhiyb (Yubo Zhi)

TRG	= ArduinoUNO
SRC	= main.c uart.c ctrl.c
INCDIRS	= . ../inc
SUBDIRS	= adc
OPTDIR	= optional

PRGER	= arduino -P /dev/ttyACM0
MCU_TARGET	= atmega328p
MCU_FREQ	= 8000000
OPTLEVEL	= s

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

EFUSE	= 0xFF
HFUSE	= 0x9C
LFUSE	= 0xE7

LOG	= ~/.make_avr.log

include ../Makefile_AVR.defs
