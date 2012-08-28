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

// data.h - Sample Data I/O
// Dan Jackson, 2010.

#ifndef DATA_H
#define DATA_H


// Packet parameters
#define USER_REPORT_TYPE 0x12		    // Originally from MiWi.h
#define DATA_ACCEL_REPORT_ID 0x78       // 0x78 (ASCII 'x'), old format was 0x58 (ASCII 'X')


#if (DEVICE_TYPE==2)
#define DATA_BYTES_PER_SAMPLE (ACCEL_BYTES_PER_SAMPLE + GYRO_BYTES_PER_SAMPLE)
#else
#define DATA_BYTES_PER_SAMPLE ACCEL_BYTES_PER_SAMPLE
#endif

#define BUFFER_PACKET_MAX_SAMPLES (80 / DATA_BYTES_PER_SAMPLE)    // 13x 6-byte, 20x 4-byte, 8x 10-byte

#define BYTES_PER_SAMPLE_FROM_FORMAT(_f) (((((_f) >> 4) & 0x3) == 0) ? 4 : (((((_f) >> 4) & 0x3) == 2) ? 6 : 0))

// Data payload -- all WORD/DWORD stored as little-endian (LSB first)
typedef struct DataPacket_t
{
	unsigned char  reportType;		// [1] = 0x12 (USER_REPORT_TYPE)
	unsigned char  reportId;	    // [1] = 0x78 (ASCII 'x')
	unsigned short deviceId;		// [2] = Device identifier (16-bit)
	unsigned char  status;			// [1] = Device status (bit 0 is battery warning, top 7 bits reserved)
	unsigned short sample;			// [2] = Analogue sample (top 6 bits indicate measurement information, lower 10 bits are the value)
	unsigned char  format;			// [1] = Accelerometer data format 0xE9 (+/- 2g, 2-bytes, 100Hz); Top two bits is range, next two bits is format [0=3x10+2-bit, 2=signed 16-bit, 3=3x10+2 & 3*16-gyro] (2); lowest four bits is the rate code: frequency = 3200 / (1 << (15-(n & 0x0f)))
	unsigned short sequenceId;		// [2] = Sequence number of first accelerometer reading in this packet (16-bit sample index -- will wrap or be reset if device sleeps and resumes)
	unsigned char  outstanding;		// [1] = Number of samples remaining on device after this packet (0xff: >= 255)
	unsigned char  sampleCount;		// [1] = Number of samples in this packet. 0x0c = 13 samples (number of accelerometer samples)
	//Sample sampleData[BUFFER_PACKET_MAX_SAMPLES];	// bytes: [sampleCount * numAxes * bytesPerSample = 78 -- would be 102 with 17 samples] (sample data)
} DataPacket;


// Public functions
extern void DataConfigLoad(void);
extern void DataConfigSetId(unsigned short id);
extern void DataConfigSetTargetId(unsigned short id);
extern void DataConfigSave(void);
extern void DataInit(void);
extern unsigned char DataPerformAccelSample(void);
extern unsigned char DataPerformGyroSample(void);
extern unsigned char DataTransmit(void);
extern void DataDumpPacket(DataPacket *dp, unsigned char size);


typedef struct 
{
    unsigned char myLongAddress[8];
    unsigned char targetLongAddress[8];
    unsigned char accelRate;
    unsigned char accelWatermark;
	unsigned char activityThreshold;
	unsigned char inactivityThreshold; 
	unsigned char activityTime;
	unsigned long transmitInterval;
	unsigned short jitterMask;
	unsigned char channel;
	unsigned short pan;
	unsigned char startRxMode;
} DataConfig;

extern /*volatile*/ DataConfig dataConfig;


typedef struct DataStatus_t
{
    // Current data
    unsigned short deviceId;        // device id
	unsigned char sampling;	        // boolean 0/1
    unsigned long totalSamples;     // total number of samples added to buffer
    unsigned long totalOverflow;    // total number of samples overflow from buffer
    unsigned char retries;
    unsigned char rxMode;			// Receiver mode
} DataStatus;

#define RX_MODE_NONE 0
#define RX_MODE_SLIP 1
#define RX_MODE_TEXT 2
#define RX_MODE_OSC  3
#define RX_MODE_HEX  4

extern /*volatile*/ DataStatus dataStatus;



// Motor command payload -- all WORD/DWORD stored as little-endian (LSB first)
typedef struct MotorPacket_t
{
	unsigned char  reportType;		// [1] = 0x12 (USER_REPORT_TYPE)
	unsigned char  reportId;	    // [1] = 0x78 (ASCII 'M')
	unsigned char  version;			// [1] = Packet version
	unsigned char  count;			// [1] = Number of devices (calculate from packet length if 0)
	unsigned short firstDeviceId;	// [2] = First device identifier (16-bit)
	unsigned short data[];			// [2*count] = Array device data, top byte is PWM value, lower byte is length (ignore packet if 0)
} MotorPacket;



#endif
