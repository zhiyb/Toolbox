TRG	= LM3S811
SUFFIX	= .axf

SRC	= ctrl.c main.c uart.c startup_${COMPILER}.c
SUBDIRS	= adc
INCDIRS	= ../inc .
ROOT	= ../StellarisWare
LOG	= ~/.make_${PART}.log

PART	= LM3S811
ENTRY	= ResetISR

OPTLEVEL	= s
COMPILER	= gcc
CROSS_COMPILE	= ${shell type arm-stellaris-eabi-gcc > /dev/null 2>&1 && \
		    echo arm-stellaris-eabi- || echo arm-none-eabi-}

INCDIRS += ${ROOT} ${ROOT}/boards/ek-lm3s811
CFLG	= -mthumb -mcpu=cortex-m3 -pedantic -ffunction-sections
CFLG	+= -fdata-sections -std=c99
DEFS	+= -DPART_${PART} -D${COMPILER}
LDFLG	= -Wl,-T,${PART}.ld -Wl,--gc-sections -Wl,--entry,${ENTRY}
LIBS	+= ${ROOT}/driverlib/${COMPILER}-cm3/libdriver-cm3.a -lm


include ../Makefile_generic.defs

CLEAN_FILES	+= ${TRG}.bin
${TRG}.bin: ${ELF}
	${OBJCOPY} -O binary $< ${TRG}.bin

flash: ${TRG}.bin
	openocd -f board/ek-lm3s811.cfg -c "init" -c "reset init" -c "flash write_image erase ${TRG}.bin" -c "reset run" -c "shutdown"
	echo $(shell date '+%Y-%m-%d %H:%M:%S'), flash, ${PART}, ${PWD}, ${TRG} >> $(LOG)
