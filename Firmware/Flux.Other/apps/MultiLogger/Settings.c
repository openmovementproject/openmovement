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

// Configuration settings, status and command handling
// Dan Jackson, 2011-2012

#define CRC_COMMANDS		// Enable code to protect commands with CRCs // TODO: Just make this the only case


// Includes
#include "HardwareProfile.h"
#include <Compiler.h>
#include <TimeDelay.h>
#include "Analog.h"
#include "Peripherals/Rtc.h"
//#include "USB/USB.h"
//#include "USB/usb_function_msd.h"
//#include "USB/usb_function_cdc.h"
#include "Ftl/FsFtl.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/bmp085.h"
#include "Peripherals/Mag.h"
#include "Peripherals/Prox.h"
#include "Peripherals/Rtc.h"
#include "Usb/USB_CDC_MSD.h"
#include "Utils/Util.h"
#include "Utils/Fifo.h"
#include "Utils/FSUtils.h"
#include "Apps/MultiLogger/Settings.h"
#include "Utils/DataStream.h"
#include "Utils/FileStream.h"
#include "Apps/MultiLogger/Recorder.h"
#include "Apps/MultiLogger/Logger.h"
#include "Utils/Command.h"
#include "Bluetooth/Bluetooth.h"
#include "Apps/MultiLogger/Sampler.h"

//#define RAW_NAND_DEBUG
#ifdef RAW_NAND_DEBUG
#include "Peripherals/Nand.h"
#endif

#define LOG_FILE "LOG.TXT"


// These addresses must match the reserved sections of program memory in the linker script
// TODO: These are device-specific -- move out of apps to project folder
#define DEVICE_ID_ADDRESS 0x2A000ul
//ROM BYTE __attribute__ ((address(DEVICE_ID_ADDRESS),space(prog),aligned(ERASE_BLOCK_SIZE),section(".device_id"),noload)) DeviceIdData[ERASE_BLOCK_SIZE];

#define LOG_ADDRESS  0x29C00ul
//ROM BYTE __attribute__ ((address(LOG_ADDRESS),space(prog),aligned(ERASE_BLOCK_SIZE),section(".log" ),noload)) LogData[ERASE_BLOCK_SIZE];

#define SETTINGS_ADDRESS  0x29800ul
//ROM BYTE __attribute__ ((address(SETTINGS_ADDRESS),space(prog),aligned(ERASE_BLOCK_SIZE),section(".settings" ),noload)) SettingsData[ERASE_BLOCK_SIZE];


extern unsigned char scratchBuffer[512];
Settings settings = {0};
Status status = {0};


// Get the device id
unsigned short SettingsGetDeviceId(void)
{
    unsigned short id = 0xffff;
    ReadProgram(DEVICE_ID_ADDRESS, &id, sizeof(unsigned short));
//KL for debugging purposes the device always appears as id = 555
#ifdef __DEBUG
	if (id == 0xffff) id = 555;
#endif
    return id;
}


// Rewrite the device id
void SettingsSetDeviceId(unsigned short newId)
{
    // Fetch current id
    settings.deviceId = SettingsGetDeviceId();
    // If different, rewrite
    if (newId != settings.deviceId)
    {
        settings.deviceId = newId;
		EraseProgramBlock(DEVICE_ID_ADDRESS);
        WriteProgramPage_actual(DEVICE_ID_ADDRESS, &settings.deviceId, sizeof(unsigned short));
    }
}


// Get a log value
unsigned short SettingsGetLogValue(unsigned int index)
{
    unsigned short value = 0x0000;
    ReadProgram(LOG_ADDRESS + index * sizeof(unsigned short), &value, sizeof(unsigned short));
    if (value == 0xffff) { value = 0x0000; }      // If uninitialized
    return value;
}


// Update a log value
unsigned short SettingsIncrementLogValue(unsigned int index)
{
    unsigned short value;

    // Read existing data to RAM
    ReadProgram(LOG_ADDRESS, scratchBuffer, 512);

    // Update value
    value = ((unsigned short *)scratchBuffer)[index];   // Retrieve
    if (value == 0xffff) { value = 0x0000; }            // If uninitialized
    value++;                                            // Increment
    ((unsigned short *)scratchBuffer)[index] = value;   // Store

    // Rewrite program memory from RAM
	EraseProgramBlock(LOG_ADDRESS);
    WriteProgramPage(LOG_ADDRESS, scratchBuffer, 512);

    return value;
}


// Add a log entry
void SettingsAddLogEntry(unsigned short status, unsigned long timestamp, const char *message)
{
    int len, i;

    // Read existing data to RAM
    ReadProgram(LOG_ADDRESS, scratchBuffer, 512);

    // Move existing log entries down one row
    memmove(scratchBuffer + 2 * LOG_SIZE, scratchBuffer + 1 * LOG_SIZE, (LOG_COUNT - 1) * LOG_SIZE);
    *(unsigned short *)(scratchBuffer + 1 * LOG_SIZE) = status;
    *(unsigned long *)(scratchBuffer + 1 * LOG_SIZE + sizeof(unsigned short)) = timestamp;

    len = strlen(message);
    if (len > LOG_SIZE - 7) { len = LOG_SIZE - 7; }
    for (i = 0; i < LOG_SIZE - 6; i++)
    {
        char *p = (char *)(scratchBuffer + 1 * LOG_SIZE + sizeof(unsigned short) + sizeof(unsigned long));
        if (i > len) { p[i] = '\0'; }
        else { p[i] = message[i]; }
    }

    // Rewrite program memory from RAM
	EraseProgramBlock(LOG_ADDRESS);
    WriteProgramPage(LOG_ADDRESS, scratchBuffer, 512);

    return;
}


// Retrieve a log entry
const char *SettingsGetLogEntry(int index, unsigned short *status, unsigned long *timestamp)
{
    if (index < 0 || index >= LOG_COUNT) { return NULL; }

    // Read existing data to RAM
    ReadProgram(LOG_ADDRESS, scratchBuffer, 512);

    if (status != NULL) *status = *(unsigned short *)(scratchBuffer + (1 + index) * LOG_SIZE);
    if (timestamp != NULL) *timestamp = *(unsigned long *)(scratchBuffer + (1 + index) * LOG_SIZE + sizeof(unsigned short));
    *(scratchBuffer + (1 + index) * LOG_SIZE + sizeof(unsigned short) + sizeof(unsigned long) + LOG_SIZE - 1) = '\0';	// Ensure entry is null-terminated
    return (const char *)(scratchBuffer + (1 + index) * LOG_SIZE + sizeof(unsigned short) + sizeof(unsigned long));
}


char SettingsGetConfig(size_t offset, void *buffer, size_t len)
{
	// Requested read must be within the first 256 bytes (the second 256 are used to confirm the data)
	if (buffer == NULL || offset < 0 || offset >= 256 || len < 0 || len >= 256 || offset + len >= 256) { return 0; }
	if (len == 0) { return 1; }
	
	// Read data to buffer
    ReadProgram(SETTINGS_ADDRESS + offset, buffer, len);

	// Confirm data    
	{
		size_t i;
		
	    // Read confirmation data to second half of scratch buffer (first half is free to be used as the buffer)
	    ReadProgram(SETTINGS_ADDRESS + 256 + offset, scratchBuffer + 256, len);
	    
	    // Check data matches
		for (i = 0; i < len; i++)
		{
			char c1 = ((const unsigned char *)buffer)[i];
			char c2 = ((const unsigned char *)(scratchBuffer + 256))[i];
			if (c1 != ~c2) { return 0; }
		}
		return 1;
	}    
}

char SettingsSetConfig(size_t offset, const void *buffer, size_t len)
{
	int i;
	
	// Requested read must be within the first 256 bytes (the second 256 are used to confirm the data)
	if (buffer == NULL || offset < 0 || offset >= 256 || len < 0 || len >= 256 || offset + len >= 256) { return 0; }
	if (len == 0) { return 1; }
	if (buffer == scratchBuffer) { return 0; }	// cannot use the scratch buffer to write to config
	
	// Get the current value to the scratch buffer
	if (SettingsGetConfig(offset, scratchBuffer, len))
	{
		char same = 1;	
		// If it was successfully read, see if it's the same as the value trying to be written
		for (i = 0; i < len; i++)
		{
			if (((const unsigned char *)buffer)[i] != ((const unsigned char *)(scratchBuffer))[i]) { same = 0; break; }
		}
		// If already set to the same data, we don't need to write anything
		if (same) { return 2; }
	}
	
	// The saved value is different (or couldn't be read) -- read existing configuration page to RAM
    ReadProgram(SETTINGS_ADDRESS, scratchBuffer, 512);

    // Update value and inverted check value
	for (i = 0; i < len; i++)
	{
		unsigned char c = ((unsigned char *)(buffer))[i];
		((unsigned char *)scratchBuffer)[offset + i] = c;
		((unsigned char *)scratchBuffer)[256 + offset + i] = ~c;
	}
    
    // Rewrite program memory from RAM
	EraseProgramBlock(SETTINGS_ADDRESS);
    if (WriteProgramPage(SETTINGS_ADDRESS, scratchBuffer, 512) != 0) { return 0; }

    // Re-read, checking for match
	if (!SettingsGetConfig(offset, scratchBuffer, len)) { return 0; }
	
	// See if it's the same as the value just written
	for (i = 0; i < len; i++)
	{
		if (((const unsigned char *)scratchBuffer)[i] != ((const unsigned char *)(buffer))[i]) { return 0; }
	}
	
	// Everything matches
	return 1;
}


// Default configuration
void DefaultConfiguration(void)
{
	// Recording setup
    settings.sessionId = 0x00000000;
    //settings.sessionId = (settings.deviceId != 0xffff) ? (10000L * settings.deviceId) : 0;
    //settings.sessionId += (SettingsGetLogValue(LOG_VALUE_RESTART) % 10000);
    //settings.sampleRate = ACCEL_DEFAULT_RATE;
#ifdef DEBUG_ALWAYS_RECORD
    settings.loggingStartTime = 0;
#else
    settings.loggingStartTime = 0xffffffff;
#endif
    settings.loggingEndTime = 0xffffffff;
    //settings.maximumSamples = 0;
    settings.debuggingInfo = DEFAULT_DEBUGGING;
    //settings.lastChangeTime = 0;
    //settings.timeZone = -1;
    
	// Clear annotation
    memset(settings.annotation, 0, ANNOTATION_SIZE * ANNOTATION_COUNT); // [64] 6x (was 14x) 32-byte annotation chunks
    
    // Sensor setup
    settings.sensorConfig[SENSOR_INDEX_ADC].enabled   = 1; settings.sensorConfig[SENSOR_INDEX_ADC].frequency   = 1;   settings.sensorConfig[SENSOR_INDEX_ADC].sensitivity = 0;     settings.sensorConfig[SENSOR_INDEX_ADC].options = 0;
    settings.sensorConfig[SENSOR_INDEX_ACCEL].enabled = 1; settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency = 100; settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity = 8;   settings.sensorConfig[SENSOR_INDEX_ACCEL].options = 0;
    settings.sensorConfig[SENSOR_INDEX_GYRO].enabled  = 1; settings.sensorConfig[SENSOR_INDEX_GYRO].frequency  = 100; settings.sensorConfig[SENSOR_INDEX_GYRO].sensitivity = 2000; settings.sensorConfig[SENSOR_INDEX_GYRO].options = 0;
    settings.sensorConfig[SENSOR_INDEX_MAG].enabled   = 1; settings.sensorConfig[SENSOR_INDEX_MAG].frequency   = 10;  settings.sensorConfig[SENSOR_INDEX_MAG].sensitivity = 0;     settings.sensorConfig[SENSOR_INDEX_MAG].options = 0;
    settings.sensorConfig[SENSOR_INDEX_ALT].enabled   = 1; settings.sensorConfig[SENSOR_INDEX_ALT].frequency   = 1;   settings.sensorConfig[SENSOR_INDEX_ALT].sensitivity = 0;     settings.sensorConfig[SENSOR_INDEX_ALT].options = 0;
#ifdef SYNCHRONOUS_SAMPLING
    settings.sensorConfig[SENSOR_INDEX_ALLAXIS].enabled = 0; settings.sensorConfig[SENSOR_INDEX_ALLAXIS].frequency = 100; settings.sensorConfig[SENSOR_INDEX_ALLAXIS].sensitivity = 0; settings.sensorConfig[SENSOR_INDEX_ALLAXIS].options = 0;
#endif
}

// Reset settings from ROM
void SettingsInitialize(void)
{
    // Clear all
    memset(&settings, 0, sizeof(settings));
    memset(&status, 0, sizeof(status));

    // Fixed settings
    settings.deviceId = 0x0000;
    settings.deviceId = SettingsGetDeviceId();

    // ACCEL
    settings.sensorConfig[SENSOR_INDEX_ACCEL].stream = 'a';
    settings.sensorConfig[SENSOR_INDEX_ACCEL].enabled = 0;
    settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency = 100;
    settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity = 8;
    settings.sensorConfig[SENSOR_INDEX_ACCEL].options = 0;
    // GYRO
    settings.sensorConfig[SENSOR_INDEX_GYRO].stream = 'g';
    settings.sensorConfig[SENSOR_INDEX_GYRO].enabled = 0;
    settings.sensorConfig[SENSOR_INDEX_GYRO].frequency = 100;
    settings.sensorConfig[SENSOR_INDEX_GYRO].sensitivity = 2000;
    settings.sensorConfig[SENSOR_INDEX_GYRO].options = 0;
    // MAG
    settings.sensorConfig[SENSOR_INDEX_MAG].stream = 'm';
    settings.sensorConfig[SENSOR_INDEX_MAG].enabled = 0;
    settings.sensorConfig[SENSOR_INDEX_MAG].frequency = 10;
    settings.sensorConfig[SENSOR_INDEX_MAG].sensitivity = 0;
    settings.sensorConfig[SENSOR_INDEX_MAG].options = 0;
    // ALT (pressure)
    settings.sensorConfig[SENSOR_INDEX_ALT].stream = 'p';
    settings.sensorConfig[SENSOR_INDEX_ALT].enabled = 0;
    settings.sensorConfig[SENSOR_INDEX_ALT].frequency = 1;
    settings.sensorConfig[SENSOR_INDEX_ALT].sensitivity = 0;
    settings.sensorConfig[SENSOR_INDEX_ALT].options = 0;
    // ADC
    settings.sensorConfig[SENSOR_INDEX_ADC].stream = 'l';
    settings.sensorConfig[SENSOR_INDEX_ADC].enabled = 0;
    settings.sensorConfig[SENSOR_INDEX_ADC].frequency = 1;
    settings.sensorConfig[SENSOR_INDEX_ADC].sensitivity = 0;
    settings.sensorConfig[SENSOR_INDEX_ADC].options = 0;
#ifdef SYNCHRONOUS_SAMPLING
    // ALLAXIS
    settings.sensorConfig[SENSOR_INDEX_ALLAXIS].stream = 'x';
    settings.sensorConfig[SENSOR_INDEX_ALLAXIS].enabled = 0;
    settings.sensorConfig[SENSOR_INDEX_ALLAXIS].frequency = 100;
    settings.sensorConfig[SENSOR_INDEX_ALLAXIS].sensitivity = 0;
    settings.sensorConfig[SENSOR_INDEX_ALLAXIS].options = 0;
#endif
    
    // Status
    //status.attached = 0;
    status.initialBattery = AdcBattToPercent(adcResult.batt);
    status.inactivity = 0;       // reset inactivity timer

    // System state
    status.connection = CONNECTION_NONE;
    status.battery = BATTERY_NORMAL;
#ifdef BLUETOOTH_DEFAULT_DISCOVERABLE
    status.discoverable = DISCOVERABLE_ALWAYS;      // TODO: Check the best default
#else
    status.discoverable = DISCOVERABLE_AUTO;      // TODO: Check the best default
#endif
    status.wireless = WIRELESS_UNKNOWN;
    status.drive = DRIVE_UNOWNED;

    // Status: Logging
    //status.accelSequenceId = 0;
    //status.gyroSequenceId = 0;
    //status.events = 0;
    //status.sampleCount = 0;
    //status.lastSampledTicks = 0;
    //status.lastWrittenTicks = 0;
    //status.debugFlashCount = 0;

    settings.streamFormat = STREAM_MODE_OMX_SLIP;      // 0 = None, 1 = WAX SLIP, (2=Text, 3=OSC, 4=HEX), 5 = OMX

    // Status: attached
    status.lockCode = 0x0000;
    status.batteryFullTimer = 0;
    status.ledOverride = -1;
    //status.diskMounted = 1;
    status.actionFlags = 0;
    status.actionCountdown = 0;
    //status.stream = 0;

    // Configuration
    status.lockCode = 0x0000;
    status.dataEcc = CONFIG_ECC_DEFAULT;
    // Set default filename (ensure null-padded buffer)
    memset(status.filespec, 0, sizeof(status.filespec)); strcpy(status.filespec, DATA_FILE);
    memset(status.folder, 0, sizeof(status.folder)); strcpy(status.folder, DATA_FOLDER);
    memset(status.calibration, 0, sizeof(status.calibration)); // zero calibration

    // "Initial lock" value
    {
        unsigned short code = 0xffff;
        if (CONFIG_GET(lockCode, &code) && code != 0 && code != 0xffff)
        {
            status.lockCode = code;
        }
    }

    // ECC
    {
        unsigned short value = 0xffff;
        if (CONFIG_GET(ecc, &value) && value != 0xffff) { status.dataEcc = (char)value; }
    } 

    // Discoverable
    {
        unsigned short value = 0xffff;
        if (CONFIG_GET(discoverable, &value) && value != 0xffff) { status.discoverable = (StateDiscoverable)value; }
    }

    // Filespec
    {
	    unsigned char newname[sizeof(status.filespec)] = {0};
        if (CONFIG_GET(filespec, newname)) { memcpy(status.filespec, newname, sizeof(status.filespec)); }
    }

    // Folder
    {
	    unsigned char newname[sizeof(status.folder)] = {0};
        if (CONFIG_GET(folder, newname)) { memcpy(status.folder, newname, sizeof(status.folder)); }
    }

    // Calibration
    {
        if (!CONFIG_GET(calibration, status.calibration)) { memset(status.calibration, 0, sizeof(status.calibration)); } // if none successfully read, zero calibration
    }
    
    status.disconnectTimer = 0;
    status.alertTimer = 0xffff;    
    
	DefaultConfiguration();

    
}


// Read settings from config file
char SettingsReadFile(const char *filename)
{
    FSFILE *fp;
    static char lineBuffer[64];
    char *line;

    // Read auto-exec config file
    fp = FSfopen(filename, "r");
    if (fp == NULL) { return 0; }
    for (;;)
    {
        line = FSfgets(lineBuffer, 64, fp);
        if (line == NULL) { break; }
        SettingsCommand(line, SETTINGS_BATCH);
    }
    FSfclose(fp);

    return 1;
}


char SettingsAction(char flags)
{
    char ret = 0;

    if (flags & ACTION_DELETE)
    {
        FSremove(DATA_FILE);
    }

    if (flags & ACTION_FORMAT_WIPE || flags & ACTION_FORMAT_QUICK)
    {
        char wipe = 0, error = 0;
        static char volumeBuffer[13];
        const char *hardwareName = HardwareName();
        volumeBuffer[0]  = hardwareName[0];
        volumeBuffer[1]  = hardwareName[1];
		volumeBuffer[2]  = hardwareName[2];
        volumeBuffer[3]  = '_';
        if (settings.deviceId == 0xffff)
        {
            volumeBuffer[4] = '_';
            volumeBuffer[5] = '_';
            volumeBuffer[6] = '_';
            volumeBuffer[7] = '_';
            volumeBuffer[8] = '_';
            volumeBuffer[9] = '\0';
        }
        else
        {
            volumeBuffer[4] = '0' + (settings.deviceId / 10000) % 10;
            volumeBuffer[5] = '0' + (settings.deviceId /  1000) % 10;
            volumeBuffer[6] = '0' + (settings.deviceId /   100) % 10;
            volumeBuffer[7] = '0' + (settings.deviceId /    10) % 10;
            volumeBuffer[8] = '0' + (settings.deviceId        ) % 10;
            volumeBuffer[9] = '\0';
        }

LED_SET(LED_WHITE);
        if (flags & ACTION_FORMAT_WIPE) { wipe = 1; }
        error |= FsFtlFormat(wipe, settings.deviceId, volumeBuffer);
LED_SET(LED_OFF);
        
        if (!StateSetDrive(DRIVE_OWNED, 1, RECORD_STOP_REASON_COMMAND)) { error |= 0x80; }

        if (error)
        { 
        	printf("ERROR: Problem during %sformat (0x%02x. 'status 3' for NAND health), failed:%s%s%s%s\r\n", wipe?"wipe/":"plain-", (unsigned char)error, (error&0x01?" MBR":""), (error&0x02?" FMT":""), (error&0x04?" VOL":""), (error&0x80?" MNT":"")); 
        }
        else
        {
	        printf("FORMAT: Complete.\r\n"); 
	    }
        LED_SET(LED_OFF);
    }

    if (flags & ACTION_CREATE)
    {
        unsigned long size = 0;
        int num;
        
        // Find current setting file
        num = LoggerFindCurrentFile(status.filespec, logger.filename, &size);
        // If none, or the current one has data, choose the next file number
        if (num < 0 || size > 1024) { num++; }
        // Use setting file name for the given number
        LoggerSetFilenameNumber(status.filespec, logger.filename, num);

        // Write metadata settings
        if (LoggerWriteMetadata(logger.filename))
        {
	        printf("COMMIT\r\n");
		} 
		else
		{
        	printf("ERROR: Problem during commit\n");
		}

    }
    else
    {
	    // Format without committing -> load default configuration to RAM
	    if (!(flags & ACTION_CREATE) && (flags & ACTION_FORMAT_WIPE || flags & ACTION_FORMAT_QUICK))
	    {
	    	DefaultConfiguration();
	    }
	}

    if (flags & ACTION_REMOUNT)
    {
        status.drive = DRIVE_MOUNTED;
    }

    if (flags & ACTION_RESTART)
    {
        ret = 1;
    }

    return ret;
}


char SettingsActionDelay(char flags, char countdown)
{
    status.actionCountdown = countdown;
    status.actionFlags = flags;
    
    // If no countdown, perform the action immediately
    if (countdown == 0)
    {
	    return SettingsAction(flags);
    }
    
	return 0;
}



char StateSetDrive(StateDrive newState, char force, unsigned short stopReason)
{
    // If we don't own a (non-full) disk
    if (newState != DRIVE_OWNED)
    {
        // Ensure recording is off
        StateSetRecording(RECORDING_OFF, stopReason);
    }

    // If we don't own the disk
    if (newState != DRIVE_OWNED && newState != DRIVE_OWNED_FULL)
    {
        // Ensure file reading is closed
        if (logger.readFile != NULL)
        {
            FSfclose(logger.readFile);
            logger.readFile = 0;
            logger.readLength = 0;
        }
    }

    // If the drive is in an error state, disallow everything except mounting or forced operations
    if (status.drive == DRIVE_ERROR && newState != DRIVE_MOUNTED && !force)
    {
        return 0;           // disallow
    }

    // If we're taking ownership
    if (newState == DRIVE_OWNED && (status.drive != newState || force))
    {
        // Initialize the filesystem
        if (FSInit())
        {
	        // Init. OK
			if (strlen(status.folder) > 0)
			{
				//FSchdir("\\");
				// Change to the data folder, make it if it doesn't exist
				if (FSchdir(status.folder) != 0)
				{
					FSmkdir(status.folder);
					FSchdir(status.folder);
				}
			}	
		}
        else
        {
            status.drive = DRIVE_ERROR;
            return 0;       // error on drive
        }
    }

    // Set the new state
    status.drive = newState;

    return 1;
}





// Command: #
int Command_Passthrough(commandParserState_t *cmd)
{
	if (cmd->argv[1] != NULL)
	{
		;
#ifdef USE_BLUETOOTH
		// If from primary, pass through to alternative
		if (cmd->flags & COMMAND_FLAG_OUTPUT_PRIMARY)
		{
			Bluetooth_write(cmd->argv[1], strlen(cmd->argv[1]));
			Bluetooth_write("\r\n", 2);
		}
		// If from alternative, pass through to primary
		if (USB_BUS_SENSE && (cmd->flags & COMMAND_FLAG_OUTPUT_ALTERNATIVE))
		{
			usb_write(cmd->argv[1], strlen(cmd->argv[1]));
			usb_write("\r\n", 2);
		}
#else
		;
#endif
	}	
    return COMMAND_RETURN_OK;
}

// Command: ID
int Command_ID(commandParserState_t *cmd)
{
    printf("ID=%s,%02x,%u,%u,%lu\r\n", HardwareName(), HARDWARE_VERSION, SOFTWARE_VERSION, settings.deviceId, settings.sessionId);
    return COMMAND_RETURN_OK;
}


// Command: DEVICE
int Command_DEVICE(commandParserState_t *cmd)
{
    if (cmd->argc > 1)																	// set parameter
    {
	    unsigned short id;
        if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow set while restricted
        id = (unsigned short)atoi(cmd->argv[1]); 
		SettingsSetDeviceId(id);
    }
    printf("DEVICE=%u,%u\r\n", settings.deviceId, SettingsGetDeviceId());
    return COMMAND_RETURN_OK;
}


// Command: TIME
int Command_TIME(commandParserState_t *cmd)
{
    unsigned long time;
    if (cmd->argc > 1)																	// set parameter
    {
        if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow set while restricted
        time = RtcFromString(cmd->argv[1]);
        if (time == 0) { return COMMAND_RETURN_PARAMETER; }
        RtcWrite(time);
    }
    else
    {
        time = RtcNow();
    }
    printf("$TIME=%s\r\n", RtcToString(time));
    return COMMAND_RETURN_OK;
}    

/*
// Command: TZONE
int Command_TZONE(commandParserState_t *cmd)
{
    if (cmd->argc > 1)																	// set parameter
    {
        if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow set while restricted
        if (cmd->argv[1][0] == '\0' ) { settings.timeZone = -1; }
        else { settings.timeZone = (short)atoi(cmd->argv[1]); }
    }
    if (settings.timeZone == -1) { printf("TZONE=\r\n"); }
    else { printf("TZONE=%d,%c%02d:%02u\r\n", settings.timeZone, settings.timeZone < 0 ? '-' : '+', abs(settings.timeZone) / 60, abs(settings.timeZone) % 60); }
    return COMMAND_RETURN_OK;
}    
*/


// Command: SAMPLE
int Command_SAMPLE(commandParserState_t *cmd)
{
	char z = 0;
	if (cmd->argc > 1 && cmd->argv[1][0] != '\0') 
	{ 
		z = cmd->argv[1][0]; 
		if (z >= 'a' && z <= 'z') { z = z + 'A' - 'a' ; }
	}


	if (!z || z == '1' || z == 'B') { printf("$BATT=%u,%u,mV,%d,%d\r\n", adcResult.batt, AdcBattToMillivolt(adcResult.batt), AdcBattToPercent(adcResult.batt), (status.battery == BATTERY_FULL ? 1 : 0)); }
#ifdef HAVE_PROXIMITY_SENSOR
	if (!z || z == '2' || z == 'L') { printf("$LIGHT=%u,%u\r\n", prox.light, 0); }
#else
	if (!z || z == '2' || z == 'L') { printf("$LIGHT=%u,%u\r\n", adcResult.ldr, AdcLdrToLux(adcResult.ldr)); }
#endif

    //	if (!z || z == '3' || z == 'T') { printf("$TEMP=%u,%d,0.1dC\r\n", adcResult.temp, AdcTempToTenthDegreeC(adcResult.temp)); }

if (writeHandler == Bluetooth_write) 	BluetoothSerialWait();
else if (writeHandler == usb_write) 	USBCDCWait();

	if (!z || z == '3' || z == 'T')
	{
        if (!gStreamEnable.alt)                 // Updated as part of alt.
        {
            DATA_STREAM_IPL_shadow_t IPLshadow;
            DATA_STREAM_INTS_DISABLE();
            BMP085_Initiate_ut_conversion();	// Begin a new sample
            DelayMs(20);						// Otherwise the following temperature measure will fail
            BMP085_read_ut();					// Read last sample
            sampler.lastTemperature = BMP085_calc_temperature(); // This is just the math bit
            DATA_STREAM_INTS_ENABLE();
        }
		printf("$TEMP=%ld\r\n", sampler.lastTemperature);
	}
    if (!z || z == '4')
	{
		unsigned short fractional;
        DateTime time;
        time = RtcNowFractional(&fractional);
        printf("$TIME=%s.%03d\r\n", RtcToString(time), RTC_FRACTIONAL_TO_MS(fractional));
	}

if (writeHandler == Bluetooth_write) 	BluetoothSerialWait();
else if (writeHandler == usb_write) 	USBCDCWait();

    // Don't do the updates if we're logging right now
	if (!z || z == '5' || z == 'A')
    {
        if (!gStreamEnable.accel)
        {
            DATA_STREAM_IPL_shadow_t IPLshadow;
            DATA_STREAM_INTS_DISABLE();
            AccelSingleSample(&sampler.lastAccel);
            DATA_STREAM_INTS_ENABLE();
        }
        printf("$ACCEL=%d,%d,%d\r\n", sampler.lastAccel.x, sampler.lastAccel.y, sampler.lastAccel.z);
    }
	if (!z || z == '6' || z == 'G')
    {
        if (!gStreamEnable.gyro)
        {
            DATA_STREAM_IPL_shadow_t IPLshadow;
            DATA_STREAM_INTS_DISABLE();
            GyroStartup();
            GyroReadFIFO(NULL, 32);
            GyroSingleSample(&sampler.lastGyro);
            GyroStandby();
            DATA_STREAM_INTS_ENABLE();
        }
        printf( "$GYRO=%d,%d,%d\r\n",  sampler.lastGyro.x,  sampler.lastGyro.y,  sampler.lastGyro.z);
    }
	if (!z || z == '7' || z == 'M')
    {
        if (!gStreamEnable.mag)
        {
            DATA_STREAM_IPL_shadow_t IPLshadow;
            DATA_STREAM_INTS_DISABLE();
            MagStartup(0); // Polled mode (12.5ms)
			MagSingleSample(NULL);
			DelayMs(15);
            MagSingleSample(&sampler.lastMag);
            MagStandby();
            DATA_STREAM_INTS_ENABLE();
        }
        printf(  "$MAG=%d,%d,%d\r\n",   sampler.lastMag.x,   sampler.lastMag.y,   sampler.lastMag.z);
    }

if (writeHandler == Bluetooth_write) 	BluetoothSerialWait();
else if (writeHandler == usb_write) 	USBCDCWait();

	if (!z || z == '8' || z == 'P')
	{
        if (!gStreamEnable.alt)
        {
            DATA_STREAM_IPL_shadow_t IPLshadow;
            DATA_STREAM_INTS_DISABLE();
            BMP085_Initiate_up_conversion();	// Begin a new sample
            DelayMs(100);
            BMP085_read_up();					// Read last sample
            sampler.lastPressure = BMP085_calc_pressure(); // This is just the math bit
            DATA_STREAM_INTS_ENABLE();
        }
        printf("$PRES=%ld\r\n", sampler.lastPressure);
	}
	if (!z || z == '9')
	{
        printf("$INACT=%u\r\n", status.inactivity);
    }
	#ifdef HAVE_PROXIMITY_SENSOR
	if (!z)
	{
        printf("$PROX=%u\r\n", prox.proximity);
    }
	#endif
//if (writeHandler == Bluetooth_write) 	BluetoothSerialWait();
//else if (writeHandler == usb_write) 	USBCDCWait();

    return COMMAND_RETURN_OK;
}


// Command: STATUS
int Command_STATUS(commandParserState_t *cmd)
{
	char z = 0;
	if (cmd->argc > 1 && cmd->argv[1][0] != '\0') 
	{ 
		z = cmd->argv[1][0]; 
		if (z >= 'a' && z <= 'z') { z = z + 'A' - 'a' ; }
	}

	if (z == '1' || z == 'T')
	{
	    if (logger.sampling != SAMPLING_OFF || status.wireless != WIRELESS_OFF) { printf("ERROR: Must not be sampling and wireless must be off.\r\n"); }
		else
		{
			unsigned short selfTest;
			printf("PROGRESS=");
			selfTest = SelfTest(1);
			printf("\r\n");
			StateSetSampling(SAMPLING_OFF, 1);		// re-init sampling to off
			printf("TEST=%04x\r\n", selfTest);
		}
	}
	
    if (!z || z == '2' || z == 'B') { printf("BATTHEALTH=%u\r\n", SettingsGetLogValue(LOG_VALUE_BATTERY)); }
	if (!z || z == '3' || z == 'F') { printf("FTL=%s\r\n", FtlHealth()); }									// FTL=warn#,erro#,spare,2(planes),bad-0,bad-1
	if (!z || z == '4' || z == 'C') { unsigned short checksum = 0; printf("CHECKSUM=%04x\r\n", checksum); }	// LATER: Firmware checksum -- avoiding writeable areas (+ bootloader?)
	if (!z || z == '5' || z == 'R') { printf("RESTART=%u\r\n", SettingsGetLogValue(LOG_VALUE_RESTART)); }	
	if (!z || z == '6' || z == 'N') { printf("NAND=%04x\r\n", FtlVerifyNandParameters()); }					// Compare NAND parameters agains compiled-in constants	
    return COMMAND_RETURN_OK;
}    


// Command: RESET
int Command_RESET(commandParserState_t *cmd)
{
    if (!(cmd->flags & COMMAND_MASK_OUTPUT))	{ return COMMAND_RETURN_RESTRICTED; }	// disallow reset if no output (e.g. if from batch file)
    printf("RESET\r\nOK\r\n");
    SettingsActionDelay(ACTION_RESTART, 2);
    return COMMAND_RETURN_OK;
}


// Command: RATE
int Command_RATE(commandParserState_t *cmd)
{
	// Old-format rate command (accelerometer only, set by rate-code)
	if (cmd->command->id == 0)
	{
	    if (cmd->argc > 1)																	// set parameter
	    {
            unsigned short value;
	        if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow set while restricted
            value = my_atoi(cmd->argv[1]);

            if ((AccelSetting(ACCEL_FREQUENCY_FOR_RATE(value), ACCEL_RANGE_FOR_RATE(value)) & 0x8000))
            {
                printf("ERROR: RATE value not valid.\n");
                return COMMAND_RETURN_PARAMETER;
            }

            // Set accelerometer from backwards-compatible rate code
            settings.sensorConfig[SENSOR_INDEX_ACCEL].enabled = 1;
            settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency = ACCEL_FREQUENCY_FOR_RATE(value);
            settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity = ACCEL_RANGE_FOR_RATE(value);
	    }
        // Display backwards-compatible accelerometer rate code
        {
            unsigned char rate = (unsigned char)AccelSetting(settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency, settings.sensorConfig[SENSOR_INDEX_ACCEL].sensitivity);
            printf("RATE=%u,%u\r\n", rate, settings.sensorConfig[SENSOR_INDEX_ACCEL].frequency);
        }
	}
	else
	{
		//RATE <device>[,<enabled>[,<frequency>[,<sensitivity>]]]
        sensor_config_t *sensorConfig = (sensor_config_t *)cmd->command->pointer;
		char device = -1;

        if (cmd->argc > 1 && (cmd->flags & COMMAND_FLAG_RESTRICTED)) { return COMMAND_RETURN_RESTRICTED; }	// disallow set while restricted
        if (sensorConfig == NULL) { return COMMAND_RETURN_NOT_HANDLED; }
		
        device = cmd->command->id;

        if (cmd->argc > 1)
        {
            char enabled = -1;
            short frequency = -1;
            short sensitivity = -1;
            char ok;

            if (cmd->argc > 1) { if (cmd->argv[1][0] != '\0') { enabled     =  (char)atoi(cmd->argv[1]); }  }
            if (cmd->argc > 2) { if (cmd->argv[2][0] != '\0') { frequency   = (short)atoi(cmd->argv[2]); } }
            if (cmd->argc > 3) { if (cmd->argv[3][0] != '\0') { sensitivity = (short)atoi(cmd->argv[3]); } }

            if (enabled == -1)     {     enabled = sensorConfig->enabled;     }
            if (frequency == -1)   {   frequency = sensorConfig->frequency;   }
            if (sensitivity == -1) { sensitivity = sensorConfig->sensitivity; }

            // Validate settings
            ok = 0;
            switch (device)
            {
                case 'A': if (AccelValidSettings(frequency, sensitivity, 0)) { ok = 1; } break;
                case 'G': if ( GyroValidSettings(frequency, sensitivity, 0)) { ok = 1; } break;
                case 'M': if (  MagValidSettings(frequency, sensitivity, 0)) { ok = 1; } break;
                case 'P': if (frequency == 1) { ok = 1; } break;
                case 'T': if (frequency == 1) { ok = 1; } break;
#ifdef SYNCHRONOUS_SAMPLING
                case 'X': { ok = 1; } break;
#endif
            }

            if (!ok)
            {
                printf("ERROR: RATE %c parameters not valid.\n", device);
                return COMMAND_RETURN_PARAMETER;
            }
	
            // Setting
            sensorConfig->enabled = enabled;
            sensorConfig->frequency = frequency;
            sensorConfig->sensitivity = sensitivity;
        }
		
		printf("RATE=%c,%d,%d,%d\r\n", device, sensorConfig->enabled, sensorConfig->frequency, sensorConfig->sensitivity);
	}
    return COMMAND_RETURN_OK;
}


// Command: CLEAR
int Command_CLEAR(commandParserState_t *cmd)
{
    char flags = 0;
    
	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted

    if (cmd->command->id == 'D')
    {
        flags |= ACTION_DELETE | ACTION_CREATE;	// ACTION_DELETE  ACTION_FORMAT_QUICK
    }
    else if (cmd->command->id == 'A')
    {
        DefaultConfiguration(); //SettingsInitialize();
        flags |= ACTION_FORMAT_WIPE | ACTION_CREATE;	// ACTION_DELETE 
    }

    if (flags != 0)
    {
        if (status.drive == DRIVE_MOUNTED)
        {
			StateSetDrive(DRIVE_OWNED, 1, RECORD_STOP_REASON_COMMAND);
            SettingsActionDelay(flags | ACTION_REMOUNT, 2);
            printf("CLEAR: Delayed activation.\r\n");
        }
        else
        {
            SettingsAction(flags);
            printf("CLEAR\r\n");
        }
    }
    else
    {
	    if (cmd->command->id == 'S')	// Clear settings
	    {
            printf("CLEAR: Defaults\r\n");
	        DefaultConfiguration();
	    }
	    else if (cmd->command->id == 'L')	// Reload settings
	    {
            printf("CLEAR: Reload\r\n");
            LoggerReadMetadata(logger.filename);                        // Read settings from the logging binary file
	    }
	    else
	    {
	        printf("CLEAR: Use 'clear data', 'clear all'.\r\n");
	    }    
    }
    return COMMAND_RETURN_OK;
}


// Command: COMMIT
int Command_COMMIT(commandParserState_t *cmd)
{
    char flags = 0;
    
	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
    if (logger.recording == RECORDING_ON) { return COMMAND_RETURN_RESTRICTED; }

    flags = ACTION_CREATE;

    if (status.drive == DRIVE_MOUNTED)
    {
		StateSetDrive(DRIVE_OWNED, 1, RECORD_STOP_REASON_COMMAND);
        SettingsActionDelay(flags | ACTION_REMOUNT, 2);
        printf("COMMIT: Delayed activation.\r\n");
    }
    else
    {
        SettingsAction(flags);
    }

    return COMMAND_RETURN_OK;
}


// Command: STREAM
int Command_STREAM(commandParserState_t *cmd)
{
	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow stream while restricted
	
#if 0
	// Support old 'STREAM' command -- set stream mode directly
    if (cmd->argc <= 1 && (cmd->flags & COMMAND_FLAG_OUTPUT_PRIMARY))
   	{
        // Set the response to the same channel as the query
		if (cmd->flags & COMMAND_FLAG_OUTPUT_PRIMARY) { StateSetStream(STREAM_USB); }
		else if (cmd->flags & COMMAND_FLAG_OUTPUT_ALTERNATIVE) { StateSetStream(STREAM_WIRELESS); }
	}
#endif

    if (cmd->argc > 1)																	// set parameter
    {
        char value = (char)atoi(cmd->argv[1]);
        if (value == 0) { StateSetStream(STREAM_NONE); }
        else if (value == 1) { StateSetStream(STREAM_WIRELESS); }
        else if (value == 2) { StateSetStream(STREAM_USB); }
        else if (value == 3) { StateSetStream(STREAM_BOTH); }
        else { return COMMAND_RETURN_NOT_HANDLED; }
    }
    printf("STREAM=%d\r\n", logger.stream);
    return COMMAND_RETURN_OK;
}


// Command: ANNOTATE**
int Command_ANNOTATE(commandParserState_t *cmd)
{
	unsigned char id;
	int j;
	
	// Check a two-digit numeric annotation id was specified as part of the command
	if (cmd->argv[0][8] < '0' || cmd->argv[0][8] > '9' || cmd->argv[0][9] < '0' || cmd->argv[0][9] > '9')
	{
		return COMMAND_RETURN_NOT_HANDLED;
	}
    id = (cmd->argv[0][8] - '0') * 10 + (cmd->argv[0][9] - '0');
    if (id >= ANNOTATION_COMPATABILITY_COUNT) { return COMMAND_RETURN_NOT_HANDLED; }    // 14x 32-byte annotation chunks = 448 bytes
    
	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow annotate functions while restricted
	
	// Set annotation
    if (cmd->argc > 1 && id < ANNOTATION_COUNT)
    {
        unsigned short len = strlen(cmd->argv[1]);
        if (len > ANNOTATION_SIZE) { len = ANNOTATION_SIZE; }
        memcpy(settings.annotation[id], cmd->argv[1], len);
        memset(settings.annotation[id] + len, ' ', ANNOTATION_SIZE - len);
    }

	// Display annotation
    printf("ANNOTATE%02d=", id);
    for (j = 0; j < ANNOTATION_SIZE; j++)
    {
	    if (id < ANNOTATION_COUNT)
	    {
	        printf("%c", ((settings.annotation[id][j]) < 32 ? ' ' : settings.annotation[id][j]));
	    }
	    else 
	    {
	        printf(" ");		// for compatability, fake (un-stored) banks 7-14
	    }    
    }
    printf("\r\n");
    return COMMAND_RETURN_OK;
}


// Command: MOUNT
int Command_MOUNT(commandParserState_t *cmd)
{
    if (cmd->argc > 1)																	// set parameter
    {
		char value;
		if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow changing mount while restricted
        value = (char)atoi(cmd->argv[1]);
		FtlFlush(0);

        if (value && status.drive != DRIVE_MOUNTED)
        {
            StateSetDrive(DRIVE_MOUNTED, 0, RECORD_STOP_REASON_COMMAND);
        }
        else if (!value && status.drive == DRIVE_MOUNTED)
        {
            StateSetDrive(DRIVE_OWNED, 0, RECORD_STOP_REASON_COMMAND);
		}
	} 
    printf("MOUNT=%d\r\n", (status.drive == DRIVE_MOUNTED) ? 1 : 0);
    return COMMAND_RETURN_OK;
}


// Command: FORMAT
int Command_FORMAT(commandParserState_t *cmd)
{
	char mode = 0;
	char commit = 0;
	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow formatting while restricted
    if (logger.recording == RECORDING_ON) { return COMMAND_RETURN_RESTRICTED; }

    if (cmd->argc > 1)																	// set parameter
    {
	    const char *p;
	    for (p = cmd->argv[1]; *p != '\0'; p++)
	    {
		    if (mode == 0 && (*p == 'q' || *p == 'Q')) { mode = 1; }
		    else if (mode == 0 && (*p == 'w' || *p == 'W')) { mode = 2; }
		    else if (commit == 0 && (*p == 'c' || *p == 'C')) { commit = 1; }
		    else { return COMMAND_RETURN_PARAMETER; }
	    }
    }
	
	if (mode == 0)
	{
        printf("ERROR: Must specify 'FORMAT Q' for quick or 'FORMAT W' for wipe, +'C' for COMMIT.\r\n");
        return COMMAND_RETURN_PARAMETER;
	}
	
	// Format
    {
        char flags = 0;
        
        DefaultConfiguration(); //SettingsInitialize();
        
        if (mode == 2) { flags |= ACTION_FORMAT_WIPE; }
        else { flags |= ACTION_FORMAT_QUICK; }
        if (commit) { flags |= ACTION_CREATE; } // Create data file with current settings

        if (status.drive == DRIVE_MOUNTED)
        {
			StateSetDrive(DRIVE_OWNED, 1, RECORD_STOP_REASON_COMMAND);
			SettingsActionDelay(flags | ACTION_REMOUNT, 2);            
            printf("FORMAT: Delayed activation.\r\n");
        }
        else
        {
            SettingsAction(flags);
        }
    }
    return COMMAND_RETURN_OK;
}


// Command: LOGFILE
int Command_LOGFILE(commandParserState_t *cmd)
{
	FSFILE *fp;
    unsigned short fractional;
    DateTime time;
    
    // Cannot write if locked, attached & mounted, or if no message specified
	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow logging while restricted
	if (status.drive != DRIVE_OWNED) { printf("ERROR: Access denied.\r\n"); return COMMAND_RETURN_RESTRICTED; }
    if (cmd->argc <= 1) { printf("ERROR: Message not specified.\r\n"); return COMMAND_RETURN_PARAMETER; }
	
	// Open the log file
    fp = FSfopen(LOG_FILE, "a");
    if (fp == NULL) { printf("ERROR: Problem opening log file.\r\n"); return COMMAND_RETURN_RESTRICTED; }
	
	// Append the message
    time = RtcNowFractional(&fractional);
    FSfprintf(fp, "%s.%03d,%s\r\n", RtcToString(time), RTC_FRACTIONAL_TO_MS(fractional), cmd->argv[1]);
    FSfclose(fp);
    
    return COMMAND_RETURN_OK;
}


// Command: LOG
int Command_LOG(commandParserState_t *cmd)
{
	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow logging while restricted
	
    if (cmd->argc > 1)
    {
		unsigned long timestamp = RtcNow();
		printf("LOG,%d,0x%04x,%s,%s\r\n", 0, LOG_CATEGORY_USER, RtcToString(timestamp), cmd->argv[1]);
        SettingsAddLogEntry(LOG_CATEGORY_USER, timestamp, cmd->argv[1]);
    }
    else
    {
        int index;
        for (index = LOG_COUNT - 1; index >= 0; index--)
        {
            unsigned short status;
            unsigned long timestamp;
            const char *value = SettingsGetLogEntry(index, &status, &timestamp);
            if (value != NULL && status != 0xffff && status != 0x0000 && *((const unsigned char *)value) != 0xff)
            {
                printf("LOG,%d,0x%04x,%s,%s\r\n", index, status, RtcToString(timestamp), value);
           		USBCDCWait();
            }
        }
        printf("OK\r\n");
    }
    return COMMAND_RETURN_OK;
}


// Command: LOCK (current lock value for connection)
int Command_LOCK(commandParserState_t *cmd)
{
	// Set LOCK status
    if (cmd->argc == 2)
    {
	    unsigned short i;
		if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
		if (strlen(cmd->argv[1]) > 5 || cmd->argv[1][0] < '0' || cmd->argv[1][0] > '9') { return COMMAND_RETURN_PARAMETER; }
		i = (unsigned short)atoi(cmd->argv[1]);
	    if (i == 0xffff) { i = 0x0000; }		// 0xffff treated as uninitialized
        status.lockCode = i;
	}
    
    // Display LOCK status
    {
        unsigned int lockStatus = 0;
        unsigned short iCode = 0xffff;
        CONFIG_GET(lockCode, &iCode);
        if (status.lockCode != 0x0000) { lockStatus += 1; }
        if (iCode != 0 && iCode != 0xffff) { lockStatus += 2; }
        printf("LOCK=%d\r\n", lockStatus);
    }
    return COMMAND_RETURN_OK;
}	
    

// Command: ILOCK (initial lock value on connection)
int Command_ILOCK(commandParserState_t *cmd)
{
	// Set ILOCK status
    if (cmd->argc == 2)
    {
	    unsigned short i;
		if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
		if (strlen(cmd->argv[1]) > 5 || cmd->argv[1][0] < '0' || cmd->argv[1][0] > '9') { return COMMAND_RETURN_PARAMETER; }
		i = (unsigned short)atoi(cmd->argv[1]);
        CONFIG_SET(lockCode, &i);
	}
    
    // Display ILOCK status
    {
        unsigned short iCode = 0xffff;
        CONFIG_GET(lockCode, &iCode);
		if (iCode == 0x0000 || iCode == 0xffff) printf("ILOCK=0\r\n"); else printf("ILOCK=1\r\n");
    }
    return COMMAND_RETURN_OK;
}


// Command: UNLOCK (unlock the current session)
int Command_UNLOCK(commandParserState_t *cmd)
{
	if (cmd->argc == 2)
    {
        unsigned short i = (unsigned short )atoi(cmd->argv[1]);
		if ((cmd->flags & COMMAND_FLAG_RESTRICTED) && i != status.lockCode)
        {
            DelayMs(1000);
			return COMMAND_RETURN_RESTRICTED;
        }
        if (status.lockCode != 0x0000)
        {
	        unsigned short iCode = 0xffff;
            status.lockCode = 0x0000;

            // If we were initially locked, auto-mount the drive
	        CONFIG_GET(lockCode, &iCode);			
			if (iCode != 0x0000 && iCode != 0xffff && status.drive != DRIVE_MOUNTED)
            {
	            FtlFlush(0);
                status.drive = DRIVE_MOUNTED;
            }
        }
        printf("LOCK=0\r\n");
    }
    return COMMAND_RETURN_OK;
}	


// Command: ECC
int Command_ECC(commandParserState_t *cmd)
{
    if (cmd->argc > 1)
    {
        int i;
    	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow set while restricted
        i = atoi(cmd->argv[1]);
        status.dataEcc = (char)i;
        CONFIG_SET(ecc, &i);
    }
    printf("ECC=%d\r\n", status.dataEcc);
    return COMMAND_RETURN_OK;
}	


// Command: RECORDING
int Command_RECORDING(commandParserState_t *cmd)
{
    if (cmd->argc > 1)
    {
	    char v;
    	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
        v = (unsigned int)atoi(cmd->argv[1]);
        logger.schedule = v;
    }
    printf("RECORDING=%d\r\n", logger.schedule);
    return COMMAND_RETURN_OK;
}	


// Command: DIR
int Command_DIR(commandParserState_t *cmd)
{
	const char *filespec = status.filespec;
	SearchRec rec;
    int safety = 0;
	
   	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
	if (status.drive != DRIVE_OWNED && status.drive != DRIVE_OWNED_FULL) { return COMMAND_RETURN_RESTRICTED; }   	// disallow if mounted
	
    if (cmd->argc > 1) { filespec = cmd->argv[1]; }
    
	if (!FindFirst(filespec, ATTR_ARCHIVE | ATTR_READ_ONLY, &rec))	// ATTR_MASK = all entries
	{
	    do
	    {
	        if (!(rec.attributes & ATTR_DIRECTORY) && !(rec.attributes & ATTR_VOLUME) && !(rec.attributes & ATTR_HIDDEN))
	        {
			    printf("$FILE,%s,%lu\r\n", rec.filename, rec.filesize);
	        }
            if (safety++ >= 10000) { break; }   // guard against corrupt FS
	    } while (!FindNext(&rec));
	}
    printf("OK\r\n");
    return COMMAND_RETURN_OK;
}	


// Command: DEL
int Command_DEL(commandParserState_t *cmd)
{
	const char *filename = NULL;
   	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
	if (status.drive != DRIVE_OWNED && status.drive != DRIVE_OWNED_FULL) { return COMMAND_RETURN_RESTRICTED; }   	// disallow if mounted
    if (cmd->argc > 1) { filename = cmd->argv[1]; }
	if (filename == NULL) { return COMMAND_RETURN_PARAMETER; }
	if (FSremove(filename) != 0)
	{
		return COMMAND_RETURN_RESTRICTED;
	}
    printf("OK\r\n");
    return COMMAND_RETURN_OK;
}	


// Command: OPEN
int Command_OPEN(commandParserState_t *cmd)
{
    if (cmd->argc > 1)
    {
	   	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
        if (logger.readFile != NULL) { logger.readLength = 0; FSfclose((FSFILE*)logger.readFile); logger.readFile = NULL; }

        // Drive must be owned
        if (status.drive != DRIVE_OWNED && status.drive != DRIVE_OWNED_FULL) { return COMMAND_RETURN_RESTRICTED; }

        // Close any existing file
        if (logger.readFile != NULL) { FSfclose(logger.readFile); logger.readFile = NULL; logger.readLength = 0; logger.readPosition = 0; }

        // Open specified file
        logger.readFile = FSfopen(cmd->argv[1], "rb");
        logger.readLength = 0;
        logger.readPosition = 0;
    }
    printf("OPEN=%d\r\n", (logger.readFile != NULL) ? 1 : 0);
    return COMMAND_RETURN_OK;
}	


// Command: CLOSE
int Command_CLOSE(commandParserState_t *cmd)
{
	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
	if (logger.readFile != NULL) { FSfclose((FSFILE*)logger.readFile); logger.readFile = NULL; }
    logger.readLength = 0;
    logger.readPosition = 0;
    printf("OK\r\n");
    return COMMAND_RETURN_OK;
}	


// Command: SEEK
int Command_SEEK(commandParserState_t *cmd)
{
    if (cmd->argc > 1)
    {
    	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
        unsigned long pos = strtoul(cmd->argv[1], NULL, 10);
        logger.readPosition = pos;
        logger.readLength = 0;
    }
	if (logger.readFile == NULL)
	{
	    printf("SEEK=%lu\r\n", logger.readPosition);   // seek for read from logging file
	}
	else
	{
        FSfseek((FSFILE*)logger.readFile, logger.readPosition, SEEK_SET);
	    printf("SEEK=%lu\r\n", FSftell((FSFILE*)logger.readFile));
	}
    return COMMAND_RETURN_OK;
}	


// Command: READ
int Command_READ(commandParserState_t *cmd)
{
   	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
    if (cmd->argc > 1)
    {
        logger.readLength = strtoul(cmd->argv[1], NULL, 10);
        if (logger.readLength > 0x8000ul) { logger.readLength = 0x8000ul; }	// 32kB max read chunk

        // Set the response to the same channel as the query
        logger.fileReadStream = STREAM_NONE;
		if (cmd->flags & COMMAND_FLAG_OUTPUT_PRIMARY) { logger.fileReadStream = STREAM_USB; }
		if (cmd->flags & COMMAND_FLAG_OUTPUT_ALTERNATIVE) { logger.fileReadStream = STREAM_WIRELESS; }
    }
	printf("READ=%lu\r\n", logger.readLength);
    return COMMAND_RETURN_OK;
}	


// Command: WIRELESS
int Command_WIRELESS(commandParserState_t *cmd)
{
    if (cmd->argc > 1)
    {
        short value;    // Make sure this is the same type as config_t.discoverable (short)
    	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
        value = (char)atoi(cmd->argv[1]);
        if (value == -1) { value = 2; } // 2 = auto
        status.discoverable = value;
		// Write to NVM
        CONFIG_SET(discoverable, &value);
        // Disconnect if required
        if (status.discoverable == 0)
        {
            StateSetWireless(WIRELESS_OFF);
        }
    }
    printf("WIRELESS=%d,%d\r\n", status.discoverable, status.wireless);
    return COMMAND_RETURN_OK;
}	


// Command: ANNOTATION
int Command_ANNOTATION(commandParserState_t *cmd)
{
   	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
    if (cmd->argc <= 1) { printf("ERROR: Annotation not specified.\r\n"); return COMMAND_RETURN_PARAMETER; }
    if (RecorderAnnotate(cmd->argv[1], strlen(cmd->argv[1])))
    {
	    status.alertTimer = 0;
        printf("ANNOTATION=1\r\n");
    }
    else
    {
        printf("ANNOTATION=0\r\n");
    }
    return COMMAND_RETURN_OK;
}	


// Command: FILE
int Command_FILE(commandParserState_t *cmd)
{
    if (cmd->argc > 1)
    {
    	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow while restricted
    	if (!ValidFilename(cmd->argv[1], 1)) { return COMMAND_RETURN_PARAMETER; }
    	// Copy filename to status (normalized)
	    {
		    int i;
		    const char *s = cmd->argv[1];
		    for (i = 0; i < sizeof(status.filespec); i++)
		    {
			    char c;
			    if (s == NULL || i >= sizeof(status.filespec) - 1) { c = '\0'; }	// pad with null
			    else
			    {
				    c = *s++;
					if (c >= 'a' && c <= 'z') { c = c + 'A' - 'a'; }	// normalize case
					if (c == '#') { c = '?'; }							// treat '#' placeholder as '?'
					if (c == '\0') { s = NULL; }						// end of source string (write null pads from now on)
				}
				status.filespec[i] = c;
			} 
	    }
		// Write filename to NVM
        CONFIG_SET(filespec, status.filespec);
    }
    printf("FILE=%s\r\n", status.filespec);
    return COMMAND_RETURN_OK;
}	


// Command: CAL**
int Command_CAL(commandParserState_t *cmd)
{
	unsigned char id;
	int j;

	// Check a two-digit numeric annotation id was specified as part of the command
	if (cmd->argv[0][3] < '0' || cmd->argv[0][3] > '9' || cmd->argv[0][4] < '0' || cmd->argv[0][4] > '9')
	{
		return COMMAND_RETURN_NOT_HANDLED;
	}
    id = (cmd->argv[0][3] - '0') * 10 + (cmd->argv[0][4] - '0');
    if (id >= 4) { return COMMAND_RETURN_PARAMETER; }    // 4x 8x calibration words
    
	if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow all calibration commands while restricted

	// Set calibration
    if (cmd->argc > 1)
    {
		if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }	// disallow calibration setting while restricted
	    for (j = 0; j < 8; j++)
	    {
		    if (j < cmd->argc - 1 && cmd->argv[j] != NULL && cmd->argv[j][0] != '\0')
		    {
			    status.calibration[id][j] = (short)atoi(cmd->argv[j + 1]);
		    }
		} 
		// Write to NVM
		CONFIG_SET(calibration, status.calibration);
    }

	// Display calibration
    printf("CAL%02d=", id);
    for (j = 0; j < 8; j++)
    {
	    if (j != 0) { printf(","); }
        printf("%d", status.calibration[id][j]);
    }
    printf("\r\n");
    return COMMAND_RETURN_OK;
}


// Command: INFO - debugging info
int Command_INFO(commandParserState_t *cmd)
{
	printf("INFO=%d,ACCEL_INT2_IE=%d,ACCEL_INT2_IF=%d,ACCEL_INT2=%d\r\n", status.inactivity, ACCEL_INT2_IE, ACCEL_INT2_IF, ACCEL_INT2);
    return COMMAND_RETURN_OK;
}





// CRC-protected commands
#ifdef CRC_COMMANDS

// Maximum mode index (of USB/alternate)
#define SETTINGS_MAX_MODE (SETTINGS_USB > SETTINGS_ALTERNATE ? SETTINGS_USB : SETTINGS_ALTERNATE)

// CRC-protected commands/responses
typedef enum { CRC_STATE_NONE, CRC_STATE_COMMAND, CRC_STATE_RESPONSE } crc_state_t;

// State tracking of CRC-protected commands/responses
typedef struct
{
	crc_state_t crcState;
	unsigned short commandId;
	unsigned short commandCrc;
	unsigned short responseCrc;
	unsigned char hasCommandCrc;	// 1=CRC_STATE_COMMAND protected by CRC, 0=CRC_STATE_COMMAND not protected by CRC (but caller still wants the ID and response CRC)
} crc_command_t;

// Per-interface state tracking of CRC-protected commands/responses
static crc_command_t crcCommand[SETTINGS_MAX_MODE] = {{0}};
#define COMMAND_RETURN_CRC -123		// Extends values from return

// CCITT 16-bit CRC
#define CRC_INITIAL 0xffff
static unsigned short crc16(const unsigned char *buffer, unsigned short len, unsigned short crc)
{
	int i;
	while (len--)
	{
		crc ^= (unsigned short)*buffer++ << 8;
		for (i = 0; i < 8; i++) { crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : (crc << 1); }
	}
	return crc;
}


// CRC-capable write handler for Bluetooth transport
void crc_Bluetooth_write(const void *buffer, unsigned int len)
{
	// Update the CRC for this interface
	SettingsMode mode = SETTINGS_ALTERNATE;
	if (crcCommand[mode].crcState == CRC_STATE_RESPONSE)
	{
		crcCommand[mode].responseCrc = crc16(buffer, len, crcCommand[mode].responseCrc);
	}
	// Chain to actual write handler
	Bluetooth_write(buffer, len);
}
	
// CRC-capable write handler for USB transport
void crc_usb_write(const void *buffer, unsigned int len)
{
	// Update the CRC for this interface
	SettingsMode mode = SETTINGS_USB;
	if (crcCommand[mode].crcState == CRC_STATE_RESPONSE)
	{
		crcCommand[mode].responseCrc = crc16(buffer, len, crcCommand[mode].responseCrc);
	}
	// Chain to actual write handler
	usb_write(buffer, len);
}

// Command: AT
int Command_AT(commandParserState_t *cmd)
{
    if (cmd->argc > 1)
    {
        unsigned short id = (unsigned short)strtoul(cmd->argv[1], NULL, 10);
        unsigned short crc = 0;
        unsigned char hasCrc = 0;
    
	    if (cmd->argc > 2)
	    {
	    	hasCrc = 1;
	        crc = (unsigned short)strtoul(cmd->argv[2], NULL, 16);
	    }    
        
		SettingsMode mode = 0;
		if (cmd->flags & COMMAND_FLAG_OUTPUT_PRIMARY) { mode = SETTINGS_USB; }
		if (cmd->flags & COMMAND_FLAG_OUTPUT_ALTERNATIVE) { mode = SETTINGS_ALTERNATE; }
		
		crcCommand[mode].crcState = CRC_STATE_COMMAND;
		crcCommand[mode].commandId = id;
		crcCommand[mode].commandCrc = crc;
		crcCommand[mode].responseCrc =  CRC_INITIAL;
		crcCommand[mode].hasCommandCrc = hasCrc;
		
		if (!hasCrc)
		{
	    	printf("OK %u\r\n", crcCommand[mode].commandId);
	 	}
	 	else
		{
	    	printf("OK %u %04X\r\n", crcCommand[mode].commandId, crcCommand[mode].commandCrc);
		}	
    }
    else
    {
	    printf("OK\r\n");
	} 
	return COMMAND_RETURN_OK;
}	
#endif


// Command list
const command_definition_t settingsCommandDefinitions[] =
{
    // Simple string returns
    {   0, "",              CommandHandlerPrintString, 	"\r\n", 0 },                        //
    {   0, "HELP", 			CommandHandlerPrintString, 	"HELP: help|echo|reset\r\n", 0 },   //printf("HELP: help|echo|id|status|sample|time|hibernate|stop|standby|serial|debug|clear|device|session|rate|annotate|read|erase|led|reset\r\n");
    {   0, "EXIT", 			CommandHandlerPrintString, 	"INFO: You'll have to close the terminal window yourself!\r\n", 0 },
    {   0, "CLS",           CommandHandlerPrintString, 	"\f", 0 },                          //
#ifdef CRC_COMMANDS
    {   0, "AT",            Command_AT,                 NULL, 0 },
#else
    {   0, "AT",   			CommandHandlerPrintString, 	"OK\r\n", 0 },
#endif
    {   0, "STANDBY",   	CommandHandlerPrintString, 	"STANDBY=0\r\n", 0 },				// (backwards compatibility)
    {   0, "MAXSAMPLES",  	CommandHandlerPrintString,  "MAXSAMPLES=0\r\n", 0 },            // (backwards compatibility)
    {   0, "LASTCHANGED",  	CommandHandlerPrintString,  "LASTCHANGED=0\r\n",        0 },    // (backwards compatibility)
    {   0, "TZONE",   	    CommandHandlerPrintString, 	"TZONE=\r\n", 0 },                  // (backwards compatibility)
    {   0, "UNMOUNT",   	CommandHandlerPrintString, 	"ERROR: Please use the command: MOUNT 0\r\n", 0 },	// reference
    // Commands to ignore
    {   0, "CMD",           CommandHandlerPrintString, 	NULL, 0 },                          // a possible response from Bluetooth command mode
    {   0, "AOK",           CommandHandlerPrintString, 	NULL, 0 },                          // a possible response from Bluetooth command mode
    {   0, "ERR",           CommandHandlerPrintString, 	NULL, 0 },                          // a possible response from Bluetooth command mode
    {   0, "!",             CommandHandlerPrintString, 	NULL, 0 },                          // a possible response from Bluetooth command mode
    {   0, "?",             CommandHandlerPrintString, 	NULL, 0 },                          // a possible response from Bluetooth command mode
    // Simple value settings
    {   0, "ECHO", 			CommandHandlerChar, 		&commEcho, 				  	0 },
    {   0, "LED", 			CommandHandlerChar, 		&status.ledOverride, 	  	0 },
    {   0, "DEBUG", 		CommandHandlerChar, 		&settings.debuggingInfo, 	0 },
    {   0, "MODE",          CommandHandlerUShort, 		&settings.streamFormat,     0 },
    {   0, "SESSION",  		CommandHandlerULong, 		&settings.sessionId,		0 },
    {   0, "START", 	    CommandHandlerTime, 		&settings.loggingStartTime, COMMAND_DEFINITION_UNPARSED },
    {   0, "HIBERNATE", 	CommandHandlerTime, 		&settings.loggingStartTime, COMMAND_DEFINITION_UNPARSED },	// (backwards compatibility)
    {   0, "STOP", 			CommandHandlerTime, 		&settings.loggingEndTime,   COMMAND_DEFINITION_UNPARSED },
    //{   0, "LASTCHANGED", 	CommandHandlerTime, 		&settings.lastChangeTime,   COMMAND_DEFINITION_READ_ONLY | COMMAND_DEFINITION_UNPARSED },	// (backwards compatibility)
    //{   0, "DATAMODE", 		CommandHandlerChar, 		&settings.dataMode, 	  	0 },	// (not present)
    {   0, "DFC", 			CommandHandlerUShort, 		&logger.debugFlashCount, 	  	0 },
    // Special prefixes
    {   0, "#",             Command_Passthrough, 	    NULL, COMMAND_DEFINITION_PREFIX },	// pass through to other interface
    // Commands with custom handling
    {   0, "ID",  			Command_ID,   				NULL, 0 },
    {   0, "DEVICE",        Command_DEVICE,             NULL, 0 },
    {   0, "TIME",  		Command_TIME, 				NULL, COMMAND_DEFINITION_UNPARSED },
    //{   0, "TZONE",  		Command_TZONE, 				NULL, 0 },  // (not present)
    {   0, "SAMPLE",  		Command_SAMPLE,				NULL, 0 },
    {   0, "STATUS",  		Command_STATUS,				NULL, 0 },
    {   0, "RESET",  	    Command_RESET,				NULL, 0 },
    { 'A', "RATE A", 	    Command_RATE, &settings.sensorConfig[SENSOR_INDEX_ACCEL], 0 },
    { 'G', "RATE G", 	    Command_RATE, &settings.sensorConfig[SENSOR_INDEX_GYRO] , 0 },
    { 'M', "RATE M", 	    Command_RATE, &settings.sensorConfig[SENSOR_INDEX_MAG]  , 0 },
    { 'P', "RATE P", 	    Command_RATE, &settings.sensorConfig[SENSOR_INDEX_ALT]  , 0 },
    { 'T', "RATE T", 	    Command_RATE, &settings.sensorConfig[SENSOR_INDEX_ADC]  , 0 },
#ifdef SYNCHRONOUS_SAMPLING
    { 'X', "RATE X", 	    Command_RATE, &settings.sensorConfig[SENSOR_INDEX_ALLAXIS]  , 0 },
#endif
    {   0, "RATE",  	    Command_RATE,				NULL, 0 },	// (backwards compatibility)
    { 'D', "CLEAR DATA",    Command_CLEAR,              NULL, 0 },
    { 'A', "CLEAR ALL",     Command_CLEAR,              NULL, 0 },
    { 'S', "CLEAR SETTINGS",Command_CLEAR,              NULL, 0 },
    { 'L', "CLEAR LOAD",    Command_CLEAR,              NULL, 0 },
    {   0, "CLEAR",         Command_CLEAR,              NULL, 0 },
    {   0, "COMMIT",        Command_COMMIT,             NULL, 0 },
    {   0, "STREAM",        Command_STREAM,             NULL, 0 },
	{   0, "ANNOTATE**",    Command_ANNOTATE,           NULL, COMMAND_DEFINITION_UNPARSED },
    {   0, "MOUNT",         Command_MOUNT,              NULL, 0 },
    {   0, "FORMAT",        Command_FORMAT,             NULL, 0 },
	{   0, "LOGFILE",       Command_LOGFILE,            NULL, COMMAND_DEFINITION_UNPARSED },
	{   0, "LOG",           Command_LOG,                NULL, COMMAND_DEFINITION_UNPARSED },        
    {   0, "LOCK",          Command_LOCK,               NULL, 0 },
    {   0, "ILOCK",         Command_ILOCK,              NULL, 0 },
    {   0, "UNLOCK",        Command_UNLOCK,             NULL, 0 },
    {   0, "ECC",           Command_ECC,                NULL, 0 },
    {   0, "RECORDING",     Command_RECORDING,          NULL, 0 },
    {   0, "DIR",           Command_DIR,                NULL, 0 },
    {   0, "DEL",           Command_DEL,                NULL, 0 },
    {   0, "OPEN",          Command_OPEN,               NULL, 0 },
    {   0, "CLOSE",         Command_CLOSE,              NULL, 0 },
    {   0, "SEEK",          Command_SEEK,               NULL, 0 },
    {   0, "READ",          Command_READ,               NULL, 0 },
    {   0, "WIRELESS",      Command_WIRELESS,           NULL, 0 },
    {   0, "ANNOTATION",    Command_ANNOTATION,         NULL, COMMAND_DEFINITION_UNPARSED },
    {   0, "FILE",          Command_FILE,               NULL, 0 },
    {   0, "CAL**",         Command_CAL,                NULL, 0 },
    {   0, "`",             Command_INFO,               NULL, 0 },
    
    {0}     // End of list
};

#ifdef APPLICATION_COMMANDS
extern const command_definition_t APPLICATION_COMMANDS[];
#endif





// Serial commands
char SettingsCommand(const char *line, SettingsMode mode)
{
	write_handler_t oldWriteHandler = writeHandler;
    int flags;
    int ret = 0;
    
    if (line == NULL) { return 0; }		// Cannot handle NULL line
    
    // Disable streaming
	//if (mode == SETTINGS_USB && logger.stream == STREAM_USB) { logger.stream = STREAM_NONE; }
    //else if (mode == SETTINGS_USB && logger.stream == STREAM_BOTH) { logger.stream = STREAM_WIRELESS; }

	// Run command parser
	flags = 0;
	if (mode == SETTINGS_USB)
	{ 
#ifdef CRC_COMMANDS
		writeHandler = crc_usb_write; 
#else
		writeHandler = usb_write; 
#endif
		flags |= COMMAND_FLAG_OUTPUT_PRIMARY;
	}
	else if (mode == SETTINGS_ALTERNATE)
	{
#ifdef CRC_COMMANDS
		writeHandler = crc_Bluetooth_write;
#else
		writeHandler = Bluetooth_write;
#endif
		flags |= COMMAND_FLAG_OUTPUT_ALTERNATIVE;
	}	
	else if (mode == SETTINGS_BATCH) { flags |= COMMAND_FLAG_SCRIPT; }
	
	// Check if restricted
	if ((mode == SETTINGS_USB || mode == SETTINGS_ALTERNATE) && (status.lockCode != 0x0000)) { flags |= COMMAND_FLAG_RESTRICTED; }		// Locked
	

#ifdef CRC_COMMANDS
	// CRC-protection on USB & alternative link (Bluetooth)
	if ((mode == SETTINGS_USB || mode == SETTINGS_ALTERNATE) && crcCommand[mode].crcState == CRC_STATE_COMMAND)
	{
		unsigned short crc;
		
		// Calculate the CRC of the incoming command line (ignore CR/LF)
		crc = crc16((unsigned char *)line, strlen(line), CRC_INITIAL);
		
		// If the protected command is a new 'AT' CRC command -- start a new command
		if ((line[0] == 'a' || line[0] == 'A') && (line[1] == 't' || line[1] == 'T') && line[2] == ' ')
		{
			crcCommand[mode].crcState = CRC_STATE_NONE;
		}
		else if (crcCommand[mode].hasCommandCrc)		// If the incoming command is protected by a CRC (and this is not just a request to protect the response)
		{
			// If it doens't match what we were told to expect, this is an error.
			if (crcCommand[mode].commandCrc != crc) 
			{ 
				crcCommand[mode].crcState = CRC_STATE_NONE;
				ret = COMMAND_RETURN_CRC; 
			}
		}
		else
		{
			// We didn't have incoming protection on the command's CRC, but our outgoing response can include the CRC of the command we received
			crcCommand[mode].commandCrc = crc;
		}	
		
		// If we're still in command mode, notify on command start
		if (crcCommand[mode].crcState == CRC_STATE_COMMAND)
		{
			// Notify that we're starting the response
		    printf("BEGIN %u %04X\r\n", crcCommand[mode].commandId, crcCommand[mode].commandCrc);
		    
			// Move to response mode
			crcCommand[mode].crcState = CRC_STATE_RESPONSE;
			crcCommand[mode].responseCrc = CRC_INITIAL;
		}	
	}
#endif
	
    // Ignore empty lines
    if (ret == 0 && line[0] == '\0')
    { 
    	ret = COMMAND_RETURN_OK; // Ignored
    }	
	
    // Application-specific command overrides
#ifdef APPLICATION_COMMANDS
    if (ret == 0)
    {
        ret = CommandParseList(APPLICATION_COMMANDS, line, flags);
    }
#endif
   	
    if (ret == 0)
    {
        ret = CommandParseList(settingsCommandDefinitions, line, flags);
    }
	
	if (ret == COMMAND_RETURN_RESTRICTED)
	{
		if (flags & COMMAND_FLAG_RESTRICTED) { printf("ERROR: Locked.\r\n"); }
		else { printf("ERROR: Access denied.\r\n"); }
	}
	else if (ret == COMMAND_RETURN_PARAMETER)
	{
		printf("ERROR: Invalid parameter.\r\n"); 
	}
	else if (ret == 0)
	{
        printf("ERROR: Unknown command: %s\r\n", line);
    }
    
#ifdef CRC_COMMANDS
	if (ret == COMMAND_RETURN_CRC)
	{
		printf("ERROR: CRC mismatch.\r\n"); 
	}

	// CRC-protection on USB & alternative link (Bluetooth)
	if ((mode == SETTINGS_USB || mode == SETTINGS_ALTERNATE) && crcCommand[mode].crcState == CRC_STATE_RESPONSE)
	{
		// Sent response, send end-of-response footer with ID and CRC
		crcCommand[mode].crcState = CRC_STATE_NONE;
		printf("DONE %u %04X\r\n", crcCommand[mode].commandId, crcCommand[mode].responseCrc); 
	}
#endif

	// Restore redirect
	writeHandler = oldWriteHandler;
    
    return ret;
}

