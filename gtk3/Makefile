# ProcMeter - A system monitoring program for Linux - Version 3.6.
#
# Makefile for gtk3 parts.
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

X_CFLAGS=`pkg-config --cflags gtk+-3.0`
X_LDFLAGS=`pkg-config --libs gtk+-3.0` -lX11

#X_CFLAGS+=-DGTK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE

# Check for GTK3 development libraries and headers

HAVE_GTK3:=$(shell pkg-config --modversion gtk+-3.0)

ifdef HAVE_GTK3
  ALL=all-gtk3
else
  ALL=all-no-gtk3
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

all-no-gtk3 :
	@echo "ProcMeter:"
	@echo "ProcMeter: GTK3 is not installed (pkg-config could not find it)."
	@echo "ProcMeter: Cannot compile procmeter3-gtk3 program."
	@echo "ProcMeter:"

all-gtk3 : ../procmeter3-gtk3

########

../procmeter3-gtk3 : $(OBJ) $(WOBJ) $(POBJ)
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
