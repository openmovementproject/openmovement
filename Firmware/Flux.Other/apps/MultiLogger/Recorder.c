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

// Unified Logger - Recording and Streaming
// Dan Jackson, Karim Ladha

// Includes
#include <Compiler.h>
#include "HardwareProfile.h"
#include "TimeDelay.h"

#include "Analog.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Mag.h"
#include "Peripherals/Rtc.h"
#include "Peripherals/bmp085.h"
#include "Peripherals/Prox.h"
#ifdef USE_BLUETOOTH
#include "Bluetooth/Bluetooth.h"
#endif

#include "Utils/Fifo.h"
#include "Utils/DataStream.h"
#include "Utils/FileStream.h"

#include "Apps/MultiLogger/Logger.h"
#include "Apps/MultiLogger/Sampler.h"
#include "Apps/MultiLogger/Recorder.h"
#include "Apps/MultiLogger/Stream.h"
#include "Apps/MultiLogger/Settings.h"
#include "Utils/Util.h"
#include "Utils/FSUtils.h"
#include "MDD File System/FSIO.h"


// Logger State
logger_t logger = {0};

// Sector buffer for making the data sectors in
unsigned char __attribute__ ((aligned(2))) scratchBuffer[512];

// Log file
void *logFile = NULL;

// File streams
//static filestream_t accelFileStream, gyroFileStream, magFileStream, altFileStream, adcFileStream;

typedef struct
{
    filestream_t fileStream;
    datastream_t *dataStream;
    int samplesPerSector;
    int criticalLevel;
} recorder_stream_t;

// Important: uses the number and order from SENSOR_INDEX_* in Settings.h
recorder_stream_t recorderStreams[NUM_SENSORS];


// Macros
#define SET_WORD(_p, _v)  { ((unsigned char *)(_p))[0] = (unsigned char)(_v); ((unsigned char *)(_p))[1] = (unsigned char)((_v) >> 8); }
#define SET_DWORD(_p, _v) { ((unsigned char *)(_p))[0] = (unsigned char)(_v); ((unsigned char *)(_p))[1] = (unsigned char)((_v) >> 8); ((unsigned char *)(_p))[2] = (unsigned char)((_v) >> 16); ((unsigned char *)(_p))[3] = (unsigned char)((_v) >> 24); }
#define GET_WORD(_p)  ( (unsigned short)(((unsigned char *)(_p))[0]) | ((unsigned short)(((unsigned char *)(_p))[1]) << 8) )
#define GET_DWORD(_p) ( (unsigned long)(((unsigned char *)(_p))[0]) | ((unsigned long)(((unsigned char *)(_p))[1]) << 8) | ((unsigned long)(((unsigned char *)(_p))[2]) << 16) | ((unsigned long)(((unsigned char *)(_p))[3]) << 24) )


void RecorderInit()
{
    // Blank whole state
    memset(&logger, 0, sizeof(logger));

    // Set individual values
    logger.interval = INTERVAL_WAITING;
    logger.schedule = SCHEDULE_INTERVAL;
    logger.recording = RECORDING_OFF;
    logger.stream = STREAM_NONE;
    logger.sampling = SAMPLING_OFF;
 	logger.debugFlashCount = LED_STATE_FLASH_TIMEOUT;	// Reset debug LED count

    // Misc
    logger.filename[0] = '\0';
    logFile = NULL;

    // Setup peripherals to defaults
    //StateSetRecording(RECORDING_OFF, 1);
}


char LoggerOpen(const char *filename)
{
	// Open the file for append

    // Initialize variables
    //status.sampleCount = 0;
    //status.lastWrittenTicks = 0;
    //status.accelSequenceId = 0;
    //status.gyroSequenceId = 0;
    //status.debugFlashCount = 3;     // Initially flash logging status

	// Initialise flags
	//gDataReady.all = 0;
	//gStreamEnable.all = 0;

	if (filename == NULL) { logFile = NULL; return 0; }

    // Open the log file for append
    logFile = FSfopen(filename, "a");
    if (logFile == NULL)
	{
		// Failed
    	return 0;
	}

    // Calculate number of sectors in file
    FSfseek(logFile, 0, SEEK_END);                              // Ensure seeked to end (should be in append mode)

    // Ensure archive attribute set to indicate file updated
    ((FSFILE *)logFile)->attributes |= ATTR_ARCHIVE;

    // Initially flash to show started
	logger.debugFlashCount = LED_STATE_FLASH_TIMEOUT;	// Reset debug LED count

	return 1; // Done
}


static void DataSamplingInit(void)
{
	// Populate packing etc. Setup the streams, Clear the data buffers
    memset(recorderStreams, 0, sizeof(recorderStreams));        // Wipe current settings

	// Accelerometer
    {
        filestream_t *fileStream = &recorderStreams[SENSOR_INDEX_ACCEL].fileStream;
        recorderStreams[SENSOR_INDEX_ACCEL].dataStream = &accelDataStream;
        recorderStreams[SENSOR_INDEX_ACCEL].samplesPerSector = ACCEL_SECTOR_SAMPLE_COUNT;
        recorderStreams[SENSOR_INDEX_ACCEL].criticalLevel = SAMPLE_BUFFER_SIZE_ACCEL - (2 * 25) - 1;

#ifndef FILE_STREAM_USE_GLOBALS
        fileStream->fileHandle = (FSFILE *)logFile;             // Handle to the file stream -- either FILE* or FSFILE*
#endif
        fileStream->streamId = 'a';                             // [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)

        // ! Sample rate from settings
        fileStream->sampleRate = settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency;
        fileStream->sampleRateModifier = 1;                     // [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)

        // ! Sample rate from settings
        if (settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity == 8)
        {
            fileStream->dataType = FILESTREAM_DATATYPE_ACCEL_8G;// [1] Data type - see FileStream.h
            #ifdef ACCEL_8BIT_MODE
                fileStream->dataConversion = -4;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
            #else
                fileStream->dataConversion = -12;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
            #endif
        }
        else if (settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity == 4)
        {
            fileStream->dataType = FILESTREAM_DATATYPE_ACCEL_4G;// [1] Data type - see FileStream.h
            #ifdef ACCEL_8BIT_MODE
                fileStream->dataConversion = -5;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
            #else
                fileStream->dataConversion = -13;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
            #endif
        }
        else if (settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity == 2)
        {
            fileStream->dataType = FILESTREAM_DATATYPE_ACCEL_2G;// [1] Data type - see FileStream.h
            #ifdef ACCEL_8BIT_MODE
                fileStream->dataConversion = -6;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
            #else
                fileStream->dataConversion = -14;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
            #endif
        }
        else
        {
            fileStream->dataType = FILESTREAM_DATATYPE_ACCEL;   // [1] Data type - see FileStream.h
            fileStream->dataConversion = 0;                     // [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
        }
        #ifdef ACCEL_8BIT_MODE
            fileStream->channelPacking = (FILESTREAM_PACKING_3_CHANNEL|FILESTREAM_PACKING_SINT8);
        #else
            fileStream->channelPacking = (FILESTREAM_PACKING_3_CHANNEL|FILESTREAM_PACKING_SINT16);      		// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
        #endif

        FileStreamInit(fileStream, scratchBuffer);
    }

	// Gyro
    {
        filestream_t *fileStream = &recorderStreams[SENSOR_INDEX_GYRO].fileStream;
        recorderStreams[SENSOR_INDEX_GYRO].dataStream = &gyroDataStream;
        recorderStreams[SENSOR_INDEX_GYRO].samplesPerSector = GYRO_SECTOR_SAMPLE_COUNT;
        recorderStreams[SENSOR_INDEX_GYRO].criticalLevel = SAMPLE_BUFFER_SIZE_GYRO - (2 * 25) - 1;
        // ---
#ifndef FILE_STREAM_USE_GLOBALS
        fileStream->fileHandle = logFile;               // Handle to the file stream -- either FILE* or FSFILE*
#endif
        fileStream->streamId = 'g';            			// [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
        fileStream->sampleRate = settings.sensorConfig[SENSOR_INDEX_GYRO].frequency; // [2] Sample rate (Hz)
        fileStream->sampleRateModifier = 1;  			// [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
        if (settings.sensorConfig[SENSOR_INDEX_GYRO].sensitivity == 250)
        {
	        fileStream->dataType = FILESTREAM_DATATYPE_GYRO_250;// [1] Data type - see FileStream.h
        }
        else if (settings.sensorConfig[SENSOR_INDEX_GYRO].sensitivity == 500)
        {
	        fileStream->dataType = FILESTREAM_DATATYPE_GYRO_500;// [1] Data type - see FileStream.h
        }
        else if (settings.sensorConfig[SENSOR_INDEX_GYRO].sensitivity == 2000)
        {
	        fileStream->dataType = FILESTREAM_DATATYPE_GYRO_2000;// [1] Data type - see FileStream.h
        }
        else
        {
	        fileStream->dataType = FILESTREAM_DATATYPE_GYRO;// [1] Data type - see FileStream.h
		}
        fileStream->dataConversion = 0;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
        fileStream->channelPacking = (FILESTREAM_PACKING_3_CHANNEL|FILESTREAM_PACKING_SINT16);      		// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
        FileStreamInit(fileStream, scratchBuffer);
    }

	// Magnetometer
    {
        filestream_t *fileStream = &recorderStreams[SENSOR_INDEX_MAG].fileStream;
        recorderStreams[SENSOR_INDEX_MAG].dataStream = &magDataStream;
        recorderStreams[SENSOR_INDEX_MAG].samplesPerSector = MAG_SECTOR_SAMPLE_COUNT;
        recorderStreams[SENSOR_INDEX_MAG].criticalLevel = SAMPLE_BUFFER_SIZE_MAG-10;//(2 * 25) - 1; //KL: Fix, dan set this to less than the SECTOR_SAMPLE_COUNT, fixed set to 1s at 10Hz
        // ---
#ifndef FILE_STREAM_USE_GLOBALS
        fileStream->fileHandle = logFile;               // Handle to the file stream -- either FILE* or FSFILE*
#endif
        fileStream->streamId = 'm';            			// [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
        fileStream->sampleRate = settings.sensorConfig[SENSOR_INDEX_MAG].frequency;   // [2] Sample rate (Hz)
        fileStream->sampleRateModifier = 1;  			// [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
        fileStream->dataType = FILESTREAM_DATATYPE_MAGNETOMETER; // [1] Data type - see FileStream.h
        fileStream->dataConversion = 0;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
        fileStream->channelPacking = (FILESTREAM_PACKING_3_CHANNEL|FILESTREAM_PACKING_SINT16);      		// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
        FileStreamInit(fileStream, scratchBuffer);
    }

	// Altimeter
    {
        filestream_t *fileStream = &recorderStreams[SENSOR_INDEX_ALT].fileStream;
        recorderStreams[SENSOR_INDEX_ALT].dataStream = &altDataStream;
        recorderStreams[SENSOR_INDEX_ALT].samplesPerSector = ALT_SECTOR_SAMPLE_COUNT;
        recorderStreams[SENSOR_INDEX_ALT].criticalLevel = SAMPLE_BUFFER_SIZE_ALT - 4;
        // ---
#ifndef FILE_STREAM_USE_GLOBALS
        fileStream->fileHandle = logFile;          	// Handle to the file stream -- either FILE* or FSFILE*
#endif
        fileStream->streamId = 'p';                   // [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
        fileStream->sampleRate = 1; //settings.sensorConfig[SENSOR_INDEX_ALT].frequency;   // [2] Sample rate (Hz)
        fileStream->sampleRateModifier = 1;  			// [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
        fileStream->dataType = FILESTREAM_DATATYPE_PRESSURE; // [1] Data type - see FileStream.h
        fileStream->dataConversion = 0;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
        fileStream->channelPacking = (FILESTREAM_PACKING_1_CHANNEL|FILESTREAM_PACKING_SINT32);      		// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
        FileStreamInit(fileStream, scratchBuffer);
    }

	// ADC
    {
        filestream_t *fileStream = &recorderStreams[SENSOR_INDEX_ADC].fileStream;
        recorderStreams[SENSOR_INDEX_ADC].dataStream = &adcDataStream;
        recorderStreams[SENSOR_INDEX_ADC].samplesPerSector = ADC_SECTOR_SAMPLE_COUNT;
        recorderStreams[SENSOR_INDEX_ADC].criticalLevel = SAMPLE_BUFFER_SIZE_ADC - 4;
        // ---
#ifndef FILE_STREAM_USE_GLOBALS
        fileStream->fileHandle = logFile;          	// Handle to the file stream -- either FILE* or FSFILE*
#endif
        fileStream->streamId = 'l';            			// [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
        fileStream->sampleRate = 1;   //settings.sensorConfig[SENSOR_INDEX_ADC].frequency;   // [2] Sample rate (Hz)
        fileStream->sampleRateModifier = 1;  			// [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
        fileStream->dataType = FILESTREAM_DATATYPE_RAW_UINT; // [1] Data type - see FileStream.h
        fileStream->dataConversion = 0;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
        //fileStream->channelPacking = (FILESTREAM_PACKING_3_CHANNEL|FILESTREAM_PACKING_UINT16);      			// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)
  		fileStream->channelPacking = (((sizeof( adc_results_t )/2)<<4)|FILESTREAM_PACKING_UINT16); 
		FileStreamInit(fileStream, scratchBuffer);
    }

#ifdef SYNCHRONOUS_SAMPLING
	// "All axis" sampling
    {
        filestream_t *fileStream = &recorderStreams[SENSOR_INDEX_ALLAXIS].fileStream;
        recorderStreams[SENSOR_INDEX_ALLAXIS].dataStream = &allAxisDataStream;
        recorderStreams[SENSOR_INDEX_ALLAXIS].samplesPerSector = ALLAXIS_SECTOR_SAMPLE_COUNT;
        recorderStreams[SENSOR_INDEX_ALLAXIS].criticalLevel = SAMPLE_BUFFER_SIZE_ALLAXIS - (2 * ALLAXIS_SECTOR_SAMPLE_COUNT) - 1;

#ifndef FILE_STREAM_USE_GLOBALS
        fileStream->fileHandle = (FSFILE *)logFile;             // Handle to the file stream -- either FILE* or FSFILE*
#endif
        fileStream->streamId = 'x';                             // [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)

        // ! Sample rate from settings
        fileStream->sampleRate = settings.sensorConfig[SENSOR_INDEX_ALLAXIS].frequency;
        fileStream->sampleRateModifier = 1;                     // [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)

        // ! Sample rate from settings
        fileStream->dataType = FILESTREAM_DATATYPE_ACCEL_GYRO_MAG;   // [1] Data type - see FileStream.h
        fileStream->dataConversion = 0;                     	// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
        fileStream->channelPacking = (FILESTREAM_PACKING_9_CHANNEL|FILESTREAM_PACKING_SINT16);      		// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)

        FileStreamInit(fileStream, scratchBuffer);
    }
#endif

    return;
}


char StateSetRecording(StateRecording newState, unsigned short stopReason)
{
    if (logger.recording != RECORDING_ON && newState == RECORDING_ON)
    {
        // We must own the drive
        if (status.drive == DRIVE_OWNED)
        {
	        // Re-read settings (in case anything remotely changed at run time)
            LoggerReadMetadata(logger.filename);    // Read settings from the logging binary file
 
            // Start recording
            if (LoggerOpen(logger.filename))
            {
                StateSetSampling(SAMPLING_ON, 1);   // re-init (fixes FSFILE * in data stream)
                logger.recording = RECORDING_ON;
            }
            else
            {
                status.drive = DRIVE_OWNED_FULL;    // Prevent retry of file open
            }
        }
    }
    else if (logger.recording != RECORDING_OFF && newState == RECORDING_OFF)
    {
        // Stop recording
        RecorderStop(stopReason);
        logger.recording = RECORDING_OFF;
        
        // Turn off sampling if neither recording or streaming
        if (logger.stream == STREAM_NONE) { StateSetSampling(SAMPLING_OFF, 1); }
        else { StateSetSampling(SAMPLING_ON, 1); }      // re-init (fixes FSFILE * in data stream)
    }
    return 1;
}


char StateSetStream(StateStream newState)
{
    if (newState == logger.stream) { return 1; }
    if (logger.stream == STREAM_NONE && newState != STREAM_NONE)
    {
        // Ensure sampling
        StateSetSampling(SAMPLING_ON, 0);
    }
    else if (logger.stream != STREAM_NONE && newState == STREAM_NONE)
    {
        // Turn off sampling if neither recording or streaming
        if (logger.recording == RECORDING_OFF) { StateSetSampling(SAMPLING_OFF, 0); }
    }
    logger.stream = newState;
    return 1;
}


char StateSetSampling(StateSampling newState, char force)
{
    if (newState == SAMPLING_ON && (logger.sampling != SAMPLING_ON || force))
    {
        dataStreamFlags_t newFlags = {{0}};

        // === START SAMPLING ===

		SetupSampling(&newFlags); 	// First turn off all sensor interupts and streams

        DataSamplingInit(); 		// Setup for data streams

        // Switch on peripherals enabled in configuration
        newFlags.all = 0;
        newFlags.accel = settings.sensorConfig[SENSOR_INDEX_ACCEL].enabled;
        newFlags.gyro  = settings.sensorConfig[SENSOR_INDEX_GYRO].enabled;
        newFlags.mag   = settings.sensorConfig[SENSOR_INDEX_MAG].enabled;
        newFlags.alt   = settings.sensorConfig[SENSOR_INDEX_ALT].enabled;
        newFlags.adc   = settings.sensorConfig[SENSOR_INDEX_ADC].enabled;
        newFlags.accel_int2 = TRUE; // Allows orientation, taps, activity etc
		#ifdef USE_PROXIMITY_SENSOR_1HZ
        newFlags.prox = TRUE; 		// Updates prox sensor at 1Hz
		#endif
#ifdef SYNCHRONOUS_SAMPLING
		newFlags.allAxis = settings.sensorConfig[SENSOR_INDEX_ALLAXIS].enabled;
#endif

        SetupSampling(&newFlags);	// Turn on new/selected streams

        logger.sampling = SAMPLING_ON;
    }
    else if (newState == SAMPLING_OFF && (logger.sampling != SAMPLING_OFF || force))
    {
        dataStreamFlags_t newFlags = {{0}};

        // === STOP SAMPLING ===
        DataSamplingInit();

        // Switch off peripherals
        newFlags.all = 0;
        newFlags.accel_int2 = TRUE; // Allows orientation, taps, activity etc
		#ifdef USE_PROXIMITY_SENSOR_1HZ
        newFlags.prox = TRUE; 		// Updates prox sensor at 1Hz
		#endif

        SetupSampling(&newFlags);

        logger.sampling = SAMPLING_OFF;
    }
    return 1;
}



// Check each stream for data and write it out to the log file
// 0 = no write, 1 = data written, 2 = waiting to write, 3 = written & waiting
char LoggerPollAndWrite(dataStreamFlags_t forceWrite)
{
    int returnValue = 0;
    int index;
    int maxLoops;

    // Loop while writing breaking out when done (for safety, loop up to a maximum number of times)
    for (maxLoops = 20; maxLoops != 0; maxLoops--)
    {
        int writtenSector = 0;

        // Now check if any of the data streams has enough data to make up a sector
        for (index = 0; index < NUM_SENSORS; index++)
        {
            static unsigned int fifoLength; 		// Length of the fifo
            static char oneSector;      // write flags
            static char critical;       // write flags

            // Get current length of FIFO
            fifoLength = FifoLength(&recorderStreams[index].dataStream->fifo);
            oneSector = (fifoLength >= recorderStreams[index].samplesPerSector);

// [dgj] Quick hack for fast streaming
if (logger.recording != RECORDING_ON && logger.stream != STREAM_NONE && settings.streamFormat != STREAM_MODE_OMX_SLIP && fifoLength >= 0) { oneSector = 2; }

            // Critical to write if we're forcing a partial write of a non-empty buffer, or if the FIFO is almost full
            if (fifoLength > 0 && (((unsigned int)forceWrite.all >> index) & 1)) { critical = 1; }
            else { critical = (fifoLength >= recorderStreams[index].criticalLevel); }
            
            // Do nothing more if it's not critical and we don't have enough data for a whole sector
            if (!critical && !oneSector) { continue; }
                
            // If is not critical, delay sending whie any enabled streams are busy
            if (!critical && StreamerIsBusy(logger.stream)) { returnValue |= 2; continue; }

            // Write this sector
            {
                unsigned long timestamp;		// Time, factional and length must be set by the internal read functions
                unsigned short timeFractional;	// Fractional part of time
                unsigned int fifoLength; 		// Current time stamp is for the value at the end of the fifo
                unsigned int writeLength;
                unsigned int popped;
                static void *sectorData;

                DataStreamTimestamp(recorderStreams[index].dataStream, &timestamp, &timeFractional, &fifoLength); // Get stream vars
                writeLength = fifoLength;					// Number to write for next sector
                if (writeLength > 0)
                {
	                char ecc;
                    long offset = -1;
                    if (writeLength > recorderStreams[index].samplesPerSector) { writeLength = recorderStreams[index].samplesPerSector; } // Clamp write length to full sector
                    sectorData = FileStreamPrepareData(&recorderStreams[index].fileStream, timestamp, timeFractional, fifoLength, writeLength);
                    popped = FifoPop(&recorderStreams[index].dataStream->fifo, sectorData, writeLength);
                    if (popped != writeLength) { return -1; }   // Fifo pop fail

                    if (logFile != NULL)
                    {
// Green LED to indicate data write (flashes as will turn off when device sleeps, or in LedTasks)
if (settings.debuggingInfo >= 3) { LED_G = 1;}

                        offset = FSftell((FSFILE *)logFile);
                    }

                    // Output to file (MUST BE FIRST! Sets checksum, etc.)
                    ecc = status.dataEcc;
                    if (index == SENSOR_INDEX_ADC || index == SENSOR_INDEX_ALT) { ecc = 1; }	// Always use ECC for ADC and Alt.
                    if (FileStreamOutputData(&recorderStreams[index].fileStream, ecc, 1) )
					{
						writtenSector |= 1;
					}
					else if (logger.stream == STREAM_NONE)
					{
						return -1;
					}

                    if (logger.stream != STREAM_NONE)
                    {
                        char prefix[40];
						if (settings.streamFormat == STREAM_MODE_OMX_SLIP)
						{
							sprintf(prefix, "DATA,0,%ld,%u,-1\r\n", offset, (unsigned int)MEDIA_SECTOR_SIZE);
						} else { prefix[0] = '\0'; }
                        writtenSector |= StreamerOutputData(scratchBuffer, logger.stream, settings.streamFormat, (unsigned short)MEDIA_SECTOR_SIZE, prefix); // Output to stream
                    }

                    fifoLength -= writeLength;                  // Subtract from fifo length and loops back
                }
            }
        }
        
        if (!writtenSector) { break; }
        returnValue |= 1;       // written at least one sector
    }

//if (maxLoops == 0) {LED_R = 1;}
//if (settings.debuggingInfo >= 2) { LED_G = 0; }
//if (returnValue == 1) {LED_B = 1;}

	return returnValue;
}



void RecorderStop(unsigned short stopReason)
{

	// Force write remaining data in buffers
	LoggerPollAndWrite((dataStreamFlags_t)((unsigned short)0xFFFF)); // all streams forced

	// Finalise the file streams
	if (logFile != NULL)
	{   
        
        // Read the first sector
        FSfseek(logFile, 0, SEEK_SET);
        if (FSfread(scratchBuffer, 1, MEDIA_SECTOR_SIZE, logFile) == MEDIA_SECTOR_SIZE)
        {
            // Check the header signature is present
            if (scratchBuffer[0] == 'H' && scratchBuffer[1] == 'A')
            {
                // Update the header
                filestream_fileheader_t *fileHeader = (filestream_fileheader_t *)scratchBuffer;

                // @170 Recording stop reason flags (0x00 = none, 0x01 = end of interval, 0x02 = commanded to stop, 0x04 = interrupted by connection, 0x08 = battery low, 0x10 = write error, 0x20 = measurement error)
                fileHeader->stopReason |= (unsigned short)stopReason;

                // @510 16-bit word-wise checksum of packet
                fileHeader->checksum = checksum(fileHeader, 510);

                // Rewrite the first sector
                FSfseek(logFile, 0, SEEK_SET);
                FSfwrite(scratchBuffer, 1, MEDIA_SECTOR_SIZE, logFile);
            }
        }

        // Close the file
        FSfclose(logFile);
        logFile = NULL;
    }

	return;
}


char RecorderAnnotate(const void *annotation, unsigned short length)
{
    DateTime timestamp;
    unsigned short fractionalTime;
    filestream_data_t *packet;

    // Check we have an open handle
    if (logFile == NULL) { return 0; }

    // Use the scratch buffer
    packet = (filestream_data_t *)scratchBuffer;

    // Get timestamp
    timestamp = RtcNowFractional(&fractionalTime);

    // Clear buffer
    memset(scratchBuffer, 0, 512);

    // Set fields
    packet->packetType = 's';       // 's' = string
    packet->streamId = '*';         // '*' = all streams
    packet->payloadLength = 0x1fc;  // 508 bytes + 4 header/length = 512 bytes total
    packet->timestamp = timestamp;
    packet->fractionalTime = fractionalTime;
    packet->timestampOffset = 0;
    if (length > 480) { length = 480; }
    packet->sampleCount = length;

    // Add annotation data
    memcpy(packet->data, annotation, length);

    // Add checksum
    packet->checksum = checksum(packet, 510);

    // Output the sector
    if (!FSfwriteSector(packet, (FSFILE *)logFile, 1))        // always ECC annotations (status.dataEcc)
    {
        return 0;   // failed to write
    }

    // written ok
    return 1;
}


// Read metadata settings from a binary file
char LoggerReadMetadata(const char *filename)
{
    FSFILE *fp;
    int total;

    if (filename == NULL || filename[0] == '\0') { return 0; }
    
    fp = FSfopen(filename, "r");
    if (fp == NULL)
    {
        return 0;
    }

    // Ensure at start of the file
    FSfseek(fp, 0, SEEK_SET);

	// Read the first sector, close the file
	total = FSfread(scratchBuffer, 1, MEDIA_SECTOR_SIZE, fp);
    FSfclose(fp);
    if (total != MEDIA_SECTOR_SIZE)
    {
        return 0;
    }


	if (scratchBuffer[0] == 'H' && scratchBuffer[1] == 'A')
    {
        filestream_fileheader_t *fileHeader = (filestream_fileheader_t *)scratchBuffer;

        // @158 Session identifier
        settings.sessionId = fileHeader->sessionId;

        // @162 Recording start time
        settings.loggingStartTime = fileHeader->recordingStart;
        
        // @166 Recording stop time
        settings.loggingEndTime = fileHeader->recordingStop;            
        
        // @172 Debugging mode
        settings.debuggingInfo = fileHeader->debuggingInfo;

        // @196 Accelerometer configuration
        settings.sensorConfig[SENSOR_INDEX_ACCEL].enabled = fileHeader->configAccel.enabled;
        settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency = fileHeader->configAccel.frequency;
        settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity = fileHeader->configAccel.sensitivity;

        // @204 Gyroscope configuration
        settings.sensorConfig[SENSOR_INDEX_GYRO].enabled = fileHeader->configGyro.enabled;
        settings.sensorConfig[SENSOR_INDEX_GYRO].frequency = fileHeader->configGyro.frequency;
        settings.sensorConfig[SENSOR_INDEX_GYRO].sensitivity = fileHeader->configGyro.sensitivity;

        // @212 Magnetometer configuration
        settings.sensorConfig[SENSOR_INDEX_MAG].enabled = fileHeader->configMag.enabled;
        settings.sensorConfig[SENSOR_INDEX_MAG].frequency = fileHeader->configMag.frequency;
        settings.sensorConfig[SENSOR_INDEX_MAG].sensitivity = fileHeader->configMag.sensitivity;

        // @220 Altimeter configuration
        settings.sensorConfig[SENSOR_INDEX_ALT].enabled = fileHeader->configAltimeter.enabled;
        settings.sensorConfig[SENSOR_INDEX_ALT].frequency = fileHeader->configAltimeter.frequency;
        settings.sensorConfig[SENSOR_INDEX_ALT].sensitivity = fileHeader->configAltimeter.sensitivity;

        // @228 Analogue configuration (Temperature, Light & Battery)
        settings.sensorConfig[SENSOR_INDEX_ADC].enabled = fileHeader->configAnalog.enabled;
        settings.sensorConfig[SENSOR_INDEX_ADC].frequency = fileHeader->configAnalog.frequency;
        settings.sensorConfig[SENSOR_INDEX_ADC].sensitivity = fileHeader->configAnalog.sensitivity;
        
#ifdef SYNCHRONOUS_SAMPLING	
        // @236 "All axis" sampling configuration
        settings.sensorConfig[SENSOR_INDEX_ALLAXIS].enabled = fileHeader->configAllAxis.enabled;
        settings.sensorConfig[SENSOR_INDEX_ALLAXIS].frequency = fileHeader->configAllAxis.frequency;
        settings.sensorConfig[SENSOR_INDEX_ALLAXIS].sensitivity = fileHeader->configAllAxis.sensitivity;
#endif

        // @318 Metadata (6x32)
        memset(settings.annotation, ' ', ANNOTATION_SIZE * ANNOTATION_COUNT);   // Clear annotation buffer (larger than this file format holds)
        memcpy(settings.annotation, scratchBuffer + 318, 192);                  // Read file annotation data
        
        return 1;
    }
    else
#if 0
    // Old .CWA format parsing
	// Parse a basic metadata sector
	if (scratchBuffer[0] == 0x4d && scratchBuffer[1] == 0x44)             // [0] header    0x444D = ("MD") Meta data block
    {
        unsigned short deviceId;
        unsigned char sampleRate;
        deviceId = GET_WORD(scratchBuffer + 5);                          // [5] deviceId
        settings.sessionId = GET_DWORD(scratchBuffer + 7);               // [7] sessionId
        settings.loggingStartTime = GET_DWORD(scratchBuffer + 13);       // [13] loggingStartTime
        settings.loggingEndTime = GET_DWORD(scratchBuffer + 17);         // [17] loggingEndTime
        //settings.maximumSamples = GET_DWORD(scratchBuffer + 21);       // [21] loggingCapacity
        settings.debuggingInfo = scratchBuffer[26];                      // [26] debuggingInfo

        sampleRate = scratchBuffer[36];                                  // [36] samplingRate
        settings.sensorConfig[SENSOR_INDEX_ACCEL].enabled = 1;
        settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency = ACCEL_FREQUENCY_FOR_RATE(sampleRate);
        settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity = ACCEL_RANGE_FOR_RATE(sampleRate);

        //settings.lastChangeTime = GET_DWORD(scratchBuffer + 37);         // [37] lastChangeTime
        //settings.timeZone = (signed short)GET_WORD(scratchBuffer + 42);  // [42] timeZone
        memcpy(settings.annotation, scratchBuffer + 64, ANNOTATION_SIZE * ANNOTATION_COUNT); // [64] 14x 32-byte annotation chunks

        // Only OK if this file has the right device id
    	return (deviceId == settings.deviceId);
    }
    else
#endif
    {
    	return 0;
    }
}


// Write metadata settings to a binary file
char LoggerWriteMetadata(const char *filename)
{
    FSFILE *fp;
    char retval = 0;

    if (filename == NULL || filename[0] == '\0') { return 0; }

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

    // Ensure at start of the file
    FSfseek(fp, 0, SEEK_SET);

#if 1   // newer header
    {   
        filestream_fileheader_t *fileHeader = (filestream_fileheader_t *)scratchBuffer;
        char *p;
        
        // Zero whole sector
        memset(scratchBuffer, 0x00, MEDIA_SECTOR_SIZE);

        fileHeader->packetType = 'H';                           // @0 Packet type (ASCII 'H' = header)
        fileHeader->packetSubType = 'A';                        // @1 Packet sub-type (ASCII 'A')
        fileHeader->payloadLength = 0x01fc;                     // @2 <0x01FC> Payload length (508, +4 type/length = 512 bytes)

        // @4 Machine-written human-readable comment (device type, version, id, firmware, session id, start/stop time, sensor config?)
        memset(fileHeader->comment, 0x00, sizeof(fileHeader->comment));
        p = fileHeader->comment;
        p += sprintf(p, "ID,%s,%02x,%u,%u\r\n", HardwareName(), HARDWARE_VERSION, SOFTWARE_VERSION, settings.deviceId);
        p += sprintf(p, "SI,%lu\r\n", settings.sessionId);
        p += sprintf(p, "T0,%s\r\n", RtcToString(settings.loggingStartTime));
        p += sprintf(p, "T1,%s\r\n", RtcToString(settings.loggingEndTime));
        p += sprintf(p, "%c,%d,%d\r\n", 'A', settings.sensorConfig[SENSOR_INDEX_ACCEL].enabled ? settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency : 0, settings.sensorConfig[SENSOR_INDEX_ACCEL].enabled ? settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity : 0);
        p += sprintf(p, "%c,%d,%d\r\n", 'G', settings.sensorConfig[SENSOR_INDEX_GYRO].enabled  ? settings.sensorConfig[SENSOR_INDEX_GYRO].frequency  : 0, settings.sensorConfig[SENSOR_INDEX_GYRO].enabled  ? settings.sensorConfig[SENSOR_INDEX_GYRO].sensitivity  : 0);
        p += sprintf(p, "%c,%d\r\n",    'M', settings.sensorConfig[SENSOR_INDEX_MAG].enabled   ? settings.sensorConfig[SENSOR_INDEX_MAG].frequency   : 0);
        p += sprintf(p, "%c,%d\r\n"  ,  'P', settings.sensorConfig[SENSOR_INDEX_ALT].enabled   ? settings.sensorConfig[SENSOR_INDEX_ALT].frequency   : 0);
        p += sprintf(p, "%c,%d\r\n",    'T', settings.sensorConfig[SENSOR_INDEX_ADC].enabled   ? settings.sensorConfig[SENSOR_INDEX_ADC].frequency   : 0);

        fileHeader->deviceType = DeviceTag();                   // @132 Device type/sub-type
        fileHeader->deviceVersion = HARDWARE_VERSION;           // @134 Device version
        fileHeader->deviceId = settings.deviceId;               // @136 Device id
        fileHeader->firmwareVer = SOFTWARE_VERSION;             // @152 Firmware version
        fileHeader->sessionId = settings.sessionId;             // @158 Session identifier
        fileHeader->recordingStart = settings.loggingStartTime; // @162 Recording start time
        fileHeader->recordingStop = settings.loggingEndTime;    // @166 Recording stop time
        fileHeader->stopReason = 0x00;                          // @170 Recording stop reason (0 = unknown, 1 = stop time reached, 2 = commanded to stop, 3 = interrupted by connection, 4 = low battery, 5 = low memory, 6 = measurement error)
        fileHeader->debuggingInfo = settings.debuggingInfo;     // @172 Debugging mode

        // @196 Accelerometer configuration
        fileHeader->configAccel.enabled         = settings.sensorConfig[SENSOR_INDEX_ACCEL].enabled;
        fileHeader->configAccel.frequency       = settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency;
        fileHeader->configAccel.sensitivity     = settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity;

        // @204 Gyroscope configuration
        fileHeader->configGyro.enabled          = settings.sensorConfig[SENSOR_INDEX_GYRO].enabled;
        fileHeader->configGyro.frequency        = settings.sensorConfig[SENSOR_INDEX_GYRO].frequency;
        fileHeader->configGyro.sensitivity      = settings.sensorConfig[SENSOR_INDEX_GYRO].sensitivity;

        // @212 Magnetometer configuration
        fileHeader->configMag.enabled           = settings.sensorConfig[SENSOR_INDEX_MAG].enabled;
        fileHeader->configMag.frequency         = settings.sensorConfig[SENSOR_INDEX_MAG].frequency;
        fileHeader->configMag.sensitivity       = settings.sensorConfig[SENSOR_INDEX_MAG].sensitivity;

        // @220 Altimeter configuration
        fileHeader->configAltimeter.enabled     = settings.sensorConfig[SENSOR_INDEX_ALT].enabled;
        fileHeader->configAltimeter.frequency   = settings.sensorConfig[SENSOR_INDEX_ALT].frequency;
        fileHeader->configAltimeter.sensitivity = settings.sensorConfig[SENSOR_INDEX_ALT].sensitivity;

        // @228 Analogue configuration (Temperature, Light & Battery)
        fileHeader->configAnalog.enabled        = settings.sensorConfig[SENSOR_INDEX_ADC].enabled;
        fileHeader->configAnalog.frequency      = settings.sensorConfig[SENSOR_INDEX_ADC].frequency;
        fileHeader->configAnalog.sensitivity    = settings.sensorConfig[SENSOR_INDEX_ADC].sensitivity;

#ifdef SYNCHRONOUS_SAMPLING	
        // @236 "All axis" sampling configuration
        fileHeader->configAllAxis.enabled        = settings.sensorConfig[SENSOR_INDEX_ALLAXIS].enabled;
        fileHeader->configAllAxis.frequency      = settings.sensorConfig[SENSOR_INDEX_ALLAXIS].frequency;
        fileHeader->configAllAxis.sensitivity    = settings.sensorConfig[SENSOR_INDEX_ALLAXIS].sensitivity;
#endif

        // @244 32 calibration words
        memcpy(fileHeader->calibration, status.calibration, 32 * 2);

        // @318 Metadata (6x32)
        memcpy(fileHeader->metadata, settings.annotation, 192);

        // @510 16-bit word-wise checksum of packet
        fileHeader->checksum = checksum(fileHeader, 510);
        
        // Output the sector
        if (FSfwrite(scratchBuffer, 1, MEDIA_SECTOR_SIZE, fp) == 512) { retval = 1; }
    }
#else   // old (.cwa) header
    {
        unsigned int total = 0;
        DateTime lastChangeTime;
        lastChangeTime = RtcNow();
        //settings.lastChangeTime = lastChangeTime;

        // Create a basic metadata sector
        memset(scratchBuffer, 0xff, MEDIA_SECTOR_SIZE);
        scratchBuffer[0] = 0x4d; scratchBuffer[1] = 0x44;             // [0] header    0x444D = ("MD") Meta data block
        scratchBuffer[2] = 0xfc; scratchBuffer[3] = 0x03;             // [2] blockSize 0x3FC (block is 1020 bytes long, + 2 + 2 header = 1024 bytes total)
        scratchBuffer[4] = 0x00;                                     // [4] performClear
        SET_WORD(scratchBuffer + 5, settings.deviceId);              // [5] deviceId
        SET_DWORD(scratchBuffer + 7, settings.sessionId);            // [7] sessionId
        SET_DWORD(scratchBuffer + 13, settings.loggingStartTime);    // [13] loggingStartTime
        SET_DWORD(scratchBuffer + 17, settings.loggingEndTime);      // [17] loggingEndTime
        //SET_DWORD(scratchBuffer + 21, settings.maximumSamples);      // [21] loggingCapacity
        scratchBuffer[26] = settings.debuggingInfo;                  // [26] debuggingInfo
        // [32] lastClearTime
        //scratchBuffer[36] = settings.sampleRate;                     // [36] samplingRate
        scratchBuffer[36] = (unsigned char)AccelSetting(settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency, settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity);

        SET_DWORD(scratchBuffer + 37, lastChangeTime);               // [37] lastChangeTime
        scratchBuffer[41] = SOFTWARE_VERSION;                        // [41] Firmware revision number
        //SET_WORD(scratchBuffer + 42, settings.timeZone);             // [42] Time Zone offset from UTC (in minutes), 0xffff = -1 = unknown
        memcpy(scratchBuffer + 64, settings.annotation, ANNOTATION_SIZE * ANNOTATION_COUNT); // [64] 14x 32-byte annotation chunks

        // Output the first sector
        total += FSfwrite(scratchBuffer, 1, MEDIA_SECTOR_SIZE, fp);

        // Output the second sector (some programs expect the metadata chunk to be at least two sectors long)
        memset(scratchBuffer, 0xff, MEDIA_SECTOR_SIZE);
        total += FSfwrite(scratchBuffer, 1, MEDIA_SECTOR_SIZE, fp);
        retval = (total == 1024);
    }
#endif
    
    FSfclose(fp);

    return retval;
}


// Extract the numeric value of a filename given the name and a template, e.g. "DATA0012.OMX" & "DATA????.OMX" --> 12
int LoggerGetFilenameNumber(const char *filespec, const char *filename)
{
    const char *s = filespec;
    const char *n = filename;
    int number = -1;
    if (s == NULL || n == NULL) { return -1; }
    for (;;)
    {
        if (*s == '?' && *n >= '0' && *n <= '9')
        {
            int num;
            if (number < 0) { number = 0; }
            num = number * 10 + (*n - '0');
            if (num < number) { return -1; }              // Check for overflow
            number = num;
        }
        else if (*s != *n) { number = -1; break; }      // Mismatch. Case sensitive is ok - everything should be normalized to upper case at this point
        else if (*s == '\0') { break; }
        s++; n++;
    }
    if (number < 0) { number = 0; }                     // Treat file specs with no number placeholders as zero-value
    return number;
}


// Set a numbered filename from a template, e.g. "DATA????.OMX" & 12 --> "DATA0012.OMX".
int LoggerSetFilenameNumber(const char *filespec, char *filename, int number)
{
    int num = number;
    int o;
    if (filename == NULL) { return -1; }
    if (filespec == NULL || num < 0) { filename[0] = '\0'; return -1; }
    for (o = strlen(filespec); o >= 0; o--)
    {
        char c = filespec[o];
        if (c == '?') { c = '0' + (num % 10); num /= 10; }
        filename[o] = c;
    }
    // Return the actual value written
    return LoggerGetFilenameNumber(filespec, filename);
}


int LoggerFindCurrentFile(const char *filespec, char *filename, unsigned long *size)
{
	SearchRec rec;
    int largestNum = -1;
    int safety = 0;
    unsigned long returnSize = 0;

    if (size != NULL) { *size = 0; }
    if (filespec == NULL) { return -1; }
	if (status.drive != DRIVE_OWNED && status.drive != DRIVE_OWNED_FULL) { return -1; }   	// disallow if mounted
    if (filename != NULL)
    {
        filename[0] = '\0';
        LoggerSetFilenameNumber(filespec, filename, 0);
    }

	if (!FindFirst(status.filespec, ATTR_ARCHIVE | ATTR_READ_ONLY, &rec))	//  = all entries
	{
	    do
	    {
	        if (!(rec.attributes & ATTR_DIRECTORY) && !(rec.attributes & ATTR_VOLUME) && !(rec.attributes & ATTR_HIDDEN))
	        {
                int num = LoggerGetFilenameNumber(status.filespec, rec.filename);
                if (num > largestNum)
                {
                    largestNum = num;
                    if (filename != NULL) { strcpy(filename, rec.filename); }
                    //if (rec.attributes & ATTR_ARCHIVE) { returnFlags |= 0x01; }
                    returnSize = rec.filesize;
                }
	        }
            if (safety++ >= 10000) { break; }   // guard against corrupt FS
	    } while (!FindNext(&rec));
	}

    if (size != NULL) { *size = returnSize; }
    
    return largestNum;
}


// Logger tasks
void RecordingTasks(void)
{
    char writeState = 0;

    if (logger.recording == RECORDING_ON || logger.stream != STREAM_NONE)
    {
    	writeState = LoggerPollAndWrite((dataStreamFlags_t)((unsigned short)0x0000)); // no streams forced
		if (writeState == -1)
		{
			StateSetRecording(RECORDING_OFF, RECORD_STOP_REASON_WRITE_ERROR);
			status.drive = DRIVE_ERROR;
			if (FSDiskFull())
			{
              	status.drive = DRIVE_OWNED_FULL;    // Prevent retry of file open
			}
		}
    }

    // Cancel data streaming if the transport is lost (TODO: Just treat as a bitfield and mask out)
    if (status.wireless != WIRELESS_CONNECTED)
    {
        if (logger.stream == STREAM_WIRELESS)  { logger.stream = STREAM_NONE; }
        else if (logger.stream == STREAM_BOTH) { logger.stream = STREAM_USB; }
    }
    if (status.connection != CONNECTION_COMPUTER)
    {
        if (logger.stream == STREAM_USB)       { logger.stream = STREAM_NONE; }
        else if (logger.stream == STREAM_BOTH) { logger.stream = STREAM_WIRELESS; }
    }

    // Cancel file streaming if the transport is lost
    if ((logger.fileReadStream == STREAM_WIRELESS && status.wireless != WIRELESS_CONNECTED) || (logger.fileReadStream == STREAM_USB && status.connection != CONNECTION_COMPUTER))
    {
        logger.readLength = 0;
        if (logger.readFile != NULL) { FSfclose(logger.readFile); logger.readFile = NULL; }
    }

    // If we're not waiting to stream a sector, and we have data to read, and we're not currently busy streaming
    if (!(writeState & 2) && (logger.readLength > 0) && !StreamerIsBusy(logger.fileReadStream))
    {
        // If we have a file open...
        if (logger.readFile != NULL && logger.fileReadStream != STREAM_NONE)
        {
            unsigned int len;
            unsigned int dataLen;

            // Go to the right location if we're not already there
            if (FSftell(logger.readFile) != logger.readPosition)
            {
                FSfseek(logger.readFile, logger.readPosition, SEEK_SET);
            }

            // Read up to one sector
            len = (unsigned int)logger.readLength;
            if (len > MEDIA_SECTOR_SIZE) { len = MEDIA_SECTOR_SIZE; }
            dataLen = FSfread(scratchBuffer, 1, len, logger.readFile);

            // Output the data
            {
                char prefix[40];
                sprintf(prefix, "DATA,1,%lu,%u,%ld\r\n", logger.readPosition, dataLen, ((FSFILE *)logger.readFile)->size - logger.readPosition + dataLen);
                StreamerOutputData(scratchBuffer, logger.fileReadStream, STREAM_MODE_OMX_SLIP /*settings.streamFormat*/, dataLen, prefix); // Output to stream
            }

            logger.readLength -= len;
            logger.readPosition += len;

            // EOF
            if (len <= 0) { logger.readLength = 0; }
        }
    }
}

//EOF

