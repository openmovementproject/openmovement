/* 
 * Copyright (c) 2009-2016, Newcastle University, UK.
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

// OmxFile - Dan Jackson, 2016.
// Methods and structures for dealing with OMX file headers and output streams.
// This replaces the 'FileStream' functionality
 
#ifndef OMXFILE_H
#define OMXFILE_H


#include <stddef.h>			// for size_t
#include <stdint.h>
#include <stdbool.h>


// OMX packet types
#define OMX_PACKET_TYPE_STREAM 'd'	// Single-timestamped rate-streamed data
#define OMX_PACKET_TYPE_EVENT  't'	// Timestamped events
#define OMX_PACKET_TYPE_STRING 's'	// Timestamp and single string (annotation)


// OMX stream type
typedef struct
{
	unsigned char packetType;			// [1] Packet type (d=single-timestamped stream data; s=string; t=packed with time stamps)
	unsigned char streamId;             // [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
	uint16_t sampleRate;                // [2] Sample rate (Hz)
	int8_t   sampleRateModifier;        // [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
	uint8_t  dataType;                  // [1] Data type
	int8_t   dataConversion;            // [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
	uint8_t  channelPacking;            // [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
} omx_stream_type_t;

// Initialize an omx_stream_t as a OMX_PACKET_TYPE_STREAM
void OmxStreamTypeInitStream(omx_stream_type_t *omxStreamType, unsigned char streamId, unsigned short sampleRate, char sampleRateModifier, unsigned char dataType, char dataConversion, unsigned char channelPacking);

// Initialize an omx_stream_t as a OMX_PACKET_TYPE_EVENT
void OmxStreamTypeInitEvent(omx_stream_type_t *omxStreamType, unsigned char streamId, unsigned char dataType, unsigned char dataSubType, size_t elementSize);

// Initialize an omx_stream_t as a OMX_PACKET_TYPE_STRING
void OmxStreamTypeInitString(omx_stream_type_t *omxStreamType);

// Size of each sample for the specified stream type
int OmxStreamTypeSampleSize(const omx_stream_type_t *omxStreamType);


// OMX stream state
typedef struct
{
	omx_stream_type_t streamType;		// Stream type
	int      maxSamples;				// Maximum number of samples per sector
	uint32_t sequenceId;                // [4] (32-bit sequence counter, each packet in a stream has a sequential number, reset when a stream is restarted)
} omx_stream_t;

// Initialize an OMX stream based on the given type
void OmxStreamInit(omx_stream_t *omxStream, const omx_stream_type_t *omxStreamType);

// Maximum number of samples per sector for the specified stream
int OmxStreamMaxSamples(const omx_stream_t *omxStream);

// Prepare a sector buffer (caller must ensure buffer is machine word aligned), returning a pointer to the data portion
void *OmxStreamPrepareSector(const omx_stream_t *omxStream, void *buffer, uint32_t timestamp, uint16_t fractionalTime, int16_t timestampOffset);

// Increments the sequence number of the stream (call once a prepared sector is successfully written)
void OmxStreamIncrementSequence(omx_stream_t *omxStream);



// Sensor settings (as stored in OMX)
typedef struct
{
    char stream;
    char enabled;
    uint16_t frequency;
    uint16_t sensitivity;
    uint16_t options;
} __attribute__((packed)) omx_sensor_config_t;


// Sensor index
#define OMX_SENSOR_ACCEL        0
#define OMX_SENSOR_GYRO         1
#define OMX_SENSOR_MAG          2
#define OMX_SENSOR_ALT          3
#define OMX_SENSOR_ADC          4
#define OMX_SENSOR_ALLAXIS      5
#define OMX_SENSOR_CONFIG_MAX   6				// OMX header has 6x omx_sensor_config_t


// Number of bytes and data bytes in a sector (to clear unused bytes / compute capacity, etc.)
#define OMX_SECTOR_SIZE (512)
#define OMX_SECTOR_DATA_SIZE (480)


// Reason to stop recording flags (0x00 = none, 0x01 = end of interval, 0x02 = commanded to stop, 0x04 = interrupted by connection, 0x08 = battery low, 0x10 = write error, 0x20 = measurement error)
// - multiple flags can be set.
typedef enum {
    OMX_STOP_REASON_NONE          = 0x00,
    OMX_STOP_REASON_INTERVAL      = 0x01,
    OMX_STOP_REASON_COMMAND       = 0x02,
    OMX_STOP_REASON_CONNECTED     = 0x04,
    OMX_STOP_REASON_BATTERY       = 0x08,
    OMX_STOP_REASON_WRITE_ERROR   = 0x10,
    OMX_STOP_REASON_MEASURE_ERROR = 0x20,
    OMX_STOP_REASON_RESERVED_1    = 0x40,
    OMX_STOP_REASON_RESERVED_2    = 0x80,
} omx_stop_reason_t;



// OMX base packet type (for header and data packets)
typedef struct
{
	unsigned char packetType;           // [1] Packet type (ASCII, a-z = with-stream, A-Z = no-stream)
	unsigned char streamId;             // [1] Stream identifier (if type is 'a'-'z'), or packet sub-type otherwise)
	uint16_t payloadLength;             // [2] <0x01FC> Payload length (payload is typically 508 bytes long, + 4 header/length = 512 bytes total)
	uint8_t  reserved[506];             // (type-specific data)
	uint16_t checksum;                  // @510 [2] 16-bit word-wise checksum of packet
} __attribute__((packed)) omx_packet_t;


// File header
typedef struct
{
	unsigned char packetType;           // @0 Packet type (ASCII 'H' = header)
	unsigned char packetSubType;        // @1 Packet sub-type (ASCII 'A')
	uint16_t payloadLength;             // @2 <0x01FC> Payload length (508, +4 type/length = 512 bytes)
	char     comment[128];              // @4 Machine-written human-readable comment (device type, version, id, firmware, session id, start/stop time, sensor config?)
	uint16_t deviceType;                // @132 Device type/sub-type
	uint16_t deviceVersion;             // @134 Device version
	uint16_t deviceId;                  // @136 Device id
	uint8_t  reserved1[14];             // @138 (reserved, write as zero)
	uint16_t firmwareVer;               // @152 Firmware version
	uint8_t  reserved2[4];              // @154 (reserved, write as zero)
	uint32_t sessionId;                 // @158 Session identifier
	uint32_t recordingStart;            // @162 Recording start time
	uint32_t recordingStop;             // @166 Recording stop time
	uint16_t stopReason;                // @170 Recording stop reason flags (0x00 = none, 0x01 = end of interval, 0x02 = commanded to stop, 0x04 = interrupted by connection, 0x08 = battery low, 0x10 = write error, 0x20 = measurement error)
	uint8_t  debuggingInfo;             // @172 Debugging mode
	uint8_t  reserved3;                 // @173 (reserved, write as zero)
	uint8_t  reserved4[6];              // @174 (reserved, write as zero)
	uint8_t  aux[16];                   // @180 (auxiliary data, write as zero)
	union
	{
		omx_sensor_config_t configSensors[OMX_SENSOR_CONFIG_MAX];	// @196 6x sensor configs (accel/gyro/mag/alt/analog/all-axis)
		struct
		{
			omx_sensor_config_t configAccel;    // @196 'a' Accelerometer configuration
			omx_sensor_config_t configGyro;     // @204 'g' Gyroscope configuration
			omx_sensor_config_t configMag;      // @212 'm' Magnetometer configuration
			omx_sensor_config_t configAltimeter;// @220 'p' Altimeter configuration
			omx_sensor_config_t configAnalog;   // @228 'l' Analogue configuration (Temperature, Light & Battery)
			omx_sensor_config_t configAllAxis;  // @236 'x' "All axis" configuration
		};
	};
	int16_t  calibration[32];		    // @244 32 calibration words
	uint8_t  reserved6[10];             // @308 (reserved, write as zero)
	uint8_t  metadata[192];             // @318 Metadata (6x32)
	uint16_t checksum;                  // @510 16-bit word-wise checksum of packet
} __attribute__((packed)) omx_fileheader_t;


// Standard data packet
typedef struct
{
	unsigned char packetType;           // @ 0 [1] Packet type (ASCII 'd' = single-timestamped data stream, 's' = timestamped string, 't' = timestamped blocks, others = reserved)
	unsigned char streamId;             // @ 1 [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
	uint16_t payloadLength;             // @ 2 [2] <0x01FC> Payload length (payload is 508 bytes long, + 4 header/length = 512 bytes total)
                                        
	uint32_t sequenceId;                // @ 4 [4] (32-bit sequence counter, each packet in a stream has a sequential number, reset when a stream is restarted)
                                        
	uint32_t timestamp;                 // @ 8 [4] Timestamp stored little-endian (top-bit 0 = packed as 0YYYYYMM MMDDDDDh hhhhmmmm mmssssss with year-offset, default 2000; top-bit 1 = 31-bit serial time value of seconds since epoch, default 1/1/2000)
	uint16_t fractionalTime;            // @12 [2] Fractional part of the time (1/65536 second)
	int16_t  timestampOffset;           // @14 [2] The sample index, relative to the start of the buffer, when the timestamp is valid (0 if at the start of the packet, can be negative or positive)
                                        
	uint16_t sampleRate;                // @16 [2] Sample rate (Hz)
	int8_t   sampleRateModifier;        // @18 [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
                                        
	uint8_t  dataType;                  // @19 [1] Data type [NOT FINALIZED!] (top-bit set indicates "non-standard" conversion; bottom 7-bits: 0x00 = reserved,  0x10-0x13 = accelerometer (g, at +-2,4,8,16g sensitivity), 0x20 = gyroscope (dps), 0x30 = magnetometer (uT/raw?), 0x40 = light (CWA-raw), 0x50 = temperature (CWA-raw), 0x60 = battery (CWA-raw), 0x70 = pressure (raw?))
	int8_t   dataConversion;            // @20 [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
	uint8_t  channelPacking;            // @21 [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
                                        
	uint16_t sampleCount;               // @22 [2] Number samples in the packet (any remaining space is unused, or used for type-specific auxilliary values)
                                        
	uint8_t  data[OMX_SECTOR_DATA_SIZE];// @24 [480] = e.g. 240 1-channel 16-bit samples, 80 3-channel 16-bit samples, 120 DWORD-packed 3-channel 10-bit samples
	uint16_t aux[3];                    // @504 [6] (Optional) auxiliary data specific to the data-type (e.g. for device configuration; or for battery/events/light/temperature in a CWA stream)
                                        
	uint16_t checksum;                  // @510 [2] 16-bit word-wise checksum of packet
} __attribute__((packed)) omx_data_t;



// Creates an annotation sector with the specified data
void OmxSectorAnnotation(void *buffer, uint32_t timestamp, uint16_t fractionalTime, const unsigned char *data, int dataLength);

// Returns the number of entries already in a sector
int OmxSectorSampleCount(const void *buffer);

// Updates a sector buffer, prepared with OmxStreamPrepareSector(), sample count to include samples that has been (externally) added to the buffer
void OmxSectorSamplesAdded(void *buffer, int addedSampleCount);

// Finalize a sector (clears any unused bytes and updates the OMX checksum)
void OmxSectorFinalize(void *buffer);


// Interpret a sector buffer as an omx_fileheader_t, returns NULL if it is not a valid header
omx_fileheader_t *OmxReadHeader(void *buffer);





// dataType
#define OMX_DATATYPE_RESERVED              0x00
#define OMX_DATATYPE_ACCEL                 0x10
#define OMX_DATATYPE_ACCEL_2G              0x11
#define OMX_DATATYPE_ACCEL_4G              0x12
#define OMX_DATATYPE_ACCEL_8G              0x13
#define OMX_DATATYPE_ACCEL_16G             0x14
#define OMX_DATATYPE_GYRO                  0x20
#define OMX_DATATYPE_GYRO_250              0x21
#define OMX_DATATYPE_GYRO_500              0x22
#define OMX_DATATYPE_GYRO_2000             0x24
#define OMX_DATATYPE_MAGNETOMETER          0x30
#define OMX_DATATYPE_LIGHT                 0x40
#define OMX_DATATYPE_TEMPERATURE           0x50
#define OMX_DATATYPE_PRESSURE              0x60
#define OMX_DATATYPE_BATTERY               0x70
#define OMX_DATATYPE_RAW                   0x71	
#define OMX_DATATYPE_ACCEL_GYRO_MAG        0x68	// "all axis" accel., gyro., mag.
// [dgj] The below should be re-numbered to be less than 0x80 -- the top bit of this byte is reserved to indicate a special *conversion* type, and should not be used for the data type
#define OMX_DATATYPE_BATTERY_LIGHT         0x80  // use 0x72 instead?
#define OMX_DATATYPE_BATTERY_LIGHT_TEMP    0x90  // use 0x73 instead?

#define OMX_DATATYPE_BATTERY_LIGHT_TEMPC   0x74  // Temperature in 0.1 deg-C units (can use for both barometer and ADC temperature readings)
#define OMX_DATATYPE_RAW_UINT              0x74  

#define OMX_DATATYPE_EPOC_STEPS    		  0x03


// channelPacking
#define OMX_PACKING_FORMAT_MASK     0x0f
#define OMX_PACKING_CHANNEL_MASK    0xf0

#define OMX_PACKING_1_CHANNEL		0x10
#define OMX_PACKING_2_CHANNEL		0x20
#define OMX_PACKING_3_CHANNEL		0x30	
#define OMX_PACKING_4_CHANNEL		0x40
#define OMX_PACKING_5_CHANNEL		0x50
//...etc
#define OMX_PACKING_9_CHANNEL		0x90
//...etc


#define OMX_PACKING_SPECIAL         0x00
#define OMX_PACKING_SINT8           0x01
#define OMX_PACKING_SINT16          0x02
#define OMX_PACKING_SINT24          0x03
#define OMX_PACKING_SINT32          0x04
#define OMX_PACKING_SINT64          0x05
#define OMX_PACKING_FLOAT           0x06
#define OMX_PACKING_DOUBLE          0x07
#define OMX_PACKING_RESERVED1       0x08
#define OMX_PACKING_UINT8           0x09
#define OMX_PACKING_UINT16          0x0a
#define OMX_PACKING_UINT24          0x0b
#define OMX_PACKING_UINT32          0x0c
#define OMX_PACKING_UINT64          0x0d
#define OMX_PACKING_RESERVED2       0x0e
#define OMX_PACKING_RESERVED3       0x0f

#define OMX_PACKING_SPECIAL_DWORD3_10_2 0x30


#endif
