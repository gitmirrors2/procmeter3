# ProcMeter - A system monitoring program for Linux - Version 3.6.
#
# Makefile for logging to a file.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994-2012 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Programs

CC=gcc
LD=gcc

# Program options

CFLAGS=-g -O2 -Wall
LDFLAGS=-g

# Compilation targets

SRC=$(wildcard *.c)
OBJ=$(foreach f,$(SRC),$(addsuffix .o,$(basename $f)))

PSRC=$(wildcard ../*.c)
POBJ=$(foreach f,$(PSRC),$(addsuffix .o,$(basename $f)))

########

all : ../procmeter3-log

########

../procmeter3-log : $(OBJ) $(POBJ)
	$(LD) $(OBJ) $(POBJ) -o $@ $(LDFLAGS) -ldl -rdynamic

########

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ -I..

window.o    : window.c    ../procmeterp.h ../procmeter.h
menu.o      : menu.c      ../procmeterp.h ../procmeter.h
run.o       : run.c       ../procmeterp.h ../procmeter.h

########

$(POBJ) :
	$(MAKE) CFLAGS="$(CFLAGS)" -C .. obj

########

.PHONY : clean

clean :
	-rm -f *.o *~ core

########

.PHONY : install

install :
