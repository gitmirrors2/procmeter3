# $Header: /home/amb/CVS/procmeter3/Makefile,v 1.22 2007-09-19 19:01:53 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.5.
#
# Makefile.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994-2007 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Paths

# INSTDIR - the default root directory for the package
INSTDIR=/usr/local

# MANDIR - the directory to install man pages to
MANDIR=$(INSTDIR)/man

# LIB_PATH - the root dir for library files
LIB_PATH=$(INSTDIR)/lib/X11/ProcMeter3

# MOD_PATH - the path modules are stored in
MOD_PATH=$(LIB_PATH)/modules

# RC_PATH - file the procmeterrc is stored in
RC_PATH=$(LIB_PATH)

# Programs

CC=gcc
LD=gcc

# Program options

CFLAGS=-g -O2 -Wall
LDFLAGS=

PATHDEFS=-DINSTDIR=\"$(INSTDIR)\"   -DLIB_PATH=\"$(LIB_PATH)\" \
         -DMOD_PATH=\"$(MOD_PATH)\" -DRC_PATH=\"$(RC_PATH)\"

# Compilation targets

SRC=$(wildcard *.c)
OBJ=$(foreach f,$(SRC),$(addsuffix .o,$(basename $f)))

########

all : procmeter3 g1procmeter3 g2procmeter3 procmeter3-log procmeter3-lcd

########

procmeter3      : obj procmeter.xaw  procmeter.modules procmeterrc.install

g1procmeter3    : obj procmeter.gtk1 procmeter.modules procmeterrc.install
g2procmeter3    : obj procmeter.gtk2 procmeter.modules procmeterrc.install

procmeter3-log  : obj procmeter.log  procmeter.modules procmeterrc.install

procmeter3-lcd  : obj procmeter.lcd  procmeter.modules procmeterrc.install

########

obj : $(OBJ)

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@  $(PATHDEFS)

procmeter.o   : procmeter.c   procmeterp.h procmeter.h
module.o      : module.c      procmeterp.h procmeter.h
procmeterrc.o : procmeterrc.c procmeterp.h procmeter.h

########

.PHONY : procmeter.modules

procmeter.modules :
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C modules

########

.PHONY : procmeter.xaw procmeter.gtk procmeter.log procmeter.lcd

procmeter.xaw  :
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C xaw

procmeter.gtk1 :
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C gtk1

procmeter.gtk2 :
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C gtk2

procmeter.log  :
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C log

procmeter.lcd  :
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C lcd

########

.PHONY : procmeterrc

procmeterrc.install: procmeterrc Makefile
	sed -e "s%path=modules%path=$(MOD_PATH)%" < procmeterrc > procmeterrc.install

########

.PHONY : clean distclean

clean :
	-rm -f *.o *~ core procmeterrc.install
	$(MAKE) -C modules clean
	$(MAKE) -C xaw  clean
	$(MAKE) -C gtk1 clean
	$(MAKE) -C gtk2 clean
	$(MAKE) -C log  clean
	$(MAKE) -C lcd  clean

distclean : clean
	-rm -f procmeter3 g1procmeter3 g2procmeter3 procmeter3-log procmeter3-lcd
	$(MAKE) -C modules distclean

########

.PHONY : install

install :
	@[ -f procmeter3 ] || [ -f g1procmeter3 ] || [ -f g2procmeter3 ] || [ -f procmeter3-log ] || [ -f procmeter3-lcd ] || \
	  (echo "*** Run 'make all' or 'make procmeter3' or 'make g1procmeter3' or 'make g2procmeter3' or 'make procmeter3-log' or 'make procmeter3-lcd' first." ; exit 1)
	install -d $(LIB_PATH)
	install -d $(MOD_PATH)
	install -d $(RC_PATH)
#
	$(MAKE) -C modules install MOD_PATH=$(MOD_PATH) LIB_PATH=$(LIB_PATH)
#
	install -d $(INSTDIR)/bin
	[ ! -f procmeter3 ]     || install -m 755 procmeter3 $(INSTDIR)/bin
	@[ -f procmeter3 ]      || (echo "" ; echo "*** The procmeter3 program has not been installed (it does not exist)." ; echo "")
	[ ! -f g1procmeter3 ]   || install -m 755 g1procmeter3 $(INSTDIR)/bin
	@[ -f g1procmeter3 ]    || (echo "" ; echo "*** The g1procmeter3 program has not been installed (it does not exist)." ; echo "")
	[ ! -f g2procmeter3 ]   || install -m 755 g2procmeter3 $(INSTDIR)/bin
	@[ -f g2procmeter3 ]    || (echo "" ; echo "*** The g2procmeter3 program has not been installed (it does not exist)." ; echo "")
	[ ! -f procmeter3-log ] || install -m 755 procmeter3-log $(INSTDIR)/bin
	@[ -f procmeter3-log ]  || (echo "" ; echo "*** The procmeter3-log program has not been installed (it does not exist)." ; echo "")
	[ ! -f procmeter3-lcd ] || install -m 755 procmeter3-lcd $(INSTDIR)/bin
	@[ -f procmeter3-lcd ]  || (echo "" ; echo "*** The procmeter3-lcd program has not been installed (it does not exist)." ; echo "")
#
	install -d $(MANDIR)/man1
	install -d $(MANDIR)/man5
	[ ! -f procmeter3 ]     || install -m 644 man/procmeter3.1      $(MANDIR)/man1
	[ ! -f procmeter3-log ] || install -m 644 man/procmeter3-log.1  $(MANDIR)/man1
	[ ! -f procmeter3-lcd ] || install -m 644 man/procmeter3-lcd.1  $(MANDIR)/man1
	[ ! -f g1procmeter3 ]   || install -m 644 man/g1procmeter3.1    $(MANDIR)/man1
	[ ! -f g2procmeter3 ]   || install -m 644 man/g2procmeter3.1    $(MANDIR)/man1
	install -m 644 man/procmeterrc.5        $(MANDIR)/man5
	install -m 644 man/procmeter3_modules.1 $(MANDIR)/man1
#
	[ ! -f $(RC_PATH)/.procmeterrc ] || mv $(RC_PATH)/.procmeterrc $(RC_PATH)/procmeterrc
	@[ ! -f $(RC_PATH)/procmeterrc ] || (echo "" ; echo "*** The $(RC_PATH)/procmeterrc file has not been installed (it already exists)." ; echo "")
	[ -f $(RC_PATH)/procmeterrc ] || install -m 644 procmeterrc.install $(RC_PATH)/procmeterrc
	install -m 644 procmeterrc.install $(RC_PATH)
#
	install -d $(LIB_PATH)/include
	install -m 644 procmeter.h $(LIB_PATH)/include
