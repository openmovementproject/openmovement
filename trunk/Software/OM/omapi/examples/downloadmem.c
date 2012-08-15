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
 *  @file downloadmem.c
 *  @brief     Open Movement API Example: Download data into RAM from all devices.
 *  @author    Dan Jackson
 *  @date      2012
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2012, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  A command-line tool to download into RAM from all devices. 
 *  This example uses the OmDeviceCallback to monitor for any connected devices and
 *    begins downloading in the background.
 *  This example uses the OmDownloadCallback and OmDownloadChunkCallback to monitor download progress.
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


/* Allocate download structure */
typedef struct 
{
    unsigned char *buffer;
    int capacity;
    int length;
} downloadStatus_t;


/* Device updated */
static void downloadmem_DeviceCallback(void *reference, int deviceId, OM_DEVICE_STATUS status)
{
    if (status == OM_DEVICE_CONNECTED)
    {
        downloadStatus_t *downloadStatus;
        int result;

        printf("DOWNLOADMEM #%d: Device CONNECTED\n", deviceId);

        /* Allocate download structure */
        downloadStatus = (downloadStatus_t *)malloc(sizeof(downloadStatus_t));
        if (downloadStatus == NULL)
        {
            printf("ERROR: Unable to allocate a download status structure\n");
            return;
        }

        /* Initialize download status structure */
        memset(downloadStatus, 0, sizeof(downloadStatus));

        /* Get the data file size */
        downloadStatus->capacity = OmGetDataFileSize(deviceId);
        if (OM_FAILED(downloadStatus->capacity))
        {
            printf("ERROR: OmGetDataFileSize() %s\n", OmErrorString(downloadStatus->capacity));
            free(downloadStatus);
            return;
        }

        /* Allocate buffer */
        downloadStatus->buffer = (unsigned char *)malloc(downloadStatus->capacity);
        if (downloadStatus->buffer == NULL)
        {
            printf("ERROR: Unable to allocate a download buffer (%d bytes)\n", downloadStatus->capacity);
            free(downloadStatus);
            return;
        }

        /* Begin download */
        printf("DOWNLOADMEM #%d: Starting download to memory\n", deviceId);
        downloadStatus->length = 0;
        result = OmBeginDownloadingReference(deviceId, 0, -1, NULL, downloadStatus);
        if (OM_FAILED(result))
        {
            printf("ERROR: OmBeginDownloadingReference() %s\n", OmErrorString(result));
            free(downloadStatus->buffer);
            free(downloadStatus);
            return;
        }

    }
    else if (status == OM_DEVICE_REMOVED)
    {
        printf("DOWNLOADMEM #%d: Device REMOVED\n", deviceId);
        /* The download will have already been cancelled in the event of a device removal */
        /*OmCancelDownload(deviceId);*/
    }
    else
    {
        printf("DOWNLOADMEM #%d: Error, unexpected status %d\n", deviceId, status);
    }
    return;
}


/* Download updated */
static void downloadmem_DownloadCallback(void *reference, int deviceId, OM_DOWNLOAD_STATUS status, int value)
{
    downloadStatus_t *downloadStatus = (downloadStatus_t *)reference;

    if (status == OM_DOWNLOAD_PROGRESS)
    { 
        printf("DOWNLOADMEM #%d: Progress %d%%.\n", deviceId, value);
    }
    else if (status == OM_DOWNLOAD_COMPLETE)
    { 
        printf("DOWNLOADMEM #%d: Complete.\n", deviceId);

        if (downloadStatus == NULL)
        {
            printf("ERROR: No download status tracker.\n");
        }
        else if (downloadStatus->buffer == NULL)
        {
            printf("ERROR: No download buffer.\n");
        }
        else if (downloadStatus->capacity < 0 || downloadStatus->length < 0 || downloadStatus->length > downloadStatus->capacity)
        {
            printf("ERROR: Download buffer offsets invalid.\n");
        }
        else if (downloadStatus->length < downloadStatus->capacity)
        {
            printf("ERROR: Download buffer was not filled.\n");
        }
        else
        {
            printf("SUCCESS: Download buffer filled in RAM: %d bytes at 0x%p\n", downloadStatus->length, downloadStatus->buffer);
            /* TODO: Do something with data at downloadStatus->buffer (downloadStatus->length bytes) */
        }
    }
    else if (status == OM_DOWNLOAD_CANCELLED)
    { 
        printf("DOWNLOADMEM #%d: Cancelled.\n", deviceId);
    }
    else if (status == OM_DOWNLOAD_ERROR) 
    { 
        printf("DOWNLOADMEM #%d: Error. (Diagnostic 0x%04x)\n", deviceId, value);
    }
    else
    {
        printf("DOWNLOADMEM #%d: Unexpected status %d / 0x%04x\n", deviceId, status, value);
    }

    // If download complete, remove download buffer
    if (status == OM_DOWNLOAD_COMPLETE || status == OM_DOWNLOAD_CANCELLED || status == OM_DOWNLOAD_ERROR) 
    {
        if (downloadStatus != NULL)
        {
            downloadStatus->capacity = 0;
            if (downloadStatus->buffer != NULL)
            {
                printf("NOTE: Freeing download buffer @0x%p\n", downloadStatus->buffer);
                free(downloadStatus->buffer);
                downloadStatus->buffer = NULL;
            }
            free(downloadStatus);
        }
    }
    return;
}


/* Download chunk */
static void downloadmem_DownloadChunkCallback(void *reference, int deviceId, void *buffer, int offset, int length)
{
    downloadStatus_t *downloadStatus = (downloadStatus_t *)reference;
    int end;

    end = offset + length;
    printf("DOWNLOADMEM #%d: Chunk @%d (of %d bytes) => %d bytes total\n", deviceId, offset, length, end);

    /* Sanity check buffer and offsets */
    if (downloadStatus == NULL)
    {
        printf("ERROR: No download status tracker.\n");
        return;
    }
    if (downloadStatus->buffer == NULL)
    {
        printf("ERROR: No download buffer.\n");
        return;
    }
    if (offset < 0 || offset >= downloadStatus->capacity || end < 0 || end > downloadStatus->capacity)
    {
        printf("ERROR: Download chunk offsets out of range.\n");
        return;
    }

    /* Copy data to RAM buffer */
    memcpy(downloadStatus->buffer + offset, buffer, length);
    downloadStatus->length = end;

    return;
}


/* Download to memory function */
int downloadmem(void)
{
    int result;

    /* Set device callback before API startup to get initially-connected devices through the callback */
    OmSetDeviceCallback(downloadmem_DeviceCallback, NULL);

    /* Set download callback */
    OmSetDownloadCallback(downloadmem_DownloadCallback, NULL);

    /* Set download chunk callback */
    OmSetDownloadChunkCallback(downloadmem_DownloadChunkCallback, NULL);

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
int downloadmem_main(int argc, char *argv[])
{
    printf("DOWNLOADMEM: download data into RAM from all devices containing data.\n");
    printf("\n");
    if (argc == 1)
    {
        return downloadmem();
    }
    else
    {
        printf("Usage: downloadmem\n");
        printf("\n");
        printf("Example: downloadmem\n");
        printf("\n");
    }
    return -1;
}

