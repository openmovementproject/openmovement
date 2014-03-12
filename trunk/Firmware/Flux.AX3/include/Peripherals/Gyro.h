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
*/

// Data types
typedef union
{
    struct { short x, y, z; };
    struct { unsigned char xl, xh, yl, yh,  zl, zh; };
    short values[3];
} gyro_t;

#define GYRO_BYTES_PER_SAMPLE 6
#define GYRO_MAX_FIFO_SAMPLES  (32)
//#define GYRO_MAX_FIFO_BYTES    (GYRO_MAX_FIFO_SAMPLES * GYRO_BYTES_PER_SAMPLE)

// Gyro detection
extern char gyroPresent;       // Gyro present (call GyroReadDeviceId() once to set this)


// TODO: Make this return the current gyro frequency
#define GyroFrequency() (100)


// Verify device ID
extern unsigned char GyroVerifyDeviceId(void);

// Initialisation for single sample mode - no interrupts
extern void GyroStartup(void);	

// Initialisation to use FIFO and fifo interrupt enable
extern void GyroStartupFifoInterrupts(void);

// Shutdown the device to conserve power
extern void GyroStandby(void);

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

