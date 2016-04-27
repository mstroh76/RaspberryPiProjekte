//
//      Simple test program to test the i2c functions 
//      for MCP port-extender with sega megadrive/genesis
//		controller and seven-segment display
//
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define false 0
#define true 1

static sig_atomic_t end = 0;

static void sighandler(int signo) {
  end = 1;
}

int I2CWrite(int fd, unsigned char Register, unsigned char Value) {
	unsigned char buf[2];

	buf[0] = Register;
	buf[1] = Value;
	if (write(fd, buf, 2) != 2) {
		fprintf(stderr, 
		"Failed to write to the i2c bus (%s)\n", strerror(errno));
		return false;
	}
	return true;
}

int I2CRead(int fd, unsigned char Register, unsigned char* Value) {
	unsigned char Value1 = 0;
	if (write(fd, &Register, 1) != 1) {
		fprintf(stderr,
		"Failed to write to the i2c bus (%s)\n", strerror(errno));
		return false;
	}

	if (read(fd, &Value1, 1) != 1) {
		fprintf(stderr, 
		"Failed to read from the i2c bus (%s)\n",strerror(errno));
		return false;
	} 
//	printf("read %d\n", (int)Value1);
	*Value = Value1;
	return true;
}

int main(void) {
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
	CBYTE Seg7_0  = Seg7_a | Seg7_b | Seg7_c | Seg7_d | Seg7_e | Seg7_f;
	CBYTE Seg7_1  = Seg7_b | Seg7_c;
	CBYTE Seg7_2  = Seg7_a | Seg7_b | Seg7_g | Seg7_e | Seg7_d;
 	CBYTE Seg7_3  = Seg7_a | Seg7_b | Seg7_g | Seg7_c | Seg7_d;
	CBYTE Seg7_4  = Seg7_f | Seg7_b | Seg7_g | Seg7_c;
	CBYTE Seg7_5  = Seg7_a | Seg7_f | Seg7_g | Seg7_c | Seg7_d;
	CBYTE Seg7_6  = Seg7_a | Seg7_f | Seg7_e | Seg7_d | Seg7_c | Seg7_g;
	CBYTE Seg7_7  = Seg7_a | Seg7_b | Seg7_c;
 	CBYTE Seg7_8  = Seg7_a | Seg7_b | Seg7_c | Seg7_d | Seg7_e | Seg7_f | Seg7_g;
	CBYTE Seg7_9  = Seg7_g | Seg7_f | Seg7_a | Seg7_b | Seg7_c;	
	CBYTE Seg7__E = Seg7_a | Seg7_f | Seg7_g | Seg7_e | Seg7_d;
	CBYTE Seg7__U = Seg7_f | Seg7_e | Seg7_d | Seg7_c | Seg7_b;
	CBYTE Seg7__d = Seg7_b | Seg7_c | Seg7_d | Seg7_e | Seg7_g;
	CBYTE Seg7__r = Seg7_e | Seg7_g;
	CBYTE Seg7__L = Seg7_f | Seg7_e | Seg7_d;
	CBYTE Seg7__C = Seg7_a | Seg7_f | Seg7_e | Seg7_d;
	CBYTE Seg7__c = Seg7_g | Seg7_e | Seg7_d;
	CBYTE Seg7__S = Seg7_a | Seg7_f | Seg7_g | Seg7_c | Seg7_d;
	CBYTE Seg7_Off = 0x00;
	
	printf("open device...\n");
	sprintf(device, "/dev/i2c-1");
	if ((fd = open(device, O_RDWR)) < 0) {
		fprintf(stderr,"Failed to open i2c bus '%s'\n", device);
        exit(1);
	}
	printf("set slave ...\n");
    if (ioctl(fd, I2C_SLAVE, SlaveAddr) < 0) {
		fprintf(stderr,
		"Failed to acquire i2c bus access or talk to slave %X\n",SlaveAddr);
		close(fd);
		exit(2);
	}
	
	printf("switch to iocon.bank 1 ...\n");
	I2CWrite(fd, MCP_IOCON, 0x80);	
	printf("write portA direction output ...\n");
	I2CWrite(fd, MCP_PortA | MCP_Direction, 0x00);
	printf("write portB direction input 1-7, output 8 ...\n");
	I2CWrite(fd, MCP_PortB | MCP_Direction, 0x7F);

	printf("test portB output ...\n");
	I2CWrite(fd, MCP_PortB | MCP_Write, 0x80);
	sleep(2);
	I2CWrite(fd, MCP_PortB | MCP_Write, 0x00);
	sleep(2);
	I2CWrite(fd, MCP_PortB | MCP_Write, 0x80);

	printf("test portA output ...\n");
	I2CWrite(fd, MCP_PortA | MCP_Write, 0xFF);
	sleep(2);
	I2CWrite(fd, MCP_PortA | MCP_Write, 0x00);
	sleep(2);
	I2CWrite(fd, MCP_PortA | MCP_Write, 0xFF);

	unsigned char Value;
	signed char ButtonSelect = 0;
	int loop;
	for (loop=1; loop<=240; loop++) {
		if (end) break;
		if (loop%2 == 0) {
			printf("write output select high (0x80) ...\n");
			I2CWrite(fd, MCP_PortB | MCP_Write, 0x80);
		} else {
			printf("write output select low (0x00) ...\n");
			I2CWrite(fd, MCP_PortB | MCP_Write, 0x00);
		}
		usleep(50000);
		if (I2CRead(fd,  MCP_PortB | MCP_Read, &Value )) {
			printf("read output 0x%x ...\n",(int)Value);
			if (!(Value & 0x01)) {
				printf("Button: Up\n");
				I2CWrite(fd, MCP_PortA | MCP_Write, ~Seg7__U);
				ButtonSelect = 2;
			} else if (!(Value & 0x02)) {
				printf("Button: Down\n");
				I2CWrite(fd, MCP_PortA | MCP_Write, ~Seg7__d);
				ButtonSelect = 2;
			} else if (Value & 0x80) { //select high
				if (!(Value & 0x04)) {
					printf("Button: Left\n");
					I2CWrite(fd, MCP_PortA | MCP_Write, ~Seg7__L);
					ButtonSelect = 1;
				} else if (!(Value & 0x08)) {
					printf("Button: Right\n");
					I2CWrite(fd, MCP_PortA | MCP_Write, ~Seg7__r);
					ButtonSelect = 1;  
				} else if (!(Value & 0x10)) {
					printf("Button: B(2)\n");
					I2CWrite(fd, MCP_PortA | MCP_Write, ~Seg7_2);
					ButtonSelect = 1;
				} else if (!(Value & 0x20)) {
					printf("Button: C(3)\n");
					I2CWrite(fd, MCP_PortA | MCP_Write, ~Seg7_3);
					ButtonSelect = 1;
				} else if (0xBF == Value) {
					printf("idle\n");
					if (1 == ButtonSelect || 2 == ButtonSelect) {
						printf("Button released\n");
						I2CWrite(fd, MCP_PortA | MCP_Write, ~Seg7_Off);
						ButtonSelect = 0;
					}
				}
			} else { //select low
				if (!(Value & 0x10)) {
					printf("Button: A(1)\n");
					I2CWrite(fd, MCP_PortA | MCP_Write, ~Seg7_1);
					ButtonSelect = -1;
				} else if (!(Value & 0x20)) {
					printf("Button: Start\n");
					I2CWrite(fd, MCP_PortA | MCP_Write, ~Seg7__S);
					ButtonSelect = -1;
				} else if (0x33 == Value) {
					printf("idle\n");
					if (-1 == ButtonSelect || 2 == ButtonSelect) {
						printf("Button released\n");
						I2CWrite(fd, MCP_PortA | MCP_Write, ~Seg7_Off);
						ButtonSelect = 0;
					}
				}
			}
		}
		usleep(200000);
	}
	printf("write output value ...\n");
	I2CWrite(fd, MCP_PortA | MCP_Write, Value);
	close(fd);
	return 0;
}
