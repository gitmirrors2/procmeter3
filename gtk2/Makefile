# $Header: /home/amb/CVS/procmeter3/gtk2/Makefile,v 1.1 2007-09-19 19:06:17 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.5.
#
# Makefile for gtk2 parts.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994-2007 Andrew M. Bishop
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

X_CFLAGS=`pkg-config --cflags gtk+-2.0`
X_LDFLAGS=`pkg-config --libs gtk+-2.0`

# Check for GTK2 development libraries and headers

HAVE_GTK2:=$(shell pkg-config --modversion gtk+-2.0)

ifdef HAVE_GTK2
  ALL=all-gtk2
else
  ALL=all-no-gtk2
endif

# Compilation targets

SRC=$(wildcard *.c)
OBJ=$(foreach f,$(SRC),$(addsuffix .o,$(basename $f)))

PSRC=$(wildcard ../*.c)
POBJ=$(foreach f,$(PSRC),$(addsuffix .o,$(basename $f)))

WSRC=$(wildcard widgets/*.c)
WOBJ=$(foreach f,$(WSRC),$(addsuffix .o,$(basename $f)))

########

all : $(ALL)

all-no-gtk2 :
	@echo "ProcMeter:"
	@echo "ProcMeter: GTK2 is not installed (pkg-config could not find it)."
	@echo "ProcMeter: Cannot compile g2procmeter3 program."
	@echo "ProcMeter:"

all-gtk2 : $(OBJ) procmeter.widget \
      ../g2procmeter3

########

../g2procmeter3 : $(OBJ) procmeter.widget procmeter.main
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
