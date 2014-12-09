#!/bin/bash
# MCLK 32 KHz, 50 % duty cycle, Raspebrry Pi PWM 
gpio pwm 1 0
gpio mode 1 pwm
gpio pwmc 300
gpio pwm 1 512
gpio mode 0 out
# XCLR High
gpio write 0 1
./Pressure $1
# XCLR Low
gpio write 0 0
# MCLK off (Low)
gpio pwm 1 0