# $Header: /home/amb/CVS/procmeter3/xaw/widgets/Makefile,v 1.5 2000-12-16 17:01:20 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.3.
#
# Makefile for athena widgets.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1998,99,2000 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Programs

CC=gcc
LD=gcc

# Program options (overridden by main Makefile)

CFLAGS=-g -O2 -Wall

XAW_CFLAGS=-I/usr/X11/include -I/usr/X11R6/include

# Compilation targets

SRC=$(wildcard *.c)
OBJ=$(foreach f,$(SRC),$(addsuffix .o,$(basename $f)))

########

all : $(OBJ)

########

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(X_CFLAGS) -I../..

PMGeneric.o : PMGeneric.c PMGeneric.h PMGenericP.h
PMGraph.o   : PMGraph.c   PMGraph.h   PMGraphP.h   PMGeneric.h PMGenericP.h ../../procmeter.h
PMText.o    : PMText.c    PMText.h    PMTextP.h    PMGeneric.h PMGenericP.h
PMBar.o     : PMBar.c     PMBar.h     PMBarP.h     PMGeneric.h PMGenericP.h ../../procmeter.h

SubMenus.o : SubMenus.c SubMenus.h

########

.PHONY : clean

clean :
	-rm -f *.o *~ core

########

.PHONY : install

install :
