#!/bin/bash
HEXVALUE=`i2cget -y 1 0x48 0x00 w`
MSB=`echo $HEXVALUE | cut -b 5-6`
LSB=`echo $HEXVALUE | cut -b 3-4`

T=`echo "ibase=16; $MSB"|bc`
if [ "$LSB" -eq "80" ]; then
	T=`echo "$T+0.5"|bc`
fi
echo $T C