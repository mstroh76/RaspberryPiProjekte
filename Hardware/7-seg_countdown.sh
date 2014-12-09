#!/bin/bash
# Sample program for Seven-segment display
# Countdown 3 -> 1 
SLEEPTIME=1
IC_ADRESS=0x27

# Set GPA to Output
i2cset -y 1 $IC_ADRESS 0x00 0x00
# read back register
i2cget -y 1 $IC_ADRESS 0x00 > /dev/null

# Write 3
echo 3
i2cset -y 1 $IC_ADRESS 0x0A 0x19
sleep $SLEEPTIME
# Write 2
echo 2
i2cset -y 1 $IC_ADRESS 0x0A 0x1C
sleep $SLEEPTIME
# Write 1
echo 1
i2cset -y 1 $IC_ADRESS 0x0A 0x7B
sleep $SLEEPTIME
# Write 0
#i2cset -y 1 $IC_ADRESS 0x0A 0x28
#sleep $SLEEPTIME

# Write all off
echo off
i2cset -y 1 $IC_ADRESS 0x0A 0xFF
sleep $SLEEPTIME
