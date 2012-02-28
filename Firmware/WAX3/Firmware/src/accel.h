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

// accel.h - ADXL345 accelerometer interface
// Dan Jackson, 2010-2011.


#ifndef ACCEL_H
#define ACCEL_H


/*
typedef struct { unsigned char rateCode, int outputDataRate, char lowPower, unsigned char currentUA; } Rate;
const Rate[] rates =
{
// rateCode, output (Hz), LP, Idd (uA)
    {  0x0f,        3200,  0, 145 }, 
    {  0x0e,        1600,  0, 100 }, 
    {  0x0d,         800,  0, 145 }, 
    {  0x0c,         400,  0, 145 }, 
    {  0x0b,         200,  0, 145 }, 
    {  0x0a,         100,  0, 145 }, 
    {  0x09,          50,  0, 100 }, 
    {  0x08,          25,  0,  65 }, 
    {  0x07,        25/2,  0,  55 }, 
    {  0x06,        25/4,  0,  40 }, 

    {  0x1c,         400,  1, 100 }, 
    {  0x1b,         200,  1,  65 }, 
    {  0x1a,         100,  1,  55 }, 
    {  0x19,          50,  1,  50 }, 
    {  0x18,          25,  1,  40 }, 
    {  0x17,        25/2,  1,  40 }, 
};
*/


#if (DEVICE_TYPE==2)
#define ACCEL_FIXED_RATE 0x8A		// 100 Hz, non-low power, +- 4g
#endif

#ifdef ACCEL_FIXED_RATE
#define ACCEL_DEFAULT_RATE ACCEL_FIXED_RATE
#else
#define ACCEL_DEFAULT_RATE      0x19		// 50Hz, low power, +- 16g
#endif

#define ACCEL_DEFAULT_WATERMARK 10  //25			// 1-32 (be sure can handle in time)
#define ACCEL_DEFAULT_ACTIVITY_THRESHOLD 0x04
#define ACCEL_DEFAULT_INACTIVITY_THRESHOLD 0x02
#ifdef DEMO_DEVICE
#define ACCEL_DEFAULT_INACTIVITY_TIME 30
#else
#define ACCEL_DEFAULT_INACTIVITY_TIME 0x04
#endif

#define ACCEL_FREQUENCY_FOR_RATE(_f) (3200 / (1 << (15-((_f) & 0x0f))))

#define ACCEL_INT_SOURCE_DATA_READY   0x80
#define ACCEL_INT_SOURCE_SINGLE_TAP   0x40
#define ACCEL_INT_SOURCE_DOUBLE_TAP   0x20
#define ACCEL_INT_SOURCE_ACTIVITY     0x10
#define ACCEL_INT_SOURCE_INACTIVITY   0x08
#define ACCEL_INT_SOURCE_FREE_FALL    0x04
#define ACCEL_INT_SOURCE_WATERMARK    0x02
#define ACCEL_INT_SOURCE_OVERRUN      0x01


#define ACCEL_DEVICE_ID 0xe5



// Accel structure
#ifdef ACCEL_DWORD_FORMAT

#define ACCEL_BYTES_PER_SAMPLE (4)
/*
typedef union Accel_t
{
	struct
	{		
		#error "Bit fields >1 are not correctly packed on the C18 compiler";
		signed short x : 10;
		signed short y : 10;
		signed short z : 10;
		unsigned char exp: 2;
	};
    unsigned long value;
} Accel;
*/

//typedef unsigned long Accel;
typedef union Accel_t
{
    // [byte-3] [byte-2] [byte-1] [byte-0]
	// eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx
	// 10987654 32109876 54321098 76543210
	struct
	{
		unsigned char byte_x07;
		unsigned char byte_x89_y05;
		unsigned char byte_y69_z03;
		unsigned char byte_z49_e01;
	};
	struct
	{
		unsigned short ushort_x09_y05;
		unsigned short ushort_y69_z09_e01;
	};
	struct
	{
		unsigned char _byte_x07;
		unsigned short ushort_x89_y09_z03;
		unsigned char _byte_z49_e01;
	};
    unsigned long value;
    unsigned char data[4];
} Accel;

// WARNING: C18 doesn't sign-extend on right-shift of signed types... aarrrggghh!
// ...oh dear, this is going to be some horiffic assembly code -- TODO: Optimize this (write a single function for conversion)...
#define ACCEL_XVALUE(_a) ((((unsigned short)((_a).value      ) & 0x03ff) | (((_a).value & 0x00000200) ? 0xfc00 : 0x0000)) << ((unsigned char)((_a).value >> 30)))
#define ACCEL_YVALUE(_a) ((((unsigned short)((_a).value >> 10) & 0x03ff) | (((_a).value & 0x00080000) ? 0xfc00 : 0x0000)) << ((unsigned char)((_a).value >> 30)))
#define ACCEL_ZVALUE(_a) ((((unsigned short)((_a).value >> 20) & 0x03ff) | (((_a).value & 0x20000000) ? 0xfc00 : 0x0000)) << ((unsigned char)((_a).value >> 30)))


#define ACCEL_SIGN10BIT(_v) (((_v) & 0x200) ? (_v) : (0xfc00 & (_v)))




#else

#define ACCEL_BYTES_PER_SAMPLE (6)

typedef union Accel_t
{
	struct
	{		
		signed short x;
		signed short y;
		signed short z;
	};
	struct
	{
		unsigned char xl;
		unsigned char xh;
		unsigned char yl;
		unsigned char yh;
		unsigned char zl;
		unsigned char zh;
	};
    unsigned char data[6];
} Accel;
#define ACCEL_XVALUE(_a) ((_a).x)
#define ACCEL_YVALUE(_a) ((_a).y)
#define ACCEL_ZVALUE(_a) ((_a).z)
#endif


#define ACCEL_MAX_FIFO_SAMPLES  (32)
#define ACCEL_MAX_FIFO_BYTES    (ACCEL_MAX_FIFO_SAMPLES * ACCEL_BYTES_PER_SAMPLE)


// Read device ID (should be ACCEL_DEVICE_ID = 0xE5)
unsigned char AccelReadDeviceId(void);

// Initialize the ADXL
extern void AccelStartup(unsigned char samplingRate, unsigned char watermark, unsigned char activityThreshold, unsigned char inactivityThreshold, unsigned char activityTime);

// Shutdown the ADXL (standby mode)
extern void AccelStandby(void);

// Enable interrupts
extern void AccelEnableInterrupts(unsigned char flags);

// Enable auto-sleep
extern void AccelEnableAutoSleep(char sleepNow);

// Checks whether accelerometer is in asleep (FIFO data supression)
extern unsigned char AccelIsAsleep(void);

// Reads the level of the ADXL into the specified address. 
extern void AccelSingleSample(Accel *value);

// Read at most 'maxEntries' 3-axis samples (6 bytes if old format OR 4 bytes if new 'DWORD' format) from the ADXL FIFO into the specified RAM buffer
extern unsigned char AccelReadFIFO(Accel *buffer, unsigned char maxEntries);

// Read tap status - b0 = Tap-Z
extern unsigned char AccelReadTapStatus(void);

// Read interrupt source - b1 = watermark, b5=double-tap, b6=single-tap
extern unsigned char AccelReadIntSource(void);

// Debug dump registers
extern void AccelDebugDumpRegisters(void);

// AccelCurrentRate
extern int AccelCurrentRate(void);

#endif

