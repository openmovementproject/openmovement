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

#include <stdint.h>


#ifdef _WIN32
#pragma pack(push, 1)
#endif


// File stream
typedef struct
{
    // Initialized by caller
    void *fileHandle;                   // Handle to the file stream -- either FILE* or FSFILE*
    uint8_t  streamId;            // [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
    uint16_t sampleRate;          // [2] Sample rate (Hz)
    signed char    sampleRateModifier;  // [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
    uint8_t  dataType;            // [1] Data type
    signed char    dataConversion;      // [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
    uint8_t  channelPacking;      // [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
    // Private
    uint32_t sequenceId;          // [4] (32-bit sequence counter, each packet in a stream has a sequential number, reset when a stream is restarted)
    void *scratchBuffer;                // Scratch buffer for writing (at least 512 bytes)
} filestream_t;


// Initialize the specified file stream (structure initialized by caller)
void FileStreamInit(filestream_t *fileStream, void *sectorBuffer);

// Prepare a stream buffer, returning a pointer to the data portion
void *FileStreamPrepareData(filestream_t *fileStream, uint32_t timestamp, uint16_t fractionalTime, int16_t timestampOffset, uint16_t sampleCount);

// Output a stream buffer (calculates the checksum before output), optional ecc (0=none, 1=ECC)
char FileStreamOutputData(filestream_t *fileStream, char ecc);


// Stream packet
typedef struct
{
    uint8_t  packetType;          // [1] Packet type (ASCII, a-z = with-stream, A-Z = no-stream)
    uint8_t  streamId;            // [1] Stream identifier (if type is 'a'-'z'), or packet sub-type otherwise)
    uint16_t payloadLength;       // [2] <0x01FC> Payload length (payload is typically 508 bytes long, + 4 header/length = 512 bytes total)
    
    uint8_t  reserved[508];       // (type-specific data)
} filestream_packet_t;


// Header packet
typedef struct
{
    uint8_t  packetType;          // [1] Packet type ASCII 'h' = header.
    uint8_t  streamId;            // [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-ASCII=reserved, *=all streams)
    uint16_t payloadLength;       // [2] <0x01FC> Payload length (payload is typically 508 bytes long, + 4 header/length = 512 bytes total)
    
    uint8_t  reserved[508];       // (TBD)
} filestream_header_t;


// Standard, timestamped data packet
typedef struct
{
    uint8_t  packetType;            // @0  [1] Packet type (ASCII 'd' = single-timestamped data stream, 's' = string, others = reserved)
    uint8_t  streamId;              // @1  [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
    uint16_t payloadLength;         // @2  [2] <0x01FC> Payload length (payload is 508 bytes long, + 4 header/length = 512 bytes total)
                                    
    uint32_t sequenceId;            // @4  [4] (32-bit sequence counter, each packet in a stream has a sequential number, reset when a stream is restarted)
                                    
    uint32_t timestamp;             // @8  [4] Timestamp stored little-endian (top-bit 0 = packed as 0YYYYYMM MMDDDDDh hhhhmmmm mmssssss with year-offset, default 2000; top-bit 1 = 31-bit serial time value of seconds since epoch, default 1/1/2000)
    uint16_t fractionalTime;        // @12 [2] Fractional part of the time (1/65536 second)
    int16_t  timestampOffset;       // @14 [2] The sample index, relative to the start of the buffer, when the timestamp is valid (0 if at the start of the packet, can be negative or positive)
                                    
    uint16_t sampleRate;            // @16 [2] Sample rate (Hz)
    int8_t   sampleRateModifier;    // @18 [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds; -1 = period in minutes)
                                    
    uint8_t  dataType;              // @19 [1] Data type [NOT FINALIZED!] (top-bit set indicates "non-standard" conversion; bottom 7-bits: 0x00 = reserved,  0x10-0x13 = accelerometer (g, at +-2,4,8,16g sensitivity), 0x20 = gyroscope (dps), 0x30 = magnetometer (uT/raw?), 0x40 = light (CWA-raw), 0x50 = temperature (CWA-raw), 0x60 = battery (CWA-raw), 0x70 = pressure (raw?))
    int8_t   dataConversion;        // @20 [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
    uint8_t  channelPacking;        // @21 [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
                                    
    uint16_t sampleCount;           // @22 [2] Number samples in the packet (any remaining space is unused, or used for type-specific auxilliary values)
                                    
    uint8_t  data[480];             // @24 [480] = e.g. 240 1-channel 16-bit samples, 80 3-channel 16-bit samples, 120 DWORD-packed 3-channel 10-bit samples
    uint16_t aux[3];                // @504 [6] (Optional) auxiliary data specific to the data-type (e.g. for device configuration; or for battery/events/light/temperature in a CWA stream)
                                    
    uint16_t checksum;              // @510 [2] 16-bit word-wise checksum of packet
} filestream_data_t;



// dataType
#define FILESTREAM_DATATYPE_RESERVED        0x00
#define FILESTREAM_DATATYPE_ACCEL           0x10
#define FILESTREAM_DATATYPE_ACCEL_2G        0x11
#define FILESTREAM_DATATYPE_ACCEL_4G        0x12
#define FILESTREAM_DATATYPE_ACCEL_8G        0x13
#define FILESTREAM_DATATYPE_ACCEL_16G       0x14
#define FILESTREAM_DATATYPE_GYRO            0x20
#define FILESTREAM_DATATYPE_MAGNETOMETER    0x30
#define FILESTREAM_DATATYPE_LIGHT           0x40
#define FILESTREAM_DATATYPE_TEMPERATURE     0x50
#define FILESTREAM_DATATYPE_PRESSURE        0x60
#define FILESTREAM_DATATYPE_BATTERY         0x70


// channelPacking
#define FILESTREAM_PACKING_FORMAT_MASK      0x0f
#define FILESTREAM_PACKING_CHANNEL_MASK     0xf0

#define FILESTREAM_PACKING_1_CHANNEL		0x10
#define FILESTREAM_PACKING_2_CHANNEL		0x20
#define FILESTREAM_PACKING_3_CHANNEL		0x30	
#define FILESTREAM_PACKING_4_CHANNEL		0x40
#define FILESTREAM_PACKING_5_CHANNEL		0x50
//...etc


#define FILESTREAM_PACKING_SPECIAL          0x00
#define FILESTREAM_PACKING_SINT8            0x01
#define FILESTREAM_PACKING_SINT16           0x02
#define FILESTREAM_PACKING_SINT24           0x03
#define FILESTREAM_PACKING_SINT32           0x04
#define FILESTREAM_PACKING_SINT64           0x05
#define FILESTREAM_PACKING_FLOAT            0x06
#define FILESTREAM_PACKING_DOUBLE           0x07
#define FILESTREAM_PACKING_RESERVED1        0x08
#define FILESTREAM_PACKING_UINT8            0x09
#define FILESTREAM_PACKING_UINT16           0x0a
#define FILESTREAM_PACKING_UINT24           0x0b
#define FILESTREAM_PACKING_UINT32           0x0c
#define FILESTREAM_PACKING_UINT64           0x0d
#define FILESTREAM_PACKING_RESERVED2        0x0e
#define FILESTREAM_PACKING_RESERVED3        0x0f

#define FILESTREAM_PACKING_SPECIAL_DWORD3_10_2 0x30


#ifdef _WIN32
#pragma pack(pop)
#endif

#endif
