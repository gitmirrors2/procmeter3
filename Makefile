# ProcMeter - A system monitoring program for Linux - Version 3.6.
#
# Makefile.
#
# Written by Andrew M. Bishop
#
# This file Copyright 1994-2012 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Paths

# INSTDIR - the default root directory for the package
INSTDIR=/usr/local

# MANDIR - the directory to install man pages to
MANDIR=$(INSTDIR)/share/man

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
LDFLAGS=-g

PATHDEFS=-DINSTDIR=\"$(INSTDIR)\"   -DLIB_PATH=\"$(LIB_PATH)\" \
         -DMOD_PATH=\"$(MOD_PATH)\" -DRC_PATH=\"$(RC_PATH)\"

# Compilation targets

SRC=$(wildcard *.c)
OBJ=$(foreach f,$(SRC),$(addsuffix .o,$(basename $f)))

########

all : procmeter3-xaw procmeter3-gtk1 procmeter3-gtk2 procmeter3-gtk3 procmeter3-log procmeter3-lcd

########

procmeter3-xaw  : obj procmeter3.modules procmeterrc.install

procmeter3-gtk1 : obj procmeter3.modules procmeterrc.install
procmeter3-gtk2 : obj procmeter3.modules procmeterrc.install
procmeter3-gtk3 : obj procmeter3.modules procmeterrc.install

procmeter3-log  : obj procmeter3.modules procmeterrc.install

procmeter3-lcd  : obj procmeter3.modules procmeterrc.install

########

obj : $(OBJ)

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@  $(PATHDEFS)

procmeter.o   : procmeter.c   procmeterp.h procmeter.h
module.o      : module.c      procmeterp.h procmeter.h
procmeterrc.o : procmeterrc.c procmeterp.h procmeter.h

########

.PHONY : procmeter3.modules

procmeter3.modules :
	$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" -C modules

########

procmeter3-xaw  :
	$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" -C xaw

procmeter3-gtk1 :
	$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" -C gtk1

procmeter3-gtk2 :
	$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" -C gtk2

procmeter3-gtk3 :
	$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" -C gtk3

procmeter3-log  :
	$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" -C log

procmeter3-lcd  :
	$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" -C lcd

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
	$(MAKE) -C gtk3 clean
	$(MAKE) -C log  clean
	$(MAKE) -C lcd  clean

distclean : clean
	-rm -f procmeter3-xaw procmeter3-gtk1 procmeter3-gtk2 procmeter3-gtk3 procmeter3-log procmeter3-lcd
	-rm -f procmeter3 gprocmeter3
	$(MAKE) -C modules distclean

########

.PHONY : install install-strip

install :
	@[ -f procmeter3-xaw ] || [ -f procmeter3-gtk1 ] || [ -f procmeter3-gtk2 ] || [ -f procmeter3-gtk3 ] || [ -f procmeter3-log ] || [ -f procmeter3-lcd ] || \
	  (echo "*** Run 'make all' or 'make procmeter3-xaw' or 'make procmeter3-gtk1' or 'make procmeter3-gtk2' or 'make procmeter3-gtk3' or 'make procmeter3-log' or 'make procmeter3-lcd' first." ; exit 1)
	install -d $(DESTDIR)$(LIB_PATH)
	install -d $(DESTDIR)$(MOD_PATH)
	install -d $(DESTDIR)$(RC_PATH)
#
	$(MAKE) -C modules install STRIP=$(STRIP) INSTDIR=$(INSTDIR) MOD_PATH=$(MOD_PATH) LIB_PATH=$(LIB_PATH)
#
	install -d $(DESTDIR)$(INSTDIR)/bin
	[ ! -f procmeter3-xaw ]  || install $(STRIP) -m 755 procmeter3-xaw $(DESTDIR)$(INSTDIR)/bin
	@[ -f procmeter3-xaw ]   || (echo "" ; echo "*** The procmeter3-xaw program has not been installed (it does not exist)." ; echo "")
	[ ! -f procmeter3-xaw ]  || ln -sf procmeter3-xaw $(DESTDIR)$(INSTDIR)/bin/procmeter3
	[ ! -f procmeter3-gtk1 ] || install $(STRIP) -m 755 procmeter3-gtk1 $(DESTDIR)$(INSTDIR)/bin
	@[ -f procmeter3-gtk1 ]  || (echo "" ; echo "*** The procmeter3-gtk1 program has not been installed (it does not exist)." ; echo "")
	[ ! -f procmeter3-gtk2 ] || install $(STRIP) -m 755 procmeter3-gtk2 $(DESTDIR)$(INSTDIR)/bin
	@[ -f procmeter3-gtk2 ]  || (echo "" ; echo "*** The procmeter3-gtk2 program has not been installed (it does not exist)." ; echo "")
	[ ! -f procmeter3-gtk3 ] || install $(STRIP) -m 755 procmeter3-gtk3 $(DESTDIR)$(INSTDIR)/bin
	@[ -f procmeter3-gtk3 ]  || (echo "" ; echo "*** The procmeter3-gtk3 program has not been installed (it does not exist)." ; echo "")
	[ -f procmeter3-gtk3 ]   || [ -f procmeter3-gtk2 ]   || [ ! -f procmeter3-gtk1 ] || ln -sf procmeter3-gtk1 $(DESTDIR)$(INSTDIR)/bin/gprocmeter3
	[ -f procmeter3-gtk3 ]   || [ ! -f procmeter3-gtk2 ] || ln -sf procmeter3-gtk2 $(DESTDIR)$(INSTDIR)/bin/gprocmeter3
	[ ! -f procmeter3-gtk3 ] || ln -sf procmeter3-gtk3 $(DESTDIR)$(INSTDIR)/bin/gprocmeter3
	[ ! -f procmeter3-log ]  || install $(STRIP) -m 755 procmeter3-log $(DESTDIR)$(INSTDIR)/bin
	@[ -f procmeter3-log ]   || (echo "" ; echo "*** The procmeter3-log program has not been installed (it does not exist)." ; echo "")
	[ ! -f procmeter3-lcd ]  || install $(STRIP) -m 755 procmeter3-lcd $(DESTDIR)$(INSTDIR)/bin
	@[ -f procmeter3-lcd ]   || (echo "" ; echo "*** The procmeter3-lcd program has not been installed (it does not exist)." ; echo "")
#
	install -d $(DESTDIR)$(MANDIR)/man1
	install -d $(DESTDIR)$(MANDIR)/man5
	[ ! -f procmeter3-xaw ]  || install -m 644 man/procmeter3-xaw.1  man/procmeter3.1  $(DESTDIR)$(MANDIR)/man1
	[ ! -f procmeter3-log ]  || install -m 644 man/procmeter3-log.1                    $(DESTDIR)$(MANDIR)/man1
	[ ! -f procmeter3-lcd ]  || install -m 644 man/procmeter3-lcd.1                    $(DESTDIR)$(MANDIR)/man1
	[ ! -f procmeter3-gtk1 ] || install -m 644 man/procmeter3-gtk1.1 man/gprocmeter3.1 $(DESTDIR)$(MANDIR)/man1
	[ ! -f procmeter3-gtk2 ] || install -m 644 man/procmeter3-gtk2.1 man/gprocmeter3.1 $(DESTDIR)$(MANDIR)/man1
	[ ! -f procmeter3-gtk3 ] || install -m 644 man/procmeter3-gtk3.1 man/gprocmeter3.1 $(DESTDIR)$(MANDIR)/man1
	install -m 644 man/procmeterrc.5        $(DESTDIR)$(MANDIR)/man5
	install -m 644 man/procmeter3_modules.1 $(DESTDIR)$(MANDIR)/man1
#
	[ ! -f $(DESTDIR)$(RC_PATH)/.procmeterrc ] || mv $(DESTDIR)$(RC_PATH)/.procmeterrc $(DESTDIR)$(RC_PATH)/procmeterrc
	@[ ! -f $(DESTDIR)$(RC_PATH)/procmeterrc ] || (echo "" ; echo "*** The $(DESTDIR)$(RC_PATH)/procmeterrc file has not been installed (it already exists)." ; echo "")
	[ -f $(DESTDIR)$(RC_PATH)/procmeterrc ] || install -m 644 procmeterrc.install $(DESTDIR)$(RC_PATH)/procmeterrc
	install -m 644 procmeterrc.install $(DESTDIR)$(RC_PATH)
#
	install -d $(DESTDIR)$(LIB_PATH)/include
	install -m 644 procmeter.h $(DESTDIR)$(LIB_PATH)/include

install-strip :
	$(MAKE) install STRIP=-s
