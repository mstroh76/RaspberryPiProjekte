//
//      Simple test program to test the wiringPi functions 
//      for GPIO22 input
//
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

static sig_atomic_t end = 0;
const int GPIO22 = 22;
const char ButtonState[2][10] = {"released", "pressed"};


volatile int nCounterChanged = 0;
volatile int nCounterEdgeDown= 0;
volatile int nCounterEdgeUp  = 0;
volatile int nButtonState  = 0;

//Interrupt Service Routine for GPIO22
void ISR_GPIO22(void) {
	nCounterChanged++;
	nButtonState = digitalRead(GPIO22);
	if (HIGH ==  nButtonState) {
			nCounterEdgeUp++;
	} else {
			nCounterEdgeDown++;
	}
}


static void sighandler(int signo){
	end = 1;
}


int main(void) {
	struct sigaction sa;

	if (wiringPiSetupGpio() == -1) {
		printf("wiringPiSetup failed\n\n");
		exit (1) ;
	}

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sighandler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT,&sa, NULL);
	sigaction(SIGTERM,&sa, NULL);

// INT_EDGE_BOTH, INT_EDGE_FALLING, INT_EDGE_RISING only one ISR per input
	if (wiringPiISR(GPIO22, INT_EDGE_BOTH, &ISR_GPIO22)  < 0) {
		printf("Unable to setup ISR for GPIO %d (%s)\n\n", 
		  GPIO22, strerror(errno));
		exit(1);
	}

	while(!end) {
		printf("Changed: %d, Down: %d, Up: %d, Button: %s\n",
		  nCounterChanged, nCounterEdgeDown, nCounterEdgeUp, 
		  ButtonState[nButtonState]);
		delay(500);
	}

	return 0;
}
