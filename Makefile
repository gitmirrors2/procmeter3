# $Header: /home/amb/CVS/procmeter3/Makefile,v 1.21 2003-01-17 18:48:39 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.4a.
#
# Makefile.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994,95,96,97,98,99,2000,01,02 Andrew M. Bishop
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

all : procmeter3 gprocmeter3 procmeter3-log procmeter3-lcd

########

procmeter3      : obj procmeter.xaw procmeter.modules procmeterrc.install

gprocmeter3     : obj procmeter.gtk procmeter.modules procmeterrc.install

procmeter3-log  : obj procmeter.log procmeter.modules procmeterrc.install

procmeter3-lcd  : obj procmeter.lcd procmeter.modules procmeterrc.install

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

procmeter.xaw :
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C xaw

procmeter.gtk :
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C gtk

procmeter.log :
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C log

procmeter.lcd :
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
	$(MAKE) -C xaw clean
	$(MAKE) -C gtk clean
	$(MAKE) -C log clean
	$(MAKE) -C lcd clean

distclean : clean
	-rm -f procmeter3 gprocmeter3 procmeter3-log procmeter3-lcd
	$(MAKE) -C modules distclean

########

.PHONY : install

install :
	@[ -f procmeter3 ] || [ -f gprocmeter3 ] || [ -f procmeter3-log ] || [ -f procmeter3-lcd ] || \
	  (echo "*** Run 'make all' or 'make procmeter3' or 'make gprocmeter3' or 'make procmeter3-log' or 'make procmeter3-lcd' first." ; exit 1)
	install -d $(LIB_PATH)
	install -d $(MOD_PATH)
	install -d $(RC_PATH)
#
	$(MAKE) -C modules install MOD_PATH=$(MOD_PATH) LIB_PATH=$(LIB_PATH)
#
	install -d $(INSTDIR)/bin
	[ ! -f procmeter3 ]     || install -m 755 procmeter3 $(INSTDIR)/bin
	@[ -f procmeter3 ]      || (echo "" ; echo "*** The procmeter3 program has not been installed (it does not exist)." ; echo "")
	[ ! -f gprocmeter3 ]    || install -m 755 gprocmeter3 $(INSTDIR)/bin
	@[ -f gprocmeter3 ]     || (echo "" ; echo "*** The gprocmeter3 program has not been installed (it does not exist)." ; echo "")
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
	[ ! -f gprocmeter3 ]    || install -m 644 man/gprocmeter3.1     $(MANDIR)/man1
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
