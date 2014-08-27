# Frequently Asked Questions

### My sensor won't pair up!

 1. Is the router in [Pairing Mode](deployment.md#pairing-sensors)?
 2. Ensure the batteries are inserted correctly.
 3. Press the button next to the battery compartment. The green LED should light on the front to indicate a pairing packet has been sent.
 4. Do other sensors pair correctly? Use the live data web interface to check.

### Networking issues

Please read the [Network Access](connecting.md#network-access) section. Still
can't connect to the router over your network? Try these troubleshooting steps:

 1. Is the networking LED (LED 3) on the router lit? If it is not, the device has not been assigned an address by the network and is not connectable.
 2. Try re-inserting/re-seating the ethernet connector.
 2. Are you on a corporate/academic network? Your network administrator may require that the device MAC address is registered before it can be assigned an address. 

It is possible that your network configuration does not support loading MS 
NETBIOS addresses (you are connecting to the device from Mac OSX, for instance). 
The IP address of the device can be used as a fallback in this case, and can
always be obtained using the serial `status` command. 

### How do I retrieve data from the device?

Please see [Fetching Data](developer-api.md#fetching-data) in the developer
guide for more information on how to fetch data over the network.

### How do I read the binary data format?

The `bax2csv` utility can be used to convert the binary data format into a
usable plaintext csv format. 

### 400 Bad Request?

This is an error code indicating that the router was unable to process a data
fetch request. Possible reasons for this include:

 1. The parameters provided in the URL were not correctly formatted and cannot be recognised
 2. Have you escaped the ampersands (&) in the URL?
 3. A request has been made which is impossible for the router to fulfill (missing data file?)
 4. Is the SD card properly inserted?

