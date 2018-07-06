//
//   measure rpm of wheel with encoder on GPIO 7 and 8       
//
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

static sig_atomic_t end = 0;
const int GPIO_LEFT  = 8;
const int GPIO_RIGHT = 7;
const int PulsPerRotation = 20;
const float circfer = 0.216; //meter

volatile int nCounterLeft = 0;
volatile int nCounterRight = 0;

//Interrupt Service Routine for GPIOs
void ISR_GPIO_LEFT(void) {
	nCounterLeft++;
}
void ISR_GPIO_RIGHT(void) {
	nCounterRight++;
}

static void sighandler(int signo) {
	end = 1;
}

int main(void) {
	struct sigaction sa;
	double fTime, fFrequencyLeft, fFrequencyRight;
	struct timeval TimeValue, TimeValueOld;
	int nCountLeft, nCountLeftOld;
	int nCountRight, nCountRightOld;

	nCountLeft = 0;
	nCountRight = 0;
	TimeValue.tv_sec = 0;
	TimeValue.tv_usec = 0;
	if (wiringPiSetupGpio() == -1) {
		printf("wiringPiSetup failed\n\n");
		exit (EXIT_FAILURE) ;
	}

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sighandler;
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

// INT_EDGE_BOTH, INT_EDGE_FALLING, INT_EDGE_RISING only one ISR per input
	if (wiringPiISR(GPIO_LEFT, INT_EDGE_FALLING, &ISR_GPIO_LEFT)  < 0) {
		printf("Unable to setup ISR for GPIO %d (%s)\n\n", 
		  GPIO_LEFT, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (wiringPiISR(GPIO_RIGHT, INT_EDGE_FALLING, &ISR_GPIO_RIGHT)  < 0) {
		printf("Unable to setup ISR for GPIO %d (%s)\n\n", 
		  GPIO_LEFT, strerror(errno));
		exit(EXIT_FAILURE);
	}

	while (!end) {
		nCountLeftOld = nCountLeft;
		nCountRightOld = nCountRight;
		TimeValueOld = TimeValue;
		delay(1000);
		gettimeofday(&TimeValue, 0);
		nCountLeft = nCounterLeft;
		nCountRight = nCounterRight;
		if (0==TimeValueOld.tv_sec && 0==TimeValueOld.tv_usec) {
			continue;
		}
		fTime = (TimeValue.tv_sec - TimeValueOld.tv_sec) + 
		 (TimeValue.tv_usec - TimeValueOld.tv_usec)/1000000.0;
		fFrequencyLeft = (double)(nCountLeft - nCountLeftOld) / fTime;
		fFrequencyRight = (double)(nCountRight - nCountRightOld) / fTime;	
		printf("IntCount: %d/%d, Freq.: %.1f/%.1f Hz, Upm %.0f/%.0f, dis.: %.1f/%.1fm\n",
		 nCountLeft - nCountLeftOld, nCountRight - nCountRightOld , fFrequencyLeft, 
		 fFrequencyRight, fFrequencyLeft*60.0/PulsPerRotation, fFrequencyRight*60.0/PulsPerRotation,
		 nCounterLeft*circfer/PulsPerRotation, nCounterRight*circfer/PulsPerRotation );
	}

	return(EXIT_SUCCESS);
}
