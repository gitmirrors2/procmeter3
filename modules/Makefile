# $Header: /home/amb/CVS/procmeter3/modules/Makefile,v 1.2 1998-09-19 15:43:56 amb Exp $
#
# ProcMeter - A system monitoring program for Linux.
#
# Makefile for the modules.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994,95,96,97,98 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

CC=gcc
LD=gcc

CFLAGS=-g -O2

INSTDIR=/usr/local

SRC=$(wildcard *.c)
OBJ=$(foreach f,$(SRC),$(addsuffix .o,$(basename $f)))
LIB=$(foreach f,$(SRC),$(addsuffix .so,$(basename $f)))

########

all : $(OBJ) $(LIB)

########

%.o:%.c ../procmeter.h
	$(CC) -c $(CFLAGS) $< -o $@ -I.. -fPIC

%.so:%.o
	$(LD) $< -o $@ -shared

########

clean :
	-rm -f *.o *~ core

########

install :
	install -d $(INSTDIR)/lib/X11/ProcMeter3/modules
	install -d $(INSTDIR)/lib/X11/ProcMeter3/example
	for module in *.so ; do \
	   [ $$module = template.so ] || install -m 755 $$module $(INSTDIR)/lib/X11/ProcMeter3/modules ;\
	done
	install -m 644 README $(INSTDIR)/lib/X11/ProcMeter3/example
	install -m 644 template.c $(INSTDIR)/lib/X11/ProcMeter3/example
