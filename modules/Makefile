# $Header: /home/amb/CVS/procmeter3/modules/Makefile,v 1.4 1999-04-18 08:17:49 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.1a.
#
# Makefile for the modules.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994,95,96,97,98,99 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

CC=gcc
LD=gcc

CFLAGS=-g -O2 -Wall

INSTDIR=/usr/local
LIB_PATH=$(INSTDIR)/lib/X11/ProcMeter3
MOD_PATH=$(LIB_PATH)/modules
RC_PATH=$(LIB_PATH)/.procmeterrc

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
	install -d $(MOD_PATH)
	install -d $(LIB_PATH)/example
	for module in *.so ; do \
	   [ $$module = template.so ] || install -m 755 $$module $(MOD_PATH) ;\
	done
	install -m 644 README $(LIB_PATH)/example
	install -m 644 template.c $(LIB_PATH)/example
