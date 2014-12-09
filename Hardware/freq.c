//
//      Simple test program to test the wiringPi functions 
//      for frequency measurement with GPIO22
//
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

static sig_atomic_t end = 0;
const int GPIO26 = 26;

volatile int nCounter = 0;

//Interrupt Service Routine for GPIO22
void ISR_GPIO26(void) {
	nCounter++;
}

static void sighandler(int signo) {
	end = 1;
}

int main(void) {
	struct sigaction sa;
	double fTime, fFrequency;
	clock_t ClockValue, ClockValueOld;
	int nCount, nCountOld;

	if (wiringPiSetupGpio() == -1) {
		printf("wiringPiSetup failed\n\n");
		exit (1) ;
	}

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sighandler;
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

// INT_EDGE_BOTH, INT_EDGE_FALLING, INT_EDGE_RISING only one ISR per input
	if (wiringPiISR(GPIO26, INT_EDGE_FALLING, &ISR_GPIO26)  < 0) {
		printf("Unable to setup ISR for GPIO %d (%s)\n\n", 
		  GPIO22, strerror(errno));
		exit(1);
	}

	ClockValue = clock();
	while (!end) {
		nCountOld = nCount; 
		ClockValueOld = ClockValue;
		delay(500);
		ClockValue = clock();
		nCount = nCounter;
		fTime = (ClockValue - ClockValueOld) / CLOCKS_PER_SEC;
		fFrequency = (nCount - nCountOld) / CLOCKS_PER_SEC;	
		printf("Time: %.3f Sec, Count: %d, Frequency: %.3f kHz\n", fTime, nCount - nCountOld, fFrequency);
	}

	return 0;
}
