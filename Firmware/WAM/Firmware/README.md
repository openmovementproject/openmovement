# WAM - Wearable Acoustic Monitor

The WAM device is a continuous audio recorder. It logs audio in uncompressed format to internal memory and can continuously store data for a period of 7 days. 

The WAM has been designed to have a very simple operation, when it is unplugged from a computer USB port it is in "recording mode". The microphone for the WAM is located on the side of the device and is responsive for audio in the human vocal range. The sampling frequency of the WAM can be set to 8KHz or 16KHz (at 16-bit).

To charge the device the WAM needs to be plugged into a USB port for a period of 2 hours. While plugged in, any recorded data can also be downloaded from the device.  

## Audio File Names

The WAM will store the audio data in uncompressed format and make a new file for every hour of recording. Each file will be named according to the time it was recorded.

* Flush and start a new file every time the hour changes.

* Each file will be up to 57.6 MB, 168 files in a week (9676.8 MB).

* 112500 sectors + 1 header sector.

* Named: `\data\AdddHHMM.wav`

  * `A`   = Unit id ((id % 26) + 'A')
  * `ddd` = Zero-padded days since 1/1/2000 ((timestamp / 24 / 60 / 60) % 1000)
  * `HH`  = Zero-padded hours of the day (00-23)
  * `MM`  = Zero-padded minutes of the hour (00-59)

  
## Real-Time Clock

The WAM has an on-board real time clock it uses to time stamp its data. The clock can be set over a serial port exposed by the WAM by typing in:

```
time=yyyy/mm/dd hh:mm
```

The current time on the device can be queried by simply typing 

```
time
```

## File offset information

The device stores a 32-bit unsigned coded form of the current WAV file name, and binary offset in the WAV file.
These entries are stored, time-stamped, into the ADC channel.
Unfortunately, the ADC channel requires 16-bit numbers, so each value is split into a high (H) and low (L) half.
Also, the ADC channel is signed, so these get interpreted by the converter as 16-bit signed (short) integers

To convert the OMX file to CSV:

```
convert D:\DATA.OMX -stream l -out ADC.CSV
```

The converted version of the OMX file may look like this

```csv
YYYY-MM-DD hh:mm:ss.000,batt,light,prox,gain,inactivity,fileL,fileH,offsetL,offsetH
2014-01-16 15:31:41.000, 647,   -1,  -1,   0,         0, 5425, 8214,      0,      0
2014-01-16 15:31:42.000, 655,   -1,  -1,  82,         1, 5425, 8214,   5120,      0
2014-01-16 15:31:43.000, 655,   -1,  -1,  82,         2, 5425, 8214,  10240,      0
2014-01-16 15:31:44.000, 655,   -1,  -1,  82,         3, 5425, 8214,  20480,      0
2014-01-16 15:31:45.000, 655,   -1,  -1,  82,         4, 5425, 8214,  30720,      0
2014-01-16 15:31:46.000, 655,   -1,  -1,  82,         5, 5425, 8214, -29696,      0
2014-01-16 15:31:47.000, 656,   -1,  -1,  82,         6, 5425, 8214, -19456,      0
2014-01-16 15:31:48.000, 655,   -1,  -1,  82,         7, 5425, 8214, -14336,      0
2014-01-16 15:31:49.000, 655,   -1,  -1,  82,         8, 5425, 8214,  -4096,      0
2014-01-16 15:31:50.000, 655,   -1,  -1,  82,         9, 5425, 8214,   6144,      1
2014-01-16 15:31:51.000, 655,   -1,  -1,  82,        10, 5425, 8214,  11264,      1
2014-01-16 15:31:52.000, 655,   -1,  -1,  82,        11, 5425, 8214,  21504,      1
2014-01-16 15:31:53.000, 655,   -1,  -1,  82,        12, 5425, 8214,  26624,      1
2014-01-16 15:31:54.000, 655,   -1,  -1,  82,        13, 5425, 8214, -28672,      1
2014-01-16 15:31:55.000, 654,   -1,  -1,  82,        14, 5425, 8214, -18432,      1
2014-01-16 15:31:56.000, 655,   -1,  -1,  82,        15, 5425, 8214, -13312,      1
2014-01-16 15:31:57.000, 655,   -1,  -1,  82,        16, 5425, 8214,  -3072,      1
2014-01-16 15:31:58.000, 655,   -1,-  1,  82,        17, 5425, 8214,   2048,      2
```

Each value `(fileL, fileH, offsetL, offsetH)` must first have the sign corrected:

```c
if (fileL < 0) fileL = fileL + 65536;
if (fileH < 0) fileH = fileH + 65536;
if (offsetL < 0) offsetL = offsetL + 65536;
if (offsetH < 0) offsetH = offsetH + 65536;
```

Then, each low/high pair must be joined into the original 32-bit value:

```c
file = fileH * 65536 + fileL;
offset = offsetH * 65536 + offsetL;
```

The file value represented as an 8-digit (zero-padded on the left) hexadecimal number is a binary-coded decimal representation of the filename.

(The ASCII numbers '0'-'9' and letters 'A'-'F' are represented directly, the letters 'G'-'Z' wrap around the hex digit values).
An example would be the hex value of file 0x20161342 would represent the file name `I0161342.WAV`.

So, for the example values:

```c
  fileL=5425
  fileH=8214
```

...as unsigned values (unchanged): `fileL=5425, fileH=8214`

...and merged into a single 32-bit value: `538318129`

...and, as hexadecimal:     `0x20161531`

...(the filename is actually `I0161531.WAV`)

And, the example file sample offset after about 17 seconds:

```
offsetL=2048
offsetH=2
```

...as unsigned values (unchanged):

```
offsetL=2048, offsetH=2
```

...and merged into a single 32-bit value: 133120 (measured in samples)

...and with a sample rate of 8192Hz, this is roughly 16.25 seconds into the file.

IMPORTANT: Because of internal buffering, the given sound sample offset for a time is roughly +/- 0.64 seconds from the true value.
(some longer-term smoothing needs applying)


## Device Settings

`SETTINGS.INI` file configuration:

```
DEVICE 1        # 1-26 = A-Z
RECORD B        # <None|Sound|Accel|Both>
HOURS 0-23      # Hour range
SCRAMBLE 15     # <0=none,1=reverse,2=extension,4=hidden,8=header>
DEBUG 1         # Debug flash <0=none,1=startup-only,2=on-press,3=always>
#LOCK <code>    # Lock code
INACTIVITY=0
RATE=73         # "73"=50Hz+-8g, "74"=100Hz+-8g; subtract 64 for +-16g sensitivity, add 64 for +-4g sensitivity, 128 for +- 2g sensitivity.
#KEY[R]=key     # Encryption key
#PROXIMITY=10000
HIBERNATE 0
STOP -1
```
