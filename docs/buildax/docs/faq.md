# Frequently Asked Questions

## My sensor won't pair up!

On the sensor, press the button next to the battery compartment. The green LED
on the sensor should light on the front to indicate a pairing packet has been sent. Things 
to check:

 1. Is the LRS in [Pairing Mode](deployment.md#pairing-sensors)?
 2. Ensure the batteries are inserted correctly.
 3. Do other sensors pair correctly? Use the [live data web interface](user-guide.md#sensors) to check.


## Networking issues

Please read the [Network Access](connecting.md#network-access) section. Still
can't connect to the LRS over your network? Try these troubleshooting steps:

#### Using the NETBIOS Address?

It is possible that your network configuration does not support loading MS 
NETBIOS addresses (you are connecting to the device from Mac OSX, for instance). 
The IP address of the device can be used as a fallback in this case, and can
always be obtained using the serial `status` command. 


#### Networking LED

Is the networking LED (LED 3) on the LRS lit? If it is not, this indicates 
that the networking stack is un-initialised. Try re-inserting/re-seating the
ethernet connector to see if the LRS connects. Possible reasons for this 
LED not being lit:

 * Network not assigning IP address
 * Bad Ethernet cable


#### Corporate/Academic Networks

Are you on a corporate/academic network? Your network administrator may 
require that the device MAC address is registered before it can be assigned an 
address. When programming firmware it is possible to reset the MAC address by
holding down the button and clicking "Run Program" in the bootloader tool. If
this is the case you will have to obtain the new MAC via the serial connection
and re-register the device.


## How do I retrieve data from the device?

Please see [Fetching Data](developer-api.md#fetching-data) in the developer
guide for more information on how to fetch data over the network. To directly
connect to the LRS as a Mass Storage device, see [USB Mass Storage](connecting.md#usb-mass-storage).



## How do I read the binary data format?

The `bax2csv` utility can be used to convert the binary data format into a
usable plaintext csv format. Please read the [Data Files](datafiles.md)
documentation for more info.


## Web interface issues

A number of HTTP error codes may be encountered with the web interface. The 
following is not an exhaustive list, but these errors are the most common.

### 404 File Not Found

 ![404](img/bax404.png)

This error indicates that the files for the web interface on the SD card are 
missing or in the wrong place. These should be copied to the SD card manually 
before running, and should be placed under the directory "WWW". 

### 400 Bad Request

This is an error code indicating that the LRS was unable to process a data
fetch request. This will occur when a request is made for data which does not 
exist on the LRS, for example:

 - Data files are missing due to manual deletion
 - Data has been requested from a time range when no data was logged
 - The fetch has been specified for the wrong logging mode (e.g. requesting text when the LRS is logging in binary mode) 

This error may also be seen when automating fetch requests with `wget` or 
`curl`, possible reasons include:

 1. The parameters provided in the URL were not correctly formatted and cannot be recognised
 2. The ampersands (&) in the URL are not correctly escaped (`\&`) and the command has forked
 3. A request has been made which is impossible for the LRS to fulfill (missing data file?)



## I have tried the above and still cannot solve my problem

Another thing to check is if there are any assertions in the ERRORS.txt
file on the SD card with timestamps around the time you saw this problem.
"Reset" lines in this file are usually fine (they are added when the LRS 
starts up to indicate the reset type), unless you have encountered a 
[General Exception](hardware.md#led-flash-codes). The 4-digit number indicates
the type of reset and is taken from the value of the reset register.

If you encounter a line reading "ASSERT" and a file/line number, this might 
indicate a firmware issue. Please send us this file and a description of what 
you were trying to do, and steps to reproduce your problem (if possible).

Still having problems - try turning it off and on!