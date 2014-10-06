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
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "Settings.h"
#include "Analog.h"
#include "Utils/Util.h"
#include "Peripherals/Rtc.h"
#include "USB/USB_CDC_MSD.h"
#include "Scales.h"
// GSM tranceiver
#include "TelitModule.h"
#include "Peripherals/Uart_vdma.h"

extern unsigned char runGsmModule;
extern char* FormDebugMessage(void);
extern char* FormNPRIMessage(void);

// These addresses must match the reserved sections of program memory in the linker script
//ROM BYTE __attribute__ ((address(DEVICE_ID_ADDRESS),space(prog),aligned(ERASE_BLOCK_SIZE),section(".device_id"),noload)) DeviceIdData[ERASE_BLOCK_SIZE];
//ROM BYTE __attribute__ ((address(LOG_ADDRESS),      space(prog),aligned(ERASE_BLOCK_SIZE),section(".log" ),     noload)) LogData[ERASE_BLOCK_SIZE];
//ROM BYTE __attribute__ ((address(SETTINGS_ADDRESS), space(prog),aligned(ERASE_BLOCK_SIZE),section(".settings" ),noload)) SettingsData[ERASE_BLOCK_SIZE];

unsigned char scratchBuffer[512];
Settings settings = {0};
Status status = {0};

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

char* SettingsGetDefaultNumber(void)
{
    // Fetch current id
	unsigned short i;
	static char number[32];
	ReadProgram(GSM_PHONE_NUMBER_ADDRESS, number, 31);
	if((number[0] != '0')&&(number[0] != '+'))		// Number doesn't start with 0 or +
	{
		settings.gsmDefaultNumber = NULL;
		return settings.gsmDefaultNumber;
	}
	for(i=1;i<32;i++) // Validate number is ok
	{
		if (number[i] == '\0') {break;}
		if ((number[i] > '9' || number[i] < '0') ||	// Non numeric number detected
			(i>24))									// Too many numbers detected
		{
			settings.gsmDefaultNumber = NULL;
			return settings.gsmDefaultNumber;
		}
	}
	settings.gsmDefaultNumber = number;
	return settings.gsmDefaultNumber;
}


void SettingsSetDefaultNumber(char* newNumber)
{
    // Fetch current id
    char* number = SettingsGetDefaultNumber();
	unsigned short len = strlen(newNumber) + 1; // write null too
	
    // If different, rewrite
	if ((number == NULL) || (strncmp(number,newNumber,24)))
	{
        WriteProgramPage(GSM_PHONE_NUMBER_ADDRESS, newNumber, len);
		// Keeps a RAM pointer to the string (otherwise we will have PSV problems trying to use non-psv rom)
		SettingsGetDefaultNumber(); 
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

// HACK: Always write -- something wrong with settings?    
if (settings.deviceId == 0xffff || settings.deviceId  == 0x0000) { settings.deviceId = INITIAL_DEVICE_ID; }


	// Other settings
	settings.config = 0;
	settings.destination = 0;
    
#endif

	settings.gsmDefaultNumber = SettingsGetDefaultNumber();

    // Current settings
    //settings.XXX = 0;
    
    // Status
    status.ledOverride = 0xFF;
	status.usb = 0;
}



// Serial commands
char SettingsCommand(const char *line)
{
    if (line == NULL || strlen(line) == 0) { return 0; }

    if (strnicmp(line, "help", 4) == 0)
    {
        printf("HELP: help|echo|reset\r\n");
    }
    else if (strnicmp(line, "sample", 6) == 0)
    {
        char z = 0;
        if (line[6] != '\0')
        {
            z = line[7] - '0';
        }

		// Update ADC
		AdcSampleWait();
		
        if (!z || z == 1) { printf("$BATT=%u,%s V,%d\r\n",  adcResult.batt,     fixed_itoa(AdcToMillivolt(2*adcResult.batt), 3), AdcBattToPercent(adcResult.batt)); }
        if (!z || z == 4)
        {
            unsigned short fractional;
            DateTime time;
            time = RtcNowFractional(&fractional);
            printf("$TIME=%s.%03d\r\n", RtcToString(time), RTC_FRACTIONAL_TO_MS(fractional));
            if (!z) USBCDCWait();
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
                RtcWrite(time);
                printf("$TIME=%s\r\n", RtcToString(time));
            }
            else
            {
                printf("ERROR: Problem parsing time.\r\n");
            }
        }
        else
        {
            time = RtcNow();
            printf("$TIME=%s\r\n", RtcToString(time));
        }
    }
    else if (strnicmp(line, "device", 6) == 0)
    {
        unsigned int id = 0xffff;
        if (line[6] != '\0')
	    { 
		    id = (unsigned int)my_atoi(line + 6); 
            SettingsSetDeviceId(id);
	    }
		printf("DEVICE=%u,%u\r\n", settings.deviceId, SettingsGetDeviceId());
		printf("HW=%s, SW=%s\r\n", HARDWARE_VERSION, SOFTWARE_VERSION);
    }
    else if (strnicmp(line, "number", 6) == 0)
    {
		if (line[6] != '\0')
		{
            SettingsSetDefaultNumber((char*)line+7);
	    }
		SettingsGetDefaultNumber();
		if (settings.gsmDefaultNumber == NULL)
		{
			printf("NO VALID NUMBER SET\r\n");
		}
		printf("NUMBER=%s\r\n", settings.gsmDefaultNumber);
    }
    else if (strnicmp(line, "led", 3) == 0)
    {
        if (line[3] != '\0')
        {
            int value = -1;
            value = (int)my_atoi(line + 4);
            status.ledOverride = value;
        }
		printf("LED=%d\r\n", status.ledOverride);
    }
    else if (strnicmp(line, "log", 3) == 0)
    {
        if (line[3] != '\0')
        {
	        unsigned long timestamp = 0;
//	        timestamp = RtcNow();
			printf("LOG,%d,0x%04x,%s,%s\r\n", 0, LOG_CATEGORY_USER, "0" /*RtcToString(timestamp)*/, line + 4);
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
					printf("LOG,%d,0x%04x,%s,%s\r\n", index, status, "0" /*RtcToString(timestamp)*/, value);
                }
            }
            printf("OK\r\n");
        }
    }
    else if (strnicmp(line, "reset", 5) == 0)
    {
        printf("RESET\r\n");
        printf("OK\r\n");
        Reset();
    }
    else if (strnicmp(line, "gsmpwr", 6) == 0)
    {
        if ((runGsmModule == FALSE) || (gsmState.machineState == GSM_OFF))
		{	gsmState.machineState = INIT_STATE;
				gsm_printf_debug = TRUE;
				printf("GSM ON\r\n");runGsmModule = TRUE;
		}
		else
		{	
			gsmState.machineStateQueue = GSM_SHUT_DOWN;
			gsmState.result = -1;
			//gsm_printf_debug = FALSE; // If this is set false you wont see pwr off ack
			printf("GSM OFF\r\n");
			runGsmModule = TRUE;
		}
	}
    else if (strnicmp(line, "gsmsms", 6) == 0)
    {
		// Just for testing
        if (runGsmModule == TRUE)
		{
			if(line[6] == '1')	
			{
				FormNPRIMessage();
			}
			else
			{
				FormDebugMessage();
			}
			if (gsmState.phoneNumber != NULL){
				printf("Sending sms\r\n");
				gsmState.machineStateQueue = GSM_SEND_SMS;
			}
			else
				printf("Number not set\r\n");
		}
		else
			{printf("GSM IS OFF\r\n");}		
	}
    else if (strnicmp(line, "gsmcall", 6) == 0)
    {
		// Just for testing
        if (runGsmModule == TRUE)
		{
			gsmState.phoneNumber = SettingsGetDefaultNumber();
			printf("Calling %s\r\n",gsmState.phoneNumber);
			gsmState.machineStateQueue = GSM_MAKE_CALL;
		}
		else
			{printf("GSM IS OFF\r\n");}		
	}
    else if (strnicmp(line, "gsmend", 6) == 0)
    {
		// Just for testing
        if (runGsmModule == TRUE)
		{
			printf("Hanging up\r\n");
			gsmState.machineStateQueue = GSM_END_CALL;
		}
		else
			{printf("GSM IS OFF\r\n");}		
	}
    else if (strnicmp(line, "exit", 4) == 0)
    {
        printf("INFO: You'll have to close the terminal window yourself!\r\n");
    }
//    else if (strnicmp(line, "AT", 2) == 0)
//    {
//        printf("OK\r\n");
//    }
    else if (line[0] == '\0')
    {
        printf("\r\n");
    }
    else
    {
		printf("ERROR: Unknown command: %s\r\n", line);
        return 0;
    }
    return 1;
}

