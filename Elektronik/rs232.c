//
//    Simple program to test RS232 functions
//
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>


static sig_atomic_t end = 0;

static void sighandler(int signo) {
  end = 1;
}

int main(int argc, char** argv) {
	struct termios tio;
	int tty_fd;
	struct sigaction sa;
	unsigned char c;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sighandler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	printf("RS232 echo program\n\n");

	memset(&tio, 0, sizeof(tio));
	tio.c_iflag = 0;
	tio.c_oflag = 0;
	//CS8 = 8 bit data.
	//CREAD = Enable receiver.
	//CLOCAL = Ignore modem status lines.
	//PARENB|PARODD = Parity odd enable.
	tio.c_cflag = CS8|PARENB|PARODD|CREAD|CLOCAL;// see termios.h
	tio.c_lflag = 0;
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 5;

	printf("Open device /dev/ttyAMA0 9600, 8o1\n");
	tty_fd = open("/dev/ttyAMA0", O_RDWR | O_NONBLOCK);
	if (-1 == tty_fd) {
		printf("unable to open device, error '%s' (%d)\n", strerror(errno), errno);
		return EXIT_FAILURE;
	}
	cfsetospeed(&tio, B9600);  // output 9600 baud 
	cfsetispeed(&tio, B9600);  // input 9600 baud
	tcsetattr(tty_fd, TCSANOW, &tio); //TCSANOW = the change occurs immediately
	
	do {
		if (read(tty_fd, &c, 1) > 0) { // read from rs232
			// if new data is available on the serial port send it back
			write(STDOUT_FILENO, &c, 1);              
			if (c != 'q') {
				write(tty_fd, &c, 1); // write to rs232
			} else {
				write(tty_fd, "\nquit\n", 6); // write to rs232
			}
		} else {
			usleep(10000);
		}
	} while (c!='q' && !end);

	printf("Close device /dev/ttyAMA0\n");
	close(tty_fd);

	return EXIT_SUCCESS;
}
