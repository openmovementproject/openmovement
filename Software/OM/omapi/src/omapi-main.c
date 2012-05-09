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

// Open Movement API - Main Functions
// Dan Jackson, 2011-2012

#include "omapi-internal.h"


const char *OmErrorString(int status)
{
    switch (status)
    {
        case OM_OK                    : return "OK";
        case OM_E_FAIL                : return "Fail";
        case OM_E_UNEXPECTED          : return "Unexpected";
        case OM_E_NOT_VALID_STATE     : return "Not valid state";
        case OM_E_OUT_OF_MEMORY       : return "Out of memory";
        case OM_E_INVALID_ARG         : return "Invalid argument";
        case OM_E_POINTER             : return "Pointer";
        case OM_E_NOT_IMPLEMENTED     : return "Not implemented";
        case OM_E_ABORT               : return "Aborted";
        case OM_E_ACCESS_DENIED       : return "Access denied";
        case OM_E_INVALID_DEVICE      : return "Invalid device";
        case OM_E_UNEXPECTED_RESPONSE : return "Unexpected response";
        case OM_E_LOCKED              : return "Locked";
    }
    if (OM_SUCCEEDED(status))
    {
        return "<success>";
    }
    return "<unknown>";
}


int OmStartup(int version)
{
    int i;

    // Checks
    if (om.initialized) return OM_E_NOT_VALID_STATE;
    if (version != OM_VERSION) return OM_E_FAIL;	// TODO: Better error code for any new version of the API

    // Setup
    om.apiVersion = version;

    // Set log stream if not already set before Startup call
    if (!om.logSet)
    {
        om.log = stderr;
    }

    // Ensure device state table is clear
    for (i = 0; i < OM_MAX_SERIAL; i++)
    {
        om.devices[i] = NULL;
    }

    // Mutex
    mutex_init(&om.portMutex, NULL);
#if !defined(_WIN32)
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); 
    mutex_init(&om.downloadMutex, &attr);
#else
    mutex_init(&om.downloadMutex, NULL);
#endif
    
    // Flag the API as initialized (before device discovery)
    om.initialized = 1;

    // Perform an initial device discovery and create device discovery thread
    OmDeviceDiscoveryStart();

    return OM_OK;
}


int OmShutdown(void)
{
    int i;

    if (!om.initialized) return OM_E_NOT_VALID_STATE;
    om.initialized = 0;

    // Destroy device discovery thread
    OmDeviceDiscoveryStop();
    
    // Clear device state table
    for (i = 0; i < OM_MAX_SERIAL; i++)
    {
        if (om.devices[i] != NULL)
        {
            // Cancel any pending downloads
            if (om.devices[i]->deviceStatus == OM_DEVICE_CONNECTED)
            {
                OmCancelDownload(i);
            }

            free(om.devices[i]);
            om.devices[i] = NULL;
        }
    }

    // Delete mutex
    mutex_destroy(&om.portMutex);
    mutex_destroy(&om.downloadMutex);

    return OM_OK;
}


int OmSetLogStream(int fd)
{
    // Close existing if set
    if (om.logSet && om.log != NULL)
    {
        fclose(om.log);
        om.log = NULL;
        om.logSet = 0;
    }

    // Handle no stream
    if (fd < 0)
    {
        om.log = NULL;
        om.logSet = 1;
        return OM_OK;
    }

    // Open specified stream
    om.log = fdopen(fd, "wt");
    if (om.log == NULL) { return OM_E_ACCESS_DENIED; }

    return OM_OK;
}


int OmSetLogCallback(OmLogCallback logCallback, void *reference)
{
    om.logCallback = logCallback;
    om.logCallbackReference = reference;
    return OM_OK;
}


int OmGetDeviceIds(int *deviceIds, int maxDevices)
{
    int i, total = 0;
    if (!om.initialized) return OM_E_NOT_VALID_STATE;
    for (i = 0; i < OM_MAX_SERIAL; i++)
    {
        OmDeviceState *deviceState = om.devices[i];
        if (deviceState != NULL && deviceState->deviceStatus == OM_DEVICE_CONNECTED)
        {
            if (maxDevices > 0 && deviceIds != NULL)
            {
                *deviceIds++ = i;   //deviceState->id;
                maxDevices--;
            }
            total++;
        }
    }
    return total;
}


int OmSetDeviceCallback(OmDeviceCallback deviceCallback, void *reference)
{
    om.deviceCallback = deviceCallback;
    om.deviceCallbackReference = reference;
    return OM_OK;
}


int OmSetDownloadCallback(OmDownloadCallback downloadCallback, void *reference)
{
    om.downloadCallback = downloadCallback;
    om.downloadCallbackReference = reference;
    return OM_OK;
}


OM_DATETIME OmDateTimeFromString(const char *value)
{
    static const unsigned char maxDaysPerMonth[12+1] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int index;
    unsigned int v;
    unsigned char year = 0, month = 0, day = 0, hours = 0, minutes = 0, seconds = 0;
    const char *c;

    if (value[0] == '\0') { return 0; }
    if (value[0] == '0' && value[1] == '\0') { return 0; }
    if (value[0] == '-') { return 0xffffffff; }
    index = 0;
    v = 0xffff;
    c = value;
    for (;;)
    {
        if (*c >= '0' && *c <= '9') // Part of value
        {
            if (v == 0xffff) { v = 0; }
            v = (v * 10) + (*c - '0');
        }
        else
        {
            if (v != 0xffff)  // End of value
            {
                if      (index == 0) { year = (v >= 2000) ? (v - 2000) : v; }
                else if (index == 1) { month = v; }
                else if (index == 2) { day = v; }
                else if (index == 3) { hours = v; }
                else if (index == 4) { minutes = v; }
                else if (index == 5) { seconds = v; }
                else { break; }
                index++;
                v = 0xffff;
                if (index >= 6) { break; }
            }
            if (*c == '\0') { break; }
        }
        c++;
    }

    // Check if parsed six elements and check validity of members
    if (    index == 6 &&
            (month >= 1 && month <= 12) &&
            (day >= 1 && day <= maxDaysPerMonth[month]) &&
            (hours <= 23) &&
            (minutes <= 59) &&
            (seconds <= 59)
        )
    {
        if (month == 2 && day == 29 && (year & 3) != 0) { month = 3; day = 1; }    // Turn 29-Feb in non-leap years into 1-Mar
        return OM_DATETIME_FROM_YMDHMS(year, month, day, hours, minutes, seconds);
    }
    else
    {
        return 0;
    }
}



#ifndef OmDateTimeToString
/** Convert a date/time number to a string ("YYYY-MM-DD hh:mm:ss") */
char *OmDateTimeToString(OM_DATETIME value, char *buffer)
{
    static char staticBuffer[20] = "YYYY-MM-DD hh:mm:ss";
    char *c = buffer;
    unsigned int v;
    if (buffer == NULL) { buffer = staticBuffer; }
    if (value < OM_DATETIME_MIN_VALID) { *c++ = '0'; *c++ = '\0'; }                // "0"
    else if (value > OM_DATETIME_MAX_VALID) { *c++ = '-'; *c++ = '1'; *c++ = '\0'; }    // "-1"
    else
    {
        v = OM_DATETIME_YEAR(value);    *c++ = '0' + ((v / 1000) % 10); *c++ = '0' + ((v / 100) % 10); *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '-';
        v = OM_DATETIME_MONTH(value);   *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '-';
        v = OM_DATETIME_DAY(value);     *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ' ';
        v = OM_DATETIME_HOURS(value);   *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
        v = OM_DATETIME_MINUTES(value); *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
        v = OM_DATETIME_SECONDS(value); *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10);
        *c++ = '\0';
    }
    return buffer;
}
#endif

