# $Header: /home/amb/CVS/procmeter3/gtk1/Makefile,v 1.3 2007-09-19 19:02:28 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.5.
#
# Makefile for gtk1 parts.
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

X_CFLAGS=`gtk-config --cflags`
X_LDFLAGS=`gtk-config --libs`

# Check for GTK1 development libraries and headers

HAVE_GTK1:=$(shell gtk-config --version)

ifdef HAVE_GTK1
  ALL=all-gtk1
else
  ALL=all-no-gtk1
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

all-no-gtk1 :
	@echo "ProcMeter:"
	@echo "ProcMeter: GTK1 is not installed (gtk-config could not be found)."
	@echo "ProcMeter: Cannot compile g1procmeter3 program."
	@echo "ProcMeter:"

all-gtk1 : $(OBJ) procmeter.widget \
      ../g1procmeter3

########

../g1procmeter3 : $(OBJ) procmeter.widget procmeter.main
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
