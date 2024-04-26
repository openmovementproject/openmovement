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

// Driver for the Mag3110 magnetometer device, this is a 16-bit device
// Karim Ladha 08-12-2012

// Needed, or else spi.h fails
#define USE_AND_OR

// Includes
#include "HardwareProfile.h"
#include <stdio.h>
#include <TimeDelay.h>
#include "Peripherals/Mag.h"

// Alternate I2C bus?
#ifdef MAG_ALTERNATE_I2C
	#define MY_I2C_OVERIDE MAG_ALTERNATE_I2C
#endif
#define USE_AND_OR
#include "Peripherals/myI2C.h"

#ifndef MAG_I2C_MODE
	#error "This magnetometer is I2C only."
#endif

// Mag3110 registers
#define	MAG_ADDR_DR_STATUS		0x00
#define	MAG_ADDR_OUT_X_MSB		0x01
#define	MAG_ADDR_OUT_X_LSB		0x02
#define	MAG_ADDR_OUT_Y_MSB		0x03
#define	MAG_ADDR_OUT_Y_LSB		0x04
#define	MAG_ADDR_OUT_Z_MSB		0x05
#define	MAG_ADDR_OUT_Z_LSB		0x06
#define	MAG_ADDR_WHO_AM_I		0x07
#define	MAG_ADDR_SYSMOD			0x08
#define	MAG_ADDR_OFF_X_MSB		0x09
#define	MAG_ADDR_OFF_X_LSB		0x0A
#define	MAG_ADDR_OFF_Y_MSB		0x0B
#define	MAG_ADDR_OFF_Y_LSB		0x0C
#define	MAG_ADDR_OFF_Z_MSB		0x0D
#define	MAG_ADDR_OFF_Z_LSB		0x0E
#define	MAG_ADDR_DIE_TEMP		0x0F
#define	MAG_ADDR_CTRL_REG1		0x10
#define	MAG_ADDR_CTRL_REG2		0x11

// I2C routines
#define LOCAL_I2C_RATE		I2C_RATE_200kHZ
#define MAG_ADDRESS			0x1C	/*I2C address*/
#define MAG_MASK_READ  		0x01 	/*I2C_READ_MASK*/
#define MAG_DEVICE_ID		0xC4 	/*Staic response*/

// I2C 
#define CUT_DOWN_I2C_CODE_SIZE
#ifndef CUT_DOWN_I2C_CODE_SIZE
#define MagOpen()              myI2COpen();myI2CStart(); WaitStartmyI2C();
#define MagAddressRead(_r)     myI2Cputc(MAG_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(MAG_ADDRESS | I2C_READ_MASK);
#define MagAddressWrite(_r)    myI2Cputc(MAG_ADDRESS); myI2Cputc((_r)); 
#define MagReadContinue()      myI2Cgetc(); myI2CAck()
#define MagReadLast()          myI2Cgetc(); myI2CNack()
#define MagWrite(_v)           myI2Cputc((_v));
#define MagClose()             myI2CStop(); WaitStopmyI2C();myI2CClose();
#define MagReopen()            myI2CRestart(); WaitRestartmyI2C();
#else
void MagOpen(void)						{myI2COpen();myI2CStart(); WaitStartmyI2C();}
void MagAddressRead(unsigned char _r)	{myI2Cputc(MAG_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(MAG_ADDRESS | I2C_READ_MASK);}
void MagAddressWrite(unsigned char _r)	{myI2Cputc(MAG_ADDRESS); myI2Cputc((_r)); }
unsigned char MagReadContinue(void)   	{unsigned char ret = myI2Cgetc(); myI2CAck();return ret;}
unsigned char MagReadLast(void)       	{unsigned char ret = myI2Cgetc(); myI2CNack();return ret;}
void MagWrite(unsigned char _v)       	{myI2Cputc((_v));}
void MagClose(void )             		{myI2CStop(); WaitStopmyI2C();myI2CClose();}
void MagReopen(void )            		{myI2CRestart(); WaitRestartmyI2C();}
#endif

static unsigned char 	magRate = 0;
				char 	magPresent = 0;

unsigned char MagRate(void) 		{ return magRate; }

// Read device ID
unsigned char MagVerifyDeviceId(void)
{
	unsigned char id;	
	MagOpen();	
	MagAddressRead(MAG_ADDR_WHO_AM_I);
	id = MagReadLast();
	MagClose();
    magPresent = (id == MAG_DEVICE_ID) ? 1 : 0;
	return magPresent;
}

// MagStartup
void MagStartup(unsigned char samplingRate)
{
	if(!magPresent)return;
	MAG_DISABLE_INTS();
	MagOpen();
	MagReopen();
	MagAddressWrite(MAG_ADDR_CTRL_REG1);
	MagWrite(0b00000000);	/*Device is turned off first*/
	MagReopen();
	MagAddressWrite(MAG_ADDR_OFF_X_MSB);	/* Offset regs*/
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_X_MSB */
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_X_LSB */
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_Y_MSB */
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_Y_LSB */
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_Z_MSB */
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_Z_LSB */
	MagReopen();
	/*MAG_ADDR_CTRL_REG2*/
	MagAddressWrite(MAG_ADDR_CTRL_REG2);
	MagWrite(0b10000000); /*
	b7		: 	AUTO_MRST_EN Automatic Magnetic Sensor Reset. 0: Automatic Magnetic sensor resets off. 1: Automatic Magnetic sensor resets on.
	b5		:	RAW 0: Normal mode -data values are corrected by the user offset register values. 1: Raw mode: data values are not corrected by the user offset register values.
	b4		:	Mag_RST 0: Reset cycle not active. 1: Reset cycle initiate or Reset cycle busy/active.*/
	MagClose(); 

	/*Write ctrl reg 1 with integrity check*/
	{
		unsigned char ctrl_reg1 = 0, read_back = 0, retrys = 3;
		switch (samplingRate)
		{
			/*keeping the minimum possible oversample ratio - lowest power - continuous normal sampling ON */
			case (80)	:	{ctrl_reg1 = 0b00000001;magRate = 80;	break;} /*900uA*/
			case (40)	:	{ctrl_reg1 = 0b00100001;magRate = 40;	break;} /*550uA*/
			case (20)	:	{ctrl_reg1 = 0b01000001;magRate = 20;	break;} /*275uA*/
			case (10)	:	{ctrl_reg1 = 0b01100001;magRate = 10;	break;} /*137uA*/
			case (5)	:	{ctrl_reg1 = 0b10000001;magRate = 5;	break;} /*68uA*/
			case (2)	:	{ctrl_reg1 = 0b10100001;magRate = 2;	break;} /*34.4uA*/
			case (1)	:	{ctrl_reg1 = 0b11000001;magRate = 1;	break;} /*17.2uA*/
			case (0)	:	{ctrl_reg1 = 0b00000010;magRate = 0;	break;} /*Polled,12.5ms OSR=16 (fastest)*/
			default 	:	{ctrl_reg1 = 0b01100010;magRate = 0;	break;} /*Polled,100ms  OSR=16*/
		}
		/*Write register with retrys*/
		while(retrys--)
		{
			MagOpen();
			MagReopen();
			MagAddressWrite(MAG_ADDR_CTRL_REG1); /*MAG_ADDR_CTRL_REG1*/
			MagWrite(ctrl_reg1); /*
				b7-5	:	DR[2:0]	Data rate selection. Default value: 000.
				b4-3	:	OS [1:0] This register configures the over sampling ratio or measurement integration time. Default value: 00.
				b2		:	FR Fast Read selection. Default value: 0: The full 16-bit values are read. 1: Fast Read, 8-bit values read from the MSB registers.
				b1		:	TM Trigger immediate measurement. 0: Normal operation based on AC condition. 1: Trigger measurement.
				b0		:	AC	0: STANDBY mode. 1: ACTIVE mode. */
			
			MagReopen();	
			MagAddressRead(MAG_ADDR_CTRL_REG1);
			read_back = MagReadLast();
			MagClose();
			if(read_back == ctrl_reg1)break; 	/*Success*/
			else ; 								/*Fail, retry..*/
		}
	}

	#ifdef MAG_ZERO_ON_POWER_UP
		// KL: This applies the first sample as an offset
		mag_t temp;
		if(samplingRate<=0) return;
		MagSingleSample(NULL); 			// Discard initial buffer contents
		MagSingleSample(NULL); 			// Discard initial buffer contents
		DelayMs(25);				// Wait sensor startup time (25ms)
		DelayMs(2000/samplingRate);	// Wait for a sample (2x sample time in ms)
		MagSingleSample(&temp);		// Discard some samples for settling
		DelayMs(2000/samplingRate);	// Wait for a sample (2x sample time in ms)		
		MagSingleSample(&temp);		// Discard some samples for settling
		DelayMs(2000/samplingRate);	// Wait for a sample (2x sample time in ms)		
		MagSingleSample(&temp);		// Discard some samples for settling
		DelayMs(2000/samplingRate);	// Wait for a sample (2x sample time in ms)		
		MagSingleSample(&temp);		// Get a sample	
		MagWriteOffsets(&temp); 	// Read sample and apply offset
	#endif

	return;
}

// Shutdown the device to standby mode (standby mode, interrupts disabled)
void MagStandby(void)
{
	if(!magPresent)return;
	MAG_DISABLE_INTS();
	MagOpen();
	MagAddressWrite(MAG_ADDR_CTRL_REG1);
	MagWrite(0b01100000);/* 10Hz OSR 16, 137uA, continuous normal sampling off - standby
	b7-5	:	DR[2:0]	Data rate selection. Default value: 000.
	b4-3	:	OS [1:0] This register configures the over sampling ratio or measurement integration time. Default value: 00.
	b2		:	FR Fast Read selection. Default value: 0: The full 16-bit values are read. 1: Fast Read, 8-bit values read from the MSB registers.
	b1		:	TM Trigger immediate measurement. 0: Normal operation based on AC condition. 1: Trigger measurement.
	b0		:	AC	0: STANDBY mode. 1: ACTIVE mode. */
	MagClose();
}

/*Note: The samples are 2's compliment and 16bit*/
void MagSingleSample(mag_t *value)
{
	if(!magPresent){value->x = value->y = value->z = 0;return;}
	unsigned char dataReadyStatus;
	static mag_t last_read = {{0}};
	int i;

	for (i=0;i<5;i++) // Retry 5 times
	{
		MagOpen();	
		MagAddressRead(MAG_ADDR_DR_STATUS);
		dataReadyStatus = MagReadContinue();
	
		// Check for read collision 
		if (dataReadyStatus == 0xC4) 
		{
			MagReadLast();	// Dummy
			MagClose();		// Close bus
			continue;		// Loop round again	
		}
		else
		{
			break; 			// Device responded ok, continue reading
		}
	}
	
	// Check for errors
	if(i>=5)
	{
		// FAILED, device not responding- return last sample
		value->xh=last_read.xh;
		value->xl=last_read.xl;
		value->yh=last_read.yh;
		value->yl=last_read.yl;
		value->zh=last_read.zh;
		value->zl=last_read.zl;
		return; 
	}

	// No collision detected
	if (value == NULL)
	{
		MagReadContinue();MagReadContinue();MagReadContinue();
		MagReadContinue();MagReadContinue();MagReadLast();
	}
	else
	{
		// Store each value to return if error occurs
		last_read.xh = value->xh = MagReadContinue();
		last_read.xl = value->xl = MagReadContinue();
		last_read.yh = value->yh = MagReadContinue();
		last_read.yl = value->yl = MagReadContinue();
		last_read.zh = value->zh = MagReadContinue();
		last_read.zl = value->zl = 		MagReadLast();

	}
	MagClose();

	return;
}

// Enable interrupts
void MagEnableInterrupts(void)
{
	if(!magPresent)return;
	// The Magnetometer is always enabled when continuous sampling
	// The INT pin is cleared (active high) by reading the data regs
	while(MAG_INT)MagSingleSample(NULL); // Clears pin
	MAG_ENABLE_INTS();
	if (MAG_INT)MAG_INT_IF = 1;
	return;
}

// Read at most 'maxEntries' 3-axis samples 
void MagWriteOffsets(mag_t *magBuffer)
{
	if(!magPresent)return;
	MagOpen();
	MagAddressWrite(MAG_ADDR_OFF_X_MSB);
	MagWrite(magBuffer->xh);
	MagWrite(magBuffer->xl);
	MagWrite(magBuffer->yh);
	MagWrite(magBuffer->yl);
	MagWrite(magBuffer->zh);
	MagWrite(magBuffer->zl);
	MagClose();
return;
}

// Read at most 'maxEntries' 3-axis samples 
unsigned char MagReadFIFO(mag_t *magBuffer, unsigned char maxEntries)
{
// This device has no FIFO
return 0;
}

// Read interrupt source
unsigned char MagReadIntSource(void)
{
// This devices interrupt always means data ready...
return 0;
}

// Debug dump registers
#if 0
void MagDebugDumpRegisters(void)
{
	static unsigned char i,regs[10];
	MagOpen();	
	MagAddressRead(MAG_ADDR_WHO_AM_I); /*otherwise it will wrap arround the result regs*/
	for (i=0;i<10;i++)
	{
		regs[i] = MagReadContinue();	
	}
	MagReadLast();
	MagClose();
	Nop();
	Nop();
	Nop();
	Nop();
 	return;
}
#endif
// 
void MagPackData(short *input, unsigned char *output)
{

}


// ------ Uniform -ValidSettings() & -StartupSettings() functions ------

// Returns whether given settings are valid
char MagValidSettings(unsigned short rateHz, unsigned short sensitivity, unsigned long flags)
{
    // 80, 40, 20, 10, 5, 2, 1
    return (rateHz == 80 || rateHz == 40 || rateHz == 20 || rateHz == 10 || rateHz == 5 || rateHz == 2 || rateHz == 1);
}

// Starts the device with the given settings
void MagStartupSettings(unsigned short rateHz, unsigned short sensitivity, unsigned long flags)
{
    MagStartup(rateHz);
    if (flags & MAG_FLAG_FIFO_INTERRUPTS) { MagEnableInterrupts(); }
}

// ------
