/* 
 * Copyright (c) 2009-2011, Newcastle University, UK.
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
// Karim Ladha and Dan Jackson 2011.
// Adapted to be more user friendly, KL 01-12-11 


// Includes
#include "HardwareProfile.h"
#include <stdio.h>
#include <TimeDelay.h>
#include "Peripherals/Gyro.h"

// Defined in HW profile
#ifdef USE_GYRO 

#ifdef GYRO_I2C_MODE

	#include "Utils/swi2c.h"

	#define GYRO_ADDRESS	0xD2
	
	#define GYRO_MASK_READ  0x01 /*I2C_READ_MASK*/
	//#define GYRO_MASK_WRITE 0x00 /*OR to write - not needed*/
	#define GYRO_MASK_BURST 0x80 
	
	// I2C functions for using the GYRO.c code
	#define GYROIdleI2C        myi2cIdle
	#define GYROStartI2C       myi2cStart
	#define GYROWriteI2C       myi2cputc
	#define GYROStopI2C        myi2cStop
	#define GYROAckI2C         myi2cAck
	#define GYRONackI2C 	   myi2cNack
	#define GYROReadI2C        myi2cgetc
	#define GYRORestartI2C     myi2cRestart
	// Not used
	#define GYROWaitStartI2C()
	#define GYROWaitStopI2C()  


	// I2C - (OR register in GyroAddressX with GYRO_MASK_BURST)
	#define GyroOpen()              GYROStartI2C(); GYROWaitStartI2C();
	#define GyroAddressRead(_r)     GYROWriteI2C(GYRO_ADDRESS); GYROWriteI2C((_r)); GYRORestartI2C(); GYROWaitStartI2C(); GYROWriteI2C(GYRO_ADDRESS | GYRO_MASK_READ);
	#define GyroAddressWrite(_r)    GYROWriteI2C(GYRO_ADDRESS); GYROWriteI2C((_r)); 
	#define GyroReadContinue()      GYROReadI2C(); GYROAckI2C()
	#define GyroReadLast()          GYROReadI2C(); GYRONackI2C()
	#define GyroWrite(_v)           GYROWriteI2C((_v));
	#define GyroClose()             GYROStopI2C(); GYROWaitStopI2C();
	#define GyroReopen()            GYRORestartI2C(); GYROWaitStartI2C();

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
		#define GyroOpen()              OpenSPIGYRO(); GY_CS = 0;
		#define GyroAddressRead(_r)     WriteSPIGYRO((_r) | GYRO_MASK_READ)
		#define GyroAddressWrite(_r)    WriteSPIGYRO((_r))
		#define GyroReadContinue()      ReadSPIGYRO()
		#define GyroReadLast()          ReadSPIGYRO()
		#define GyroWrite(_v)           WriteSPIGYRO((_v))
		#define GyroClose()             GY_CS = 1; CloseSPIGYRO();
		#define GyroReopen()			GY_CS = 1; Delay10us(1); GY_CS = 0;
		
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


unsigned char GyroReadDeviceId(void)
{
	unsigned char id;	
	GyroOpen();	
	GyroAddressRead(GYRO_WHO_AM_I);
	id = GyroReadLast();
	GyroClose();
	return id;
}

void GyroStartup(void)
{
	GyroOpen();	
	GyroAddressWrite(GYRO_CTRL_REG1 | GYRO_MASK_BURST);
	GyroWrite(0b11111111);  // GYRO_CTRL_REG1 - full rate, full bw, no power down, all axis on
	GyroWrite(0b00001001);  // GYRO_CTRL_REG2 - normal mode, minimal high pass filter
	GyroWrite(0b00000000);  // GYRO_CTRL_REG3 - no interrupts selected, push pull o/p, active high
	GyroWrite(0b00110000);  // GYRO_CTRL_REG4 - 4 wire SPI, normal data format, self test off, 2000dps
	GyroWrite(0b00000000);  // GYRO_CTRL_REG5 - normal mode, FIFO off, high pass filt off
	GyroClose();
}

void GyroStartupFifoInterrupts(void)
{
	// Startup with fifo in stream mode
	GyroOpen();	
	GyroAddressWrite(GYRO_CTRL_REG1 | GYRO_MASK_BURST);
	GyroWrite(0b00111111);  // GYRO_CTRL_REG1 - 100Hz, full bw, no power down, all axis on
	GyroWrite(0b00001001);  // GYRO_CTRL_REG2 - normal mode, 0.01 Hz high pass filter
	GyroWrite(0b00000100);  // GYRO_CTRL_REG3 - FIFO Watermark and FIFO Overrun interrupts INT2 data ready selected, INT1 active high off
	GyroWrite(0b00110000);  // GYRO_CTRL_REG4 - 4 wire SPI, normal data format, self test off, 2000dps
	GyroWrite(0b01000101);  // GYRO_CTRL_REG5 - normal mode, FIFO on, high pass filt on for data and ints
	GyroClose();
	// Setup watermark and stream mode
	DelayMs(1);
	GyroOpen();	
	GyroAddressWrite(GYRO_FIFO_CTRL_REG | GYRO_MASK_BURST);
	GyroWrite(0b01000000 | 25);	// Stream to fifo mode - watermark = 25 of 32
	GyroClose();
	// Empty fifo
	DelayMs(1);
	GyroReadFIFO(NULL,32);
	// Enable the data ready interrupt
	GY_INT2_IF = 0;
	GY_INT2_IE = 1;
}

void GyroStandby(void)
{
	GyroOpen();	
	GyroAddressWrite(GYRO_CTRL_REG1 | GYRO_MASK_BURST);
	GyroWrite(0b00000000);  // GYRO_CTRL_REG1 - no axes enabled, power down mode, bandwidth and data rate (100 Hz ODR, 12.5 cut-off)
	GyroWrite(0b00001001);  // GYRO_CTRL_REG2 - normal mode, minimal high pass filter
	GyroWrite(0b00000000);  // GYRO_CTRL_REG3 - no interrupts selected, push pull o/p, active high
	GyroWrite(0b00000000);  // GYRO_CTRL_REG4 - 4 wire SPI, normal data format, self test off, 250dps
	GyroWrite(0b00000000);  // GYRO_CTRL_REG5 - normal mode, FIFO off, high pass filt off
	GyroClose();
}

signed char GyroReadTemp(void)
{
	unsigned char temp;
	GyroOpen();	
	GyroAddressRead(GYRO_OUT_TEMP);
	temp = GyroReadLast();
	GyroClose();
	return (GYRO_TEMP_OFFSET - temp);
}

void GyroSingleSample(gyro_t *value)
{
	GyroOpen();	
	GyroAddressRead(GYRO_OUT_X_L | GYRO_MASK_BURST);
	value->x = GyroReadContinue();
	value->x |= ((unsigned int)GyroReadContinue()<<8);
	value->y = GyroReadContinue();
	value->y |= ((unsigned int)GyroReadContinue()<<8);
	value->z = GyroReadContinue();
	value->z |= ((unsigned int)GyroReadLast()<<8);

#ifdef CONVERT_AXIS_CWA2	
	// WAX+GYRO axis conversion (to match CWA2): Ax' = Ay, Ay' = Ax, Az' = -Az, Gx = -Gx, Gy = -Gy
	value->x = -value->x;
	value->y = -value->y;
#endif
	
	GyroClose();
}

unsigned char GyroReadFifoLength(void)
{
	unsigned char availableFifo;
	GyroOpen();	
	GyroAddressRead(GYRO_FIFO_SRC_REG);
	availableFifo = GyroReadLast();
	availableFifo &= 0x1f;				// 0-31 elements
	GyroClose();
	return availableFifo;
}

// Read at most 'maxEntries' 3-axis samples from the Gyro FIFO into the specified RAM buffer
unsigned char GyroReadFIFO(gyro_t *gyroBuffer, unsigned char maxEntries)
{
	unsigned char availableFifo;
	unsigned char numRead;
	unsigned char *p;
	int i;

    // Check if we have any room to fit
    if (maxEntries == 0) { return 0; }
	numRead = 0;
	p = (unsigned char *)gyroBuffer;

	// Read number of available samples	in FIFO
	GyroOpen();	
	GyroAddressRead(GYRO_FIFO_SRC_REG);
	availableFifo = GyroReadLast();
	availableFifo &= 0x1f;				// 0-31 elements

	/*This has been changed to use the wrap arround feature of the FIFO*/
	GyroReopen();
	GyroAddressRead(GYRO_OUT_X_L | GYRO_MASK_BURST);

	// While data is in the FIFO and we have space in the buffer
	while (availableFifo > 1 && maxEntries > 1)
	{
		// Data 0 contains LSB. Data 1 contains MSB
        if (gyroBuffer != NULL)
        {
			*p++ = GyroReadContinue();		// XL
			*p++ = GyroReadContinue();		// XH
			*p++ = GyroReadContinue();		// YL
			*p++ = GyroReadContinue();		// YH
			*p++ = GyroReadContinue();		// ZL
			*p++ = GyroReadContinue();		// ZH
        }
        else
        {
            // Empty one entry from FIFO if NULL pointer (discarding packet)
			GyroReadContinue(); GyroReadContinue(); GyroReadContinue(); GyroReadContinue(); GyroReadContinue(); GyroReadContinue(); 
        }
		// Update status
		maxEntries--;
		availableFifo--;
		numRead++;
	}

	/*Last sample uses a ReadLast() so it cant be in the while loop*/
	// Data 0 contains LSB. Data 1 contains MSB
	      if (gyroBuffer != NULL)
	      {
			*p++ = GyroReadContinue();		// XL
			*p++ = GyroReadContinue();		// XH
			*p++ = GyroReadContinue();		// YL
			*p++ = GyroReadContinue();		// YH
			*p++ = GyroReadContinue();		// ZL
			*p++ = GyroReadLast();		// ZH
	      }
	      else
	      {
	         // Empty one entry from FIFO if NULL pointer (discarding packet)
			 GyroReadContinue(); GyroReadContinue(); GyroReadContinue(); GyroReadContinue(); GyroReadContinue(); GyroReadLast(); 
	      }
	// Update status
	maxEntries--;
	availableFifo--;
	numRead++;

	GyroClose();

	return numRead;
}

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

#else
	#warning "You compiled this .c file but didn't enable the gyro in HW profile"
#endif
