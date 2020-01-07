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

/**
 *  @file clear.c
 *  @brief     Open Movement API Example: Clear all attached devices.
 *  @author    Dan Jackson
 *  @date      2011-2012
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2012, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  A command-line tool to clear all attached devices.
 *  This example uses OmGetDeviceIds() to get a list of attached devices.
 *  Only devices attached at start-up will be cleared, then the utility will exit.
 *  
 *  @remarks Makes use of \ref api_init, \ref settings, \ref return_codes
 */


/* Headers */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* API header */
#include "omapi.h"

#define ALL_DEVICES -42

/* Clear function */
int clear(int activeDeviceId, int doClear, int timeSync, int checkVersion)
{
    int numDevices;
    int *deviceIds;
    int result;
    int i;
	int retValue = 0;

    /* Start the API */
    result = OmStartup(OM_VERSION);
    if (OM_FAILED(result)) { printf("ERROR: OmStartup() %s\n", OmErrorString(result)); return -1; }

    /* Query the current number of devices attached */
    result = OmGetDeviceIds(NULL, 0);
    if (OM_FAILED(result)) { printf("ERROR: OmGetDeviceIds() %s\n", OmErrorString(result)); return -1; }
    numDevices = result;

    /* Get the currently-attached devices ids */
    deviceIds = (int *)malloc(numDevices * sizeof(int));
    result = OmGetDeviceIds(deviceIds, numDevices);
    if (OM_FAILED(result)) { printf("ERROR: OmGetDeviceIds() %s\n", OmErrorString(result)); return -1; }
    /* Cope with fewer devices being returned (if some were just removed).
     * If more devices were just attached, we'll ignore them for now.
     * (Production code wouldn't typically use OmGetDeviceIds but would instead work asynchronously with OmDeviceCallback) */
    if (result < numDevices) { numDevices = result; } 

    /* For each device currently connected... */
    for (i = 0; i < numDevices; i++)
    {
        int deviceId = deviceIds[i];
		int errorCount = 0;
		int flaggedDevice = 0;

		if (deviceId != activeDeviceId && activeDeviceId != ALL_DEVICES) continue;

		if (doClear || timeSync)
		{
			/* Set the LED to red */
			result = OmSetLed(deviceId, OM_LED_RED);
			if (OM_FAILED(result)) { errorCount++; printf("WARNING: OmSetLed() %s\n", OmErrorString(result)); }
		}

		if (checkVersion)
		{
			/* Check hardware and firmware versions */
			int firmwareVersion = -1;
			int hardwareVersion = -1;
			int deviceType = -1;			// "AX#" device (0=AX3, 6=AX6)
			result = OmGetVersion(deviceId, &firmwareVersion, &hardwareVersion);
			deviceType = hardwareVersion >> 16;
			hardwareVersion &= 0xffff;
			if (OM_FAILED(result)) { errorCount++; printf("ERROR: OmGetVersion() %s\n", OmErrorString(result)); }
			if (deviceType == 0 && (firmwareVersion == 42)) flaggedDevice = 1;
			if (deviceType == 6 && (firmwareVersion < 56)) flaggedDevice = 1;
			printf("CLEARING #%d: type=%d, firmware=%d, hardware=%d, flagged=%d\n", deviceId, deviceType, firmwareVersion, hardwareVersion, flaggedDevice);
		}

		if (doClear)
		{
			/* Zero the session id */
			printf("CLEARING #%d: SessionId = 0...\n", deviceId);
			result = OmSetSessionId(deviceId, 0);
			if (OM_FAILED(result)) { errorCount++; printf("WARNING: OmSetSessionId() %s\n", OmErrorString(result)); }

			/* Clear the metadata */
			printf("CLEARING #%d: Metadata = <clear>...\n", deviceId);
			result = OmSetMetadata(deviceId, NULL, 0);
			if (OM_FAILED(result)) { errorCount++; printf("WARNING: OmSetMetadata() %s\n", OmErrorString(result)); }

			/* Disable logging */
			printf("CLEARING #%d: Delays = <never-log>...\n", deviceId);
			result = OmSetDelays(deviceId, OM_DATETIME_INFINITE, OM_DATETIME_INFINITE);
			if (OM_FAILED(result)) { errorCount++; printf("WARNING: OmSetDelays() %s\n", OmErrorString(result)); }

			/* Configure the sample rate to the defaults */
			printf("CLEARING #%d: AccelConfig = <defaults>...\n", deviceId);
			result = OmSetAccelConfig(deviceId, OM_ACCEL_DEFAULT_RATE, OM_ACCEL_DEFAULT_RANGE);
			if (OM_FAILED(result)) { errorCount++; printf("WARNING: OmSetAccelConfig() %s\n", OmErrorString(result)); }
		}

		/* Synchronize the time */
		if (timeSync)
		{
			time_t now;
			struct tm* tm;
			OM_DATETIME nowTime;

			/* Get the current time */
			now = time(NULL);
			tm = localtime(&now);
			nowTime = OM_DATETIME_FROM_YMDHMS(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

			printf("CLEARING #%d: Synchronizing the time...\n", deviceId);
			result = OmSetTime(deviceId, nowTime);
			if (OM_FAILED(result)) { errorCount++; printf("ERROR: OmSetTime() %s\n", OmErrorString(result)); }
		}

		if (doClear)
		{
			/* Clear the data and commit the settings */
			printf("CLEARING #%d: Clear data and commit...\n", deviceId);
			result = OmEraseDataAndCommit(deviceId, OM_ERASE_QUICKFORMAT);
			if (OM_FAILED(result)) { errorCount++; printf("WARNING: OmEraseDataAndCommit() %s\n", OmErrorString(result)); }
		}
        
		if (errorCount)
		{
			/* Set the LED to blue */
			result = OmSetLed(deviceId, OM_LED_BLUE);
			retValue = 1;
		}
		else if (flaggedDevice)
		{
			/* Set the LED to cyan if flagged */
			result = OmSetLed(deviceId, OM_LED_CYAN);
			retValue = 42;
		}
		else
		{
			/* Set the LED to magenta if all ok */
			result = OmSetLed(deviceId, OM_LED_MAGENTA);
			retValue = 0;
		}

    }

    /* Free our list of device ids */
    free(deviceIds);

    /* Shutdown the API */
    result = OmShutdown();
    if (OM_FAILED(result)) { printf("ERROR: OmShutdown() %s\n", OmErrorString(result)); return -1; }

    return retValue;
}


/* Main function */
int clear_main(int argc, char *argv[])
{
	int deviceId = 0;
	int doClear = 1;
	int timeSync = 0;
	int checkVersion = 0;
	int i;
	int positional = 0;
	int error = 0;

    printf("CLEAR: clear all attached devices.\n");
    printf("\n");

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (strcmp(argv[i], "-timesync") == 0) { timeSync = 1; }
			else if (strcmp(argv[i], "-noclear") == 0) { doClear = 0; }
			else if (strcmp(argv[i], "-checkversion") == 0) { checkVersion = 1; }
			else
			{
				printf("ERROR: Unknown option: %s\n", argv[i]);
				error = 1;
				break;
			}
		}
		else
		{
			if (positional == 0)
			{
				deviceId = atoi(argv[i]);
				if (strcmp(argv[i], "ALL") == 0)
				{
					deviceId = ALL_DEVICES;
				}
			}
			else
			{
				printf("ERROR: Unknown positional argument #%d: %s\n", positional + 1, argv[i]);
				error = 1;
				break;
			}
			positional++;
		}
	}
	
	if (deviceId == 0)
	{
		printf("ERROR: Device (or 'all' devices) not specified\n");
		error = 1;
	}

	if (error)
    {
        printf("Usage: clear [-timesync] [-noclear] [-checkversion] [<id>|ALL]\n");
        printf("\n");
		printf("Where:\n");
		printf("        id:  device id to clear\n");
		printf("        ALL: case-sensitive word to confirm clearing all attached devices\n");
		printf("\n");
        printf("Example: clear ALL\n");
        printf("\n");
		return -1;
    }

	return clear(deviceId, doClear, timeSync, checkVersion);
}

