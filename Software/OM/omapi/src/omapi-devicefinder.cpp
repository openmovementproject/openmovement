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

// Open Movement API - Windows-specific Device Finder
// Dan Jackson, 2011-2012

#ifndef _WIN32
#error "Windows-specific device discovery"
#endif

#include "DeviceFinder.h"

#include "omapi-internal.h"


#define DEBUG_MOUNT


/** Windows-specific device finder instance */
static DeviceFinder *deviceFinder;


/** Internal callback handler from DeviceFinder for device addition. */
static void OmWindowsAddedCallback(void *reference, const Device &device)
{
    char root[128] = {0};
    char desiredVolumePath[256] = {0};

    // Current (first returned) mount point
    std::string volumePath = device.volumePath;
    
#ifdef DEBUG_MOUNT
OmLog(1, "1: '%s'\n", volumePath.c_str());
#endif

    // Mount location
    sprintf(root, "C:\\Mount");

    // Desired mount point
    if (root != NULL && root[0] != '\0')
    {
        sprintf(desiredVolumePath, "%s\\AX3_%05u\\", root, device.serialNumber);
        //sprintf(desiredVolumePath, "%s\\%s\\", root, device.serialString);
    }

#ifdef DEBUG_MOUNT
OmLog(1, "2: %s\n", root);
OmLog(1, "3: %s\n", desiredVolumePath);
#endif

    // Get existing mount points
    char *volumePathNames = NULL;
    DWORD cchBufferLength = 0;
    DWORD cchReturnLength = 0;
    GetVolumePathNamesForVolumeNameA(device.volumeName.c_str(), volumePathNames, cchBufferLength, &cchReturnLength);
    cchBufferLength = cchReturnLength + MAX_PATH;   // Add a bit as some comments seem to indicate that the wrong length may be returned on some Windows versions
    volumePathNames = (char *)malloc(cchBufferLength * sizeof(char));
    if (volumePathNames != NULL)
    {
        volumePathNames[0] = 0; volumePathNames[1] = 0; // Initialize as a zero-length list of strings
        GetVolumePathNamesForVolumeNameA(device.volumeName.c_str(), volumePathNames, cchBufferLength, &cchReturnLength);
    }

    // Scan existing mount points
    int numMountPoints = 0;
    char hasDesiredMountPoint = 0;
    for (char *name = volumePathNames; name != NULL && name[0] != '\0'; name += strlen(name) + 1)
    {
        numMountPoints++;
        if (desiredVolumePath != NULL && desiredVolumePath[0] != '\0' && stricmp(name, desiredVolumePath) == 0)
        {
            hasDesiredMountPoint = 1;
        }
        if (volumePath.length() == 0)
        {
            volumePath = name;
#ifdef DEBUG_MOUNT
OmLog(1, "1a: Found non-initial point: %s\n", volumePath.c_str());
#endif
        }
    }

    // If we don't have the desired mount point, try to add it
    if (!hasDesiredMountPoint)
    {
#ifdef DEBUG_MOUNT
OmLog(1, "4: Creating desired mount point...\n");
#endif
        // Make root folder if it doesn't exist
        if (root != NULL && root[0] != '\0')
        {
            DWORD attribs = GetFileAttributesA(root);
            if (attribs == INVALID_FILE_ATTRIBUTES || !(attribs & FILE_ATTRIBUTE_DIRECTORY))
            { 
#ifdef DEBUG_MOUNT
OmLog(1, "5: Creating mount point root...\n");
#endif
                if (!CreateDirectoryA(root, NULL)) 
                { 
#ifdef DEBUG_MOUNT
OmLog(1, "5a: Failed to create mount point root...\n");
#endif
                    desiredVolumePath[0] = '\0'; 
                } 
            }
        }

        // Make mount folder if it doesn't exist
        if (desiredVolumePath != NULL && desiredVolumePath[0] != '\0')
        {
            DWORD attribs = GetFileAttributesA(desiredVolumePath);
            if (attribs == INVALID_FILE_ATTRIBUTES || !(attribs & FILE_ATTRIBUTE_DIRECTORY)) 
            { 
#ifdef DEBUG_MOUNT
OmLog(1, "6: Creating mount point...\n");
#endif
                if (!CreateDirectoryA(desiredVolumePath, NULL)) 
                { 
#ifdef DEBUG_MOUNT
OmLog(1, "6a: Failed to create mount point...\n");
#endif
                    desiredVolumePath[0] = '\0'; 
                } 
            }
        }

        // Set the volume mount point
        if (desiredVolumePath != NULL && desiredVolumePath[0] != '\0')
        {
#ifdef DEBUG_MOUNT
OmLog(1, "7: Setting mount point... SetVolumeMountPointA(\"%s\", \"%s\");\n", desiredVolumePath, device.volumeName.c_str());
#endif
            if (SetVolumeMountPointA(desiredVolumePath, device.volumeName.c_str()))
            {
#ifdef DEBUG_MOUNT
OmLog(1, "7b: Set mount point...\n");
#endif
                hasDesiredMountPoint = 1;
            }
            else
            {
                DWORD err = GetLastError();
                if (err == 0x00000005)
                {
OmLog(1, "7a: Failed to set mount point... access denied, must run as an Administrator for re-mounting.\n");
                }
#ifdef DEBUG_MOUNT
else OmLog(1, "7a: Failed to set mount point... %08x\n", err);
#endif
            }
        }
    }

    // If we have the desired mount point, use that in preference to any other first-found mount point
    if (hasDesiredMountPoint)
    {
        volumePath = desiredVolumePath;
#ifdef DEBUG_MOUNT
OmLog(1, "8: Has mount point: %s\n", volumePath.c_str());
#endif
    }

#if 1
    // Un-mount any other (unused) mount points
    for (char *name = volumePathNames; name != NULL && name[0] != '\0'; name += strlen(name) + 1)
    {
        //numMountPoints++;
        if (desiredVolumePath != NULL && desiredVolumePath[0] != '\0' && stricmp(name, volumePath.c_str()) != 0)
        {
            OmLog(1, "DEBUG: Un-mounting unused mount point: %s  -- using: %s\n", name, volumePath.c_str());
            if (!DeleteVolumeMountPointA(name))
            {
                OmLog(1, "WARNING: Failed to un-mount unused mount point: %s  -- using: %s\n", name, volumePath.c_str());
            }
        }
    }
#endif

    // Free mount point list
    if (volumePathNames != NULL) { free(volumePathNames); }

    // Call the device discovery using the found volume
    OmDeviceDiscovery(OM_DEVICE_CONNECTED, device.serialNumber, device.port.c_str(), volumePath.c_str());
}

/** Internal callback handler from DeviceFinder for device removal. */
static void OmWindowsRemovedCallback(void *reference, const Device &device)
{
    OmDeviceDiscovery(OM_DEVICE_REMOVED, device.serialNumber, device.port.c_str(), device.volumePath.c_str());
}


/** Internal method to start device discovery. */
extern "C" void OmDeviceDiscoveryStart(void)
{
    // Device discovery -- Start() performs the initial device discovery then starts the update thread
    (deviceFinder = new DeviceFinder(OM_USB_ID))->Start(true, OmWindowsAddedCallback, OmWindowsRemovedCallback, NULL);
}

/** Internal method to stop device discovery. */
extern "C" void OmDeviceDiscoveryStop(void)
{
    // Stops and removes the device finder
    delete deviceFinder;
}

