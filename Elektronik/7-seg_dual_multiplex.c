//
//      Simple test program to test the i2c functions 
//      for MCP port-extender with seven-segment dual display.
//      Shows how multiplexing the digits is working
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
const int SlaveAddr     = 0x23;
const int MCP_PortA     = 0x00;
const int MCP_PortB     = 0x10; //IOCON.Bank 1
const int MCP_Direction = 0x00; 
const int MCP_PullUp    = 0x06; //IOCON.Bank 1
const int MCP_Read      = 0x09; //IOCON.Bank 1
const int MCP_Write     = 0x0A; //IOCON.Bank 1
const int MCP_IOCON     = 0x0A; //IOCON.Bank 0

const int Seg7_Dot_Off       = 0x10;
const int Seg7_Digi1_Off     = 0x40;
const int Seg7_Digi2_Off     = 0x80;

#define BYTE unsigned char

static void sighandler(int signo){
  end = 1;
}

void I2CWrite(int fd, BYTE Register, BYTE Value){
	BYTE buf[2];

	buf[0] = Register;
	buf[1] = Value ;
	if (write(fd, buf, 2) != 2) {
		fprintf(stderr, 
		"Failed to write to the i2c bus (%s)\n", strerror(errno));
		return;
	}
}

void MultiplexWrite(int fd, BYTE Value1, BYTE Value2, int nStayTime, int ShowTime){
	int nTime = 0;
	while (nTime < ShowTime) {
		I2CWrite(fd, MCP_PortA | MCP_Write, Value1 | Seg7_Digi2_Off | Seg7_Dot_Off);
		usleep(nStayTime);
		I2CWrite(fd, MCP_PortA | MCP_Write, Value2 | Seg7_Digi1_Off | Seg7_Dot_Off);
		usleep(nStayTime);
		nTime = nTime + (2*nStayTime);
	}
}


int main(void) {
	int fd;
	char device[20];
	int count;
	
	printf("open device...\n");
	sprintf(device, "/dev/i2c-1");
	if ((fd = open(device, O_RDWR)) < 0) {
		fprintf(stderr, "Failed to open i2c bus '%s'\n", device);
        exit(1);
	}
	printf("set slave ...\n");
    if (ioctl(fd, I2C_SLAVE, SlaveAddr) < 0) {
		fprintf(stderr,
		"Failed to acquire i2c bus access or talk to slave %X\n", SlaveAddr);
		close(fd);
		exit(2);
	}

	printf("switch to iocon.bank 1 ...\n");
	I2CWrite(fd, MCP_IOCON, 0x80);	
	printf("start output dark...\n");
	I2CWrite(fd, MCP_PortA | MCP_Write, Seg7_Digi1_Off | Seg7_Digi2_Off);

	printf("write direction output ...\n");
	I2CWrite(fd, MCP_PortA | MCP_Direction, 0x00);

	printf("count up both digits ...\n");
	for (count=0; count<=9; count++){
		I2CWrite(fd, MCP_PortA | MCP_Write, count | Seg7_Dot_Off);
		usleep(300000);
	}
	printf("count up digit 2 ...\n");
	for (count=0; count<=9; count++){
		I2CWrite(fd, MCP_PortA | MCP_Write, count | Seg7_Digi1_Off | Seg7_Dot_Off);
		usleep(300000);
	}
	printf("count up digit 1 ...\n");
	for (count=0; count<=9; count++){
		I2CWrite(fd, MCP_PortA | MCP_Write, count | Seg7_Digi2_Off | Seg7_Dot_Off);
		usleep(300000);
	}
	printf("show 04 Hz ...\n");
	MultiplexWrite(fd, 0, 4, 250000, 3000000);
	printf("show 10 Hz ...\n");
	MultiplexWrite(fd, 1, 0, 100000, 3000000);
	printf("show 21 Hz ...\n");
	MultiplexWrite(fd, 2, 1, 47600,  3000000);
	printf("show 43 Hz ...\n");
	MultiplexWrite(fd, 4, 3, 23200,  3000000);
	printf("show 98 Hz ...\n");
	MultiplexWrite(fd, 9, 8, 10200,  3000000);
	printf("switch off both digits ...\n");

	I2CWrite(fd, MCP_PortA | MCP_Write, Seg7_Digi1_Off | Seg7_Digi2_Off);	
	close(fd);
	return 0;
}
