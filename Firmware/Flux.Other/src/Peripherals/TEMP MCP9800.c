// Device driver for the MCP9800 12 bit I2C temperature sensor 
// KL 20-01-2012

// Include
#define USE_AND_OR
#include "GenericTypeDefs.h"
#include "Peripherals/myI2C.h"
#include "Peripherals/TEMP MCP9800.h"
// User defines for rate ant address
#define LOCAL_I2C_RATE		((OSCCONbits.COSC==1)? 72 : 18)		/*200kHz for this device, controls baud*/
#define I2C_ADDRESS			0x90 	/*I2C address MCP3421A0*/

// I2C functions used in this code are all myI2C 
#define SENSOROpen()              myI2COpen();myI2CStart(); WaitStartmyI2C();
#define SENSORAddressRead(_r)     myI2Cputc(I2C_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitStartmyI2C(); myI2Cputc(I2C_ADDRESS | I2C_READ_MASK);
#define SENSORAddressWrite(_r)    myI2Cputc(I2C_ADDRESS); myI2Cputc((_r)); 
#define SENSORReadContinue()      myI2Cgetc(); myI2CAck()
#define SENSORReadLast()          myI2Cgetc(); myI2CNack()
#define SENSORWrite(_v)           myI2Cputc((_v));
#define SENSORClose()             myI2CStop(); WaitStopmyI2C();myI2CClose();
#define SENSORReopen()            myI2CRestart(); WaitRestartmyI2C();

// Code
static unsigned char saved_MCP9800_setup = 0;
void MCP9800Init(unsigned char setup)
{
	saved_MCP9800_setup = setup;
	SENSOROpen();
	SENSORAddressWrite(MCP9800_CONFIG_REG);
	SENSORWrite(setup);
	SENSORClose();
}

void MCP9800Off(void)
{
	SENSOROpen();
	SENSORAddressWrite(MCP9800_CONFIG_REG);
	SENSORWrite(MCP9800_SHUTDOWN);
	SENSORClose();
}

void MCP9800Sample(void)
{
	// Shutdown to access one-shot
	MCP9800Off(); 
	// Then trigger a one shot conversion
	SENSOROpen();
	SENSORAddressWrite(MCP9800_CONFIG_REG);
	SENSORWrite(saved_MCP9800_setup | MCP9800_ONE_SHOT | MCP9800_SHUTDOWN);
	SENSORClose();	
}

unsigned char MCP9800CheckComplete(void)
{
	unsigned char config;
	SENSOROpen();
	SENSORAddressRead(MCP9800_CONFIG_REG);
	config = SENSORReadLast();
	SENSORClose();
	if (config & MCP9800_ONE_SHOT_nCOMPLETE)return FALSE; 
	else 									return TRUE;
}

// Assumes the current conversion is over
signed int MCP9800Read(void)
{
	signed int result;
	SENSOROpen();
	SENSORAddressRead(MCP9800_RESULT_REG);
	result = SENSORReadContinue();
	result <<= 8;
	result |= SENSORReadLast();
	SENSORClose();
	return result;
}

// Convert the reading into 0.1C steps - bottom 8 bits are fractional part in modulo 2 format
signed int MCP9800inCelcius(signed int MCP9800conversion)
{
	return ((signed int)(((signed long)MCP9800conversion*10)>>8));
}

//EOF
