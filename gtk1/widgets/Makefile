# ProcMeter - A system monitoring program for Linux - Version 3.6.
#
# Makefile for gtk1 widgets.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1998-2012 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Programs

CC=gcc

# Program options (overridden by main Makefile)

CFLAGS=-g -O2 -Wall

X_CFLAGS=`gtk-config --cflags`

# Compilation targets

SRC=$(wildcard *.c)
OBJ=$(foreach f,$(SRC),$(addsuffix .o,$(basename $f)))

########

all : $(OBJ)

########

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(X_CFLAGS) -I../..

PMGeneric.o : PMGeneric.c PMGeneric.h
PMGraph.o   : PMGraph.c   PMGraph.h   PMGeneric.h ../../procmeter.h
PMText.o    : PMText.c    PMText.h    PMGeneric.h
PMBar.o     : PMBar.c     PMBar.h     PMGeneric.h ../../procmeter.h

########

.PHONY : clean

clean :
	-rm -f *.o *~ core

########

.PHONY : install

install :
