# Frequently Asked Questions

## My sensor won't pair up!

On the sensor, press the button next to the battery compartment. The green LED
should light on the front to indicate a pairing packet has been sent. Things 
to check:

 1. Is the router in [Pairing Mode](deployment.md#pairing-sensors)?
 2. Ensure the batteries are inserted correctly.
 3. Do other sensors pair correctly? Use the [live data web interface](user-guide.md#sensors) to check.


## Networking issues

Please read the [Network Access](connecting.md#network-access) section. Still
can't connect to the router over your network? Try these troubleshooting steps:

#### Using the NETBIOS Address?

It is possible that your network configuration does not support loading MS 
NETBIOS addresses (you are connecting to the device from Mac OSX, for instance). 
The IP address of the device can be used as a fallback in this case, and can
always be obtained using the serial `status` command. 


#### Networking LED

Is the networking LED (LED 3) on the router lit? If it is not, this indicates 
that the networking stack is initialised. Try re-inserting/re-seating the
ethernet connector to see if the router connects. Possible reasons for this 
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
connect to the router as a Mass Storage device, see [USB Mass Storage](connecting.md#usb-mass-storage).



## How do I read the binary data format?

The `bax2csv` utility can be used to convert the binary data format into a
usable plaintext csv format. Please read the [Data Files](datafiles.md)
documentation for more info.



## 400 Bad Request?

This is an error code indicating that the router was unable to process a data
fetch request. Possible reasons for this include:

 1. The parameters provided in the URL were not correctly formatted and cannot be recognised
 2. Have you escaped the ampersands (&) in the URL?
 3. A request has been made which is impossible for the router to fulfill (missing data file?)
 4. Is the SD card properly inserted?



## I have tried the above and still cannot solve my problem

Another thing to check is if there are any assertions in the ERRORS.txt
file on the SD card with timestamps around the time you saw this problem.
"Reset"s in this file (with 0040 or 0083) are usually fine (they are added
when the router starts up to indicate the reset type).

If you see a line reading "ASSERT" and a file/line number, this might 
indicate a firmware issue. Please send us this file and a description of what 
you were trying to do, and steps to reproduce your problem (if possible).
