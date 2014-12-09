#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c-dev.h>

unsigned short ReadAddress(int fd, char nAddress, const char* szDataName) {
	char buf[3] = {0};

	buf[0] = 0xFF;
	buf[1] = nAddress;
	if (write(fd, buf, 2) != 2) {
		fprintf(stderr, 
		"Failed to write to the i2c bus (%s)\n",strerror(errno));
		return 0;
	}
	usleep(80000);
	buf[0] = 0xFD;
	if (write(fd, buf, 1) != 1) {
		fprintf(stderr, 
		"Failed to write to the i2c bus (%s)\n",strerror(errno));
		return 0;
	}

	unsigned short* pData = (unsigned short*) &buf[1];
	if (read(fd, buf, 2) != 2) {
		fprintf(stderr, 
		"Failed to read from the i2c bus (%s)\n",strerror(errno));
		return 0;
	} else {
		buf[2]=buf[0]; 
		//printf("%s data: 0x%X (%hu)\n", szDataName,*pData,*pData);
	}
	return *pData;
}

void main(int argc, char* argv[]) {
	const int SlaveAddr   = 0x77;
	const int cnConstants = 11;
	const int cnMaxLine   = 10;
	int fd;
	char device[20];
	int D1,D2;
	int nParameters[cnConstants];

	if (argc >=2 ) {
		FILE* file;
		const char* szFilename=NULL;
		const int cnMaxLine = 10;
		char szBuffer[cnMaxLine+1];
		
		szFilename = argv[1];	
		if (szFilename) {
			file = fopen(szFilename, "rb");
		}
		if (file) {
			int nParamCnt;
			for (nParamCnt=0; nParamCnt<cnConstants; nParamCnt++) {
				if (!fgets(szBuffer, cnMaxLine, file)) {
					fprintf(stderr,
					"Error reading line %d from file '%s'\n", nParamCnt+1, szFilename);
					exit(1);
				}		
				if (sscanf(szBuffer, "%X", &nParameters[nParamCnt]) !=1 ) {
					fprintf(stderr,
					"Error scanning line %d from file '%s'\n", nParamCnt+1, szFilename);
					exit(1);
				}
			}
			fclose(file);
		}
		else{
			fprintf(stderr, "Can't open file '%s' for reading\n", szFilename);
			exit(1);
		}
		
	}
	else{
		fprintf(stderr, "Constant file parameter missing\n");
		exit(1);	
	}

	sprintf(device, "/dev/i2c-1");
	if ((fd = open(device, O_RDWR)) < 0) {
		fprintf(stderr,"Failed to open i2c bus '%s'\n",device);
        exit(1);
    }
    if (ioctl(fd, I2C_SLAVE, SlaveAddr) < 0) {
		fprintf(stderr,
		"Failed to acquire i2c bus access or talk to slave %X\n", SlaveAddr);
		close(fd);
		exit(1);
	}
	
	D1 = ReadAddress(fd, 0xF0, "Pressure");
	D2 = ReadAddress(fd, 0xE8, "Temperature");
	usleep(50000); //drop first reading
	D1 = ReadAddress(fd, 0xF0, "Pressure");
	D2 = ReadAddress(fd, 0xE8, "Temperature");	
	close(fd);
	if (D1 != 0 && D2 != 0) {
		int C1, C2, C3, C4, C5, C6, C7, A, B, C, D;
		int dUT, OFF, SENS, X, P, T;

		C1 = nParameters[0];
		C2 = nParameters[1];
		C3 = nParameters[2];
		C4 = nParameters[3];
		C5 = nParameters[4];
		C6 = nParameters[5];
		C7 = nParameters[6];
		A  = nParameters[7];
		B  = nParameters[8];
		C  = nParameters[9];
		D  = nParameters[10];

		if (D2 >= C5) {
			dUT = D2 - C5 - ((D2 - C5) / 128) * ((D2 - C5) / 128) * A / (2 << C);
		} else {
			dUT = D2 - C5 - ((D2 - C5) / 128) * ((D2 - C5) / 128) * B / (2 << C);
		}
		OFF = (C2 + (C4 - 1024) * dUT / 16384) * 4;
		SENS = C1 + C3 * dUT / 1024;
		X = SENS * (D1 - 7168) / 16384 - OFF;
		P = X * 10 / 32 + C7;
		T = 250 + dUT * C6 / 65536 - dUT / (2 << D);
		
		printf("Pressure=%.1f hPa\tTemperature=%.1f C\n", P / 10.0f, T / 10.0f);
		exit(0);
	} else {
		fprintf(stderr, 
		"ADC read error (i2c bus ok?, XCLR high?, MCLK=32kHz?\n");
		exit(1);	
	}
}
