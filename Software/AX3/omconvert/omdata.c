/*
* Copyright (c) 2014, Newcastle University, UK.
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

// Open Movement Fast File Loader (.CWA/.OMX)
// Dan Jackson, 2014


// AIMS:
// * fast to use in-place (segment chains are a consistent configuration and samples-per-sector)
// * 

// ADC channel 'l' uint16_t values[3];  // [0]-batt, [1]-LDR, [2]-Temp


#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#define timegm _mkgmtime
	#include <io.h>
#else
	#define _BSD_SOURCE		// Both of these lines
	#include <features.h>	// ...needed for timegm() in time.h on Linux
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>



#define USE_MMAP

#ifdef USE_MMAP
	#ifdef _WIN32
		// Windows implementation
		#include "mmap-win32.h"
	#else
		#include <sys/mman.h>
	#endif
#endif

#ifndef _WIN32
	#include <unistd.h>
	#define _open open
	#define _close close
	#define _read read
	#define _stat stat
	#define _fstat fstat
	#define _O_RDONLY O_RDONLY
	#define _O_BINARY 0
#endif


#include "omdata.h"

// Packed date/time
#define DATETIME_YEAR(_v)    ((unsigned char)(((_v) >> 26) & 0x3f))
#define DATETIME_MONTH(_v)   ((unsigned char)(((_v) >> 22) & 0x0f))
#define DATETIME_DAY(_v)     ((unsigned char)(((_v) >> 17) & 0x1f))
#define DATETIME_HOURS(_v)   ((unsigned char)(((_v) >> 12) & 0x1f))
#define DATETIME_MINUTES(_v) ((unsigned char)(((_v) >>  6) & 0x3f))
#define DATETIME_SECONDS(_v) ((unsigned char)(((_v)      ) & 0x3f))


// OMX Formats
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

#define FILESTREAM_DATATYPE_EPOCH_STEPS    		  0x03

// channelPacking - channels
#define FILESTREAM_PACKING_CHANNEL_MASK    0xf0
#define FILESTREAM_PACKING_1_CHANNEL		0x10
#define FILESTREAM_PACKING_2_CHANNEL		0x20
#define FILESTREAM_PACKING_3_CHANNEL		0x30	
#define FILESTREAM_PACKING_4_CHANNEL		0x40
#define FILESTREAM_PACKING_5_CHANNEL		0x50
#define FILESTREAM_PACKING_9_CHANNEL		0x90

// channelPacking - format
#define FILESTREAM_PACKING_FORMAT_MASK     0x0f
//#define FILESTREAM_PACKING_SPECIAL         0x00
#define FILESTREAM_PACKING_SINT8           0x01
#define FILESTREAM_PACKING_SINT16          0x02
//#define FILESTREAM_PACKING_SINT24          0x03
#define FILESTREAM_PACKING_SINT32          0x04
//#define FILESTREAM_PACKING_SINT64          0x05
//#define FILESTREAM_PACKING_FLOAT           0x06
//#define FILESTREAM_PACKING_DOUBLE          0x07
//#define FILESTREAM_PACKING_RESERVED1       0x08
#define FILESTREAM_PACKING_UINT8           0x09
#define FILESTREAM_PACKING_UINT16          0x0a
//#define FILESTREAM_PACKING_UINT24          0x0b
#define FILESTREAM_PACKING_UINT32          0x0c
//#define FILESTREAM_PACKING_UINT64          0x0d
//#define FILESTREAM_PACKING_RESERVED2       0x0e
//#define FILESTREAM_PACKING_RESERVED3       0x0f
#define FILESTREAM_PACKING_SPECIAL_DWORD3_10_2 0x30





// scale: convert to real units, range: limit of sensor
static double OmDataSampleRate(const void *buffer, double *outScale, double *outRange)
{
	const unsigned char *p = (const unsigned char *)buffer;
	if (p[1] == 'X') // CWA file
	{
		uint8_t sampleRate = p[24];
		double freq;
		if (sampleRate == 0)
		{
			// Old format, frequency stored directly in (currently) 'timestampOffset' field
			int16_t timestampOffset = ((int16_t)p[27] << 8) | p[26];
			freq = (double)(unsigned short)timestampOffset;
			if (outRange != NULL) { *outRange = 16.0; }
		}
		else
		{
			// New format - rate is coded
			freq = 3200.0 / (1 << (15 - (sampleRate & 0x0f)));
			if (freq <= 0.0) { freq = 1.0; }
			if (outRange != NULL) { *outRange = (double)(16 >> (sampleRate >> 6)); }
		}

		if (outScale != NULL) { *outScale = 1.0 / 256; }	// CWA scaling is always 1/256

		return freq;
	}
	else if (p[0] == 'd')	// OMX file
	{
		char sampleRateModifier = p[18];
		double sampleRate = (double)((unsigned short)p[16] | ((unsigned short)p[17] << 8));
		double scale = 1.0;
		double range = 1.0;
		unsigned char dataType = p[19];
		//unsigned char dataConversion = p[20];  // @20 [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))

		if (sampleRateModifier == 1)        // 1 = Sample rate in Hz
		{
			;
		}
		else if (sampleRateModifier > 1)    // >1 = Sample rate divisor
		{
			sampleRate /= sampleRateModifier;
		}
		else if (sampleRateModifier > 1)    // <-1 = Sample rate multiplier
		{
			sampleRate *= -sampleRateModifier;
		}
		else if (sampleRateModifier == 0)   // 0 = Period in seconds
		{
			if (sampleRate != 0) { sampleRate = 1.0 / sampleRate; }
		}
		else if (sampleRateModifier == -1)  // -1 = Period in minutes
		{
			if (sampleRate != 0) { sampleRate = 1.0 / (sampleRate * 60.0); }
		}

		// Calculate scale
		if (p[1] == 'a')
		{
			range = 8.0;
			switch (dataType)
			{
				case FILESTREAM_DATATYPE_ACCEL_2G: range = 2.0; break;		// dataConversion = -14;
				case FILESTREAM_DATATYPE_ACCEL_4G: range = 4.0; break;		// dataConversion = -13;
				case FILESTREAM_DATATYPE_ACCEL_8G: range = 8.0; break;		// dataConversion = -12;
				case FILESTREAM_DATATYPE_ACCEL_16G: range = 16.0; break;	// dataConversion = -11;
			}
			scale = (2 * range) / 65536.0;
		}
		else if (p[1] == 'g')
		{
			range = 2000.0; scale = 0.07;
			switch (dataType)
			{
				case FILESTREAM_DATATYPE_GYRO_250:  range =  250.0; scale = 0.00875; break;
				case FILESTREAM_DATATYPE_GYRO_500:  range =  500.0; scale = 0.0175; break;
				case FILESTREAM_DATATYPE_GYRO_2000: range = 2000.0; scale = 0.07; break;
			}
		}
		else if (p[1] == 'm')
		{
			range = 3276.8;
			scale = 0.1;
		}
		else if (p[1] == 'p')
		{
			// Pressure/temperature
			//fileStream->streamId = 'p';                   // [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
			//fileStream->dataType = FILESTREAM_DATATYPE_PRESSURE; // [1] Data type - see FileStream.h
			//fileStream->channelPacking = (FILESTREAM_PACKING_1_CHANNEL | FILESTREAM_PACKING_SINT32);      		// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
		}
		else if (p[1] == 'l')
		{
			// ADC
			//fileStream->streamId = 'l';            			// [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
			//fileStream->dataType = FILESTREAM_DATATYPE_RAW_UINT; // [1] Data type - see FileStream.h
			//fileStream->channelPacking = (FILESTREAM_PACKING_3_CHANNEL|FILESTREAM_PACKING_UINT16);      			// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
		}
		//else if (p[1] == 'x')
		//{
			// All-axis
			//fileStream->streamId = 'x';                             // [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
			//fileStream->dataType = FILESTREAM_DATATYPE_ACCEL_GYRO_MAG;   // [1] Data type - see FileStream.h
			//fileStream->channelPacking = (FILESTREAM_PACKING_9_CHANNEL|FILESTREAM_PACKING_SINT16);      		// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
		//}

		if (outRange != NULL) { *outRange = range; }
		if (outScale != NULL) { *outScale = scale; }

		return sampleRate;
	}

	return 0;
}



static uint32_t OmDataTimestamp(uint32_t timestamp)
{
	time_t tSec;								// Seconds since epoch

	// Min
	if (timestamp == 0) { return 0; }
	// Max
	if (timestamp >= 0xffffffff) { return (uint32_t)-1; }

	// Check if it's a serial time value
#if 0
	if (timestamp & 0x80000000ul)
	{
		tSec = (timestamp & 0x7ffffffful) + (10957ul * 24 * 60 * 60);	// Adjust Y2K epoch to unix epoch
	}
	else
#endif
	{
		struct tm tParts = { 0 };					// Time elements (YMDHMS)
		tParts.tm_year = 2000 + DATETIME_YEAR(timestamp) - 1900;
		tParts.tm_mon = DATETIME_MONTH(timestamp) - 1;
		tParts.tm_mday = DATETIME_DAY(timestamp);
		tParts.tm_hour = DATETIME_HOURS(timestamp);
		tParts.tm_min = DATETIME_MINUTES(timestamp);
		tParts.tm_sec = DATETIME_SECONDS(timestamp);
		tSec = timegm(&tParts);						// Pack from YMDHMS
	}
	return (uint32_t)tSec;
}


double OmDataTimestampForSector(omdata_t *omdata, int sectorIndex, int *sampleIndexOffset)
{
	const unsigned char *p = (const unsigned char *)omdata->buffer + (OMDATA_SECTOR_SIZE * sectorIndex);
	uint32_t timestamp = 0;
	uint16_t fractional = 0;
	int16_t timestampOffset = 0;

	if (p[1] == 'X') // CWA file
	{
		timestamp = ((int32_t)p[17] << 24) | ((int32_t)p[16] << 16) | ((int32_t)p[15] << 8) | p[14];
		timestampOffset = ((int16_t)p[27] << 8) | p[26];

		if (p[24] == 0)		// sampleRate 0 indicates old format
		{
			// Timestamp offset in very old files is actually the frequency -- don't use it...
			timestampOffset = 0;
		}
		else
		{
			// If we have a fractional offset, the timestamp offset was artificially modified for backwards-compatibility, undo this...
			uint16_t deviceId = ((int16_t)p[5] << 8) | p[4];
			if (deviceId & 0x8000)
			{
				double freq = OmDataSampleRate(p, NULL, NULL);

				// Need to undo backwards-compatible shim: Take into account how many whole samples the fractional part of timestamp accounts for:  relativeOffset = fifoLength - (short)(((unsigned long)timeFractional * AccelFrequency()) >> 16);
				// relativeOffset = fifoLength - (short)(((unsigned long)timeFractional * AccelFrequency()) >> 16);
				//                         nearest whole sample
				//          whole-sec       | /fifo-pos@time
				//           |              |/
				// [0][1][2][3][4][5][6][7][8][9]
				fractional = ((deviceId & 0x7fff) << 1);	// use 15-bits as 16-bit fractional time
				timestampOffset += (short)(((unsigned long)fractional * (int)freq) >> 16);	// (frequency is modelled as an integer on board)
			}
		}
	}
	else if (p[0] == 'd')	// OMX file
	{
		// timestamp        @8 +4  Last reported RTC value, 0 = unknown
		timestamp = (unsigned long)p[8] | ((unsigned long)p[9] << 8) | ((unsigned long)p[10] << 16) | ((unsigned long)p[11] << 24);

		// fractionalTime   @12 +2 Fractional part
		fractional = (unsigned short)p[12] | ((unsigned short)p[13] << 8);

		// timestampOffset  @14 +2  (Signed) relative sample index from the start of the buffer where the whole-second timestamp is valid
		timestampOffset = (short)((unsigned short)p[14] | ((unsigned short)p[15] << 8));
	}

	// Calculate block start time (seconds since epoch)
	uint32_t tSec = OmDataTimestamp(timestamp);

	// Return sample index offset
	if (sampleIndexOffset != NULL) { *sampleIndexOffset = timestampOffset; }

	// Return time
	double t = tSec + (fractional / 65536.0);

	return t;
}




/*
[CWA] Offset from a data sector:
@  0 WORD  packetHeader ('A' and 'X')
@  2 WORD  packetPayloadLength (508 bytes)
@  4 WORD  fractionOfASecond (if top bit set) (previously 'deviceId')
@  6 DWORD sessionId
@ 10 DWORD sequenceId
@ 14 DWORD timestamp (packed YMDHMS)
@ 18 WORD  light
@ 20 WORD  temperature
@ 22 BYTE  eventsFlag
@ 23 BYTE  battery
@ 24 BYTE  sampleRate
@ 25 BYTE  numAxesBPS
@ 26 WORD  timestampOffset
@ 28 WORD  sampleCount
@ 30 [480] data
@510 WORD  checksum

[OMX] Offset from a data sector
@  0 BYTE  packetType;          // Packet type (ASCII 'd' = single-timestamped data stream, 's' = string, others = reserved)
@  1 BYTE  streamId;            // Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
@  2 WORD  payloadLength;       // <0x01FC> Payload length (payload is 508 bytes long, + 4 header/length = 512 bytes total)
@  4 DWORD sequenceId;          // (32-bit sequence counter, each packet in a stream has a sequential number, reset when a stream is restarted)
@  8 DWORD timestamp;           // Timestamp stored little-endian (top-bit 0 = packed as 0YYYYYMM MMDDDDDh hhhhmmmm mmssssss with year-offset, default 2000; top-bit 1 = 31-bit serial time value of seconds since epoch, default 1/1/2000)
@ 12 WORD  fractionalTime;      // Fractional part of the time (1/65536 second)
@ 14 WORD  timestampOffset;     // The sample index, relative to the start of the buffer, when the timestamp is valid (0 if at the start of the packet, can be negative or positive)
@ 16 WORD  sampleRate;          // Sample rate (Hz)
@ 18 BYTE  sampleRateModifier;  // Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
@ 19 BYTE  dataType;            // Data type [NOT FINALIZED!] (top-bit set indicates "non-standard" conversion; bottom 7-bits: 0x00 = reserved,  0x10-0x13 = accelerometer (g, at +-2,4,8,16g sensitivity), 0x20 = gyroscope (dps), 0x30 = magnetometer (uT/raw?), 0x40 = light (CWA-raw), 0x50 = temperature (CWA-raw), 0x60 = battery (CWA-raw), 0x70 = pressure (raw?))
@ 20 BYTE  dataConversion;      // Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
@ 21 BYTE  channelPacking;      // Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
@ 22 WORD  sampleCount;         // Number samples in the packet (any remaining space is unused, or used for type-specific auxilliary values)
@ 24 [480] data;                // @24 [480] = e.g. 240 1-channel 16-bit samples, 80 3-channel 16-bit samples, 120 DWORD-packed 3-channel 10-bit samples
@504 WORD[3] aux;               // @504 [6] (Optional) auxiliary data specific to the data-type (e.g. for device configuration; or for battery/events/light/temperature in a CWA stream)
@510 WORD  checksum;            // @510 [2] 16-bit word-wise checksum of packet
*/

static int OmDataAddSector(omdata_t *omdata, int sectorIndex, bool extractSideChannelsAsStream)
{
	const unsigned char *p = (const unsigned char *)omdata->buffer + (OMDATA_SECTOR_SIZE * sectorIndex);

	// Data
	char format = -1;
	uint32_t sequenceId;
	char streamIndex = -1;
	if ((p[0] == 'A' || p[0] == 'G') && p[1] == 'X')			// CWA Data
	{
		format = 0;
		streamIndex = p[0] - 'A' + 'a';
		sequenceId = ((uint32_t)p[13] << 24) | ((uint32_t)p[12] << 16) | ((uint32_t)p[11] << 8) | (uint32_t)p[10];
	}
	else if (p[0] == 'd')										// OMX Data
	{
		format = 1;
		streamIndex = p[1];
		sequenceId = ((uint32_t)p[7] << 24) | ((uint32_t)p[6] << 16) | ((uint32_t)p[5] << 8) | (uint32_t)p[4];
	}

	// * Each stream consists of multiple segments (segments are between broken parts: sequenceId mismatch / checksum error / etc.)
	// * Each segment is a list of sector indexes for that stream
	// * After loading, find the minimum next start/end time from each stream segment, advance to that time, repeat (tracking whether we're in/out of each stream) -- for any periods that no streams are active, this is a recording break (could merge if short).
	// * Each recording is a list of streams, each stream has a list of segments.

	// Check if there's a configuration change mid-stream (num-axes, format, etc.)  Non-full sectors set a flag as 'last sector in stream'.
	int offset = 0, packing = 0, channels = 0, samplesPerSector = 0;
	double scaling = 0;
	double range = 0;
	double sampleRate = OmDataSampleRate(p, &scaling, &range);
	int maxValues = -1;

	// Configuration (this will be constant along an entire segment)
	if (format == 0)	// CWA
	{
		offset = 30;

		packing = p[25];
		channels = ((packing >> 4) & 0x0f);
		if (!channels) channels = 1;

		if ((packing & 0x0f) == 0 && channels == 3) { packing = FILESTREAM_PACKING_SPECIAL_DWORD3_10_2; maxValues = 360; }
		else if ((packing & 0x0f) == 1) { packing = FILESTREAM_PACKING_SINT8; maxValues = 480; }
		else if ((packing & 0x0f) == 2) { packing = FILESTREAM_PACKING_SINT16; maxValues = 480 / 2; }
		//else if ((packing & 0x0f) == 4) { packing = FILESTREAM_PACKING_SINT32; maxValues = 480 / 4; }

		samplesPerSector = (unsigned short)p[28] | ((unsigned short)p[29] << 8);
		if (maxValues <= 0 || samplesPerSector * channels > maxValues) { return -1; }
	}
	else if (format == 1)
	{
		offset = 24;

		packing = p[21];
		channels = (packing >> 4);
		if (!channels) channels = 1;

		if (packing == FILESTREAM_PACKING_SPECIAL_DWORD3_10_2) { channels = 3; maxValues = 360; }
		else if ((packing & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT32) { packing = FILESTREAM_PACKING_SINT32; maxValues = 480 / 4; }
		else if ((packing & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_UINT32) { packing = FILESTREAM_PACKING_UINT32; maxValues = 480 / 4; }
		else if ((packing & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT16) { packing = FILESTREAM_PACKING_SINT16; maxValues = 480 / 2; }
		else if ((packing & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_UINT16) { packing = FILESTREAM_PACKING_UINT16; maxValues = 480 / 2; }
		else if ((packing & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT8)  { packing = FILESTREAM_PACKING_SINT8; maxValues = 480; }
		else if ((packing & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_UINT8)  { packing = FILESTREAM_PACKING_UINT8; maxValues = 480; }

		//dataType = p[19];         // @19 [1] Data type [NOT FINALIZED!] (top-bit set indicates "non-standard" conversion; bottom 7-bits: 0x00 = reserved,  0x10-0x13 = accelerometer (g, at +-2,4,8,16g sensitivity), 0x20 = gyroscope (dps), 0x30 = magnetometer (uT/raw?), 0x40 = light (CWA-raw), 0x50 = temperature (CWA-raw), 0x60 = battery (CWA-raw), 0x70 = pressure (raw?))
		//state->dataConversion = p[20];   // @20 [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))

		samplesPerSector = (unsigned short)p[22] | ((unsigned short)p[23] << 8);
		if (maxValues <= 0 || samplesPerSector * channels > maxValues) { return -2; }
	}
	else
	{
		return -3;
	}


	bool startNewSegment = false;

	// If asked to, fake an ADC stream for the data embedded in a CWA sector
	if (extractSideChannelsAsStream)
	{
		// Create virtual segments for CWA temperature, battery, light (embedded in normal accelerometer sectors)
		//unsigned short values[3];  // [0]-batt, [1]-LDR, [2]-Temp
		streamIndex = 'l';		// Fake an ADC stream)
		offset = 0;
		packing = 0;
		channels = 3;
		scaling = 1;
		sampleRate = (sampleRate != 0) ? maxValues / sampleRate : 1;		// One sample per sector // samplesPerSector
		samplesPerSector = 1;
	}

	//fprintf(stderr, "OMDATA: Stream %c\n", streamIndex);
	omdata_stream_t *stream = &omdata->stream[(int)streamIndex];
	if (!stream->inUse)
	{
		stream->inUse = true;
		stream->lastSequenceId = (uint32_t)-1; // = 0xffffffff;
		stream->segmentFirst = NULL;
		stream->segmentLast = NULL;
	}

	omdata_segment_t *seg = stream->segmentLast;

	// If we don't have a previous segment, start a new one
	if (stream->segmentLast == NULL)
	{
		fprintf(stderr, "OMDATA: Stream %c creating a new segment.\n", streamIndex);
		startNewSegment = true;
	}
	else
	{
		// If this was already marked as having read a short (final) packet, or if any of the parameters have changed (allow the samples-per-sector to be less) -- change format
		if (seg->lastPacketShort || offset != seg->offset || packing != seg->packing || channels != seg->channels || scaling != seg->scaling || samplesPerSector > seg->samplesPerSector || sampleRate != seg->sampleRate)
		{
			fprintf(stderr, "OMDATA: Stream %c config changed or after last short packet.\n", streamIndex);
			startNewSegment = true;
		}

		// If we're not changing the format now, if the samples-per-sector is less, mark this as a final packet
		if (!startNewSegment && samplesPerSector < seg->samplesPerSector)
		{
			fprintf(stderr, "OMDATA: Stream %c last packet short (%d of %d).\n", streamIndex, samplesPerSector, seg->samplesPerSector);
			seg->lastPacketShort = true;
		}

	}

	// Check the delta
	unsigned int delta = (uint32_t)(sequenceId - stream->lastSequenceId);
	if (delta != 1)
	{
		fprintf(stderr, "OMDATA: Stream %c delta %d != 1 (%d -> %d)\n", streamIndex, delta, stream->lastSequenceId, sequenceId);
		startNewSegment = true;
	}

	// See if we need to start a new segment
	//omdata_segment_t *seg;
	if (startNewSegment)
	{
		if (stream->segmentLast != NULL)
		{
			fprintf(stderr, "OMDATA: Break in stream %c (sequence id %d -> %d)\n", streamIndex, stream->lastSequenceId, sequenceId);
		}

		seg = (omdata_segment_t *)malloc(sizeof(omdata_segment_t));
		memset(seg, 0, sizeof(omdata_segment_t));
		if (stream->segmentFirst == NULL) { stream->segmentFirst = seg; }
		if (stream->segmentLast != NULL) { stream->segmentLast->segmentNext = seg; }
		stream->segmentLast = seg;

		seg->offset = offset;						// Offset of data within sector (depends on format)
		seg->packing = packing;						// The type of data and the way the data is packed
		seg->channels = channels;					// The number of channels
		seg->scaling = scaling;						// The required conversion into units
		seg->samplesPerSector = samplesPerSector;	// The number of samples in every sector (the last sector in a segment is permitted to have fewer)
		seg->sampleRate = sampleRate;
		seg->lastPacketShort = 0;
	}
	else
	{
		seg = stream->segmentLast;
	}

	// Timestamps (monotonically increasing index)
	int sampleIndexOffset = 0;
	double timestampValue = OmDataTimestampForSector(omdata, sectorIndex, &sampleIndexOffset);
	if (extractSideChannelsAsStream)
	{
		sampleIndexOffset = 0;		// TODO: Need to represent underlying sample offset at aux channel rate (measured in actual, high-speed sample rate)
	}
	int segmentSampleIndex = seg->sectorCount * seg->samplesPerSector + sampleIndexOffset;
	// If it's the first value or it's a different timestamp index, add it...
	if (seg->timestampCount <= 0 || seg->timestamps[seg->timestampCount - 1].sample != segmentSampleIndex)
	{
		// Continue the current segment - grow buffer if needed
		if (seg->timestamps == NULL || seg->timestampCount >= seg->timestampCapacity)
		{
			seg->timestampCapacity = (15 * seg->timestampCapacity / 10 + 1);
			seg->timestamps = (omdata_segment_timestamp_t *)realloc(seg->timestamps, seg->timestampCapacity * sizeof(omdata_segment_timestamp_t));
		}
		// Add element
		seg->timestamps[seg->timestampCount].sample = segmentSampleIndex;
		seg->timestamps[seg->timestampCount].timestamp = timestampValue;
		//printf("TIME,%d,%d,%f\n", seg->timestampCount, segmentSampleIndex, timestampValue);
		seg->timestampCount++;
	}

	// Add the number of samples in this sector
	seg->numSamples += samplesPerSector;

	// Continue the current segment - grow buffer if needed
	if (seg->sectorIndex == NULL || seg->sectorCount >= seg->sectorCapacity)
	{
		seg->sectorCapacity = (15 * seg->sectorCapacity / 10 + 1);
		seg->sectorIndex = (unsigned int *)realloc(seg->sectorIndex, seg->sectorCapacity * sizeof(unsigned int));
	}
	// Add element
	seg->sectorIndex[seg->sectorCount] = sectorIndex;
	seg->sectorCount++;

	// Update last sequence id
	stream->lastSequenceId = sequenceId;
	return 0;
}

#define READ_UINT8(_p) (*((unsigned char *)(_p)))
#define READ_UINT16(_p) (*((unsigned char *)(_p)) | ((unsigned short)*((unsigned char *)(_p) + 1) << 8))
#define READ_UINT32(_p) (*((unsigned char *)(_p)) | ((unsigned int)*((unsigned char *)(_p) + 1) << 8) | ((unsigned int)*((unsigned char *)(_p) + 2) << 16) | ((unsigned int)*((unsigned char *)(_p) + 3) << 24))

static int OmDataProcessSectors(omdata_t *omdata, int sectorStartIndex, int sectorCount)
{
	const unsigned char *buffer = omdata->buffer;
	int i;

	// Go through each sector
	for (i = sectorStartIndex; i < sectorStartIndex + sectorCount; i++)
	{
		const unsigned char *p = (const unsigned char *)buffer + (OMDATA_SECTOR_SIZE * i);
		int j;

		omdata->statsTotalSectors++;

		// Check header flag
		if (p[0] < 32 || p[0] >= 128 || p[1] < 32 || p[1] >= 128) { fprintf(stderr, "OMDATA: Bad sector type @%d header=0x%02x 0x%02x\n", i, p[0], p[1]); omdata->statsBadSectors++; continue; }

		// Check length
		uint16_t payloadLength = ((uint16_t)p[3] << 8) | (uint16_t)p[2];
		int numSectors = ((payloadLength + 4 + 512 - 1) / OMDATA_SECTOR_SIZE);
		if ((payloadLength + 4) & (OMDATA_SECTOR_SIZE - 1) || numSectors == 0) { fprintf(stderr, "OMDATA: Bad payload length @%d length=%d\n", i, payloadLength); omdata->statsBadSectors++; continue; }
		if (numSectors != 1 && !(p[0] == 'M' || p[1] == 'D')) { fprintf(stderr, "OMDATA: Unexpected payload length @%d length=%d\n", i, payloadLength); omdata->statsBadSectors++; continue; }

		if (i == 0 && p[0] == 'M' && p[1] == 'D')				// CWA Header
		{
			omdata_metadata_t *md;
			unsigned char sampleCode;

			fprintf(stderr, "OMDATA: Header (CWA)...\n");
			i += (numSectors - 1);		// Can skip multiple

			md = &omdata->metadata;
			memset(md, 0, sizeof(omdata_metadata_t));

			// .CWA doesn't contain this information, use these defaults (overridden by future version of the file)
			md->deviceType = 0x5743;	// "CW"
			strcpy(md->deviceTypeString, "CWA");
			md->deviceVersion = 17;

			md->deviceId = READ_UINT16(p + 5);			// CWA@5	unsigned short deviceId;
			md->sessionId = READ_UINT32(p + 7);			// CWA@7	unsigned long sessionId;		
			md->recordingStart = OmDataTimestamp(READ_UINT32(p + 13));	// CWA@13	unsigned long loggingStartTime;
			md->recordingStop = OmDataTimestamp(READ_UINT32(p + 17));	// CWA@17	unsigned long loggingEndTime
			md->debuggingInfo = READ_UINT8(p + 26);		// CWA@26	char debuggingInfo;
			//md->clearTime = OmDataTimestamp(READ_UINT32(p + 32));		// CWA@32	unsigned long lastClearTime;
			sampleCode = READ_UINT8(p + 36);			// CWA@36	unsigned char samplingRate;
			//md->changeTime = OmDataTimestamp(READ_UINT32(p + 37));		// CWA@37	unsigned long lastChangeTime;
			md->firmwareVer = READ_UINT8(p + 41);		// CWA@41	unsigned char firmwareRevision;
			memset(md->metadata, 0, sizeof(md->metadata));
			memcpy(md->metadata, p + 64, 448);			// CWA@64	unsigned char annotation[448];
			for (j = 0; j < sizeof(md->metadata); j++)
			{
				if (md->metadata[j] == 0xff) { md->metadata[j] = '\0'; }
			}
			for (j = strlen((const char *)md->metadata) - 1; j >= 0; j--)
			{
				if (md->metadata[j] != ' ') { break; }
				md->metadata[j] = '\0';
			}

			// Rate calculations
			md->configAccel.stream = 'a';
			md->configAccel.enabled = 1;
			md->configAccel.frequency = (3200 / (1 << (15 - (sampleCode & 0x0f))));
			md->configAccel.sensitivity = (16 >> (sampleCode >> 6));
			md->configAccel.options = (sampleCode & 0x10) ? 1 : 0;		// 1 = low power mode

			continue;
		}

		if (i == 0 && p[0] == 'H' && p[1] == 'A')			// OMX Header
		{
			omdata_metadata_t *md;

			fprintf(stderr, "OMDATA: Header (OMX)...\n");
			i += (numSectors - 1);		// Can skip multiple

			md = &omdata->metadata;
			memset(md, 0, sizeof(omdata_metadata_t));

			md->deviceType = READ_UINT16(p + 132);		// OMX@132 Device type/sub-type
			md->deviceTypeString[0] = (char)(md->deviceType);
			md->deviceTypeString[1] = (char)(md->deviceType >> 8);
			md->deviceTypeString[2] = 0;
			md->deviceVersion = READ_UINT16(p + 134);	// OMX@134 Device version
			md->deviceId = READ_UINT16(p + 136);		// OMX@136/CWA@5 Device id
			md->firmwareVer = READ_UINT16(p + 152);		// OMX@152 Firmware version (CWA@41 uint8_t)
			md->sessionId = READ_UINT32(p+158);			// OMX@158/CWA@7 Session identifier
			md->recordingStart = OmDataTimestamp(READ_UINT32(p + 162));	// OMX@162/CWA@13 Recording start time
			md->recordingStop = OmDataTimestamp(READ_UINT32(p + 166));	// OMX@166/CWA@17 Recording stop time
			md->stopReason = READ_UINT16(p + 170);		// OMX@170 Recording stop reason flags (0x00 = none, 0x01 = end of interval, 0x02 = commanded to stop, 0x04 = interrupted by connection, 0x08 = battery low, 0x10 = write error, 0x20 = measurement error)
			md->debuggingInfo = READ_UINT8(p + 172);	// OMX@172/CWA@26 Debugging mode
			memcpy(md->aux, p + 180, 16);				// OMX@180 (auxiliary data, write as zero)

			// OMX@196 Accelerometer configuration (CWA@36 uint8_t sample rate code)
			md->configAccel.stream = READ_UINT8(p + 196);
			md->configAccel.enabled = READ_UINT8(p + 197);
			md->configAccel.frequency = READ_UINT16(p + 198);
			md->configAccel.sensitivity = READ_UINT16(p + 200);
			md->configAccel.options = READ_UINT16(p + 202);

			// OMX@204 Gyroscope configuration
			md->configGyro.stream = READ_UINT8(p + 204);
			md->configGyro.enabled = READ_UINT8(p + 205);
			md->configGyro.frequency = READ_UINT16(p + 206);
			md->configGyro.sensitivity = READ_UINT16(p + 208);
			md->configGyro.options = READ_UINT16(p + 210);

			// OMX@212 Magnetometer configuration
			md->configMag.stream = READ_UINT8(p + 212);
			md->configMag.enabled = READ_UINT8(p + 213);
			md->configMag.frequency = READ_UINT16(p + 214);
			md->configMag.sensitivity = READ_UINT16(p + 216);
			md->configMag.options = READ_UINT16(p + 218);

			// OMX@220 Altimeter configuration
			md->configAltimeter.stream = READ_UINT8(p + 220);
			md->configAltimeter.enabled = READ_UINT8(p + 221);
			md->configAltimeter.frequency = READ_UINT16(p + 222);
			md->configAltimeter.sensitivity = READ_UINT16(p + 224);
			md->configAltimeter.options = READ_UINT16(p + 226);

			// OMX@228 Analogue configuration (Temperature, Light & Battery)
			md->configAnalog.stream = READ_UINT8(p + 228);
			md->configAnalog.enabled = READ_UINT8(p + 229);
			md->configAnalog.frequency = READ_UINT16(p + 230);
			md->configAnalog.sensitivity = READ_UINT16(p + 232);
			md->configAnalog.options = READ_UINT16(p + 234);

			// OMX@236 "All axis" configuration
			md->configAllAxis.stream = READ_UINT8(p + 236);
			md->configAllAxis.enabled = READ_UINT8(p + 237);
			md->configAllAxis.frequency = READ_UINT16(p + 238);
			md->configAllAxis.sensitivity = READ_UINT16(p + 240);
			md->configAllAxis.options = READ_UINT16(p + 242);

			memcpy(md->calibration, p + 244, 32);	// OMX@244 32 calibration words
			memset(md->metadata, 0, sizeof(md->metadata));
			memcpy(md->metadata, p + 318, 192);		// OMX@318/CWA@64 Metadata (6x32=192 in OMX, 14x32=448 in CWA)
			for (j = 0; j < sizeof(md->metadata); j++)
			{
				if (md->metadata[j] == 0xff) { md->metadata[j] = '\0'; }
			}
			for (j = strlen((const char *)md->metadata) - 1; j >= 0; j--)
			{
				if (md->metadata[j] != ' ') { break; }
				md->metadata[j] = '\0';
			}

			continue;
		}

		// Check checksum
		unsigned int d;
		unsigned short s = 0;
		for (d = 0; d < (OMDATA_SECTOR_SIZE / 2); d++)
		{
			s += ((const unsigned short *)p)[d];
		}
		if (s != 0) 
		{ 
			omdata->statsBadSectors++;
			fprintf(stderr, "OMDATA: Bad sector @%d checksum=0x%04x\n", i, s); 
			continue; 
		}


		// Data
		char format = -1;
		char streamIndex = -1;
		if ((p[0] == 'A' || p[0] == 'G' || p[0] == 'M') && p[1] == 'X')			// CWA Data
		{
			format = 0;
			streamIndex = p[0] - 'A' + 'a';
		}
		else if (p[0] == 'd')										// OMX Data
		{
			format = 1;
			streamIndex = p[1];
		}

		if (streamIndex < 0 || streamIndex >= OMDATA_MAX_STREAM)
		{
			if (p[0] == 's')
			{
				;	// Skipping string sector
			}
			else
			{
				fprintf(stderr, "OMDATA: Unhandled sector @%d header=%c%c\n", i, p[0], p[1]);
			}
			continue;
		}

		// Add sector
		if (OmDataAddSector(omdata, i, false) == 0)
		{
			omdata->statsDataSectors++;
		}
		else
		{
			fprintf(stderr, "OMDATA: Unexpected formatting for sector @%d header=%c%c\n", i, p[0], p[1]);
			omdata->statsBadSectors++;
			continue;
		}

		if (format == 0 && (streamIndex == 'a' || streamIndex == 'g'))
		{
			// Create virtual segments for CWA temperature, battery, light (embedded in normal accelerometer sectors) -- create a function to encapsulate below...
			//unsigned short values[3];  // [0]-batt, [1]-LDR, [2]-Temp
			OmDataAddSector(omdata, i, true);
		}

	}

	return 0;
}







static int OmDataProcessSegments(omdata_t *omdata)
{
	// Check each stream
	int streamIndex;
	for (streamIndex = 0; streamIndex < OMDATA_MAX_STREAM; streamIndex++)
	{
		omdata_stream_t *stream = &omdata->stream[streamIndex];
		if (!stream->inUse) { continue; }
		// Check each segment
		omdata_segment_t *seg;
		for (seg = stream->segmentFirst; seg != NULL; seg = seg->segmentNext)
		{
			if (seg->sectorCount > 0 && seg->timestampCount > 0 && seg->sampleRate > 0)
			{
				// Estimate segment start/stop time
				// TODO: This estimate is from the configured sample rate (not observed rate)

				// Start time
				{
					int index = seg->timestamps[0].sample;				// seg->timestampCount
					double timestamp = seg->timestamps[0].timestamp;	// seg->timestampCount
					seg->startTime = timestamp - (index / seg->sampleRate);
				}

				// End time
				{
					int index = seg->timestamps[seg->timestampCount - 1].sample;
					double timestamp = seg->timestamps[seg->timestampCount - 1].timestamp;
					seg->endTime = timestamp + ((seg->numSamples - index) / seg->sampleRate);
				}

			}
			else
			{
				seg->startTime = 0;
				seg->endTime = 0;
			}
		}
	}

	return 0;
}


static int OmDataCalculateSessions(omdata_t *omdata, double sessionOverlap)
{
	// Tracks segments within a stream
	typedef struct 
	{
		bool trackerStarted;
		bool segmentStarted;
		omdata_segment_t *currentSegment;
	} omdata_segment_tracker_t;
	omdata_segment_tracker_t segmentTrackers[OMDATA_MAX_STREAM] = { { 0 } };
	omdata_session_t *lastSession = NULL;
	omdata_session_t *currentSession = NULL;
	double currentTime = 0;
	int pass;

	// Find the sessions from the streams
	for (pass = 0;; pass++)
	{
		// Check each stream
		int streamIndex;
		int nextSmallestIndex = -1;
		int numStarted = 0;
		double nextSmallestTime = 0;
//fprintf(stderr, "--- PASS %d ---\n", pass);
		for (streamIndex = 0; streamIndex < OMDATA_MAX_STREAM; streamIndex++)
		{
			omdata_stream_t *stream = &omdata->stream[streamIndex];
			if (!stream->inUse) { continue; }

			omdata_segment_tracker_t *tracker = &segmentTrackers[streamIndex];

			// Check for initial condition
			if (!tracker->trackerStarted)
			{
//fprintf(stderr, "(Stream %c reset to first segment @%p)\n", streamIndex, stream->segmentFirst);
				tracker->currentSegment = stream->segmentFirst;
				tracker->trackerStarted = true;
			}

//fprintf(stderr, "Stream %c: segmentStarted=%d, currentSegment.startTime=%f, currentSegment.endTime=%f\n", streamIndex, tracker->segmentStarted, tracker->currentSegment ? tracker->currentSegment->startTime : 0.0, tracker->currentSegment ? tracker->currentSegment->endTime : 0.0);


			// Check for final condition
			if (tracker->currentSegment == NULL)
			{ 
//fprintf(stderr, "(Stream %c finished - skipping)\n", streamIndex);
				continue;
			}
			
			// If this is not started, should we start it?
			if (!tracker->segmentStarted && currentTime >= tracker->currentSegment->startTime)
			{
				tracker->segmentStarted = true;

//fprintf(stderr, "... segmentStarted! (currentSession=%f)\n", currentSession ? currentSession->startTime : -1);

				// We're recording a new segment -- check we have a current session
				if (currentSession == NULL)
				{
					// Need to make a new session
					currentSession = (omdata_session_t *)malloc(sizeof(omdata_session_t));
					memset(currentSession, 0, sizeof(omdata_session_t));
//fprintf(stderr, "... new session!\n");

					// If this is the first session one, set it
					if (omdata->firstSession == NULL)
					{
						omdata->firstSession = currentSession;
					}

					// This is the first segment in the session, set the start and end time
					currentSession->startTime = tracker->currentSegment->startTime;
					currentSession->endTime = tracker->currentSegment->endTime;

					// If this is not the first session, continue the chain
					if (lastSession != NULL)
					{
						lastSession->sessionNext = currentSession;
					}
					lastSession = currentSession;
				}

				// We're recording a new segment -- add it to the chain
				currentSession->stream[streamIndex].inUse = true;
				if (currentSession->stream[streamIndex].segmentFirst == NULL)
				{
//fprintf(stderr, "... first segment in session's chain.\n");
					currentSession->stream[streamIndex].segmentFirst = tracker->currentSegment;
				}
				// (if it's not the first, it will already be on the chain)
				currentSession->stream[streamIndex].segmentLast = tracker->currentSegment;


				// Update the start time if it's the earliest segment in the session

				// If this segment started earlier than the earliest segment in the session, set the start time
				if (tracker->currentSegment->startTime < currentSession->startTime)
				{
					currentSession->startTime = tracker->currentSegment->startTime;
				}

				// If this segment ended later than the latest segment in the session, set the end time
				if (tracker->currentSegment->endTime > currentSession->endTime)
				{
					currentSession->endTime = tracker->currentSegment->endTime;
				}

			}
			else if (tracker->segmentStarted && currentTime >= tracker->currentSegment->endTime) // If this has started, should we end it and move to the next segment?
			{
//fprintf(stderr, "... end segment.\n");
				tracker->segmentStarted = false;
				tracker->currentSegment = tracker->currentSegment->segmentNext;
//if (tracker->currentSegment == NULL) { fprintf(stderr, "... LAST SEGMENT!\n"); }
			}

			// Count the number of started segments
			if (tracker->segmentStarted) { numStarted++; }

			// If there are more segments in the stream...
			if (tracker->currentSegment != NULL)
			{
				// Find the next time to jump to
				double nextTime = tracker->segmentStarted ? tracker->currentSegment->endTime : tracker->currentSegment->startTime;
				if (nextTime < tracker->currentSegment->startTime) { nextTime = tracker->currentSegment->startTime; }	// Cope with an invalid range
				if (nextTime > currentTime && (nextSmallestIndex < 0 || nextTime <= nextSmallestTime))	// Ensure going forwards
				{
					nextSmallestTime = nextTime;
					nextSmallestIndex = streamIndex;
				}
			}

		}

//		fprintf(stderr, "==> %d active, current time %f, next time %f (%c)\n", numStarted, currentTime, nextSmallestTime, nextSmallestIndex);

		// Check if we're at the end of a session
		if (numStarted <= 0 && currentSession != NULL)
		{
			// Allow small gaps between segments (e.g. around a small area of corruption)
			if ((nextSmallestTime - currentTime) >= sessionOverlap)
			{
//fprintf(stderr, "! Gap too large: breaking segment chains and clearing the current session.\n");

				// Break the segment chains...
				for (streamIndex = 0; streamIndex < OMDATA_MAX_STREAM; streamIndex++)
				{
					omdata_segment_tracker_t *tracker = &segmentTrackers[streamIndex];
					if (tracker->currentSegment != NULL)
					{
						tracker->currentSegment->segmentNext = NULL;
						tracker->currentSegment = NULL;
					}
				}

				// We're no longer in a session
				currentSession = NULL;
			}
		}

		// Check for finished
		if (nextSmallestIndex < 0) { break; }

		// Advance time
		currentTime = nextSmallestTime;
	};

	return 0;
}


int OmDataCanLoad(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) { return 0; }
	//fseek(fp, 0, SEEK_END); long length = ftell(fp); fseek(fp, 0, SEEK_SET); buffer = (unsigned char *)malloc(length); if (buffer == NULL) { fclose(fp); return 0; }

	char buffer[4] = { 0 };
	if (fread(buffer, 1, sizeof(buffer), fp) != sizeof(buffer)) { fclose(fp); return 0; }
	fclose(fp);

	if (buffer[0] == 'M' && buffer[1] == 'D') { return 1; }
	else if (buffer[0] == 'H' && buffer[1] == 'A') { return 1; }
	else if (buffer[0] == 'd') { return 1; }
	else if (buffer[1] == 'X') { return 1; }

	return 0;
}



char OmDataAnalyzeTimestamps(omdata_t *omdata)
{
	if (omdata == NULL) { return -1; }

	int sectorCount = omdata->length / OMDATA_SECTOR_SIZE + 1;
	omdata->timestampOffset = (double *)calloc(sectorCount + 1, sizeof(double));

	FILE *dfp = NULL;
#if 0
	dfp = fopen("/temp/times.csv", "wt");
	fprintf(stderr, "NOTE: Writing timestamp log file...\n");
#endif
	//if (dfp == NULL) { return -1; }

	// Step 1. Determine any oscillator non-uniformity by looking at sample period
	int streamIndex;
	double worstDifference = 0.0f;
	double affectedT = 0;
	for (streamIndex = 0; streamIndex < OMDATA_MAX_STREAM; streamIndex++)
	{
		omdata_stream_t *stream = &omdata->stream[streamIndex];
		if (!stream->inUse) { continue; }
		if (streamIndex != 'a') { continue; }	// Only run on one channel of accel

		// Segment info
		omdata_segment_t *seg;
		for (seg = stream->segmentFirst; seg != NULL; seg = seg->segmentNext)
		{
			// Note seg->startTime and seg->endTime not yet valid
			//fprintf(stderr, "Sectors: %d\n", seg->sectorCount);
			//fprintf(stderr, "Samples: %d\n", seg->numSamples);
//			fprintf(stderr, ">>> %d samples in %d sectors with %d timestamps.\n", seg->numSamples, seg->sectorCount, seg->timestampCount);
			int i;
			omdata_segment_timestamp_t *lts = NULL;
			double startTime = 0;
			double slidingAverage = 0.0f;
			double lastPeriod = 0;
			int stableSamples = 0;
			for (i = 0; i < seg->timestampCount; i++)
			{
				omdata_segment_timestamp_t *ts = &seg->timestamps[i];
				int sectorIndexOffset = ts->sample / seg->samplesPerSector;
				if (sectorIndexOffset < 0 || sectorIndexOffset >= seg->sectorCount)
				{
					//fprintf(stderr, "WARNING: Sector index offset %d out of range (0-%d) for segment timestamp %d.\n", sectorIndexOffset, seg->sectorCount, i);
					if (seg->sectorCount <= 0) continue;
					sectorIndexOffset = seg->sectorCount - 1;
				}

				if (startTime == 0) { startTime = ts->timestamp; }
				if (lts != NULL && ts->sample > lts->sample)
				{
					double relT = ts->timestamp - startTime;
					int deltaI = ts->sample - lts->sample;
					double deltaT = ts->timestamp - lts->timestamp;
					double period = deltaT / deltaI;
					double freq = deltaI / deltaT;
					if (dfp != NULL) { fprintf(dfp, "%d,%f,%d,%f,%f\n", ts->sample, relT, deltaI, deltaT, freq); }

					double allowance = 0.006 * slidingAverage;
					double fade = 0.05 * deltaT;
					double diff = slidingAverage - period;

if (diff / slidingAverage > worstDifference) { worstDifference = diff / slidingAverage; }

					if (i <= 1) { slidingAverage = period; }
					else if (fabs(lastPeriod - period) < allowance) 
					{
						stableSamples += deltaI;
					}
					else 
					{ 
						stableSamples = 0;
					}

					if (fabs(diff) > allowance && relT > 90.0)
					{
						int sector = seg->sectorIndex[sectorIndexOffset];
						double timeslip = diff * deltaI;
						omdata->timestampOffset[sector] += timeslip;
//						printf("%02d:%02d:%02d.%02d,%f,%f,%d,%f\n", (int)relT / 60 / 60, ((int)relT / 60) % 60, (int)relT % 60, (int)((relT - (int)relT) * 100), slidingAverage, period, sector, timeslip);
						stableSamples = 0;
						affectedT += deltaT;
					}

					if (stableSamples > 10 * seg->sampleRate)
					{
						slidingAverage = ((1 - fade) * slidingAverage) + (fade * period);
					}


					lastPeriod = period;
				}
				lts = ts;
			}
		}
	}


	// Step 2. Cumulative sum of any timestamp offsets
	double cumulativeOffset = 0;
	for (int i = 0; i < sectorCount; i++)
	{
		cumulativeOffset += omdata->timestampOffset[i];
		omdata->timestampOffset[i] = cumulativeOffset;
if (i + 1 >= sectorCount && cumulativeOffset > 0.0) { fprintf(stderr, "DEBUG: Cumulative offset %f over %f (worst prop diff %f)\n", cumulativeOffset, affectedT, worstDifference); }
	}


	// Step 3. Apply any offsets to all streams
	for (streamIndex = 0; streamIndex < OMDATA_MAX_STREAM; streamIndex++)
	{
		omdata_stream_t *stream = &omdata->stream[streamIndex];
		if (!stream->inUse) { continue; }
		for (omdata_segment_t *seg = stream->segmentFirst; seg != NULL; seg = seg->segmentNext)
		{
			for (int i = 0; i < seg->timestampCount; i++)
			{
				omdata_segment_timestamp_t *ts = &seg->timestamps[i];
				int sectorIndexOffset = ts->sample / seg->samplesPerSector;
				if (sectorIndexOffset < 0 || sectorIndexOffset >= seg->sectorCount)
				{
					//fprintf(stderr, "WARNING: Sector index offset %d out of range (0-%d) for segment timestamp %d.\n", sectorIndexOffset, seg->sectorCount, i);
					if (seg->sectorCount <= 0) continue;
					sectorIndexOffset = seg->sectorCount - 1;
				}
				int sector = seg->sectorIndex[sectorIndexOffset];
				ts->timestamp += omdata->timestampOffset[sector];
			}
		}
	}


	if (dfp != NULL) { fclose(dfp); }
	return 0;
}


int OmDataLoad(omdata_t *omdata, const char *filename)
{
	unsigned char *buffer = NULL;

	fprintf(stderr, "OMDATA: Loading file: %s\n", filename);
	if (omdata == NULL) { return 0; }
	memset(omdata, 0, sizeof(omdata_t));
	if (filename == NULL || filename[0] == '\0') { return 0; }

	// Open the file
	int fd = _open(filename, _O_RDONLY | _O_BINARY);
	struct _stat sb;
	if (fd == -1) { fprintf(stderr, "ERROR: Problem opening file for reading.\n"); return 0; }
	if (_fstat(fd, &sb) == -1) { fprintf(stderr, "ERROR: Problem fstat-ing file.\n"); return 0; }
	long length = sb.st_size;

#ifdef USE_MMAP
	fprintf(stderr, "OMDATA: Mapping %d bytes...\n", (int)length);
	buffer = (unsigned char *)mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
	if (buffer == MAP_FAILED || buffer == NULL) { fprintf(stderr, "ERROR: Problem mapping %d bytes.\n", (int)length); _close(fd); return 0; }
	_close(fd);	// We can close the underlying file here
#else
	fprintf(stderr, "OMDATA: Allocating and reading %d bytes...\n", length);
	buffer = (unsigned char *)malloc(length);
	if (buffer == NULL) { _close(fd); fprintf(stderr, "ERROR: Problem allocating %d bytes.\n", length); return 0; }
	if (_read(fd, buffer, length) != length) { fprintf(stderr, "ERROR: Problem reading %d bytes.\n", length); free(buffer); _close(fd); return 0; }
	_close(fd);
#endif
		
	omdata->buffer = buffer;
	omdata->length = length;
	int sectorCount = omdata->length / OMDATA_SECTOR_SIZE;
	fprintf(stderr, "OMDATA: Processing sectors (%d)...\n", sectorCount);
	OmDataProcessSectors(omdata, 0, sectorCount);

	fprintf(stderr, "OMDATA: Analysing timestamps...\n");
	OmDataAnalyzeTimestamps(omdata);

	fprintf(stderr, "OMDATA: Processing segments...\n");
	OmDataProcessSegments(omdata);

	fprintf(stderr, "OMDATA: Determining sessions...\n");
	OmDataCalculateSessions(omdata, 7 * 24 * 60.0 * 60.0);	// Allow up to one week between sessions

	fprintf(stderr, "OMDATA: Processed.\n");
	return 1;
}


int OmDataDump(omdata_t *omdata)
{
	if (omdata != NULL)
	{
		// For each session
		int sessionIndex = 0;
		omdata_session_t *session;
		fprintf(stderr, "---\n");
		for (session = omdata->firstSession; session != NULL; session = session->sessionNext, sessionIndex++)
		{
			fprintf(stderr, "--- SESSION #%d ---\n", sessionIndex);

			int streamIndex;
			for (streamIndex = 0; streamIndex < OMDATA_MAX_STREAM; streamIndex++)
			{
				omdata_stream_t *stream = &session->stream[streamIndex];

				if (!stream->inUse) { continue; }

				fprintf(stderr, "--- STREAM %c ---\n", streamIndex);

				// Segment info
				int segmentIndex = 0;
				omdata_segment_t *seg;
				for (seg = stream->segmentFirst; seg != NULL; seg = seg->segmentNext, segmentIndex++)
				{
					fprintf(stderr, "--- SEGMENT %d ---\n", segmentIndex);
					fprintf(stderr, "Sectors: %d\n", seg->sectorCount);
					fprintf(stderr, "Samples: %d\n", seg->numSamples);
					fprintf(stderr, "Start time: %f\n", seg->startTime);
					fprintf(stderr, "End time: %f\n", seg->endTime);
				}
			}

		}
		fprintf(stderr, "===\n");

	}
	return 0;
}



int OmDataFree(omdata_t *omdata)
{
	if (omdata != NULL)
	{
		// For each stream
		int streamIndex;
		for (streamIndex = 0; streamIndex < OMDATA_MAX_STREAM; streamIndex++)
		{
			omdata_stream_t *stream = &omdata->stream[streamIndex];

			if (!stream->inUse) { continue; }

			// Free segment info
			omdata_segment_t *seg, *nextSeg;
			for (seg = stream->segmentFirst; seg != NULL; seg = nextSeg)
			{
				nextSeg = seg->segmentNext;
				free(seg);
			}
			stream->segmentFirst = NULL;
			stream->segmentLast = NULL;
		}

		// Free large buffer
		if (omdata->buffer != NULL)
		{
#ifdef USE_MMAP
			munmap((void *)omdata->buffer, (size_t)omdata->length);
#else
			free((void *)omdata->buffer);
#endif
			omdata->buffer = NULL;
		}
		omdata->length = 0;

		if (omdata->timestampOffset != NULL)
		{
			free(omdata->timestampOffset);
		}

		// Clear everything
		memset(omdata, 0, sizeof(omdata_t));
	}
	return 0;
}


char OmDataGetValues(omdata_t *data, omdata_segment_t *seg, int sampleIndex, int16_t *values)
{
	if (seg->samplesPerSector > 0 && seg->channels > 0)
	{
		int sectorWithinSegmentIndex = (sampleIndex / seg->samplesPerSector);

		if (sampleIndex >= 0 && sampleIndex < seg->numSamples && sectorWithinSegmentIndex < seg->sectorCount)
		{
			int sectorIndex = seg->sectorIndex[sectorWithinSegmentIndex];
			const unsigned char *p = (const unsigned char *)data->buffer + (OMDATA_SECTOR_SIZE * sectorIndex);

			int sampleWithinSector = (sampleIndex % seg->samplesPerSector);

			if (seg->packing == 0)	// Side-channel samples in CWA sectors (battery, light, temperature)
			{
				// Create virtual segments for CWA temperature, battery, light (embedded in normal accelerometer sectors) -- create a function to encapsulate below...
				//unsigned short values[3];  // [0]-batt, [1]-LDR, [2]-Temp
				values[0] = ((int16_t)p[23] << 1) + 512;		// @23 BYTE Battery - expand compressed byte into range
				values[1] = p[18] | ((int16_t)p[19] << 8);		// @18 WORD Light
				values[2] = p[20] | ((int16_t)p[21] << 8);		// @20 WORD Temperature
				//values[3] = p[22];								// @22 BYTE eventsFlag
				return 0;
			}
			else if (seg->packing == FILESTREAM_PACKING_SPECIAL_DWORD3_10_2)
			{
				int bytesPerSample = 4;
				const uint32_t *pp = (const uint32_t *)(p + seg->offset + (bytesPerSample * sampleWithinSector));
				uint32_t value = *pp;
				unsigned char e = (unsigned char)(value >> 30);		// 3=16g, 2=8g, 1=4g, 0=2g

				// [byte-3] [byte-2] [byte-1] [byte-0]
				// eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx
				// 10987654 32109876 54321098 76543210
				values[0] = (signed short)((unsigned short)(value << 6) & (unsigned short)0xffc0) >> (6 - e);		// Sign-extend 10-bit value, adjust for exponent
				values[1] = (signed short)((unsigned short)(value >> 4) & (unsigned short)0xffc0) >> (6 - e);		// Sign-extend 10-bit value, adjust for exponent
				values[2] = (signed short)((unsigned short)(value >> 14) & (unsigned short)0xffc0) >> (6 - e);		// Sign-extend 10-bit value, adjust for exponent

				// e: 3=16g (-4096 to 4095), 2=8g (-2048 to 2047), 1=4g (-1024 to 1023), 0=2g (-512 to 511).
				int clipMin = -(1 << (9 + e));
				int clipMax = -clipMin - 1;

				return (values[0] <= clipMin || values[0] >= clipMax || values[1] <= clipMin || values[1] >= clipMax || values[2] <= clipMin || values[2] >= clipMax);
			}
			else if ((seg->packing & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT16 || (seg->packing & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_UINT16)
			{
				int bytesPerSample = 2 * seg->channels;
				const int16_t *pp = (const int16_t *)(p + seg->offset + (bytesPerSample * sampleWithinSector));
				memcpy(values, pp, bytesPerSample);

				// TODO: Correct limits need to come from segment format/type
				int clipMin = -32768, clipMax = 32767;
				
				return (values[0] <= clipMin || values[0] >= clipMax || values[1] <= clipMin || values[1] >= clipMax || values[2] <= clipMin || values[2] >= clipMax);
			}
			else
			{
				// TODO: Fix API to work with variable width return types (currently 16-bit)
				fprintf(stderr, "!");
			}
		}
	}

	// Invalid
	memset(values, 0, OMDATA_MAX_CHANNELS * sizeof(int16_t));
	return 1;
}


