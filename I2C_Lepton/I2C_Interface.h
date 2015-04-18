
#ifndef I2C_INTERFACE
#define I2C_INTERFACE

extern int ak; // acknowledged integer
extern unsigned char buf_out[100]; // output from master
extern unsigned char buf_in[100];// input for master from slave
extern unsigned char data_out[100]; //data that is transferred from master to slave
extern int file; // file descriptor for lepton camera
extern const int adress_lepton;
extern const char i2c_name[40];

/* Commands
***************
Total number of known commands is saved in "n_commands"
Commands are saved in two dimensional array "commandList":
 	First column: 	command ID
	Second column: 	number of words transmitted
	Third column: 	type of command (Get=0; Set=1; Run=2)
*/
extern const int n_commands;
extern unsigned short commandList[19][3];

// Functions
extern int i2c_init(const char *filename, int addr);
extern void i2c_close();
extern void i2c_read(int n_in, int n_out);
extern void WriteToCommandReg(unsigned short CommandID, unsigned short  CommandDataLength, unsigned short CommandIdx);
extern void WriteToDatalenghtReg(unsigned short DataLength);
extern void WriteToDataReg(unsigned short n_words);
extern void WaitingTillAccomplished();
extern void ReadDataReg(unsigned short n_read);
extern void ReadOut(unsigned short n_read);
extern int ReadI2CCommand();

#endif
