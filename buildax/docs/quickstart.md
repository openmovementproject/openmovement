
# Quick Start Guide


Depending on the number of sensors needing to be paired, and the size of the
building a full BuildAX deployment may be done from scratch in as little as 
20 minutes. There are three main parts in this guide:

 1. Set up the BuildAX LRS
 2. Pair sensors to LRS
 3. Deploy sensors in building 

## Part List

The following components are required for a basic deployment:
 
 * BuildAX LRS
 * SD Card (1GB+ recommended)
 * USB A-B cable
 * USB Mains power adapter
 * Ethernet cable

If you do not require network connectivity for your deployment, feel free to 
disregard steps related to this and use the alternate [serial commands](commands-lrs.md).


## LRS Setup

<span class="alert alert-info"> 
**First-time Setup**
For first-time setup, you may prefer to connect the LRS to a laptop (rather
than the USB mains power adapter) in order to troubleshoot network connectivity
using the serial interface.
</span>


### Pre-deployment steps:

 * Download the [Web Interface zip file](https://code.google.com/p/openmovement/wiki/Downloads) found in the Downloads section of the OpenMovement repository to your computer. Note that this only needs to be done once, when you first set up the LRS.
 * Extract the files (keeping them inside the "WWW" directory) to the root of your SD card, using a card reader


### Setting up:

 1. With the LRS powered OFF, eject card from computer & insert into LRS
 2. Connect the ethernet cable between the BuildAX LRS and a network access 
 point (or home Broadband router)
 3. Connect USB power cable to the BuildAX LRS and wall adapter
 4. Connect to the LRS by typing its address into your browser (click [here] for tips on finding address).
 ([Additional steps](connecting.md#finding-the-ip-address) are required if your 
 network doesn't support NETBIOS or your LRS is missing the sticker with 
 this information)
 5. Log into the LRS with the default credentials (`admin`/`password`)
 6. Click "Settings" and turn on logging (and set other options if required)
 7. Change default credentials

Logging options can be set via the web interface (see [here](user-guide.md#settings))
or via the console (see [here](commands-lrs.md#settings)).

### Video

<iframe width="560" height="315" src="//www.youtube.com/embed/8GK42VeqjOI" frameborder="0" allowfullscreen></iframe>



## Pair/Deploy ENV Sensors

For a more in-depth walk through pairing, please see the the [deployment](deployment.md#sensor-deployment) section.

Quick pairing steps:

 1. Press the button on the back of the LRS until the LEDs flash. You have 5 minutes to pair your sensors
 2. Open the BuildAX sensor. Press the button next to the battery compartment, holding the sensor close to the LRS.
 3. The LED on the front of the sensor should illuminate green to indicate that a pairing packet has been sent.
 4. Determine if the sensor is paired using the [live data](user-guide.md#sensors) interface (or [console](connecting.md#serial-usb))

Quick deployment steps:

 1. Choose a location out of direct sunlight, with good airflow.
 2. Using a sticky pad or hook, attach the back plate of the sensor to the wall
 3. Clip the sensor into the backplate
 4. Verify that packets are still being received from the device

### Video

<iframe width="560" height="315" src="//www.youtube.com/embed/eoTs1xR43Pc" frameborder="0" allowfullscreen></iframe>

You're done!
