/* 
 * Copyright (c) 2011, Newcastle University, UK.
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

// Program to find a COM port on Windows from a specified USB vid/pid
// Dan Jackson, 2011

/* From a batch file:
    set COMPORT=
    for /f %%f in ('findcom.exe') do set COMPORT=%%f
*/

#ifndef _WIN32
#error "Windows-specific code."
#endif

// Windows header
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

// Setup API
#include <setupapi.h>
#include <cfgmgr32.h>
#pragma comment(lib, "setupapi.lib")

// Headers
#include <string.h>
#include <stdio.h>

// Constants for USB device
#define DEFAULT_VID 0x04D8           // USB Vendor ID
#define DEFAULT_PID 0x000A           // USB Product ID


// Find port names for a given USB VID & PID
int OutputPorts(unsigned short vid, unsigned short pid)
{
    int count = 0;
    GUID *pGuids;
    DWORD dwGuids = 0;
    unsigned int guidIndex;
    char prefix[32];
    int index;

    // PNPDeviceID to search for
    sprintf(prefix, "USB\\VID_%04X&PID_%04X", vid, pid);

    // Convert the name "Ports" to a GUID
    SetupDiClassGuidsFromNameA("Ports", NULL, 0, &dwGuids) ;
    if (dwGuids == 0) { fprintf(stderr, "ERROR: SetupDiClassGuidsFromName() failed.\n"); return -1; }
    pGuids = (GUID *)malloc(dwGuids * sizeof(GUID));
    if (!SetupDiClassGuidsFromNameA("Ports", pGuids, dwGuids, &dwGuids)) { fprintf(stderr, "ERROR: SetupDiClassGuidsFromName() failed.\n"); free(pGuids); return -1; }

    // For each GUID returned
    for (guidIndex = 0; guidIndex < dwGuids; guidIndex++)
    {
        HDEVINFO hDevInfo;

        // From the root of the device tree, look for all devices that match the interface GUID
        hDevInfo = SetupDiGetClassDevs(&pGuids[guidIndex], NULL, NULL, DIGCF_PRESENT);
        if (hDevInfo == INVALID_HANDLE_VALUE) { free(pGuids); return -1; }

        for (index = 0; ; index++)
        {
            char usbId[MAX_PATH] = "";

            // Enumerate the current device
            SP_DEVINFO_DATA devInfo;
            devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
            if (!SetupDiEnumDeviceInfo(hDevInfo, index, &devInfo)) { break; }

            // Get USB id for device
            CM_Get_Device_IDA(devInfo.DevInst, usbId, MAX_PATH, 0);

            // If this is the device we're after
            if (strncmp(usbId, prefix, strlen(prefix)) == 0)
            {
                HKEY hDeviceKey;

#if 1
                // Move up one level to get to the composite device string
                char usbComposite[MAX_PATH] = "";
                DWORD parent = 0;
                CM_Get_Parent(&parent, devInfo.DevInst, 0);
                CM_Get_Device_IDA(parent, usbComposite, MAX_PATH, 0);
                if (strncmp(usbComposite, prefix, strlen(prefix)) != 0)
                {
                    usbComposite[0] = '\0';     // If it doesn't match the vid/pid, this is not a composite device
                }
#endif

                // Registry key for the ports settings
                hDeviceKey = SetupDiOpenDevRegKey(hDevInfo, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
                if (hDeviceKey)
                {
                    // Name of the port
                    char portName[MAX_PATH] = "";
                    DWORD dwSize = sizeof(portName);
                    DWORD dwType = 0;
                    if ((RegQueryValueExA(hDeviceKey, "PortName", NULL, &dwType, (LPBYTE)portName, &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ))
                    {
// Debug output
//fprintf(stderr, "[PORT:USB] %s\n", usbId);
//fprintf(stderr, "[PORT:COMPOSITE] %s\n", usbComposite);
//fprintf(stderr, "[PORT:NAME] %s\n", portName);
                        printf("\\\\.\\%s\n", portName);
                        count++;
                    }
                }

            }
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
    free(pGuids); 
    return count;
}


// Main
int main(int argc, char *argv[])
{
	int vid = DEFAULT_VID;
	int pid = DEFAULT_PID;
    int count = 0;
	
	if (argc > 1) { sscanf(argv[1], "%x", &vid); }
	if (argc > 2) { sscanf(argv[2], "%x", &pid); }
	
	fprintf(stderr, "Searching for VID=%04X&PID=%04X...\n", vid, pid);

    count = OutputPorts(vid, pid);

#if defined(_WIN32) && defined(_DEBUG)
    if (IsDebuggerPresent()) { fprintf(stderr, "Press [enter] to exit..."); getc(stdin); }
#endif

	return (count > 0) ? 0 : -1;
}

