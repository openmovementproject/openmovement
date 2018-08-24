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
OmState om = 
#ifdef _WIN32
	{0};
#else
	{};
#endif


/** Log text to the current log stream. */
int OmLog(int level, const char *format, ...)
{
    int ret = -1;
    if (om.debug < level) { return -1; }
    if (om.log != NULL)
    {
        va_list args;
        va_start(args, format);
        ret = vfprintf(om.log, format, args);
        va_end(args);
		fflush(om.log);
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


OmDeviceState *OmDevice(int serial)
{
	return om.deviceList[serial];
}



#ifdef _WIN32
#ifdef OM_DEBUG_MUTEX
int OmDebugMutexLock(mutex_t *mutex, const char *mutexName, const char *source, int line, const char *caller, int deviceId)
{
    DWORD dwTimeout = 1000;
    OmLog(3, "LOCK: #%d %.0s:%d %s() mutex_lock(%s) called\n", deviceId, source, line, caller, mutexName);
    for(;;)
    {
        HRESULT hr = WaitForSingleObject(*(mutex), dwTimeout);
        const char *status = "<unknown>";
        if (hr == WAIT_OBJECT_0) { break; }

        if (hr == WAIT_TIMEOUT) { status = "WAIT_TIMEOUT"; }
        else if (hr == WAIT_FAILED) { status = "WAIT_FAILED"; }
        else if (hr == WAIT_ABANDONED) { status = "WAIT_ABANDONED"; }

        if (hr == WAIT_OBJECT_0) { break; }
        OmLog(3, "LOCK: #%d %.0s:%d %s() mutex_lock(%s) 0x%8x=%s @%d\n", deviceId, source, line, caller, mutexName, hr, status, dwTimeout);
        dwTimeout *= 2;     // double the timeout each time to prevent log pollution
    }
    OmLog(3, "LOCK: #%d %.0s:%d %s() mutex_lock(%s) complete\n", deviceId, source, line, caller, mutexName);
    return 0;
}

int OmDebugMutexUnlock(mutex_t *mutex, const char *mutexName, const char *source, int line, const char *caller, int deviceId)
{
    OmLog(3, "UNLOCK: #%d %.0s:%d %s() mutex_unlock(%s) called\n", deviceId, source, line, caller, mutexName);
    if (ReleaseMutex(*(mutex)) == 0)
    {
        OmLog(3, "UNLOCK: #%d %.0s:%d %s() mutex_unlock(%s) failed\n", deviceId, source, line, caller, mutexName);
        return 1;
    }
    else
    {
        OmLog(3, "UNLOCK: #%d %.0s:%d %s() mutex_unlock(%s) complete\n", deviceId, source, line, caller, mutexName);
        return 0;
    }
}
#endif
#endif

/** Internal, method for handling device discovery. */
void OmDeviceDiscovery(OM_DEVICE_STATUS status, unsigned int inSerialNumber, const char *serialId, const char *port, const char *volumePath)
{
    if (status == OM_DEVICE_CONNECTED)
    {
        unsigned short serialNumber;
        OmDeviceState *deviceState;

        if (inSerialNumber >= 0xffff0000) { OmLog(0, "WARNING: Ignoring added device with invalid serial number %u\n", inSerialNumber); return; }
        if (volumePath == NULL || volumePath[0] == '\0') { OmLog(0, "WARNING: Ignoring added device with no mount point (%u)\n", inSerialNumber); return; }
        serialNumber = (unsigned short)inSerialNumber;

        // Get the current OmDeviceState structure, or make one if it doesn't exist
        deviceState = om.deviceList[serialNumber];
        if (deviceState == NULL)
        {
            // Create and initialize the structure
            deviceState = (OmDeviceState *)malloc(sizeof(OmDeviceState));
            if (deviceState == NULL)
            {
                OmLog(0, "ERROR: Cannot add new device %u - out of memory.\n", inSerialNumber); 
                return;
            }
            memset(deviceState, 0, sizeof(OmDeviceState));
            deviceState->fd = -1;
        }

OmLog(0, "DEBUG: Device added #%d  %s  %s\n", serialNumber, port, volumePath);

        // Update the OmDeviceState structure
        deviceState->id = serialNumber;
        snprintf(deviceState->port, OM_MAX_CDC_PATH, "%s", port);
        snprintf(deviceState->root, OM_MAX_MSD_PATH, "%s", volumePath);
        snprintf(deviceState->serialId, OM_MAX_SERIALID_LEN, "%s", serialId);

        // Download status
        //deviceState->downloadStatus = OM_DOWNLOAD_NONE;
        //deviceState->downloadValue = 0;

        // Finally, set the connected flag and the entry in devices
        deviceState->deviceStatus = OM_DEVICE_CONNECTED;
        om.deviceList[serialNumber] = deviceState;

        // Call user's device callback
        if (om.deviceCallback != NULL)
        {
OmLog(0, "DEBUG: callback for OM_DEVICE_CONNECTED...\n");
            om.deviceCallback(om.deviceCallbackReference, serialNumber, OM_DEVICE_CONNECTED);
        }
    }
    else if (status == OM_DEVICE_REMOVED)
    {
        unsigned short serialNumber;
        OmDeviceState *deviceState;

		if (inSerialNumber >= 0xffff0000) { OmLog(0, "WARNING: Ignoring removed device with invalid serial number %u\n", inSerialNumber); return; }
		serialNumber = (unsigned short)inSerialNumber;

OmLog(0, "DEBUG: Device removed: #%d\n", serialNumber);

        // Get the current OmDeviceState structure
        deviceState = om.deviceList[serialNumber];
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
OmLog(0, "DEBUG: callback for OM_DEVICE_REMOVED...\n");
            om.deviceCallback(om.deviceCallbackReference, serialNumber, OM_DEVICE_REMOVED);
        }
    }
}


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



#ifdef _WIN32

#define NUM_RETRIES_OPEN 5

/** (Win32-specific) Internal method to open a serial port */
static int OmPortOpen(const char *infile, char writeable)
{
	int retries = 0;
	int fd = -1;        // fd = fileno(stdin);

	if (infile != NULL && infile[0] == '\0') { return -1; }		// Invalid parameter

	// Attempt to open the port, retrying on failure.
	for (;;)
	{
		int flags;

		// If retrying and already open, close the port.
		if (fd >= 0) { close(fd); fd = -1; }

		// If we have any attempts left
		if (retries > NUM_RETRIES_OPEN) { break; }
		else if (retries > 0) { OmLog(0, "NOTE: Retry %d to open: %s\n", retries, infile); }
		retries++;

		//OmLog(0, "Opening with retries: %s\n", infile);
		flags = O_BINARY;
		flags |= (writeable) ? O_RDWR : O_RDONLY;
		fd = open(infile, flags);
		if (fd < 0)
		{
			OmLog(0, "ERROR: Problem opening input (%s: %d): %s\n", (errno == ENOENT ? "ENOENT" : (errno == EACCES ? "EACCES" : "other: ")), errno, infile);
			continue;
		}

		/* Set serial port parameters (OS-specific) */
		{
			HANDLE hSerial;
			DCB dcbSerialParams = { 0 };
			COMMTIMEOUTS timeouts = { 0 };
			char dcbOk;

			hSerial = (HANDLE)_get_osfhandle(fd);
			if (hSerial == INVALID_HANDLE_VALUE)
			{
				OmLog(0, "WARNING: Failed to get HANDLE from file: %s\n", infile);
				continue;		// retry
			}

			// Always call ClearCommError() here
			{
				DWORD comErrors = 0;
				COMSTAT comStat = { 0 };
				ClearCommError(hSerial, &comErrors, &comStat);
			}

			if (SetupComm(hSerial, 4096, 4096) == 0)
			{
				OmLog(0, "WARNING: SetupComm() failed: %s\n", infile);
				continue;		// retry
			}

			if (PurgeComm(hSerial, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR) == 0)
			{
				OmLog(0, "WARNING: PurgeComm() failed.\n");
				continue;		// retry
			}

			if (ClearCommBreak(hSerial) == 0)
			{
				//OmLog(0, "NOTE: ClearCommBreak() failed.\n");
			}

			dcbOk = 0;
			dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
			if (GetCommState(hSerial, &dcbSerialParams)) { dcbOk = 1; }

			if (!dcbOk)
			{
				// If GetCommState() failed with error 995, call ClearCommError() before retrying
				DWORD comErrors = 0;
				COMSTAT comStat = { 0 };
				OmLog(0, "WARNING: GetCommState() failed (clearing errors and retrying this): %s\n", infile);
				ClearCommError(hSerial, &comErrors, &comStat);
				if (GetCommState(hSerial, &dcbSerialParams)) { dcbOk = 1; }
			}

			if (!dcbOk)
			{
				OmLog(0, "WARNING: Retrying GetCommState() failed: %s\n", infile);
				continue;		// retry
			}

			//dcbSerialParams.BaudRate = CBR_115200;
			dcbSerialParams.fBinary = TRUE;
			dcbSerialParams.fParity = FALSE;
			dcbSerialParams.fOutxCtsFlow = FALSE;
			dcbSerialParams.fOutxDsrFlow = FALSE;
			dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
			dcbSerialParams.fDsrSensitivity = FALSE;
			dcbSerialParams.fTXContinueOnXoff = FALSE;
			dcbSerialParams.fOutX = FALSE;
			dcbSerialParams.fInX = FALSE;
			dcbSerialParams.fErrorChar = FALSE;
			dcbSerialParams.fNull = FALSE;
			dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;
			dcbSerialParams.fAbortOnError = 0;  // SetDcbFlag bit 14 to zero (clears fAbortOnError).
			//dcbSerialParams.fDummy2 = <do-not-use>;
			dcbSerialParams.wReserved = 0;
			dcbSerialParams.XonLim = 0;
			dcbSerialParams.XoffLim = 0;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.Parity = NOPARITY;
			dcbSerialParams.StopBits = ONESTOPBIT;
			//dcbSerialParams.XonChar;
			//dcbSerialParams.XoffChar;
			dcbSerialParams.ErrorChar = 0;
			//dcbSerialParams.EofChar;
			//dcbSerialParams.EvtChar;
			//dcbSerialParams.wReserved1;

			if (!SetCommState(hSerial, &dcbSerialParams))
			{
				// If SetCommState() failed with error 995, call ClearCommError() before retrying
				DWORD comErrors = 0;
				COMSTAT comStat = { 0 };
				OmLog(0, "WARNING: SetCommState() failed (clearing errors and retrying this): %s\n", infile);
				ClearCommError(hSerial, &comErrors, &comStat);
				if (SetCommState(hSerial, &dcbSerialParams))
				{
					OmLog(0, "WARNING: Retrying SetCommState() failed: %s\n", infile);
					continue;		// retry
				}
			};

			//PurgeComm(PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
			//ClearCommBreak(hFile);
			//FlushFileBuffers();

			timeouts.ReadIntervalTimeout = 0;
#define TIMEOUT_CONSTANT 20
#ifdef TIMEOUT_CONSTANT
			timeouts.ReadTotalTimeoutConstant = TIMEOUT_CONSTANT;
#else
			timeouts.ReadTotalTimeoutConstant = 20;
#endif
			timeouts.ReadTotalTimeoutMultiplier = 0;
			timeouts.WriteTotalTimeoutConstant = 2500;
			timeouts.WriteTotalTimeoutMultiplier = 0;
			if (!SetCommTimeouts(hSerial, &timeouts))
			{
				OmLog(0, "WARNING: SetCommTimeouts() failed: %s\n", infile);
				continue;		// retry
			}

			//GetCommProperties();

		}

		// Everything went OK, return handle.
		return fd;
	}

	// Failed, even after retries
	OmLog(0, "ERROR: Open failed: %s\n", infile);
	return -1;
}

#else

/** (Non-Windows) Internal method to open a serial port */
static int OmPortOpen(const char *infile, char writeable)
{
	int fd = -1;        // fd = fileno(stdin);
	int flags;

	if (infile == NULL && infile[0] == '\0') { return -1; }

	flags = O_NOCTTY | O_NDELAY;
	flags |= (writeable) ? O_RDWR : O_RDONLY;

	fd = open(infile, flags);
	if (fd < 0)
	{
		if (errno == ENOENT) { OmLog(0, "ERROR: Problem opening input (ENOENT): %s\n", infile); }
		else if (errno == EACCES) { OmLog(0, "ERROR: Problem opening input (EACCES): %s\n", infile); }
		else { OmLog(0, "ERROR: Problem opening input (other): %s\n", infile); }
		return -1;
	}

	/* Set serial port parameters (OS-specific) */
	fcntl(fd, F_SETFL, 0);    /* Clear all descriptor flags */
	/* Set the port options */
	{
		struct termios options;
		tcgetattr(fd, &options);
		options.c_cflag = (options.c_cflag | CLOCAL | CREAD | CS8) & ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
		options.c_lflag &= ~(ICANON | ECHO | ISIG); /* Enable data to be processed as raw input */
		tcsetattr(fd, TCSANOW, &options);
		// #warning "Could we set up serial port timeout values?"
	}

	return fd;
}

#endif



/** Internal method to read a line from the device */
int OmPortReadLine(unsigned int deviceId, char *inBuffer, int len, unsigned long timeout)
{
    unsigned long start = OmMilliseconds();
    int received = 0;
    unsigned char cc;
    int c;
    int fd;
	unsigned long elapsed = 0;

    OmLog(3, "OmPortReadLine(%d, _,%d, %d);", deviceId, len, timeout);

	OmDeviceState *device = OmDevice(deviceId);
	if (device == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    fd = device->fd;
    if (fd < 0) { return -1; }
    if (inBuffer != NULL) { inBuffer[0] = '\0'; }
    for (;;)
    {
        // Read character
        c = -1; cc = 0;
        if (read(fd, &cc, 1) == 1) { c = cc; }

        // If timeout (or NULL)
        if (c <= 0)
        {
#if defined(_WIN32) && defined(TIMEOUT_CONSTANT)
			elapsed += TIMEOUT_CONSTANT;
#else
			elapsed = OmMilliseconds() - start;
#endif

OmLog(3, "-T/O(%d/%d)", elapsed, timeout);

			if (timeout > 0 && elapsed > timeout)
			{
OmLog(3, "- Overall timeout > %d", timeout);
				return -1; 
			}

            continue;   // try to read again until timeout
        }
        else if (c == '\r' || c == '\n')
        {
OmLog(5, "[CRLF]", c);    // For extreme logging
            if (received > 0)
            {
OmLog(3, "- Done (CRLF), %d bytes", received);
                return received;
            }
        }
        else
        {
OmLog(5, "[%c]", c);    // For extreme logging
            if (received < len - 1)
            {
                if (inBuffer != NULL)
                { 
                    inBuffer[received] = (char)c; 
                    inBuffer[received + 1] = '\0';
                }
                received++;
            }
        }
    }
}


/** Internal method to write to the device */
int OmPortWrite(unsigned int deviceId, const char *command)
{
    int fd;
	OmDeviceState *device = OmDevice(deviceId);
	if (device == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    fd = device->fd;
    if (fd < 0) { return OM_E_FAIL; }
    if (command == NULL) { return OM_E_POINTER; }
OmLog(3, "OmPortWrite(%d, \"%s\");\n", deviceId, command);
    if (write(fd, command, (int)strlen(command)) != (int)strlen(command)) { return OM_E_FAIL; }
    return OM_OK;
}


/** Internal method to safely acquire an open serial port for a device. */
int OmPortAcquire(unsigned int deviceId)
{
    int status;

    // Check system and device state
    if (!om.initialized) return OM_E_NOT_VALID_STATE;
	OmDeviceState *device = OmDevice(deviceId);
	if (device == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    if (device->deviceStatus != OM_DEVICE_CONNECTED) return OM_E_INVALID_DEVICE;   // Device lost

    // Open port
    mutex_lock(&om.portMutex);              // Lock port mutex to open the port
    do          // This is only a 'do' to allow a single code path to hit the mutex unlock with any exceptional 'break's
    {
        // Check if already open
        if (device->fd >= 0) { status = OM_E_ACCESS_DENIED; break; }

        // Open the port
        device->fd = OmPortOpen(device->port, 1);

        // Check if opened successfully
        if (device->fd < 0) { status = OM_E_ACCESS_DENIED; break; }

        status = OM_OK;
    } while (0);
    mutex_unlock(&om.portMutex);            // Release port mutex after opening the port

    return status;
}


/** Internal method to safely release a serial port for a device. */
int OmPortRelease(unsigned int deviceId)
{
    // Check device state
	OmDeviceState *device = OmDevice(deviceId);
	if (device == NULL) return OM_E_INVALID_DEVICE;   // Device never seen

    // Close port
    mutex_lock(&om.portMutex);              // Lock port mutex to close the port
    if (device->fd >= 0)
    { 
        close(device->fd);
        device->fd = -1;
    }
    mutex_unlock(&om.portMutex);            // Release port mutex after closing the port
    return OM_OK;
}



