
#include "I2C_Interface.h"
#include <stdio.h>


unsigned char buf_out[100]; // output from master
unsigned char buf_in[100];// input for master from slave
unsigned char data_out[100]; // data to be transferred from master to slave
const char i2c_name[40] = "/dev/i2c-1";
const int adress_lepton = 0x2A;

int main() {

	unsigned short CommandID;
	unsigned short CommandWordLength;// number of words that are transferred
	unsigned short CommandType; // (Get=0; Set=1; Run=2)

	// Read in desired I2C command from user over command prompt
	int CommandIdx;
	CommandIdx=ReadI2CCommand();
	CommandID=commandList[CommandIdx][0];
	CommandWordLength=commandList[CommandIdx][1];
	CommandType=commandList[CommandIdx][2];

	unsigned short CommandByteLength=CommandWordLength<<1; // number of bytes that are transferred



	// Establish I2C bus to Lepton camera
	file = i2c_init(i2c_name, adress_lepton);

	// Execute desired Command
	WriteToCommandReg(CommandID,CommandWordLength,CommandType);

	// Read data from data register in master input buffer
	ReadDataReg(CommandByteLength);

	// Display content in master input buffer
	ReadOut(CommandByteLength);

	// Close I2C bus to Lepton camera
	i2c_close();

	return 0;

}

