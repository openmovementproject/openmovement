# Open Movement AX3 Sensor - Technical Documentation


## Introduction

The AX3 is a miniature logging accelerometer. It has on-board memory, a microcontroller, a MEMS sensor ([ADXL345](http://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf)) and a Real Time Clock (RTC). The AX3 was designed for a range of applications ranging from clinical and health research to human movement science and is now globally adopted for these applications. The AX3 also includes a temperature and light sensor.  The AX3 sensor is based on a 16-bit architecture using a PIC microcontroller. The firmware supports a serial based API (over USB port) and logs its data to an open format file (.CWA continuous wave accelerometry). Each file supports the ability to add metadata, record device configurations as well as error detection and correction. 

The sensor is part of the Open Movement project, which is a collection of hardware and software developed under open source terms with a variety of uses in mind, including: health research, digital interaction, instrumentation, gaming and music. With a global community of users including industry, universities and major health research organizations, Open Movement has fast being positioned as the de-facto standard for open source movement science.



## Firmware

The firmware is in the Open Source *Open Movement* project and the compiled result also includes elements from the Microchip C30 compiler standard library (standard C routines), Microchip PIC24F peripheral library (on-chip functionality) and the Microchip 'Application Libraries' (USB stack and filesystem), 

The firmware is written in the C programming language and is suitable for compilation with the Microchip C30 compiler. Project files are included for MPLAB 8 and MPLAB X development environments. The .HEX output files are suitable for 'bootloading' on to a Microchip PIC24F Microcontroller.

The device offers two main modes of operation: connected (recharging and USB connectivity) and disconnected (sleeping or logging).  


### Device Connected

The connected state allows the device to recharge and communicate over USB.  Recharging takes place when connected to either a PC USB or a stand-alone 5V supply.  On every connection for a short time before exiting the device will enumerate over USB as a 'bootloader' device.

During normal use, the firmware enumerates as a composite USB device presenting a Mass Storage Device (USB MSD, similar to a 'USB drive') and a Communications Device Class (USB CDC, a 'virtual serial port') interface.  

* MSD interface: presents a drive with one or more data files – see below for more details on the stored data. 

* CDC interface: allows the device to be queried and configured (time, battery status, settings, etc.) -- see below for more details on the communication protocol.


### Device Disconnected

Depending on the settings, the device can either sleep indefinitely or log measurements (immediately or within a timed interval).  



## Stored Data


### Device Properties

There are device-specific properties, which will be internally maintained in (rewriteable) program memory:

* Device serial number (presented over CDC, at USB enumeration, MSD query, volume label on self-format, written to file header - must be set to match the external numeric identifier).

* Device log (records recording stop and restart reasons, for diagnostic purposes).

The current date and time are maintained in a Real-Time Clock (RTC).

The 'firmware version' and 'hardware version' are compiled-in constants.


### Recording Settings

There are recording-specific settings.  A text configuration file (using the communication protocol) is used to select the data file name, together with settings that will be written to the data file if it does not already exist. 

* File name (an 8.3 non-"long file name")

* Recording session identifier

* Measurement start date/time (0 = 'always started', -1 = 'never started').

* Measurement stop date/time (0 = 'always stopped', -1 = 'never stopped').

* Sensor accelerometer configuration (sampling frequency and sensitivity).


### Measurement data

Measurement data is written to a file ('CWA-DATA.CWA') in the root folder, which includes header information and the logged sensor data.  The disk should be formatted at FAT16 or FAT32.

The accelerometer sensor's internal sampling frequency is used and the measurements are treated as a stream and recorded in time-stamped blocks so that:

* blocking/aliasing artefacts are avoided (which could occur if sensors are sampled from a synchronous source, which would not by synchronized with the sensor's internal measurement frequency)

* better power consumption is achieved as the accelerometer has a hardware FIFO allowing the main processor to sleep while it collects data.


The stream has any partially-full block flushed before closing the file.

Within the data file, integrity is assured through:

* A sector-sized block-based data structure ensures that, even after significant damage, recovery can always be attempted as the block-boundary will always be known and blocks include a sequence identifier and timestamp.

* The flash-translation layer includes an error-correction code to fix 1-2 bit, and detect 2-4 bit errors that could arise from NAND-level charge loss.

* Data blocks include a 16-bit checksum to provide long-term detection of errors, which will be isolated to a single block (of 80 to 120 samples).


#### CWA File Timestamps

All timestamps are packed into a 32-bit unsigned value: 

```c
// Timestamps are packed into a 32-bit value: (MSB) YYYYYYMM MMDDDDDh hhhhmmmm mmssssss (LSB)
typedef enum uint32_t cwa_timestamp_t;
```

#### Accelerometer values

In the 'un-packed' mode, the accelerometer values are simply stored as short signed integers:

```c
typedef struct
{
	int16_t x, y, z;
} accel_t;
```

When the 'packing' mode is used, the accelerometer values are stored packed into a single 32-bit integer:

```c
// Packed accelerometer value - must sign-extend each component value and left-shift by exponent 'e', units are 1/256 g.
//        [byte-3] [byte-2] [byte-1] [byte-0]
//        eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx
//        10987654 32109876 54321098 76543210
```


#### CWA File Header

The CWA binary file header block is always located at offset 0 in the file, and always 1024 bytes in length.  The odd alignment of elements is inherited from the original 8-bit version of the firmware, the structure must be considered tightly packed.

```c
typedef struct
{
    uint16_t packetHeader;                      ///< @ 0  +2   ASCII "MD", little-endian (0x444D)
    uint16_t packetLength;                      ///< @ 2  +2   Packet length (1020 bytes, with header (4) = 1024 bytes total)
    uint8_t  reserved1;                         ///< @ 4  +1   (1 byte reserved)
    uint16_t deviceId;                          ///< @ 5  +2   Device identifier
    uint32_t sessionId;                         ///< @ 7  +4   Unique session identifier
    uint16_t reserved2;                         ///< @11  +2   (2 bytes reserved)
    cwa_timestamp_t loggingStartTime;           ///< @13  +4   Start time for delayed logging
    cwa_timestamp_t loggingEndTime;             ///< @17  +4   Stop time for delayed logging
    uint32_t loggingCapacity;                   ///< @21  +4   (Deprecated: preset maximum number of samples to collect, 0 = unlimited)
    uint8_t  reserved3[11];                     ///< @25  +11  (11 bytes reserved)
    uint8_t  samplingRate;                      ///< @36  +1   Sampling rate code, frequency (3200/(1<<(15-(rate & 0x0f)))) Hz, range (+/-g) (16 >> (rate >> 6)).
    cwa_timestamp_t lastChangeTime;             ///< @37  +4   Last change metadata time
    uint8_t  firmwareRevision;                  ///< @41  +1   Firmware revision number
    int16_t  timeZone;                          ///< @42  +2   (Unused: originally reserved for a "Time Zone offset from UTC in minutes", 0xffff = -1 = unknown)
    uint8_t  reserved4[20];                     ///< @44  +20  (20 bytes reserved)
    uint8_t  annotation[OM_METADATA_SIZE];      ///< @64  +448 Scratch buffer / meta-data (448 characters, ignore trailing 0x20/0x00/0xff bytes, url-encoded UTF-8 name-value pairs)
    uint8_t  reserved[512];                     ///< @512 +512 (Unused: originally reserved for post-collection scratch buffer / meta-data) (512 bytes)
} cwa_header_t;
```


#### CWA File Data Blocks

The CWA binary file data blocks are always 512 bytes in length at a 512-byte-multiple offset, the first at offset 1024 (immediately after the header).  The structure must be considered tightly packed.

```c
typedef struct
{
    uint16_t packetHeader;                      ///< @ 0  +2   ASCII "AX", little-endian (0x5841)
    uint16_t packetLength;                      ///< @ 2  +2   Packet length (508 bytes, with header (4) = 512 bytes total)
    uint16_t deviceFractional;                  ///< @ 4  +2   Top bit set: 15-bit fraction of a second for the time stamp, the timestampOffset was already adjusted to minimize this assuming ideal sample rate; Top bit clear: 15-bit device identifier, 0 = unknown;
    uint32_t sessionId;                         ///< @ 6  +4   Unique session identifier, 0 = unknown
    uint32_t sequenceId;                        ///< @10  +4   Sequence counter (0-indexed), each packet has a new number (reset if restarted)
    cwa_timestamp_t timestamp;                  ///< @14  +4   Last reported RTC value, 0 = unknown
    uint16_t light;                             ///< @18  +2   Last recorded light sensor value in raw units, 0 = none
    uint16_t temperature;                       ///< @20  +2   Last recorded temperature sensor value in raw units, 0 = none
    uint8_t  battery;                           ///< @23  +1   Last recorded battery level in raw units, 0 = unknown
    uint8_t  events;                            ///< @22  +1   Event flags since last packet, b0 = resume logging, b1 = reserved for single-tap event, b2 = reserved for double-tap event, b3 = reserved, b4 = reserved for diagnostic hardware buffer, b5 = reserved for diagnostic software buffer, b6 = reserved for diagnostic internal flag, b7 = reserved)
    uint8_t  sampleRate;                        ///< @24  +1   Sample rate code, frequency (3200/(1<<(15-(rate & 0x0f)))) Hz, range (+/-g) (16 >> (rate >> 6)).
    uint8_t  numAxesBPS;                        ///< @25  +1   0x32 (top nibble: number of axes = 3; bottom nibble: packing format - 2 = 3x 16-bit signed, 0 = 3x 10-bit signed + 2-bit exponent)
    int16_t  timestampOffset;                   ///< @26  +2   Relative sample index from the start of the buffer where the whole-second timestamp is valid
    uint16_t sampleCount;                       ///< @28  +2   Number of accelerometer samples (80 or 120 if this sector is full)
    uint8_t  rawSampleData[480];                ///< @30  +480 Raw sample data.  Each sample is either 3x 16-bit signed values (x, y, z) or one 32-bit packed value (The bits in bytes [3][2][1][0]: eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx, e = binary exponent, lsb on right)
    uint16_t checksum;                          ///< @510 +2   Checksum of packet (16-bit word-wise sum of the whole packet should be zero)
} OM_READER_DATA_PACKET;
```


## Communication Protocol

Commands and responses are all in plain (7-bit ASCII) text and delimited with <CR>/<LF> ("\r\n") line endings.  The sections below use the following conventions:

```
TEXT
<value-placeholder>
[optional]
{alternative-1 | alternative-2 | ...}
```

These commands can also be placed in to a file 'SETTINGS.INI' in the root directory.  This is primarily used for adjusting the .CWA file format directly to use the 'unpacked mode' by using the command 'DATAMODE 4' (2 is the default 'packed mode').

There are additional commands for more technical/diagnostic use.


### Settings


#### Session identifier

The session id (0 to ~2000000000) is set using:

	SESSION <numeric-id>

Query only:

	SESSION

Both of the above return the value:

	SESSION=<numeric-id>


#### Measurement start time

Set the date and time to sleep until before starting measuring:

	HIBERNATE <YYYY-MM-DD,hh:mm:ss>
	
Set "always begin measuring":

	HIBERNATE 0

Set "never begin measuring":

	HIBERNATE -1

Query only:

	HIBERNATE

All of the above return the current status:

	HIBERNATE={-1|0|<YYYY-MM-DD,hh:mm:ss>}


#### Measurement stop time

Set the date and time to stop measuring after:

	STOP <YYYY-MM-DD,hh:mm:ss>

Set "always stop measuring":

	STOP 0
	
Set "never stop measuring":

	STOP -1

Query only:

	STOP

All of the above return the current status:

	STOP={-1|0|<YYYY-MM-DD,hh:mm:ss>}


#### Accelerometer configuration

To set the accelerometer sensors 'rate code':

	RATE <rate-code>

Query only:

	RATE

The above return the current status:

	RATE=<rate-code>,<frequency-hz>


The rate code is split into separate parts: the sensitivity/range and sample frequency.  The frequency rate is (3200/(1<<(15-(rate & 0x0f)))) Hz, and the range is (+/-g) (16 >> (rate >> 6)).
		
A code can be created by the sum of the parts:

Sensitivity range:

* 0 = +/-16g
* 64 = +/-8g
* 128 = +/-4g
* 256 = +/-2g

Rate:

* 6 = 6.25 Hz
* 7 = 12.5 Hz
* 8 = 25 Hz
* 9 = 50 Hz
* 10 = 100 Hz
* 11 = 200 Hz
* 12 = 400 Hz
* 13 = 800 Hz
* 14 = 1600 Hz
* 15 = 3200 Hz

Power mode:

* 16 = low power mode


### Commands


#### Device identification

Query the device identification:

	ID

Responds:
	
	ID=<type>,<hardwareVer>,<firmwareVer>,<deviceId>


#### Real-time clock

Set the time:

	TIME <YYYY-MM-DD,hh:mm:ss>

Query only:

	TIME

Both of the above return the RTC value:

	TIME=<YYYY-MM-DD,hh:mm:ss>

	
#### Battery

Query current battery level:

	SAMPLE 1

Returns:

	$BATT=<raw-ADC>,<millivolts>,mV,<percentage>,<fully-charged>

	
#### Storing settings

This settings file can be written to the data file header by the device for the current RAM-based settings by using the command:

	COMMIT

The whole drive can be quick formatted with:

	FORMAT {Q|W}[C]

Where 'Q' performs a "quick format" (the filesystem is recreated), and 'W' thoroughly clears all of the NAND memory.  The optional 'C' performs a 'COMMIT' command afterwards and rewrites the header of the configuration file.  



## Bootloader

Standard use:

1. The device connects and enumerates as a 'bootloader' device (only for a few seconds).

2. The desktop client could ignore this (and wait for the timeout); or detect this device and send it a 'run' command to exit the bootloader immediately.

3. (normal device operations as described above)


Update the firmware:

1. The desktop client can query the device's firmware, and offer to update if it has a newer version available.

2. The device can be sent a 'RESET' command (to enter bootloader mode).

3. The desktop client detects the bootloader device and sends it a 'query' command (this is enough to prevent it from timing-out of the bootloader mode), followed by program, verify, and then run.

4. (normal device operations as described above)

