# ProcMeter - A system monitoring program for Linux - Version 3.6.
#
# Makefile for gtk1 parts.
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

X_CFLAGS=`gtk-config --cflags`
X_LDFLAGS=`gtk-config --libs` -lX11

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
	@echo "ProcMeter: Cannot compile procmeter3-gtk1 program."
	@echo "ProcMeter:"

all-gtk1 : ../procmeter3-gtk1

########

../procmeter3-gtk1 : $(OBJ) $(WOBJ) $(POBJ)
	$(LD) $(OBJ) $(WOBJ) $(POBJ) -o $@ $(LDFLAGS) $(X_LDFLAGS) -ldl -rdynamic

########

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(X_CFLAGS) -I..

resources.o : resources.c ../procmeterp.h ../procmeter.h window.h
menus.o     : menus.c     ../procmeterp.h ../procmeter.h window.h
window.o    : window.c    ../procmeterp.h ../procmeter.h window.h
run.o       : run.c       ../procmeterp.h ../procmeter.h window.h

########

$(WOBJ) : 
	$(MAKE) CFLAGS="$(CFLAGS)" X_CFLAGS="$(X_CFLAGS)" -C widgets

########

$(POBJ) :
	$(MAKE) CFLAGS="$(CFLAGS)" -C .. obj

########

.PHONY : clean

clean :
	-rm -f *.o *~ core
	$(MAKE) -C widgets clean

########

.PHONY : install

install :
