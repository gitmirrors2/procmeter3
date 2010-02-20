#!/bin/sh

CC=$1
CFLAGS=$2

cat <<EOF > libsensors-test.c
#include <sensors/sensors.h>
#if (SENSORS_API_VERSION & 0xf00) != 0x400
#error
#endif
EOF

$CC -c $CFLAGS libsensors-test.c -o libsensors-test.o 2> /dev/null

rm libsensors-test.c

if [ -f libsensors-test.o ]; then
    rm libsensors-test.o
    exit 1
else
    echo "libsensors does not appear to be installed - skipping compilation."
    exit 0
fi
