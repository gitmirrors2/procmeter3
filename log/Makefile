# $Header: /home/amb/CVS/procmeter3/log/Makefile,v 1.1 2002-06-04 12:48:24 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.3b.
#
# Makefile for non X windows parts.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994,95,96,97,98,99,2000,02 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Programs

CC=gcc
LD=gcc

# Program options

CFLAGS=-g -O2 -Wall
LDFLAGS=

# Compilation targets

SRC=$(wildcard *.c)
OBJ=$(foreach f,$(SRC),$(addsuffix .o,$(basename $f)))

PSRC=$(wildcard ../*.c)
POBJ=$(foreach f,$(PSRC),$(addsuffix .o,$(basename $f)))

########

all : $(OBJ) \
      ../procmeter3-no-x

########

../procmeter3-no-x : $(OBJ) procmeter.main
	$(LD) $(OBJ) $(POBJ) -o $@ -ldl $(LDFLAGS)

########

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ -I..

window.o    : window.c    ../procmeterp.h ../procmeter.h
menu.o      : menu.c      ../procmeterp.h ../procmeter.h
run.o       : run.c       ../procmeterp.h ../procmeter.h
widget.o    : widget.c    ../procmeterp.h ../procmeter.h

########

.PHONY : procmeter.main

$(POBJ) : procmeter.main

procmeter.main :
	$(MAKE) CFLAGS="$(CFLAGS)" -C .. obj

########

.PHONY : clean

clean :
	-rm -f *.o *~ core

########

.PHONY : install

install :
