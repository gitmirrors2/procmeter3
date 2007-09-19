# $Header: /home/amb/CVS/procmeter3/gtk2/widgets/Makefile,v 1.1 2007-09-19 19:03:46 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.5.
#
# Makefile for gtk2 widgets.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1998-2007 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Programs

CC=gcc
LD=gcc

# Program options (overridden by main Makefile)

CFLAGS=-g -O2 -Wall

X_CFLAGS=`pkg-config --cflags gtk+-2.0`

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

SubMenus.o : SubMenus.c SubMenus.h

########

.PHONY : clean

clean :
	-rm -f *.o *~ core

########

.PHONY : install

install :
