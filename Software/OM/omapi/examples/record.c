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
 *  @file record.c
 *  @brief     Open Movement API Example: Batch clear and set devices to record.
 *  @author    Dan Jackson
 *  @date      2012
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2012, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  A command-line tool to batch setup all connected devices to record, time synchronization.
 *  Session identifiers is set to the device id.
 *  The pairing of session identifiers and their assigned device identifiers is written to an output file (if specified).
 *  The time is synchronized with the PC at setup time.
 *
 *  @remarks Makes use of \ref api_init, \ref device_status, \ref settings, \ref return_codes, \ref datetime
 */


/* Headers - need a sleep function to wait */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(millis) sleep((millis) / 1000)
#endif
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* API header */
#include "omapi.h"

static FILE *ofp = NULL;


int record_setup(int deviceId)
{
    int result;
    char timeString[32];
    time_t now;
    struct tm *tm;
    OM_DATETIME nowTime;
    int firmwareVersion, hardwareVersion;
    int battery;

    /* Check hardware and firmware versions */
    result = OmGetVersion(deviceId, &firmwareVersion, &hardwareVersion);
    if (OM_FAILED(result)) { printf("ERROR: OmGetVersion() %s\n", OmErrorString(result)); return 0; }
    printf("RECORD #%d: Firmware %d, Hardware %d\n", deviceId, firmwareVersion, hardwareVersion);

    /* Check battery level */
    battery = OmGetBatteryLevel(deviceId);
    if (OM_FAILED(battery)) { printf("ERROR: OmGetBatteryLevel() %s\n", OmErrorString(battery)); return 0; }
    else
    {
        printf("RECORD #%d: Battery at %d%% (%s)\n", deviceId, battery, (battery >= 100) ? "charged" : "charging");
    }

    /* Synchronize the time */
    now = time(NULL);
    tm = localtime(&now);
    nowTime = OM_DATETIME_FROM_YMDHMS(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    fprintf(stderr, "RECORD #%d: Synchronizing the time...\n", deviceId);
    result = OmSetTime(deviceId, nowTime);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmSetTime() %s\n", OmErrorString(result)); return 0; }

    /* Set the accelerometer configuration */
    result = OmSetAccelConfig(deviceId, OM_ACCEL_DEFAULT_RATE, OM_ACCEL_DEFAULT_RANGE);
    fprintf(stderr, "RECORD #%d: Setting accelerometer configuration...\n", deviceId);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmSetAccelConfig() %s\n", OmErrorString(result)); return 0; }

    /* Set the session id (use the deviceId) */
    result = OmSetSessionId(deviceId, deviceId);
    fprintf(stderr, "RECORD #%d: Setting session id: %u\n", deviceId, deviceId);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmSetSessionId() %s\n", OmErrorString(result)); return 0; }

    /* Set the delayed start/stop times */
    fprintf(stderr, "RECORD #%d: Setting start/stop: ZERO/INFINITY\n", deviceId);
    result = OmSetDelays(deviceId, OM_DATETIME_ZERO, OM_DATETIME_INFINITE);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmSetDelays() %s\n", OmErrorString(result)); return 0; }

    /* Commit the new settings */
    fprintf(stderr, "RECORD #%d: Committing new settings...\n", deviceId);
    result = OmEraseDataAndCommit(deviceId, OM_ERASE_QUICKFORMAT);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmEraseDataAndCommit() %s\n", OmErrorString(result)); return 0; }

    /* The device is ready for recording */
    fprintf(stderr, "RECORD,%u,%s,%d,%d\n", deviceId, timeString, battery, firmwareVersion);
    printf("RECORD,%u,%s,%d,%d\n", deviceId, timeString, battery, firmwareVersion);
    if (ofp != NULL)
    {
        fprintf(ofp, "RECORD,%u,%s,%d,%d\n", deviceId, timeString, battery, firmwareVersion);
        fflush(ofp);
    }

    /* The device is prepared for recording - set the LED to magenta. */
    OmSetLed(deviceId, OM_LED_MAGENTA);

    return 1;
}


/* Device updated */
static void record_DeviceCallback(void *reference, int deviceId, OM_DEVICE_STATUS status)
{
    if (status == OM_DEVICE_CONNECTED)
    {
        fprintf(stderr, "RECORD #%d: Device CONNECTED\n", deviceId);
        
        /* Setup */
        record_setup(deviceId);

    }
    else if (status == OM_DEVICE_REMOVED)
    {
        fprintf(stderr, "RECORD #%d: Device REMOVED\n", deviceId);
    }
    else
    {
        fprintf(stderr, "RECORD #%d: Error, unexpected status %d\n", deviceId, status);
    }
    return;
}


/* Record function */
int record(const char *outfile)
{
    int result;

    /* Open the input and output files */
    if (outfile != NULL)
    {
        ofp = fopen(outfile, "at");
    }

    /* Set device callback before API startup to get initially-connected devices through the callback */
    OmSetDeviceCallback(record_DeviceCallback, NULL);

    /* Start the API */
    result = OmStartup(OM_VERSION);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmStartup() %s\n", OmErrorString(result)); return -1; }

    for (;;)
    {
        /* Wait 5 seconds */
        Sleep(5000);
        fprintf(stderr, ".\n");
    }

    /* Shutdown the API */
    result = OmShutdown();
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmShutdown() %s\n", OmErrorString(result)); return -1; }

    /* Close the input and output files */
    if (ofp != NULL) { fclose(ofp); }

    return 0;
}


/* Main function */
int record_main(int argc, char *argv[])
{
    printf("RECORD: batch sets clear and record fully-charged devices.\n");
    printf("\n");
    if (argc > 1)
    {
        const char *outfile = NULL;
        outfile = argv[1];
        return record(outfile);
    }
    else
    {
        printf("Usage: record <output-file>\n");
        printf("\n");
        printf("Where: output-file: will receive the device ids\n");
        printf("\n");
        printf("Example: deploy output.csv\n");
        printf("\n");
    }
    return -1;
}

