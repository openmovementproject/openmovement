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
void OmDeviceDiscovery(OM_DEVICE_STATUS status, unsigned int inSerialNumber, const char *port, const char *volumePath)
{
    if (status == OM_DEVICE_CONNECTED)
    {
        unsigned short serialNumber;
        OmDeviceState *deviceState;

        if (inSerialNumber > OM_MAX_SERIAL) { OmLog(0, "WARNING: Ignoring added device with invalid serial number %u\n", inSerialNumber); return; }
        if (volumePath == NULL || volumePath[0] == '\0') { OmLog(0, "WARNING: Ignoring added device with no mount point (%u)\n", inSerialNumber); return; }
        serialNumber = (unsigned short)inSerialNumber;

        // Get the current OmDeviceState structure, or make one if it doesn't exist
        deviceState = om.devices[serialNumber];
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

OmLog(0, "DEBUG: Device added %d  %s  %s\n", serialNumber, port, volumePath);

        // Update the OmDeviceState structure
        deviceState->id = serialNumber;
        snprintf(deviceState->port, OM_MAX_CDC_PATH, "%s", port);
        snprintf(deviceState->root, OM_MAX_MSD_PATH, "%s", volumePath);

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

        if (inSerialNumber > OM_MAX_SERIAL) { OmLog(0, "WARNING: Ignoring removed device with invalid serial number %u\n", inSerialNumber); return; }
        serialNumber = (unsigned short)inSerialNumber;

OmLog(0, "DEBUG: Device removed: %d\n", serialNumber);

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

/** Internal, extract device id from serial id **/
int GetDeviceId(char *serial_id) {
    int value = 0;
    if(serial_id) {
        while(*serial_id != '\0' && *serial_id != '_') {
            serial_id++;
        }
        serial_id++;
        //convert rest to integer
        while(*serial_id != '\0' && *serial_id <= '9' && *serial_id >= '0') {
            value = (value * 10) + (*serial_id - '0');
            serial_id++;
        }
    }
    return value;
}

/** Internal, method to add a device to the device list **/
void AddDevice(char *block_device, char *mount_path, char *serial_device, int id) {
    deviceNode *dev = (deviceNode *)malloc(sizeof(deviceNode));
    strcpy(dev->block_device, block_device);
    strcpy(dev->mount_path, mount_path);
    strcpy(dev->serial_device, serial_device);
    dev->device_id = id;
    dev->next = NULL;
    
    OmDeviceDiscovery(OM_DEVICE_CONNECTED, dev->device_id, dev->serial_device, dev->mount_path);

    if(deviceList == NULL) {
        deviceList = dev;
    } else {
        dev->next = deviceList;
        deviceList = dev;
    }
}

/** Internal, method to remove a device from the device list **/
void RemoveDevice(char *block_device) 
{
    deviceNode *current;
    deviceNode *previous = NULL;

    for(current = deviceList; current != NULL; previous = current, current = current->next) {
        
        //found device
        if(strcmp(current->block_device, block_device) == 0) {
            
            OmDeviceDiscovery(OM_DEVICE_REMOVED, current->device_id, current->serial_device, current->mount_path);
            if(previous == NULL) {
                deviceList = current->next;
            } else {
                previous->next = current->next;
            }

            free(current);
            
            return;
        }
    }
}

/** Internal, method for getting devices already plugged in **/
void InitDeviceFinder() 
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    deviceList = NULL;
    
    udev = udev_new();
    if(!udev) {
        printf("Can't create udev\n");
        exit(1);
    }

    //find block devices with vendor id and product id.
    enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_add_match_property(enumerate, "ID_PRODUCT_ID", "0057");
    udev_enumerate_add_match_property(enumerate, "ID_VENDOR_ID", "04d8");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        strcpy(path, udev_device_get_devnode(dev));

        //get mount path first.
        char mount_path[100];
        strcpy(mount_path, "/media/");
        char *name = udev_device_get_property_value(dev, "ID_FS_LABEL");
        if(name != NULL) {
            dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
            if(dev) {
               
                //get serial device path.
                char serial_device[100];
                char *serial_id = udev_device_get_sysattr_value(dev, "serial");
                GetSerialDevice(serial_id, serial_device);
                if(strlen(serial_device) > 0) {
                    strcat(mount_path, name);
                    //now save device info.
                    AddDevice(path, mount_path, serial_device, GetDeviceId(serial_id));
                }
            }
        }
    }
    
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}


/** Internal, method for getting serial device by id **/
void GetSerialDevice(char *serial_id, char *serial_device)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    udev = udev_new();
    if(!udev) {
        printf("Can't create udev\n");
        exit(1);
    }

    //find serial devices with vendor id and product id and serial id.
    enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "tty");
    udev_enumerate_add_match_property(enumerate, "ID_SERIAL_SHORT", serial_id);
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        strcpy(serial_device, udev_device_get_devnode(dev));
        break;
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}


/** Internal, method for updating a list of seen devices. */
thread_return_t OmDeviceDiscoveryThread(void *arg)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_device *dev;
    struct udev_monitor *mon;

    udev = udev_new();

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
    udev_monitor_enable_receiving(mon);


    // Polled device discovery for non-Windows
    while (!om.quitDiscoveryThread)
    {
        dev = udev_monitor_receive_device(mon);
        if(dev) {

            //get block device name.
            struct udev_device *usb_dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

            //make sure it's one of our devices.
            if(strcmp(udev_device_get_sysattr_value(usb_dev, "idVendor"), "04d8") == 0 && strcmp(udev_device_get_sysattr_value(usb_dev, "idProduct"), "0057") == 0) {

                //get block device name.
                char *block_device = udev_device_get_devnode(dev);

                //check type of action (remove or add)
                char *action = udev_device_get_action(dev);

                if(strcmp(action, "add") == 0)  {
                    //get mount path.
                    char mount_path[100];
                    strcpy(mount_path, "/media/");
                    char *name = udev_device_get_property_value(dev, "ID_FS_LABEL");
                    if(name != NULL) {
                        char serial_device[100];
                        char *serial_id = udev_device_get_sysattr_value(usb_dev, "serial");
                        GetSerialDevice(serial_id, &serial_device[0]);
                        
                        if(strlen(serial_device) > 0) {
                            strcat(mount_path, name);
                            AddDevice(&block_device[0], &mount_path[0], &serial_device[0], GetDeviceId(serial_id));
                        }
                    }
                } else if(strcmp(action, "remove") == 0) {
                    RemoveDevice(block_device);
                }
            }
        }
    }
 
    udev_unref(udev);
    return thread_return_value(0);
}

/** Internal method to start device discovery. */
void OmDeviceDiscoveryStart(void)
{
    InitDeviceFinder();
    // Perform an initial device discovery and create device discovery thread
    om.quitDiscoveryThread = 0;
    //OmUpdateDevices();
    thread_create(&om.discoveryThread, NULL, OmDeviceDiscoveryThread, NULL);
}

/** Internal method to stop device discovery. */
void OmDeviceDiscoveryStop(void)
{
    om.quitDiscoveryThread = 1;
    thread_cancel(&om.discoveryThread);     // thread_join(&om.discoveryThread, NULL);
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
    int fd = -1;        // fd = fileno(stdin);

    if (infile != NULL && infile[0] != '\0')
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
            if (errno == ENOENT) { OmLog(0, "ERROR: Problem opening input (ENOENT): %s\n", infile); }
            else if (errno == EACCES) { OmLog(0, "ERROR: Problem opening input (EACCES): %s\n", infile); }
            else { OmLog(0, "ERROR: Problem opening input (other): %s\n", infile); }
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
                OmLog(0, "ERROR: Failed to get HANDLE from file.\n");
            }
            else
            {
                char dcbOk = 0;
                dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

                if (GetCommState(hSerial, &dcbSerialParams)) { dcbOk = 1; }

                if (!dcbOk)
                {
                    // If GetCommState() failed with error 995, call ClearCommError() before retrying
                    DWORD comErrors = 0;
                    COMSTAT comStat = {0};
                    OmLog(0, "WARNING: GetCommState() failed (clearing errors and retrying).\n");
                    ClearCommError(hSerial, &comErrors, &comStat);
                    if (GetCommState(hSerial, &dcbSerialParams)) { dcbOk = 1; }
                }

                if (!dcbOk)
                {
                    OmLog(0, "ERROR: GetCommState() failed.\n");
                }
                else
                {
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
                        COMSTAT comStat = {0};
                        OmLog(0, "WARNING: SetCommState() failed (clearing errors and retrying).\n");
                        ClearCommError(hSerial, &comErrors, &comStat);
                        if (SetCommState(hSerial, &dcbSerialParams))
                        {
                            OmLog(0, "ERROR: SetCommState() failed.\n");
                        }
                    };
                }

                timeouts.ReadIntervalTimeout = 0;
                timeouts.ReadTotalTimeoutConstant = 20;
                timeouts.ReadTotalTimeoutMultiplier = 0;
                timeouts.WriteTotalTimeoutConstant = 2500;
                timeouts.WriteTotalTimeoutMultiplier = 0;
                if (!SetCommTimeouts(hSerial, &timeouts))
                {
                    OmLog(0, "ERROR: SetCommTimeouts() failed.\n");
                }

//GetCommProperties();

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
    unsigned char cc;
    int c;
    int fd;

    OmLog(3, "OmPortReadLine(%d, _,%d, %d);", deviceId, len, timeout);

    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    fd = om.devices[deviceId]->fd;
    if (fd < 0) { return -1; }
    if (inBuffer != NULL) { inBuffer[0] = '\0'; }
    for (;;)
    {
        unsigned long elapsed;

        // Read character
        c = -1; cc = 0;
        if (read(fd, &cc, 1) == 1) { c = cc; }

        elapsed = OmMilliseconds() - start;

        if (timeout > 0 && elapsed > timeout)
        {
OmLog(3, "- Overall timeout > %d", timeout);
            return -1; 
        }

        // If timeout (or NULL)
        if (c <= 0)
        {
OmLog(3, "-T/O(%d/%d)", elapsed, timeout);
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
int OmPortWrite(unsigned short deviceId, const char *command)
{
    int fd;
    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen
    fd = om.devices[deviceId]->fd;
    if (fd < 0) { return OM_E_FAIL; }
    if (command == NULL) { return OM_E_POINTER; }
OmLog(3, "OmPortWrite(%d, \"%s\");\n", deviceId, command);
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
    mutex_lock(&om.portMutex);              // Lock port mutex to open the port
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
    mutex_unlock(&om.portMutex);            // Release port mutex after opening the port

    return status;
}


/** Internal method to safely release a serial port for a device. */
int OmPortRelease(unsigned short deviceId)
{
    // Check device state
    if (om.devices[deviceId] == NULL) return OM_E_INVALID_DEVICE;   // Device never seen

    // Close port
    mutex_lock(&om.portMutex);              // Lock port mutex to close the port
    if (om.devices[deviceId]->fd >= 0)
    { 
        close(om.devices[deviceId]->fd);
        om.devices[deviceId]->fd = -1;
    }
    mutex_unlock(&om.portMutex);            // Release port mutex after closing the port
    return OM_OK;
}



