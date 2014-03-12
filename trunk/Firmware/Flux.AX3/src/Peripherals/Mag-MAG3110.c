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
#include "myI2C.h"

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
#define USE_AND_OR
#include "myI2C.h"
#define LOCAL_I2C_RATE		((OSCCONbits.COSC==1)? 72 : 18)		/*200kHz for this device, controls baud*/
#define MAG_ADDRESS			0x1C	/*I2C address*/
#define MAG_MASK_READ  		0x01 	/*I2C_READ_MASK*/
#define MAG_DEVICE_ID		0xC4 	/*Staic response*/

// I2C functions for using the Mag.c code
#define MagIdleI2C        myI2CIdle
#define MagStartI2C       myI2CStart
#define MagWriteI2C       myI2Cputc
#define MagStopI2C        myI2CStop
#define MagAckI2C         myI2CAck
#define MagNackI2C 	      myI2CNack
#define MagReadI2C        myI2Cgetc
#define MagRestartI2C     myI2CRestart
#define MagOpenI2C()	  myI2COpen()
#define MagWaitStartI2C() WaitStartmyI2C()
#define MagWaitStopI2C()  WaitStopmyI2C()
#define MagWaitRestartI2C() WaitRestartmyI2C()
#define MagCloseI2C()	  myI2CClose()

// I2C - (OR register in MagAddressX with Mag_MASK_BURST)
#define MagOpen()              MagOpenI2C();MagStartI2C(); MagWaitStartI2C();
#define MagAddressRead(_r)     MagWriteI2C(MAG_ADDRESS); MagWriteI2C((_r)); MagRestartI2C(); MagWaitStartI2C(); MagWriteI2C(MAG_ADDRESS | MAG_MASK_READ);
#define MagAddressWrite(_r)    MagWriteI2C(MAG_ADDRESS); MagWriteI2C((_r)); 
#define MagReadContinue()      MagReadI2C(); MagAckI2C()
#define MagReadLast()          MagReadI2C(); MagNackI2C()
#define MagWrite(_v)           MagWriteI2C((_v));
#define MagClose()             MagStopI2C(); MagWaitStopI2C();MagCloseI2C();
#define MagReopen()            MagRestartI2C(); MagWaitRestartI2C();


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
	MagOpen();
	MagAddressWrite(MAG_ADDR_OFF_X_MSB);	/* Offset regs*/
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_X_MSB */
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_X_LSB */
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_Y_MSB */
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_Y_LSB */
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_Z_MSB */
	MagWrite(0b00000000);	/*	MAG_ADDR_OFF_Z_LSB */
	MagReopen();
	MagAddressWrite(MAG_ADDR_CTRL_REG1);
	/*MAG_ADDR_CTRL_REG1*/
	{
		unsigned char ctrl_reg1 = 0;
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
			default 	:	{ctrl_reg1 = 0b01100001;magRate = 10;	break;}/* 10Hz OSR 16, 137uA */
		}
		MagWrite(ctrl_reg1); /*
		b7-5	:	DR[2:0]	Data rate selection. Default value: 000.
		b4-3	:	OS [1:0] This register configures the over sampling ratio or measurement integration time. Default value: 00.
		b2		:	FR Fast Read selection. Default value: 0: The full 16-bit values are read. 1: Fast Read, 8-bit values read from the MSB registers.
		b1		:	TM Trigger immediate measurement. 0: Normal operation based on AC condition. 1: Trigger measurement.
		b0		:	AC	0: STANDBY mode. 1: ACTIVE mode. */
	}
	/*MAG_ADDR_CTRL_REG2*/
	MagWrite(0b00000000); /*
	b7		: 	AUTO_MRST_EN Automatic Magnetic Sensor Reset. 0: Automatic Magnetic sensor resets off. 1: Automatic Magnetic sensor resets on.
	b5		:	RAW 0: Normal mode: data values are corrected by the user offset register values. 1: Raw mode: data values are not corrected by the user offset register values.
	b4		:	Mag_RST 0: Reset cycle not active. 1: Reset cycle initiate or Reset cycle busy/active.*/
	MagClose(); 
	return;
}

// Shutdown the device to standby mode (standby mode, interrupts disabled)
void MagStandby(void)
{
	MagOpen();
	MagAddressWrite(MAG_ADDR_CTRL_REG1);
	MagWrite(0b01100000);/* 10Hz OSR 16, 137uA, continuous normal sampling ON
	b7-5	:	DR[2:0]	Data rate selection. Default value: 000.
	b4-3	:	OS [1:0] This register configures the over sampling ratio or measurement integration time. Default value: 00.
	b2		:	FR Fast Read selection. Default value: 0: The full 16-bit values are read. 1: Fast Read, 8-bit values read from the MSB registers.
	b1		:	TM Trigger immediate measurement. 0: Normal operation based on AC condition. 1: Trigger measurement.
	b0		:	AC	0: STANDBY mode. 1: ACTIVE mode. */
	/*MAG_ADDR_CTRL_REG2*/	
	MagClose();
}

/*Note: The samples are 2's compliment and 16bit*/
void MagSingleSample(mag_t *value)
{
	MagOpen();	
	MagAddressRead(MAG_ADDR_OUT_X_MSB);
	if (value == NULL)
	{
		MagReadContinue();MagReadContinue();MagReadContinue();
		MagReadContinue();MagReadContinue();MagReadLast();
	}
	else
	{
	value->xh = MagReadContinue();
	value->xl = MagReadContinue();
	value->yh = MagReadContinue();
	value->yl = MagReadContinue();
	value->zh = MagReadContinue();
	value->zl = MagReadLast();
	}
	MagClose();
	return;
}

// Enable interrupts
void MagEnableInterrupts(unsigned char flags, unsigned char pinMask)
{
	// The Magnetometer is always enabled when continuous sampling
	// The INT pin is cleared (active high) by reading the data regs
	MagSingleSample(NULL);
	MAG_CN_INT = 1;
	CN_INTS_ON();
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

// 
void MagPackData(short *input, unsigned char *output)
{

}
