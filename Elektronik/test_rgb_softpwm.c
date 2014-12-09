//
//  Simple program to test the wiringPi functions 
//  for software PWM with GPIO23, GPIO24 and GPIO27
//	connected to a RGB - LED
//
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

static sig_atomic_t end = 0;
const int PWM_BaseTiming_uS = 100;
const int PWM_Range = 64;
const int GPIO23 = 23;
const int GPIO24 = 24;
const int GPIO27 = 27;
int LED_RGB[3]   = { 23, 24, 27 };
const int RED    = 0;
const int GREEN  = 1;
const int BLUE   = 2;

static void sighandler(int signo){
	end = 1;
}

int main (void) {
	struct sigaction sa;
	int PWMValue[3];
	float PWM_Freq = 1.0 / (PWM_Range * PWM_BaseTiming_uS * 0.000001);
	int LedCount;

	printf("Raspberry Pi wiringPi test for rgb soft pwm (press ctrl+c to quit)\n");
	printf("Using %g Hz Base-Frequency\n", PWM_Freq);

	if (wiringPiSetupGpio() == -1){
		printf("wiringPiSetup failed\n\n");
		exit (1) ;
	}

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sighandler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT,&sa, NULL);
	sigaction(SIGTERM,&sa, NULL);

	for (LedCount=0; LedCount < 3; LedCount++) {
	  if (softPwmCreate(LED_RGB[LedCount], 0, PWM_Range) != 0) {
	  	printf("softPwmCreate failed\n\n");
	  	exit (1) ;
	  }
	}

	for (LedCount=0; LedCount<3; LedCount++) {
		softPwmWrite(LED_RGB[LedCount], 0);
	}
	for (PWMValue[RED]=1; PWMValue[RED]<=PWM_Range; PWMValue[RED]*=2) {
		if (end) {
			break;
		}	
		softPwmWrite(LED_RGB[RED], PWMValue[RED]);
		printf("Red=%d%%\n", PWMValue[RED]*100/PWM_Range);
		delay(1000);
	}
	softPwmWrite(LED_RGB[RED], 0);
	for (PWMValue[GREEN]=1; PWMValue[GREEN]<=PWM_Range; PWMValue[GREEN]*=2) {
		if (end) {
			break;
		}
		softPwmWrite(LED_RGB[GREEN], PWMValue[GREEN]);
		printf("Green=%d%%\n", PWMValue[GREEN]*100/PWM_Range);
		delay(1000);
	}
	softPwmWrite(LED_RGB[GREEN], 0);
	for (PWMValue[BLUE]=1; PWMValue[BLUE]<=PWM_Range; PWMValue[BLUE]*=2) {
		if (end) {
			break;
		}	
		softPwmWrite(LED_RGB[BLUE], PWMValue[BLUE]);
		printf("Blue=%d%%\n", PWMValue[BLUE]*100/PWM_Range);
		delay(1000);
	}
	softPwmWrite(LED_RGB[BLUE], 0);

	while (!end) {
		for (PWMValue[RED] = 1; PWMValue[RED] <= PWM_Range; PWMValue[RED]*=2) {
			if (end) {
				break;
			}
			softPwmWrite(LED_RGB[RED], PWMValue[RED]);
			for (PWMValue[GREEN]=1; PWMValue[GREEN]<=PWM_Range; PWMValue[GREEN]*=2) {
				if (end) {
					break;
				}
				softPwmWrite(LED_RGB[GREEN], PWMValue[GREEN]);
				for (PWMValue[BLUE]=1; PWMValue[BLUE]<=PWM_Range; PWMValue[BLUE]*=2) {
					if (end) {
						break;
					}
					softPwmWrite(LED_RGB[BLUE], PWMValue[BLUE]);
					printf("Red=%g%%, Green=%g%%, Blue=%g%%\n",
					 PWMValue[RED]*100.0/PWM_Range,
					 PWMValue[GREEN]*100.0/PWM_Range,
					 PWMValue[BLUE]*100.0/PWM_Range);
					delay(250);
				}
			}
		}
	}

	for (LedCount=0; LedCount < 3; LedCount++) {
		softPwmWrite(LED_RGB[LedCount], 0);
	}
	delay(100);

 return 0;
}
