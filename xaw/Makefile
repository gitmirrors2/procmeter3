# $Header: /home/amb/CVS/procmeter3/xaw/Makefile,v 1.1 2000-12-16 16:54:23 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.3.
#
# Makefile for xaw parts.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994,95,96,97,98,99,2000 Andrew M. Bishop
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

X_CFLAGS=-I/usr/X11/include -I/usr/X11R6/include
X_LDFLAGS=-L/usr/X11/lib -L/usr/X11R6/lib -lXaw -lXmu -lXt -lXext -lSM -lICE -lX11

# Compilation targets

SRC=$(wildcard *.c)
OBJ=$(foreach f,$(SRC),$(addsuffix .o,$(basename $f)))

PSRC=$(wildcard ../*.c)
POBJ=$(foreach f,$(PSRC),$(addsuffix .o,$(basename $f)))

WSRC=$(wildcard widgets/*.c)
WOBJ=$(foreach f,$(WSRC),$(addsuffix .o,$(basename $f)))

########

all : $(OBJ) procmeter.widget \
      ../procmeter3

########

../procmeter3 : $(OBJ) procmeter.widget procmeter.main
	$(LD) $(OBJ) $(WOBJ) $(POBJ) -o $@ -ldl $(LDFLAGS) $(X_LDFLAGS)

########

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(X_CFLAGS) -I..

bitmap.o    : bitmap.c    ../procmeterp.h ../procmeter.h window.h
resources.o : resources.c ../procmeterp.h ../procmeter.h window.h
menus.o     : menus.c     ../procmeterp.h ../procmeter.h window.h
window.o    : window.c    ../procmeterp.h ../procmeter.h window.h
run.o       : run.c       ../procmeterp.h ../procmeter.h window.h

########

.PHONY : procmeter.widget

procmeter.widget :
	$(MAKE) CFLAGS="$(CFLAGS)" X_CFLAGS="$(X_CFLAGS)" -C widgets

########

.PHONY : procmeter.main

$(POBJ) : procmeter.main

procmeter.main :
	$(MAKE) CFLAGS="$(CFLAGS)" -C .. obj

########

.PHONY : clean

clean :
	-rm -f *.o *~ core
	$(MAKE) -C widgets clean

########

.PHONY : install

install :
