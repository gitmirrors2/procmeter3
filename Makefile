# $Header: /home/amb/CVS/procmeter3/Makefile,v 1.1 1998-09-19 15:19:00 amb Exp $
#
# ProcMeter - A system monitoring program for Linux.
#
# Makefile.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994,95,96,97,98 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

INSTDIR=/usr/local

CC=gcc
LD=gcc

XINCLUDE=-I/usr/X11/include
XLIBRARY=-L/usr/X11/lib -lXaw -lXmu -lXt -lXext -lSM -lICE -lX11

CFLAGS=-g -O2

COMPILE=$(CC) -c $(CFLAGS)

OBJ =procmeter.o module.o procmeterrc.o \
     xbitmap.o xmenus.o xresources.o xwindow.o
WOBJ=widgets/PMGeneric.o widgets/PMGraph.o widgets/PMText.o

########

all : procmeter3 procmeter.modules procmeter.widgets

########

procmeter3 : $(OBJ) procmeter.widgets procmeter.modules
	$(LD) $(OBJ) $(WOBJ) -o $@ -ldl $(XLIBRARY)

########

procmeter.o   : procmeter.c   procmeterp.h procmeter.h
	$(CC) -c $(CFLAGS) $< -o $@ -DINSTDIR=\"$(INSTDIR)\"
module.o      : module.c      procmeterp.h procmeter.h
	$(CC) -c $(CFLAGS) $< -o $@ -DINSTDIR=\"$(INSTDIR)\"
procmeterrc.o : procmeterrc.c procmeterp.h procmeter.h
	$(CC) -c $(CFLAGS) $< -o $@ -DINSTDIR=\"$(INSTDIR)\"

xbitmap.o     : xbitmap.c     procmeterp.h procmeter.h xwindow.h
	$(CC) -c $(CFLAGS) $< -o $@ $(XINCLUDE) -DINSTDIR=\"$(INSTDIR)\"
xmenus.o      : xmenus.c      procmeterp.h procmeter.h xwindow.h
	$(CC) -c $(CFLAGS) $< -o $@ $(XINCLUDE) -DINSTDIR=\"$(INSTDIR)\"
xresources.o  : xresources.c  procmeterp.h procmeter.h xwindow.h
	$(CC) -c $(CFLAGS) $< -o $@ $(XINCLUDE) -DINSTDIR=\"$(INSTDIR)\"
xwindow.o     : xwindow.c     procmeterp.h procmeter.h xwindow.h
	$(CC) -c $(CFLAGS) $< -o $@ $(XINCLUDE) -DINSTDIR=\"$(INSTDIR)\"

########

procmeter.modules :
	$(MAKE) -C modules

########

procmeter.widgets :
	$(MAKE) -C widgets

########

clean :
	-rm -f *.o *~ core
	$(MAKE) -C modules clean
	$(MAKE) -C widgets clean

########

install :
	install -d $(INSTDIR)/lib/X11/procmeter3
	$(MAKE) -C modules install INSTDIR=$(INSTDIR)
	$(MAKE) -C widgets install INSTDIR=$(INSTDIR)
	sed -e "s%path=modules%path=$(INSTDIR)/lib/X11/procmeter3/modules%" < .procmeterrc > .procmeterrc.install
	install -d $(INSTDIR)/bin
	install -m 755 procmeter3 $(INSTDIR)/bin
	install -d $(INSTDIR)/man/man1
	install -d $(INSTDIR)/man/man5
	install -m 644 procmeter.1 $(INSTDIR)/man/man1/procmeter3.1
	install -m 644 procmeterrc.5 $(INSTDIR)/man/man5/procmeterrc.1
	install -m 644 .procmeterrc.install $(INSTDIR)/lib/X11/procmeter3/.procmeterrc
	install -d $(INSTDIR)/lib/X11/procmeter3/include
	install -m 644 procmeter.h $(INSTDIR)/lib/X11/procmeter3/include
