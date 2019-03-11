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

// Configuration settings, status and command handling
// Dan Jackson, 2011-2012

// Includes
#include "HardwareProfile.h"
#include <Compiler.h>
#include <TimeDelay.h>
#include "Peripherals/Analog.h"
#include "Peripherals/Rtc.h"
//#include "USB/USB.h"
//#include "USB/usb_function_msd.h"
//#include "USB/usb_function_cdc.h"
#include "Ftl/FsFtl.h"
#include "Utils/FsUtils.h"
#include "Peripherals/Accel.h"
#ifdef USE_GYRO
#include "Peripherals/Gyro.h"
#endif
#include "Peripherals/Rtc.h"
#include "Usb/USB_CDC_MSD.h"
#include "Utils/Util.h"
#include "Settings.h"
#include "Utils/Fifo.h"
#include "Logger.h"

#define RAW_NAND_DEBUG
#ifdef RAW_NAND_DEBUG
#include "Peripherals/Nand.h"
#endif

#ifdef USE_BMI160_IMU
#include "IMU-BMI160.h"
#endif

#define LOG_FILE "LOG.TXT"


extern unsigned char scratchBuffer[640];

Settings settings = {0};
Status status = {0};



// Get the device id
unsigned long SettingsGetDeviceId(void)
{
    unsigned long id = DEVICEID_UNSET;
    ReadProgram(DEVICE_ID_ADDRESS, &id, sizeof(unsigned long));
    // If high word is not set, treat as zero
    if ((id >> 16) == 0xffff)
    {
        id &= 0x0000fffful;
    }
    return id;
}


// Rewrite the device id
void SettingsSetDeviceId(unsigned long newId)
{
    // Check voltage ok to program
    if (adcResult.batt < BATT_CHARGE_MIN_LOG) { return; }
    // Fetch current id
    settings.deviceId = SettingsGetDeviceId();
    // If different, rewrite
    if (newId != settings.deviceId)
    {
        settings.deviceId = newId;
        WriteProgramPage(DEVICE_ID_ADDRESS, &settings.deviceId, sizeof(unsigned long));
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

    // Check voltage ok to program
    if (adcResult.batt < BATT_CHARGE_MIN_LOG) { return 0xffff; }

    // Read existing data to RAM
    ReadProgram(LOG_ADDRESS, scratchBuffer, 512);

    // Update value
    value = ((unsigned short *)scratchBuffer)[index];   // Retrieve
    if (value == 0xffff) { value = 0x0000; }            // If uninitialized
    value++;                                            // Increment
    ((unsigned short *)scratchBuffer)[index] = value;   // Store

    // Rewrite program memory from RAM
    WriteProgramPage(LOG_ADDRESS, scratchBuffer, 512);

    return value;
}


// Add a log entry
void SettingsAddLogEntry(unsigned short status, unsigned long timestamp, const char *message)
{
    int len, i;

    // Check voltage ok to program
    if (adcResult.batt < BATT_CHARGE_MIN_LOG) { return; }

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



// Get a config value
unsigned short SettingsGetConfigValue(unsigned int index)
{
    unsigned short value1 = 0xffff;
    unsigned short value2 = 0xffff;
    ReadProgram(SETTINGS_ADDRESS + (2 * index + 0) * sizeof(unsigned short), &value1, sizeof(unsigned short));
    ReadProgram(SETTINGS_ADDRESS + (2 * index + 1) * sizeof(unsigned short), &value2, sizeof(unsigned short));
    if (value1 == ~value2) { return value1; }
    return 0xffff;
}


// Set a config value
char SettingsSetConfigValue(unsigned int index, unsigned short value)
{
	// Short-cut if value is already set correctly
	unsigned short existing = SettingsGetConfigValue(index);
	if (existing == value) { return 1; }
	
    // Read existing data to RAM
    ReadProgram(SETTINGS_ADDRESS, scratchBuffer, 512);

    // Update value
    //oldValue = ((unsigned short *)scratchBuffer)[index];   // Retrieve
    ((unsigned short *)scratchBuffer)[2 * index + 0] =  value;   // Store value
    ((unsigned short *)scratchBuffer)[2 * index + 1] = ~value;   // Store second copy of inverted value

    // Rewrite program memory from RAM
    WriteProgramPage(SETTINGS_ADDRESS, scratchBuffer, 512);

    // Check for mismatch
    if (SettingsGetConfigValue(index) == value) { return 0; }

    return 1;
}

// Reset settings from ROM
void SettingsInitialize(void)
{
    // Clear all
    memset(&settings, 0, sizeof(settings));
    memset(&status, 0, sizeof(status));

    // Fixed settings
    //settings.deviceId = DEVICEID_UNSET;
    settings.deviceId = SettingsGetDeviceId();
#ifdef __DEBUG
    settings.deviceId = 0x1;
#endif    

    // Current settings
    settings.sessionId = 0x00000000;
#if 1
    settings.sessionId = (settings.deviceId != 0xffff) ? (10000L * settings.deviceId) : 0;
    settings.sessionId += (SettingsGetLogValue(LOG_VALUE_RESTART) % 10000);
#endif
    settings.sampleRate = ACCEL_DEFAULT_RATE;
#ifdef USE_BMI160_IMU  
    settings.gyroRange = 0;
#endif    
    settings.loggingStartTime = 0xffffffff;
    settings.loggingEndTime = 0xffffffff;
    settings.maximumSamples = 0;
    settings.debuggingInfo = DEFAULT_DEBUGGING;
    settings.lastChangeTime = 0;
    settings.timeZone = -1;
    settings.dataMode = FORMAT_SINGLE_FILE | FORMAT_CWA_UNPACKED;       // FORMAT_SINGLE_FILE | FORMAT_CWA_PACKED

    // Status
    status.attached = 0;
    status.initialBattery = adcResult.batt;

    // Status: Logging
    status.accelSequenceId = 0;
    status.gyroSequenceId = 0;
    status.events = 0;
    status.sampleCount = 0;
    status.lastSampledTicks = 0x0000;
    status.lastWrittenTicks = 0x0000;
    status.debugFlashCount = 0;

    // Status: attached
    status.batteryFull = 0;
    status.ledOverride = -1;
    status.diskMounted = 1;
    status.actionFlags = 0;
    status.actionCountdown = 0;
    status.stream = 0;

    // Configuration
    status.lockCode = 0x0000;
    status.dataEcc = CONFIG_ECC_DEFAULT;
    status.fractional = CONFIG_FRACTIONAL_DEFAULT;

    // "Initial lock" value
    {
        unsigned short code;
        code = SettingsGetConfigValue(CONFIG_LOCK);
        if (code != 0 && code != 0xffff)
        {
            status.lockCode = code;
            status.diskMounted = 0; // Disk will be unmounted at start
        }
    }

    // ECC
    {
        unsigned short value;
        value = SettingsGetConfigValue(CONFIG_ECC);
        if (value != 0xffff)
        {
            status.dataEcc = (char)value;
        }
    } 

    // Fractional
    {
        unsigned short value;
        value = SettingsGetConfigValue(CONFIG_FRACTIONAL);
        if (value != 0xffff)
        {
		    status.fractional = (char)value;
        }
    } 
    
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
        FSremove(DEFAULT_FILE);
    }

    if (flags & ACTION_FORMAT_WIPE || flags & ACTION_FORMAT_QUICK)
    {
        char wipe = 0, error = 0;
        static char volumeBuffer[13];
        volumeBuffer[0]  = HARDWARE_TYPE_0; // 'A';
        volumeBuffer[1]  = HARDWARE_TYPE_1; // 'X';
        volumeBuffer[2]  = HARDWARE_TYPE_2; // '3';
        volumeBuffer[3]  = '0' + ((HARDWARE_VERSION >> 4) & 0x0f);
        volumeBuffer[4]  = '0' + ((HARDWARE_VERSION     ) & 0x0f);
        volumeBuffer[5]  = '_';
        if (settings.deviceId == DEVICEID_UNSET)
        {
            volumeBuffer[6]  = '_';
            volumeBuffer[7]  = '_';
            volumeBuffer[8]  = '_';
            volumeBuffer[9]  = '_';
            volumeBuffer[10] = '_';
            volumeBuffer[11] = '\0';
        }
        else
        {
	        if (settings.deviceId > 99999ul)
			{
		        volumeBuffer[3]  = '_';
            	volumeBuffer[4]  = '0' + (settings.deviceId / 1000000ul) % 10;
            	volumeBuffer[5]  = '0' + (settings.deviceId /  100000ul) % 10;
			}
            volumeBuffer[6]  = '0' + (settings.deviceId /   10000) % 10;
            volumeBuffer[7]  = '0' + (settings.deviceId /    1000) % 10;
            volumeBuffer[8]  = '0' + (settings.deviceId /     100) % 10;
            volumeBuffer[9]  = '0' + (settings.deviceId /      10) % 10;
            volumeBuffer[10] = '0' + (settings.deviceId          ) % 10;
            volumeBuffer[11] = '\0';
        }

        LED_SET(LED_RED);
        if (flags & ACTION_FORMAT_WIPE) { wipe = 1; }
        error |= FsFtlFormat(wipe, settings.deviceId, volumeBuffer);
        
        if (!FSInit()) { error |= 0x80; }
        
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
        // Write settings to the logging binary file
        if (LoggerWriteMetadata(DEFAULT_FILE, 0))
        {
	        printf("COMMIT\r\n");
		} 
		else
		{
        	printf("ERROR: Problem during commit\n");
		}

/*
extern unsigned char FSerrno;
int i;
for (i = FSerrno; i > 0; i--) { LED_SET(LED_MAGENTA); DelayMs(1000); LED_SET(LED_GREEN); DelayMs(1000); }
*/

    }

    if (flags & ACTION_REMOUNT)
    {
        status.diskMounted = 1;
    }

    if (flags & ACTION_RESTART)
    {
        ret = 1;
    }

    return ret;
}


// Serial commands
char SettingsCommand(const char *line, SettingsMode mode)
{
    char locked = (mode == SETTINGS_USB) && (status.lockCode != 0x0000);
    if (line == NULL || strlen(line) == 0) { return 0; }
	if (mode == SETTINGS_USB && status.stream) { status.stream = 0; }

    if (mode != SETTINGS_BATCH && strnicmp(line, "help", 4) == 0)
    {
        //printf("HELP: help|echo|id|status|sample|time|hibernate|stop|standby|serial|debug|clear|device|session|rate|annotate|read|erase|led|reset\r\n");
        printf("HELP: help|echo|reset\r\n");
    }
    else if (strnicmp(line, "echo", 4) == 0)
    {
        int value = -1;
        if (line[5] != '\0') { value = (int)my_atoi(line + 5); }
        if (value == 0 || value == 1)
        {
            commEcho = value;
        }
        printf("ECHO=%d\r\n", commEcho);
    }
    else if (strnicmp(line, "id",2) == 0)
    {
        printf("ID=%c%c%c,%x,%u,%lu,%lu\r\n", HARDWARE_LABEL_0, HARDWARE_LABEL_1, HARDWARE_LABEL_2, HARDWARE_VERSION, SOFTWARE_VERSION, settings.deviceId, settings.sessionId);
    }
    else if (strnicmp(line, "sample", 6) == 0 && mode != SETTINGS_BATCH)
    {
        unsigned short v;
        char z = 0;
        if (line[6] != '\0')
        {
            z = line[7] - '0';
        }

        // Update ADC values
        if (z >= 0 && z <= 3)
        {
            AdcInit();
            if (!z || z == 2)
                AdcSampleWait();
            else
                AdcSampleNow();
        }

        if (!z || z == 1)
        {
            printf("$BATT=");
            v = adcResult.batt;
            printf("%u,%u,mV,%d,%d\r\n", v, AdcBattToMillivolt(v), AdcBattToPercent(v), (status.batteryFull >= BATT_FULL_INTERVAL) ? 1 : 0);
            if (!z) USBCDCWait();
        }

        if (!z || z == 2)
        {
            printf("$LDR=");
            v = adcResult.ldr;
            printf("%u,%u\r\n", v, AdcLdrToLux(v));
            if (!z) USBCDCWait();
        }

        if (!z || z == 3)
        {
            printf("$TEMP=");
            v = adcResult.temp;
            printf("%u,%d,0.1dC\r\n", v, AdcTempToTenthDegreeC(v));
            if (!z) USBCDCWait();
        }

        if (!z || z == 4)
        {
            unsigned short fractional;
            DateTime time;
            time = RtcNowFractional(&fractional);
            printf("$TIME=%s.%03d\r\n", RtcToString(time), RTC_FRACTIONAL_TO_MS(fractional));
            if (!z) USBCDCWait();
        }

        
        if (!z || z == 5)
        {
            accel_t sample;

            printf("$ACCEL=");
            AccelSingleSample(&sample);
            printf("%d,%d,%d\r\n", sample.x, sample.y, sample.z);
            if (!z) USBCDCWait();
        }

        if (!z || z == 5)
        {
#if defined(USE_GYRO) || defined(USE_BMI160_IMU) 
            if (gyroPresent)
            {
                gyro_t sample;
                GyroSingleSample(&sample);
                printf("$GYRO=%d,%d,%d\r\n", sample.x, sample.y, sample.z);
            }
            else
#endif
            { printf("$GYRO=\r\n"); }
            if (!z) USBCDCWait();
        }
    }
    else if (strnicmp(line, "status", 6) == 0)
    {
        char z = 0;
        if (line[6] != '\0')
        {
            z = line[7] - '0';
        }

        if (!z || z == 1)
        {
            printf("TEST=%04x\r\n", SelfTest());
            if (!z) USBCDCWait();
        }

        if (!z || z == 2)
        {
            printf("BATTHEALTH=%u\r\n", SettingsGetLogValue(LOG_VALUE_BATTERY));
            if (!z) USBCDCWait();
        }

        if (!z || z == 3)
        {
		    // FTL=warn#,erro#,spare,2(planes),bad-0,bad-1
            printf("FTL=%s\r\n", FtlHealth());
            if (!z) USBCDCWait();
        }

        if (!z || z == 4)
        {
            unsigned short checksum = 0;
// TODO: Firmware checksum -- avoiding writeable areas (+ bootloader?)
            printf("CHECKSUM=%04x\r\n", checksum);
            if (!z) USBCDCWait();
        }

        if (!z || z == 5)
        {
            printf("RESTART=%u\r\n", SettingsGetLogValue(LOG_VALUE_RESTART));
            if (!z) USBCDCWait();
        }

        if (!z || z == 6)
        {
		    // Compare NAND parameters agains compiled-in constants
            printf("NAND=%04x\r\n", FtlVerifyNandParameters());
            if (!z) USBCDCWait();
        }

        if (!z || z == 7)
        {
		    // Read NAND parameters
		    unsigned char id[6] = {0};
		    NandReadDevicedId(id);
            printf("NANDID=%02x:%02x:%02x:%02x:%02x:%02x,%d\r\n", id[0], id[1], id[2], id[3], id[4], id[5], nandPresent);
            if (!z) USBCDCWait();
        }

    }
// KL 2014: Selftest results
else if (strnicmp(line, "test_results", 12) == 0)
{
	unsigned short offset = 0, length, size;
	ReadProgram(SELF_TEST_ADDRESS, scratchBuffer, 512);
	length = strlen((char*)scratchBuffer);
	if(scratchBuffer[0] == '\f')
	{
		for(;;)
		{
			size = 64; // Printf limititations....
			if(size > (length - offset))size = length - offset;
			if(offset >= length)break;
			write(0,scratchBuffer+offset,size);
			USBCDCWait();
			offset += size;
		}
	}
	else
	{
		printf("Results not found\r\n");	
	}
	// Full re-test and wipe
	if(line[12] == '+' && line[13] == 'W')
	{
		// Schedule factory reset (re-test+format+log)
		WriteProgramPage(SELF_TEST_ADDRESS, "W", 1);
		printf("Wipe scheduled\r\n");
	}
	// Re-test periferals only
	if(line[12] == '+' && line[13] == 'R')
	{
		// Schedule factory reset (re-test+format+log)
		WriteProgramPage(SELF_TEST_ADDRESS, "R", 1);
		printf("Re-test scheduled\r\n");
	}
}
    else if (strnicmp(line, "time", 4) == 0)
    {
        unsigned long time;
        if (line[4] != '\0')
        {
            time = RtcFromString(line + 5);
            if (time != 0)
            {
                if (locked)
                {
                    printf("ERROR: Locked.\r\n");
                }
                else
                {
                    RtcWrite(time);
                    if (mode != SETTINGS_BATCH) printf("$TIME=%s\r\n", RtcToString(time));
                }
            }
            else
            {
                if (mode != SETTINGS_BATCH) printf("ERROR: Problem parsing time.\r\n");
            }
        }
        else
        {
            time = RtcNow();
            if (mode != SETTINGS_BATCH) printf("$TIME=%s\r\n", RtcToString(time));
        }
    }
    else if (strnicmp(line, "tzone", 5) == 0)
    {
        if (line[5] != '\0')
        {
            if (line[6] == '\0') { settings.timeZone = -1; }
            else if (locked) { printf("ERROR: Locked.\r\n"); }
            else { settings.timeZone = (int)my_atoi(line + 6); }
        }

        if (settings.timeZone == -1)
        {
            printf("TZONE=\r\n");
        }
        else
        {
            printf("TZONE=%d,%c%02d:%02u\r\n", settings.timeZone, settings.timeZone < 0 ? '-' : '+', abs(settings.timeZone) / 60, abs(settings.timeZone) % 60);
        }
    }
    else if (strnicmp(line, "hibernate", 9) == 0)
    {
        if (line[9] != '\0')
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else { settings.loggingStartTime = RtcFromString(line + 10); }
        }
        if (mode != SETTINGS_BATCH) printf("HIBERNATE=%s\r\n", RtcToString(settings.loggingStartTime));
    }
    else if (strnicmp(line, "stop", 4) == 0)
    {
        if (line[4] != '\0')
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else { settings.loggingEndTime = RtcFromString(line + 5); }
        }
        if (mode != SETTINGS_BATCH) printf("STOP=%s\r\n", RtcToString(settings.loggingEndTime));
    }
    else if (strnicmp(line, "lastchanged", 11) == 0)
    {
        if (mode != SETTINGS_BATCH) printf("LASTCHANGED=%s\r\n", RtcToString(settings.lastChangeTime));
    }
    else if (strnicmp(line, "maxsamples", 10) == 0)
    {
        if (line[10] != '\0')
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else { settings.maximumSamples = my_atoi(line + 11); }
        }
        if (mode != SETTINGS_BATCH) printf("MAXSAMPLES=%lu\r\n", settings.maximumSamples);
    }
    else if (strnicmp(line, "debug", 5) == 0)
    {
        if (line[5] != '\0')
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else { settings.debuggingInfo = my_atoi(line + 6); }
        }
        if (mode != SETTINGS_BATCH) printf("DEBUG=%d\r\n", settings.debuggingInfo);
    }
    else if (strnicmp(line, "session", 7) == 0)
    {
        if (line[7] != '\0')
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else { settings.sessionId = my_atoi(line + 8); }
        }
        if (mode != SETTINGS_BATCH) printf("SESSION=%lu\r\n", settings.sessionId);
    }
    else if (strnicmp(line, "rate", 4) == 0)
    {
        if (line[4] != '\0')
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else
            {
                settings.sampleRate = my_atoi(line + 4);
#ifdef USE_BMI160_IMU
                settings.gyroRange = 0;                     // Missing parameter will disable gyro
                const char* chPtr = &line[4];
                unsigned char offset = 0;
                // Increment to first none-digit upto N chars of integer + separator + first char of gyro range
                while(++offset <= 7)
                {
                    chPtr++;
                    if(*chPtr < 32)                         // No change to value, stop on ctrl code
                        break;
                    if((*chPtr >= '0') && (*chPtr <= '9'))  // Digit of 'sampleRate', skip
                        continue;
                    if(*chPtr == ',')chPtr++;               // Skip a comma
                    settings.gyroRange = my_atoi(chPtr);    // Not a digit/null, end of first integer - set gyroRange
                    break;
                }
#endif
            }
        }
#ifdef USE_BMI160_IMU        
        if (mode != SETTINGS_BATCH) printf("RATE=%u,%u,%u\r\n", settings.sampleRate, ACCEL_FREQUENCY_FOR_RATE(settings.sampleRate), settings.gyroRange);
#else        
        if (mode != SETTINGS_BATCH) printf("RATE=%u,%u\r\n", settings.sampleRate, ACCEL_FREQUENCY_FOR_RATE(settings.sampleRate));
#endif        
    }
    else if (strnicmp(line, "clear", 5) == 0)
    {
        char flags = 0;

        if (locked) { printf("ERROR: Locked.\r\n"); }
        else if (strnicmp(line, "clear data", 10) == 0)
        {
            flags |= ACTION_DELETE | ACTION_CREATE;	// ACTION_DELETE  ACTION_FORMAT_QUICK
        }
        else if (strnicmp(line, "clear all", 9) == 0)
        {
            SettingsInitialize();
            flags |= ACTION_FORMAT_WIPE | ACTION_CREATE;	// ACTION_DELETE 
        }

        if (flags != 0)
        {
            if (status.diskMounted && status.attached)
            {
                status.diskMounted = 0;
                status.actionFlags = flags | ACTION_REMOUNT;
                status.actionCountdown = 2;
                FSInit();
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
            printf("CLEAR: Use 'clear data' or 'clear all'.\r\n");
        }
    }
    else if (strnicmp(line, "commit", 6) == 0)
    {
        char flags;
        flags = ACTION_CREATE;
        if (locked) { printf("ERROR: Locked.\r\n"); }
        else if (status.diskMounted && status.attached)
        {
            status.diskMounted = 0;
            status.actionFlags = flags | ACTION_REMOUNT;
            status.actionCountdown = 1;
            FSInit();
            printf("COMMIT: Delayed activation.\r\n");
        }
        else
        {
            SettingsAction(flags);
        }
    }
    else if (strnicmp(line, "device", 6) == 0)
    {
        unsigned long id = DEVICEID_UNSET;
        if (line[6] != '\0')
	    { 
		    id = my_atoi(line + 6); 
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else { SettingsSetDeviceId(id); }
	    }    
        printf("DEVICE=%lu,%lu\r\n", settings.deviceId, SettingsGetDeviceId());
    }
    else if (strnicmp(line, "annotate", 8) == 0)
    {
        if (line[8] >= '0' && line[8] <= '9' && line[9] >= '0' && line[9] <= '9' && (line[10] == '\0' || line[10] == '='))
        {
            unsigned char id = (line[8] - '0') * 10 + (line[9] - '0');
            if (id < ANNOTATION_COUNT)    // 14x 32-byte annotation chunks
            {
                if (locked) { printf("ERROR: Locked.\r\n"); }
                else
                {
                    unsigned char j;
                    if (line[10] == '=')
                    {
                        unsigned short len = strlen(line + 11);
                        if (len > ANNOTATION_SIZE) { len = ANNOTATION_SIZE; }
                        memcpy(settings.annotation[id], line + 11, len);
                        memset(settings.annotation[id] + len, ' ', ANNOTATION_SIZE - len);
                    }

                    printf("ANNOTATE%02d=", id);
                    for (j = 0; j < 32; j++)
                    {
                        printf("%c", ((settings.annotation[id][j]) < 32 ? ' ' : settings.annotation[id][j]));
                    }
                    printf("\r\n");
                }
            }
            else
            {
                printf("ERROR: Invalid annotation id %d\r\n", id);
            }
        }
        else
        {
            printf("ERROR: Usage is ANNOTATE##=\r\n");
        }
    }
    else if (strnicmp(line, "readl", 5) == 0)
    {
        unsigned long sector = 0xffffffff;
        if (line[5] != '\0') { sector = my_atoi(line + 6); }

        if (locked) { printf("ERROR: Locked.\r\n"); }
        else if (sector != 0xffffffff)
        {
            if (!FtlReadSector(sector, scratchBuffer))
            {
                // Check the buffer contents against expected manual test/write patterns
                unsigned int i;
                unsigned char lc = scratchBuffer[0], sequential = 1, identical = 1;
                for (i = 0; i < MEDIA_SECTOR_SIZE; i++)
                {
                    if (scratchBuffer[i] != lc) { identical = 0; }
                    if (scratchBuffer[i] != (unsigned char)i) { sequential = 0; }
                }
                
                printf("ERROR: Problem reading logical sector %lu\r\n", sector);
                printhexdump(scratchBuffer, 0, MEDIA_SECTOR_SIZE);
                printf("ERROR%s\r\n", identical ? ",IDENTICAL" : (sequential ? ",SEQUENTIAL" : ""));
            }
            else
            {
                // Check the buffer contents against expected manual test/write patterns
                unsigned int i;
                unsigned char lc = scratchBuffer[0], sequential = 1, identical = 1;
                for (i = 0; i < MEDIA_SECTOR_SIZE; i++)
                {
                    if (scratchBuffer[i] != lc) { identical = 0; }
                    if (scratchBuffer[i] != (unsigned char)i) { sequential = 0; }
                }
                
                printf("READL=%lu\r\n", sector);
                printhexdump(scratchBuffer, 0, MEDIA_SECTOR_SIZE);
                printf("OK%s\r\n", identical ? ",IDENTICAL" : (sequential ? ",SEQUENTIAL" : ""));
            }
        }
        else
        {
            printf("READ: Specify logical sector\r\n");
        }
    }
    else if (strnicmp(line, "writel", 6) == 0)
    {
        unsigned long sector = 0xffffffff;
        int value = -1;
        if (line[6] != '\0')
        {
            char *sep = strpbrk(line + 7, " ");
            if (sep != NULL) { *sep = '\0'; value = my_atoi(sep + 1);  }
            sector = my_atoi(line + 7);
        }
        if (locked) { printf("ERROR: Locked.\r\n"); }
        else if (sector != 0xffffffff)
        {
			unsigned int i;
			for (i = 0; i < MEDIA_SECTOR_SIZE; i++) { scratchBuffer[i] = (value < 0) ? (unsigned char)i : (unsigned char)value; }
			if (!FtlWriteSector(sector, scratchBuffer, 1))
            {
                printf("ERROR: Problem writing logical sector %lu\r\n", sector);
            }
            else
            {
                printf("WRITEL=%lu\r\n", sector);
            }
        }
        else
        {
            printf("WRITE: Specify logical sector\r\n");
        }
    }
    else if (strnicmp(line, "readraw", 7) == 0)
    {
        unsigned long sector = 0xffffffff;
        if (line[7] != '\0') { sector = my_atoi(line + 8); }

        if (locked) { printf("ERROR: Locked.\r\n"); }
        else if (sector != 0xffffffff)
        {
            if (!FtlReadRawSector(sector, scratchBuffer))	// An FTL-safe raw read
            {
                printf("ERROR: Problem reading physical sector %lu\r\n", sector);
            }
            else
            {
                // Check the buffer contents against expected manual test/write patterns
                unsigned int i;
                unsigned char lc = scratchBuffer[0], sequential = 1, identical = 1;
                for (i = 0; i < MEDIA_SECTOR_SIZE; i++)
                {
                    if (scratchBuffer[i] != lc) { identical = 0; }
                    if (scratchBuffer[i] != (unsigned char)i) { sequential = 0; }
                }

                printf("READRAW=%lu\r\n", sector);
                printhexdump(scratchBuffer, 0, MEDIA_SECTOR_SIZE);
                printf("OK%s\r\n", identical ? ",IDENTICAL" : (sequential ? ",SEQUENTIAL" : ""));
            }
        }
        else
        {
            printf("READRAW: Specify raw sector\r\n");
        }
    }
#ifdef NAND_BLOCK_MARK
    else if (strnicmp(line, "FILEBLOCK", 9) == 0)
    {
        unsigned long offset = 0xffffffff;
        if (line[9] != '\0')
        {
            offset = my_atoi(line + 10);
        }

        if (locked) { printf("ERROR: Locked.\r\n"); }
        else if (offset != 0xffffffff)
        {
			unsigned long logicalSector;			
			unsigned short physicalBlock = 0xFFFF;
			unsigned char page;	
			unsigned char sectorInPage;
			
			// Calculate block, page, and sector in page from the file offset
			logicalSector = FSFileOffsetToLogicalSector(DEFAULT_FILE, offset);

			// If the block was found... 
			if (logicalSector > 0 && logicalSector < 0xFFFFFFFFul)
			{
				// Translate a logical sector address to a physical block address (and, the physical page address and sector-in-page offset)
				if (FtlTranslateLogicalSectorToPhysical(logicalSector, &physicalBlock, &page, &sectorInPage))
				{
					printf("FILEBLOCK=%u\r\n", physicalBlock);
				}
				else
				{
					printf("ERROR: Problem finding the physical block for the logical sector.\r\n");
				}
			}
			else
			{
				printf("ERROR: Problem finding the logical sector for the file offset.\r\n");
			}
			
        }
        else
        {
            printf("FILEBLOCK: Data file offset not specified.\r\n");
        }
    }
    else if (strnicmp(line, "BADBLOCK", 8) == 0)
    {
        unsigned short physicalBlock = 0xffff;
        if (line[8] != '\0')
        {
            physicalBlock = (unsigned short)my_atoi(line + 9);
        }

        if (locked) { printf("ERROR: Locked.\r\n"); }
        else if (physicalBlock != 0xffff)
        {
			// Relocate the block contents and mark the specified physical block as bad
			char ret = FtlRelocatePhysicalBlockAndMarkBad(physicalBlock);
			if (ret)
			{
				if (ret == -1)
				{
					// Already marked as bad
					printf("BADBLOCK=%u,1\r\n", physicalBlock);
				}
				else
				{
					// Successfully marked as bad
					printf("BADBLOCK=%u\r\n", physicalBlock);
				}
			}
			else
			{
				printf("ERROR: Problem marking block as bad.\r\n");
			}
			
        }
        else
        {
            printf("BADBLOCK: Physical block not specified.\r\n");
        }
    }
#endif
#ifdef RAW_NAND_DEBUG
    else if (strnicmp(line, "writeraw", 8) == 0)
    {
        unsigned long sector = 0xffffffff;
        int value = -1;
        if (line[8] != '\0')
        {
            char *sep = strpbrk(line + 9, " ");
            if (sep != NULL) { *sep = '\0'; value = my_atoi(sep + 1);  }
            sector = my_atoi(line + 9);
        }

        if (locked) { printf("ERROR: Locked.\r\n"); }
        else if (sector != 0xffffffff)
        {
			// Calculate block, page, and sector in page from the full sector address
			unsigned short block = (unsigned short)(sector >> FTL_SECTOR_TO_BLOCK_SHIFT);
			unsigned char page = (unsigned char)(sector >> FTL_SECTOR_TO_PAGE_SHIFT) & (unsigned char)((1 << (FTL_SECTOR_TO_BLOCK_SHIFT - FTL_SECTOR_TO_PAGE_SHIFT)) - 1);	
			unsigned char sectorInPage = (unsigned char)sector & ((1 << FTL_SECTOR_TO_PAGE_SHIFT) - 1);
			unsigned int i;
			for (i = 0; i < MEDIA_SECTOR_SIZE; i++) { scratchBuffer[i] = (value < 0) ? (unsigned char)i : (unsigned char)value; }
			// Directly write to the physical block
			NandLoadPageWrite(block, page, block, page);
			NandWriteBuffer(sectorInPage * MEDIA_SECTOR_SIZE, scratchBuffer, MEDIA_SECTOR_SIZE);
			if (!NandStorePage())
			{
	            printf("ERROR: Problem writing raw sector %lu\r\n", sector);
	  		}
	  		else
	  		{
	            printf("WRITERAW=%lu\r\n", sector);
	            printf("OK, but the FTL state is unspecified and the disk will probably need a FORMAT\r\n");
	  		}		          
        }
        else
        {
            printf("WRITERAW: Specify raw sector [VERY DANGEROUS!]\r\n");
        }
    }
    else if (strnicmp(line, "eraseraw", 8) == 0)
    {
        unsigned long sector = 0xffffffff;
        if (line[8] != '\0') { sector = my_atoi(line + 9); }

        if (locked) { printf("ERROR: Locked.\r\n"); }
        else if (sector != 0xffffffff)
        {
			// Calculate block from the full sector address
			unsigned short block = (unsigned short)(sector >> FTL_SECTOR_TO_BLOCK_SHIFT);
			if (!NandEraseBlock(block))
			{
                printf("ERROR: Problem erasing physical block %u for raw sector %lu\r\n", block, sector);
	  		}
	  		else
	  		{
	            printf("ERASE=%lu,%u\r\n", sector, block);
	            printf("OK, but the FTL state is unspecified and the disk will probably need a FORMAT\r\n");
	  		}
        }
        else
        {
            printf("ERASE: Specify a raw sector in the block to erase [VERY DANGEROUS!]\r\n");
        }
    }
#endif
    else if (strnicmp(line, "standby", 7) == 0) { printf("STANDBY=0\r\n"); }
    else if (strnicmp(line, "stream", 6) == 0)
    {
        if (locked) { printf("ERROR: Locked.\r\n"); }
        else
        {
            // Enable streaming
            status.stream = 1;
        }
    }
    else if (strnicmp(line, "datamode", 8) == 0)
    {
        const char *s;
        if(line[8]!='\0')
        {
            int i = my_atoi(line + 9);
            if (i != -1)
            {
                if (locked) { printf("ERROR: Locked.\r\n"); }
                else { settings.dataMode = (unsigned char)i; }
            }
        }
        s = "None";
        if ((settings.dataMode & FORMAT_MASK_TYPE) == FORMAT_CSV) s = "CSV";
        else if ((settings.dataMode & FORMAT_MASK_TYPE) == FORMAT_CWA_PACKED) s = "CWA/packed";
        else if ((settings.dataMode & FORMAT_MASK_TYPE) == FORMAT_CWA_UNPACKED) s = "CWA/unpacked";
        printf("DATAMODE=%u,%s,%s\r\n", settings.dataMode, (settings.dataMode & FORMAT_SINGLE_FILE) ? "single-file" : "auto-number", s);
    }
    else if (strnicmp(line, "mount", 5) == 0)
    {
        int value = -1;
        if (line[5] != '\0') { value = (int)my_atoi(line + 6); }
        if (value == 0 || value == 1)
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else
            {
	            FtlFlush(0);
                status.diskMounted = value;
                // If we unmount and are not in a batch file
                if (value == 0 && mode != SETTINGS_BATCH)
                {
                    FSInit();       // re-init the filesystem
                }
            }
        }
        printf("MOUNT=%d\r\n", status.diskMounted);
    }
    else if (strnicmp(line, "led", 3) == 0)
    {
        if (line[3] != '\0')
        {
            int value = -1;
            value = (int)my_atoi(line + 4);
            status.ledOverride = value;
        }
        if (mode != SETTINGS_BATCH) printf("LED=%d\r\n", status.ledOverride);
    }
    else if (mode != SETTINGS_BATCH && strnicmp(line, "format", 6) == 0)
    {
        if (locked) { printf("ERROR: Locked.\r\n"); }
        else if (line[6] != '\0' && (line[7] == 'Q' || line[7] == 'q' || line[7] == 'W' || line[7] == 'w' || line[7] == 'D' || line[7] == 'd'))
        {
            if (line[7] == 'd' || line[7] == 'D')
            {
                if (line[8] == '\0') { printf("FORMAT: Destroy.\r\n"); FtlDestroy(0); }
                else if (line[8] == 'B') { printf("FORMAT: Destroy, including user-marked bad blocks.\r\n"); FtlDestroy(1); }
#ifdef NAND_BLOCK_MARK
                //else if (line[8] == '!') { printf("FORMAT: Destroy, including all bad blocks (do not do this).\r\n"); FtlDestroy(42); }   // this is a bad idea
#endif
            }
            else
            {
                char wipe = (line[7] == 'W' || line[7] == 'w') ? 1 : 0;
                char flags = 0;
                if (wipe) { flags |= ACTION_FORMAT_WIPE; }
                else { flags |= ACTION_FORMAT_QUICK; }

                if (line[8] == 'C' || line[8] == 'c') { flags |= ACTION_CREATE; } // Create data file with current settings

                if (status.diskMounted && status.attached)
                {
                    status.diskMounted = 0;
                    status.actionFlags = flags | ACTION_REMOUNT;
                    status.actionCountdown = 2;
                    FSInit();
                    printf("FORMAT: Delayed activation.\r\n");
                }
                else
                {
                    SettingsAction(flags);
                }
            }
        }
        else
        {
            printf("ERROR: Must specify 'FORMAT Q' for quick or 'FORMAT W' for wipe, +'C' for COMMIT.\r\n");
        }
    }
    else if (strnicmp(line, "logfile", 7) == 0)
    {
        if (!status.diskMounted || !status.attached)
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else if (line[7] != '\0')
            {
                FSFILE *fp;
                fp = FSfopen(LOG_FILE, "a");
                if (fp != NULL)
                {
                    unsigned short fractional;
                    DateTime time;
                    time = RtcNowFractional(&fractional);
                    FSfprintf(fp, "%s.%03d,%s\r\n", RtcToString(time), RTC_FRACTIONAL_TO_MS(fractional), line + 8);
                    FSfclose(fp);
                }
            }
        }
        else
        {
            printf("ERROR: Cannot while attached and mounted.\r\n");
        }
    }
    else if (strnicmp(line, "log", 3) == 0)
    {
        if (locked) { printf("ERROR: Locked.\r\n"); }
        else if (line[3] != '\0')
        {
	        unsigned long timestamp = RtcNow();
	        printf("LOG,%d,0x%04x,%s,%s\r\n", 0, LOG_CATEGORY_USER, RtcToString(timestamp), line + 4);
            SettingsAddLogEntry(LOG_CATEGORY_USER, timestamp, line + 4);
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
    }
    else if (strnicmp(line, "lock", 4) == 0)
    {
        unsigned int i = 0xffff;

        if ((line[4] == ':' || line[4] == '=' || line[4] == ' ') && (line[5] >= '0' && line[5] <= '9') && strlen(line) <= 10) // Slightly more careful parsing than usual
        {
            i = (unsigned int)my_atoi(line + 5);
        }

        if (i != 0xffff)
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else
            {
                status.lockCode = i;

                if (mode != SETTINGS_BATCH)
                {
                    unsigned int lockStatus = 0;
                    unsigned short iCode = SettingsGetConfigValue(CONFIG_LOCK);
                    if (status.lockCode != 0x0000) { lockStatus += 1; }
                    if (iCode != 0 && iCode != 0xffff) { lockStatus += 2; }
                    printf("LOCK=%d\r\n", lockStatus);
                }
            }
        }
        else
        {
            unsigned int lockStatus = 0;
            unsigned short iCode = SettingsGetConfigValue(CONFIG_LOCK);
            if (status.lockCode != 0x0000) { lockStatus += 1; }
            if (iCode != 0 && iCode != 0xffff) { lockStatus += 2; }
            printf("LOCK=%d\r\n", lockStatus);
        }
    }
    else if (strnicmp(line, "ilock", 5) == 0 && (mode != SETTINGS_BATCH))
    {
        unsigned int i = 0xffff;

        if ((line[5] == ':' || line[5] == '=' || line[5] == ' ') && (line[6] >= '0' && line[6] <= '9') && strlen(line) <= 11) // Slightly more careful parsing than usual
        {
            i = (unsigned int)my_atoi(line + 6);
        }

        if (i != 0xffff)
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else
            {
                SettingsSetConfigValue(CONFIG_LOCK, i);
                if (i == 0) printf("ILOCK=0\r\n"); else printf("ILOCK=1\r\n");
            }
        }
        else
        {
            unsigned short code = SettingsGetConfigValue(CONFIG_LOCK);
            if (code == 0 || code == 0xffff) printf("ILOCK=0\r\n"); else printf("ILOCK=1\r\n");
        }
    }
    else if (strnicmp(line, "unlock", 6) == 0)
    {
        if (line[6] != '\0')
        {
            unsigned int i = (unsigned int)my_atoi(line + 7);
            if (locked && i != status.lockCode)
            {
                DelayMs(1000);
                printf("ERROR: Locked.\r\n");
            }
            else
            {
                if (status.lockCode != 0x0000)
                {
                    unsigned short code = SettingsGetConfigValue(CONFIG_LOCK);
                    status.lockCode = 0x0000;
    
                    // If we were initially locked, mount the drive
                    if (code != 0 && code != 0xffff && !status.diskMounted)
                    {
        	            FtlFlush(0);
                        status.diskMounted = 1;
                    }
                }

                if (mode != SETTINGS_BATCH) printf("LOCK=0\r\n");
            }
        }
    }
    else if (strnicmp(line, "ecc", 3) == 0)
    {
        if (line[3] != '\0')
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else
            {
                unsigned int i = (unsigned int)my_atoi(line + 4);
                status.dataEcc = i;
                if (mode != SETTINGS_BATCH) { SettingsSetConfigValue(CONFIG_ECC, status.dataEcc); }
                printf("ECC=%d\r\n", status.dataEcc);
            }
        }
        else
        {
            printf("ECC=%d\r\n", status.dataEcc);
        }
    }
    else if (strnicmp(line, "fractional", 10) == 0)
    {
        if (line[10] != '\0')
        {
            if (locked) { printf("ERROR: Locked.\r\n"); }
            else
            {
                unsigned int i = (unsigned int)my_atoi(line + 4);
                status.fractional = i;
                if (mode != SETTINGS_BATCH) { SettingsSetConfigValue(CONFIG_FRACTIONAL, status.fractional); }
                printf("FRACTIONAL=%d\r\n", status.fractional);
            }
        }
        else
        {
        	printf("FRACTIONAL=%d\r\n", status.fractional);
        }
    }
    else if (mode != SETTINGS_BATCH && strnicmp(line, "reset", 5) == 0)
    {
        printf("RESET\r\n");
        printf("OK\r\n");
        status.actionCountdown = 2;
        status.actionFlags = ACTION_RESTART;
    }
    else if (strnicmp(line, "exit", 4) == 0)
    {
        if (mode != SETTINGS_BATCH) printf("INFO: You'll have to close the terminal window yourself!\r\n");
    }
    else if (strnicmp(line, "AT", 2) == 0)
    {
        if (mode != SETTINGS_BATCH) printf("OK\r\n");
    }
    else if (line[0] == '\0')
    {
        if (mode != SETTINGS_BATCH) printf("\r\n");
    }
    else
    {
        if (mode != SETTINGS_BATCH) printf("ERROR: Unknown command: %s\r\n", line);
        return 0;
    }
    return 1;
}

