# AX3/AX6 FAQ

## Getting Started

* [AX Device Overview](https://github.com/digitalinteraction/openmovement/wiki/AX3)
* [AX Quick Start Guide](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/AX3%20Quick%20start%20guide.pdf)
* [AX-GUI Configuration and Processing Software](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI)
* [AX Devices for Physical Activity Research](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-research.md) for information about using the AX devices in physical activity research.


## Troubleshooting

* [AX Troubleshooting Guide](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-troubleshooting.md) for help with connecting or using the devices.


## Maximum recording duration

Approximate maximum recording duration from a single full battery charge.  A typical configuration may be 100 Hz &plusmn;8 _g_.

| Device  | Configuration  |   Sample Capacity |       6.25 Hz |     12.5 Hz |      25 Hz |    50 Hz |  100 Hz | 200 Hz | 400 Hz | 800 Hz | 1600 Hz | 3200 Hz |
|---------|----------------|------------------:|--------------:|------------:|-----------:|---------:|--------:|-------:|-------:|-------:|--------:|--------:|
| AX3     | packed         | 119009040 samples |             - | ~65d (110d) | ~48d (55d) |     27d* |  13.5d* |   6.8d |   3.4d |  41.3h |   20.6h |   10.3h |
| AX3     | unpacked       |  79339360 samples |  >65d? (146d) | ~65d? (73d) |      36.5d |      18d |     9d* |   4.5d |   2.3d |  27.5h |   13.7h |    6.8h |
| AX6     | accel. only    | 158714880 samples | >146d? (293d) |        146d |        73d |    36.5d |  18.3d  |   9.1d |   4.5d |  55.1h |   27.5h |       - |
| AX6     | accel. + gyro. |  79357440 samples |             - |           - | 7-9d (36.7d) | 7-9d (18d) | 7-9d* |   4.5d |   2.3d |  27.5h |   13.7h |       - |

* `d` days (<= 400 Hz)
* `h` hours (>= 800 Hz)
* `-` an unsupported configuration. 
* `?` an extrapolated best-guess
* `~` uncertainty in the value (charge-limited, and device-dependent)
* `()` where a recording is likely to be battery-limited, times in brackets are the storage capacity (which could only be reached if recharged). 
* `*` where the storage- and battery-limits are similar, which is reached first may be device-dependent. 

<!-- AX6 12.5Hz Accelerometer-only 149 days 100%-39% battery -->


## Sensor Data Characteristics

The AX6 and AX3 have the same physical dimensions, but the AX6 has more memory and battery capacity, and a different underlying sensor that allows it to (optionally) record gyroscope data.

| Characteristic             | AX3                                             | AX6                                           |
|----------------------------|-------------------------------------------------|-----------------------------------------------|
| Sample Rate                | 6.25`*`/12.5/25/50/100/200/400/800/1600/3200 Hz | 6.25`*`/12.5`*`/25/50/100/200/400/800/1600 Hz |
| Accelerometer Range        | &plusmn;2/4/8/16 _g_                            | &plusmn;2/4/8/16 _g_                          |
| Gyroscope Range            | _none_                                           | 125/250/500/1000/2000 &deg;/s                 |
| Underlying sensing device  | [Analog Devices ADXL345](https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf) | [Bosch BMI160](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi160-ds000.pdf) |
| Notes                      | `*` 'Unpacked' recording mode only.             | `*` With gyroscope off.                       |

<!-- | Samples per 512 byte storage block | Packed (10-bit mode): 120; Unpacked (full resolution): 80. | Accelerometer-only: 80; Accelerometer+Gyroscope: 40. | -->

* Sensor axes: [AX Sensor Axes](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-rotation.md)
* Device orientation: [AX Orientation](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax-orientation.png)


## Synchronizing data between devices, or with other devices

The AX devices have an internal real-time clock to keep track of time.  When the device is configured, the internal clock is set to the system time of the configuring computer.  This time is set to the nearest second, and the operation itself may be subject to some jitter (e.g. from the operating system performing other actions).  Afterwards, as with any clock, it will be subject to clock drift, potentially of the order of around ±2 seconds per day.  
 
There is no external communication while the AX devices are recording, so they cannot directly have their clocks set or read during normal use.  However, depending on your application, you may have some options, including:
 
* For a single sensor over, say, a week or so, this clock drift rate may be acceptable to directly combine with some external sensors, such as GPS or a mobile phone (assuming the configuring computer had a similar time when configuring).
 
* If you are placing multiple accelerometers on a single moving body over a long period of time (e.g. a person), then there is some software that synchronizes the signal from devices that are likely to see a similar movement: [timesync](https://github.com/digitalinteraction/timesync/).
 
* Where the setup/access allows (e.g. a lab-based recording, or one with frequent points of contact), you can introduce a "marker" -- a specific movement signal at one or more points (e.g. vigorous shaking before and after a session) that has its time externally recorded.  For a lab-based session, it might be appropriate to video record the session in a way that captures the shaking times directly.  It might be useful to introduce an external clock on a screen, e.g. this page: [Time Sync Clock](https://config.openmovement.dev/timesync/) -- on some supported phones/browsers (e.g. Chrome browser on Android), you can hold the phone against the device and tap-and-hold the screen to introduce an optical and vibration marker/pattern for the time.  You can also use this page to create event markers (by tapping the screen, clicking a mouse, or pressing keys on a keyboard), then download or clear the log of event markers with the buttons at the top of the page.


## Time Zone and DST

The AX devices take the configuring device's local time at the time of configuration, and the time and date continue from there for any subsequently recorded data.  They cannot adjust for timezone changes or daylight savings, as they will not know the current country or locally-defined time zone or daylight savings rules, and also as that could lead to duplicate or skipped hours of the day.  As long as you know the configuration time zone (the last configuration time is recorded in the .CWA data file), you could choose to later reinterpret these times in another time zone (including applying any DST change), but you would have to choose what to do with any duplicate or skipped hours of the day.


## Usage environments

The AX enclosure is designed for IP68 rated water resistance -- not technically "waterproof" (i.e. not ATM rated) as it is a static measure, and not dynamic.  It was designed so that it can be used for daily activities, which includes washing and casual swimming.  Importantly, not continuous immersion, substantial depth, extreme/saltwater/watersports activities, sauna, etc.  Additional care should be taken to avoid impacts while immersed.  The device must be removed and not used if there are any problems or indications of damage.  The USB socket must be free of debris and completely dry before connection.


## Technical Details

* [AX3 Datasheet](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/AX3v2%20Datasheet.pdf)
* [AX6 Datasheet](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/AX6%20Datasheet.pdf)
* [AX Device - Technical Documentation](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-technical.md) for firmware details
* [AX3/AX6 Auxiliary Data](ax3-auxiliary.md) for details of using temperature and light sensor data.
* [Open Movement API](http://digitalinteraction.github.io/openmovement/omapi/html/)
* [AX Case Dimensions](https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Docs/ax3/ax-case-dimensions.svg)
* [Can .CWA files be edited?](https://github.com/digitalinteraction/omsynth/blob/master/README.md#splicing-a-cwa-file)
