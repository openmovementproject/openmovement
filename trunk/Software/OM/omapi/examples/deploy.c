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
 *  @file deploy.c
 *  @brief     Open Movement API Example: Batch setup fully-charged and cleared devices.
 *  @author    Dan Jackson
 *  @date      2011-2012
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2012, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  A command-line tool to batch setup of all fully-charged and cleared devices, includes delayed activation and time synchronization.
 *  Session identifiers are issued from a list in a user-supplied file. 
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


/* Deploy function */
int deploy(const char *infile, const char *outfile)
{
    int result;
    unsigned int nextSessionId = 0;
    FILE *ifp = NULL;
    FILE *ofp = NULL;

    /* Open the input and output files */
    ifp = fopen(infile, "rt");
    if (outfile != NULL)
    {
        ofp = fopen(outfile, "at");
    }

    /* Start the API */
    result = OmStartup(OM_VERSION);
    if (OM_FAILED(result)) { printf("ERROR: OmStartup() %s\n", OmErrorString(result)); return -1; }

    /* Loop until we've exhausted the incoming list of session identifiers to issue. */
    /* NOTE: Production code should not loop in this way, but instead respond to the OmDeviceCallback events. */
    while (!feof(ifp))
    {
        int numDevices;
        int *deviceIds;
        int i;

        /* Query the current number of devices attached */
        result = OmGetDeviceIds(NULL, 0);
        if (OM_FAILED(result)) { printf("ERROR: OmGetDeviceIds() %s\n", OmErrorString(result)); return -1; }
        numDevices = result;

        /* Get the currently-attached devices ids */
        deviceIds = (int *)malloc(numDevices * sizeof(int));
        result = OmGetDeviceIds(deviceIds, numDevices);
        if (OM_FAILED(result)) { printf("ERROR: OmGetDeviceIds() %s\n", OmErrorString(result)); return -1; }
        /* Cope with fewer devices being returned (if some were just removed). */
        if (result < numDevices) { numDevices = result; } 

        /* For each device currently connected... */
        for (i = 0; i < numDevices; i++)
        {
            int dataOffsetBlocks, dataNumBlocks;
            unsigned int sessionId;
            int deviceId = deviceIds[i];
            char timeStartString[32], timeStopString[32];

            /* If we don't have a 'next session id', read the file to get one */
            while (nextSessionId == 0)
            {
                static char line[100];
                if (fgets(line, 100, ifp) == NULL) { break; }
                nextSessionId = (unsigned int)atoi(line);
            }
            if (nextSessionId == 0) { break; }

            /* Check battery level */
            result = OmGetBatteryLevel(deviceId);
            if (OM_FAILED(result)) { printf("WARNING: OmGetBatteryLevel() %s\n", OmErrorString(result)); continue; }

            /* Check if still charging */
            if (result < 100)
            {
                /* Set the LED to yellow to indicate charging */
                OmSetLed(deviceId, OM_LED_YELLOW);
                printf("%d = CHARGING\n", deviceId);
                continue;
            }

            /* The battery is at 100%, check the data range */
            result = OmGetDataRange(deviceId, NULL, &dataOffsetBlocks, &dataNumBlocks, NULL, NULL);
            if (OM_FAILED(result)) { printf("ERROR: OmGetDataRange() %s\n", OmErrorString(result)); continue; }

            /* See if this device is clear (has a zero session id) */
            result = OmGetSessionId(deviceId, &sessionId);
            if (OM_FAILED(result)) { printf("ERROR: OmGetSessionId() %s\n", OmErrorString(result)); continue; }

            /* Check if there's any data blocks stored (more than just the headers) */
            if (dataNumBlocks - dataOffsetBlocks > 0)
            {
                printf("DEPLOY #%d: Ignoring - has data stored on the device.\n");
                /* Set the LED to red to indicate full but un-copied data */
                OmSetLed(deviceId, OM_LED_RED);
                printf("%d = HAS-DATA (run 'clear' example to reset)\n", deviceId);
                continue;
            }

            if (sessionId != 0)
            {
                /* The device is already prepared for deployment - set the LED to green. */
                OmSetLed(deviceId, OM_LED_GREEN);
                printf("%d = ALREADY-DEPLOYED (run 'clear' example to reset)\n", deviceId);
                continue;
            }

            /* This is a clear device, ready to set up for deployment */

            /* Synchronize the time */
            {
                time_t now;
                struct tm *tm;
                OM_DATETIME nowTime;

                /* Get the current time */
                now = time(NULL);
                tm = localtime(&now);
                nowTime = OM_DATETIME_FROM_YMDHMS(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

                printf("DEPLOY #%d: Synchronizing the time...\n", deviceId);
                result = OmSetTime(deviceId, nowTime);
            }
            if (OM_FAILED(result)) { printf("ERROR: OmSetTime() %s\n", OmErrorString(result)); continue; }

            /* Get the time */
            {
                OM_DATETIME time;
                char buffer[OM_DATETIME_BUFFER_SIZE];
                result = OmGetTime(deviceId, &time);
                if (OM_FAILED(result)) { printf("WARNING: OmGetTime() %s\n", OmErrorString(result)); }
                printf("DEPLOY #%d: Device time: %s\n", deviceId, OmDateTimeToString(time, buffer));
            }

            /* Set the delayed start/stop times */
            {
                time_t now;
                struct tm *tm;
                OM_DATETIME startTime, stopTime;
                char buffer[OM_DATETIME_BUFFER_SIZE];

                /* Get the current time */
                now = time(NULL);

#if 1
                printf("DEPLOY #%d: Setting delayed start/stop times (start in 3 days at 9am, stop after 1 week)\n", deviceId);

                /* Start recording on the day 3 days from now, at 9am */
                now += 3 * 24 * 60 * 60;    /* 3 days in seconds */
                tm = localtime(&now);
                tm->tm_hour = 9;
                tm->tm_min = 0;
                tm->tm_sec = 0;
                startTime = OM_DATETIME_FROM_YMDHMS(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                sprintf(timeStartString, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

                /* Stop recording 7 days from that day, at 9am */
                now += 7 * 24 * 60 * 60;    /* 7 days in seconds */
                tm = localtime(&now);
                tm->tm_hour = 9;
                tm->tm_min = 0;
                tm->tm_sec = 0;
                stopTime = OM_DATETIME_FROM_YMDHMS(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                sprintf(timeStopString, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

#else
                printf("DEPLOY #%d: Setting delayed start/stop times (start in 1 minute, stop after 1 minute)\n", deviceId);

                /* Start recording in 1 minute */
                now += 60;
                tm = localtime(&now);
                startTime = OM_DATETIME_FROM_YMDHMS(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                sprintf(timeStartString, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

                /* Stop recording 1 minute later */
                now += 60;
                tm = localtime(&now);
                stopTime = OM_DATETIME_FROM_YMDHMS(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                sprintf(timeStopString, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

#endif

                /* Set the delayed start/stop times */
                printf("DEPLOY #%d: Setting start: %s [%s]\n", deviceId, timeStartString, OmDateTimeToString(startTime, buffer));
                printf("DEPLOY #%d: Setting stop:  %s [%s]\n", deviceId, timeStopString, OmDateTimeToString(stopTime, buffer));
                result = OmSetDelays(deviceId, startTime, stopTime);
            }
            if (OM_FAILED(result)) { printf("ERROR: OmSetDelays() %s\n", OmErrorString(result)); continue; }

            /* Get the delayed start/stop times */
            {
                OM_DATETIME start, stop;
                char buffer[OM_DATETIME_BUFFER_SIZE];
                result = OmGetDelays(deviceId, &start, &stop);
                if (OM_FAILED(result)) { printf("WARNING: OmGetTime() %s\n", OmErrorString(result)); }
                printf("DEPLOY #%d: Device delayed start: %s\n", deviceId, OmDateTimeToString(start, buffer));
                printf("DEPLOY #%d: Device delayed stop:  %s\n", deviceId, OmDateTimeToString(stop, buffer));
            }

            /* Set the session id */
            result = OmSetSessionId(deviceId, nextSessionId);
            printf("DEPLOY #%d: Setting session id: %u\n", deviceId, nextSessionId);
            if (OM_FAILED(result)) { printf("ERROR: OmSetSessionId() %s\n", OmErrorString(result)); continue; }

            /* Get the session id */
            {
                unsigned int sessionId;
                result = OmGetSessionId(deviceId, &sessionId);
                if (OM_FAILED(result)) { printf("WARNING: OmGetSessionId() %s\n", OmErrorString(result)); }
                printf("DEPLOY #%d: Device session id: %u\n", deviceId, sessionId);
            }

            /* Commit the new settings */
            printf("DEPLOY #%d: Committing new settings...\n", deviceId);
            OmClearDataAndCommit(deviceId);

            /* The device is ready for deployment */
            printf("DEPLOYED,%u,%d,%s,%s\n", nextSessionId, deviceId, timeStartString, timeStopString);
            if (ofp != NULL)
            {
                fprintf(ofp, "%u,%u,%s,%s\n", nextSessionId, deviceId, timeStartString, timeStopString);
            }
            nextSessionId = 0;

            /* The device is prepared for deployment - set the LED to green. */
            OmSetLed(deviceId, OM_LED_GREEN);
        }

        /* Free our list of device ids */
        free(deviceIds);

        /* Wait 15 seconds before querying devices again */
        Sleep(15000);
        printf("---\n");
    }

    /* Shutdown the API */
    result = OmShutdown();
    if (OM_FAILED(result)) { printf("ERROR: OmShutdown() %s\n", OmErrorString(result)); return -1; }

    /* Close the input and output files */
    if (ifp != NULL) { fclose(ifp); }
    if (ofp != NULL) { fclose(ofp); }

    return 0;
}


/* Main function */
int deploy_main(int argc, char *argv[])
{
    printf("DEPLOY: batch setup of all fully-charged and clear devices.\n");
    printf("\n");
    if (argc > 1)
    {
        const char *infile = argv[1];
        const char *outfile = NULL;
        if (argc > 2)
        {
            outfile = argv[2];
        }
        return deploy(infile, outfile);
    }
    else
    {
        printf("Usage: deploy <input-file> [output-file]\n");
        printf("\n");
        printf("Where: input-file: contains one numeric session identifier on each line to issue to each deployed device\n");
        printf("       output-file: will receive the mapping of device ids to each session identifiers\n");
        printf("\n");
        printf("Example: deploy input.txt output.txt\n");
        printf("\n");
    }
    return -1;
}

