# $Header: /home/amb/CVS/procmeter3/Makefile,v 1.5 1999-02-13 11:34:37 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.1.
#
# Makefile.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994,95,96,97,98,99 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Paths:

# INSTDIR - the default root directory for the package
INSTDIR=/usr/local

# LIB_PATH - the root dir for library files
LIB_PATH=$(INSTDIR)/lib/X11/ProcMeter3

# MOD_PATH - the path modules are stored in
MOD_PATH=$(LIB_PATH)/modules

# RC_PATH - file the procmeterrc is stored in
RC_PATH=$(LIB_PATH)/procmeterrc

CC=gcc
LD=gcc

XINCLUDE=-I/usr/X11/include
XLIBRARY=-L/usr/X11/lib -lXaw -lXmu -lXt -lXext -lSM -lICE -lX11

CFLAGS=-g -O2 -Wall
CDEFS=-DINSTDIR=\"$(INSTDIR)\" -DLIB_PATH=\"$(LIB_PATH)\" \
      -DMOD_PATH=\"$(MOD_PATH)\" -DRC_PATH=\"$(RC_PATH)\"

OBJ =procmeter.o module.o procmeterrc.o \
     xbitmap.o xmenus.o xresources.o xwindow.o
WOBJ=widgets/PMGeneric.o widgets/PMGraph.o widgets/PMText.o

########

all : procmeter.modules procmeter.widgets procmeter3 procmeterrc.install

########

procmeter3 : $(OBJ) $(WOBJ)
	$(LD) $(OBJ) $(WOBJ) -o $@ -ldl $(XLIBRARY)

########

procmeter.o   : procmeter.c   procmeterp.h procmeter.h
	$(CC) -c $(CFLAGS) $< -o $@  $(CDEFS)
module.o      : module.c      procmeterp.h procmeter.h
	$(CC) -c $(CFLAGS) $< -o $@  $(CDEFS)
procmeterrc.o : procmeterrc.c procmeterp.h procmeter.h
	$(CC) -c $(CFLAGS) $< -o $@  $(CDEFS)

xbitmap.o     : xbitmap.c     procmeterp.h procmeter.h xwindow.h
	$(CC) -c $(CFLAGS) $< -o $@ $(CDEFS) $(XINCLUDE) 
xmenus.o      : xmenus.c      procmeterp.h procmeter.h xwindow.h
	$(CC) -c $(CFLAGS) $< -o $@ $(CDEFS) $(XINCLUDE) 
xresources.o  : xresources.c  procmeterp.h procmeter.h xwindow.h
	$(CC) -c $(CFLAGS) $< -o $@ $(CDEFS) $(XINCLUDE) 
xwindow.o     : xwindow.c     procmeterp.h procmeter.h xwindow.h
	$(CC) -c $(CFLAGS) $< -o $@ $(CDEFS) $(XINCLUDE) 

########

procmeter.modules :
	$(MAKE) CFLAGS="$(CFLAGS)" -C modules

########

procmeter.widgets :
	$(MAKE) CFLAGS="$(CFLAGS)" -C widgets

########

.PHONY: procmeterrc

procmeterrc.install: procmeterrc Makefile
	sed -e "s%path=modules%path=$(MOD_PATH)%" < procmeterrc > procmeterrc.install

########


clean :
	-rm -f *.o *~ core procmeterrc.install
	$(MAKE) -C modules clean
	$(MAKE) -C widgets clean

########

install : all
	install -d $(LIB_PATH)
	$(MAKE) -C modules install MOD_PATH=$(MOD_PATH) LIB_PATH=$(LIB_PATH)
	$(MAKE) -C widgets install MOD_PATH=$(MOD_PATH) LIB_PATH=$(LIB_PATH)
	install -d $(INSTDIR)/bin
	install -m 755 procmeter3 $(INSTDIR)/bin
	install -d $(INSTDIR)/man/man1
	install -d $(INSTDIR)/man/man5
	install -m 644 procmeter.1 $(INSTDIR)/man/man1/procmeter3.1
	install -m 644 procmeterrc.5 $(INSTDIR)/man/man5/procmeterrc.5
	[ ! -f $(LIB_PATH)/.procmeterrc ] || mv $(LIB_PATH)/.procmeterrc $(LIB_PATH)/procmeterrc
	[ ! -f $(RC_PATH) ] || mv $(RC_PATH) $(RC_PATH).bak
	install -m 644 procmeterrc.install $(RC_PATH)
	install -d $(LIB_PATH)/include
	install -m 644 procmeter.h $(LIB_PATH)/include

