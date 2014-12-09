//
//      Simple test program to test the wiringPi functions 
//      for software PWM with GPIO27
//
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

static sig_atomic_t end = 0;
const int GPIO27 = 27;
const int PWM_BaseTiming_uS = 100;
const int PWM_Range = 50;

static void sighandler(int signo){
  end = 1;
}

int main (void) {
	struct sigaction sa;
	int PWMValue;
	float PWM_Freq = 1.0 / (PWM_Range * PWM_BaseTiming_uS * 0.000001);

	printf("Raspberry Pi wiringPi test for soft pwm (press ctrl+c to quit)\n");
	printf("Using %g Hz Base-Frequency\n\n", PWM_Freq);

	if (wiringPiSetupGpio() == -1) {
		printf("wiringPiSetup failed\n\n");
		exit(1) ;
	}

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sighandler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT,&sa, NULL);
	sigaction(SIGTERM,&sa, NULL);

	if (softPwmCreate(GPIO27, 0, PWM_Range) != 0) {
		printf("softPwmCreate failed\n\n");
		exit(2) ;
	}

	int count;
	for (count=0; count<5; count++) { 
		printf("go from off to full light\n");
		for (PWMValue = 0; PWMValue <= PWM_Range; PWMValue++) {
			if (end) {
				break;
			}
			softPwmWrite(GPIO27, PWMValue);
			delay(20);
		}
		printf("go from full light to off\n");
		for (PWMValue = PWM_Range; PWMValue >= 0; PWMValue--) {
			if (end) {
				break;
			}
			softPwmWrite(GPIO27, PWMValue);
			delay(20);
		}
	}

	softPwmWrite(GPIO27, 0);
	delay(100);
	return 0;
}
