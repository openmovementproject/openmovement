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

// accel.c - ADXL345 accelerometer interface
// Dan Jackson, Karim Ladha, Cas Ladha, 2010-2011.


#ifndef ACCEL_H
#define ACCEL_H


// ADXL sampling rate codes (current shown as normal / low-power mode)
#define ACCEL_RATE_3200     0x0f    // 145 uA (no low-power mode)
#define ACCEL_RATE_1600     0x0e    // 100 uA (no low-power mode)
#define ACCEL_RATE_800      0x0d    // 145 uA (no low-power mode)
#define ACCEL_RATE_400      0x0c    // 145 uA / 100 uA
#define ACCEL_RATE_200      0x0b    // 145 uA /  65 uA
#define ACCEL_RATE_100      0x0a    // 145 uA /  55 uA
#define ACCEL_RATE_50       0x09    // 100 uA /  50 uA
#define ACCEL_RATE_25       0x08    //  65 uA /  40 uA
#define ACCEL_RATE_12_5     0x07    //  55 uA /  40 uA
#define ACCEL_RATE_6_25     0x06    //  40 uA (no low-power mode)

// Sampling rate modifiers
#define ACCEL_RATE_NORMAL   0x00    // Normal mode
#define ACCEL_RATE_LOW_POWER 0x10   // Low-power mode, only valid for rates from 12.5 Hz - 400 Hz

// In this API, the top two bits of the sampling rate value are used to determine the acceleromter's range.
// (For backwards compatibility we treat the top two bits as the reverse of the ADXL's internal range values)
// 0=±16g, 1=±8g, 2=±4g, 3=±2g
#define ACCEL_RANGE_16G     0x00
#define ACCEL_RANGE_8G      0x40
#define ACCEL_RANGE_4G      0x80
#define ACCEL_RANGE_2G      0xC0

// Rate calculations
#define ACCEL_FREQUENCY_FOR_RATE(_f)   (3200 / (1 << (15-((_f) & 0x0f))))
#define ACCEL_IS_VALID_RATE(_v)        (((_v & 0x3f) >= 0x6 && (_v & 0x3f) <= 0xf) || ((_v & 0x3f) >= 0x17 && (_v & 0x3f) <= 0x1c))
#define ACCEL_IS_LOW_POWER_RATE(_v)    ((_v & 0x3f) & 0x10)

// Default settings
#define ACCEL_DEFAULT_RATE      (ACCEL_RATE_100 | ACCEL_RANGE_8G)   // ACCEL_RATE_LOW_POWER
#define ACCEL_DEFAULT_WATERMARK 25          // up to 31


#define ACCEL_INT_SOURCE_DATA_READY   0x80
#define ACCEL_INT_SOURCE_SINGLE_TAP   0x40
#define ACCEL_INT_SOURCE_DOUBLE_TAP   0x20
#define ACCEL_INT_SOURCE_ACTIVITY     0x10
#define ACCEL_INT_SOURCE_INACTIVITY   0x08
#define ACCEL_INT_SOURCE_FREE_FALL    0x04
#define ACCEL_INT_SOURCE_WATERMARK    0x02
#define ACCEL_INT_SOURCE_OVERRUN      0x01


#define ACCEL_BYTES_PER_SAMPLE 6
#define ACCEL_MAX_FIFO_SAMPLES  (32)
#define ACCEL_MAX_FIFO_BYTES    (ACCEL_MAX_FIFO_SAMPLES * ACCEL_BYTES_PER_SAMPLE)


// Types
typedef struct
{
	int x; int y; int z;
}accel_t;

// Read device ID (should be ACCEL_DEVICE_ID = 0xE5)
#define ACCEL_DEVICE_ID 0xE5
unsigned char AccelReadDeviceId(void);

// Initialize the ADXL
extern void AccelStartup(unsigned char samplingRate);					

// Shutdown the ADXL (standby mode)
extern void AccelStandby(void);

// Enable interrupts
void AccelEnableInterrupts(unsigned char flags, unsigned char pinMask);

// Reads a 3-axis value from the ADXL into the specified address.
extern void AccelSingleSample(short *value);

// Read FIFO queue length
extern unsigned char AccelReadFifoLength(void);

// Reads the ADXL FIFO (bytes = ADXL_BYTES_PER_SAMPLE * entries)
extern unsigned char AccelReadFIFO(short *buffer, unsigned char maxEntries);

// Read tap status - b0 = Tap-Z
extern unsigned char AccelReadTapStatus(void);

// Read interrupt source - b1 = watermark, b5=double-tap, b6=single-tap
extern unsigned char AccelReadIntSource(void);

// Debug dump registers
extern void AccelDebugDumpRegisters(void);

// Returns the current rate code
unsigned char AccelRate(void);

// Returns the current frequency
unsigned short AccelFrequency(void);

// Packs a buffer of 16-bit (x,y,z) values into an output buffer (4 bytes per entry)
extern void AccelPackData(short *input, unsigned char *output);

// Unpack DWORD - Sign-extend 10-bit value, adjust for exponent
#define ACCEL_UNPACK_XVALUE(_a) ( (signed short)((unsigned short)0xffc0 & (unsigned short)((_a) <<  6)) >> (6 - ((unsigned char)((_a) >> 30))) )
#define ACCEL_UNPACK_YVALUE(_a) ( (signed short)((unsigned short)0xffc0 & (unsigned short)((_a) >>  4)) >> (6 - ((unsigned char)((_a) >> 30))) )
#define ACCEL_UNPACK_ZVALUE(_a) ( (signed short)((unsigned short)0xffc0 & (unsigned short)((_a) >> 14)) >> (6 - ((unsigned char)((_a) >> 30))) )

#endif

