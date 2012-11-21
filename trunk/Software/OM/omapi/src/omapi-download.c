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

// Open Movement API - Download Functions
// Dan Jackson, 2011-2012

#include "omapi-internal.h"
#include <sys/stat.h>


/** Download buffer size */
#define OM_DOWNLOAD_BLOCK_SET (256)


/** Internal method to update the download progress. */
static int OmDoDownloadUpdate(unsigned short deviceId, OM_DOWNLOAD_STATUS downloadStatus, int downloadValue)
{
    OmDeviceState *device;
 
    // Check system and device state
    if (!om.initialized) return OM_E_NOT_VALID_STATE;
    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    device = om.devices[deviceId];

    // Acquire download mutex here (otherwise there is a small window here in which the state will become unknown if accessed from another thread)
    mutex_lock(&om.downloadMutex);          // Lock download mutex to update device state structure
    device->downloadStatus = downloadStatus;
    device->downloadValue = downloadValue;
    mutex_unlock(&om.downloadMutex);        // Release download mutex after updating device state structure

    // Call user-supplied callback
    if (om.downloadCallback != NULL)
    {
        om.downloadCallback(device->downloadReference != NULL ? device->downloadReference : om.downloadCallbackReference, deviceId, device->downloadStatus, device->downloadValue);
    }

    return OM_OK;
}


/** Internal method to run the download thread. */
static thread_return_t OmDownloadThread(void *arg)
{
    int downloadValue = OM_E_UNEXPECTED;
    OM_DOWNLOAD_STATUS downloadStatus = OM_DOWNLOAD_ERROR;
    OmDeviceState *deviceState = (OmDeviceState *)arg;
    char *buffer = (char *)malloc((OM_DOWNLOAD_BLOCK_SET * OM_BLOCK_SIZE));
    if (buffer == NULL)
    { 
        downloadStatus = (OM_DOWNLOAD_STATUS)OM_E_OUT_OF_MEMORY; 
    }
    else if (deviceState->downloadSource == NULL)
    {
        downloadStatus = (OM_DOWNLOAD_STATUS)OM_E_POINTER; 
    }
    else
    {
        // Initial status update
        downloadStatus = OM_DOWNLOAD_PROGRESS;
        downloadValue = 0;
        OmDoDownloadUpdate(deviceState->id, downloadStatus, downloadValue);

        // Copy loop
        while (downloadStatus == OM_DOWNLOAD_PROGRESS)
        {
            int blocksRead, blocksWritten;
            int position;
            int toRead;

            // Calculate how many blocks to read
            toRead = deviceState->downloadBlocksTotal - deviceState->downloadBlocksCopied;
            if (toRead > OM_DOWNLOAD_BLOCK_SET) { toRead = OM_DOWNLOAD_BLOCK_SET; }
            if (toRead <= 0) { downloadValue = 100; downloadStatus = OM_DOWNLOAD_COMPLETE; break; }

            // Check for cancellation
            if (deviceState->downloadCancel) { downloadStatus = OM_DOWNLOAD_CANCELLED; break; }

            // Check for unexpected end
            if (feof(deviceState->downloadSource)) { downloadStatus = OM_DOWNLOAD_ERROR; break; }

            // Get current position
            position = ftell(deviceState->downloadSource);

            // Read a block of data
            blocksRead = fread(buffer, OM_BLOCK_SIZE, toRead, deviceState->downloadSource);
            if (blocksRead <= 0) { downloadStatus = OM_DOWNLOAD_ERROR; downloadValue = OM_E_ACCESS_DENIED; break; }

            // Check for cancellation
            if (deviceState->downloadCancel) { downloadStatus = OM_DOWNLOAD_CANCELLED; break; }

            // Call user-supplied chunk callback
            if (om.downloadChunkCallback != NULL)
            {
                om.downloadChunkCallback(deviceState->downloadReference != NULL ? deviceState->downloadReference : om.downloadChunkCallbackReference, deviceState->id, buffer, position, blocksRead * OM_BLOCK_SIZE);
            }

            // Write the block of data
            if (deviceState->downloadDest == NULL)
            {
                blocksWritten = blocksRead;
            }
            else
            {
                blocksWritten = fwrite(buffer, OM_BLOCK_SIZE, blocksRead, deviceState->downloadDest);
                if (blocksWritten != blocksRead) {  downloadStatus = OM_DOWNLOAD_ERROR; downloadValue = OM_E_ACCESS_DENIED; break; }
            }

            // Update progress
            deviceState->downloadBlocksCopied += blocksWritten;
            if (deviceState->downloadBlocksTotal == 0) { downloadValue = 0; }
            else { downloadValue = (int)(deviceState->downloadBlocksCopied * 100UL / deviceState->downloadBlocksTotal); }
            OmDoDownloadUpdate(deviceState->id, downloadStatus, downloadValue);
        }
    }

    // Close resources
    if (buffer != NULL) { free(buffer); }
    if (deviceState->downloadSource != NULL) { fclose(deviceState->downloadSource); }
    if (deviceState->downloadDest != NULL) { fclose(deviceState->downloadDest); }

    // Update progress
    OmDoDownloadUpdate(deviceState->id, downloadStatus, downloadValue);

    // Return
    return thread_return_value(0);
}


int OmGetDataFileSize(int deviceId)
{
    int status;
    char filename[OM_MAX_PATH];
    struct stat s;

    status = OmGetDataFilename(deviceId, filename);
    if (OM_FAILED(status)) { return status; }

    if (stat(filename, &s) != 0) { return OM_E_ACCESS_DENIED; }

    return (int)s.st_size;
}


int OmGetDataFilename(int deviceId, char *filenameBuffer)
{
    if (filenameBuffer == NULL) return OM_E_POINTER;
    filenameBuffer[0] = '\0';
    // Check system and device state
    if (!om.initialized) return OM_E_NOT_VALID_STATE;
    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    if (om.devices[deviceId]->deviceStatus != OM_DEVICE_CONNECTED) return OM_E_INVALID_DEVICE;   // Device lost
    if (strlen(om.devices[deviceId]->root) == 0)
    {
        // We don't have a path to the root
        // ??? (Could re-mount the volume to a path here?)
        return OM_E_FAIL;
    }
    strcat(filenameBuffer, om.devices[deviceId]->root);
#if !defined(_WIN32)
    strcat(filenameBuffer, "/");
#endif
    strcat(filenameBuffer, OM_DEFAULT_FILENAME);

    // Check file existence/properties
#if 0
    {
        struct stat s;
        int result;
        result = stat(filenameBuffer, &s);
        if (result != 0)
        {
            return OM_E_ACCESS_DENIED;
        }
        //buf.st_size       // file size
        //buf.st_mtime
    }
#endif

    return OM_OK;
}


int OmGetDataRange(int deviceId, int *dataBlockSize, int *dataOffsetBlocks, int *dataNumBlocks, OM_DATETIME *startTime, OM_DATETIME *endTime)
{
    int status;
    char filename[OM_MAX_PATH];
    OmReaderHandle reader;

    status = OmGetDataFilename(deviceId, filename);
    if (OM_FAILED(status)) { return status; }

    reader = OmReaderOpen(filename);
    if (reader == NULL) { return OM_E_ACCESS_DENIED; }

    status = OmReaderDataRange(reader, dataBlockSize, dataOffsetBlocks, dataNumBlocks, startTime, endTime);

    OmReaderClose(reader);

    return status;
}


int OmBeginDownloading(int deviceId, int dataOffsetBlocks, int dataLengthBlocks, const char *destinationFile)
{
    return OmBeginDownloadingReference(deviceId, dataOffsetBlocks, dataLengthBlocks, destinationFile, NULL);
}


int OmBeginDownloadingReference(int deviceId, int dataOffsetBlocks, int dataLengthBlocks, const char *destinationFile, void *reference)
{
    int status;
    char filename[OM_MAX_PATH];
    OmDeviceState *device;
 
    // Check system and device state
    if (!om.initialized) return OM_E_NOT_VALID_STATE;
    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    if (om.devices[deviceId]->deviceStatus != OM_DEVICE_CONNECTED) return OM_E_INVALID_DEVICE;   // Device lost
    device = om.devices[deviceId];

    // Check parameters
    if (dataOffsetBlocks < 0) return OM_E_INVALID_ARG;
    if (dataLengthBlocks < -1) return OM_E_INVALID_ARG;
    if (destinationFile != NULL && !strlen(destinationFile)) return OM_E_INVALID_ARG;

    // Acquire download mutex here (otherwise there is a small window here in which the state will become unknown if two threads start a download at exactly the same time).
    mutex_lock(&om.downloadMutex);          // Lock download mutex to begin download thread
    do          // This is only a 'do' to allow a single code path to hit the mutex unlock with any exceptional 'break's
    {
        int fileTotalBlocks;

        // Checks if we are already downloading, fails if a download is in progress
        if (device->downloadStatus == OM_DOWNLOAD_PROGRESS) { status = OM_E_NOT_VALID_STATE; break; }

        // Sets the download status to not-started
        device->downloadStatus = OM_DOWNLOAD_NONE;

        // Checks system and device state and gets the data file name for the specified device
        status = OmGetDataFilename(deviceId, filename);
        if (OM_FAILED(status)) { break; }

        // Open the source file
        device->downloadSource = fopen(filename, "rb");
        if (device->downloadSource == NULL) { status = OM_E_ACCESS_DENIED; break; }

        // Open the destination file
        if (destinationFile == NULL)
        {
            device->downloadDest = NULL;
        }
        else
        {
            device->downloadDest = fopen(destinationFile, "wb");
            if (device->downloadDest == NULL) { fclose(device->downloadSource); device->downloadSource = NULL; status = OM_E_ACCESS_DENIED; break; }
        }

        // Calculate the total number of blocks in the file
        fseek(device->downloadSource, 0, SEEK_END);
        fileTotalBlocks = ftell(device->downloadSource) / OM_BLOCK_SIZE;

        // Seek to the requested block offset
        if (dataOffsetBlocks > fileTotalBlocks) { fclose(device->downloadSource); device->downloadSource = NULL; status = OM_E_INVALID_ARG; break; }
        fseek(device->downloadSource, OM_BLOCK_SIZE * dataOffsetBlocks, SEEK_SET);

        // If the requested total number of blocks is negative, use the actual number remaining
        if (dataLengthBlocks < 0)
        {
            device->downloadBlocksTotal = fileTotalBlocks - dataOffsetBlocks;
        }
        else
        {
            device->downloadBlocksTotal = fileTotalBlocks;
        }

        // If the requested number of blocks is too many, return
        if (dataOffsetBlocks + device->downloadBlocksTotal > fileTotalBlocks) { fclose(device->downloadSource); device->downloadSource = NULL; status = OM_E_INVALID_ARG; break; }

        // Start the download thread
        device->downloadBlocksCopied = 0;
        device->downloadCancel = 0;
        device->downloadReference = reference;
        //OmDoDownloadUpdate(device->id, OM_DOWNLOAD_PROGRESS, 0);        // Removed - don't do an initial update here, one is done in OmDownloadThread anyway, and don't want to call out to user code with the download mutex held
        thread_create(&device->downloadThread, NULL, OmDownloadThread, device);

        status = OM_OK;
    } while(0);
    mutex_unlock(&om.downloadMutex);        // Release download mutex after updating beginning download thread

    return status;
}


int OmQueryDownload(int deviceId, OM_DOWNLOAD_STATUS *downloadStatus, int *downloadValue)
{
    OmDeviceState *device;
    OM_DOWNLOAD_STATUS dStatus;
    int value;

    // Check system and device state
    if (!om.initialized) return OM_E_NOT_VALID_STATE;
    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    if (om.devices[deviceId]->deviceStatus != OM_DEVICE_CONNECTED) return OM_E_INVALID_DEVICE;   // Device lost
    device = om.devices[deviceId];

    // Acquire download mutex here (otherwise there's a small chance the status and value may be inconsistent and invalidated by a download start/update/stop).
    mutex_lock(&om.downloadMutex);          // Lock download mutex to query device state structure
    {
        dStatus = device->downloadStatus;
        value = device->downloadValue;
    }
    mutex_unlock(&om.downloadMutex);        // Release download mutex after querying device state structure

    // Output values
    if (downloadStatus != NULL) { *downloadStatus = dStatus; }
    if (downloadValue != NULL) { *downloadValue = value; }

    return OM_OK;
}


int OmWaitForDownload(int deviceId, OM_DOWNLOAD_STATUS *downloadStatus, int *downloadValue)
{
    OM_DOWNLOAD_STATUS dStatus = OM_DOWNLOAD_NONE;
    int dValue = -1;
    int status;

    // Check download state
    OmLog(3, "OmWaitForDownload() started.\n");
    status = OmQueryDownload(deviceId, &dStatus, &dValue);
    if (OM_FAILED(status)) { return status; }

    // If downloading...
    if (dStatus == OM_DOWNLOAD_PROGRESS && om.devices[deviceId]->downloadThread != NULL)
    {
        // Wait for download thread to terminate
        OmLog(3, "OmWaitForDownload() waiting for download thread to terminate...\n");
        thread_join(&om.devices[deviceId]->downloadThread, NULL);
    }

    // Check completed download state
    OmLog(3, "OmWaitForDownload() checking status...\n");
    status = OmQueryDownload(deviceId, &dStatus, &dValue);
    if (OM_FAILED(status)) { return status; }

    // Return values
    if (downloadStatus != NULL) { *downloadStatus = dStatus; }
    if (downloadValue != NULL) { *downloadValue = dValue; }

    return OM_OK;
}


int OmCancelDownload(int deviceId)
{
    // Check system and device state
    if (!om.initialized) return OM_E_NOT_VALID_STATE;
    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen

    // Set signal for download to cancel
    om.devices[deviceId]->downloadCancel = 1;

    // Wait for the download to stop (or return immediately if not in progress)
    return OmWaitForDownload(deviceId, NULL, NULL);
}


OmReaderHandle OmReaderOpenDeviceData(int deviceId)
{
    char filenameBuffer[256];
    int status;
    status = OmGetDataFilename(deviceId, filenameBuffer);
    if (OM_FAILED(status)) { return NULL; }
    return OmReaderOpen(filenameBuffer);
}
