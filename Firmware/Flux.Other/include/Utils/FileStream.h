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

#ifndef FILESTREAM_H
#define FILESTREAM_H


// File stream
typedef struct
{
    // Initialized by caller
#ifndef FILE_STREAM_USE_GLOBALS
    void *fileHandle;                   // Handle to the file stream -- either FILE* or FSFILE*
#endif
	unsigned char  streamId;            // [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
	unsigned short sampleRate;          // [2] Sample rate (Hz)
	signed char    sampleRateModifier;  // [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
	unsigned char  dataType;            // [1] Data type
	signed char    dataConversion;      // [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
	unsigned char  channelPacking;      // [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
    // Private
	unsigned long  sequenceId;          // [4] (32-bit sequence counter, each packet in a stream has a sequential number, reset when a stream is restarted)
#ifndef FILE_STREAM_USE_GLOBALS
    void *scratchBuffer;                // Scratch buffer for writing (at least 512 bytes)
#endif
} filestream_t;

#ifdef FILE_STREAM_USE_GLOBALS
extern unsigned char __attribute__((aligned(2))) scratchBuffer[512];
extern void* logFile;
#endif

// Initialize the specified file stream (structure initialized by caller)
void FileStreamInit(filestream_t *fileStream, void *sectorBuffer);

// Increment the sequence id - causes parser to NOT interpolate between frames
void FileStreamIncrementId(filestream_t *fileStream);

// Prepare a stream buffer, returning a pointer to the data portion
void *FileStreamPrepareData(filestream_t *fileStream, unsigned long timestamp, unsigned short fractionalTime, signed short timestampOffset, unsigned short sampleCount);

// Output a stream buffer (calculates the checksum before output), optional ecc (0=none, 1=ECC), optional checksum (0=none, 1=calculate)
char FileStreamOutputData(filestream_t *fileStream, char useEcc, char useChecksum);


// Stream packet
typedef struct
{
	unsigned char  packetType;          // [1] Packet type (ASCII, a-z = with-stream, A-Z = no-stream)
	unsigned char  streamId;            // [1] Stream identifier (if type is 'a'-'z'), or packet sub-type otherwise)
	unsigned short payloadLength;       // [2] <0x01FC> Payload length (payload is typically 508 bytes long, + 4 header/length = 512 bytes total)
	
	unsigned char reserved[506];        // (type-specific data)

	unsigned short checksum;            // @510 [2] 16-bit word-wise checksum of packet
} filestream_packet_t;


// Sensor settings
typedef struct
{
    char stream;
    char enabled;
    unsigned short frequency;
    unsigned short sensitivity;
    unsigned short options;
} filestream_sensor_config_t;

// Reason to stop recording flags (0x00 = none, 0x01 = end of interval, 0x02 = commanded to stop, 0x04 = interrupted by connection, 0x08 = battery low, 0x10 = write error, 0x20 = measurement error)
typedef enum {
    RECORD_STOP_REASON_NONE = 0x00,
    RECORD_STOP_REASON_INTERVAL = 0x01,
    RECORD_STOP_REASON_COMMAND = 0x02,
    RECORD_STOP_REASON_CONNECTED = 0x04,
    RECORD_STOP_REASON_BATTERY = 0x08,
    RECORD_STOP_REASON_WRITE_ERROR = 0x10,
    RECORD_STOP_REASON_MEASURE_ERROR = 0x20,
} record_stop_reason_t;

// File header
typedef struct
{
	unsigned char packetType;       // @0 Packet type (ASCII 'H' = header)
	unsigned char packetSubType;    // @1 Packet sub-type (ASCII 'A')
	unsigned short payloadLength;   // @2 <0x01FC> Payload length (508, +4 type/length = 512 bytes)
	char comment[128];              // @4 Machine-written human-readable comment (device type, version, id, firmware, session id, start/stop time, sensor config?)
	unsigned short deviceType;      // @132 Device type/sub-type
	unsigned short deviceVersion;   // @134 Device version
	unsigned short deviceId;        // @136 Device id
	unsigned char reserved1[14];    // @138 (reserved, write as zero)
	unsigned short firmwareVer;     // @152 Firmware version
	unsigned char reserved2[4];     // @154 (reserved, write as zero)
	unsigned long sessionId;        // @158 Session identifier
	unsigned long recordingStart;   // @162 Recording start time
	unsigned long recordingStop;    // @166 Recording stop time
	unsigned short stopReason;      // @170 Recording stop reason flags (0x00 = none, 0x01 = end of interval, 0x02 = commanded to stop, 0x04 = interrupted by connection, 0x08 = battery low, 0x10 = write error, 0x20 = measurement error)
	unsigned char debuggingInfo;    // @172 Debugging mode
	unsigned char reserved3;        // @173 (reserved, write as zero)
	unsigned char reserved4[6];     // @174 (reserved, write as zero)
	unsigned char aux[16];          // @180 (auxiliary data, write as zero)
    filestream_sensor_config_t configAccel;     // @196 Accelerometer configuration
	filestream_sensor_config_t configGyro;      // @204 Gyroscope configuration
	filestream_sensor_config_t configMag;       // @212 Magnetometer configuration
	filestream_sensor_config_t configAltimeter; // @220 Altimeter configuration
	filestream_sensor_config_t configAnalog;    // @228 Analogue configuration (Temperature, Light & Battery)
#ifdef SYNCHRONOUS_SAMPLING	
	filestream_sensor_config_t configAllAxis;   // @236 "All axis" configuration
#else
	unsigned char reserved5[8];     // @236 (reserved, write as zero)
#endif
	short calibration[32];			// @244 32 calibration words
	unsigned char reserved6[10];    // @308 (reserved, write as zero)
	unsigned char metadata[192];    // @318 Metadata (6x32)
	unsigned short checksum;        // @510 16-bit word-wise checksum of packet
} filestream_fileheader_t;



// Header packet
typedef struct
{
	unsigned char  packetType;          // [1] Packet type ASCII 'h' = header.
	unsigned char  streamId;            // [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-ASCII=reserved, *=all streams)
	unsigned short payloadLength;       // [2] <0x01FC> Payload length (payload is typically 508 bytes long, + 4 header/length = 512 bytes total)
	
	unsigned char reserved[506];        // (TBD)

	unsigned short checksum;            // @510 [2] 16-bit word-wise checksum of packet
} filestream_streamheader_t;


// Standard, timestamped data packet
typedef struct
{
	unsigned char  packetType;          // @ 0 [1] Packet type (ASCII 'd' = single-timestamped data stream, 's' = string, others = reserved)
	unsigned char  streamId;            // @ 1 [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
	unsigned short payloadLength;       // @ 2 [2] <0x01FC> Payload length (payload is 508 bytes long, + 4 header/length = 512 bytes total)

	unsigned long  sequenceId;          // @ 4 [4] (32-bit sequence counter, each packet in a stream has a sequential number, reset when a stream is restarted)

	unsigned long  timestamp;           // @ 8 [4] Timestamp stored little-endian (top-bit 0 = packed as 0YYYYYMM MMDDDDDh hhhhmmmm mmssssss with year-offset, default 2000; top-bit 1 = 31-bit serial time value of seconds since epoch, default 1/1/2000)
	unsigned short fractionalTime;      // @12 [2] Fractional part of the time (1/65536 second)
	signed short   timestampOffset;     // @14 [2] The sample index, relative to the start of the buffer, when the timestamp is valid (0 if at the start of the packet, can be negative or positive)

	unsigned short sampleRate;          // @16 [2] Sample rate (Hz)
	signed char    sampleRateModifier;  // @18 [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)

	unsigned char  dataType;            // @19 [1] Data type [NOT FINALIZED!] (top-bit set indicates "non-standard" conversion; bottom 7-bits: 0x00 = reserved,  0x10-0x13 = accelerometer (g, at +-2,4,8,16g sensitivity), 0x20 = gyroscope (dps), 0x30 = magnetometer (uT/raw?), 0x40 = light (CWA-raw), 0x50 = temperature (CWA-raw), 0x60 = battery (CWA-raw), 0x70 = pressure (raw?))
	signed char    dataConversion;      // @20 [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
	unsigned char  channelPacking;      // @21 [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)

	unsigned short sampleCount;         // @22 [2] Number samples in the packet (any remaining space is unused, or used for type-specific auxilliary values)

	unsigned char data[480];            // @24 [480] = e.g. 240 1-channel 16-bit samples, 80 3-channel 16-bit samples, 120 DWORD-packed 3-channel 10-bit samples
	unsigned short aux[3];              // @504 [6] (Optional) auxiliary data specific to the data-type (e.g. for device configuration; or for battery/events/light/temperature in a CWA stream)

	unsigned short checksum;            // @510 [2] 16-bit word-wise checksum of packet
} filestream_data_t;



// dataType
#define FILESTREAM_DATATYPE_RESERVED              0x00
#define FILESTREAM_DATATYPE_ACCEL                 0x10
#define FILESTREAM_DATATYPE_ACCEL_2G              0x11
#define FILESTREAM_DATATYPE_ACCEL_4G              0x12
#define FILESTREAM_DATATYPE_ACCEL_8G              0x13
#define FILESTREAM_DATATYPE_ACCEL_16G             0x14
#define FILESTREAM_DATATYPE_GYRO                  0x20
#define FILESTREAM_DATATYPE_GYRO_250              0x21
#define FILESTREAM_DATATYPE_GYRO_500              0x22
#define FILESTREAM_DATATYPE_GYRO_2000             0x24
#define FILESTREAM_DATATYPE_MAGNETOMETER          0x30
#define FILESTREAM_DATATYPE_LIGHT                 0x40
#define FILESTREAM_DATATYPE_TEMPERATURE           0x50
#define FILESTREAM_DATATYPE_PRESSURE              0x60
#define FILESTREAM_DATATYPE_BATTERY               0x70
#define FILESTREAM_DATATYPE_RAW					  0x71	
#define FILESTREAM_DATATYPE_ACCEL_GYRO_MAG        0x68	// "all axis" accel., gyro., mag.
// [dgj] The below should be re-numbered to be less than 0x80 -- the top bit of this byte is reserved to indicate a special *conversion* type, and should not be used for the data type
#define FILESTREAM_DATATYPE_BATTERY_LIGHT         0x80  // use 0x72 instead?
#define FILESTREAM_DATATYPE_BATTERY_LIGHT_TEMP    0x90  // use 0x73 instead?

#define FILESTREAM_DATATYPE_BATTERY_LIGHT_TEMPC   0x74  // Temperature in 0.1 deg-C units (can use for both barometer and ADC temperature readings)
#define FILESTREAM_DATATYPE_RAW_UINT   			  0x74  

#define FILESTREAM_DATATYPE_EPOC_STEPS    		  0x03


// channelPacking
#define FILESTREAM_PACKING_FORMAT_MASK     0x0f
#define FILESTREAM_PACKING_CHANNEL_MASK    0xf0

#define FILESTREAM_PACKING_1_CHANNEL		0x10
#define FILESTREAM_PACKING_2_CHANNEL		0x20
#define FILESTREAM_PACKING_3_CHANNEL		0x30	
#define FILESTREAM_PACKING_4_CHANNEL		0x40
#define FILESTREAM_PACKING_5_CHANNEL		0x50
//...etc
#define FILESTREAM_PACKING_9_CHANNEL		0x90
//...etc


#define FILESTREAM_PACKING_SPECIAL         0x00
#define FILESTREAM_PACKING_SINT8           0x01
#define FILESTREAM_PACKING_SINT16          0x02
#define FILESTREAM_PACKING_SINT24          0x03
#define FILESTREAM_PACKING_SINT32          0x04
#define FILESTREAM_PACKING_SINT64          0x05
#define FILESTREAM_PACKING_FLOAT           0x06
#define FILESTREAM_PACKING_DOUBLE          0x07
#define FILESTREAM_PACKING_RESERVED1       0x08
#define FILESTREAM_PACKING_UINT8           0x09
#define FILESTREAM_PACKING_UINT16          0x0a
#define FILESTREAM_PACKING_UINT24          0x0b
#define FILESTREAM_PACKING_UINT32          0x0c
#define FILESTREAM_PACKING_UINT64          0x0d
#define FILESTREAM_PACKING_RESERVED2       0x0e
#define FILESTREAM_PACKING_RESERVED3       0x0f

#define FILESTREAM_PACKING_SPECIAL_DWORD3_10_2 0x30


#endif
