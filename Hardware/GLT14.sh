#!/bin/bash
# mount i2c onewire filesystem
owfs -m /owfs /dev/i2c-1

while true; do
# PWM Fading
./7-seg_param 1
./test_softpwm

# PWM RGB
./7-seg_param 2
./test_rgb_softpwm_sw

# Joypad
./7-seg_param 3
sleep 2
./joypad_7-seg

# DHT22 - Feuchte
./7-seg_param 4
RH=`loldht 1 | tail -n 1| cut -d " " -f 3`
RH=`echo $RH | awk '{printf("%d\n",$1 + 0.5)}'`
./7-seg_dual_param $RH 6

# DHT11 - Feuchte
./7-seg_param 5
RH=`loldht 7 | grep Humidity | cut -d . -f 1 | cut -d " " -f 3`
./7-seg_dual_param $RH 6

#Temperatur
./7-seg_param 6
TEMP=`cat /owfs/28.A5F672020000/temperature  | sed -e 's/^[ \t]*//'`
TEMP=`echo $TEMP | awk '{printf("%d\n",$1 + 0.5)}'`
./7-seg_dual_param $TEMP 6

# Dual 7 Segment
./7-seg_param 7
./7-seg_dual_multiplex
done

