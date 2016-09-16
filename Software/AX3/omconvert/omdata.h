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

#ifndef OMLOAD_H
#define OMLOAD_H


#include <stdint.h>
#include <stdbool.h>
//#include <stdlib.h>


#define OMDATA_SECTOR_SIZE 512

#define OMDATA_MAX_STREAM 128

#define OMDATA_MAX_CHANNELS 16


// Timestamp for a sample number within a segment
typedef struct
{
	double timestamp;
	int sample;
} omdata_segment_timestamp_t;


// Data 'segment' (contiguous sectors) for a stream, forms a chain of segments
struct omdata_segment_tag_t;
typedef struct omdata_segment_tag_t
{
	struct omdata_segment_tag_t *segmentNext;
	double startTime;
	double endTime;

	// Index of sectors (each with 'samplesPerSector' up to 'numSamples', only the last sector may have fewer)
	unsigned int *sectorIndex;
	int sectorCapacity;
	int sectorCount;

	// Timestamps (monotonically increasing index)
	omdata_segment_timestamp_t *timestamps;
	int timestampCapacity;
	int timestampCount;

	// Configuration (this will be constant along an entire segment)
	int offset;				// Offset of data within sector (depends on format)
	int packing;			// The type of data and the way the data is packed
	int channels;			// The number of channels
	double scaling;			// The required conversion into units
	int samplesPerSector;	// The number of samples in every sector (the last sector in a segment is permitted to have fewer)
	char lastPacketShort;	// Whether the last packet is short
	int numSamples;			// Total number of samples
	double sampleRate;		// Sample rate (as configured)

} omdata_segment_t;

// Data stream information
typedef struct
{
	bool inUse;
	omdata_segment_t *segmentFirst;
	omdata_segment_t *segmentLast;
	uint32_t lastSequenceId;
} omdata_stream_t;

// Session type
struct omdata_session_tag_t;
typedef struct omdata_session_tag_t
{
	omdata_stream_t stream[OMDATA_MAX_STREAM];
	struct omdata_session_tag_t *sessionNext;
	double startTime;
	double endTime;
} omdata_session_t;


// CWA header
//	unsigned short header;					// CWA@0 0x444D = ("MD") Meta data block
//	unsigned short blockSize;				// CWA@2 0xFFFC = Packet size (2^16 - 2 - 2)

////1 x	unsigned char performClear;				// CWA@4 0 = none, 1 = data, 2 = full
////[1 byte @ 4, previously performClear]
////-> 1 byte calibrationEnabled (99 'c' = enabled but not present, 67 'C' = enabled and present)

//	unsigned short deviceId;				// CWA@5
//	unsigned long sessionId;				// CWA@7
//x	unsigned short shippingMinLightLevel;	// CWA@11
//	unsigned long loggingStartTime;			// CWA@13
//	unsigned long loggingEndTime;			// CWA@17
//	unsigned long loggingCapacity;			// CWA@21
//x	unsigned char allowStandby;				// CWA@25 User allowed to transition LOGGING->STANDBY (and if POWERED->STANDBY/LOGGING)
//	char debuggingInfo;						// CWA@26 Additional LED debugging info (0=no double-tap, 1=double-tap detection, 2=constant flash status)

//2 x	unsigned short batteryMinimumToLog;		// CWA@27 Minimum battery level required for logging
//2 x	unsigned short batteryWarning; 			// CWA@29 Battery level below which show warning colour for logging status
//1 x	unsigned char enableSerial;				// CWA@31 Serial commands enabled during logging (will consume more power as processor in IDLE not sleep
//4 x	unsigned long lastClearTime;			// CWA@32 Last clear data time
////[9 bytes @27, previously batteryMinimumToLog, batteryWarning, enableSerial, lastClearTime]
////-> 1 byte  reserved (0)
////-> 4 bytes temperatureOffsetX
////-> 4 bytes temperatureOffsetY

//	unsigned char samplingRate;				// CWA@36 Sampling rate
//	unsigned long lastChangeTime;			// CWA@37 Last change metadata time
//	unsigned char firmwareRevision;			// CWA@41 Firmware revision number
//	signed short timeZone;					// CWA@42 Time Zone offset from UTC (in minutes), 0xffff = -1 = unknown

////20 x	unsigned char reserved[20];				// CWA@44 Another 20 bytes reserved before the annotation starts
////[20 bytes @44, previously reserved]
////-> 6 bytes scaleMinusOneXYZ
////-> 6 bytes offsetXYZ
////-> 4 bytes referenceTemperature
////-> 4 bytes temperatureOffsetZ

//	unsigned char annotation[448];			// CWA@64 Annotation (user text metadata), 14* 32-byte chunks (448 bytes total), "LBL=1234567890123456789012345678"

// OMX Header
//	unsigned char packetType;				// OMX@0 Packet type (ASCII 'H' = header)
//	unsigned char packetSubType;			// OMX@1 Packet sub-type (ASCII 'A')
//	unsigned short payloadLength;			// OMX@2 <0x01FC> Payload length (508, +4 type/length = 512 bytes)
//	char comment[128];						// OMX@4 Machine-written human-readable comment (device type, version, id, firmware, session id, start/stop time, sensor config?)
//	unsigned short deviceType;				// OMX@132 Device type/sub-type
//	unsigned short deviceVersion;			// OMX@134 Device version
//	unsigned short deviceId;				// OMX@136 Device id
//	unsigned char reserved1[14];			// OMX@138 (reserved, write as zero)
//	unsigned short firmwareVer;				// OMX@152 Firmware version
//	unsigned char reserved2[4];				// OMX@154 (reserved, write as zero)
//	unsigned long sessionId;				// OMX@158 Session identifier
//	unsigned long recordingStart;			// OMX@162 Recording start time
//	unsigned long recordingStop;			// OMX@166 Recording stop time
//	unsigned short stopReason;				// OMX@170 Recording stop reason flags (0x00 = none, 0x01 = end of interval, 0x02 = commanded to stop, 0x04 = interrupted by connection, 0x08 = battery low, 0x10 = write error, 0x20 = measurement error)
//	unsigned char debuggingInfo;			// OMX@172 Debugging mode
//	unsigned char reserved3;				// OMX@173 (reserved, write as zero)
//	unsigned char reserved4[6];				// OMX@174 (reserved, write as zero)
//	unsigned char aux[16];					// OMX@180 (auxiliary data, write as zero)
//	omdata_sensor_config_t configAccel;		// OMX@196 Accelerometer configuration
//	omdata_sensor_config_t configGyro;		// OMX@204 Gyroscope configuration
//	omdata_sensor_config_t configMag;		// OMX@212 Magnetometer configuration
//	omdata_sensor_config_t configAltimeter;	// OMX@220 Altimeter configuration
//	omdata_sensor_config_t configAnalog;	// OMX@228 Analogue configuration (Temperature, Light & Battery)
//	omdata_sensor_config_t configAllAxis;	// OMX@236 "All axis" configuration
//	unsigned char calibration[74];			// OMX@244 74 calibration bytes
//	unsigned char metadata[192];			// OMX@318 Metadata (6x32=192)

// Sensor settings
typedef struct
{
	char stream;
	char enabled;
	unsigned short frequency;
	unsigned short sensitivity;
	unsigned short options;
} omdata_sensor_config_t;

// Metadata
typedef struct
{
	char deviceTypeString[32];
	unsigned short deviceType;				// OMX@132 Device type/sub-type
	unsigned short deviceVersion;			// OMX@134 Device version
	unsigned short deviceId;				// OMX@136/CWA@5 Device id
	unsigned short firmwareVer;				// OMX@152 Firmware version (CWA@41 uint8_t)
	unsigned long sessionId;				// OMX@158/CWA@7 Session identifier
	unsigned long recordingStart;			// OMX@162/CWA@13 Recording start time
	unsigned long recordingStop;			// OMX@166/CWA@17 Recording stop time
	unsigned short stopReason;				// OMX@170 Recording stop reason flags (0x00 = none, 0x01 = end of interval, 0x02 = commanded to stop, 0x04 = interrupted by connection, 0x08 = battery low, 0x10 = write error, 0x20 = measurement error)
	unsigned char debuggingInfo;			// OMX@172/CWA@26 Debugging mode
	unsigned char aux[16];					// OMX@180 (auxiliary data, write as zero)
	//unsigned long clearTime;				// CWA@32 "last clear time" (CWA only)
	//unsigned long changeTime;				// CWA@37 "last change metadata time" (CWA only)
	omdata_sensor_config_t configAccel;		// OMX@196 Accelerometer configuration (CWA@36 uint8_t sample rate code)
	omdata_sensor_config_t configGyro;		// OMX@204 Gyroscope configuration
	omdata_sensor_config_t configMag;		// OMX@212 Magnetometer configuration
	omdata_sensor_config_t configAltimeter;	// OMX@220 Altimeter configuration
	omdata_sensor_config_t configAnalog;	// OMX@228 Analogue configuration (Temperature, Light & Battery)
	omdata_sensor_config_t configAllAxis;	// OMX@236 "All axis" configuration
	short calibration[32];					// OMX@244 32 calibration words
	unsigned char metadata[448 + 1];		// OMX@318/CWA@64 Metadata (6x32=192 in OMX, 14x32=448 in CWA)
} omdata_metadata_t;

// Data type
typedef struct
{
	const unsigned char *buffer;
	size_t length;
	double *timestampOffset;
	omdata_stream_t stream[OMDATA_MAX_STREAM];
	omdata_session_t *firstSession;
	omdata_metadata_t metadata;

	int statsTotalSectors;		// Total number of input sectors (including non-data sectors)
	int statsBadSectors;		// Total number of bad sectors
	int statsDataSectors;		// Total number of data sectors
} omdata_t;


// Check whether can load data
int OmDataCanLoad(const char *filename);

// Load data
int OmDataLoad(omdata_t *omdata, const char *filename);

// Debug dump data summary
int OmDataDump(omdata_t *omdata);

// Retrieve values from a segment-offset (returns if clipped)
char OmDataGetValues(omdata_t *data, omdata_segment_t *seg, int sampleIndex, int16_t *values);

// Get the timestamp and sample offset for a specific sector
double OmDataTimestampForSector(omdata_t *omdata, int sectorIndex, int *sampleIndexOffset);

// Free data resources
int OmDataFree(omdata_t *omdata);

#endif

