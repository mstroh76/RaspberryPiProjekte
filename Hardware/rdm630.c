//
//      Simple program read Tag id from RDM630 
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
	unsigned char TagID[10+1];
	unsigned char szHex[2+1];
	unsigned char TagValue[5];
	unsigned char ChecksumCalc;
	unsigned char Checksum[2];
	unsigned char ChecksumValue;
	int nPos, count;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sighandler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT,&sa, NULL);
	sigaction(SIGTERM,&sa, NULL);

	printf("Start RS232 echo program\n");

	memset(&tio, 0, sizeof(tio));
	tio.c_iflag=0;
	tio.c_oflag=0;
	//CS8 = 8 bit data.
	//CREAD = Enable receiver.
	tio.c_cflag=CS8|CREAD|CLOCAL;           // 8N1, see termios.h for more information
	tio.c_lflag=0;
	tio.c_cc[VMIN]=1;
	tio.c_cc[VTIME]=5;

	printf("Open device /dev/ttyAMA 9600, 8o1\n");
	tty_fd=open("/dev/ttyAMA0", O_RDWR | O_NONBLOCK);
	if (-1 == tty_fd) {
		printf("unable to open device, error '%s' (%d)\n", strerror(errno), errno);
		return EXIT_FAILURE;
	}
	cfsetospeed(&tio, B9600);            // 9600 baud
	cfsetispeed(&tio, B9600);            // 9600 baud

	tcsetattr(tty_fd,TCSANOW,&tio);

	nPos=0;
	do {
		if (read(tty_fd, &c, 1) > 0) {
			if (c==2) {
				nPos=0;
				TagID[nPos]='\0';
			} else if (c==3) {
				ChecksumCalc = 0;
				szHex[2] = '\0';
				for (count=0; count<=4; count++) {
					strncpy(szHex, &TagID[count*2], 2);
					sscanf(szHex, "%X", &TagValue[count]);
					ChecksumCalc ^= TagValue[count];
				}
				strncpy(szHex, Checksum, 2);
				sscanf(szHex, "%X", &ChecksumValue);
				if (ChecksumCalc == ChecksumValue) {
					printf("TagID = %s\n", TagID);
				} else {
					printf("TagID = %s, Checksum error! calc = 0x%x, found = 0x%x \n", 
					  TagID, ChecksumCalc, ChecksumValue);
				}
			} else {
				if (nPos<10) {
					TagID[nPos]=c;
					TagID[nPos+1]='\0';
				} else if (nPos<12) {
					Checksum[nPos-10]=c;
				}
				nPos++;
			}
		}
	} while (!end);

	printf("Close device /dev/ttyAMA\n");
	close(tty_fd);

	return EXIT_SUCCESS;
}