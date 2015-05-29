# Frequently Asked Questions

## My sensor won't pair up!

On the sensor, press the button next to the battery compartment. The green LED
on the sensor should light on the front to indicate a pairing packet has been sent. Things 
to check:

 1. Is the LRS in [Pairing Mode](deployment.md#pairing-sensors)?
 2. Ensure the batteries are inserted correctly.
 3. Do other sensors pair correctly? Use the [live data web interface](user-guide.md#sensors) to check.



---
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

Some gigabit unmanaged switches can also cause this behaviour (observed with a 
Netgear GS108). The ethernet chip (enc28j60) used by the BuildAX LRS runs at
10 Megabits/second, and some unmanaged switches do not negotiate the speed and 
wire protocol of a 10M connection properly. Trying a different cable (for 
example, CAT5 instead of 5e or 6) may mitigate this problem (untested).


#### Corporate/Academic Networks

Are you on a corporate/academic network? Your network administrator may 
require that the device MAC address is registered before it can be assigned an 
address. When programming firmware it is possible to reset the MAC address by
holding down the button and clicking "Run Program" in the bootloader tool. If
this is the case you will have to obtain the new MAC via the serial connection
and re-register the device.


#### Have you just flashed new firmware to the device?

It is possible that when the firmware is written to the device and the device
reset, the MAC address which the LRS uses to negotiate with the network will
be re-generated. To do this intentionally, hold down the button on the LRS when 
clicking "Run Application" from the bootloader app.

To change the MAC address, use the [`mac!` command](commands-lrs.md#mac)
followed by `setting.save` and `reset`.

---
## How do I retrieve data from the device?

Please see [Fetching Data](developer-api.md#fetching-data) in the developer
guide for more information on how to fetch data over the network. To directly
connect to the LRS as a Mass Storage device, see [USB Mass Storage](connecting.md#usb-mass-storage).



---
## How do I read the binary data format?

The `bax2csv` utility can be used to convert the binary data format into a
usable plaintext csv format. Please read the [Data Files](datafiles.md)
documentation for more info.



---
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



---
## I cannot connect to a device using Serial!

There are a number of things you can check here:

 - Am I connecting to the correct serial device (COM port in Windows)?
 - Is the USB cable a known good (working) cable? 

LRS Specific checks:

 - Have I installed a USB Serial driver?
 - Is LED 2 (the USB Serial status LED) lit?

ENV / other BAX sensors specific checks:

 - Have I installed the [driver](http://www.ftdichip.com/FTDrivers.htm) for my FTDI cable?
 - Are the 3 pins of my FTDI connector connected in the [correct order](connecting.md#connecting-to-env-sensors)?


---
## I have tried the above and still cannot solve my problem

The first thing you should do is reset the router, and check if this solves
the problem. Locally, simply unplug the device and plug it in again. 
This can also be done remotely by [issuing the `reset` command](commands-lrs.md#reset)
over telnet or serial.

If this does not solve your problem, you should check if there are any 
assertions in the ERRORS.txt file on the SD card with timestamps around the 
time you saw this problem. "Reset" lines in this file are usually fine (they 
are added when the LRS starts up to indicate the reset type), unless you have 
encountered a [General Exception](hardware.md#led-flash-codes). The 4-digit 
number indicates the type of reset and is taken from the value of the reset 
register.

If you encounter a line reading "ASSERT" and a file/line number, this might 
indicate a firmware issue. Please send us this file and a description of what 
you were trying to do, and steps to reproduce your problem (if possible).

---
## Will BuildAX interfere with my other equipment/BMS/WiFi/etc

As the BuildAX v2 sensors documented here run on the 433MHz ISM radio band, they will not interfere with (or be disrupted by) networks like the 802.11 WiFi protocol family which run on the 2.4GHz band.

Additionally, the design of the radio protocol means that our packets are very short- this further reduces the probability that they will interfere with other 433MHz protocols (or each other).

Finally, the radio chipsets used have been EMC-tested (and type-approved) in order for the hardware to receive the CE marking. This means that BuildAX conforms to the EEA directives for electromagnetic noise emission.


