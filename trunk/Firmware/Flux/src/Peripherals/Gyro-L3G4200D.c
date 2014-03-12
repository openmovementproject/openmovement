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

// gyro.c - L3G4200 Gyroscope interface.
// Karim Ladha and Dan Jackson 2011-2012.
// Adapted to be more user friendly, KL 01-12-11 


// Includes
#include "HardwareProfile.h"
#include <stdio.h>
#include <TimeDelay.h>

// Defined in HW profile
#ifdef USE_GYRO 

#include "Peripherals/Gyro.h"

char gyroPresent = 0;
static unsigned int gyroFrequency = 0;
static unsigned int gyroRange = 0;

#define GYRO_DEVICE_ID 0xD3	/*Static responce*/

#ifdef GYRO_I2C_MODE
	// Needed, or else i2c.h fails
	#define USE_AND_OR

	// Alternate I2C bus?
	#ifdef GYRO_ALTERNATE_I2C
		#define MY_I2C_OVERIDE	GYRO_ALTERNATE_I2C
	#endif
	#include "Peripherals/myI2C.h"

	#define LOCAL_I2C_RATE		I2C_RATE_400kHZ
	#define GYRO_ADDRESS		0xD2	/*I2C address*/
	#define GYRO_MASK_READ  	0x01 	/*I2C_READ_MASK*/
	#define GYRO_MASK_BURST		0x80	/*This allows the same code to be used for SPI routines on this device*/

	// I2C 
	#define CUT_DOWN_I2C_CODE_SIZE
	#ifndef CUT_DOWN_I2C_CODE_SIZE
	#define GyroOpen()              myI2COpen();myI2CStart(); WaitStartmyI2C();
	#define GyroAddressRead(_r)     myI2Cputc(GYRO_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(GYRO_ADDRESS | I2C_READ_MASK);
	#define GyroAddressWrite(_r)    myI2Cputc(GYRO_ADDRESS); myI2Cputc((_r)); 
	#define GyroReadContinue()      myI2Cgetc(); myI2CAck()
	#define GyroReadLast()          myI2Cgetc(); myI2CNack()
	#define GyroWrite(_v)           myI2Cputc((_v));
	#define GyroClose()             myI2CStop(); WaitStopmyI2C();myI2CClose();
	#define GyroReopen()            myI2CRestart(); WaitRestartmyI2C();
	#else
	void GyroOpen(void)						{myI2COpen();myI2CStart(); WaitStartmyI2C();}
	void GyroAddressRead(unsigned char _r)	{myI2Cputc(GYRO_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(GYRO_ADDRESS | I2C_READ_MASK);}
	void GyroAddressWrite(unsigned char _r)	{myI2Cputc(GYRO_ADDRESS); myI2Cputc((_r)); }
	unsigned char GyroReadContinue(void)   	{unsigned char ret = myI2Cgetc(); myI2CAck();return ret;}
	unsigned char GyroReadLast(void)       	{unsigned char ret = myI2Cgetc(); myI2CNack();return ret;}
	void GyroWrite(unsigned char _v)       	{myI2Cputc((_v));}
	void GyroClose(void )             		{myI2CStop(); WaitStopmyI2C();myI2CClose();}
	void GyroReopen(void )            		{myI2CRestart(); WaitRestartmyI2C();}
	#endif

#else

	#ifdef USE_PIC18
			#error "PIC18 SPI Gyro interface not written"
	#else
	
		// You can use read-modify-write sequences on the SPI buffer regs on a pic24
		// Needed, or else spi.h fails
		#define USE_AND_OR
		#include <spi.h>
				
		// SPI routines
		inline void WriteSPIGYRO(unsigned int data)	{
													unsigned int dummy;
													if(SPI1_Rx_Buf_Full)
														dummy = SPI1BUF; 				// Clear flag
													while(SPI1_Tx_Buf_Full);			// Dont write till last byte is txed	
													SPI1BUF=(data);
													while(!SPI1_Rx_Buf_Full);			// Wait till byte clocked in
													dummy = SPI1BUF;					// Read it
													}												
		inline unsigned int ReadSPIGYRO(void)			{
													unsigned int data;
													if(SPI1_Rx_Buf_Full)
														data = SPI1BUF; 				// Clear flag
													while(SPI1_Tx_Buf_Full);			// Dont write till last byte is txed	
													SPI1BUF=(0xff);
													while(!SPI1_Rx_Buf_Full);			// Wait till byte clocked in
													data = SPI1BUF;						// Read it
													return (data&0xff);
													}	
		
		#define CloseSPIGYRO()		CloseSPI1()
		#define OpenSPIGYRO(){		OpenSPI1(		/* Turn on SPI module 1 */\
											/*config 1*/\
											(ENABLE_SCK_PIN |\
											ENABLE_SDO_PIN |\
											SPI_MODE8_ON |\
											SPI_SMP_OFF/*?*/ |\
											SPI_CKE_OFF |\
											CLK_POL_ACTIVE_LOW |\
											MASTER_ENABLE_ON |\
											SEC_PRESCAL_1_1 |\
											PRI_PRESCAL_4_1),\
											/*config 2*/\
											(FRAME_ENABLE_OFF |\
											FRAME_SYNC_OUTPUT |\
											FRAME_SYNC_ACTIVE_LOW |\
											SPI_FRM_PULSE_PREV_CLK |\
											SPI_ENH_BUFF_DISABLE ),\
											/*config 3*/\
											(SPI_ENABLE |\
											SPI_IDLE_STOP |\
											SPI_RX_OVFLOW_CLR ));}
		// Defines
		#define GYRO_MASK_READ             0x80 /*OR to read*/
		//#define GYRO_MASK_WRITE            0x00 /*OR to write - not needed*/
		#define GYRO_MASK_BURST            0x40 /*OR for burst*/
		
		// SPI - (OR register in GyroAddressX with GYRO_MASK_BURST)
		#define GyroOpen()              OpenSPIGYRO(); GYRO_CS = 0;
		#define GyroAddressRead(_r)     WriteSPIGYRO((_r) | GYRO_MASK_READ)
		#define GyroAddressWrite(_r)    WriteSPIGYRO((_r))
		#define GyroReadContinue()      ReadSPIGYRO()
		#define GyroReadLast()          ReadSPIGYRO()
		#define GyroWrite(_v)           WriteSPIGYRO((_v))
		#define GyroClose()             GYRO_CS = 1; CloseSPIGYRO();
		#define GyroReopen()			GYRO_CS = 1; Delay10us(1); GYRO_CS = 0;
		
	#endif

#endif


// Device specific defines - register addresses
#define GYRO_TEMP_OFFSET 50
//GYRO_Reserved 		 0x00-0x0E and 0x10-0x1F
#define GYRO_WHO_AM_I 		0x0F   // READ ONLY,  default=0b11010011
#define GYRO_CTRL_REG1 		0x20   // READ/WRITE, default=0b00000111
#define GYRO_CTRL_REG2 		0x21   // READ/WRITE, default=0b00000000
#define GYRO_CTRL_REG3 		0x22   // READ/WRITE, default=0b00000000
#define GYRO_CTRL_REG4 		0x23   // READ/WRITE, default=0b00000000
#define GYRO_CTRL_REG5 		0x24   // READ/WRITE, default=0b00000000
#define GYRO_REFERENCE 		0x25   // READ/WRITE, default=0b00000000
#define GYRO_OUT_TEMP 		0x26   // READ ONLY
#define GYRO_STATUS_REG		0x27   // READ ONLY
#define GYRO_OUT_X_L 		0x28   // READ ONLY
#define GYRO_OUT_X_H 		0x29   // READ ONLY
#define GYRO_OUT_Y_L 		0x2A   // READ ONLY
#define GYRO_OUT_Y_H 		0x2B   // READ ONLY
#define GYRO_OUT_Z_L 		0x2C   // READ ONLY
#define GYRO_OUT_Z_H 		0x2D   // READ ONLY
#define GYRO_FIFO_CTRL_REG  0x2E   // READ/WRITE, default=0b00000000
#define GYRO_FIFO_SRC_REG 	0x2F   // READ ONLY
#define GYRO_INT1_CFG 		0x30   // READ/WRITE, default=0b00000000
#define GYRO_INT1_SRC 		0x31   // READ ONLY
#define GYRO_INT1_TSH_XH 	0x32   // READ/WRITE, default=0b00000000
#define GYRO_INT1_TSH_XL 	0x33   // READ/WRITE, default=0b00000000
#define GYRO_INT1_TSH_YH 	0x34   // READ/WRITE, default=0b00000000
#define GYRO_INT1_TSH_YL 	0x35   // READ/WRITE, default=0b00000000
#define GYRO_INT1_TSH_ZH 	0x36   // READ/WRITE, default=0b00000000
#define GYRO_INT1_TSH_ZL 	0x37   // READ/WRITE, default=0b00000000
#define GYRO_INT1_DURATION 	0x38   // READ/WRITE, default=0b00000000


// Get device current setup
unsigned int GyroFrequency(void)
{
	return gyroFrequency;
}
unsigned int GyroRange(void)
{
	return gyroRange;
}

// Query device - blocks other functions if returns false
unsigned char GyroVerifyDeviceId(void)
{
	unsigned char id;	
	GyroOpen();	
	GyroAddressRead(GYRO_WHO_AM_I);
	id = GyroReadLast();
	GyroClose();
    gyroPresent = (id == GYRO_DEVICE_ID) ? 1 : 0;
	return gyroPresent;
}

void GyroStartupWith(unsigned char ctrlreg1,unsigned char ctrlreg2,unsigned char ctrlreg3,unsigned char ctrlreg4,unsigned char ctrlreg5)
{
    if ((!gyroPresent) && (ctrlreg1 & GYRO_POWER_ON)) // Ignore startup unless its powering the device off
		{ return; }

	// Stop interrupts during config
	GYRO_INT2_IE = 0;

	// Startup by setting ctrl regs
	GyroOpen();	
	GyroAddressWrite(GYRO_CTRL_REG1 | GYRO_MASK_BURST);
	GyroWrite(0);			// GYRO_CTRL_REG1 - Device  off first
	GyroWrite(ctrlreg2);  	// GYRO_CTRL_REG2
	GyroWrite(ctrlreg3);  	// GYRO_CTRL_REG3 
	GyroWrite(ctrlreg4);  	// GYRO_CTRL_REG4 
	GyroWrite(ctrlreg5);  	// GYRO_CTRL_REG5 
	GyroClose();

	DelayMs(10);

	switch (ctrlreg1 & 0xC0) {
		case (GYRO_RATE_800) : {gyroFrequency = 800; break;}
		case (GYRO_RATE_400) : {gyroFrequency = 400; break;}
		case (GYRO_RATE_200) : {gyroFrequency = 200; break;}
		case (GYRO_RATE_100) : {gyroFrequency = 100; break;}
		default : {gyroFrequency = 0; break;}
	}
	switch (ctrlreg4 & 0x30) {
		case (GYRO_250DPS) : 	{gyroRange = 250; break;}
		case (GYRO_500DPS) :	{gyroRange = 500; break;}
		case (GYRO_2000DPS) :	{gyroRange = 2000; break;}
		default : 	{gyroRange = 0; break;}
	}

	// Fifo ints - clear fifo etc 
	if(ctrlreg3 == GYRO_FIFO_INT_ON)
	{
		// Setup watermark
		GyroOpen();	
		GyroAddressWrite(GYRO_FIFO_CTRL_REG);
		GyroWrite(0b01000000 | GYRO_FIFO_WATERMARK);// Stream to fifo mode - set water mark
		GyroClose();
	}
	// else if (ctrlreg3 == ANY_OTHER_INTERRUPTS)
	// Setup other specific regs here such as activity thresholds etc

	// Now turn on the device
	GyroOpen();	
	GyroAddressWrite(GYRO_CTRL_REG1);
	GyroWrite(ctrlreg1);  // GYRO_CTRL_REG1 - Device on
	GyroClose();

	if(ctrlreg3 == GYRO_FIFO_INT_ON)
	{
		// Empty fifo
		DelayMs(50);
		GyroReadFIFO(NULL, GYRO_MAX_FIFO_SAMPLES);
		// Enable ints
		GYRO_INT2_IP = GYRO_INT_PRIORITY;
		GYRO_INT2_IF = 0;
		GYRO_INT2_IE = 1;
	}
}

signed char GyroReadTemp(void)
{
	unsigned char temp;
    if (!gyroPresent) { return -1; }
	GyroOpen();	
	GyroAddressRead(GYRO_OUT_TEMP);
	temp = GyroReadLast();
	GyroClose();
	return (GYRO_TEMP_OFFSET - temp);
}

void GyroSingleSample(gyro_t *value)
{
    if (!gyroPresent) { value->x = 0; value->y = 0; value->z = 0; return; }
	GyroOpen();	
	GyroAddressRead(GYRO_OUT_X_L | GYRO_MASK_BURST);
	value->xl = GyroReadContinue();
	value->xh = GyroReadContinue();
	value->yl = GyroReadContinue();
	value->yh = GyroReadContinue();
	value->zl = GyroReadContinue();
	value->zh = GyroReadLast();
	GyroClose();
}

unsigned char GyroReadFifoLength(void)
{
	unsigned char availableFifo;
    if (!gyroPresent) { return 0; }
	GyroOpen();	
	GyroAddressRead(GYRO_FIFO_SRC_REG);
	availableFifo = GyroReadLast(); 
	availableFifo &= 0x3f;				// 0-31 elements
	GyroClose();
	return availableFifo;
}

// Read at most 'maxEntries' 3-axis samples from the Gyro FIFO into the specified RAM buffer
unsigned char GyroReadFIFO(gyro_t *gyroBuffer, unsigned char maxEntries)
{
	unsigned char number_in_fifo, number_read=0;
	unsigned char * ptr = (unsigned char*)gyroBuffer; // Pointer cast to bytes
	// Blocks access if not detected
	if (!gyroPresent) return 0;
	if (maxEntries == 0) return 0;

	// Read number of available samples	in FIFO
	GyroOpen();	
	GyroAddressRead(GYRO_FIFO_SRC_REG);
	number_in_fifo = GyroReadLast();
	GyroClose();
	number_in_fifo &= 0x3f;					// 0-31 elements
    if (number_in_fifo == 0) { return 0; }

	/*This uses the wrap arround feature of the FIFO*/
	GyroOpen();
	GyroAddressRead(GYRO_OUT_X_L | GYRO_MASK_BURST);

	// Loop read contents of fifo
	while ((number_in_fifo>1)&&(maxEntries>1))
	{
		if (gyroBuffer == NULL)
		{	// Empty one entry from FIFO if NULL pointer (discarding packet)
			GyroReadContinue(); GyroReadContinue(); GyroReadContinue(); 
			GyroReadContinue(); GyroReadContinue(); GyroReadContinue(); 
		}
		else
		{
			*ptr++ = GyroReadContinue();		// XL
			*ptr++ = GyroReadContinue();		// XH
			*ptr++ = GyroReadContinue();		// YL
			*ptr++ = GyroReadContinue();		// YH
			*ptr++ = GyroReadContinue();		// ZL
			*ptr++ = GyroReadContinue();		// ZH
		}
		number_in_fifo--;
		maxEntries--;
		number_read++;		
	}

	// Last sample, must use read last to close bus
	if (gyroBuffer == NULL)
	{	// Empty one entry from FIFO if NULL pointer (discarding packet)
		GyroReadContinue(); GyroReadContinue(); GyroReadContinue(); 
		GyroReadContinue(); GyroReadContinue(); GyroReadLast(); 
	}
	else
	{
			*ptr++ = GyroReadContinue();		// XL
			*ptr++ = GyroReadContinue();		// XH
			*ptr++ = GyroReadContinue();		// YL
			*ptr++ = GyroReadContinue();		// YH
			*ptr++ = GyroReadContinue();		// ZL
			*ptr++ = GyroReadLast();		// ZH		
	}
	GyroClose();
	number_read++;

	return number_read;
}

// These are debug only functions from here
unsigned char GyroReadRegister(unsigned char reg)
{
	unsigned char value;
	GyroOpen();	
	GyroAddressRead(reg);
	value = GyroReadLast();
	GyroClose();
	return value;
}

void GyroReadRegisters(unsigned char reg, unsigned char count, unsigned char *buffer)
{
	if (count > 0)
	{
		GyroOpen();	
		GyroAddressRead(reg | GYRO_MASK_BURST);
		for (; count > 1; --count)
		{
			*buffer++ = GyroReadContinue();
		}
		*buffer = GyroReadLast();
		GyroClose();
	}
	return;
}


// ------ Uniform -ValidSettings() & -StartupSettings() functions ------

// Returns whether given settings are valid
char GyroValidSettings(unsigned short rateHz, unsigned short sensitivityDps, unsigned long flags)
{
    //flags;      // ignored
    return ((rateHz == 100 || rateHz == 200 || rateHz == 400 || rateHz == 800) && (sensitivityDps == 250 || sensitivityDps == 500 || sensitivityDps == 2000));
}

// Starts the device with the given settings
void GyroStartupSettings(unsigned short rateHz, unsigned short sensitivityDps, unsigned long flags)
{
    unsigned char ctrlreg1, ctrlreg2, ctrlreg3, ctrlreg4, ctrlreg5;

    // Register 1 - Rate settings
    ctrlreg1 = GYRO_BW_FULL | GYRO_POWER_ON | GYRO_3AXIS_ON;
    switch (rateHz)
    {
        case 100: ctrlreg1 |= GYRO_RATE_100; break;
        case 200: ctrlreg1 |= GYRO_RATE_200; break;
        case 400: ctrlreg1 |= GYRO_RATE_400; break;
        case 800: ctrlreg1 |= GYRO_RATE_800; break;
    }

    // Register 2
    ctrlreg2 = GYRO_HP_NORMAL | GYRO_HP_BW_DIV_100;
    
    // Register 3 - Interrupt settings
    ctrlreg3 = 0;
    if (flags & GYRO_FLAG_FIFO_INTERRUPTS) { ctrlreg3 = GYRO_FIFO_INT_ON; }

    // Register 4 - Sensitivity settings
    ctrlreg4 = GYRO_BDU_ON;
    switch (sensitivityDps)
    {
        case 250:  ctrlreg4 |= GYRO_250DPS; break;
        case 500:  ctrlreg4 |= GYRO_500DPS; break;
        case 2000: ctrlreg4 |= GYRO_2000DPS; break;
    }
    
    // Register 5 - FIFO settings
    ctrlreg5 = GYRO_NO_FILT;
    if (flags & GYRO_FLAG_FIFO_INTERRUPTS) { ctrlreg5 |= GYRO_FIFO_ON; }

    GyroStartupWith(ctrlreg1, ctrlreg2, ctrlreg3, ctrlreg4, ctrlreg5);
}

// ------


#else
//	#warning "You compiled this .c file but didn't enable the gyro in HW profile"
#endif
