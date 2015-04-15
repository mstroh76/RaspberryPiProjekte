//
//    Simple program to test RDM630 RFID reader
//
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

static sig_atomic_t end = 0;
const int cnTagIDSize = 10;
const int cnTagIDSizeByte = 5; 

static void sighandler(int signo) {
  end = 1;
}

unsigned char Get2HexValue(const char* szAscii) {
	char szHex[2+1];
	unsigned int nHex;	

	szHex[2] = '0';
	strncpy(szHex, szAscii, 2);
	sscanf(szHex, "%X", &nHex); //two ascii char to hex value
	return((unsigned char) nHex);
}

int IsChecksumOK(const char* TagID, const char* Checksum){
	char ChecksumValue;
	char ChecksumCalc;
	int count;

	ChecksumCalc = 0;
	for (count=0; count<cnTagIDSizeByte; count++) {
		ChecksumCalc ^= Get2HexValue(&TagID[count*2]); //calc checksum as exor
	}
	ChecksumValue = Get2HexValue(Checksum);

	if (ChecksumCalc == ChecksumValue) { // compare checksum calc and transmitted
		return 1;
	} else {
		return 0;
	}
}

int main(int argc, char** argv) {
	struct termios tio;
	int tty_fd;
	struct sigaction sa;
	char cRS232Sign;
	char TagID[cnTagIDSize+1];
 	char TagIDOld[cnTagIDSize+1];
	char Checksum[2];
	int	nPos;
	fd_set readfs; // file descriptor set
	int maxfd;     // maximum file desciptor in file descriptor set
	struct timeval timeout; // timeout for wait of rs232 input
	int select_result;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sighandler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT,&sa, NULL);
	sigaction(SIGTERM,&sa, NULL);

	printf("RDM630 test program\n");

	memset(&tio, 0, sizeof(tio));
	tio.c_iflag = 0;
	tio.c_oflag = 0;
	//CS8 = 8 bit data.
	//CREAD = Enable receiver.
	tio.c_cflag = CS8|CREAD|CLOCAL; // 8N1, see termios.h for more information
	tio.c_lflag = 0;
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 5;

	const char cszRS232Device = "/dev/ttyAMA0";
	//const char cszRS232Device[] = "/dev/ttyUSB0";

	printf("Open device '%s' 9600, 8n1\n", cszRS232Device);
	tty_fd = open(cszRS232Device, O_RDWR | O_NONBLOCK);
	if (-1 == tty_fd) {
		printf("unable to open device, error '%s' (%d)\n", strerror(errno), errno);
		return EXIT_FAILURE;
	}
	FD_ZERO(&readfs);

	cfsetospeed(&tio, B9600);            // 9600 baud
	cfsetispeed(&tio, B9600);            // 9600 baud

	tcsetattr(tty_fd, TCSANOW, &tio);

	nPos = 0;
	TagID[0] = '\0';
	TagIDOld[0] = '\0';

	do {
		/* set timeout value for rs232 input */
		timeout.tv_usec = 0;  /* milliseconds */
		timeout.tv_sec  = 1;  /* seconds */
		maxfd = tty_fd + 1;
		FD_SET(tty_fd, &readfs);  /* set wait for rs232 input*/

		select_result = select(maxfd, &readfs, NULL, NULL, &timeout);
		if (select_result < 0 ) {
			if (errno != EINTR) { // not signal (Ctrl+c)
				printf("select error %d\n", errno);
			}
		} else if (select_result == 0 ) { // timeout
			if (TagID[0] != '\0') {
				printf("TagID %s gone\n", TagID);
				nPos = 0;
				TagID[0] = '\0';
				TagIDOld[0] = '\0';
			}
		} else {
			if (FD_ISSET(tty_fd, &readfs)) {
				while (read(tty_fd, &cRS232Sign, 1)>0) {
					if (cRS232Sign == 2) { // STX - Start
						nPos = 0;
						TagID[nPos] = '\0';
					} else if (cRS232Sign == 3) { // ETX - Ende
						if (IsChecksumOK(TagID, Checksum)) {
							if (strcmp(TagIDOld, TagID)) {
								printf("TagID %s found\n", TagID);
								strcpy(TagIDOld, TagID);
							}
						} else {
							printf("TagID = %s, Checksum error!\n", TagID);
						}
					} else { //char received
						if (nPos<cnTagIDSize) {
							TagID[nPos] = cRS232Sign;
							TagID[nPos+1] = '\0';
						} else if (nPos<12) {
							Checksum[nPos-cnTagIDSize] = cRS232Sign;
						}
						nPos++;
					}
				}
			}
		}
	} while (!end);

	printf("Close device '%s'\n", cszRS232Device);
	close(tty_fd);

	return EXIT_SUCCESS;
}
