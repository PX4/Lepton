#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "I2C_Interface.h"

int ak; // acknowledge integer
int file; // file descriptor for lepton camera

unsigned short commandList[19][3]={
		{(unsigned short)0x0100,2,0}, 	// Get AGC Enable State
		{(unsigned short)0x0101,2,1}, 	// Set AGC Enable State (1.Word:{0,1,2}; 2.Word:0)
		{(unsigned short)0x0124,1,0},	// Get AGC HEQ Damping factor
		{(unsigned short)0x0125,1,1}, 	// Set AGC HEQ Damping factor
		{(unsigned short)0x012C,1,0},	// Get AGC HEQ Clip Limit High
		{(unsigned short)0x012D,1,1}, 	// Set AGC HEQ Clip Limit High (1.Word:[0,4800])
		{(unsigned short)0x0144,2,0},	// Get AGC HEQ Output Scale Factor
		{(unsigned short)0x0145,2,1},	// Set AGC HEQ Output Scale Factor (1.Word:{0,1}; 2.Word:0)
		{(unsigned short)0x0148,2,0},	// Get AGC Calculation enable state
		{(unsigned short)0x0149,2,1},	// Set AGC Calculation enable state (1.Word:{0,1,2}; 2.Word:0)
		{(unsigned short)0x0208,4,0},
		{(unsigned short)0x0210,1,0},
		{(unsigned short)0x2014,1,0},
		{(unsigned short)0x0218,2,0}, // Get SYS Telemetry state
		{(unsigned short)0x0219,2,1}, // Set SYS Telemetry state (1.Word:{0,1,2}; 2.Word:0)
		{(unsigned short)0x021C,2,0},// Get SYS Telemetry location
		{(unsigned short)0x021D,2,1},// Set SYS Telemetry location (1.Word:{0,1,2}; 2.Word:0)
		{(unsigned short)0x0324,2,0}, // Get Video Freeze Enable State
		{(unsigned short)0x0325,2,1} // Set Video Freeze Enable State (1.Word:{0,1,2}; 2.Word:0)
};
const int n_commands=19;

int i2c_init(const char *filename, const int addr) {

	// Check I2C connectivity
	file = open(filename, O_RDWR);
	if (file < 0) {
		printf("Failed to open bus\n");

	}

	// Check connectivity to lepton camera
	ak = ioctl(file, I2C_SLAVE, addr);
	if (ak < 0) {
		printf("Failed to acquire bus access and/or talk to slave\n");
	}
	return file;
}
void i2c_close(){
	// closes I2C connection
	close(file);
	return;
}


void i2c_read(int n_in, int n_out) {

	// Setting pointer to desired register address
	ak = write(file, buf_out, n_out);

	if (ak != n_out) {
		printf("Failed to access desired register\n");
	}

	// Reading in content of desired register and saving it in master input buffer
	ak = read(file, buf_in, n_in);
	if (ak != n_in) {
		printf("Could not read content in status register\n");
	}

}

void WriteToCommandReg( unsigned short CommandID, unsigned short  CommandDataLength, unsigned short CommandType) {

	// Write data to data register if it is a set command
	if(CommandType==1){
		WriteToDataReg(CommandDataLength);
	}

	// Write to Data length register
	WriteToDatalenghtReg(CommandDataLength);

	// Write command into master output buffer
	buf_out[0] = 0x00;
	buf_out[1] = 0x04;
	buf_out[2]=(CommandID>>8)&0xFF;
	buf_out[3]=CommandID&0xFF;

	// Write command into command register
	ak = write(file, buf_out, 4);
	if (ak != 4) {
		printf("Error writing %i bytes\n", 4);
	}
	// Wait until execution of command is accomplished
	WaitingTillAccomplished();

	return;

}

void WriteToDataReg(unsigned short n_words){

	// Write n_words data into master output buffer
		buf_out[0] = 0x00;
		buf_out[1] = 0x08;

		for(int j=0;j<n_words*2;j++){
					buf_out[j+2]=data_out[j];
					}

		// Write data into data register
		ak = write(file, buf_out, 4);
		if (ak != 4) {
			printf("Error writing %i bytes\n", 4);
		}
		// Wait until execution of command is accomplished
		WaitingTillAccomplished();
		return;
}

void WriteToDatalenghtReg(unsigned short DataLength) {

	// Write address and data length into master output buffer
	buf_out[0] = 0x00;
	buf_out[1] = 0x06;
	buf_out[2] = (DataLength>>8)&0xFF;;
	buf_out[3] = DataLength&0xFF;


	// Write data length into data length register
	ak = write(file, buf_out, 4);

	if (ak != 4) {
		printf("Error writing %i bytes to data length register\n", 4);
	}
	return;

}


void ReadDataReg(unsigned short n_read){

	// Write address of data register into master output buffer
	buf_out[0]=0x00;
	buf_out[1]=0x08;

	// reads n_read bytes from data register and saves it in master input buffer
	i2c_read(n_read,2);

	return;
}

void ReadOut(unsigned short n_read) {
 short output_int=0;

	// Reads out n_out bytes content of master input buffer in hexadecimal format
	printf("Master Input buffer\n");
	printf("*******************\n");
	for (unsigned short i = 0; i < n_read; i++) {

		printf("%d. byte:0x%0*x\n", i + 1, 2, buf_in[i]);
	}
	printf("Total in hexadecimal format: 0x");
	for (unsigned short i = 0; i < n_read; i++) {


		}
	printf("\n");
	for (unsigned short i = 0; i < n_read/2; i++) {
		for (unsigned short j=0;j<2;j++){
output_int=(((short)buf_in[i+j])<<8*(1-j))+output_int;

		}
		printf("%d. word: %d\n",i+1,output_int);
		output_int=0;
			}
		printf("\n\n");
	return;
}


int ReadI2CCommand(){
	int temp;
	// Read in input command
	 printf("Please enter command in hexadecimal format : ");
	  scanf("%x", &temp);

	 // Check whether command is valid
	int i;
	int j;
	for(i=0;i<n_commands;i++){
	if( commandList[i][0]==(unsigned short)temp){
		break;
		}
	}

if (i==n_commands){
	printf("Unknown command.\n");
	exit(EXIT_FAILURE);
		}

else{

	printf("Chosen command\n");
	printf("***************\n");
	printf("CommandID=0x%0*x\n",4,commandList[i][0]);
	printf("Number of words=%d\n",commandList[i][1]);

	// Load set data
		if (commandList[i][2]==1){
			for(j=0;j<commandList[i][1];j++){
			printf("Please enter data of %d. word to be transferred : ",j+1);
			scanf("%d", &temp);
			data_out[j*2]=(((unsigned short)temp)>>8)&0xFF;
			data_out[j*2+1]=((unsigned short)temp)&0xFF;
			}
		}
		printf("\n");
	return i;
	}
}



void WaitingTillAccomplished(){

// Checks BUSY bit of status register and returns when cleared

	buf_out[0]=0x00;
	buf_out[1]=0x02;
	i2c_read(2,2);

	while(buf_in[1] & 1)	{
					printf("Waiting for command to be accomplished\n");
					i2c_read(2,2);
					}

	// Sanity check
	printf("Status register\n");
	printf("***************\n");
	printf("Error Code: %d\n", (int)buf_in[0]);
	printf("second byte:0x%0*x\n", 2, buf_in[1]);
	printf("\n");
	return;
}


