// Device driver for the MCP3421 18 bit I2C ADC 
// KL 20-01-2012

// Include
#define USE_AND_OR
#include "GenericTypeDefs.h"
#include "Peripherals/myI2C.h"
#include "Peripherals/ADC MCP3421.h"

// User defines for rate ant address
#define LOCAL_I2C_RATE		I2C_RATE_1000kHZ		
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

// Globals
static unsigned char saved_MCP3421_setup = 0;
unsigned char MCP3421Present = FALSE;

// Code
unsigned char MCP3421Verify(void)
{
	ADCOpen();
	ADCAddressRead();
	MCP3421Present = myAckStat();
	ADCReadContinue(); // Dummy reads
	ADCReadContinue();
	ADCReadContinue();
	ADCReadLast();
	ADCClose();
	return MCP3421Present;	
}
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
	if((saved_MCP3421_setup&MCP3421_SAMPLE_18BIT_267MS) == MCP3421_SAMPLE_18BIT_267MS) 
	{
		result = ADCReadContinue();
		result <<= 8;
		result |= ADCReadContinue();
		result <<= 8;
		result |= ADCReadLast();
		if (result  & 0x00800000L) result|=0xff000000L;
	}
	// 12, 14 or 16 bit modes - 2 bytes
	else 
	{
		result = ADCReadContinue();
		result <<= 8;
		result |= ADCReadLast();
		if (result  & 0x00008000L) result|=0xffff0000L;
	}
	ADCClose();
	return result;
}

#ifdef USE_THERMOCOUPLE_TYPE_K
// Convert to 8bit+8bit signed fractional -128 to 127 degrees C (same as MCP9800)
signed short MCP3421CalcTemperatureFractional (signed long conversionResult)
{
	/*
	The MCP3421 needs to be set to 18bit and a gain of 8v/v. FVR is fixed at 2.048v
	At this setting the sensitivity is (2*FVR)/((2^18)*8) =  1.953125uV
	K-Type thermocouples yield typically 41uV/^C
	The degrees/count is therefore: (4,096,000)/(41*(2^21)) = 1/20.992 or ~1/21
	*/
	signed long tempFractional;
	// Get signed result in 0.1^c steps
	tempFractional = ((signed long)conversionResult<<8)/21; 
	// Clamp
	if (tempFractional > 32767) tempFractional = 	32767;
	if (tempFractional < -32768) tempFractional = 	-32768;

	// Return value is the signed absolute thermocouple temperature in 0.1C steps
	return (signed short )tempFractional;
}
// Calculate compensated temp in 0.1 degrees C using the MCP9800 value as the cold junction
signed int MCP3421CalcTemperature (signed long conversionResult, signed int coldJunction)
{
	/*
	The MCP3421 needs to be set to 18bit and a gain of 8v/v. FVR is fixed at 2.048v
	At this setting the sensitivity is (2*FVR)/((2^18)*8) =  1.953125uV
	K-Type thermocouples yield typically 41uV/^C
	The degrees/count is therefore: (4,096,000)/(41*(2^21)) = 1/20.992 or ~1/21
	To perform the offset (cold junction compensation) we add the MCP9800 result to the conversion.
	I.e. If the MCP9800 says -1 and the thermocouple says -1, the thermocouple is at -2 degrees.
	MCP9800 resolution (using the full 16 bits) is 1/256th of a celcius, MCP3421 counts per celcius ~=21
	So we multiply by 21 and divide by 256: Count offset = ((signed long)<MCP9800>*21)>>8; 
	*/
	signed long tempAbsolute;
	signed int fractional;
	// Account for offset, coldJunction is in the MCP9800 12 bit result format <8bit signed temperature><4bit signed fraction><4bit = 0>
	conversionResult += ((signed long)coldJunction*21)>>8;
	// Get signed result in 0.1^c steps
	tempAbsolute = ((signed long)10*conversionResult)/21; 
	// Get fractional part (div by 2.1 == 122/256)
	/* You should avoid modulo with signed numbers*/
	if (conversionResult < 0) // negative?
	{fractional = -(((signed int)((-conversionResult)%21)*122)>>8);}
	else
	{fractional = (((signed int)((conversionResult)%21)*122)>>8);}
	// Add on fractional
	tempAbsolute += fractional;
	// Return value is the signed absolute thermocouple temperature in 0.1C steps
	return tempAbsolute;
}
#endif

//EOF
