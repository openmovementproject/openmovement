# Deployment

## Pairing Sensors

To pair sensors to the BuildAX Router, the router must first be put into 
Pairing Mode. To do this, hold the button on the rear of the device until the 
lights flash alternately. The Router will stay in pairing mode for a period of
5 minutes.

### What is "Pairing"?

The BuildAX v2 sensors broadcast data packets containing information about 
their environment over the air. This data could potentially be used by
eavesdroppers to infer whether a building is occupied, something existing 
systems have typically not addressed. As a security measure, all BuildAX v2 
sensors use 128-bit AES encryption to prevent packets being read while in 
transmission.

To decrypt data from the sensors, the router must know the key which was used
to encrypt it. An encryption key is transmitted from the sensor to the router 
when the button on the rear of the sensor (see [#2 on the hardware diagram](hardware.md#rear)) 
is pressed. The radio broadcast power is set to the minimum on the sensor, to
limit the range at which the packet can be received. 

The router then stores the unique key for the sensor in the `BAX_INFO.BIN` 
file on the SD card. To instruct the router to listen for keys, it must be put
into "Pairing Mode", by pressing the button on the rear of the device.

__Note__: The router will log packets from BuildAX v2 sensors regardless of 
whether it has received an encryption packet, as these may be decrypted at a 
later date and would otherwise be lost. 


## Sensor Deployment

### Sensor Range

The achievable range of the sensors will depend on a number of factors,
including the materials used in the construction of the building, the thickness
of the walls, and the "noise-floor" of your deployment environment. 

If there are a lot of devices transmitting on the same 434MHz band as the 
sensors (for example, in a city environment) the radio frequency will be 
noisier and the sensors will not be "heard" by the router at further distances.

Tips on deploying sensors in a building:

 * Position the router at a central location, so as to be in range of all sensors.
 * Air temperature and humidity in a room are stratified (change based on height). 
 * The PIR lens is directional. Mount sensors on the wall **vertically** if this is important.
 * If a sensor at the limit of its range, try upping the transmission interval.
 * Walls appear "thicker" when the radio signal has to travel through them diagonally.
 * Try to avoid direct sunlight when attaching sensors to surfaces. This will skew the data they generate.
 * Also remember the position of the Sun will change throughout the day!

#### RSSI

A sensor's signal strength is indicated by its RSSI value (Received Signal
Strength Indication). This can be thought of as the "quietness" of the radio
signal. In BuildAX 2, the RSSI value is in dBm.

RSSI values can be found in the web interface (see the [Live Data](user-guide.md#sensors)
page), or in the 4th column of the CSV-format data available as a file, or
over the various streaming interfaces (telnet, serial, websocket).

Signal strength will be affected by the battery level of the sensor, the 
distance from the router, and any obstacles which the signal has to travel
through in-between.

#### Noise floor

All deployment locations have a certain amount of radio noise present in the
spectrum used by the BAX sensors. This level of noise is referred to as the
noise "floor", and sensors transmitting must be "louder" than this background
noise level in order to be heard.


