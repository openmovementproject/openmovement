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

// Device Finder
// Dan Jackson, 2011-2012


#ifndef DEVICEFINDER_H
#define DEVICEFINDER_H

#include <list>
#include <string>
#include <map>


class Device
{
public:
    std::string usb;                // Unique PNP prefix, e.g. "USB\VID_04D8&PID_0057\7&91737B1&0"
    std::string port;               // Serial port device name, e.g. "\\.\COM98"
    std::string usbStor;            // USBSTOR identifier, e.g. "USBSTOR\DISK&VEN_CWA&PROD_CWA_MASS_STORAGE&REV_0017\8&1A780901&0&CWA17_65535&0"
    std::string usbComposite;       // USB composite device ID
    unsigned int deviceNumber;      // Device number, e.g. 1
    std::string physicalVolume;     // e.g. "\Device\HarddiskVolume105"
    std::string volumeName;         // e.g. "\\?\Volume{377c1972-0fbb-11e1-98fc-0024bed79d50}\"
    std::string volumePath;         // e.g. "E:\"
    std::string serialString;       // Parent composite device serial number, e.g. "CWA17_00123"
    unsigned int serialNumber;      // Serial number, e.g. 123

    std::string ToString();

    bool operator==(const Device &other) const;
    bool operator!=(const Device &other) const { return !(*this == other); }
};


typedef void(*DeviceFinderCallback)(void *, const Device &);


class DeviceFinder
{

public:
    // Constructs a DeviceFinder with the specified vendor id and product id
    DeviceFinder(unsigned int vidPid);

    // Destructs the DeviceFinder
    ~DeviceFinder();

    // Some VID/PID identifiers
    static const unsigned int VID_PID_CWA = 0x04D80057;   // VID/PID for CWA Composite Device
    static const unsigned int VID_PID_WAX = 0x04D8000A;   // VID/PID for WAX CDC Device

    // Macros to construct/destruct a single vid/pid identifier
    #define VID_PID(vid, pid) ((((unsigned int)(vid)) << 16) | (unsigned short)(pid))
    #define VID(id) ((unsigned short)((id) >> 16))
    #define PID(id) ((unsigned short)(id))

    // Discovery loop
    bool Start(bool continuous, DeviceFinderCallback addedCallback, DeviceFinderCallback removedCallback, void *callbackReference);
    void Stop(void);

    // (Usually internal)
    bool Initialize(void);
    bool Uninitialize(void);
    bool FindDevices(std::list<Device>& devices);
    bool InitialScanDevices(void);
    bool RescanDevices(void);
    std::map<int, Device> deviceMap;

    // (private)
    unsigned int DiscoveryLoop(void);
    int WinProc(void *windowHandle, unsigned int message, unsigned int wParam, long lParam);

private:

    // (Usually internal) method to perform searches
    static bool MappingUsbToPort(unsigned int vidPid, std::map<std::string, std::string>& usbToPortMap);
    static bool MappingUsbToUsbstorAndUsbComposite(unsigned int vidPid, std::map<std::string, std::string>& usbToUsbstorMap, std::map<std::string, std::string>& usbToUsbComposite);
    bool MappingUsbstorToDeviceNumber(std::map<std::string, int>& usbStorToDeviceMap);
    static bool MappingDeviceNumberToPhysicalVolume(std::map<int, std::string>& deviceNumberToPhysicalVolumeMap);
    static bool MappingPhysicalVolumeToVolumeName(std::map<std::string, std::string>& physicalVolumeToVolumeNameMap);
    static std::string GetVolumePathForVolumeName(std::string volumeName);

    static std::string GetUniquePart(std::string id);
    static unsigned int GetDeviceNumber(wchar_t *devicePath);

    // General
    bool initialized;
    unsigned int vidPid;                // USB vendor id & product id

    // Device discovery
    volatile bool quitFlag;             // Thread termination flag
    void *thread;                       // HANDLE 
    void *pLoc;                         // IWbemLocator
    void *pSvc;                         // IWbemServices
    void *hWndDeviceFinder;             // HWND
    void *hDeviceNotify;                // HDEVNOTIFY
    int timerUpdateCountdown;

    // Callback
    DeviceFinderCallback addedCallback;
    DeviceFinderCallback removedCallback;
    void *callbackReference;
};



#endif
