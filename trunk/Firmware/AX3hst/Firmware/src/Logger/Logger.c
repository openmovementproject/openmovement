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
 *
 * New logger code using new file format
 * KL 24-01-2012
 */
 
// Includes
#include <Compiler.h>
#include "HardwareProfile.h"
#include "TimeDelay.h"

#include "Peripherals/Analog.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Rtc.h"
#include "Peripherals/ADC MCP3421.h"
#include "Peripherals/TEMP MCP9800.h"

#include "Logger.h"
#include "Settings.h"
#include "Utils/Util.h"
#include "Utils/Fifo.h"
#include "Utils/DataStream.h"
#include "Utils/FileStream.h"
#include "MDD File System/FSIO.h"

#define DATA_STREAM_SW_INT_IF IFS3bits.INT4IF
#define DATA_STREAM_SW_INT_IE IEC3bits.INT4IE
#define DATA_STREAM_SW_INT_IP IPC13bits.INT4IP

// Data buffers, must be >1 sectors worth of samples (480/entrySize) + (sampleRate * writeLatencyTime) 
#define ACCEL_SECTOR_SAMPLE_COUNT	80 		/*Number of samples per sector*/
accel_t accelBuffer[160]; /*0.8 seconds spare*/

#define TEMP_SECTOR_SAMPLE_COUNT	120 	/*Number of samples per sector*/
typedef struct {
	signed short probe;
	signed short board;
}temp_t;
temp_t tempBuffer[130]; /*10 seconds spare*/

#define ADC_SECTOR_SAMPLE_COUNT 120
typedef struct {
	signed short light;
	signed short batt;
}adcVals_t;
adcVals_t adcBuffer[240]; /* 4 seconds spare */

// Sector buffer for making the data sectors in
unsigned char scratchBuffer[512];

// Restart flag
extern unsigned char restart;

// Log file
static FSFILE * logFile = NULL;

// Other globals

// Data streams
datastream_t 	accelDataStream, tempDataStream, adcDataStream;
// File streams
filestream_t 	accelFileStream, tempFileStream, adcFileStream;

// Data ready flags
typedef union {
	struct {
		unsigned char accel  : 1;
		unsigned char temp   : 1;
		unsigned char adc	 : 1;
		unsigned char accel_int2 : 1;
		unsigned char unused : 4;
	};
	unsigned char all;
} dataStreamFlags_t;

volatile dataStreamFlags_t gDataReady, gStreamEnable ;

// Inactivity measure - used to turn the accel on / off
#define INACTIVITY_THRESHOLD 30
volatile unsigned int inactivity_timer = 0;


// Interrupts for getting fifo time stamps at RTC_INT_PRIORITY or below
void LoggerTimerTasks(void)
{
	if((gStreamEnable.adc)&&(!gDataReady.adc))	
	{
		DataStreamUpdateTimestamp(&adcDataStream); 	// Update the time stamp
		gDataReady.adc = TRUE; 						// Indicates fifo should sample the values
		DATA_STREAM_SW_INT_IF = 1; 					// SW trigger fifo int tasks
	}	
}
void LoggerRTCTasks(void) // Called at 1Hz from rtc int
{
	if((gStreamEnable.temp)&&(!gDataReady.temp))	
	{
		DataStreamUpdateTimestamp(&tempDataStream);
		gDataReady.temp = TRUE; 	// Indicates fifo should sample the values
		DATA_STREAM_SW_INT_IF = 1; 	// SW trigger fifo int tasks
	}
	if (++inactivity_timer > INACTIVITY_THRESHOLD) inactivity_timer = INACTIVITY_THRESHOLD;// Inactivity timer increment and ceiling
}
void __attribute__((interrupt,auto_psv)) _INT1Interrupt(void)
{
	// This is only for the fifo int
	IFS1bits.INT1IF = 0; 
	if (gDataReady.accel != TRUE) 	// Data not already ready
	{
		gDataReady.accel = TRUE;	// Indicate data ready
		DataStreamUpdateTimestamp(&accelDataStream); // Not actually valid until read in sw triggered int
	}
	DATA_STREAM_SW_INT_IF = 1; 		// SW trigger fifo int tasks 
}
void __attribute__((interrupt,auto_psv)) _INT2Interrupt(void)
{
	// This is for other accel ints
	IFS1bits.INT2IF = 0;
	gDataReady.accel_int2 = TRUE;	// Indicate data ready
	DATA_STREAM_SW_INT_IF = 1; 		// SW trigger fifo int tasks 
}

// Interrupts for all device->fifo comms (i2c) at GLOBAL_I2C_PRIORITY
void __attribute__((interrupt,auto_psv)) _INT4Interrupt(void)
{
	dataStreamFlags_t dataReady;

	// Find out which devices have data
	asm("#DISI 0x3FFF");
	DATA_STREAM_SW_INT_IF = 0;	// Clear the flag in the DISI
	dataReady = gDataReady; 	// Grab a copy of the global var
	gDataReady.all = 0;			// Clear the global var
	asm("#DISI 0x0000");

	// Process all the device to fifo tasks
	if (dataReady.accel) 
	{
		// Empty hardware FIFO upto ACCEL_FIFO_WATERMARK bytes to keep time stamp valid from last interrupt
		unsigned short remain = ACCEL_FIFO_WATERMARK;
		unsigned char passes;
        for (passes = 2; passes != 0; --passes)
        {
            unsigned short contiguous, num;
            void *buffer;
            // See how much contiguous free space we have in the buffer
            contiguous = FifoContiguousSpaces(&accelDataStream.fifo, &buffer);
            // If we aren't able to fit *any* in, we've over-run our software buffer
            if (contiguous == 0)
            {
                status.events |= DATA_EVENT_BUFFER_OVERFLOW;    // Flag a software FIFO over-run error
                AccelReadFIFO(NULL, ACCEL_MAX_FIFO_SAMPLES);    // Dump hardware FIFO contents to prevent continuous watermark/over-run interrupts
                break;
            }
            // Reads the ADXL hardware FIFO (bytes = ADXL_BYTES_PER_SAMPLE * entries)
			if (contiguous > remain) contiguous = remain;
			if (gStreamEnable.accel) 
			{ 
				num = AccelReadFIFO((accel_t *)buffer, contiguous);
				FifoExternallyAdded(&accelDataStream.fifo, num);
				LED_B=1;
			}
			else					 
			{ 
				num = AccelReadFIFO(NULL, 32);break; // Just clear the fifo 
			}
            // ??? There were no more entries to read - hardware fault, possibly wrong interrrupt triggered the fifo int
            if (num == 0) { break; }
			remain -= num;
            // Inform the buffer we've directly added some data
            
			// If no more data to read out
			if (remain == 0) { break; }
        }
	}

	if (dataReady.temp)
	{
		static temp_t temp;
		static unsigned char prescaler = 0;
		// Get the last samples
		temp.probe = MCP3421CalcTemperatureFractional(MCP3421Read());
		temp.board = MCP9800Read();	
		// Push into fifo
		FifoPush(&tempDataStream.fifo, &temp, 1);
		// Initiate the next conversions
		MCP3421Sample();
		// Prescaled MCP9800 for lower power - once every 10secs
		if (++prescaler>=10){prescaler=0;MCP9800Sample();}
	//LED_G=1;
	} 

	if (dataReady.adc)
	{
		static adcVals_t temporary;
		// Sample ADC
		AdcInit(); 
		AdcSampleWait();
		AdcOff(); //1.1ms
		// Pack
		temporary.batt = adcResult.batt;
		temporary.light = adcResult.ldr;
		// Push into fifo
		FifoPush(&adcDataStream.fifo, &temporary, 1);
	//LED_R=1;
	} 

	// Check for accel interrupts
	if ((dataReady.accel_int2)||(PORTDbits.RD0))
	{
		// Accel int2 indicates other accel event triggers
		unsigned char reason = AccelReadIntSource();
		if (reason & ACCEL_INT_SOURCE_TRANSIENT)  // Activity detected
		{			
			LED_R = 1;						// If accelerometer is off - turn it on
			AccelReadTransientSource();
			inactivity_timer = 0;
		}
	}
	
	// All data sources are processed, this interrupt will re-enter if new sources have become available
}

char LoggerInit(const char *filename)
{
	// Open the file for appendfail
	char retval = 0;

    // Initialize variables
    status.sampleCount = 0;
    status.lastWrittenTicks = 0;
    status.events = DATA_EVENT_RESUME;
    status.accelSequenceId = 0;
    status.gyroSequenceId = 0;
    status.debugFlashCount = 3;     // Initially flash logging status

    // Open the log file for append
    logFile = FSfopen(filename, "a");
    if (logFile != NULL)
    {
            // Calculate number of sectors in file
            FSfseek(logFile, 0, SEEK_END);                              // Ensure seeked to end (should be in append mode)

            // Ensure archive attribute set to indicate file updated
            logFile->attributes |= ATTR_ARCHIVE;
            
            // Initially flash to show started
            status.debugFlashCount = 5;
			
			retval = 1;
    }
	else    
	{
		// Failed
    	return 0;
	}

	// Populate packing etc.
	// Accellerometer
    accelFileStream.fileHandle = logFile;          	// Handle to the file stream -- either FILE* or FSFILE*
	accelFileStream.streamId = 'a';            			// [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
	accelFileStream.sampleRate = 100;          			// [2] Sample rate (Hz)
	accelFileStream.sampleRateModifier = 1;  			// [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
	accelFileStream.dataType = FILESTREAM_DATATYPE_ACCEL; // [1] Data type - see FileStream.h
	accelFileStream.dataConversion = -12;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
	accelFileStream.channelPacking = (FILESTREAM_PACKING_3_CHANNEL|FILESTREAM_PACKING_SINT16);      		// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)

	// Temperature sensors
    tempFileStream.fileHandle = logFile;          	// Handle to the file stream -- either FILE* or FSFILE*
	tempFileStream.streamId = 't';            			// [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
	tempFileStream.sampleRate = 1;          			// [2] Sample rate (Hz)
	tempFileStream.sampleRateModifier = 1;  			// [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
	tempFileStream.dataType = FILESTREAM_DATATYPE_TEMPERATURE; // [1] Data type - see FileStream.h
	tempFileStream.dataConversion = -8;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
	tempFileStream.channelPacking = (FILESTREAM_PACKING_2_CHANNEL|FILESTREAM_PACKING_SINT16);      			// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)

	// ADC
    adcFileStream.fileHandle = logFile;          	// Handle to the file stream -- either FILE* or FSFILE*
	adcFileStream.streamId = 'l';            			// [1] Stream identifier (by convention: a-ccel, g-yro, m-agnetometer, p-ressure, other-ASCII, non-alphanumeric=reserved, *=all streams)
	adcFileStream.sampleRate = 4;          			// [2] Sample rate (Hz)
	adcFileStream.sampleRateModifier = 1;  			// [1] Sample rate modifier (1 = none; >1 = divisor: sample rate / value; <-1 = multiplier: -value * sample rate; 0 = period in seconds: sample rate / 60; -1 = period in minutes: sample rate / 3600)
	adcFileStream.dataType = FILESTREAM_DATATYPE_BATTERY_LIGHT; // [1] Data type - see FileStream.h
	adcFileStream.dataConversion = 0;      			// [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))
	adcFileStream.channelPacking = (FILESTREAM_PACKING_2_CHANNEL|FILESTREAM_PACKING_SINT16);      			// [1] Packing type (0x32 = 3-channel 2-bytes-per-sample (16-bit); 0x12 = single-channel 16-bit; 0x30 = DWORD packing of 3-axis 10-bit 0-3 binary shifted)


	// Setup the streams
	FileStreamInit(&accelFileStream, scratchBuffer);
	FileStreamInit(&tempFileStream, scratchBuffer);
	FileStreamInit(&adcFileStream, scratchBuffer);
	DataStreamInit(&accelDataStream, sizeof(accel_t), (sizeof(accelBuffer)/sizeof(accel_t)), accelBuffer);
	DataStreamInit(&tempDataStream, sizeof(temp_t), (sizeof(tempBuffer)/sizeof(temp_t)), tempBuffer);
	DataStreamInit(&adcDataStream, sizeof(adcVals_t), (sizeof(adcBuffer)/sizeof(adcVals_t)), adcBuffer);

	return 1; // Success
}

void LoggerStart(void)
{
	// Clear data ready flags
	gDataReady.accel = FALSE;
	gDataReady.temp = FALSE;
	gDataReady.accel = FALSE;

	// Start the streams
	DATA_STREAM_SW_INT_IP = GLOBAL_I2C_PROCESSOR_PRIORITY;
	DATA_STREAM_SW_INT_IF = 0;
	DATA_STREAM_SW_INT_IE = 1;

	gStreamEnable.temp = TRUE;
	gStreamEnable.adc = TRUE;
	gStreamEnable.accel = TRUE;
	AccelStartup(ACCEL_RANGE_8G|ACCEL_RATE_100);
	AccelEnableInterrupts(	ACCEL_INT_SOURCE_TRANSIENT | ACCEL_INT_SOURCE_TAP | ACCEL_INT_SOURCE_WATERMARK, ACCEL_INT_SOURCE_WATERMARK ); // Last field diverts source to int1

	// The fifos will start filling up now... they will need emptying 
}

// Check each stream for data and write it out to the log file
// 0 = no write, 1 = data written, -1 = write error
char LoggerPollAndWrite(dataStreamFlags_t forceWrite)
{
	unsigned int retval = 0;
	static unsigned long timestamp;			// Time,factional and length must be set by the internal read functions
	static unsigned short timeFractional;	// Fractional part of time
	static unsigned int fifoLength; 		// Current time stamp is for the value at the end of the fifo
	unsigned int writeLength;
	unsigned int popped;
	static void* temp;
	// Now check if any of the data streams has enough data to make up a sector

	DataStreamTimestamp(&accelDataStream, &timestamp, &timeFractional, &fifoLength); // Get stream vars
	while((fifoLength >= ACCEL_SECTOR_SAMPLE_COUNT) || (forceWrite.accel))// Copy out all available full sectors
	{
		retval = 1;
		writeLength = fifoLength;					// Change number to write for next sector
		if (writeLength > ACCEL_SECTOR_SAMPLE_COUNT) 
			{writeLength = ACCEL_SECTOR_SAMPLE_COUNT;}// set write length for full sector
		if (writeLength == 0) break;

		temp = FileStreamPrepareData(&accelFileStream, timestamp, timeFractional, fifoLength, writeLength);
		popped = FifoPop(&accelDataStream.fifo, temp, writeLength);

		if (popped != writeLength) 							{return -1;} /*Fifo pop FAIL*/
		if(!FileStreamOutputData(&accelFileStream, 0 , 0))	{return -1;} /*FS write FAIL*/

		fifoLength-=writeLength;
		// Subtract from fifo length and loops back
	}

	DataStreamTimestamp(&tempDataStream, &timestamp, &timeFractional, &fifoLength); // Get stream vars
	while((fifoLength >= TEMP_SECTOR_SAMPLE_COUNT) || (forceWrite.temp))// Copy out all available full sectors
	{
		retval = 1;
		writeLength = fifoLength;					// Change number to write for next sector
		if (writeLength > TEMP_SECTOR_SAMPLE_COUNT) 
			{writeLength = TEMP_SECTOR_SAMPLE_COUNT;}// set write length for full sector
		if (writeLength == 0) break;

		temp = FileStreamPrepareData(&tempFileStream, timestamp, timeFractional, fifoLength, writeLength);
		popped = FifoPop(&tempDataStream.fifo, temp, writeLength);

		if (popped != writeLength) 							{return -1;} /*Fifo pop FAIL*/
		if(!FileStreamOutputData(&tempFileStream, 0 , 0))	{return -1;} /*FS write FAIL*/

		fifoLength-=writeLength;
		// Subtract from fifo length and loops back
	}

	DataStreamTimestamp(&adcDataStream, &timestamp, &timeFractional, &fifoLength); // Get stream vars
	while((fifoLength >= ADC_SECTOR_SAMPLE_COUNT) || (forceWrite.adc))// Copy out all available full sectors
	{
		retval = 1;
		writeLength = fifoLength;					// Change number to write for next sector
		if (writeLength > ADC_SECTOR_SAMPLE_COUNT) 
			{writeLength = ADC_SECTOR_SAMPLE_COUNT;}// set write length for full sector
		if (writeLength == 0) break;

		temp = FileStreamPrepareData(&adcFileStream, timestamp, timeFractional, fifoLength, writeLength);
		popped = FifoPop(&adcDataStream.fifo, temp, writeLength);

		if (popped != writeLength) 							{return -1;} /*Fifo pop FAIL*/
		if(!FileStreamOutputData(&adcFileStream, 0 , 0))	{return -1;} /*FS write FAIL*/

		fifoLength-=writeLength;
		// Subtract from fifo length and loops back
	}

	return retval;
	// Thats it..
}

void LoggerStop(void)
{
	// Stop all the interrupts
	ACCEL_INT1_IE = 0;
	gStreamEnable.temp = FALSE;
	gStreamEnable.adc = FALSE;
	gStreamEnable.accel = FALSE;
	// Turn off the peripherals
	AccelStandby();
	MCP9800Off();
	MCP3421Off();

	// Force write remaining data in buffers
	LoggerPollAndWrite((dataStreamFlags_t)((unsigned char)0xFF)); // all streams forced

	// Finalise the file streams
	if (logFile != NULL)
	{   // Close the file
        FSfclose(logFile);
        logFile = NULL;
    }
	FtlShutdown();

	return;
}

// Clear the data buffers
void LoggerClear(void)
{
	DataStreamClear(&accelDataStream);
	DataStreamClear(&tempDataStream);
	DataStreamClear(&adcDataStream);
}

// Read metadata settings from a binary file
char LoggerReadMetadata(const char *filename)
{
	char retval = 0;
	return retval;
}
// Write metadata settings to a binary file
char LoggerWriteMetadata(const char *filename)
{
	char retval = 0;
	return retval;
}


// Stop logging conditions (values seen in log entries)
typedef enum
{ 
	STOP_NONE                   = 0,    // (not seen)
	STOP_INTERVAL               = 1,    // Blue 0 ..
	STOP_SAMPLE_LIMIT           = 2,    // Blue 1 -.-.
	STOP_DISK_FULL              = 2,    // Blue 1 -.-. (same as sample limit)
	NOT_STARTED_NO_INTERVAL     = 3,    // Blue 2 --.--.
	NOT_STARTED_AFTER_INTERVAL  = 4,    // Blue 3 ---.---.
	NOT_STARTED_SAMPLE_LIMIT    = 5,    // Blue 4 ----.----.
	NOT_STARTED_DISK_FULL       = 5,    // Blue 4 ----.----. (same as sample limit)
	NOT_STARTED_WAIT_USB        = 6,    // (not seen)
	STOP_USB                    = 7,    // (not seen)
	NOT_STARTED_INITIAL_BATTERY = 8,    // Red 0 ..
	NOT_STARTED_WAIT_BATTERY    = 9,    // Red 1 -.-.
	STOP_BATTERY                = 10,   // Red 2 --.--.
	NOT_STARTED_FILE_OPEN       = 11,   // Red 3 ---.---.
	STOP_LOGGING_WRITE_ERR      = 12,   // Red 4 ----.----. (not seen, restarted)
	STOP_LOGGING_SAMPLE_ERR     = 13,   // Red 5 -----.-----. (not seen, restarted)
} StopCondition;

// Log entry strings
const char *stopConditionString[] =
{
	"STOP_NONE",                   // 0
	"STOP_INTERVAL",               // 1
	"STOP_SAMPLE_LIMIT",           // 2 (or STOP_DISK_FULL)
	"NOT_STARTED_NO_INTERVAL",     // 3
	"NOT_STARTED_AFTER_INTERVAL",  // 4
	"NOT_STARTED_SAMPLE_LIMIT",    // 5 (or NOT_STARTED_DISK_FULL)
	"NOT_STARTED_WAIT_USB",        // 6
	"STOP_USB",                    // 7
	"NOT_STARTED_INITIAL_BATTERY", // 8
	"NOT_STARTED_WAIT_BATTERY",    // 9
	"STOP_BATTERY",                // 10
	"NOT_STARTED_FILE_OPEN",       // 11
	"STOP_LOGGING_WRITE_ERR",      // 12
	"STOP_LOGGING_SAMPLE_ERR",     // 13
};

// Flash codes
const char stopFlashCode[] =
{
//     0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13
//   off,  B:0,  B:1,  B:2,  B:3,  B:4,  B:5,  B:6,  R:0,  R:1,  R:2,  R:3,  R:4,  R:5
    0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45
};


// Logging mode
void RunLogging(void)
{
    const char *filename = DEFAULT_FILE;
    StopCondition stopCondition = 0;

	LED_SET(LED_OFF);
    AdcInit();
    AdcSampleWait();

    SettingsIncrementLogValue(LOG_VALUE_RESTART);   // Increment restart counter

    if (adcResult.batt < BATT_CHARGE_MIN_SAFE)
    {
        // Battery level insufficient to start logging -- error
        stopCondition = NOT_STARTED_INITIAL_BATTERY;
    }
    else if (settings.loggingStartTime >= settings.loggingEndTime)
    {
        // We have a no defined logging interval (not even 'infinite') -- don't log
        stopCondition = NOT_STARTED_NO_INTERVAL;   // not an error
    }
    else if (settings.loggingEndTime > settings.loggingStartTime && RtcNow() >= settings.loggingEndTime)
    {
        // We have a set finish time and we are after that time -- don't log
        stopCondition = NOT_STARTED_AFTER_INTERVAL;   // not an error
    }
    else
    {
        // If we have a delayed start and we are before that time -- wait
        if (settings.loggingStartTime < settings.loggingEndTime && RtcNow() < settings.loggingStartTime)
        {
            // Strobes LED every few seconds until after start time or USB-connected
            status.debugFlashCount = 5;
            while (restart != 1 && !stopCondition)
            {
                unsigned int i;

                // Reset SW-WDT
                RtcSwwdtReset();

                // Don't always perform the battery amd RTC checks
                for (i = 0; i < 5; i++)
                {
                    // Exit condition: USB connection
                    if (USB_BUS_SENSE) { stopCondition = NOT_STARTED_WAIT_USB; restart = 1; break; }

                    if (settings.debuggingInfo >= 1 || status.debugFlashCount > 0) { LED_SET(LED_YELLOW); if (status.debugFlashCount > 0) status.debugFlashCount--; Delay10us(5); }
                    LED_SET(LED_OFF);

                    SystemPwrSave(WAKE_ON_WDT|WAKE_ON_USB|DONT_RESTORE_PERIPHERALS|ADC_POWER_DOWN|LOWER_PWR_SLOWER_WAKE|SAVE_INT_STATUS|ALLOW_VECTOR_ON_WAKE);
                }
                if (stopCondition) { break; }

                AdcInit();
                AdcSampleNow();

                // Exit condition: Battery too low
                if (adcResult.batt < BATT_CHARGE_MIN_SAFE) { stopCondition = NOT_STARTED_WAIT_BATTERY; break; }

                // Exit condition: Start logging
                if (RtcNow() >= settings.loggingStartTime) { break; }
            }
            LED_SET(LED_OFF);
        }

        // If battery not too low and USB not connected, start logging (we are after any delayed start time)
        if (restart != 1 && !stopCondition)
        {
            if (FSDiskFree() == 0)
            {
	            stopCondition = NOT_STARTED_DISK_FULL;
            }
            else if (!LoggerInit(filename))
            {
	            stopCondition = NOT_STARTED_FILE_OPEN;
            }
            else
            {
                // If written required number of samples, stop logging
                if (settings.maximumSamples != 0 && status.sampleCount >= settings.maximumSamples)
                { 
                	stopCondition = NOT_STARTED_SAMPLE_LIMIT; 		// not an error
                }
                else
                {
					// Begins filling fifos etc. Call LoggerTasks() regularly
					LoggerStart();




	                // Logging loop
	                while (restart != 1 && !stopCondition)
	                {
	                    unsigned short now = RtcSeconds(); //Seconds
                        short result;

	                    // Exit condition: USB connection
	                    if (USB_BUS_SENSE) { stopCondition = STOP_USB; restart = 1; break; }
	
						// LoggingTasks
						result = LoggerPollAndWrite((dataStreamFlags_t)((unsigned char)0));

	                    if (result == 1) // 0 = no write or -1 = write fail
	                    {
                            // We successfully sampled
                            status.lastSampledTicks = now;
							status.lastWrittenTicks = now;

                            // Reset SW-WDT
                            RtcSwwdtReset();
	
	                        // Check if battery too low...
	                        if (adcResult.batt < BATT_CHARGE_MIN_SAFE)
	                        {
	                            // Get a second opinion to be certain and, if so, stop logging
	                            AdcInit(); AdcSampleNow();AdcOff();
	                            if (adcResult.batt < BATT_CHARGE_MIN_SAFE) { stopCondition = STOP_BATTERY; break; }
	                        }
	
	                        // If after logging end time, stop logging
	                        if (RtcNow() > settings.loggingEndTime) { stopCondition = STOP_INTERVAL; break; }
	                    }
	
	                    // Status monitor
	                    if (status.lastSampledTicks == 0x0000) { status.lastSampledTicks = now; }
	                    if (status.lastWrittenTicks == 0x0000) { status.lastWrittenTicks = now; }
	
                        if (now - status.lastWrittenTicks > 600)
                        {
                            // If not been able to successfully write for 30 seconds, there may be a Filesystem/Ftl/NAND problem, stop logging
                            // If we detected a write problem and the disk is full, this is a valid stop reason, otherwise it's a write error
                             if (FSDiskFree() == 0)
                             {
         	                    stopCondition = STOP_DISK_FULL;
                             }
                             else
                             {
                                 stopCondition = STOP_LOGGING_WRITE_ERR;
                                 restart = 1;
                             }
							 break;
                        }
                        
						if (now - status.lastSampledTicks > 300)
                        {
                            // If not been sampled enough for a sector for 15 seconds, there's an ADXL problem, log the error and restart
                            stopCondition = STOP_LOGGING_SAMPLE_ERR;
                            restart = 1;
                            break;
                        }

						if ((inactivity_timer >= INACTIVITY_THRESHOLD)&&(gStreamEnable.accel==TRUE))
						{
							dataStreamFlags_t doForce;
							// Close existing streams not needed in sleep
							gStreamEnable.accel = FALSE;
							// Empty fifo for closed streams
							doForce.all = ~gStreamEnable.all;
							LoggerPollAndWrite(doForce);
							// Increament the fileStream.sequenceId to alert parser
							FileStreamIncrementId(&accelFileStream);
						}
						if ((inactivity_timer < INACTIVITY_THRESHOLD)&&(gStreamEnable.accel==FALSE)) // New activity detected
						{
							// Restart streams
							DataStreamClear(&accelDataStream);
							// Restart the stream
							gStreamEnable.accel = TRUE;
						}
	
	                    // Sleep until ADXL INT1, RTC, USB or WDT
	                    SystemPwrSave(WAKE_ON_RTC|WAKE_ON_WDT|WAKE_ON_USB|WAKE_ON_ADXL1|WAKE_ON_TIMER1|ADC_POWER_DOWN|LOWER_PWR_SLOWER_WAKE|SAVE_INT_STATUS|ALLOW_VECTOR_ON_WAKE);

	                }




				}
	
                LoggerStop();
            }
        }
    }

    // Blue while powering down (lowest power)
    LED_SET(LED_BLUE);

    // Shutdown rtc
    RtcInterruptOff();

    // Add log entry for stop reason
    SettingsAddLogEntry(LOG_CATEGORY_STOP | (unsigned int)stopCondition, RtcNow(), stopConditionString[stopCondition]);

    // Turn off LEDs
    LED_SET(LED_OFF);

    // Sleep if not connected (and not restarting)
    if (restart != 1)
    {
        unsigned char led, countReset, countdown;
        led = (stopFlashCode[stopCondition] >> 4);
        countReset = stopFlashCode[stopCondition] & 0x0f;
        countdown = countReset;
        CLOCK_INTOSC(); // Lower power
        
        // Strobes LED every few seconds until connected
        while (restart != 1)
        {
            if (USB_BUS_SENSE) { restart = 1; break; }

            // Reset SW-WDT
            RtcSwwdtReset();

            SystemPwrSave(WAKE_ON_RTC|LOWER_PWR_SLOWER_WAKE|WAKE_ON_USB|ADC_POWER_DOWN|ACCEL_POWER_DOWN|GYRO_POWER_DOWN);
            
            if (led)
            {
                LED_SET(led);
                if (countdown > 0)
                {
                	Delay10us(15);
                	countdown--;
                }
                else
                {
                    Delay10us(3);
                    countdown = countReset;
                }
                LED_SET(LED_OFF);
            }
        }
    }

    return;
}

//EOF

