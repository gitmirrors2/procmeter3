# $Header: /home/amb/CVS/procmeter3/Makefile,v 1.13 2001-01-05 19:49:53 amb Exp $
#
# ProcMeter - A system monitoring program for Linux - Version 3.3.
#
# Makefile.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994,95,96,97,98,99,2000,01 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Paths

# INSTDIR - the default root directory for the package
INSTDIR=/tmp/procmeter/usr/local

# LIB_PATH - the root dir for library files
LIB_PATH=$(INSTDIR)/lib/X11/ProcMeter3

# MOD_PATH - the path modules are stored in
MOD_PATH=$(LIB_PATH)/modules

# RC_PATH - file the procmeterrc is stored in
RC_PATH=$(LIB_PATH)/procmeterrc

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

all : procmeter3 gprocmeter3

########

procmeter3  : obj procmeter.xaw procmeter.modules procmeterrc.install

gprocmeter3 : obj procmeter.gtk procmeter.modules procmeterrc.install

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

.PHONY : procmeter.xaw procmeter.gtk

procmeter.xaw :
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C xaw

procmeter.gtk :
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C gtk

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

distclean : clean
	-rm -f procmeter3 gprocmeter3 procmeterrc.install
	$(MAKE) -C modules distclean

########

.PHONY : install

install :
	@[ -f procmeter3 ] || [ -f gprocmeter3 ] || (echo "*** Run 'make all' or 'make procmeter3' or 'make gprocmeter3' first." ; exit 1)
	install -d $(LIB_PATH)
	install -d $(MOD_PATH)
	$(MAKE) -C modules install MOD_PATH=$(MOD_PATH) LIB_PATH=$(LIB_PATH)
	install -d $(INSTDIR)/bin
	[ ! -f procmeter3 ] || install -m 755 procmeter3 $(INSTDIR)/bin
	@[ -f procmeter3 ] || (echo "" ; echo "*** The procmeter3 program has not been installed (it does not exist)." ; echo "")
	[ ! -f gprocmeter3 ] || install -m 755 gprocmeter3 $(INSTDIR)/bin
	@[ -f gprocmeter3 ] || (echo "" ; echo "*** The gprocmeter3 program has not been installed (it does not exist)." ; echo "")
	install -d $(INSTDIR)/man/man1
	install -d $(INSTDIR)/man/man5
	[ ! -f procmeter3 ] || install -m 644 procmeter3.1 $(INSTDIR)/man/man1
	[ ! -f gprocmeter3 ] || install -m 644 gprocmeter3.1 $(INSTDIR)/man/man1
	install -m 644 procmeterrc.5 $(INSTDIR)/man/man5/procmeterrc.5
	[ ! -f $(LIB_PATH)/.procmeterrc ] || mv $(LIB_PATH)/.procmeterrc $(LIB_PATH)/procmeterrc
	[ ! -f $(RC_PATH) ] || for n in 0 1 2 3 4 5 6 7 8 9; do \
				  [ ! -f $(RC_PATH) -o -f $(RC_PATH).bak.$$n ] || mv $(RC_PATH) $(RC_PATH).bak.$$n ; \
				  done
	@[ ! -f $(RC_PATH) ] || (echo "" ; echo "*** The $(RC_PATH) file has not been installed (it already exists)." ; echo "")
	[ -f $(RC_PATH) ] || install -m 644 procmeterrc.install $(RC_PATH)
	install -d $(LIB_PATH)/include
	install -m 644 procmeter.h $(LIB_PATH)/include
