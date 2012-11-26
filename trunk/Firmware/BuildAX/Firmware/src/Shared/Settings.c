/* 
 * Copyright (c) 2012, Newcastle University, UK.
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
// Dan Jackson, 2012

// Includes
#include <Compiler.h>
#include "HardwareProfile.h"
#include "Settings.h"
#include "WirelessProtocols/Console.h"
#include "Analogue.h"
#include "Peripherals/Humidicon.h"
#include "Network.h"


// These addresses must match the reserved sections of program memory in the linker script
//ROM BYTE __attribute__ ((address(DEVICE_ID_ADDRESS),space(prog),aligned(ERASE_BLOCK_SIZE),section(".device_id"),noload)) DeviceIdData[ERASE_BLOCK_SIZE];
//ROM BYTE __attribute__ ((address(LOG_ADDRESS),      space(prog),aligned(ERASE_BLOCK_SIZE),section(".log" ),     noload)) LogData[ERASE_BLOCK_SIZE];
//ROM BYTE __attribute__ ((address(SETTINGS_ADDRESS), space(prog),aligned(ERASE_BLOCK_SIZE),section(".settings" ),noload)) SettingsData[ERASE_BLOCK_SIZE];


Settings settings = {0};
Status status = {0};

static unsigned char scratchBuffer[512];

#define consoleprintf(...) { sprintf((char *)scratchBuffer, __VA_ARGS__); Printf((const char*)scratchBuffer); }


// Get the device id
unsigned short SettingsGetDeviceId(void)
{
    unsigned short id = 0xffff;
    ReadProgram(DEVICE_ID_ADDRESS, &id, sizeof(unsigned short));
    return id;
}


// Rewrite the device id
void SettingsSetDeviceId(unsigned short newId)
{
    // Fetch current id
    unsigned short id = SettingsGetDeviceId();
    // If different, rewrite
    if (newId != id)
    {
        WriteProgramPage(DEVICE_ID_ADDRESS, &newId, sizeof(unsigned short));
    }
    settings.deviceId = newId;
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
    //if (adcResult.batt < BATT_CHARGE_MIN_LOG) { return 0xffff; }

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
    //if (adcResult.batt < BATT_CHARGE_MIN_LOG) { return; }

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
    return (const char *)(scratchBuffer + (1 + index) * LOG_SIZE + sizeof(unsigned short) + sizeof(unsigned long));
}


// Get a config value
unsigned short SettingsGetConfigValue(unsigned int index)
{
    unsigned short value = 0xffff;
    ReadProgram(SETTINGS_ADDRESS + index * sizeof(unsigned short), &value, sizeof(unsigned short));
    return value;
}

// Set a config value
unsigned short SettingsSetConfigValue(unsigned int index, unsigned short value)
{
    // Read existing data to RAM
    ReadProgram(SETTINGS_ADDRESS, scratchBuffer, 512);

    // Update value
    ((unsigned short *)scratchBuffer)[index] = value;   // Store

    // Rewrite program memory from RAM
    WriteProgramPage(SETTINGS_ADDRESS, scratchBuffer, 512);

    return SettingsGetConfigValue(index);
}




// Reset settings from ROM
void SettingsInitialize(void)
{
    // Clear all
    memset(&settings, 0, sizeof(settings));
    memset(&status, 0, sizeof(status));

    // Fixed settings
    settings.deviceId = SettingsGetDeviceId();
    
    // Correct uninitialized device ID
#ifdef COORD
    if (settings.deviceId != 0x0000) { SettingsSetDeviceId(0x0000); }
#elif defined(INITIAL_DEVICE_ID)
    if (settings.deviceId == 0xffff) { SettingsSetDeviceId(INITIAL_DEVICE_ID); }
#endif


	// Check if settings are valid
	{
		unsigned short configMagic;
		unsigned char version = 0;
		
		configMagic = SettingsGetConfigValue(SETTING_INDEX_VALID);
		
		if ((configMagic & 0x00ff) == 0x00cc)
		{
			version = (unsigned char)(configMagic >> 8);
		}
		
		if (version != CONFIG_VERSION)
		{
			// Yuck, these churn the program memory (luckily only once unless re-flashed) -- redo the way it works to just program the whole structure instead
			SettingsSetConfigValue(SETTING_INDEX_CONFIG, 0x0000);
			SettingsSetConfigValue(SETTING_INDEX_DESTINATION, 0x0000);
			SettingsSetConfigValue(SETTING_INDEX_CHANNEL_MASK, CHANNEL_MASK_DEFAULT);
			SettingsSetConfigValue(SETTING_INDEX_DATA_INTERVAL, DEFAULT_DATA_INTERVAL);
			SettingsSetConfigValue(SETTING_INDEX_TRANSMIT_THRESHOLD, DEFAULT_TRANSMIT_THRESHOLD);
			SettingsSetConfigValue(SETTING_INDEX_AUDIO_GAIN, DEFAULT_AUDIO_GAIN);
		}
		
		if (version != CONFIG_VERSION)
		{
			SettingsSetConfigValue(SETTING_INDEX_VALID, 0x00cc | (CONFIG_VERSION << 8));
		}	
	}	

	// Other settings
	settings.config = SettingsGetConfigValue(SETTING_INDEX_CONFIG);
	settings.destination = SettingsGetConfigValue(SETTING_INDEX_DESTINATION);
	settings.channelmask = SettingsGetConfigValue(SETTING_INDEX_CHANNEL_MASK);
	settings.dataInterval = SettingsGetConfigValue(SETTING_INDEX_DATA_INTERVAL);
    if (settings.dataInterval < 1) { settings.dataInterval = 1; }
    if (settings.dataInterval > DATA_MAX_INTERVAL) { settings.dataInterval = DATA_MAX_INTERVAL; }
	settings.transmitThreshold = SettingsGetConfigValue(SETTING_INDEX_TRANSMIT_THRESHOLD);
    if (settings.transmitThreshold < 1) { settings.transmitThreshold = 1; }
    if (settings.transmitThreshold > MAX_PACKETS - 1) { settings.transmitThreshold = MAX_PACKETS - 1; }
	settings.audioGain = SettingsGetConfigValue(SETTING_INDEX_AUDIO_GAIN);

    // Current settings
    //settings.XXX = 0;
    
    // Status
    status.ledOverride = 0xFF;
	status.usb = 0;
	status.lowpower = 0;
    status.noconnect = 0;
}



// Serial commands
char SettingsCommand(const char *line)
{
    if (line == NULL || strlen(line) == 0) { return 0; }

    if (strnicmp(line, "help", 4) == 0)
    {
        consoleprintf("HELP: help|id|sample|device|target|channel|transmit|interval|gain\r\n");
    }
/*
    else if (strnicmp(line, "echo", 4) == 0)
    {
        int value = -1;
        if (line[5] != '\0') { value = (int)my_atoi(line + 5); }
        if (value == 0 || value == 1)
        {
            commEcho = value;
        }
        consoleprintf("ECHO=%d\r\n", commEcho);
    }
*/
    else if (strnicmp(line, "id",2) == 0)
    {
		consoleprintf(
			"ID="
			#ifdef COORD
				"TEDDI-C"
			#elif defined(ROUTER)
				"TEDDI-R"
			#elif defined(SENSOR)
				"TEDDI-S"
			#else
				"TEDDI-?"
			#endif
			",%x,%u,%u\r\n", HARDWARE_VERSION, SOFTWARE_VERSION, settings.deviceId);
    }
    else if (strnicmp(line, "sample", 6) == 0)
    {
        unsigned short v;
        char z = 0;
        if (line[6] != '\0')
        {
            z = line[7] - '0';
        }

#ifndef SENSOR
		// Update ADC
		AdcInit(ADC_CLOCK_SYSTEM_PLL);
		AdcSampleWait();
		
		reading.light = adcResult.light;
		reading.battery = adcResult.batt;
		reading.pir[0] = adcResult.pir;
		reading.audio[0] = adcResult.mic_ave;
		reading.temp = adcResult.temp;	// (old)
		
		// Read humidity/temperature
        if (!z || z == 8 || z == 9)
        {
			Humidicon_measurement_request();
		    DelayMs(1);				// TODO: What delay is required?
			Humidicon_read();
			
			reading.humidity = gHumResults.humidity;
			reading.temp = gHumResults.temperature;
		}
#else
        if (!z) { consoleprintf("[old]\r\n"); }
#endif
		
        if (!z || z == 1) { consoleprintf("$BATT=%u,%s,V,%d\r\n",  reading.battery,     fixed_itoa(AdcToMillivolt(reading.battery), 3), AdcBattToPercent(reading.battery)); }
        
        // AdcLdrToLux - convert ADC reading to log10(lux) * 10^3  -->  lux = pow(10.0, log10LuxTimes10Power3 / 1000.0);

        
        if (!z || z == 2) { consoleprintf("$LIGHT=%u,%s,log10(lux)\r\n", reading.light, fixed_itoa(AdcLdrToLux(reading.light), 3)); }
#ifndef SENSOR
		if (!z || z == 3) { consoleprintf("$TEMP2=%u,%s,^C\r\n",   reading.temp,     fixed_itoa(AdcTempToTenthDegreeC(reading.temp), 1)); }
#endif
        if (!z || z == 4) { consoleprintf("$PIR=%u,%s,V\r\n",      reading.pir[0],      fixed_itoa(AdcToMillivolt(reading.pir[0]), 3)); }
        if (!z || z == 5) { consoleprintf("$MIC_AVE=%u,%s,V\r\n",  reading.audio[0],    fixed_itoa(AdcToMillivolt(reading.audio[0]), 3)); }
#ifndef SENSOR
        if (!z || z == 6) { consoleprintf("$MIC_PEAK=%u,%s,V\r\n", adcResult.mic_peak, fixed_itoa(AdcToMillivolt(adcResult.mic_peak), 3)); }
        if (!z || z == 7) { consoleprintf("$MIC_RAW=%u,%s,V\r\n",  adcResult.mic_raw,  fixed_itoa(AdcToMillivolt(adcResult.mic_raw), 3)); }
#endif
        if (!z || z == 8 || z == 9)
        {
	        if (!z || z == 8) { consoleprintf("$TEMP=%u,%s,^C,%d\r\n",     reading.temp,        fixed_itoa(Humidicon_Convert_centiC(reading.temp), 2), gHumResults.status); }
	        if (!z || z == 9) { consoleprintf("$HUMIDITY=%u,%s,%%,%d\r\n", reading.humidity,    fixed_itoa(Humidicon_Convert_percentage(reading.humidity), 2), gHumResults.status); }
        }

    }
	/*
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
                    consoleprintf("ERROR: Locked.\r\n");
                }
                else
                {
                    RtcWrite(time);
                    if (mode != SETTINGS_BATCH) consoleprintf("$TIME=%s\r\n", RtcToString(time));
                }
            }
            else
            {
                if (mode != SETTINGS_BATCH) consoleprintf("ERROR: Problem parsing time.\r\n");
            }
        }
        else
        {
            time = RtcNow();
            if (mode != SETTINGS_BATCH) consoleprintf("$TIME=%s\r\n", RtcToString(time));
        }
    }
	*/
    else if (strnicmp(line, "device", 6) == 0)
    {
        unsigned int id = 0xffff;
        if (line[6] != '\0')
	    { 
		    id = (unsigned int)my_atoi(line + 6); 
            SettingsSetDeviceId(id);
	    }
		consoleprintf("DEVICE=%u,%u\r\n", settings.deviceId, SettingsGetDeviceId());
    }
    else if (strnicmp(line, "led", 3) == 0)
    {
        if (line[3] != '\0')
        {
            int value = -1;
            value = (int)my_atoi(line + 4);
            status.ledOverride = value;
        }
		consoleprintf("LED=%d\r\n", status.ledOverride);
    }
    else if (strnicmp(line, "log", 3) == 0)
    {
        if (line[3] != '\0')
        {
	        unsigned long timestamp = 0;
//	        timestamp = RtcNow();
			consoleprintf("LOG,%d,0x%04x,%s,%s\r\n", 0, LOG_CATEGORY_USER, "0" /*RtcToString(timestamp)*/, line + 4);
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
					consoleprintf("LOG,%d,0x%04x,%s,%s\r\n", index, status, "0" /*RtcToString(timestamp)*/, value);
                }
            }
            consoleprintf("OK\r\n");
        }
    }
	else if (strnicmp(line, "target", 6) == 0)
	{
		unsigned short id = 0xffff;
		if (line[6] != '\0') { id = (unsigned short)my_atoi(line + 7); }
		if (id != 0xffff)
		{
			settings.destination = SettingsSetConfigValue(SETTING_INDEX_DESTINATION, id);
		}
		printf("TARGET=%u\r\n", settings.destination);
	}
	else if (strnicmp(line, "channel", 7) == 0)
	{
		// Set mask from list of channels
		if (line[7] != '\0')
		{
			unsigned short channel = 0x0000;
			int val = -1;
			const char *p;
			for (p = line + 7; ; p++)
			{
				if (*p >= '0' && *p <= '9')
				{
					if (val < 0) { val = 0; }
					val = val * 10 + (*p - '0');
				}
				else
				{
					if (val >= 0)
					{
						channel |= (1 << (val - 11));
						val = -1;
					}
					if (*p == '\0') { break; }
				}
			}
			settings.channelmask = SettingsSetConfigValue(SETTING_INDEX_CHANNEL_MASK, channel);
		}
		
		// Display mask from list of channels
		{
			int numchans = 0;
			int channel;
			printf("CHANNEL=");
			for (channel = 11; channel <= 26; channel++)
			{
				if ((settings.channelmask >> (channel - 11)) & 1)
				{
					if (numchans) { printf(","); }
					printf("%u", channel);
					numchans++;
				}
			}	
			printf("\r\n");
		}	
	}
	else if (strnicmp(line, "transmit", 8) == 0)
	{
		unsigned short value = 0xffff;
		if (line[8] != '\0') { value = (unsigned short)my_atoi(line + 9); }
		if (value != 0xffff)
		{
		    if (value < 1) { value = 1; }
		    if (value > MAX_PACKETS - 1) { value = MAX_PACKETS - 1; }
			settings.transmitThreshold = SettingsSetConfigValue(SETTING_INDEX_TRANSMIT_THRESHOLD, value);
		}
		printf("TRANSMIT=%u\r\n", settings.transmitThreshold);
	}
	else if (strnicmp(line, "interval", 8) == 0)
	{
		unsigned short value = 0xffff;
		if (line[8] != '\0') { value = (unsigned short)my_atoi(line + 9); }
		if (value != 0xffff)
		{
		    if (value < 1) { value = 1; }
		    if (value > DATA_MAX_INTERVAL) { value = DATA_MAX_INTERVAL; }
			settings.dataInterval = SettingsSetConfigValue(SETTING_INDEX_DATA_INTERVAL, value);
		}
		printf("INTERVAL=%u\r\n", settings.dataInterval);
	}
	else if (strnicmp(line, "gain", 4) == 0)
	{
		unsigned short value = 0xffff;
		if (line[4] != '\0') { value = (unsigned short)my_atoi(line + 5); }
		if (value != 0xffff)
		{
		    if (value < 5) { value = 1; }
		    else if (value < 50) { value = 10; }
		    else { value = 100; }
			settings.audioGain = SettingsSetConfigValue(SETTING_INDEX_AUDIO_GAIN, value);
			SettingsUpdateGain();
		}
		printf("GAIN=%u\r\n", settings.audioGain);
	}
	else if (strnicmp(line, "connect", 7) == 0)
	{
	    status.noconnect = 0;
		printf("CONNECT=1\r\n");
	}
    else if (strnicmp(line, "reset", 5) == 0)
    {
        consoleprintf("RESET\r\n");
        consoleprintf("OK\r\n");
        Reset();
    }
    else if (strnicmp(line, "exit", 4) == 0)
    {
        consoleprintf("INFO: You'll have to close the terminal window yourself!\r\n");
    }
    else if (strnicmp(line, "AT", 2) == 0)
    {
        consoleprintf("OK\r\n");
    }
    else if (line[0] == '\0')
    {
        consoleprintf("\r\n");
    }
    else
    {
		consoleprintf("ERROR: Unknown command: %s\r\n", line);
        return 0;
    }
    return 1;
}


void SettingsUpdateGain(void)
{
	// Set audio gain
	if (settings.audioGain < 5) { AUDIO_GAIN_1(); }
	else if (settings.audioGain < 50) { AUDIO_GAIN_10(); }
	else { AUDIO_GAIN_100(); }
}	
