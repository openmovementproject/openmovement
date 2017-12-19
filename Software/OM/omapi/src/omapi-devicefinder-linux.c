/* 
 * Copyright (c) 2009-, Newcastle University, UK.
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

// Open Movement API - Device Discovery (Linux)
// Dan Jackson, 2011-

#if defined(__linux__)

#include "omapi-internal.h"

#include <libudev.h> // sudo apt-get install libudev-dev

// User must be a member of the 'dialout' group (or sudo chmod 666 /dev/ttyACM0)
// sudo usermod -a -G dialout $USER

typedef struct {
    char base_device[256];      // "/dev/sdb"
    char block_device[256];     // "/dev/sdb1"
    char mount_path[256];       // "/media/AX317_?????"
    char serial_device[256];    // "/dev/ttyACM0"
    int device_id;              // 0-65535
    struct deviceNode *next;    // Linked list
} deviceNode;

static deviceNode *deviceList;


/** Internal, extract device id from serial id **/
static int GetDeviceId(char *serial_id)
{
    int value = 0;
    if (serial_id)
    {
        while (*serial_id != '\0' && *serial_id != '_')
        {
            serial_id++;
        }
        serial_id++;
        // Convert rest to integer
        while (*serial_id != '\0' && *serial_id <= '9' && *serial_id >= '0')
        {
            value = (value * 10) + (*serial_id - '0');
            serial_id++;
        }
    }
    return value;
}

/** Internal, method to add a device to the device list **/
static void AddDevice(char *block_device, char *mount_path, char *serial_device, int id)
{
    deviceNode *dev = NULL;

    // Find existing node
    for (deviceNode *current = deviceList; current != NULL; current = current->next)
    {
        if (current->device_id == id)
        {
            dev = current;
            break;
        }
    }

    // If not found, create new node
    if (dev == NULL)
    {
// printf("DEVICENODE: Creating...\n");
        dev = (deviceNode *)malloc(sizeof(deviceNode));
        memset(dev, 0, sizeof(deviceNode));
        dev->next = deviceList;
        deviceList = dev;

        // Set details
        strcpy(dev->block_device, block_device);
        strcpy(dev->mount_path, mount_path);
        strcpy(dev->serial_device, serial_device);
        dev->device_id = id;

        // Build base device from drive path (not partition) -- remove trailing digits
        strcpy(dev->base_device, block_device);
        for (char *p = dev->base_device + strlen(dev->base_device) - 1; p >= dev->base_device; p--) {
            if (*p >= '0' && *p <= '9') {
                *p = '\0';
            } else {
                break;
            }
        }
        
        OmDeviceDiscovery(OM_DEVICE_CONNECTED, dev->device_id, dev->serial_device, dev->mount_path);
    }
    else
    {
        ; // printf("DEVICENODE: Updating existing...\n");
    }

}

/** Internal, method to remove a device from the device list **/
static void RemoveDevice(char *base_device) 
{
    deviceNode *current;
    deviceNode *previous = NULL;

    for (current = deviceList; current != NULL; previous = current, current = current->next)
    {
        // Found device
        if (strcmp(current->base_device, base_device) == 0)
        {
            OmDeviceDiscovery(OM_DEVICE_REMOVED, current->device_id, current->serial_device, current->mount_path);
            if (previous == NULL)
            {
                deviceList = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            free(current);
            return;
        }
    }
}

/** Internal, method for getting serial device by id **/
static void GetSerialDevice(char *serial_id, char *serial_device)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    udev = udev_new();
    if (!udev)
    {
        printf("ERROR: Can't create udev (serial)\n");
        exit(1);
    }

    // Find serial devices with vendor id and product id and serial id.
    enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "tty");
    udev_enumerate_add_match_property(enumerate, "ID_SERIAL_SHORT", serial_id);
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        strcpy(serial_device, udev_device_get_devnode(dev));
        break;
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

/** Internal, method for getting devices already plugged in **/
static void InitDeviceFinder() 
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    deviceList = NULL;
    
    udev = udev_new();
    if (!udev)
    {
        printf("ERROR: Can't create udev (init)\n");
        exit(1);
    }

    // Find block devices with vendor id and product id.
    enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_add_match_property(enumerate, "ID_PRODUCT_ID", "0057");
    udev_enumerate_add_match_property(enumerate, "ID_VENDOR_ID", "04d8");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *syspath;
        syspath = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, syspath);

        const path[256];
        strcpy(path, udev_device_get_devnode(dev));

        // Get mount path
        char mount_path[256];
        strcpy(mount_path, "/media/");
        char *name = udev_device_get_property_value(dev, "ID_FS_LABEL");
        if (name != NULL)
        {
            dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
            if (dev)
            {
                // Get serial device path
                char serial_device[256];
                char *serial_id = udev_device_get_sysattr_value(dev, "serial");
                GetSerialDevice(serial_id, serial_device);
                if (strlen(serial_device) > 0)
                {
                    int id = GetDeviceId(serial_id);
                    strcat(mount_path, name);
// printf("SYSPATH: %s\n", syspath);
// printf("DEVNODE: %s\n", path);
// printf("MOUNT: %s\n", mount_path);
// printf("SERIAL: %s\n", serial_device);
// printf("ID: %d\n", id);
                    // Save device info
                    AddDevice(path, mount_path, serial_device, id);
                }
            }
        }
    }
    
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

static unsigned int timestamp()
{
    struct timeb timer;
    ftime(&timer);
    return (unsigned int)(timer.time * 1000 + timer.millitm);
}

// HACK: Wait until a serial device path is actually readable (with timeout)
// ...then try to read a line (Linux sends "~\xf0~~x\xf0~\r", thinks it's a modem?)
// ...takes about 3 seconds on Ubuntu 16 with X in a VM
static char WaitUntilReadable(const char *path)
{
// printf("OPENING: %s: ", path);
    for (int tries = 0; tries < 10 * 4; tries++)
    {
// printf("%d;", tries);
        int fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
        if (fd >= 0)
        {
            // int flags = fcntl(fd, F_GETFL, 0);
            // if (flags == -1) { flags = 0; }
            // fcntl(fd, F_SETFL, flags | O_NONBLOCK);
            unsigned int start = timestamp();
// printf("OPEN! %u\n", start);
            // Write an end-of-line
            unsigned char *wc = "\r\n";
            int nw = write(fd, wc, strlen(wc));
// printf("WROTE: %d %s\n", nw, wc);
            // Read a line (with timeout)
            while (timestamp() < (unsigned int)(start + 2000))
            {
                unsigned char c = 0xcc;
                int n = read(fd, &c, 1);
// printf("READ: @%u %d 0x%02x = %c\n", timestamp(), n, c, c);
                if (n < 0) // would block
                {
                    usleep(50 * 1000);
                }
                else if (n == 0) // no more data
                {
                    break;
                }
                else if  (c == '\n') // end-of-line
                {
// printf("LINE: READ!\n");
                    close(fd);
                    return -1;
                }
            }
// printf("LINE: FAIL\n");
            close(fd);
            return 1;
        }
        usleep(250 * 1000);
    }
// printf("FAIL!\n");
    return 0;
}

/** Internal, method for updating a list of seen devices. */
thread_return_t OmDeviceDiscoveryThread(void *arg)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_device *dev;
    struct udev_monitor *mon;

    udev = udev_new();
    if (!udev)
    {
        printf("ERROR: Can't create udev (thread)\n");
        exit(1);
    }

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
    udev_monitor_enable_receiving(mon);

    // Polled device discovery for non-Windows
    while (!om.quitDiscoveryThread)
    {
        dev = udev_monitor_receive_device(mon);
        if (dev == NULL)
        {
            sleep(1);
        }
        else
        {
            // printf("Path: %s\n", udev_device_get_syspath(dev));
            // Check type of action (remove or add)
            char *action = udev_device_get_action(dev);
            // Get block device name.
            char *block_device = udev_device_get_devnode(dev);

printf("DEVICE-ACTION: %s %s\n", action, block_device);

            if (strcmp(action, "remove") == 0)
            {
                RemoveDevice(block_device);
            }
            else if (strcmp(action, "add") == 0)
            {
                // Get block device name.
                struct udev_device *usb_dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

                // Make sure it's one of our devices.
                if (strcmp(udev_device_get_sysattr_value(usb_dev, "idVendor"), "04d8") == 0 && strcmp(udev_device_get_sysattr_value(usb_dev, "idProduct"), "0057") == 0)
                {
                    // Get mount path.
                    char mount_path[256];
                    strcpy(mount_path, "/media/");
                    char *name = udev_device_get_property_value(dev, "ID_FS_LABEL");
                    if (name != NULL)
                    {
                        char serial_device[256] = "";
                        char *serial_id = udev_device_get_sysattr_value(usb_dev, "serial");
                        GetSerialDevice(serial_id, &serial_device[0]);
                        
                        if (strlen(serial_device) > 0)
                        {
                            int id = GetDeviceId(serial_id);
                            strcat(mount_path, name);
                            WaitUntilReadable(serial_device);
                            AddDevice(block_device, mount_path, serial_device, id);
                        }
                    }
                }
            }
            else
            {
                ; // printf("NOTE: Unhandled action: %s %s\n", action, block_device);
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

#endif  // __linux__

