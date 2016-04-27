//
//      Simple test program to test the i2c functions 
//      for MCP port-extender with seven-segment dual display
//		Shows two digits given as parameter
//	
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

static sig_atomic_t end = 0;
//Adresses of MCP IC
const int SlaveAddr     = 0x23;
const int MCP_PortA     = 0x00;
const int MCP_PortB     = 0x10; //IOCON.Bank 1
const int MCP_Direction = 0x00; 
const int MCP_PullUp    = 0x06; //IOCON.Bank 1
const int MCP_Read      = 0x09; //IOCON.Bank 1
const int MCP_Write     = 0x0A; //IOCON.Bank 1
const int MCP_IOCON     = 0x0A; //IOCON.Bank 0

//Outpout for digit deactivation
const int Seg7_Dot_Off   = 0x10;
const int Seg7_Digi1_Off = 0x40;
const int Seg7_Digi2_Off = 0x80;

#define BYTE unsigned char

static void sighandler(int signo){
  end = 1;
}

void I2CWrite(int fd, BYTE Register, BYTE Value) {
	BYTE buf[2];

	buf[0] = Register;
	buf[1] = Value ;
	if (write(fd, buf, 2) != 2) {
		fprintf(stderr, 
		"Failed to write to the i2c bus (%s)\n", strerror(errno));
		return;
	}
}

void MultiplexWrite(int fd, BYTE Val1, BYTE Val2, int nStayTime, int ShowTime) {
	int nTime = 0;
	while (nTime < ShowTime) {
		I2CWrite(fd, MCP_PortA | MCP_Write, Val1 | Seg7_Digi2_Off | Seg7_Dot_Off);
		usleep(nStayTime);
		I2CWrite(fd, MCP_PortA | MCP_Write, Val2 | Seg7_Digi1_Off | Seg7_Dot_Off);
		usleep(nStayTime);
		nTime = nTime + (2*nStayTime);
	}
}

int main(int argc, char* argv[]) {
	int fd;
	char device[20];
	BYTE Value1, Value2;
	int count;
	int ShowTime_us = 3000000;
	
	Value1 = Value2 = -1 ;
	if (argc >= 2 ) {
		if ( argv[1][0] >= '0' && argv[1][0] <= '9') {
			Value1 = argv[1][0]-'0';
		}
		if ( 0 == argv[1][1] ) { // only single digit
			Value2 = Value1;
			Value1 = 0;			
		} else if ( argv[1][1] >= '0' && argv[1][1] <= '9') {
			Value2 = argv[1][1]-'0';
		}
	}
	if (Value1 > 9 || Value1 < 0) {
		printf("Parameter 1, digit 1 invalid\n");
		exit(1);
 	}
	if (Value2 > 9 || Value2 < 0) {
		printf("Parameter 1, digit 2 invalid\n");
		exit(2);
	}
	if (argc >= 3) {
		ShowTime_us = atoi(argv[2])*1000000;
		if (ShowTime_us <=0 ) {
			ShowTime_us = 1*1000000;
		}
	}
	printf("Time = %d us\n", ShowTime_us);
	printf("Value1=%d, Value2=%d\n", Value1, Value2);

	printf("open device...\n");
	sprintf(device, "/dev/i2c-1");
	if ((fd = open(device, O_RDWR)) < 0) {
		fprintf(stderr,"Failed to open i2c bus '%s'\n", device);
		exit(3);
	}
	printf("set slave ...\n");
	if (ioctl(fd, I2C_SLAVE, SlaveAddr) < 0) {
		fprintf(stderr,
		"Failed to acquire i2c bus access or talk to slave %X\n", SlaveAddr);
		close(fd);
		exit(4);
	}
	printf("switch to iocon.bank 1 ...\n");
	I2CWrite(fd, MCP_IOCON, 0x80);	
	printf("write direction output ...\n");
	I2CWrite(fd, MCP_PortA | MCP_Direction, 0x00);	

	printf("Write = %d %d \n", Value1,  Value2);
	MultiplexWrite(fd, Value1, Value2, 10000, ShowTime_us);
	I2CWrite(fd, MCP_PortA | MCP_Write, Seg7_Digi1_Off | Seg7_Digi2_Off);

	close(fd);
	return 0;
}
