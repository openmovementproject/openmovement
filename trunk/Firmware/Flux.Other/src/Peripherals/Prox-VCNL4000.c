/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
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

// Driver for the VCNL4000 proximity sensor
// Karim Ladha 28-06-2012

// Includes
#include "HardwareProfile.h"

#ifdef HAS_PROX

#include "GenericTypedefs.h"
#include <TimeDelay.h>
#include "Peripherals/Prox.h"
#include "Peripherals/myI2C.h"

// I2C routines
#define USE_AND_OR
//#include "Peripherals/myI2C.h"
#ifndef PROX_I2C_RATE
#define LOCAL_I2C_RATE		I2C_RATE_1000kHZ
#else
#define LOCAL_I2C_RATE		PROX_I2C_RATE
#endif
#define PROX_ADDRESS		0x26 	/*I2C address*/
#define PROX_MASK_READ  	0x01 	/*I2C_READ_MASK*/

// The device id
#define PROX_DEVICE_ID		0x11 /*Staic response*/

// Registers
#define 	VCNL_ADDR_COMMAND			0x80
#define 	VCNL_ADDR_PID				0x81
#define 	VCNL_ADDR_IRCURRENT			0x83
#define 	VCNL_ADDR_ALPARAM			0x84
#define 	VCNL_ADDR_ALRESH			0x85
#define 	VCNL_ADDR_ALRESL			0x86
#define 	VCNL_ADDR_PROXRESL			0x87
#define 	VCNL_ADDR_PROXRESH			0x88
#define 	VCNL_ADDR_PROXFREQ			0x89
#define 	VCNL_ADDR_PROXTIMING		0x8A

// I2C 
#define CUT_DOWN_I2C_CODE_SIZE
#ifndef CUT_DOWN_I2C_CODE_SIZE
#define PROXOpen()              myI2COpen();myI2CStart(); WaitStartmyI2C();
#define PROXAddressRead(_r)     myI2Cputc(PROX_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(PROX_ADDRESS | I2C_READ_MASK);
#define PROXAddressWrite(_r)    myI2Cputc(PROX_ADDRESS); myI2Cputc((_r)); 
#define PROXReadContinue()      myI2Cgetc(); myI2CAck()
#define PROXReadLast()          myI2Cgetc(); myI2CNack()
#define PROXWrite(_v)           myI2Cputc((_v));
#define PROXClose()             myI2CStop(); WaitStopmyI2C();myI2CClose();
#define PROXReopen()            myI2CRestart(); WaitRestartmyI2C();
#else
void PROXOpen(void)						{myI2COpen();myI2CStart(); WaitStartmyI2C();}
void PROXAddressRead(unsigned char _r)	{myI2Cputc(PROX_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(PROX_ADDRESS | I2C_READ_MASK);}
void PROXAddressWrite(unsigned char _r)	{myI2Cputc(PROX_ADDRESS); myI2Cputc((_r)); }
unsigned char PROXReadContinue(void)   	{unsigned char ret = myI2Cgetc(); myI2CAck();return ret;}
unsigned char PROXReadLast(void)       	{unsigned char ret = myI2Cgetc(); myI2CNack();return ret;}
void PROXWrite(unsigned char _v)       	{myI2Cputc((_v));}
void PROXClose(void )             		{myI2CStop(); WaitStopmyI2C();myI2CClose();}
void PROXReopen(void )            		{myI2CRestart(); WaitRestartmyI2C();}
#endif

// Globals
char 	proxPresent = 0;
prox_t 	prox = {{0}};


// Read device ID, sets present flag
unsigned char ProxVerifyDeviceId(void)
{
	unsigned char id;	
	PROXOpen();	
	PROXAddressRead(VCNL_ADDR_PID);
	id = PROXReadLast();
	PROXClose();
    proxPresent = (id == PROX_DEVICE_ID) ? 1 : 0;
	return proxPresent;
}

// Prox startup
void ProxStartup(void)
{
	// Blocks access if not detected
	if (!proxPresent) return;
	PROXOpen();
	PROXAddressWrite(VCNL_ADDR_COMMAND); 
	PROXWrite(0x00);	// Stop conversions
	PROXReopen();
	PROXAddressWrite(VCNL_ADDR_IRCURRENT); 
	PROXWrite(0x02); 	// IR current = 20mA 	
	PROXReopen();
	PROXAddressWrite(VCNL_ADDR_ALPARAM); 
	PROXWrite(0x0D);	//	Auto-sample off, Auto-compensate, 32 sample averaged 
	PROXReopen();
	PROXAddressWrite(VCNL_ADDR_PROXFREQ);
	PROXWrite(0x02);	// 781kHz modulation freq
	PROXAddressWrite(VCNL_ADDR_PROXTIMING);
	PROXWrite(0x00);	// Not sure what to write to this - see datasheet
	PROXClose();

	ProxReadSample(); 	// Clear data ready flags
	ProxStartSample();
	return;
}

// Initiate on demand measurement
void ProxStandby(void)
{
	// Blocks access if not detected
	if (!proxPresent)  return;

	PROXOpen();
	PROXAddressWrite(VCNL_ADDR_ALPARAM); 
	PROXWrite(0x0D);	// Auto-sample off, Auto-compensate, 32 sample averaged 
	PROXClose();
	return;
}

// Initiate a conversion
void ProxStartSample(void)
{
	// Blocks access if not detected
	if (!proxPresent) return;

	PROXOpen();
	PROXAddressWrite(VCNL_ADDR_COMMAND); 
	PROXWrite(0x18);	// Initiate on demand measurements of proximity and light
	PROXClose();

// HACK: to overcome the problem with this device
#warning "This is a hack, there are better ways to implement this"
DelayMs(1);

	return;
}

// Reads a conversion
prox_t* ProxReadSample(void)
{
	// Blocks access if not detected
	if (!proxPresent) return &prox;

	PROXOpen();
	PROXAddressRead(VCNL_ADDR_ALRESH); 
	prox.values[1] = 	PROXReadContinue();
	prox.values[0] = 	PROXReadContinue();
	prox.values[3] = 	PROXReadContinue();
	prox.values[2] = 	PROXReadLast();
	return &prox;
}

// Reads a conversion
unsigned char ProxSampleReady(void)
{
	unsigned char retval;

	// Blocks access if not detected
	if (!proxPresent) return TRUE;

	PROXOpen();
	PROXAddressRead(VCNL_ADDR_COMMAND); 
	retval = PROXReadLast();
	PROXClose(); 

	if((retval&0x60)==0x60) return TRUE;
	else return FALSE;
}


#endif // HAS_PROX

//EOF
