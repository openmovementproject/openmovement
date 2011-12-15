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

// Open Movement API - Windows-specific Device Finder
// Dan Jackson, 2011

#ifndef _WIN32
#error "Windows-specific device discovery"
#endif

#include "DeviceFinder.h"

#include "omapi-internal.h"


/** Windows-specific device finder instance */
static DeviceFinder *deviceFinder;


/** Internal callback handler from DeviceFinder for device addition. */
static void OmWindowsAddedCallback(void *reference, const Device &device)
{
    OmDeviceDiscovery(OM_DEVICE_CONNECTED, device.serialNumber, device.port.c_str(), device.volumePath.c_str());
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

