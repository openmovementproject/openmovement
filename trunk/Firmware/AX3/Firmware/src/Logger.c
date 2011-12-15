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

// Data Logger
// Dan Jackson, 2011

// Includes
#include <Compiler.h>
#include "HardwareProfile.h"
#include "Peripherals/Accel.h"
//#include "Peripherals/Gyro.h"
#include "Peripherals/Rtc.h"
#include "Ftl/FsFtl.h"
#include "util.h"
#include "Analogue.h"
#include "Data.h"
#include "Logger.h"
#include "Settings.h"


#define SECTOR_SIZE 512
#define DATA_PACKET_SIZE SECTOR_SIZE
#define BUFFER_SAMPLE_COUNT_PACKED 120
#define BUFFER_SAMPLE_COUNT_UNPACKED 80


// 512-byte data packet -- all WORD/DWORD stored as little-endian (LSB first)
typedef struct DataPacket_t
{
	unsigned short packetHeader;	// [2] = 0x5841 (ASCII "AX", little-endian)
	unsigned short packetLength;	// [2] = 0x01FC (contents of this packet is 508 bytes long, + 2 + 2 header = 512 bytes total)
	unsigned short deviceId;		// [2] (16-bit device identifier, 0 = unknown)
	unsigned long  sessionId;		// [4] (32-bit unique session identifier, 0 = unknown)
	unsigned long  sequenceId;		// [4] (32-bit sequence counter, each packet has a new number -- reset if restarted?)
	unsigned long  timestamp;		// [4] (last reported RTC value, 0 = unknown) [YYYYYYMM MMDDDDDh hhhhmmmm mmssssss]
	unsigned short light;			// [2] (last recorded light sensor value in raw units, 0 = none)
	unsigned short temperature;		// [2] (last recorded temperature sensor value in raw units, 0 = none)
	unsigned char  events;			// [1] (event flags since last packet, b0 = resume logging from standby, b1 = single-tap event, b2 = double-tap event, b3-b7 = reserved)
	unsigned char  battery;			// [1] (last recorded battery level in 6/256V, 0 = unknown)
	unsigned char  sampleRate;		// [1] = sample rate code (3200/(1<<(15-(rate & 0x0f)))) Hz, if 0, then old format where sample rate stored in 'timestampOffset' field as whole number of Hz
	unsigned char  numAxesBPS;		// [1] = 0x32 (top nibble: number of axes = 3; bottom nibble: bytes-per-sample; 2=3x 16-bit signed; 0=3x 10-bit signed + 2 bit exponent)
	signed short   timestampOffset;	// [2] = [if sampleRate is non-zero:] Relative sample index from the start of the buffer where the whole-second timestamp is valid [otherwise, if sampleRate is zero, this is the old format with the sample rate in Hz]
	unsigned short sampleCount;		// [2] = 80 or 120 samples (number of accelerometer samples)
//	Accel sampleData[BUFFER_SAMPLE_COUNT];
//	unsigned short checksum;		// [2] = [if sampleRate is non-zero:] checksum of packet, [if sampleRate is zero:] reserved (0x0000)
} DataPacket;

// (sizeof(DataPacket) + BUFFER_SAMPLE_COUNT * 4 + 2 != DATA_PACKET_SIZE)


// IMPORTANT: This structure has major packing/alignment problems on PIC24 -- word access must be word-aligned.

/*
// Data status structure
typedef struct DataMeta_t
{
    // Header
    unsigned short header;              // [0] 0x444D = ("MD") Meta data block
    unsigned short blockSize;           // [2] 0xFFFC = Packet size (2^16 - 2 - 2)

	// Stored data
	unsigned char performClear;         // [4] 0 = none, 1 = data, 2 = full
	unsigned short deviceId;            // [5]
	unsigned long sessionId;            // [7]
	unsigned short shippingMinLightLevel; // [11]
	unsigned long loggingStartTime;     // [13]
	unsigned long loggingEndTime;       // [17]
	unsigned long loggingCapacity;      // [21]
	unsigned char allowStandby;			// [25] User allowed to transition LOGGING->STANDBY (and if POWERED->STANDBY/LOGGING)
	char debuggingInfo;					// [26] Additional LED debugging info (0=no double-tap, 1=double-tap detection, 2=constant flash status)
	unsigned short batteryMinimumToLog;	// [27] Minimum battery level required for logging
	unsigned short batteryWarning; 		// [29] Battery level below which show warning colour for logging status
	unsigned char enableSerial;			// [31] Serial commands enabled during logging (will consume more power as processor in IDLE not sleep
	unsigned long lastClearTime;        // [32] Last clear data time
	unsigned char samplingRate;			// [36] Sampling rate
	unsigned long lastChangeTime;       // [37] Last change metadata time
	unsigned char firmwareRevision;		// [41] Firmware revision number
	signed short timeZone;				// [42] Time Zone offset from UTC (in minutes), 0xffff = -1 = unknown

    // unsigned char reserved[20];      // [44] Another 20 bytes reserved before the annotation starts

    // [@64] Annotation (user text metadata), 14* 32-byte chunks (448 bytes total), "LBL=1234567890123456789012345678"
    // NOTE: Annotations are NOT stored in RAM, they are left in flash memory
    //Annotation annotation[];
} DataMeta;
*/




// Output buffer
unsigned char scratchBuffer[640];       // 1 sector (512) + longest expected CSV line
#define loggerBuffer scratchBuffer
unsigned short bufferOffset = 0;

// Macros
#define SET_WORD(_p, _v)  { ((unsigned char *)(_p))[0] = (unsigned char)(_v); ((unsigned char *)(_p))[1] = (unsigned char)((_v) >> 8); }
#define SET_DWORD(_p, _v) { ((unsigned char *)(_p))[0] = (unsigned char)(_v); ((unsigned char *)(_p))[1] = (unsigned char)((_v) >> 8); ((unsigned char *)(_p))[2] = (unsigned char)((_v) >> 16); ((unsigned char *)(_p))[3] = (unsigned char)((_v) >> 24); }
#define GET_WORD(_p)  ( (unsigned short)(((unsigned char *)(_p))[0]) | ((unsigned short)(((unsigned char *)(_p))[1]) << 8) )
#define GET_DWORD(_p) ( (unsigned long)(((unsigned char *)(_p))[0]) | ((unsigned long)(((unsigned char *)(_p))[1]) << 8) | ((unsigned long)(((unsigned char *)(_p))[2]) << 16) | ((unsigned long)(((unsigned char *)(_p))[3]) << 24) )


// Read metadata settings from a binary file
char LoggerReadMetadata(const char *filename)
{
    FSFILE *fp;
    int total;

    fp = FSfopen(filename, "r");
    if (fp == NULL)
    {
        return 0;
    }

    // Ensure at start of the file
    FSfseek(fp, 0, SEEK_SET);

	// Read the first sector, close the file
	total = FSfread(loggerBuffer, 1, SECTOR_SIZE, fp);
    FSfclose(fp);
    if (total != SECTOR_SIZE)
    {
        return 0;
    }

	// Parse a basic metadata sector
	if (loggerBuffer[0] == 0x4d && loggerBuffer[1] == 0x44)             // [0] header    0x444D = ("MD") Meta data block
    {
        unsigned short deviceId;
        deviceId = GET_WORD(loggerBuffer + 5);                          // [5] deviceId
        settings.sessionId = GET_DWORD(loggerBuffer + 7);               // [7] sessionId
        settings.loggingStartTime = GET_DWORD(loggerBuffer + 13);       // [13] loggingStartTime
        settings.loggingEndTime = GET_DWORD(loggerBuffer + 17);         // [17] loggingEndTime
        settings.maximumSamples = GET_DWORD(loggerBuffer + 21);         // [21] loggingCapacity
        settings.debuggingInfo = loggerBuffer[26];                      // [26] debuggingInfo
        settings.sampleRate = loggerBuffer[36];                         // [36] samplingRate
        settings.lastChangeTime = GET_DWORD(loggerBuffer + 37);         // [37] lastChangeTime
        settings.timeZone = (signed short)GET_WORD(loggerBuffer + 42);  // [42] timeZone
        memcpy(settings.annotation, loggerBuffer + 64, ANNOTATION_SIZE * ANNOTATION_COUNT); // [64] 14x 32-byte annotation chunks

        // Only OK if this file has the right device id
    	return (deviceId == settings.deviceId);
    }
    else
    {
    	return 0;
    }
}


// Write metadata settings to a binary file
char LoggerWriteMetadata(const char *filename)
{
    FSFILE *fp;
    unsigned int total = 0;

    // Can't just use "a+" as (for portability) not allowed to write anywhere except end of stream,
    fp = FSfopen(filename, "r+");
    if (fp == NULL)
    {
        // "r+" won't create a new file, make sure it exists here
        fp = FSfopen(filename, "w+");
        if (fp == NULL) { return 0; }
    }

    // Ensure archive attribute set to indicate file updated
    fp->attributes |= ATTR_ARCHIVE;

    settings.lastChangeTime = RtcNow();

    // Ensure at start of the file
    FSfseek(fp, 0, SEEK_SET);

	// Create a basic metadata sector
	memset(loggerBuffer, 0xff, SECTOR_SIZE);
	loggerBuffer[0] = 0x4d; loggerBuffer[1] = 0x44;             // [0] header    0x444D = ("MD") Meta data block
	loggerBuffer[2] = 0xfc; loggerBuffer[3] = 0x03;             // [2] blockSize 0x3FC (block is 1020 bytes long, + 2 + 2 header = 1024 bytes total)
	loggerBuffer[4] = 0x00;                                     // [4] performClear
    SET_WORD(loggerBuffer + 5, settings.deviceId);              // [5] deviceId
    SET_DWORD(loggerBuffer + 7, settings.sessionId);            // [7] sessionId
	SET_DWORD(loggerBuffer + 13, settings.loggingStartTime);    // [13] loggingStartTime
    SET_DWORD(loggerBuffer + 17, settings.loggingEndTime);      // [17] loggingEndTime
    SET_DWORD(loggerBuffer + 21, settings.maximumSamples);      // [21] loggingCapacity
	loggerBuffer[26] = settings.debuggingInfo;                  // [26] debuggingInfo
    // [32] lastClearTime
    loggerBuffer[36] = settings.sampleRate;                     // [36] samplingRate
    SET_DWORD(loggerBuffer + 37, settings.lastChangeTime);      // [37] lastChangeTime
	loggerBuffer[41] = SOFTWARE_VERSION;                        // [41] Firmware revision number
    SET_WORD(loggerBuffer + 42, settings.timeZone);             // [42] Time Zone offset from UTC (in minutes), 0xffff = -1 = unknown
    memcpy(loggerBuffer + 64, settings.annotation, ANNOTATION_SIZE * ANNOTATION_COUNT); // [64] 14x 32-byte annotation chunks

	// Output the first sector
	total += FSfwrite(loggerBuffer, 1, SECTOR_SIZE, fp);

    // Output the second sector (some programs expect the metadata chunk to be at least two sectors long)
    memset(loggerBuffer, 0xff, SECTOR_SIZE);
	total += FSfwrite(loggerBuffer, 1, SECTOR_SIZE, fp);

    FSfclose(fp);

	return (total == 1024);
}


static FSFILE *logFile = NULL;


// Start logging
char LoggerStart(const char *filename)
{
    char exists;

    // Initialize variables
    status.sampleCount = 0;
    status.lastWrittenTicks = 0;
    status.events = DATA_EVENT_RESUME;
    status.sequenceId = 0;
    status.debugFlashCount = 3;     // Initially flash logging status
    bufferOffset = 0;

    // Check we can open the file...
    exists = 0;
    logFile = FSfopen(filename, "r");
    if (logFile != NULL)
    {
        exists = 1;
        FSfclose(logFile);
    }

    if ((settings.dataMode & FORMAT_MASK_TYPE) ==  FORMAT_CWA_PACKED || (settings.dataMode & FORMAT_MASK_TYPE) == FORMAT_CWA_UNPACKED)
    {
        char packed = (settings.dataMode & FORMAT_CWA_PACKED);
        unsigned short samples = packed ? BUFFER_SAMPLE_COUNT_PACKED : BUFFER_SAMPLE_COUNT_UNPACKED;

        // Start a new file if it doesn't exist
        if (!exists)
        {
            LoggerWriteMetadata(filename);
        }

        // Open the log file for append
        logFile = FSfopen(filename, "a");
        if (logFile != NULL)
        {
            unsigned long length;

            // Calculate number of sectors in file
            FSfseek(logFile, 0, SEEK_END);                              // Ensure seeked to end (should be in append mode)
            length = FSftell(logFile) / SECTOR_SIZE;                    // Length in sectors
            if (length > 2) { length -= 2; } else { length = 0; }       // Remove 2 sectors of header

            // Calculate total sample count
            status.sampleCount = length * samples;

            // Ensure archive attribute set to indicate file updated
            logFile->attributes |= ATTR_ARCHIVE;
            
            // Initially flash to show started
            status.debugFlashCount = 5;

            // Success
            return 1;
        }
    }
    else if ((settings.dataMode & FORMAT_MASK_TYPE) == FORMAT_CSV)
    {
        // Start a new file if it doesn't exist
        if (!exists)
        {
            // Open the log file for append
            logFile = FSfopen(filename, "w");

            if (logFile != NULL)
            {
                // Print header
                bufferOffset += sprintf((char *)loggerBuffer + bufferOffset, "Time,Ax*256,Ay*256,Az*256\r\n");
            }
        }
        else
        {
            // Open the log file for append
            logFile = FSfopen(filename, "a");
        }

        if (logFile != NULL)
        {
            // Ensure archive attribute set to indicate file updated
            logFile->attributes |= ATTR_ARCHIVE;

            // Initially flash to show started
            status.debugFlashCount = 5;

            return 1;
        }
    }
    else
    {
        ; // Unknown/no file format
    }

    // Failed
    return 0;
}


// Stop logging
void LoggerStop(void)
{
    if (logFile != NULL)
    {
        // If we're logging a partial sector at a time...
        if ((settings.dataMode & FORMAT_MASK_TYPE) == FORMAT_CSV)
        {
            // If we've got a partially-written sector, flush it
            if (bufferOffset > 0)
            {
                FSfwrite(loggerBuffer, 1, bufferOffset, logFile);
                bufferOffset = 0;
            }
        }

        // Close the file
        FSfclose(logFile);
        logFile = NULL;
    }
}




// Write a logging sector
char LoggerWrite(void)
{
    unsigned short pending;
    char ret = 0;

    pending = DataLength();

    if ((settings.dataMode & FORMAT_MASK_TYPE) ==  FORMAT_CWA_PACKED || (settings.dataMode & FORMAT_MASK_TYPE) == FORMAT_CWA_UNPACKED)
    {
        char packed = (settings.dataMode & FORMAT_CWA_PACKED);
        unsigned short samples = packed ? BUFFER_SAMPLE_COUNT_PACKED : BUFFER_SAMPLE_COUNT_UNPACKED;

        if (pending >= samples)
        {
            DataPacket *dp = (DataPacket *)loggerBuffer;
            unsigned long timestamp;
            short relativeOffset;
            unsigned short i;
            unsigned short battery;

            if (settings.debuggingInfo >= 3 || (settings.debuggingInfo >= 2 && status.debugFlashCount)) { LED_SET(GREEN); }
            if (status.debugFlashCount) { status.debugFlashCount--; }

            // New battery scaling
            battery = ADCresult[ADC_BATTERY];
            if (battery < 512) { battery = 0; }
            else { battery -= 512; }
            if (battery > 255) { battery = 255; }

            // Update ADC reading
            InitADCOn();
            SampleADC();

            // Update events from data overflow
            status.events |= DataEvents();

            // Calculate timestamp and offset
            DataTimestamp(&timestamp, &relativeOffset);

            dp->packetHeader = 0x5841;              // [2] = 0x5841 (ASCII "AX", little-endian)
            dp->packetLength = 0x01FC;              // [2] = 0x01FC (contents of this packet is 508 bytes long, + 2 + 2 header = 512 bytes total)
            dp->deviceId = settings.deviceId;     	// [2] (16-bit device identifier, 0 = unknown)
            dp->sessionId = settings.sessionId;     // [4] (32-bit unique session identifier, 0 = unknown)
            dp->sequenceId = (status.sequenceId++);	// [4] (32-bit sequence counter, each packet has a new number -- reset if restarted?)
            dp->timestamp = timestamp;              // [4] (last reported RTC value, 0 = unknown) [YYYYYYMM MMDDDDDh hhhhmmmm mmssssss]
            dp->light = ADCresult[ADC_LDR];         // [2] (last recorded light sensor value in raw units, 0 = none)
            dp->temperature = ADCresult[ADC_TEMP];  // [2] (last recorded temperature sensor value in raw units, 0 = none)
            dp->events = status.events;             // [1] (event flags since last packet, b0 = resume logging from standby, b1 = single-tap event, b2 = double-tap event, b3 = reserved, b4 = hardware overflow, b5 = software overflow, b6-b7 = reserved)
            status.events = 0;
            dp->battery = battery;                  // [1] (last recorded battery level, Voltage = Value * 3 / 512 + 3)
            dp->sampleRate = settings.sampleRate;	// [1] = sample rate code (3200/(1<<(15-(rate & 0x0f)))) Hz, if 0, then old format where sample rate stored in 'timestampOffset' field as whole number of Hz
            dp->numAxesBPS = 0x30;               	// [1] = 0x30 / 0x32 (top nibble: number of axes = 3; bottom nibble: bytes-per-sample; 2=3x 16-bit signed; 0=3x 10-bit signed + 2 bit exponent)
            dp->timestampOffset = relativeOffset;	// [2] = [if sampleRate is non-zero:] Relative sample index from the start of the buffer where the whole-second timestamp is valid [otherwise, if sampleRate is zero, this is the old format with the sample rate in Hz]
            dp->sampleCount = samples;              // [2] = 80 or 120 samples (number of accelerometer samples)

            // Store accelerometer values
            if (!packed)
            {
                // Retrieve raw accelerometer values
                DataPop((DataType *)(loggerBuffer + 30), samples);
            }
            else
            {
                // Pack accelerometer values
                // TODO: Make this more efficient for bulk processing
                for (i = 0; i < samples; i++)
                {
                    DataType value;
                    DataPop(&value, 1);
                    AccelPackData((short *)&value, loggerBuffer + 30 + (i << 2));
                }
            }

            // Calculate and store checksum
            *(unsigned short *)&loggerBuffer[510] = checksum(loggerBuffer, 510);

            // Output the sector
            if (FSfwriteSector(loggerBuffer, logFile, FALSE))       // No ECC for data sectors to improve read speed (they are protected by a checksum)
            {
                status.sampleCount += samples;
                ret = 1;
            }

            // Ensure LED off
            LED_SET(OFF);
        }
    }
    else if ((settings.dataMode & FORMAT_MASK_TYPE) == FORMAT_CSV)
    {
        if (pending > 0)
        {
            unsigned short i;
            unsigned short fractional;
            const char *timeString;

            // NOTE: As using the hardware FIFO, it makes correct timestamps a lot trickier.
            // HACK: For now, just use the current time
            timeString = RtcToString(RtcNowFractional(&fractional));
            fractional = RTC_FRACTIONAL_TO_MS(fractional);

            // Now that the raw sample data is buffered, it makes it slightly trickier to do efficient sector-based CSV writing...
            for (i = 0; i < pending; i++)
            {
                DataType value;
                DataPop(&value, 1);

                // Append new line (must not be greater than 128 bytes)
                bufferOffset += sprintf((char *)loggerBuffer + bufferOffset, "%s.%03d,%d,%d,%d\r\n", timeString, fractional, value.x, value.y, value.z);
                status.sampleCount++;

                // If we've completed a sector
                if (bufferOffset >= SECTOR_SIZE)
                {
                    if (settings.debuggingInfo >= 3 || (settings.debuggingInfo >= 2 && status.debugFlashCount)) { LED_SET(GREEN); }
                    if (status.debugFlashCount) { status.debugFlashCount--; }

                    // Write the sector
                    if (FSfwriteSector(loggerBuffer, logFile, TRUE))       // ECC for CSV files
                    {
                        ret = 1;
                    }

                    // Move up any characters that overflowed the sector (the buffer allows no more than 128 of these)
                    memmove(loggerBuffer, loggerBuffer + bufferOffset, (bufferOffset - SECTOR_SIZE));   // (memmove just in case we allow > 512 extra in the future)
                    bufferOffset -= SECTOR_SIZE;

                    // Ensure LED off
                    LED_SET(OFF);
                }
            }
        }

    }
    else
    {
        ;   // Unknown logging type (or none)
    }

    return ret;
}

