# $Header: /home/amb/CVS/procmeter3/xaw/widgets/Makefile,v 1.1 1998-09-19 15:30:14 amb Exp $
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

CFLAGS=-g -O2

OBJ=PMGeneric.o PMGraph.o PMText.o

########

all : $(OBJ)

########

PMGeneric.o : PMGeneric.c PMGeneric.h PMGenericP.h
	$(CC) -c $(CFLAGS) $< -o $@
PMGraph.o : PMGraph.c PMGraph.h PMGraphP.h PMGeneric.h PMGenericP.h ../procmeter.h
	$(CC) -c $(CFLAGS) $< -o $@ -I..
PMText.o : PMText.c PMText.h PMTextP.h PMGeneric.h PMGenericP.h
	$(CC) -c $(CFLAGS) $< -o $@

########

clean :
	-rm -f *.o *~ core

########

install :

