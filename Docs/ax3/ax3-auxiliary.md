# AX3/AX6 Data

This document describes advanced details of AX device data.  For additional information, see [Technical Documentation](ax3-technical.md) for details on how the data is stored, and [Sensor Data Characteristics](ax3-faq.md#sensor-data-characteristics) for details of the underlying sensing device.


## Auxiliary Data: Battery, Temperature, Light

The AX3/AX6 device primarily records high-frequency movement data, but also record additional, low-frequency, *auxiliary* data.  This data includes light level indication, device temperature, and device battery voltage.  This data is always recorded, regardless of configuration, whenever the device is logging movement data.

The sensors are internally sampled at 1 Hz, then the most recently sampled value is only stored once per written data "sector" -- this is every 120 samples (AX3 packed mode), 80 samples (AX3 unpacked mode, or AX6 in accelerometer-only mode), or 40 samples (AX6 with gyroscope enabled).

See below for specific information on the battery, temperature, and light.


### Battery Voltage

The device battery voltage is sampled as a 10-bit ADC value (0-1023).  

Note that the battery ADC reading is stored into an 8-bit value (0-255) in the [.CWA file format](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-technical.md#cwa-file-data-blocks) as only the top half of this range is useful.  Stored as: ${packed = \lfloor(value - 512) / 2\rfloor}$; and restored on reading as: ${value = packed * 2 + 512}$.  

To convert the battery ADC values into voltage (Volts), the conversion is:

$$
voltage = value * 6 / 1024
$$


### Device Temperature

The device internal temperature is measured by an on-board temperature sensor ([MCP9700](https://www.microchip.com/en-us/product/MCP9700)) which outputs the temperature as a linearly changing voltage of $10 mV/&deg;C$, with an offset of $500 mV @ 0&deg;C$: $voltage = temperature * 0.01 + 0.5$.  This is sampled and stored as a 10-bit ADC value (0-1023): $value = voltage * 1024 / 3$.  The voltage (Volts) is calculated from the ADC value as: $voltage = value * 3 / 1024$.  As $temperature = (voltage - 0.5) * 100$, this is $temperature = (value * 3 / 1024 - 0.5) * 100$, and simplified below.

To convert the temperature ADC values into degrees Celsius, the simplified conversion is:

$$
temperature = (value * 75 / 256) - 50
$$

<!--
The internal temperature sensor is useful for auto-calibration of the movement data.
-->


### Light Level Indicator

The AX devices use a light sensor ([APDS-9007](https://docs.broadcom.com/docs/AV02-0512EN)) which has a logarithmic response over a wide dynamic range.  The sensor arrangement is most suitable as a general, relative, indicator of light, for example to distinguish a varying/stable level, or daily maxima/minima. 

The AX light level indicator is complicated as, in order to not compromise the enclosure's protection, the sensor is used without a full optical window, and so its view is through the (partially transparent) case material and, if used, (partially transparent) strap.  Sensors worn on the wrist might become obscured in use by the wearer's clothing or bedclothes, and subject to reflections, shadows, etc.  

The logarithmic output means that light is detectable through the enclosure or strap. For example, changes from 10-100 lux and 1 Klux - 10 Klux both have a relative change of *10* (which is a *10 uA* difference in the sensor output, and the AX3 uses a load resistor of 100 kOhm to convert this 10 uA change into a 1 V change, which becomes 341.3 raw 10-bit ADC units).

**AX6:** To make better use of the range, the AX6 uses a load resistor of only 10 kOhm (e.g. converts a 10 uA change into a 0.1 V change, which becomes 34.13 raw ADC units).  For AX6 data, multiply raw values by 10 to be equivalent to the AX3 raw values discussed below.

For many applications, it may be best to use the raw ADC values as a relative indicator of light, as it remains in the linear space of perception.  If you are certain that you want the Lux value, to convert the light ADC values into Lux, the conversion is:

$$
lux = 10^{(value + 512) * 6 / 1024}
$$

As the recorded level is logarithmic, even small measurement noise or inter-device variation will become large when converting to lux.  As lights tends towards the upper limit, the output curve of the sensor starts to become shallow in the kLux ranges, so digitized values may become increasingly noisy towards saturation.


## Exporting/Loading Auxiliary Data

### OmGui

Light and temperature can be exported in *OmGui* software by following these steps:

1. Ensure your *Workspace* set to a folder containing `.cwa` data files (use *Choose Working Folder* to select a folder).
2. In the *Data Files* tab, select the `.cwa` data file you would like to export the data from.
3. Click *Plugins...* in toolbar in the *Data Files* tab.
4. Ensure *Convert CWA* is selected as the Plugin, and click *Run...*
5. You must enter a file name in the *Output file name and format* box, the same as the source `.cwa` file is recommended.
6. Ensure *CSV* is selected.
7. Under *Streams to convert*, click *Light* / *Temperature* / *Battery* as required.
8. Select a time format, *String yyyy-mm-dd hh:mm:ss.f* is recommended.
9. Click *Convert Now*
10. A `.csv` file, with the name you specified, will be created in the Workspace folder.

The exported `.csv` columns will be, for AX3 or AX6 (accelerometer-only mode):

> `Time, Ax(g), Ay(g), Az(g), LightADC, TemperatureADC, BatteryADC`

...or, for AX6 (with gyroscope enabled):

> `Time, Ax(g), Ay(g), Az(g), Gx(d/s), Gy(d/s), Gz(d/s), LightADC, TemperatureADC, BatteryADC`

`LightADC`, `TemperatureADC` and `BatteryADC` depend on the selected *Streams to convert*, and are to be interpreted as described above


### CSV Exporter

One way to quickly export just the auxiliary data from the CWA file is to use the [`cwa-convert`](https://github.com/digitalinteraction/openmovement/tree/master/Software/AX3/cwa-convert/c) tool to create a .CSV file.  See link for cross-platform usage or, if you have [OmGui](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI) installed, you can run the command (where `CWA-DATA.CWA` is your filename):

```cmd
"%ProgramFiles(x86)%\Open Movement\OM GUI\Plugins\Convert_CWA\cwa-convert.exe" "CWA-DATA.CWA" -nodata -t:block -skip 240 -light -temp -batt > "OUTPUT.CSV"
```

The export will skip 240 samples, so not export quite every auxiliary value, but they are slow-changing.  The exported columns will be:

> `Time, LightADC, TemperatureADC, BatteryADC`

You can alternatively output the battery voltage with the `-battv` parameter, and the temperature in degrees Celsius with the `-tempc` parameter (the latter will only work on later versions of `cwa-convert`).

If you would like to export the accelerometer (and, if present, gyroscope) data, along with (possibly duplicated) auxiliary data, remove the `-nodata`, `-t:block`, and `-skip` parameters:

```cmd
"%ProgramFiles(x86)%\Open Movement\OM GUI\Plugins\Convert_CWA\cwa-convert.exe" "CWA-DATA.CWA" -light -temp -batt > "OUTPUT.CSV"
```

The exported columns will be, for AX3 or AX6 (accelerometer-only mode):

> `Time, Ax(g), Ay(g), Az(g), LightADC, TemperatureADC, BatteryADC`

...or, for AX6 (with gyroscope enabled):

> `Time, Ax(g), Ay(g), Az(g), Gx(d/s), Gy(d/s), Gz(d/s), LightADC, TemperatureADC, BatteryADC`

`LightADC`, `TemperatureADC` and `BatteryADC` depend on the specified options, and are to be interpreted as described above.

<!--
Unlabelled data can usually be identified as the A columns will have a vector magnitude of 1 at rest, the G columns will have a vector magnitude of 0 at rest, the light reading will generally have cases of abrupt fluctuations, while the temperature reading will only change gradually.  Raw ADC values, rather than converted values, can be spotted as they are always whole numbers in the range 0-1023.
-->


### MATLAB Loader

The MATLAB loader [CWA_readFile.m](https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/Analysis/Matlab/CWA_readFile.m) can be used to load the light (raw values) and temperature (Celsius):

```matlab
data = CWA_readFile('CWA-DATA.CWA', 'modality', [1 1 1]);
% data.AXES  %% (time Ax Ay Az) or (time Ax Ay Az Gx Gy Gz)
% data.LIGHT %% (raw light values)
% data.TEMP  %% (degrees Celsius)
```


### Python Loader

The Python loader [openmovement-python](https://github.com/digitalinteraction/openmovement-python#cwa_load---cwa-file-loader) can be used to load the light and temperature ADC values as follows:

```python
with CwaData(filename, include_gyro=False, include_light=True, include_temperature=True) as cwa_data:
    # As an ndarray of [time,accel_x,accel_y,accel_z,light,temperature]
    sample_values = cwa_data.get_sample_values()
    # As a pandas DataFrame
    samples = cwa_data.get_samples()
```


## Sensor Data Calibration

An explicit design goal of the AX devices was to record only the raw data from an underlying sensor, with everything else applied afterwards in software.

The sensor data is initially bound within manufacturer's tolerances, as described in the [data sheets](ax3-faq.md#sensor-data-characteristics).  The Open Movement quality control testing procedures (e.g. 5-day delay and 8-day recording) are designed to filter out any devices that report data outside of specific thresholds.

Accelerometer data can have a small amount of per-axis *offset* and *gain* error.  This may change based on temperature, ageing or, to a small extent, even local gravity variation.  A static calibration can be insufficient for complete correction, and so a dynamic "auto-" calibration is a standard part of accelerometer data analysis.  

The self-calibration technique was popularized in [Autocalibration of accelerometer data for free-living physical activity assessment using local gravity and temperature: an evaluation on four continents](https://journals.physiology.org/doi/full/10.1152/japplphysiol.00421.2014) by van Hees et al.  

Implementations of this technique are in software such as:

* [GGIR](https://cran.r-project.org/web/packages/GGIR/vignettes/GGIR.html#52_Auto-calibration)
* [biobankAccelerometerAnalysis](https://github.com/OxWearables/biobankAccelerometerAnalysis#under-the-hood)
* [omconvert](https://github.com/digitalinteraction/omconvert#analysis-methods)
* [Open Movement - MATLAB](https://github.com/digitalinteraction/openmovement/blob/master/Software/Analysis/Matlab/estimateCalibration.m)

Auto-calibration uses the variety of orientations observed over the recording to determine calibration parameters which can then be applied to the data.  It is also possible to re-use calibration parameters from one device session to another session from the same device, and this would be best where the temperature ranges observed are roughly comparable.

Analysis algorithms are typically derived from datasets with either raw or auto-calibrated data.  In addition, some algorithms are designed to have a low-sensitivity to calibration.  For example, a high-pass filter can be used to drastically reduce the effect of any systematic offset.


## Sensor Fusion: Inertial Measurement Unit

A MEMS accelerometer should perhaps be more accurately be described as an inertial sensor: they not only capture acceleration, but will show a deflection from gravity even when at rest, and this gravity vector changes with the orientation of the sensor.  The resultant signal can therefore be seen as a mix of rotation and acceleration.  One consequence of this is that you cannot, in the general case (e.g. without other constraints or knowledge), easily infer velocity by just using an accelerometer. 

However, signals from an accelerometer *and* a gyroscope can be fused to separate rotation relative to gravity, and *liner acceleration*.  This linear acceleration will be somewhat noisy but could, for example (at least in theory), be used to try to infer approximate velocity (it would likely have to use some heuristics to reset the integration when it is deemed to be stationary).  Again, theoretically, this velocity could in be integrated to approximate position (but any original noise would accumulate very rapidly). 

If you want to combine the accelerometer/gyroscope data, known as *sensor fusion*, to act as a single inertial measurement unit (IMU) device, you could consider:

* MATLAB’s Inertial Sensor Fusion Algorithms: https://uk.mathworks.com/help/fusion/inertial-sensor-fusion.html

* Sensor fusion in the open source GGIR package in the R language: https://cran.r-project.org/web/packages/GGIR/vignettes/SensorFusionWithGGIR.html

* Adapting some previous sensor fusion work in Open Movement (for another sensor), *Azimuth*: https://github.com/digitalinteraction/openmovement/tree/master/Software/Azimuth

* A custom solution based on the raw data

Note that some special cases/constraints allow some values to be calculated from the raw sensor data, without sensor fusion.  For example:

* The *roll* angle with respect to ground (perpendicular to gravity) can be calculated from low-frequency accelerometer data using *atan2* of the two axes normally in the ground plane.  The roll angle becomes undefined as it approaches the gravity vector. 

* The *pitch* angle with respect to ground (perpendicular to gravity) can be calculated from low-frequency accelerometer data using *atan2* of the axis perpendicular to gravity, and the vector length of the other two axes (square root of the sum of the other two axis values squared). The pitch ange becomes undefined as it approaches the ground plane. 

* If the device was perfectly aligned with the axis of interest, then the gyroscope value for the corresponding axis will be the rate of change (degrees-per-second) for that axis.
