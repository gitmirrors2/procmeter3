#!/bin/sh

for prog in gtk1 gtk2 xaw log lcd; do

    make distclean

    make -k procmeter3-$prog

    make install DESTDIR=/tmp/procmeter3-$prog

done
