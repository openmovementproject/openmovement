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
// Dan Jackson, 2011

// Includes
#include "HardwareProfile.h"
#include <Compiler.h>
#include "Analog.h"
#include "Peripherals/Rtc.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Rtc.h"
#include "Usb/USB_CDC_MSD.h"
#include "Utils/Util.h"
#include "Utils/Fifo.h"
#include "Settings.h"


unsigned char scratchBuffer[512];
Settings settings = {0};
Status status = {0};

// Reset settings from ROM
void SettingsInitialize(void)
{
    // Clear all
    memset(&settings, 0, sizeof(settings));
    memset(&status, 0, sizeof(status));
    
    // Status
    status.attached = 0;
    status.initialBattery = adcResult.batt;

    // Status: attached
    status.lockCode = 0x0000;
    status.batteryFull = 0;
    status.ledOverride = -1;
    status.actionFlags = 0;
    status.actionCountdown = 0;
    status.stream = 0;

	// Bluetooth
	settings.btEnable = 0;
	status.bt_run = 0;
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
    else if (strnicmp(line, "sample", 6) == 0 && mode != SETTINGS_BATCH)
    {
		unsigned short v;
		AdcInit();
		AdcSampleWait();

        {
            unsigned short fractional;
            DateTime time;
            time = RtcNowFractional(&fractional);
            printf("$TIME=%s.%03d\r\n", RtcToString(time), RTC_FRACTIONAL_TO_MS(fractional));
            USBCDCWait();
        }

        printf("$BATT=");
        v = adcResult.batt;
        printf("%u,%u,mV,%d,%d\r\n", v, AdcBattToMillivolt(v), AdcBattToPercent(v), status.batteryFull);
        USBCDCWait();

        printf("$LDR=");
        v = adcResult.ldr;
        printf("%u,%u\r\n", v, AdcLdrToLux(v));
		USBCDCWait();

        printf("$IR1=");
        v = adcResult.ir1;
        printf("%u\r\n", v);
		USBCDCWait();

        printf("$IR2=");
        v = adcResult.ir2;
        printf("%u\r\n", v);
		USBCDCWait();

		{
            accel_t sample;
            printf("$ACCEL=");
            AccelSingleSample(&sample);
            printf("%d,%d,%d\r\n", sample.x, sample.y, sample.z);
            USBCDCWait();
        }

		{
			gyro_t sample;
			GyroSingleSample(&sample);
			printf("$GYRO=%d,%d,%d\r\n", sample.x, sample.y, sample.z);
			USBCDCWait();
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
    else if (strnicmp(line, "stream", 6) == 0)
    {
        // Enable streaming
        status.stream = 1;
    }
    else if (strnicmp(line, "btrun", 5) == 0)
    {
		if (settings.btEnable)
		{
	        // Enable bluetooth
			printf("BT on\r\n");
	        status.bt_run = 1;
		}
		else
		{
			printf("BT disabled!\r\n");		
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

