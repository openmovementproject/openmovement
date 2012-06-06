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

// NOTE: Finding the pairs of CDC and MSD instances, together with their composite parent device serial number, 
//       is quite a mess on Windows.  This file is still a complete mess from getting it working!
// TODO: The big messy mixture of char strings, wchar_t strings, std::string, etc. needs a lot of tidying up!


//#define DEBUG_PRINT

#ifndef _WIN32
#error "Windows-specific code."
#endif

#define _WIN32_DCOM
#define _CRT_SECURE_NO_WARNINGS
#include <comdef.h>
#include <wbemidl.h>
#include <windows.h>
#pragma comment(lib, "wbemuuid.lib")

#include <setupapi.h>
#include <winioctl.h>
#include <cfgmgr32.h>
#pragma comment(lib, "setupapi.lib")

#pragma comment(lib, "advapi32.lib")    // For RegQueryValueEx()
//#pragma comment(lib, "gdi32.lib")       // For CreateSolidBrush()

#include <dbt.h>

#include <tchar.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <set>
#include "DeviceFinder.h"

using namespace std;


#if 1
extern "C" int OmLog(const char *format, ...);
#define Log(...) OmLog(__VA_ARGS__)
#else
#define Log(...) fprintf(stderr, __VA_ARGS__)
#endif


// Utility function
static const char *GetLastErrorString()
{
    static char lastError[1024];
    lastError[0] = '\0';
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lastError, 1024, NULL);
    return lastError;
}



std::string Device::ToString()
{
    std::string returnValue;
    returnValue += "[";
    returnValue += serialNumber;
    returnValue += ": ";
    returnValue += port;
    returnValue += " / ";
    returnValue += volumePath;
    returnValue += "]";
    return returnValue;
}

bool Device::operator==(const Device &other) const
{
    return this->usb == other.usb
        && this->port == other.port
        && this->usbStor == other.usbStor
        && this->usbComposite == other.usbComposite
        && this->deviceNumber == other.deviceNumber
        && this->physicalVolume == other.physicalVolume
        && this->volumeName == other.volumeName
        && this->volumePath == other.volumePath
        && this->serialString == other.serialString
        && this->serialNumber == other.serialNumber;
}


bool DeviceFinder::Initialize(void)
{
    if (initialized) { return true; }

    initialized = false;
    pLoc = NULL;
    pSvc = NULL;
    hWndDeviceFinder = NULL;
    hDeviceNotify = NULL;

    // Initialize COM
    HRESULT hr;
    hr =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    if (hr != S_OK && hr != S_FALSE && hr != RPC_E_CHANGED_MODE) { cerr << "ERROR: Failed to initialize COM library: " << hr << endl; return false; }

    // Set COM security levels
    hr =  CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hr)) { cerr << "WARNING: Failed to initialize security: " << hr << endl; }

    // Obtain the WMI initial locator
    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
    if (FAILED(hr)) { cerr << "ERROR: Failed to create IWbemLocator object: " << hr << endl; CoUninitialize(); return false; }

    // Connect to the root\cimv2 WMI namespace through the IWbemLocator::ConnectServer method with the current user and obtain pointer pSvc to make IWbemServices calls.
    hr = ((IWbemLocator *)pLoc)->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, (IWbemServices **)&pSvc);
    if (FAILED(hr)) { cerr << "ERROR: Could not connect to WMI ROOT\\CIMV2: " << hr << endl; ((IWbemLocator *)pLoc)->Release(); pLoc = NULL; CoUninitialize(); return false; }

    // Set security levels on the proxy
    hr = CoSetProxyBlanket((IWbemServices *)pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    if (FAILED(hr)) { cerr << "WARNING: Could not set proxy blanket: " << hr << endl; }

    initialized = true;
    return true;
}


bool DeviceFinder::Uninitialize(void)
{
    // Cleanup IWbemServices, IWbemLocator
    if (pSvc != NULL) { ((IWbemServices *)pSvc)->Release(); pSvc = NULL; }
    if (pLoc != NULL) { ((IWbemLocator *)pLoc)->Release(); pLoc = NULL; }

    // Un-initialize COM
// TODO: Track whether we initialized com or if it was already initialized
//    if (initialized) { CoUninitialize(); }

    initialized = false; 
    return true;
}


// Constructs a DeviceFinder with the specified vendor id and product id
DeviceFinder::DeviceFinder(unsigned int vidPid)
{
    this->initialized = false;
    this->vidPid = vidPid;
    return;
}


// Destructs the DeviceFinder
DeviceFinder::~DeviceFinder()
{
    Stop();
    Uninitialize();
}


// From a USB string, find the unique part (will be the same for composite child instances)
std::string DeviceFinder::GetUniquePart(std::string id)
{
    // Input:
    //   USB\VID_04D8&PID_0057&MI_01\6&23C68C4E&0&0001
    //   USB\VID_04D8&PID_0057&MI_00\6&23C68C4E&0&0000
    // Output:
    //   USB\VID_04D8&PID_0057\6&23C68C4E&0

    int miIndex = id.find("&MI_");
    if (miIndex >= 0)
    {
        id = id.erase(miIndex, 6);
    }

    int lastAnd = id.find_last_of('&');
    int lastSlash = id.find_last_of('\\');
    if (lastSlash >= miIndex && lastAnd > lastSlash)
    {
        id = id.substr(0, lastAnd);
    }

    return id;
}


// Get device type and number from device path
unsigned int DeviceFinder::GetDeviceNumber(wchar_t *devicePath)
{
    HANDLE handle = CreateFileW(devicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, NULL);
    if (handle == INVALID_HANDLE_VALUE) { return -1; }
    DWORD len = 0;
    STORAGE_DEVICE_NUMBER sdn = {0};
    DeviceIoControl(handle, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &len, NULL);
    CloseHandle(handle);
    return sdn.DeviceNumber;
}


/*
// (Usually internal) method to perform USB serial port search against vid/pid
bool DeviceFinder::FindPorts(unsigned int vidPid, std::map<std::string, std::string>& ports)
{
    // Clear list of port names
    ports.clear();

    // PNPDeviceID to search for
    char prefix[32];
    sprintf_s(prefix, 32, "USB\\VID_%04X&PID_%04X", VID(vidPid), PID(vidPid));

    HRESULT hr;

    // Use the IWbemServices pointer to make a WMI request for Win32_SerialPort
    IEnumWbemClassObject* pEnumerator = NULL;
    hr = ((IWbemServices *)pSvc)->ExecQuery(bstr_t("WQL"), bstr_t("SELECT PNPDeviceID, DeviceID FROM Win32_SerialPort"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    if (FAILED(hr)) { cerr << "ERROR: Query for Win32_SerialPort has failed: " << hr << endl; return false; }

    // Get the data from the query
    if (pEnumerator)
    {
        for (;;)
        {
            // Get next item in enumeration
            IWbemClassObject *pclsObj;
            ULONG uReturn = 0;
            hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            // If no more items, exit loop
            if (uReturn == 0) { break; }

            // Get value of the "PNPDeviceID" property
            VARIANT vtPropPnpDeviceId;
            hr = pclsObj->Get(L"PNPDeviceID", 0, &vtPropPnpDeviceId, 0, 0);
            char pnpDeviceId[256] = { 0 };
            wcstombs_s(NULL, pnpDeviceId, vtPropPnpDeviceId.bstrVal, 255);                  // Convert to ASCII
            VariantClear(&vtPropPnpDeviceId);

            //cerr << "DeviceFinder: Checking PNPDeviceId: " << pnpDeviceId << " --> " << endl;

            // See if matches requested VID/PID
            if (_strnicmp(pnpDeviceId, prefix, strlen(prefix)) == 0)
            {
                string uniqueId = GetUniquePart(pnpDeviceId);

                // Get value of the "DeviceID" property
                VARIANT vtPropDeviceId;
                hr = pclsObj->Get(L"DeviceID", 0, &vtPropDeviceId, 0, 0);
                char deviceId[256] = { 0 };
                wcstombs_s(NULL, deviceId, vtPropDeviceId.bstrVal, 255);                    // Convert to ASCII
                VariantClear(&vtPropDeviceId);

                // Prefix with UNC device specifier (to work with port numbers higher than the few reserved names)
                string portName = string("\\\\.\\") + deviceId;
                cerr << "PORT: " << uniqueId << " --> " << portName << endl;

                // Add to list of matched port names
                ports[uniqueId] = portName;
            }

            pclsObj->Release();
        }
        pEnumerator->Release();
    }

    return true;
}
*/


/*
bool DeviceFinder::DiskDriveToLogicalDrive(char *deviceId)
{
    // deviceId = "\\.\PHYSICALDRIVE1"
    HRESULT hr;

    _bstr_t query;
    query += L"ASSOCIATORS OF {Win32_DiskDrive.DeviceID=\"";
    for (char *p = deviceId; *p != '\0'; p++)
    {
        wchar_t c[2];
        c[0] = *p; c[1] = '\0';
        if (*p == '\\') { query += c; }
        query += c;
    }
    query += L"\"} WHERE AssocClass = Win32_DiskDriveToDiskPartition";

    IEnumWbemClassObject* pEnumerator2 = NULL;
    hr = ((IWbemServices *)pSvc)->ExecQuery(bstr_t("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator2);
    if (FAILED(hr) || !pEnumerator2) { cerr << "ERROR: Query for Win32_DiskDriveToDiskPartition has failed: " << hr << endl; return false; }
    for (;;)
    {
        VARIANT vtProp;

        // Get next item in enumeration
        IWbemClassObject *pclsObj2;
        ULONG uReturn2 = 0;
        hr = pEnumerator2->Next(WBEM_INFINITE, 1, &pclsObj2, &uReturn2);

        // If no more items, exit loop
        if (uReturn2 == 0) { break; }

        // Get value of the "DeviceID" property
        hr = pclsObj2->Get(L"DeviceID", 0, &vtProp, 0, 0);
        char deviceId2[256] = { 0 };
        wcstombs_s(NULL, deviceId2, vtProp.bstrVal, 255);                    // Convert to ASCII
        VariantClear(&vtProp);

cerr << "[DRIVE:PARTITION] " << deviceId2 << endl;


        // Get the logical disks for the partition
        _bstr_t query2;
        query2 += L"ASSOCIATORS OF {Win32_DiskPartition.DeviceID=\"";
        for (char *p = deviceId2; *p != '\0'; p++)
        {
            wchar_t c[2];
            c[0] = *p; c[1] = '\0';
            if (*p == '\\') { query2 += c; }
            query2 += c;
        }
        query2 += L"\"} WHERE AssocClass = Win32_LogicalDiskToPartition";



        IEnumWbemClassObject* pEnumerator3 = NULL;
        hr = ((IWbemServices *)pSvc)->ExecQuery(bstr_t("WQL"), query2, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator3);
        if (FAILED(hr) || !pEnumerator3) { cerr << "ERROR: Query for Win32_LogicalDiskToPartition has failed: " << hr << endl; break; }
        for (;;)
        {
            // Get next item in enumeration
            IWbemClassObject *pclsObj3;
            ULONG uReturn3 = 0;
            hr = pEnumerator3->Next(WBEM_INFINITE, 1, &pclsObj3, &uReturn3);

            // If no more items, exit loop
            if (uReturn3 == 0) { break; }

            // Get value of the "DeviceID" property
            hr = pclsObj3->Get(L"DeviceID", 0, &vtProp, 0, 0);
            char deviceId3[256] = { 0 };
            wcstombs_s(NULL, deviceId3, vtProp.bstrVal, 255);                    // Convert to ASCII
            VariantClear(&vtProp);

cerr << "[DRIVE:LOGICAL] " << deviceId3 << endl;


        }

    }
    return true;
}
*/


// Find a mapping of unique USB prefix to port names
bool DeviceFinder::MappingUsbToPort(unsigned int vidPid, std::map<std::string, std::string>& usbToPortMap)
{
    usbToPortMap.clear();

    // PNPDeviceID to search for
    char prefix[32];
    sprintf_s(prefix, 32, "USB\\VID_%04X&PID_%04X", VID(vidPid), PID(vidPid));

    // Convert the name "Ports" to a GUID
    DWORD dwGuids = 0;
    SetupDiClassGuidsFromNameW(L"Ports", NULL, 0, &dwGuids) ;
    if (dwGuids == 0) { Log("ERROR: SetupDiClassGuidsFromName() failed.\n"); return false; }
    GUID *pGuids = new GUID[dwGuids];       // &GUID_DEVINTERFACE_COMPORT
    if (!SetupDiClassGuidsFromNameW(L"Ports", pGuids, dwGuids, &dwGuids)) { Log("ERROR: SetupDiClassGuidsFromName() failed.\n"); return false; }

    // For each GUID returned
    for (unsigned int guidIndex = 0; guidIndex < dwGuids; guidIndex++)
    {
        // From the root of the device tree, look for all devices that match the interface GUID
        HDEVINFO hDevInfo = SetupDiGetClassDevs(&pGuids[guidIndex], NULL, NULL, DIGCF_PRESENT);     // | DIGCF_DEVICEINTERFACE    
        if (hDevInfo == INVALID_HANDLE_VALUE) { delete [] pGuids; return false; }

        for (int index = 0; ; index++)
        {
            wchar_t scratch[256];

            // Enumerate the current device
            SP_DEVINFO_DATA devInfo;
            devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
            if (!SetupDiEnumDeviceInfo(hDevInfo, index, &devInfo)) { break; }

            // Get USB id for device
            scratch[0] = 0;
            CM_Get_Device_IDW(devInfo.DevInst, scratch, 256, 0);
            char usbId[256];
            wcstombs_s(NULL, usbId, scratch, 256);

//Log("[PORT:USB] %s\n", usbId);

            // If this is the device we're after
            if (strncmp(usbId, prefix, strlen(prefix)) == 0)
            {
                /*
                // Move up one level to get to the composite device
                DWORD parent = 0;
                CM_Get_Parent(&parent, devInfo.DevInst, 0);
                scratch[0] = 0;
                CM_Get_Device_ID(parent, scratch, 256, 0);
                char usbComposite[256];
                wcstombs_s(NULL, usbComposite, scratch, 256);
                if (strncmp(usbComposite, prefix, strlen(prefix)) != 0)
                {
                    usbComposite[0] = '\0';
                }
                */

                // Registry key for the ports settings
                char portName[256] = "";
                HKEY hDeviceKey = SetupDiOpenDevRegKey(hDevInfo, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
                if (hDeviceKey)
                {
                    // Name of the port
                    scratch[0] = 0;
                    DWORD dwSize = sizeof(scratch);
                    DWORD dwType = 0;

                    if ((RegQueryValueExW(hDeviceKey, L"PortName", NULL, &dwType, (LPBYTE)scratch, &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ))
                    {
                        portName[0] = '\\'; portName[1] = '\\'; portName[2] = '.'; portName[3] = '\\';
                        wcstombs(portName + 4, scratch, 255 - 4);
                    }
                }

                char usbUnique[256] = "";
                strcpy(usbUnique, GetUniquePart(usbId).c_str());

                // Store mapping
//Log("[PORT:KEY] %s\n", key);
//Log("[PORT:NAME] %s\n", portName);

#ifdef DEBUG_PRINT
    Log("[USB->PORT] %s -> %s\n", usbUnique, portName);
#endif

                usbToPortMap[usbUnique] = portName;
            }
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
    delete [] pGuids;
    return true;
}



// Find drive mapping from USBSTOR id to their parent USB device id
bool DeviceFinder::MappingUsbToUsbstorAndUsbComposite(unsigned int vidPid, std::map<std::string, std::string>& usbToUsbstorMap, std::map<std::string, std::string>& usbToUsbCompositeMap)
{
    usbToUsbstorMap.clear();
    usbToUsbCompositeMap.clear();

    // PNPDeviceID to search for
    char prefix[32];
    sprintf_s(prefix, 32, "USB\\VID_%04X&PID_%04X", VID(vidPid), PID(vidPid));

    const GUID *pGuid = &GUID_DEVINTERFACE_DISK;

    // From the root of the device tree, look for all devices that match the interface GUID
    HANDLE hDevInfo = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE) { return false; }

    for (int index = 0; ; index++)
    {
        wchar_t scratch[256];
                
        // Enumerate the current device
        SP_DEVINFO_DATA devInfo;
        devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
        if (!SetupDiEnumDeviceInfo(hDevInfo, index, &devInfo)) { break; }

#if 0
        char path[256] = "";

        // Device Interface Data structure
        SP_DEVICE_INTERFACE_DATA devInterface;
        devInterface.cbSize = sizeof(devInterface);
        if (SetupDiEnumDeviceInterfaces(hDevInfo, NULL, pGuid, (unsigned int)index, &devInterface))
        {
            // Get more detailed information
            DWORD nRequiredSize = 0;
            SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInterface, NULL, 0, &nRequiredSize, NULL);
            SP_DEVICE_INTERFACE_DETAIL_DATA *devInterfaceDetail = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(nRequiredSize + sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA));
            if (devInterfaceDetail != NULL)
            {
                devInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInterface, devInterfaceDetail, nRequiredSize, NULL, NULL))
                { 
                    wcstombs_s(NULL, path, devInterfaceDetail->DevicePath, 256);
                }
                free(devInterfaceDetail);
            }
        }

        Log("[DRIVEMAP:PATH] %s\n", path);
#endif

        // Move up one level to get to the "USB" level
        DWORD parent = 0;
        CM_Get_Parent(&parent, devInfo.DevInst, 0);
        scratch[0] = 0;
        CM_Get_Device_IDW(parent, scratch, 256, 0);
        char usbId[256];
        wcstombs_s(NULL, usbId, scratch, 256);
        //Log("[DRIVEMAP:USB] %s\n", usbId);

        // If this is the device we're after
        if (strncmp(usbId, prefix, strlen(prefix)) == 0)
        {
            // Get USBSTOR name at current level
            scratch[0] = 0;
            CM_Get_Device_IDW(devInfo.DevInst, scratch, 256, 0);
            char usbstorId[256];
            wcstombs_s(NULL, usbstorId, scratch, 256);

            // Move up another level to get to the composite device
            DWORD grandparent = 0;
            CM_Get_Parent(&grandparent, parent, 0);
            scratch[0] = 0;
            CM_Get_Device_IDW(grandparent, scratch, 256, 0);
            char usbComposite[256];
            wcstombs_s(NULL, usbComposite, scratch, 256);
            if (strncmp(usbComposite, prefix, strlen(prefix)) != 0)
            {
                usbComposite[0] = '\0';
            }

            // If the grandparent doesn't seem to be a composite, use the unique part of the USB id instead
            char usbUnique[256];
            strcpy(usbUnique, GetUniquePart(usbId).c_str());

            // Store mapping
//Log("[DRIVEMAP:KEY] %s\n", key);
//Log("[DRIVEMAP:USBSTOR] %s\n", usbstorId);

#ifdef DEBUG_PRINT
            Log("[USB->USBSTOR] %s -> %s\n", usbUnique, usbstorId);
#endif

            usbToUsbstorMap[usbUnique] = usbstorId;
            usbToUsbCompositeMap[usbUnique] = usbComposite;
        }

    }
    SetupDiDestroyDeviceInfoList(hDevInfo);
    return true;
}


// (Usually internal) method to perform usb drive search
bool DeviceFinder::MappingUsbstorToDeviceNumber(std::map<std::string, int>& usbStorToDeviceMap)
{
    HRESULT hr;

    // Clear list of drive names
    usbStorToDeviceMap.clear();

    // Ensure pSvc is initialized
    Initialize();

    if (pSvc == NULL) { cerr << "ERROR: Unable to get usbstor-deviceNumber mapping." << endl;  return false; }

    // Use the IWbemServices pointer to make a WMI request for Win32_SerialPort
    IEnumWbemClassObject* pEnumerator = NULL;
    hr = ((IWbemServices *)pSvc)->ExecQuery(bstr_t("WQL"), bstr_t("SELECT PNPDeviceID, DeviceID FROM Win32_DiskDrive WHERE InterfaceType='USB'"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    if (FAILED(hr)) { cerr << "ERROR: Query for Win32_DiskDrive has failed: " << hr << endl; return false; }

    // Get the data from the query
    if (pEnumerator)
    {
        for (;;)
        {
            // Get next item in enumeration
            IWbemClassObject *pclsObj;
            ULONG uReturn = 0;
            hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            // If no more items, exit loop
            if (uReturn == 0) { break; }

            // Get value of the "PNPDeviceID" property
            VARIANT vtProp;
            hr = pclsObj->Get(L"PNPDeviceID", 0, &vtProp, 0, 0);
            char usbstorId[256] = { 0 };
            wcstombs_s(NULL, usbstorId, vtProp.bstrVal, 255);                  // Convert to ASCII
            VariantClear(&vtProp);
//cerr << "[DRIVE:USBSTOR] " << usbstorId << endl;

            // Get value of the "DeviceID" property
            hr = pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);
            char deviceId[256] = { 0 };
            wcstombs_s(NULL, deviceId, vtProp.bstrVal, 255);                    // Convert to ASCII
            unsigned int deviceNumber = GetDeviceNumber(vtProp.bstrVal);
            VariantClear(&vtProp);
//cerr << "[DRIVE:DEVICEID] " << deviceId << endl;
//cerr << "[DRIVE:DEVICENUMBER] " << deviceNumber << endl;
            //DiskDriveToLogicalDrive(deviceId);

#ifdef DEBUG_PRINT
            Log("[USBSTOR->DEVICENUMBER] %s -> %u\n", usbstorId, deviceNumber);
#endif
            usbStorToDeviceMap[usbstorId] = deviceNumber;

            pclsObj->Release();
        }
        pEnumerator->Release();
    }

    return true;
}


// Find the mapping of drive device numbers to the first volume returned for that drive
bool DeviceFinder::MappingDeviceNumberToPhysicalVolume(std::map<int, std::string>& deviceNumberToPhysicalVolumeMap)
{
    deviceNumberToPhysicalVolumeMap.clear();

    HDEVINFO devInfoSet = SetupDiGetClassDevsW(&GUID_DEVINTERFACE_VOLUME, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    SP_DEVICE_INTERFACE_DATA devInterface = { 0 };
    devInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    for (int i = 0; SetupDiEnumDeviceInterfaces(devInfoSet, NULL, &GUID_DEVINTERFACE_VOLUME, i, &devInterface); ++i)
    {
        SP_DEVINFO_DATA devInfoData = { 0 };
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        DWORD len;
        SetupDiGetDeviceInterfaceDetailW(devInfoSet, &devInterface, NULL, 0, &len, &devInfoData);
        std::vector<char> buf(len);
        SP_DEVICE_INTERFACE_DETAIL_DATA_W *devInterfaceDetail = (SP_DEVICE_INTERFACE_DETAIL_DATA_W *)&buf[0];
        devInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
        if (SetupDiGetDeviceInterfaceDetailW(devInfoSet, &devInterface, devInterfaceDetail, len, NULL, &devInfoData)) 
        {
            unsigned int volumeDeviceNumber = GetDeviceNumber(devInterfaceDetail->DevicePath);
            // If we don't already have a volume for this device...            
            if (deviceNumberToPhysicalVolumeMap.find(volumeDeviceNumber) == deviceNumberToPhysicalVolumeMap.end()) 
            {
                wchar_t buf[MAX_PATH + 1];
                DWORD type, len;
                if (SetupDiGetDeviceRegistryPropertyW(devInfoSet, &devInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, &type, (unsigned char *)buf, MAX_PATH, &len)) 
                {
                    char physicalVolume[MAX_PATH + 1];
                    wcstombs(physicalVolume, buf, MAX_PATH);
#ifdef DEBUG_PRINT
    Log("[DEVICE->PHYSICALVOLUME] %u -> %s\n", volumeDeviceNumber, physicalVolume);
#endif
                    deviceNumberToPhysicalVolumeMap[volumeDeviceNumber] = physicalVolume;
                }
            }
        }
    }
    return true;
}


// Find the mapping of physical volume name to the volume name
bool DeviceFinder::MappingPhysicalVolumeToVolumeName(std::map<std::string, std::string>& physicalVolumeToVolumeNameMap)
{
    physicalVolumeToVolumeNameMap.clear();

    // Enumerate through all system volumes
    wchar_t volumeNameW[MAX_PATH] = L"";
    HANDLE hFind = FindFirstVolumeW(volumeNameW, sizeof(volumeNameW)/sizeof(volumeNameW[0]));
    if (hFind == INVALID_HANDLE_VALUE) { return false; }
    for (;;)
    {
        size_t index = wcslen(volumeNameW) - 1;
        if (volumeNameW[0] != L'\\' || volumeNameW[1] != L'\\' || volumeNameW[2] != L'?' || volumeNameW[3] != L'\\' || volumeNameW[index] != L'\\') 
        {
            // error, expected prefix missing
        }
        else
        {
            char volumeName[MAX_PATH];
            wcstombs(volumeName, volumeNameW, MAX_PATH);

            // Get physical volume name
            wchar_t physicalVolumeW[MAX_PATH] = L"";
            physicalVolumeW[0] = L'\0';
            volumeNameW[index] = L'\0';     // Remove trailing backslash for QueryDosDevice
            DWORD count = QueryDosDeviceW(&volumeNameW[4], physicalVolumeW, sizeof(physicalVolumeW)/sizeof(physicalVolumeW[0])); 
            volumeNameW[index] = L'\\';     // Replace trailing backslash
            char physicalVolume[MAX_PATH];
            wcstombs(physicalVolume, physicalVolumeW, MAX_PATH);

#ifdef DEBUG_PRINT
            Log("[PHYSICALVOLUME->VOLUMENAME] %s -> %s\n", physicalVolume, volumeName);
#endif
            physicalVolumeToVolumeNameMap[physicalVolume] = volumeName;
        }

        // Move to the next volume
        if (!FindNextVolumeW(hFind, volumeNameW, sizeof(volumeNameW)/sizeof(volumeNameW[0]))) { break; }
    }

    FindVolumeClose(hFind);
    return true;
}



// Find the (first) mapping path for the given volume name
std::string DeviceFinder::GetVolumePathForVolumeName(std::string volumeName)
{
    if (volumeName.length() <= 0) { return ""; }
    std::string ret = "";
    wchar_t *volumeNameW = new wchar_t[volumeName.length() + 2];
    mbstowcs(volumeNameW, volumeName.c_str(), volumeName.length() + 1);
    DWORD charCount = MAX_PATH + 1;
    for (;;) 
    {
        // Allocate a buffer to hold the paths
        wchar_t *names = new wchar_t[charCount + 2];

        // Obtain all of the paths for this volume
        if (GetVolumePathNamesForVolumeNameW(volumeNameW, names, charCount + 1, &charCount))
        {
            // Enumerate over the paths
            //for (wchar_t *nameIndex = names; *nameIndex != L'\0'; nameIndex += wcslen(nameIndex) + 1) { wprintf(L"- %s\n", nameIndex); }

            // Return the first path
            char *buf = new char[wcslen(names) + 1];
            wcstombs(buf, names, wcslen(names) + 1);
            ret = buf;
            delete[] buf;

            delete[] names;
            break;
        }
        delete[] names; 
        if (GetLastError() != ERROR_MORE_DATA) { break; }
    }
    delete[] volumeNameW;
    return ret;
}


bool DeviceFinder::FindDevices(std::list<Device>& devices)
{
    devices.clear();

    std::map<std::string, std::string> mapUsbToPort;
    if (!MappingUsbToPort(vidPid, mapUsbToPort)) { Log("ERROR: Problem finding ports.\n"); return false; }

    std::map<std::string, std::string> mapUsbToUsbstor;
    std::map<std::string, std::string> mapUsbToUsbComposite;
    if (!MappingUsbToUsbstorAndUsbComposite(vidPid, mapUsbToUsbstor, mapUsbToUsbComposite)) { Log("ERROR: Problem finding drive mapping.\n"); return false; }

    std::map<std::string, int> mapUsbstorToDeviceNumber;
    if (!MappingUsbstorToDeviceNumber(mapUsbstorToDeviceNumber)) { Log("ERROR: Problem finding drives.\n"); return false; }

    std::map<int, std::string> mapDeviceNumberToPhysicalVolume;
    if (!MappingDeviceNumberToPhysicalVolume(mapDeviceNumberToPhysicalVolume)) { Log("ERROR: Problem finding physical volumes.\n"); return false; }

    std::map<std::string, std::string> mapPhysicalVolumeToVolumeName;
    if (!MappingPhysicalVolumeToVolumeName(mapPhysicalVolumeToVolumeName)) { Log("ERROR: Problem finding volume names.\n"); return false; }

    for (map<string, string>::const_iterator i = mapUsbToPort.begin(); i != mapUsbToPort.end(); ++i)
    {
        string usb = (*i).first;
        string port = (*i).second;
        string usbStor = mapUsbToUsbstor[usb];
        string usbComposite = mapUsbToUsbComposite[usb];
        unsigned int deviceNumber = mapUsbstorToDeviceNumber[usbStor];
        string physicalVolume = mapDeviceNumberToPhysicalVolume[deviceNumber];
        string volumeName = mapPhysicalVolumeToVolumeName[physicalVolume];
        string volumePath = GetVolumePathForVolumeName(volumeName);

        // Find serial string from composite device name
        string serialString;
        char prefix[32];
        sprintf_s(prefix, 32, "USB\\VID_%04X&PID_%04X", VID(vidPid), PID(vidPid));

        if (usbComposite.length() > 0)
        {
            // If this is the composite device we're after, find the serial string
            if (strncmp(usbComposite.c_str(), prefix, strlen(prefix)) == 0)
            {
                int index = strlen(prefix);
                if (usbComposite[index] == '\\') { index++; }
                serialString = usbComposite.substr(index);
            }
        }

        // If serial string not set, use the unique id
        if (serialString.length() == 0)
        {
            // If this is the device we're after
            if (strncmp(usb.c_str(), prefix, strlen(prefix)) == 0)
            {
                int index = strlen(prefix);
                if (usb[index] == '\\') { index++; }
                serialString = usb.substr(index);
            }
        }

        // Find the serial number from the serial string
        int serialNumber = -1;
//printf("SERIAL: %s\n", serialString.c_str());
        if (serialString.length() > 0)
        {
            int firstDigit = serialString.find_first_of('&') + 1;
            int lastDigit = -1;
            if ((serialString[firstDigit] >= '0' && serialString[firstDigit] <= '9') || (serialString[firstDigit] >= 'A' && serialString[firstDigit] <= 'F'))
            {
                lastDigit = serialString.find_first_not_of("0123456789ABCDEF", firstDigit) - 1;
            }

            // Check whether this is a Windows-generated number for a device without a serial number
            if (firstDigit >= 1 && lastDigit >= firstDigit)
            {
                // Extract the part after the first ampersand as a hexadecimal serial number
                serialNumber = strtol(serialString.substr(firstDigit, lastDigit - firstDigit + 1).c_str(), NULL, 16);
                if (serialNumber <= 0xffff) { serialNumber |= 0x80000000; } // ensure it's not an actual (16-bit) serial number
            }
            else
            {
                // Extract the last decimal numeric part of the serial string as a serial number
                int lastNumber = serialString.find_last_of("0123456789");
                if (lastNumber >= 0)
                {
                    int firstNumber = serialString.find_last_not_of("0123456789", lastNumber) + 1;
                    if (firstNumber >= 0)
                    {
                        serialNumber = atoi(serialString.substr(firstNumber, lastNumber - firstNumber + 1).c_str());
                    }
                }
            }
        }


        Device device;
        device.usb = usb;
        device.port = port;
        device.usbStor = usbStor;
        device.usbComposite = usbComposite;
        device.deviceNumber = deviceNumber;
        device.physicalVolume = physicalVolume;
        device.volumeName = volumeName;
        device.volumePath = volumePath;
        device.serialString = serialString;
        device.serialNumber = serialNumber;
        devices.push_back(device);
    }

#if 0
    char b[16];
    string s = "";
    s += _itoa(devices.size(), b, 10); s += " device(s). From: ";
    s += _itoa(mapUsbToPort.size(), b, 10); s += "; ";
    s += _itoa(mapUsbToUsbstor.size(), b, 10); s += "; ";
    s += _itoa(mapUsbToUsbComposite.size(), b, 10); s += "; ";
    s += _itoa(mapUsbstorToDeviceNumber.size(), b, 10); s += "; ";
    s += _itoa(mapDeviceNumberToPhysicalVolume.size(), b, 10); s += "; ";
    s += _itoa(mapPhysicalVolumeToVolumeName.size(), b, 10); s += "; ";
    MessageBoxA(NULL, s.c_str(), "Debug", 0);
#endif

    return true;
}








// Device class GUID (USB serial host PnP drivers)
static GUID WceusbshGUID = { 0x25dbce51, 0x6c8f, 0x4a72, 0x8a,0x6d,0xb5,0x4c,0x2b,0x4f,0xc8,0x35 };


// Registers a HWND for notification of changes in the device interfaces for the specified interface class GUID
static BOOL DoRegisterDeviceInterfaceToHwnd(GUID InterfaceClassGuid, HWND hWnd, HDEVNOTIFY *phDeviceNotify)
{
    HDEVNOTIFY hDeviceNotify;
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = InterfaceClassGuid;
    hDeviceNotify = RegisterDeviceNotification(hWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
    if (phDeviceNotify != NULL) { *phDeviceNotify = hDeviceNotify; }
    if (hDeviceNotify == NULL) { return FALSE; }
    return TRUE;
}


// Static windows callback procedure (jumps to class method)
static INT_PTR WINAPI WinProcTrampoline(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DeviceFinder* instance;

    if (message == WM_CREATE)
    { 
        instance = (DeviceFinder*)((LPCREATESTRUCT)lParam)->lpCreateParams; 
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)instance);  // GWL_USERDATA
    }
    else
    {
        instance = (DeviceFinder*)GetWindowLongPtr(hWnd, GWLP_USERDATA);    // GWL_USERDATA
    }

    if (!instance) { return DefWindowProc(hWnd, message, wParam, lParam); }
    return instance->WinProc(hWnd, message, wParam, lParam);
}

#define TIMER_ID 1
#define TIMER_INTERVAL 500
#define TIMER_UPDATE_COUNTDOWN 1000

// Class instance of Windows message handler
int DeviceFinder::WinProc(void *windowHandle, unsigned int message, unsigned int wParam, long lParam)
{
    HWND hWnd = (HWND)windowHandle;
    LRESULT lRet = 1;

    switch (message)
    {
        case WM_CREATE:
            // Registration at application startup when the window is created.
            if (!DoRegisterDeviceInterfaceToHwnd(WceusbshGUID, hWnd, &hDeviceNotify))
            {
                MessageBoxA(NULL, "Error registering device finder device interface", "Error", 0);
            }
            break;

        case WM_DEVICECHANGE:
        {
#if 0
printf("\r\nWM_DEVICECHANGE: ");
            
            char debugMessageBuffer[32];
            sprintf(debugMessageBuffer, "DBT_0x%08x", wParam);
            char *debugMessage = debugMessageBuffer;
            switch (wParam)
            {
                case DBT_DEVICEARRIVAL              : debugMessage = "DBT_DEVICEARRIVAL";           break;
                case DBT_DEVICEQUERYREMOVE          : debugMessage = "DBT_DEVICEQUERYREMOVE";       break;
                case DBT_DEVICEQUERYREMOVEFAILED    : debugMessage = "DBT_DEVICEQUERYREMOVEFAILED"; break;
                case DBT_DEVICEREMOVEPENDING        : debugMessage = "DBT_DEVICEREMOVEPENDING";     break;
                case DBT_DEVICEREMOVECOMPLETE       : debugMessage = "DBT_DEVICEREMOVECOMPLETE";    break;
                case DBT_DEVICETYPESPECIFIC         : debugMessage = "DBT_DEVICETYPESPECIFIC";      break;
                case DBT_CUSTOMEVENT                : debugMessage = "DBT_CUSTOMEVENT";             break;
                case DBT_DEVTYP_DEVINST             : debugMessage = "DBT_DEVTYP_DEVINST";          break;      // a DEVTYP
            }

printf("%s ", debugMessage);

            PDEV_BROADCAST_HDR lpDevBroadcastHdr = (PDEV_BROADCAST_HDR)lParam;

            char debugDeviceTypeBuffer[32];
            sprintf(debugDeviceTypeBuffer, "DBT_DEVTYP_0x%08x", lpDevBroadcastHdr->dbch_devicetype);
            char *debugDeviceType = debugDeviceTypeBuffer;
            switch (lpDevBroadcastHdr->dbch_devicetype)
            {
                case DBT_DEVTYP_OEM                 : debugDeviceType = "DBT_DEVTYP_OEM";           break;
                case DBT_DEVTYP_DEVNODE             : debugDeviceType = "DBT_DEVTYP_DEVNODE";       break;
                case DBT_DEVTYP_VOLUME              : debugDeviceType = "DBT_DEVTYP_VOLUME";        break;
                case DBT_DEVTYP_PORT                : debugDeviceType = "DBT_DEVTYP_PORT";          break;
                case DBT_DEVTYP_NET                 : debugDeviceType = "DBT_DEVTYP_NET";           break;
                case DBT_DEVTYP_DEVICEINTERFACE     : debugDeviceType = "DBT_DEVTYP_DEVICEINTERFACE"; break;
                case DBT_DEVTYP_HANDLE              : debugDeviceType = "DBT_DEVTYP_HANDLE";        break;
                case DBT_DEVTYP_DEVINST             : debugDeviceType = "DBT_DEVTYP_DEVINST";       break;
            }

printf("%s ", debugDeviceType);

            if (lpDevBroadcastHdr->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME lpDevBroadcastVolume = (PDEV_BROADCAST_VOLUME)lpDevBroadcastHdr;
                printf("unitmask = 0x%04x", lpDevBroadcastVolume->dbcv_unitmask);
            }
            if (lpDevBroadcastHdr->dbch_devicetype == DBT_DEVTYP_PORT)
            {
                PDEV_BROADCAST_PORT lpDevBroadcastPort = (PDEV_BROADCAST_PORT)lpDevBroadcastHdr;
                printf("name = %S", lpDevBroadcastPort->dbcp_name);
            }
            if (lpDevBroadcastHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                PDEV_BROADCAST_DEVICEINTERFACE lpDevBroadcastDeviceInterface = (PDEV_BROADCAST_DEVICEINTERFACE)lpDevBroadcastHdr;
                printf("name = %S", lpDevBroadcastDeviceInterface->dbcc_name);
            }
            
printf("\r\n");

#endif
            timerUpdateCountdown = TIMER_UPDATE_COUNTDOWN;
        }
        break;

    case WM_CLOSE:
        if (hDeviceNotify != NULL) { UnregisterDeviceNotification((HDEVNOTIFY)hDeviceNotify); }
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_TIMER:
        if (wParam == TIMER_ID)
        {
            if (timerUpdateCountdown > 0)
            {
                if (timerUpdateCountdown > TIMER_INTERVAL)
                { 
                    timerUpdateCountdown -= TIMER_INTERVAL; 
                }
                else
                {
                    timerUpdateCountdown = 0; 

                    RescanDevices();
                }
            }

        }
        break;

    default:
        // Send all other messages on to the default windows handler.
        lRet = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return lRet;
}


unsigned int DeviceFinder::DiscoveryLoop(void)
{
    // Initialize and register the window class
    WNDCLASSEX wndClass;
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndClass.hInstance = (HINSTANCE)GetModuleHandle(NULL);
    wndClass.lpfnWndProc = (WNDPROC)WinProcTrampoline;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hIcon = LoadIcon(0,IDI_APPLICATION);
    wndClass.hbrBackground = NULL; //CreateSolidBrush(RGB(192,192,192));
    wndClass.hCursor = LoadCursor(0, IDC_ARROW);
    wndClass.lpszClassName = TEXT("DeviceFinderClass");
    wndClass.lpszMenuName = NULL;
    wndClass.hIconSm = wndClass.hIcon;

    ATOM registerClass = RegisterClassEx(&wndClass);
    if (!registerClass) 
    { 
        //MessageBoxA(NULL, "Error creating device finder window class", "Error", 0);
        //return 0; 
    }

    // Create window
    HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_APPWINDOW, wndClass.lpszClassName, TEXT("DeviceFinder"), WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, (HINSTANCE)GetModuleHandle(NULL), this);
    hWndDeviceFinder = (void *)hWnd;
    if (hWnd == NULL) 
    { 
        MessageBoxA(NULL, "Error creating device finder window", "Error", 0);
        return 0; 
    }
    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    // Start a timer
    timerUpdateCountdown = 0;
    if (!SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, NULL))
    {
        MessageBoxA(NULL, "Error creating device finder timer", "Error", 0);
    }

    // Message pump loops until the window is destroyed
    MSG msg; 
    int retVal;
    while ((retVal = GetMessage(&msg, NULL, 0, 0)) != 0) 
    { 
        if (retVal == -1) break;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KillTimer(hWnd, TIMER_ID);
    hWndDeviceFinder = NULL;

    return 1;
}


static DWORD WINAPI DiscoveryThreadTrampoline(void *reference)
{
    return (DWORD)((DeviceFinder *)reference)->DiscoveryLoop();
}


bool DeviceFinder::InitialScanDevices(void)
{
    bool ret = RescanDevices();
    Uninitialize();
    return ret;
}


bool DeviceFinder::Start(bool continuous, DeviceFinderCallback addedCallback, DeviceFinderCallback removedCallback, void *callbackReference)
{
    // Set quit flag
    quitFlag = !continuous;
    hWndDeviceFinder = NULL;

    // Callback
    this->addedCallback = addedCallback;
    this->removedCallback = removedCallback;
    this->callbackReference = callbackReference;

    // Perform an initial scan
    InitialScanDevices();

    // Continuous loop
    if (!quitFlag)
    {
        thread = CreateThread(NULL, 0, DiscoveryThreadTrampoline, this, 0, NULL);
        if (thread == NULL)
        {
            MessageBoxA(NULL, "Error creating device finder thread", "Error", 0);
        }
    }

    return true;
}


void DeviceFinder::Stop(void)
{
    // Signal then join thread, wait for end
    quitFlag = true;
    while (hWndDeviceFinder != NULL)
    {
        PostMessage((HWND)hWndDeviceFinder, WM_CLOSE, NULL, NULL);
        if (WaitForSingleObject((HANDLE)thread, INFINITE) != WAIT_OBJECT_0) { ; }
    }
}


bool DeviceFinder::RescanDevices(void)
{
//Log("RESCAN:\n");

    // Obtain a list of currently attached devices
    list<Device> devices;
    if (!this->FindDevices(devices)) { Log("ERROR: Problem finding devices.\n"); }

    // Put the new devices into a map
    map<int, Device> newDeviceMap;
    for (list<Device>::const_iterator i = devices.begin(); i != devices.end(); ++i)
    {
        newDeviceMap[(*i).serialNumber] = (*i);
    }

    // Scan for removed and added devices. ('Updated' devices with changed parameters are treated as removed and then re-added)
    // NOTE: The way the class is currently being used doesn't actually require a lock on the deviceMap structure.
    //       It is only accessed on the update thread, either directly in the RescanDevices() function itself,
    //         or from the user-supplied callback functions that are called from RescanDevices().
    //       If the class is to be used in any other way, a mutex primitive must be used.
    set<int> removed;
    set<int> added;

    // Scan for removed devices
    for (map<int, Device>::const_iterator i = deviceMap.begin(); i != deviceMap.end(); ++i)
    {
        Device const &oldDevice = (*i).second;
        if (newDeviceMap.find(oldDevice.serialNumber) == newDeviceMap.end()) 
        {
            removed.insert(oldDevice.serialNumber);
        }
        else
        {
            Device const &newDevice = newDeviceMap[oldDevice.serialNumber];
            // If any device parameters have changed, remove and re-add the device
            if (newDevice != oldDevice)
            {
                removed.insert(oldDevice.serialNumber);
                added.insert(oldDevice.serialNumber);
            }
        }
    }

    // Scan for added devices
    for (map<int, Device>::const_iterator i = newDeviceMap.begin(); i != newDeviceMap.end(); ++i)
    {
        Device const &newDevice = (*i).second;
        if (deviceMap.find(newDevice.serialNumber) == deviceMap.end()) 
        {
            added.insert(newDevice.serialNumber);
        }
    }

    // Process removals first
    for (set<int>::const_iterator i = removed.begin(); i != removed.end(); ++i)
    {
        if (removedCallback != NULL) removedCallback(callbackReference, deviceMap[*i]);   // Call the device handler
//Log("REMOVED: %s\n", deviceMap[*i].ToString().c_str());
        deviceMap.erase(*i);        // Remove the element
    }

    // Now process additions
    for (set<int>::const_iterator i = added.begin(); i != added.end(); ++i)
    {
        deviceMap[*i] = newDeviceMap[*i]; // Add the element
//Log("ADDED: %s\n", newDeviceMap[*i].ToString().c_str());
        if (addedCallback != NULL) addedCallback(callbackReference, deviceMap[*i]);   // Call the device handler
    }

//Log("---\n");

    return true;
}

