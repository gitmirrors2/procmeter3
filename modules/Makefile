# $Header: /home/amb/CVS/procmeter3/modules/Makefile,v 1.7 2007-09-26 18:49:53 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.5.
#
# Makefile for the modules.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994-2007 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Paths (overridden by main Makefile)

INSTDIR=/usr/local
LIB_PATH=$(INSTDIR)/lib/X11/ProcMeter3
MOD_PATH=$(LIB_PATH)/modules

# Programs

CC=gcc
LD=gcc

# Program options (overridden by main Makefile)

CFLAGS=-g -O2 -Wall
LDFLAGS=

# Compilation targets

SRC=$(wildcard *.c)
OBJ=$(foreach f,$(SRC),$(addsuffix .o,$(basename $f)))
LIB=$(foreach f,$(SRC),$(addsuffix .so,$(basename $f)))

########

all : $(OBJ) $(LIB)

########

%.o:%.c ../procmeter.h
	$(CC) -c $(CFLAGS) $< -o $@ -I.. -fPIC

%.so:%.o
	$(LD) $(LDFLAGS) $< -o $@ -shared

########

.PHONY : clean distclean

clean :
	-rm -f *.o *~ core

distclean : clean
	-rm -f *.so

########

.PHONY : install

install :
	install -d $(DESTDIR)$(MOD_PATH)
	install -d $(DESTDIR)$(LIB_PATH)/example
	for module in *.so ; do \
	   [ $$module = template.so ] || install -m 755 $$module $(DESTDIR)$(MOD_PATH) ;\
	done
	install -m 644 README $(DESTDIR)$(LIB_PATH)/example
	install -m 644 template.c $(DESTDIR)$(LIB_PATH)/example
