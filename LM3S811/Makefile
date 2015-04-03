#******************************************************************************
#
# Makefile - Rules for building LM3S811 programs
#
# Copyright (c) 2006-2013 Texas Instruments Incorporated.  All rights reserved.
# Software License Agreement
# 
# Texas Instruments (TI) is supplying this software for use solely and
# exclusively on TI's microcontroller products. The software is owned by
# TI and/or its suppliers, and is protected under applicable copyright
# laws. You may not combine this software with "viral" open-source
# software in order to form a larger program.
# 
# THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
# NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
# NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
# CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
# DAMAGES, FOR ANY REASON WHATSOEVER.
# 
# This is part of revision 10636 of the EK-LM3S811 Firmware Package.
#
#******************************************************************************
# Modified by zhiyb(Yubo Zhi, yz39g13@soton.ac.uk)

#
# Defines the program name.
#
PRG	= LM3S811

#
# Defines source files
#
SRC	= ctrl.c main.c startup_gcc.c uart.c
SRC	+= adc/adc.c adc/timer0.c
SRC	+= startup_${COMPILER}.c
OBJS	= $(addprefix ${COMPILER}/,$(subst .c,.o,$(subst .cpp,.o,${SRC})))
OBJS	+= ${PART}.ld
OBJS	+= ${ROOT}/driverlib/${COMPILER}-cm3/libdriver-cm3.a

#
# Defines the part type that this project uses.
#
PART	= LM3S811

#
# Defines the log file.
#
LOG	= ~/.make_${PART}.log

#
# The base directory for StellarisWare.
#
ROOT	= ../StellarisWare

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
#VPATH=${ROOT}/boards/ek-lm3s811/drivers

#
# Where to find header files that do not live in the source directory.
#
IPATH	= ${ROOT}/boards/ek-lm3s811
IPATH	+= ${ROOT}
IPATH	+= . ../inc
IPATH	+= adc

#
# CFLAGS modifies.
#
CFLAGS	+= -g -Wall -Werror -Os

#
# The default rule, which causes the program to be built.
#
all: ${COMPILER}-adc ${COMPILER}
all: ${COMPILER}/${PRG}.axf
all: size

${COMPILER}-adc:
	@mkdir -p ${COMPILER}/adc

#
# Command for display sizes of built sections.
#
SIZE=${PREFIX}-size

#
# Display sizes of built sections.
#
.PHONY: size
size: ${COMPILER}/${PRG}.axf
	@if [ 'x${VERBOSE}' = x ]; then				\
		echo "  SIZE  ${<}";				\
	 else							\
	 	echo ${SIZE} ${<};				\
	 fi
	@${SIZE} ${<}

#
# The rule to clean out all the build products.
#
clean:
	rm -rf ${COMPILER} ${wildcard *~}

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir -p ${COMPILER}

#
# Rules for building the program.
#
${COMPILER}/${PRG}.axf: ${OBJS}
SCATTERgcc_${PRG}=${PART}.ld
ENTRY_${PRG}=ResetISR

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif

#
# Program the MCU
#
flash: all
	openocd -f board/ek-lm3s811.cfg -c "init" -c "reset init" -c "flash write_image erase ${COMPILER}/${PRG}.bin" -c "reset run" -c "shutdown"
	echo $(shell date '+%Y-%m-%d %H:%M:%S'), flash, ${PART}, ${PWD}, ${PRG} >> $(LOG)
