//
//      Simple test program to test the i2c functions 
//      for MCP port-extender with seven-segment display
//		
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

static sig_atomic_t end = 0;

static void sighandler(int signo) {
  end = 1;
}

void I2CWrite(int fd, unsigned char Register, unsigned char Value) {
	unsigned char buf[2];

	buf[0] = Register;
	buf[1] = Value ;
	if (write(fd, buf, 2) != 2) {
		fprintf(stderr, 
		"Failed to write to the i2c bus (%s)\n", strerror(errno));
		return;
	}
}

int main(int argc, char* argv[]) {
	int fd;
	char device[20];
	//Adresses of MCP IC
	const int SlaveAddr     = 0x27;
	const int MCP_PortA     = 0x00;
	const int MCP_PortB     = 0x10; //IOCON.Bank 1
	const int MCP_Direction = 0x00; 
	const int MCP_PullUp    = 0x06; //IOCON.Bank 1
	const int MCP_Read      = 0x09; //IOCON.Bank 1
	const int MCP_Write     = 0x0A; //IOCON.Bank 1
	const int MCP_IOCON     = 0x0A; //IOCON.Bank 0

	//Define Output pin to segment of display
	//(this has to match with wiring)
	#define Seg7_a 0x40
	#define Seg7_b 0x80
	#define Seg7_c 0x04
	#define Seg7_d 0x02
	#define Seg7_e 0x01
	#define Seg7_f 0x10
	#define Seg7_g 0x20
	#define Seg7_dot 0x08
	//Define active segment for numbers
	#define CBYTE const unsigned char
	CBYTE Seg7_0 = Seg7_a | Seg7_b | Seg7_c | Seg7_d | Seg7_e | Seg7_f;
	CBYTE Seg7_1 = Seg7_b | Seg7_c;
	CBYTE Seg7_2 = Seg7_a | Seg7_b | Seg7_g | Seg7_e | Seg7_d;
 	CBYTE Seg7_3 = Seg7_a | Seg7_b | Seg7_g | Seg7_c | Seg7_d;
	CBYTE Seg7_4 = Seg7_f | Seg7_b | Seg7_g | Seg7_c;
	CBYTE Seg7_5 = Seg7_a | Seg7_f | Seg7_g | Seg7_c | Seg7_d;
	CBYTE Seg7_6 = Seg7_a | Seg7_f | Seg7_e | Seg7_d | Seg7_c | Seg7_g;
	CBYTE Seg7_7 = Seg7_a | Seg7_b | Seg7_c;
 	CBYTE Seg7_8 = Seg7_a | Seg7_b | Seg7_c | Seg7_d | Seg7_e | Seg7_f | Seg7_g;
	CBYTE Seg7_9 = Seg7_g | Seg7_f | Seg7_a | Seg7_b | Seg7_c;

	//Store segmentcoding to a array and invert for 'active-low'
	unsigned char Segment7[10];
	Segment7[0] = ~Seg7_0;
	Segment7[1] = ~Seg7_1;
	Segment7[2] = ~Seg7_2;
	Segment7[3] = ~Seg7_3;
	Segment7[4] = ~Seg7_4;
	Segment7[5] = ~Seg7_5;
	Segment7[6] = ~Seg7_6;
	Segment7[7] = ~Seg7_7;
	Segment7[8] = ~Seg7_8;
	Segment7[9] = ~Seg7_9;

	int value = -1;	
	if (argc >= 2) {
		 value = atoi(argv[1]);
	}
	if (value>9 || value<0) {
		printf("Parameter 1 invalid\n");
		exit(1);
	}
	printf("open device...\n");
	sprintf(device, "/dev/i2c-1");
	if ((fd = open(device, O_RDWR)) < 0) {
		fprintf(stderr,"Failed to open i2c bus '%s'\n", device);
		exit(2);
    }
	printf("set slave ...\n");
    if (ioctl(fd, I2C_SLAVE, SlaveAddr) < 0) {
		fprintf(stderr,
		"Failed to acquire i2c bus access or talk to slave %X\n", SlaveAddr);
		close(fd);
		exit(3);
	}
	printf("switch to iocon.bank 1 ...\n");
	I2CWrite(fd, MCP_IOCON, 0x80);
	printf("write direction output ...\n");
	I2CWrite(fd, MCP_PortA | MCP_Direction, 0x00);
	printf("write output 0xFF ...\n");
	I2CWrite(fd, MCP_PortA | MCP_Write, 0xFF);
	printf("write output Seven-segment display %d\n", value);
	I2CWrite(fd, MCP_PortA | MCP_Write, Segment7[value]);

	close(fd);
	return 0;
}
