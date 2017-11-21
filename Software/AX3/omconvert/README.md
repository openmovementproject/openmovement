# Open Movement Conversion Program

## Overview

The conversion program resamples the input raw `.CWA` file (which records data at the naturally slightly-variable rate of the underlying sensor) based on the more precise timestamps that up to one sample per sector are tagged with.

The conversion process can be run as follows (there is a default `-calibrate 1` for auto-calibration):

```bash
omconvert input.cwa -out output.wav -info information.txt
```


## WAV file

The output `.wav` file uses the standard [.WAV file format](https://en.wikipedia.org/wiki/WAV) is pretty standard for audio files -- and there is plenty of documentation available for that format: [Audio File Format Specifications](http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html), [WAVE PCM soundfile format](http://soundfile.sapp.org/doc/WaveFormat/).

The output file contains four channels: the first three are for the X-/Y-/Z-axes respectively, and the fourth contains auxiliary data which can typically be ignored.  The format uses signed 16-bit values (values from -32768 to +32767) -- this is typically normalized to the -1 to +1 range, and then scaled by the channel scaling (which will be 8 for the +/-2, 4 or 8*g* range; and 16 for the +/-16*g* range -- where 1*g* is 9.81m/s/s).

The output file headers are padded so that the underlying data always starts at a fixed 1kB (1024 byte) offset.


### Metadata

#### Timestamps

The timestamp of the initial sample is given in the *Creation Date* (`ICRD`) WAV chunk, in the format `YYYY-MM-DD hh:mm:ss.000`, and subsequent samples are relative to that time at the file's sample rate. This timestamp is also stored in the *Comment* chunk as the `Time` value.

Other metadata is stored in key-value pairs in other WAV file headers: each pair separated by a new line, and each key-and value separated by a colon (`:`) character (leading and trailing whitespace should be ignored).


#### Device metadata

Metadata about the device that made the recording is stored in the *Artist* 
 (`IART`) WAV chunk: the device id, device type, revision, firmware version.

```
Id: 1234
Device: CWA
Revision: 17
Firmware: 44
```

#### Recording metadata

Metadata about the recording itself is stored in the *Title* (`INAM`) WAV chunk: session identifier, recording start time, recording stop time, sensor sample rate and sensitivity, user metadata

```
Session: 1
Start: 2014-08-01 10:00:00
Stop: 2014-12-08 10:00:00
Rate-A: 100,8
Metadata: 
```

#### Formatting metadata

Metadata about this WAV file is stored in the *Comment* (`ICMT`) WAV chunk: the time of the first sample, channel mapping, ranges.

```
Time: 2014-08-01 10:00:00.000
Channel-1: Accel-X
Scale-1: 8
Channel-2: Accel-Y
Scale-2: 8
Channel-3: Accel-Z
Scale-3: 8
Channel-4: Aux
```

The channel mapping can be used to identify the X-/Y-/Z-axes (which are currently always generated as the first/second/third three channels anyway), and the per-channel scaling (which is currently the same for all channels, and defaults to *8* for +/-8*g* range, and is *16* for +/-16*g* range).  This scaling factor is for normalized sample values (in the -1 to +1 range, note that the underlying signed 16-bit integers are in the range -32768 to +32767).




### Auxiliary channel data

The 16-bit value is treated as a bit-field:

  `ncttttuu vvvvvvvv`

Where the top two bits describe the validity of the data in this sample row:

|   Bits | Values | Meaning                                               |
| -----: | -----: | :---------------------------------------------------- |
|   `nc` |     00 | Data available, no channels clipped                   |
|   `nc` |     01 | Data available, some channels clipped                 |
|   `nc` |     10 | Data not available                                    |

The next four bits describe whether this row contains additional auxiliary data:

|   Bits | Values | Meaning                                               |
| -----: | -----: | :---------------------------------------------------- |
| `tttt` |   0000 | No auxiliary sample data                              |
| `tttt` |   0001 | Battery, 10-bit raw ADC reading in `uu vvvvvvvv`      |
| `tttt` |   0010 | Light, 10-bit raw ADC reading in `uu vvvvvvvv`        |
| `tttt` |   0011 | Temperature, 10-bit raw ADC reading in `uu vvvvvvvv`  |


## Return code

To make it easy for a calling application to determine whether processing completed successfully, a standard process exit code gives the reason the application terminated.  The application should check for a zero return code from the process to indicate successfully completed the conversion and writing of the information file.

The return code values are based on Linux `sysexits.h` `EX_` constants:

| Return code   	| Value	| Description                                                        |
| :-------------- | ----: | :----------------------------------------------------------------- |
| EXIT_OK         |     0 | Successfully converted (information file and output file written)  |
| EXIT_USAGE      |    64 | Command line usage error                                           |
| EXIT_DATAERR    |    65 | Data format error (input file cannot be parsed)                    |
| EXIT_NOINPUT    |    66 | Cannot open input                                                  |
| EXIT_SOFTWARE   |    70 | Internal software error                                            |
| EXIT_OSERR      |    71 | System error                                                       |
| EXIT_CANTCREAT  |    73 | Can't create output file                                           |
| EXIT_IOERR      |    74 | Input/output error (an individual read/write failed)               |
| EXIT_CONFIG     |    78 | Configuration error                                                |


## Information File

To make it easy for a calling application to confirm certain parameters about the process or data are as expected, the conversion generates easily-consumed metadata about the conversion process and the data.  The calling application may check the values in the information file match expectations (e.g. that the sample rate is 100Hz and +/-8g range, the requested recording duration is 7 days, start/stop times are 10am, the number of erroneous sectors is very low, the total output is 7 days, whether the auto-calibration was successful).

The metadata is written in a properties-like format (like HTML/SMTP headers, YAML subset, etc.), this is some information from the calibration process, concatenated with the existing header data from the WAV file.

```
:
::: Data about the conversion process
Result-file-version: 1
Convert-version: 1
Processed: 2015-01-01 12:00:00
File-input: /path/filename.cwa
File-output: /path/filename.wav
Results-output: /path/filename.txt
Auto-calibration: 1
Calibration-Result: 0
Calibration: 1.00,1.00,1.00,0.00,0.00,0.00,0.00,0.00,0.00,20
Input-sectors-total: 504002
Input-sectors-data: 504000
Input-sectors-bad: 0
Output-rate: 100
Output-channels: 4
Output-duration: 604800
Output-samples: 60480000
:
::: Data about the device that made the recording
Id: 1234
Device: CWA
Revision: 17
Firmware: 44
:
::: Data about the recording itself
Session: 123456
Start: 2014-08-01 10:00:00
Stop: 2014-12-08 10:00:00
Config-A: 100,8
Metadata:
:
::: Data about this file representation
Time: 2014-08-01 10:00:00.000
Channel-1: Accel-X
Scale-1: 8
Channel-2: Accel-Y
Scale-2: 8
Channel-3: Accel-Z
Scale-3: 8
Channel-4: Aux 
:
::: Data about the final state
Exit: 0
```
