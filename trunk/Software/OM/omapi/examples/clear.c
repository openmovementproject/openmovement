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

/**
 *  @file clear.c
 *  @brief     Open Movement API Example: Clear all attached devices.
 *  @author    Dan Jackson
 *  @date      2011
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2011, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  A command-line tool to clear all attached devices.
 *  This example uses OmGetDeviceIds() to get a list of attached devices.
 *  Only devices attached at start-up will be cleared, then the utility will exit.
 *  
 *  @remarks Makes use of \ref api_init, \ref settings, \ref return_codes
 */


/* Headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* API header */
#include "omapi.h"


/* Clear function */
int clear(void)
{
    int numDevices;
    int *deviceIds;
    int result;
    int i;

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

        /* Set the LED to red */
        result = OmSetLed(deviceId, OM_LED_RED);
        if (OM_FAILED(result)) { printf("WARNING: OmSetLed() %s\n", OmErrorString(result)); }

        /* Zero the session id */
        printf("CLEARING #%d: SessionId = 0...\n", deviceId);
        result = OmSetSessionId(deviceId, 0); 
        if (OM_FAILED(result)) { printf("WARNING: OmSetSessionId() %s\n", OmErrorString(result)); }

        /* Clear the metadata */
        printf("CLEARING #%d: Metadata = <clear>...\n", deviceId);
        result = OmSetMetadata(deviceId, NULL, 0); 
        if (OM_FAILED(result)) { printf("WARNING: OmSetMetadata() %s\n", OmErrorString(result)); }

        /* Disable logging */
        printf("CLEARING #%d: Delays = <never-log>...\n", deviceId);
        result = OmSetDelays(deviceId, OM_DATETIME_INFINITE, OM_DATETIME_INFINITE); 
        if (OM_FAILED(result)) { printf("WARNING: OmSetDelays() %s\n", OmErrorString(result)); }

        /* Clear the data and commit the settings */
        printf("CLEARING #%d: Clear data and commit...\n", deviceId);
        result = OmClearDataAndCommit(deviceId); 
        if (OM_FAILED(result)) 
        { 
            printf("WARNING: OmClearDataAndCommit() %s\n", OmErrorString(result));
        }
        else
        {
            /* Set the LED to green */
            result = OmSetLed(deviceId, OM_LED_GREEN);
            if (OM_FAILED(result)) { printf("WARNING: OmSetLed() %s\n", OmErrorString(result)); }
        }

    }

    /* Free our list of device ids */
    free(deviceIds);

    /* Shutdown the API */
    result = OmShutdown();
    if (OM_FAILED(result)) { printf("ERROR: OmShutdown() %s\n", OmErrorString(result)); return -1; }

    return 0;
}


/* Main function */
int clear_main(int argc, char *argv[])
{
    printf("CLEAR: clear all attached devices.\n");
    printf("\n");
    if (argc > 1 && strcmp(argv[1], "ALL") == 0)
    {
        return clear();
    }
    else
    {
        printf("Usage: clear ALL\n");
        printf("\n");
        printf("Where: ALL: is a case-sensitive word 'ALL' to confirm clearing all attached devices.\n");
        printf("\n");
        printf("Example: clear ALL\n");
        printf("\n");
    }
    return -1;
}

