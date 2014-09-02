
# Quick Start Guide


Depending on the number of sensors needing to be paired, and the size of the
building a full BuildAX deployment may be done from scratch in as little as 
20 minutes. There are three main parts in this guide:

 1. Set up the BuildAX Router
 2. Pair sensors to Router
 3. Deploy sensors in building 

## Part List

The following components are required for a basic deployment:
 
 * BAX Router
 * SD Card (1GB+ recommended)
 * USB A-B cable
 * USB Mains power adapter
 * Ethernet cable

If you do not require network connectivity for your deployment, feel free to 
disregard steps related to this and use the alternate [serial commands](commands.md).

## Router Setup

For first-time setup, you may prefer to connect the router to a laptop (rather
than the USB mains power adapter) in order to troubleshoot network connectivity
using the serial interface.

Pre-deployment steps:

 * Copy WWW files onto your SD card (to enable the web interface)

Setting up:

 1. Insert SD card into the BAX router
 2. Connect the ethernet cable between the BuildAX Router and a network access 
 point (or home Broadband router)
 3. Connect USB power cable to the BAX router and wall adapter
 4. Connect to the router by typing its address into your browser.
 ([Additional steps](connecting.md#finding-the-ip-address) are required if your 
 network doesn't support NETBIOS or your router is missing the sticker with 
 this information)
 5. Log into the router with the default credentials (`admin`/`password`)
 6. Click "Settings" and turn on logging (and set other options if required)
 7. Change default credentials

Logging options can be set via the web interface (see [here](user-guide.md#settings))
or via the console (see [here](commands.md#settings)).

### Video

**TODO**
[//]: # (E.G. YouTube embed- uncomment this: <iframe width="560" height="315" src="//www.youtube.com/embed/dZBaF6EE0Cc" frameborder="0" allowfullscreen></iframe>)

## Pair

For a more in-depth walk through pairing, please see the [deployment](deployment.md#pairing-sensors)
section.

Quick pairing steps:

 1. Press the button on the back of the router until the lights flash. You have 5 minutes to pair your sensors
 2. Holding the sensor close to the router. Encryption packets are sent at low radio power
 3. Open each BuildAX sensor and press the button next to the battery compartment
 4. The LED on the front of the sensor should light to indicate that a pairing packet has been sent.
 5. Determine if the sensor is paired using the [live data](user-guide.md#sensors) interface or [console](connecting.md#serial-usb)

### Video

**TODO**


## Deploy

This is covered in more depth in the [deployment](deployment.md#sensor-deployment) 
section.

Quick deployment steps:

 1. Choose a location out of direct sunlight, with good airflow.
 2. Using a sticky pad or hook, attach the back plate of the sensor to the wall
 3. Clip the sensor into the backplate
 4. Verify that packets are still being received from the device

### Video

**TODO**

You're done!
