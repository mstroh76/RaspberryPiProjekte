#!/bin/bash
CNT_ADDRESS=0x50

SENSH=`cat $1 | head -n 1 | cut -b 3-4 | tr '[:lower:]' '[:upper:]'`
SENS=`echo "obase=10; ibase=16; $SENSH" | bc`
echo "Sens: 0x$SENSH (${SENS})"

OFFSETH=`cat $1 | tail -n 1 | cut -b 3-6 | tr '[:lower:]' '[:upper:]'`
OFFSET=`echo "obase=10; ibase=16; $OFFSETH" | bc`
echo "Offset: 0x$OFFSETH (${OFFSET})"

FREQ10=`i2cset -y 1 $CNT_ADDRESS 0x00 0xA0 &&
i2cset -y 1 $CNT_ADDRESS 0x03 0x00 &&
i2cset -y 1 $CNT_ADDRESS 0x02 0x00 &&
i2cset -y 1 $CNT_ADDRESS 0x01 0x00 &&
i2cset -y 1 $CNT_ADDRESS 0x00 0x22 &&
sleep 10 &&
i2cset -y 1 $CNT_ADDRESS 0x00 0xA0 &&
i2cget -y 1 $CNT_ADDRESS 0x03 | cut -b 3-4 | tr -d '\n' &&
i2cget -y 1 $CNT_ADDRESS 0x02 | cut -b 3-4 | tr -d '\n' &&
i2cget -y 1 $CNT_ADDRESS 0x01 | cut -b 3-4 | tr -d '\n'`
FREQ=`echo "${FREQ10}/10" | bc`
echo "Frequency: $FREQ Hz"

RH=`echo "scale=3; ($OFFSET-$FREQ)*${SENS}/2^10*100" | bc`
echo "Realtiv humidity=$RH %"

