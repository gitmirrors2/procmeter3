#!/bin/sh

CC=$1
CFLAGS=$2
libsensorstest=libsensorstest-$$

cat <<EOF > $libsensorstest.c
#include <sensors/sensors.h>
#if ( (SENSORS_API_VERSION & 0xf00) != 0x400 ) && ( (SENSORS_API_VERSION & 0xf00) != 0x500 )
#error
#endif
EOF

$CC -c $CFLAGS $libsensorstest.c -o $libsensorstest.o 2> /dev/null

rm $libsensorstest.c

if [ -f $libsensorstest.o ]; then
    rm $libsensorstest.o
    exit 1
else
    echo "libsensors does not appear to be installed - skipping compilation."
    exit 0
fi
