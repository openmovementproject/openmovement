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

// Open Movement API - Internal Functions
// Dan Jackson, 2011-2012

#include "omapi-internal.h"

//#define DEBUG_COMMANDS


/** The state of the API */
OmState om = {0};


/** Log text to the current log stream. */
int OmLog(const char *format, ...)
{
    int ret = -1;
    if (om.log != NULL)
    {
        va_list args;
        va_start(args, format);
        ret = vfprintf(om.log, format, args);
        va_end(args);
    }
    if (om.logCallback != NULL)
    {
        char message[512];
        va_list args;
        message[0] = '\0';
        va_start(args, format);
        ret = vsnprintf(message, sizeof(message) - 1, format, args);
        va_end(args);
        om.logCallback(om.logCallbackReference, message);
    }
    return ret;
}


/** Internal, method for handling device discovery. */
void OmDeviceDiscovery(OM_DEVICE_STATUS status, unsigned int inSerialNumber, const char *port, const char *volumePath)
{
    if (status == OM_DEVICE_CONNECTED)
    {
        unsigned short serialNumber;
        OmDeviceState *deviceState;

        if (inSerialNumber > OM_MAX_SERIAL) { OmLog("WARNING: Ignoring added device with invalid serial number %u\n", inSerialNumber); return; }
        serialNumber = (unsigned short)inSerialNumber;

        // Get the current OmDeviceState structure, or make one if it doesn't exist
        deviceState = om.devices[serialNumber];
        if (deviceState == NULL)
        {
            // Create and initialize the structure
            deviceState = (OmDeviceState *)malloc(sizeof(OmDeviceState));
            if (deviceState == NULL)
            {
                OmLog("WARNING: Cannot add new device %u - out of memory.\n", inSerialNumber); 
                return;
            }
            memset(deviceState, 0, sizeof(OmDeviceState));
            deviceState->fd = -1;
        }

OmLog("DEBUG: Device added %d  %s  %s\n", serialNumber, port, volumePath);

        // Update the OmDeviceState structure
        deviceState->id = serialNumber;
        sprintf_s(deviceState->port, OM_MAX_CDC_PATH, "%s", port);
        sprintf_s(deviceState->root, OM_MAX_MSD_PATH, "%s", volumePath);

        // Download status
        //deviceState->downloadStatus = OM_DOWNLOAD_NONE;
        //deviceState->downloadValue = 0;

        // Finally, set the connected flag and the entry in devices
        deviceState->deviceStatus = OM_DEVICE_CONNECTED;
        om.devices[serialNumber] = deviceState;

        // Call user's device callback
        if (om.deviceCallback != NULL)
        {
            om.deviceCallback(om.deviceCallbackReference, serialNumber, OM_DEVICE_CONNECTED);
        }
    }
    else if (status == OM_DEVICE_REMOVED)
    {
        unsigned short serialNumber;
        OmDeviceState *deviceState;

        if (inSerialNumber > OM_MAX_SERIAL) { OmLog("WARNING: Ignoring removed device with invalid serial number %u\n", inSerialNumber); return; }
        serialNumber = (unsigned short)inSerialNumber;

OmLog("DEBUG: Device removed: %d\n", serialNumber);

        // Get the current OmDeviceState structure
        deviceState = om.devices[serialNumber];
        if (deviceState == NULL) { return; }        // Removal called for a never-seen device (should not be possible from the DeviceFinder)

        // Set the removed status
        deviceState->deviceStatus = OM_DEVICE_REMOVED;

        // Port
        //deviceState->portMutex;
        //deviceState->fd = -1;

        // Download
        //deviceState->downloadStatus = OM_DOWNLOAD_NONE;
        //deviceState->downloadValue = 0;
        OmCancelDownload(serialNumber);

        // Call user's device callback
        if (om.deviceCallback != NULL)
        {
            om.deviceCallback(om.deviceCallbackReference, serialNumber, OM_DEVICE_REMOVED);
        }
    }
}



#if !defined(_WIN32)

/** Internal, method for updating a list of seen devices. */
int OmUpdateDevices(void)
{
    #warning "No non-Win32 device discovery implementation\n"

    // Example
    OmDeviceDiscovery(OM_DEVICE_CONNECTED, 1, "/dev/tty.usbmodem", "/dev/disk1");
    return OM_E_FAIL;
}

/** Internal, thread for discovering devices. */
thread_return_t OmDeviceDiscoveryThread(void *arg)
{
    // Polled device discovery for non-Windows
    while (!om.quitDiscoveryThread)
    {
        int status;
        status = OmUpdateDevices();
        if (status != OM_OK)
        {
            OmLog("ERROR: OmDeviceDiscoveryThread() - %s\n", OmErrorString(status));
        }
        sleep(10);       // Sleep (seconds)
    }
    return thread_return_value(0);
}

/** Internal method to start device discovery. */
void OmDeviceDiscoveryStart(void)
{
    // Perform an initial device discovery and create device discovery thread
    om.quitDiscoveryThread = 0;
    OmUpdateDevices();
    thread_create(&om.discoveryThread, NULL, OmDeviceDiscoveryThread, NULL);
}

/** Internal method to stop device discovery. */
void OmDeviceDiscoveryStop(void)
{
    om.quitDiscoveryThread = 1;
    thread_join(&om.discoveryThread, NULL);
}

#endif


/** Internal method to obtain the number of milliseconds since the epoch. */
unsigned long long OmMillisecondsEpoch(void)
{
    struct timeb tp;
    ftime(&tp);
    return (unsigned long long)tp.time * 1000 + tp.millitm;
}


/** Internal method to obtain a timer value in milliseconds. */
unsigned long OmMilliseconds(void)
{
    return (unsigned long)OmMillisecondsEpoch();
}


/** Internal method to open a serial port */
static int OmPortOpen(const char *infile, char writeable)
{
    int fd;
    
    fd = fileno(stdin);
    if (infile != NULL && infile[0] != '\0' && !(infile[0] == '-' && infile[1] == '\0'))
    {
#ifdef _WIN32
        int flags = O_BINARY;
#else
        int flags = O_NOCTTY | O_NDELAY;
#endif
        flags |= (writeable) ? O_RDWR : O_RDONLY;

        fd = open(infile, flags);
        if (fd < 0)
        {
            OmLog("ERROR: Problem opening input: %s\n", infile);
            return -1;
        }

        /* Set serial port parameters (OS-specific) */
#ifdef _WIN32
        {
            HANDLE hSerial;
            DCB dcbSerialParams = {0};
            COMMTIMEOUTS timeouts = {0};

            hSerial = (HANDLE)_get_osfhandle(fd);
            if (hSerial == INVALID_HANDLE_VALUE)
            {
                OmLog("ERROR: Failed to get HANDLE from file.\n");
            }
            else
            {
                dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
                if (!GetCommState(hSerial, &dcbSerialParams))
                {
                    OmLog("ERROR: GetCommState() failed.\n");
                }
                else
                {
                    //dcbSerialParams.BaudRate = CBR_115200;
                    dcbSerialParams.ByteSize = 8;
                    dcbSerialParams.StopBits = ONESTOPBIT;
                    dcbSerialParams.Parity = NOPARITY;
                    if (!SetCommState(hSerial, &dcbSerialParams)){
                        OmLog("ERROR: SetCommState() failed.\n");
                    };
                }

                timeouts.ReadIntervalTimeout = 0;
                timeouts.ReadTotalTimeoutConstant = 20;
                timeouts.ReadTotalTimeoutMultiplier = 0;
                timeouts.WriteTotalTimeoutConstant = 2500;
                timeouts.WriteTotalTimeoutMultiplier = 0;
                if (!SetCommTimeouts(hSerial, &timeouts))
                {
                    OmLog("ERROR: SetCommTimeouts() failed.\n");
                }
            }
        }
#else
        fcntl(fd, F_SETFL, 0);    /* Clear all descriptor flags */
        /* Set the port options */
        {
            struct termios options;
            tcgetattr(fd, &options);
            options.c_cflag = (options.c_cflag | CLOCAL | CREAD | CS8) & ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
            options.c_lflag &= ~(ICANON | ECHO | ISIG); /* Enable data to be processed as raw input */
            tcsetattr(fd, TCSANOW, &options);
#warning "Must set up serial port timeout values"
        }
#endif

    }
    return fd;
}


/** Internal method to read a line from the device */
int OmPortReadLine(unsigned short deviceId, char *inBuffer, int len, unsigned long timeout)
{
    unsigned long start = OmMilliseconds();
    int received = 0;
    unsigned char c;
    int fd;

    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    fd = om.devices[deviceId]->fd;
    if (fd < 0) { return -1; }
    if (inBuffer != NULL) { inBuffer[0] = '\0'; }
    for (;;)
    {
        c = -1;
        read(fd, &c, 1);

        if (timeout > 0 && OmMilliseconds() - start > timeout)
        {
            return -1; 
        }

        // If timeout or NULL
        if (c <= 0)
        {
            continue;   // try to read again until timeout
        }
        else if (c == '\r' || c == '\n')
        {
            if (received > 0) { return received; }
        }
        else
        {
            if (received < len - 1)
            {
                if (inBuffer != NULL)
                { 
                    inBuffer[received] = (char)c; 
                    inBuffer[received + 1] = '\0'; 
                }
            }
            received++;
        }
    }
}


/** Internal method to write to the device */
int OmPortWrite(unsigned short deviceId, const char *command)
{
    int fd;
    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    fd = om.devices[deviceId]->fd;
    if (fd < 0) { return OM_E_FAIL; }
    if (command == NULL) { return OM_E_POINTER; }
    if (write(fd, command, strlen(command)) != strlen(command)) { return OM_E_FAIL; }
    return OM_OK;
}


/** Internal method to safely acquire an open serial port for a device. */
int OmPortAcquire(unsigned short deviceId)
{
    int status;

    // Check system and device state
    if (!om.initialized) return OM_E_NOT_VALID_STATE;
    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    if (om.devices[deviceId]->deviceStatus != OM_DEVICE_CONNECTED) return OM_E_INVALID_DEVICE;   // Device lost

    // Open port
    mutex_lock(&om.portMutex);
    do          // This is only a 'do' to allow a single code path to hit the mutex unlock with any exceptional 'break's
    {
        // Check if already open
        if (om.devices[deviceId]->fd >= 0) { status = OM_E_ACCESS_DENIED; break; }

        // Open the port
        om.devices[deviceId]->fd = OmPortOpen(om.devices[deviceId]->port, 1);

        // Check if opened successfully
        if (om.devices[deviceId]->fd < 0) { status = OM_E_ACCESS_DENIED; break; }

        status = OM_OK;
    } while (0);
    mutex_unlock(&om.portMutex);        // Release the mutex

    return status;
}


/** Internal method to safely release a serial port for a device. */
int OmPortRelease(unsigned short deviceId)
{
    // Check device state
    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen

    // Close port
    mutex_lock(&om.portMutex);          // Acquire the mutex
    if (om.devices[deviceId]->fd >= 0)
    { 
        close(om.devices[deviceId]->fd);
        om.devices[deviceId]->fd = -1;
    }
    mutex_unlock(&om.portMutex);        // Release the mutex
    return OM_OK;
}



