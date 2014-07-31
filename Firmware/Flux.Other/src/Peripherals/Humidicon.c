/* 
 * Copyright (c) 2012, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */

// Humidicon device driver
// KL 06-01-2012

#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "Peripherals/myI2C.h"
#include "Peripherals/Humidicon.h"

// Globals
char humPresent = 0;
humidicon_t gHumResults = {0};

// I2C routines
#define USE_AND_OR
#include "Peripherals/myI2C.h"
#define LOCAL_I2C_RATE		I2C_RATE_400kHZ
#define HIH6130_ADD		0x4E

// I2C
#define humOpen()              myI2COpen();myI2CStart(); WaitStartmyI2C();
#define humAddressRead(_r)     myI2Cputc(HIH6130_ADD); myI2Cputc((_r)); myI2CRestart(); WaitStartmyI2C(); myI2Cputc(HIH6130_ADD | I2C_READ_MASK);
#define humAddressWrite(_r)    myI2Cputc(HIH6130_ADD); myI2Cputc((_r)); 
#define humReadContinue()      myI2Cgetc(); myI2CAck()
#define humReadLast()          myI2Cgetc(); myI2CNack()
#define humWrite(_v)           myI2Cputc((_v));
#define humClose()             myI2CStop(); WaitStopmyI2C();myI2CClose();
#define humReopen()            myI2CRestart(); WaitRestartmyI2C();

char Humidicon_init(void) 
{
	// No init needed
	humPresent = FALSE;
	Humidicon_measurement_request();
	#ifndef USE_MY_SW_I2C
		if (I2C_STATbits_ACKSTAT == 0)humPresent = TRUE;
	#else
		if (myAckStat())humPresent = TRUE;
	#endif
	return humPresent;
}


char Humidicon_measurement_request(void) 
{
	if (!humPresent) return -1;
	
	humOpen();
	humWrite(HIH6130_ADD); // This is all you need
	humClose();
	
	// Return status
	#ifndef USE_MY_SW_I2C
		if (I2C_STATbits_ACKSTAT != 0) return FALSE;
	#else
		if (!myAckStat()) return FALSE;
	#endif

	// Verify non-zero results
	if (gHumResults.status == 0 && gHumResults.humidity == 0 && gHumResults.temperature == 0)
	{
		return FALSE;
	}

	return TRUE;
}

/* 	Reads into global structure */
void Humidicon_read(void)
{
	unsigned char data[4]; 
	
	if(!humPresent)
	{
		gHumResults.status = -1;		// (-1 = not present)
		gHumResults.humidity = 0xffff;
		gHumResults.temperature = 0xffff;
		return;   
	}	
	
	humOpen();
	humAddressRead(HIH6130_ADD);
	data[0] = humReadContinue();
	data[1] = humReadContinue();
	data[2] = humReadContinue();
	data[3] = humReadLast();
	humClose();

	// [old version] Put in global regs
	//gHumResults.humidity = ((data[0]&0x3f)<<8) + data[1];
	//gHumResults.temperature = ((data[0]<<8) + data[1])>>2;

	// [dgj] Put in global regs
	gHumResults.status = (char)(data[0] >> 6);		// (0 = normal, 1 = stale, 2 = command mode, 3 = diagnostic)
	gHumResults.humidity = ((unsigned short)(data[0] & 0x3f) << 8) | data[1];
	gHumResults.temperature = ((unsigned short)data[2] << 6) | (data[3] >> 2);

	return;
}


/* 	[dgj] Convert temperature to centi-degrees C (0.01 C) */
short Humidicon_Convert_centiC(unsigned short value)
{
	// 14-bit (0 - 16383)
	if (value == 0xffff) { return 0; }
	return (short)((16500UL * value + 8191) / 16384) - 4000;
}

/* 	[dgj] Convert humidity in 1/100 of a percent (0.01 %) */
unsigned short Humidicon_Convert_percentage(unsigned short value)
{
	// 14-bit (0 - 16383)
	if (value == 0xffff) { return 0; }
	return (unsigned short)((10000UL * value + 8191) / 16384);
}

//EOF



