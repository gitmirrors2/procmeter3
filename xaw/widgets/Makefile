# $Header: /home/amb/CVS/procmeter3/xaw/widgets/Makefile,v 1.2 1999-04-17 06:22:58 amb Exp $
#
# ProcMeter - A system monitoring program for Linux.
#
# Widgets Makefile.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1998 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

CC=gcc
LD=gcc

XINCLUDE=-I/usr/X11/include

CFLAGS=-g -O2

OBJ=PMGeneric.o PMGraph.o PMText.o

########

all : $(OBJ)

########

PMGeneric.o : PMGeneric.c PMGeneric.h PMGenericP.h
	$(CC) -c $(CFLAGS) $< -o $@ $(XINCLUDE)
PMGraph.o : PMGraph.c PMGraph.h PMGraphP.h PMGeneric.h PMGenericP.h ../procmeter.h
	$(CC) -c $(CFLAGS) $< -o $@ -I.. $(XINCLUDE)
PMText.o : PMText.c PMText.h PMTextP.h PMGeneric.h PMGenericP.h
	$(CC) -c $(CFLAGS) $< -o $@ $(XINCLUDE)

########

clean :
	-rm -f *.o *~ core

########

install :

