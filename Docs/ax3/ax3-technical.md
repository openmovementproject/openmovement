# Open Movement AX Device - Technical Documentation


## Introduction

The AX3 is a miniature logging accelerometer, and the AX6 is a miniature logging accelerometer+gyroscope. The devices have on-board memory, a microcontroller, a MEMS sensor and a Real Time Clock (RTC). The AX devices were designed for a range of applications, from clinical and health research to human movement science, and are now globally adopted for these applications. The AX devices also include a temperature and light sensor.  The firmware supports a serial based API (over USB port) and logs its data to an open format file (`.CWA` continuous wave accelerometry). Each file supports the ability to add metadata, record device configurations as well as error detection and correction. 

The sensor is part of the Open Movement project, which is a collection of hardware and software developed under open source terms with a variety of uses in mind, including: health research, digital interaction, instrumentation, gaming and music. With a global community of users including industry, universities and major health research organizations, Open Movement has fast being positioned as the de-facto standard for open source movement science.



## Firmware

The firmware is in the Open Source *Open Movement* project and the compiled result also includes elements from the *Microchip C30 compiler standard library* (standard C routines), *Microchip PIC24F peripheral library* (on-chip functionality) and the *Microchip Application Libraries* (USB stack and filesystem), 

The firmware is written in the *C* programming language and is suitable for compilation with the *Microchip C30 compiler*. Project files are included for *MPLAB 8* and *MPLAB X* development environments. The `.HEX` output files are suitable for *bootloading* on to a *Microchip PIC24F Microcontroller*.

The device offers two main modes of operation: connected (recharging and USB connectivity) and disconnected (sleeping or logging).  


### Device Connected

The connected state allows the device to recharge and communicate over USB.  Recharging takes place when connected to either a PC USB or a stand-alone 5V supply.  On each connection (for a short time before exiting) the device will enumerate over USB as a *bootloader* device.

During normal use, the firmware enumerates as a composite USB device presenting a *Mass Storage Device* (*USB MSD*, similar to a *USB drive*) and a *Communications Device Class* (*USB CDC*, a *virtual serial port*) interface.  

* MSD interface: presents a drive with one or more data files – see below for more details on the stored data. 

* CDC interface: allows the device to be queried and configured (time, battery status, settings, etc.) -- see below for more details on the communication protocol.

The USB serial number will begin with "CWA" for an AX3 device, and "AX6" for an AX6 device.


### Device Disconnected

Depending on the settings, the device can either sleep indefinitely or log measurements (immediately or within a timed interval).  



## Stored Data


### Device Properties

There are device-specific properties, which will be internally maintained in (rewriteable) program memory:

* Device serial number (presented over CDC, at USB enumeration, MSD query, volume label on self-format, written to file header - must be set to match the external numeric identifier).

* Device log (records recording stop and restart reasons, for diagnostic purposes).

The current date and time are maintained in a Real-Time Clock (RTC).

The *firmware version* is a compiled-in constant.


### Recording Settings

There are recording-specific settings stored in the data file `CWA-DATA.CWA` in the root folder:

* Recording session identifier.

* Measurement start date/time (0 = *always started*, -1 = *never started*).

* Measurement stop date/time (0 = *always stopped*, -1 = *never stopped*).

* Sensor accelerometer configuration (sampling frequency and sensitivity).


### Measurement data

*Note:* There are multiple ways to load the raw data into programming languages and/or analysis environments: [AX Research: Raw Data](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-research.md#raw-data).

Measurement data is written to the file `CWA-DATA.CWA` in the root folder, which includes header information and the logged sensor data.  The disk should be formatted at *FAT16* or *FAT32* -- ideally by using the device's own `FORMAT` command which will align the filesystem for maximum efficiency.

The accelerometer sensor's internal sampling frequency is used and the measurements are treated as a stream and recorded in time-stamped blocks so that:

* blocking/aliasing artefacts are avoided (which could occur if sensors are sampled from a synchronous source, which would not by synchronized with the sensor's internal measurement frequency)

* better power consumption is achieved as the accelerometer has a hardware FIFO allowing the main processor to sleep while it collects data.


The stream has any partially-full block flushed before closing the file.

Within the data file, integrity is assured through:

* A sector-sized block-based data structure ensures that, even after significant damage, recovery can always be attempted as the block-boundary will always be known and blocks include a sequence identifier and timestamp.

* The flash-translation layer includes an error-correction code to fix 1-2 bit, and detect 2-4 bit errors that could arise from NAND-level charge loss.

* Data blocks include a 16-bit checksum to provide long-term detection of errors, which will be isolated to a single block (of 80 to 120 samples).


#### CWA File Timestamps

All timestamps are packed into a 32-bit unsigned value, with a year offset of 2000 and months and days beginning at 1: 

```c
// Timestamps are packed into a 32-bit value: (MSB) YYYYYYMM MMDDDDDh hhhhmmmm mmssssss (LSB)
typedef enum uint32_t cwa_timestamp_t;
```

#### Sensor values

In the *un-packed* mode, 3-axis (accelerometer-only) values are simply stored as short signed integers.

```c
typedef struct
{
    int16_t x, y, z;
} accel_t;
```

The unpacked scaling comes from the top three bits of the `lightScale` value, where the scaling is (1/2^(8+n)) in units of *g* -- this is always 1/256 *g* for the AX3.

When the AX6 synchronous gyroscope is enabled, the measurements are 6-axis and stored as follows:

```c
typedef struct
{
    int16_t gx, gy, gz;
    int16_t ax, ay, az;
} imu_t;
```

Bits 10-12 of the lightScale` value determine the gyroscope range of (8000/2^n) degrees per second.

When the *packed* mode is used (AX3 only), the accelerometer values are stored packed into a single 32-bit integer -- each axis value is a signed integer and must be sign-extended and left-shifted by the exponent 'e', resulting units are 1/256 *g*:

```c
//   [byte-3] [byte-2] [byte-1] [byte-0]
//   eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx
//   10987654 32109876 54321098 76543210
```


#### CWA File Header

The CWA binary file header block is always located at offset 0 in the file, and always 1024 bytes in length.  **The structure must be considered tightly packed** (the odd alignment of elements is inherited from the original 8-bit version of the firmware).

```c
typedef struct
{
    uint16_t packetHeader;                      ///< @ 0  +2   ASCII "MD", little-endian (0x444D)
    uint16_t packetLength;                      ///< @ 2  +2   Packet length (1020 bytes, with header (4) = 1024 bytes total)
    uint8_t  hardwareType;                      ///< @ 4  +1   Hardware type (0x00/0xff/0x17 = AX3, 0x64 = AX6)
    uint16_t deviceId;                          ///< @ 5  +2   Device identifier
    uint32_t sessionId;                         ///< @ 7  +4   Unique session identifier
    uint16_t upperDeviceId;                     ///< @11  +2   Upper word of device id (if 0xffff is read, treat as 0x0000)
    cwa_timestamp_t loggingStartTime;           ///< @13  +4   Start time for delayed logging
    cwa_timestamp_t loggingEndTime;             ///< @17  +4   Stop time for delayed logging
    uint32_t loggingCapacity;                   ///< @21  +4   (Deprecated: preset maximum number of samples to collect, should be 0 = unlimited)
    uint8_t  reserved1[1];                      ///< @25  +1   (1 byte reserved)
    uint8_t  flashLed;                          ///< @26  +1   Flash LED during recording
    uint8_t  reserved2[8];                      ///< @27  +8   (8 bytes reserved)
    uint8_t  sensorConfig;                      ///< @35  +1   Fixed rate sensor configuration (AX6 only), 0x00 or 0xff means accel only, otherwise bottom nibble is gyro range (8000/2^n dps): 2=2000, 3=1000, 4=500, 5=250, 6=125, top nibble non-zero is magnetometer enabled.
    uint8_t  samplingRate;                      ///< @36  +1   Sampling rate code, frequency (3200/(1<<(15-(rate & 0x0f)))) Hz, range (+/-g) (16 >> (rate >> 6)).
    cwa_timestamp_t lastChangeTime;             ///< @37  +4   Last change meta-data time
    uint8_t  firmwareRevision;                  ///< @41  +1   Firmware revision number
    int16_t  timeZone;                          ///< @42  +2   (Unused: originally reserved for a "Time Zone offset from UTC in minutes", 0xffff = -1 = unknown)
    uint8_t  reserved3[20];                     ///< @44  +20  (20 bytes reserved)
    uint8_t  annotation[OM_METADATA_SIZE];      ///< @64  +448 Scratch buffer / meta-data (448 characters, ignore trailing 0x20/0x00/0xff bytes, url-encoded UTF-8 name-value pairs)
    uint8_t  reserved[512];                     ///< @512 +512 (Reserved for device-specific meta-data in the same format as the user meta-data) (512 bytes)
} cwa_header_t;
```

**NOTE:** For `.CWA` files produced by an *AX3*, `sensorConfig` can be ignored.


#### CWA File Data Blocks

The CWA binary file data blocks are always 512 bytes in length at a 512-byte-multiple offset, the first immediately after the header (typically at offset 1024).  **The structure must be considered tightly packed.**

```c
typedef struct
{
    uint16_t packetHeader;                      ///< @ 0  +2   ASCII "AX", little-endian (0x5841)	
    uint16_t packetLength;                      ///< @ 2  +2   Packet length (508 bytes, with header (4) = 512 bytes total)
    uint16_t deviceFractional;                  ///< @ 4  +2   Top bit set: 15-bit fraction of a second for the time stamp, the timestampOffset was already adjusted to minimize this assuming ideal sample rate; Top bit clear: 15-bit device identifier, 0 = unknown;
    uint32_t sessionId;                         ///< @ 6  +4   Unique session identifier, 0 = unknown
    uint32_t sequenceId;                        ///< @10  +4   Sequence counter (0-indexed), each packet has a new number (reset if restarted)
    cwa_timestamp_t timestamp;                  ///< @14  +4   Last reported RTC value, 0 = unknown
    uint16_t lightScale;                        ///< @18  +2   AAAGGGLLLLLLLLLL Bottom 10 bits is last recorded light sensor value in raw units, 0 = none; top three bits are unpacked accel scale (1/2^(8+n) g); next three bits are gyro scale (8000/2^n dps)
    uint16_t temperature;                       ///< @20  +2   Last recorded temperature sensor value in raw units (bottom 10-bits), 0 = none; (top 6-bits reserved)
    uint8_t  events;                            ///< @22  +1   Event flags since last packet, b0 = resume logging, b1 = reserved for single-tap event, b2 = reserved for double-tap event, b3 = reserved, b4 = reserved for diagnostic hardware buffer, b5 = reserved for diagnostic software buffer, b6 = reserved for diagnostic internal flag, b7 = reserved)
    uint8_t  battery;                           ///< @23  +1   Last recorded battery level in scaled/cropped raw units (double and add 512 for 10-bit ADC value), 0 = unknown
    uint8_t  sampleRate;                        ///< @24  +1   Sample rate code, frequency (3200/(1<<(15-(rate & 0x0f)))) Hz, range (+/-g) (16 >> (rate >> 6)).
    uint8_t  numAxesBPS;                        ///< @25  +1   0x32 (top nibble: number of axes, 3=Axyz, 6=Gxyz/Axyz, 9=Gxyz/Axyz/Mxyz; bottom nibble: packing format - 2 = 3x 16-bit signed, 0 = 3x 10-bit signed + 2-bit exponent)
    int16_t  timestampOffset;                   ///< @26  +2   Relative sample index from the start of the buffer where the whole-second timestamp is valid
    uint16_t sampleCount;                       ///< @28  +2   Number of sensor samples (if this sector is full -- Axyz: 80 or 120 samples, Gxyz/Axyz: 40 samples)
    uint8_t  rawSampleData[480];                ///< @30  +480 Raw sample data.  Each sample is either 3x/6x/9x 16-bit signed values (x, y, z) or one 32-bit packed value (The bits in bytes [3][2][1][0]: eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx, e = binary exponent, lsb on right)
    uint16_t checksum;                          ///< @510 +2   Checksum of packet (16-bit word-wise sum of the whole packet should be zero)
} OM_READER_DATA_PACKET;
```

**NOTE:** For `.CWA` files produced by an *AX3*: `lightScale` top 6 bits will be zero and can be ignored and `numAxesBPS` top nibble will always be `3`.

The complexity of the way the timestamp is stored is for historical/backwards-compatibility reasons. Early 8-bit prototype devices didn't have the ability to track the real-time clock to a resolution less than one second, yet did know when the second changed.  To maximize the effective timer resolution (to be the same order as the sampling rate), the relative index of the sample just before the roll-over was recorded (`timestampOffset`). Before any AX3 devices were generally available, back in December 2011 (Firmware V23) one of the device's timers to the RTC's clock crystal to take a precise (sub-second) measurement whenever the underlying accelerometer's buffer fills up (for standard rates: every 25 samples, when it wakes the main processor): recording the index of the sample, and the full timestamp (with fractional part).  This most recent measurement is only actually stored when a whole sector is written. To allow existing file readers to maintain a high timer resolution, the best way to record this additional precision was to make new use of an existing field (`deviceFractional`, indicated by the top bit set) and adjust the `timestampOffset` to be the nearest sample to the whole-second roll-over (assuming the sample rate is exactly as configured).  If the high-precision timer is detected, this adjustment can be undone by newer readers, and the fractional time used instead.  In summary: the original format timestamp has no fractional part and the timestamp-offset is the relative sample index where that time is correct; newer readers can spot that the timestamp now includes the fractional part and the timestamp-offset can be adjusted to point to the sample the fractional timestamp was actually for (the backwards-compatible adjustment is undone).


## Communication Protocol

*Note:* A software API exists that can be used to create your own software to communicate with the AX devices: [AX libOMAPI](https://github.com/digitalinteraction/libomapi/)

Commands and responses are all in plain, 7-bit ASCII text and delimited with *CR*/*LF* (`\r\n`) line endings.  The sections below use the following conventions:

```
TEXT
<value-placeholder>
[optional-text]
{alternative-text-1 | alternative-text-2 | ...}
```

These commands can also be placed in to a file `SETTINGS.INI` in the root directory.  This is primarily used for adjusting the `.CWA` file format directly to use the *unpacked mode* by using the command `DATAMODE 20` (`18` is the default *packed mode*).

There are additional commands to those listed here, for more technical/diagnostic use.


### Settings


#### Session identifier

The session id (0 to 2^31, giving nine numeric digits) is set using:

	SESSION <numeric-session-id>

Query only:

	SESSION

Both of the above return the value:

	SESSION=<numeric-session-id>


#### Measurement start time

Set the date and time to "hibernate" before starting the recording:

	HIBERNATE <YYYY-MM-DD,hh:mm:ss>

Set "always begin measuring" (`0`=infinitely in the past):

	HIBERNATE 0

Set "never begin measuring" (`-1`=infinitely in the future):

	HIBERNATE -1

Query only:

	HIBERNATE

All of the above return the current status:

	HIBERNATE={-1|0|<YYYY-MM-DD,hh:mm:ss>}


#### Measurement stop time

Set the date and time to stop measuring after:

	STOP <YYYY-MM-DD,hh:mm:ss>

Set "always stop measuring" (`0`=infinitely in the past):

	STOP 0
	
Set "never stop measuring" (`-1`=infinitely in the future):

	STOP -1

Query only:

	STOP

All of the above return the current status:

	STOP={-1|0|<YYYY-MM-DD,hh:mm:ss>}


#### Sensor configuration

To set the accelerometer sensor's `rate-code`:

	RATE <rate-code>

On an AX6, the above command will disable the gyroscope (it will run in accelerometer-only mode).  The synchronous gyroscope can be enabled with a valid `gyro-range` (250/500/1000/2000 dps) using a variation of the command:

	RATE <rate-code>,<gyro-range>

Query only:

	RATE

The above return the current status -- on an AX3:

	RATE=<rate-code>,<frequency-hz>

...on an AX6:

	RATE=<rate-code>,<frequency-hz>,<gyro-range>

The `rate-code` is split into separate parts: the sensitivity/range and sample frequency.  The sample frequency is `(3200/(1<<(15-(rate & 0x0f))))` Hz, and the range is (+/-g) `(16 >> (rate >> 6))`.

A code can be created by the sum of the parts:

Sensitivity range:

* 0 = +/-16g
* 64 = +/-8g
* 128 = +/-4g
* 192 = +/-2g

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

Where `type` will be `CWA` for an *AX3*, and `AX6` for an *AX6*.


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

	$BATT=<raw-ADC>,<millivolts>,mV,<percentage>,<charge-termination-flag>

	
#### Storing settings

The current RAM-based settings can be written to the data file header by the device for by using the command:

	COMMIT

The whole drive can be formatted with:

	FORMAT {Q|W}[C]

Where `Q` performs a *quick format* (the filesystem is recreated), and `W` thoroughly *wipes* by clearing all of the NAND memory.  The optional `C` performs the 'COMMIT' command afterwards and rewrites the header of the data file.  

During a `FORMAT`/`COMMIT` the USB mass storage drive is seen as "ejected" by the operating system while the device writes to the memeory, and then seen as re-inserted afterwards.  


#### Additional configuration file

The standard configuration comes from the header of the data file (`CWA-DATA.CWA`), but settings controlling the data format of the file can be written to an external settings file, `SETTINGS.INI` (containing plain ASCII commands, CR/LF `\r\n` terminated, and stored in the root folder of the device).  The typical use of this would be to signify an *un-packed* setting for data storage, which is set when the configuration file contains the following line (`18` is the default *packed* mode):

	DATAMODE=20


### Data Preview Stream

There is a command to enable a stream of preview data from the underlying sensor:
 
      STREAM
 
...the stream will end when another line is sent to the device (including an empty line, terminated with CR/LF).

If the device is an AX3, the comma-separated values will be:
 
      Ax,Ay,Az
 
...where the accelerometer axis values (A) should be divided by 256 to be in units of g (9.81 m/s/s).

If the device is an AX6, the comma-separated values will be:
 
      Ax,Ay,Az,Gx,Gy,Gz
 
...where the accelerometer axis values (A) should be divided by 4096 to be in units of g (9.81 m/s/s), and the gyroscope axis values (G) should be multiplied by 32768 and divided by 1000 for degrees per second.

Note that this streaming mode is only intended to preview the sampled data.  Importantly, the data is *single sampled*, not at a fixed regular rate, and may be a relatively low sample rate that is slightly variable (depending on the line length and other communication overheads).  These factors make it unsuitable for most uses, other than the intended use of previewing the data.


## Bootloader

Standard use:

1. The device connects and enumerates as a *bootloader* device (only for a few seconds).

2. The desktop client could ignore this (and wait for the timeout); or detect this device and send it a *run* command to exit the bootloader immediately.

3. (normal device operations as described above)


Update the firmware:

1. The desktop client can query the device's firmware from the normal run mode then, if it has a newer version available, offer to update.

2. The device can be sent a `RESET` command (to enter bootloader mode).

3. The desktop client detects the bootloader device and sends it a *query* command (this is enough to prevent it from timing-out of the bootloader mode), followed by program, verify, and then run.

4. (normal device operations as described above)

