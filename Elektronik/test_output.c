//
//      Simple test program to test the wiringPi functions 
//      for GPIO23 und GPIO24 output
//
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

static sig_atomic_t end = 0;
const int GPIO23=23;
const int GPIO24=24;

static void sighandler(int signo){
  end = 1;
}

int main (void){
  struct sigaction sa;

  printf("Raspberry Pi wiringPi test program (press ctrl+c to quit)\n");

  if (wiringPiSetupGpio() == -1){
    printf("wiringPiSetup failed\n\n");
    exit (1) ;
  }

  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sighandler;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT,&sa, NULL);
  sigaction(SIGTERM,&sa, NULL);

  pinMode(GPIO23, OUTPUT);
  pinMode(GPIO24, OUTPUT);

  delay(500);

  printf("all off\n");
  digitalWrite(GPIO23, LOW);
  digitalWrite(GPIO24, LOW);
  delay(1000);
  printf("all on\n");
  digitalWrite(GPIO23, HIGH);
  digitalWrite(GPIO24, HIGH);
  delay(1000);

  printf("blinking...\n");
  while(!end){
    digitalWrite(GPIO23, HIGH);
    digitalWrite(GPIO24, LOW);
    delay(1000);
    digitalWrite(GPIO23, LOW);
    digitalWrite(GPIO24, HIGH);
    delay(1000);
  }
  printf("\nall off\n");
  digitalWrite(GPIO23, LOW);
  digitalWrite(GPIO24, LOW);

  return 0;
}