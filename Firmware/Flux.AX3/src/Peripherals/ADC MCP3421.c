// Device driver for the MCP3421 18 bit I2C ADC 
// KL 20-01-2012

// Include
#define USE_AND_OR
#include "GenericTypeDefs.h"
#include "myI2C.h"
#include "Peripherals/ADC MCP3421.h"

// User defines for rate ant address
#define LOCAL_I2C_RATE		((OSCCONbits.COSC==1)? 72 : 18)		/*200kHz for this device, controls baud*/
#define I2C_ADDRESS			0xD0 	/*I2C address MCP3421A0*/

// I2C functions used in this code are all myI2C - The device only has one register for writing and one for reading so no actual addressing
#define ADCOpen()              myI2COpen();myI2CStart(); WaitStartmyI2C();
/*#define ADCAddressRead(_r)   myI2Cputc(I2C_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitStartmyI2C(); myI2Cputc(I2C_ADDRESS | I2C_READ_MASK);*/
#define ADCAddressRead()       myI2Cputc(I2C_ADDRESS | I2C_READ_MASK);
#define ADCAddressWrite(_r)    myI2Cputc(I2C_ADDRESS); myI2Cputc((_r)); 
#define ADCReadContinue()      myI2Cgetc(); myI2CAck()
#define ADCReadLast()          myI2Cgetc(); myI2CNack()
#define ADCWrite(_v)           myI2Cputc((_v));
#define ADCClose()             myI2CStop(); WaitStopmyI2C();myI2CClose();
#define ADCReopen()            myI2CRestart(); WaitRestartmyI2C();

// Code
static unsigned char saved_MCP3421_setup = 0;
void MCP3421Init(unsigned char setup)
{
	saved_MCP3421_setup = setup;
	ADCOpen();
	ADCAddressWrite(setup);
	ADCClose();
}
void MCP3421Off(void)
{
	ADCOpen();
	ADCAddressWrite(0);
	ADCClose();
}
void MCP3421Sample(void)
{
	ADCOpen();
	ADCAddressWrite(saved_MCP3421_setup|MCP3421_START_SAMPLE);
	ADCClose();
}
unsigned char MCP3421CheckComplete(void)
{
	unsigned char config;
	ADCOpen();
	ADCAddressRead();
	ADCReadContinue(); // Dummy reads
	ADCReadContinue();
	ADCReadContinue();
	config = ADCReadLast();
	ADCClose();
	if (config & MCP3421_SAMPLE_nREADY) return FALSE; 
	else 								return TRUE;
}

// Assumes the current conversion is over
signed long MCP3421Read(void)
{
	signed long result;
	ADCOpen();
	ADCAddressRead();
	// If in 18 bit mode  - 3 bytes
	if((saved_MCP3421_setup|MCP3421_SAMPLE_18BIT_267MS) == MCP3421_SAMPLE_18BIT_267MS) 
	{
		result = ADCReadContinue();
		result <<= 8;
		result = ADCReadContinue();
		result <<= 8;
		result |= ADCReadLast();
	}
	// 12, 14 or 16 bit modes - 2 bytes
	else 
	{
		result = ADCReadContinue();
		result <<= 8;
		result |= ADCReadLast();
	}
	ADCClose();
	return result;
}

//EOF
