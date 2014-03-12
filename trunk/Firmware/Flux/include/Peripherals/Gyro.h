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

// gyro.h - L3G4200 Gyroscope interface.
// Karim Ladha, Dan Jackson 2011-2012.
// Adapted to be more user friendly, KL 01-12-11 
/*
Revisions:
01-12-11 : KL - Added FIFO capability
10-02-12 : KL - Reworked for smaller and better functionality + added rate/range vars
*/

#ifndef GYRO_H
#define GYRO_H

#include "HardwareProfile.h"

#ifdef USE_GYRO
	// Defined in HW profile
	#ifndef GYRO_FIFO_WATERMARK 
		#warning "Watermark not defined, assuming 25"
		#define GYRO_FIFO_WATERMARK 25
	#endif
	
	#ifndef GYRO_IS_L3G4200D
		#warning "Assuming device is L3G4200D"
		#define GYRO_IS_L3G4200D
	#endif
#endif

#ifdef GYRO_IS_L3G4200D
	/*CRL_REG1*/
	#define GYRO_RATE_800	0xC0
	#define GYRO_RATE_400	0x80
	#define GYRO_RATE_200	0x40
	#define GYRO_RATE_100   0x00
	#define GYRO_BW_FULL	0x30
	#define GYRO_BW_SEL_0	0x00
	#define GYRO_BW_SEL_1	0x10
	#define GYRO_BW_SEL_2	0x20
	#define GYRO_BW_SEL_3	0x30
	#define GYRO_POWER_ON	0x08	
	#define GYRO_3AXIS_ON	0x07	
	#define GYRO_POWER_OFF	0x00
	/*CRL_REG2*/
	#define GYRO_HP_NORMAL		0x20
	#define GYRO_HP_REF_ONLY	0x10
	#define GYRO_HP_RESET		0x00
	#define GYRO_AUTO_RESET		0x30
	#define GYRO_HP_BW_DIV_12_5	0x00
	#define GYRO_HP_BW_DIV_25	0x01
	#define GYRO_HP_BW_DIV_50	0x02
	#define GYRO_HP_BW_DIV_100	0x03
	#define GYRO_HP_BW_DIV_200	0x04
	#define GYRO_HP_BW_DIV_400	0x05
	#define GYRO_HP_BW_DIV_1000	0x06
	#define GYRO_HP_BW_DIV_2000	0x07
	#define GYRO_HP_BW_DIV_4000	0x08
	#define GYRO_HP_BW_DIV_8000	0x09
	/*CRL_REG3*/
	#define GYRO_FIFO_INT_ON	0x04	
	#define GYRO_NO_INTS		0x00
	/*CRL_REG4*/
	#define GYRO_BDU_ON		0x80
	#define GYRO_250DPS 	0x00
	#define GYRO_500DPS 	0x10
	#define GYRO_2000DPS 	0x20
	/*CRL_REG5*/
	#define GYRO_FIFO_ON	0x40
	#define GYRO_NO_FILT	0x00
	#define GYRO_LP_ON		0x02
	#define GYRO_BP_ON		0x13
	#define GYRO_HP_ON 		0x11
	
	// For compatibility - SINGLE SAMPLE
	#define GYRO_STARTUP_SINGLE_SAMPLE_DEFAULT	GYRO_RATE_100|GYRO_BW_FULL|GYRO_POWER_ON|GYRO_3AXIS_ON,\
												GYRO_HP_NORMAL|GYRO_HP_BW_DIV_100,\
												GYRO_NO_INTS,\
												GYRO_2000DPS,\
												GYRO_NO_FILT
	
	// For compatibility - FIFO
	#define GYRO_STARTUP_FIFO_DEFAULT			GYRO_RATE_100|GYRO_BW_FULL|GYRO_POWER_ON|GYRO_3AXIS_ON,\
												GYRO_HP_NORMAL|GYRO_HP_BW_DIV_100,\
												GYRO_FIFO_INT_ON,\
												GYRO_2000DPS,\
												GYRO_FIFO_ON|GYRO_NO_FILT
	// For compatibility - Power down
	#define GYRO_STANDBY						GYRO_RATE_100|GYRO_BW_FULL|GYRO_POWER_OFF,\
												GYRO_HP_NORMAL|GYRO_HP_BW_DIV_100,\
												GYRO_NO_INTS,\
												GYRO_2000DPS,\
												GYRO_NO_FILT
#endif // if DEVICE_IS_L3G4200D


// Data types
typedef union
{
    struct { short x, y, z; };
    struct { unsigned char xl, xh, yl, yh,  zl, zh; };
    short values[3];
} gyro_t;

#define GYRO_BYTES_PER_SAMPLE 6
#define GYRO_MAX_FIFO_SAMPLES  (32)

// Gyro detection
extern char gyroPresent;       // Gyro present (call GyroReadDeviceId() once to set this)

// Verify device ID - call first to set gyroPresent var
extern unsigned char GyroVerifyDeviceId(void);

// Get device parameters - valid after setup (init = 0)
extern unsigned int GyroFrequency(void);
extern unsigned int GyroRange(void);

// Initialisation for single sample mode - no interrupts
extern void GyroStartupWith(unsigned char ctrlreg1,unsigned char ctrlreg2,unsigned char ctrlreg3,unsigned char ctrlreg4,unsigned char ctrlreg5);	
// Predefined constants
#define GyroStartup()						GyroStartupWith(GYRO_STARTUP_SINGLE_SAMPLE_DEFAULT)
#define GyroStartupFifoInterrupts()			GyroStartupWith(GYRO_STARTUP_FIFO_DEFAULT)
#define GyroStandby()						GyroStartupWith(GYRO_STANDBY)

// Read a single xyz value from the device
extern void GyroSingleSample(gyro_t *value);			

// Returns how many bytes are in the fifo
extern unsigned char GyroReadFifoLength(void);

// Reads specified number of bytes from the fifo to the buffer if available - returns number read
extern unsigned char GyroReadFIFO(gyro_t *gyroBuffer, unsigned char maxEntries);

// The internal temperature register in celcius - needs calibrating with the GYRO_TEMP_OFFSET
extern signed char GyroReadTemp(void);

// Reads a device register - debugging
extern unsigned char GyroReadRegister(unsigned char reg);
// Reads multiple device registers - debugging
// note: fifo causes re-wrap about x,y,z values
extern void GyroReadRegisters(unsigned char reg, unsigned char count, unsigned char *buffer);


// ------ Uniform -ValidSettings() & -StartupSettings() functions ------

// Flags for settings
#define GYRO_FLAG_FIFO_INTERRUPTS 0x0001

// Returns whether given settings are valid
char GyroValidSettings(unsigned short rateHz, unsigned short sensitivityDps, unsigned long flags);

// Starts the device with the given settings
void GyroStartupSettings(unsigned short rateHz, unsigned short sensitivityDps, unsigned long flags);

// ------


#endif
//EOF

