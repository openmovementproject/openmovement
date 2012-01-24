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
 *  @file download.c
 *  @brief     Open Movement API Example: Download data from all devices.
 *  @author    Dan Jackson
 *  @date      2011-2012
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2012, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  A command-line tool to download from all devices that containing data. 
 *  This example uses the OmDeviceCallback to monitor for any connected devices and
 *    begins downloading in the background.
 *  This example uses the OmDownloadCallback to monitor download progress.
 *
 *  @remarks Makes use of \ref api_init, \ref download, \ref return_codes
 */


/* Headers */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* API header */
#include "omapi.h"


/* Globals */
static const char *downloadPath = ".";


/* Device updated */
static void download_DeviceCallback(void *reference, int deviceId, OM_DEVICE_STATUS status)
{
    if (status == OM_DEVICE_CONNECTED)
    {
        int result;
        unsigned int sessionId;
        int dataBlockSize = 0, dataOffsetBlocks = 0, dataNumBlocks = 0;
        OM_DATETIME startTime = 0, endTime = 0;

        printf("DOWNLOAD #%d: Device CONNECTED\n", deviceId);

        /* Get the session id */
        result = OmGetSessionId(deviceId, &sessionId);
        if (OM_FAILED(result)) { printf("ERROR: OmGetSessionId() %s\n", OmErrorString(result)); return; }
        printf("DOWNLOAD #%d: SessionId = %d\n", deviceId, sessionId);

        /* Get the data range */
        result = OmGetDataRange(deviceId, &dataBlockSize, &dataOffsetBlocks, &dataNumBlocks, &startTime, &endTime);
        if (OM_FAILED(result)) { printf("ERROR: OmGetDataRange() %s\n", OmErrorString(result)); return; }
        printf("DOWNLOAD #%d: %d blocks data, at offset %d blocks (1 block = %d bytes)\n", deviceId, dataNumBlocks, dataOffsetBlocks, dataBlockSize);
        /* Display the data start and end times */
        {
            char startString[OM_DATETIME_BUFFER_SIZE], endString[OM_DATETIME_BUFFER_SIZE];
            OmDateTimeToString(startTime, startString);
            OmDateTimeToString(endTime, endString);
            printf("DOWNLOAD #%d: ... %s --> %s\n", deviceId, startString, endString);
        }

        /* Check if there's any data blocks stored (not just the headers) */
        if (dataNumBlocks - dataOffsetBlocks <= 0)
        {
            printf("DOWNLOAD #%d: Ignoring - no data stored.\n");
        }
        else
        {
            char *file;

            /* Allocate filename string */
            file = (char *)malloc(strlen(downloadPath) + 32);  /* downloadPath + "/4294967295-65535.cwa" + 1 (NULL-terminated) */

            /* Copy path, and platform-specific path separator char */
            strcpy(file, downloadPath);
            #ifdef _WIN32
                if (file[strlen(file) - 1] != '\\') strcat(file, "\\");
            #else
                if (file[strlen(file) - 1] != '/') strcat(file, "/");
            #endif

            /* Append session-id and device-id as part of the filename */
            sprintf(file + strlen(file), "%010u-%05u.cwa", sessionId, deviceId);

            /* Begin download */
            printf("DOWNLOAD #%d: Starting download to file: %s\n", deviceId, file);
            result = OmBeginDownloading(deviceId, 0, -1, file);
            if (OM_FAILED(result)) { printf("ERROR: OmBeginDownloading() %s\n", OmErrorString(result)); }

            /* Free filename string */
            free(file);
        }
    }
    else if (status == OM_DEVICE_REMOVED)
    {
        printf("DOWNLOAD #%d: Device REMOVED\n", deviceId);
        /* The download will have already been cancelled in the event of a device removal */
        /*OmCancelDownload(deviceId);*/
    }
    else
    {
        printf("DOWNLOAD #%d: Error, unexpected status %d\n", deviceId, status);
    }
    return;
}


/* Download updated */
static void download_DownloadCallback(void *reference, int deviceId, OM_DOWNLOAD_STATUS status, int value)
{
    if (status == OM_DOWNLOAD_PROGRESS)
    { 
        printf("DOWNLOAD #%d: Progress %d%%.\n", deviceId, value);
    }
    else if (status == OM_DOWNLOAD_COMPLETE)
    { 
        printf("DOWNLOAD #%d: Complete.\n", deviceId);
    }
    else if (status == OM_DOWNLOAD_CANCELLED)
    { 
        printf("DOWNLOAD #%d: Cancelled.\n", deviceId);
    }
    else if (status == OM_DOWNLOAD_ERROR) 
    { 
        printf("DOWNLOAD #%d: Error. (Diagnostic 0x%04x)\n", deviceId, value);
    }
    else
    {
        printf("DOWNLOAD #%d: Unexpected status %d / 0x%04x\n", deviceId, status, value);
    }
    return;
}


/* Download function */
int download(const char *outpath)
{
    int result;

    /* Set the global path (used by the device callback) */
    downloadPath = outpath;

    /* Set device callback before API startup to get initially-connected devices through the callback */
    OmSetDeviceCallback(download_DeviceCallback, NULL);

    /* Set download callback */
    OmSetDownloadCallback(download_DownloadCallback, NULL);

    printf("Waiting for devices...\n");

    /* Start the API */
    result = OmStartup(OM_VERSION);
    if (OM_FAILED(result)) { printf("ERROR: OmStartup() %s\n", OmErrorString(result)); return -1; }

    /* Block this thread waiting for a console key-press to terminate the downloader.
     * The callbacks will allow any existing and future devices connected to have their 
     * data downloaded simultaneously. */
    getchar();

    /* Shutdown the API (this will cleanly cancel any partial downloads). */
    result = OmShutdown();
    if (OM_FAILED(result)) { printf("ERROR: OmShutdown() %s\n", OmErrorString(result)); return -1; }

    return 0;
}


/* Main function */
int download_main(int argc, char *argv[])
{
    printf("DOWNLOAD: download data from all devices containing data.\n");
    printf("\n");
    if (argc > 1)
    {
        const char *outpath = argv[1];
        return download(outpath);
    }
    else
    {
        printf("Usage: download <output-path>\n");
        printf("\n");
        printf("Where: output-path: the directory to download the data files to.\n");
        printf("\n");
        printf("Example: download .\n");
        printf("\n");
    }
    return -1;
}

