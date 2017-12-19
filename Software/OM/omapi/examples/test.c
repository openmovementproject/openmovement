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
 *  @file test.c
 *  @brief     Open Movement API Example: Self-test all connected devices.
 *  @author    Dan Jackson
 *  @date      2011-2012
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2012, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  A command-line tool to self-test all connected devices; controls the LED to indicate any failures.
 *  This example uses OmGetDeviceIds() to get a list of attached devices.
 *  Only devices attached at start-up will be cleared, then the utility will exit.
 *
 *  @remarks Makes use of \ref api_init, \ref device_status, \ref return_codes
 */


/* Headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* API header */
#include "omapi.h"


static int testDevice(int deviceId)
{
	int firmwareVersion = -1, hardwareVersion = -1;
	int accelerometer[3];
	int errors = 0;
	int result;

	/* Set the LED to blue to indicate testing */
	result = OmSetLed(deviceId, OM_LED_BLUE);
	if (OM_FAILED(result)) { printf("WARNING: OmSetLed() %s\n", OmErrorString(result)); }

	/* Check hardware and firmware versions */
	result = OmGetVersion(deviceId, &firmwareVersion, &hardwareVersion);
	if (OM_FAILED(result)) { printf("WARNING: OmGetVersion() %s\n", OmErrorString(result)); errors++; }
	printf("CHECK #%d: Firmware %d, Hardware %d\n", deviceId, firmwareVersion, hardwareVersion);

	/* Check battery level */
	result = OmGetBatteryLevel(deviceId);
	if (OM_FAILED(result)) { printf("WARNING: OmGetBatteryLevel() %s\n", OmErrorString(result)); errors++; }
	else
	{
		printf("CHECK #%d: Battery at %d%% (%s)\n", deviceId, result, (result >= 100) ? "charged" : "charging");
	}

	/* Get accelerometer readings */
	result = OmGetAccelerometer(deviceId, &accelerometer[0], &accelerometer[1], &accelerometer[2]);
	if (OM_FAILED(result)) { printf("WARNING: OmGetAccelerometer() %s\n", OmErrorString(result)); errors++; }
	else
	{
		printf("CHECK #%d: Accelerometer at (x = %d, y = %d, z = %d)\n", deviceId, accelerometer[0], accelerometer[1], accelerometer[2]);
	}

	/* Check self-test */
	result = OmSelfTest(deviceId);
	if (OM_FAILED(result)) { printf("WARNING: OmSelfTest() %s\n", OmErrorString(result)); errors++; }
	else
	{
		if (result == 0)
		{
			printf("CHECK #%d: Self-test: OK\n", deviceId);
		}
		else
		{
			errors++;
			printf("CHECK #%d: Self-test: FAILED (diagnostic 0x%04x)\n", deviceId, result);
		}
	}

	/* Check memory health */
	result = OmGetMemoryHealth(deviceId);
	if (OM_FAILED(result)) { printf("WARNING: OmGetMemoryHealth() %s\n", OmErrorString(result)); errors++; }
	else
	{
		if (result <= OM_MEMORY_HEALTH_ERROR)
		{
			errors++;
			printf("CHECK #%d: Memory health: FAILED (at least one plane has (or is near to having) no free blocks)\n", deviceId);
		}
		else if (result <= OM_MEMORY_HEALTH_WARNING)
		{
			printf("CHECK #%d: Memory health: WARNING (only %d free blocks on worst plane)\n", deviceId, result);
		}
		else
		{
			printf("CHECK #%d: Memory health: OK (at least %d free blocks on each plane)\n", deviceId, result);
		}
	}

	/* Check battery health */
	result = OmGetBatteryHealth(deviceId);
	if (OM_FAILED(result)) { printf("WARNING: OmGetBatteryHealth() %s\n", OmErrorString(result)); errors++; }
	else
	{
		if (result > 500)
		{
			printf("CHECK #%d: Battery health: NOTICE (%d cycles)\n", deviceId, result);
		}
		else
		{
			printf("CHECK #%d: Battery health: OK (%d cycles)\n", deviceId, result);
		}
	}

	/* Set the LED to WHITE if successful, or RED otherwise */
	if (errors > 0)
	{
		result = OmSetLed(deviceId, OM_LED_RED);
	}
	else
	{
		result = OmSetLed(deviceId, OM_LED_WHITE);
	}
	if (OM_FAILED(result)) { printf("WARNING: OmSetLed() %s\n", OmErrorString(result)); }

	return 0;
}


/* Device updated */
static void deviceCallback(void *reference, int deviceId, OM_DEVICE_STATUS status)
{
    if (status == OM_DEVICE_CONNECTED)
    {
        printf("TEST #%d: Device CONNECTED\n", deviceId);
		testDevice(deviceId);
    }
    else if (status == OM_DEVICE_REMOVED)
    {
        printf("TEST #%d: Device REMOVED\n", deviceId);
    }
    else
    {
        printf("TEST #%d: Error, unexpected status %d\n", deviceId, status);
    }
    return;
}


/* Device test function */
int test(char wait)
{
    int numDevices;
    int *deviceIds;
	int result;
	int i;

	if (wait)
	{
		/* Set device callback before API startup to get initially-connected devices through the callback */
		OmSetDeviceCallback(deviceCallback, NULL);

		printf("Waiting for devices...\n");
	}
	
    /* Start the API */
    result = OmStartup(OM_VERSION);
    if (OM_FAILED(result)) { printf("ERROR: OmStartup() %s\n", OmErrorString(result)); return -1; }

	if (!wait)
	{
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
			printf("TEST #%d: Device already CONNECTED\n", deviceIds[i]);
			testDevice(deviceIds[i]);
		}

		/* Free our list of device ids */
		free(deviceIds);
	}
	else
	{
		/* Block this thread waiting for a console key-press to terminate the downloader.
		 * The callbacks will allow any existing and future devices connected to have their 
		 * data downloaded simultaneously. */
		getchar();
		printf("Key pressed, shutting down...\n");
	}

    /* Shutdown the API */
    result = OmShutdown();
    if (OM_FAILED(result)) { printf("ERROR: OmShutdown() %s\n", OmErrorString(result)); return -1; }

    return 0;
}


/* Main function */
int test_main(int argc, char *argv[])
{
	char wait = 0;
    printf("TEST: self-test all connected devices.\n");
    printf("\n");
	if (argc > 1 && !strcmp(argv[1], "-wait")) { wait = 1; }
    return test(wait);
}

