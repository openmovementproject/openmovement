# AX Devices for Physical Activity Research

## Introduction

The AX devices record raw, high-frequency data directly from an underlying movement sensor - this ensures the dataset remains useful for any future analysis that you may want to perform.  The devices are implementations of the Open Movement project ([openmovement.dev](https://openmovement.dev)), and the electronic designs, device firmware, supporting software, and documentation are all open source and freely available.

Raw movement data from an open source device, unlike proprietary solutions, is entirely open to inspection.  Published algorithms can be applied to process the raw data to derive metrics (see below).

AX devices have been used in many studies, for example: [Google Scholar: Open Movement AX3/AX6](https://scholar.google.com/scholar?q=%28%22Open+Movement%22+|+%22Axivity%22%29+%28%22AX3%22+|+%22AX6%22%29)


## Collecting Data

A typical study protocol might be to configure a device to record within a specific interval, then give/mail the device to be worn by a participant for the specified period.  Typically the device is mounted within a wrist strap for the best compliance, but could be attached at other sites.  See also [AX Quick Start Guide](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/AX3%20Quick%20start%20guide.pdf).

The maximum duration of a single recording depends on the device and configuration, but for a typical rate of 100 Hz, this is almost two weeks.  For details, please see: [AX Devices FAQ](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-faq.md#maximum-recording-duration).  If a longer recording is required, a replacement configured unit could be mailed with instructions to swap them over and to return the first unit.  

Whenever a unit is returned, the data can be downloaded, the device recharged, then reconfigured, and sent out to another participant.  The recommended software for configuring and downloading is [OmGui](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI).


## Data Analysis

The raw accelerometer data can be processed to derive metrics, and there are many published algorithms for analysing raw accelerometer data.  

Some simple analysis methods are available through the open source OmGui configuration tool, as described at: [OmGui - Analysis and Data Manipulation](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI#analysis-and-data-manipulation). 

For a richer analysis, a popular option in research is the open source *GGIR* package, which includes sleep analysis and can also produce more visual reports.  A good introduction to the package is given in: [GGIR: A Research Community–Driven Open Source R Package for Generating Physical Activity and Sleep Outcomes From Multi-Day Raw Accelerometer Data](https://journals.humankinetics.com/view/journals/jmpb/2/3/article-p188.xml), and a *getting started* guide is available at: [Accelerometer data processing with GGIR](https://cran.r-project.org/web/packages/GGIR/vignettes/GGIR.html).  Step counting can be [added as an external function](https://cran.r-project.org/web/packages/GGIR/vignettes/ExternalFunction.pdf): [Verisense Step Count Algorithm for GGIR](https://github.com/ShimmerEngineering/Verisense-Toolbox/tree/master/Verisense_step_algorithm). 

Alternative analysis toolsets include: [biobankAccelerometerAnalysis](https://github.com/OxWearables/biobankAccelerometerAnalysis) (Oxford Wearables Group, Oxford University; Academic Use Licence; not currently compatible with AX6 data); [pampro](https://github.com/Thomite/pampro) (MRC Epidemiology, Cambridge University; not compatible with AX6 data).

Additionally, any available algorithm designed to process raw data might also be used, e.g. from a published descriptions of an approach to handling the raw data, or open source code: [Google Scholar: Accelerometer Analysis](https://scholar.google.com/scholar?q=%22accelerometer%22+%22analysis%22) ...as there is broad equivalence between raw accelerometer sensors for physical activity, as found by, e.g.:

   * Ladha, C., Ladha, K., Jackson, D., & Olivier, P. (2013, June). Shaker table validation of Open Movement AX3 accelerometer. In Ahmerst (ICAMPAM 2013 AMHERST): In 3rd International Conference on Ambulatory Monitoring of Physical Activity and Movement (pp. 69-70).

   * Rowlands, A. V., Mirkes, E. M., Yates, T., Clemes, S., Davies, M., Khunti, K., & Edwardson, C. L. (2017). Accelerometer-assessed physical activity in epidemiology: are monitors equivalent?

   * Feng, Y., Wong, C. K., Janeja, V., Kuber, R., & Mentis, H. M. (2017). Comparison of tri-axial accelerometers step-count accuracy in slow walking conditions. Gait & posture, 53, 11-16.

   * Crowley, P., Skotte, J., Stamatakis, E., Hamer, M., Aadahl, M., Stevens, M. L., ... & Holtermann, A. (2019). Comparison of physical behavior estimates from three different thigh-worn accelerometers brands: a proof-of-concept for the prospective physical activity, sitting, and sleep Consortium (ProPASS). International Journal of Behavioral Nutrition and Physical Activity, 16(1), 1-7.


### Raw data

The [raw data](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-technical.md#measurement-data) can be loaded into many languages/processing environments, such as:

   * [MATLAB](https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/Analysis/Matlab/CWA_readFile.m)
   * [R](https://cran.r-project.org/web/packages/GGIR/index.html)
   * [Python](https://github.com/digitalinteraction/openmovement-python/#cwa_load---cwa-file-loader)
   * [C](https://github.com/digitalinteraction/libomapi/)
   * [Other languages](https://github.com/digitalinteraction/libomapi/tree/master/bindings)

...or, indirectly via another file format:

   * [CSV-exported from the GUI tool](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI#export-function)
   * [CSV batch export of raw data](https://github.com/digitalinteraction/openmovement/tree/master/Software/AX3/cwa-convert/c#readme) (cross-platform)
   * [CSV batch export of fixed-rate data](https://github.com/digitalinteraction/omconvert/#file-conversion) (cross-platform)
   * [Multi-channel .WAV file](https://github.com/digitalinteraction/omconvert/blob/master/src/omconvert/README.md) (cross-platform)
 
A sample data file (inside a `.zip` archive) is available at:

  * [longitudinal_data.zip](https://github.com/digitalinteraction/openmovement/raw/master/Downloads/AX3/longitudinal_data.zip)


### Analysis Expertise

In addition to the open source software and documentation discussed above, other organisations and people do offer expertise in analysing data from Open Movement devices.  Including the following (Note: the Open Movement project does not necessarily endorse, nor is responsible for, any third-parties):

* *McRoberts* offer [analytics using Open Movement sensors](https://mcroberts.nl/axivity-services). 
* *Dr. Vincent van Hees* (author of *GGIR* analysis package) offers [consulting on movement data](https://accelting.com), including Open Movement sensors.
* *Dr Rob Thompson* offers [consulting on animal movement](https://rjtanalytics.com). 


## Large-scale deployments

For large-scale deployments, you may be interested in the following:

1.	The standard *OmGui* configuration software can configure multiple devices at once (select multiple devices using the standard Shift+click for range selection, and Ctrl+click for toggle selection) before configuring - but only as long as they have an identical configuration.  However, it is recommended to set a unique session ID for each device, so this option may not be suitable:

    * [OmGui](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI)
 
2.	The *AX-Deploy* software is specifically for mass deployments, where each device may have a unique configuration:
 
    * [AX-Deploy Website](https://github.com/digitalinteraction/ax-deploy/)
    *	[AX-Deploy Documentation](https://digitalinteraction.github.io/ax-deploy/)
 
    ...this tool can be used to configure devices quickly by scanning a barcode with a "configuration code" (see the bottom of the documentation) containing session identifier.  This is not be compatible with AX6 device gyroscopes, however, it may still be suitable to use for batch download/clearing.

3.	There is a web-based configuration tool that can be used for one-at-a-time configuration (but not download), which does have the advantage that you can easily construct URLs to assign specific configurations/session IDs etc, and also may work from Android tablets/phones (check with specific devices).

    * [AX-Config Website](https://config.openmovement.dev/)
    * [AX-Config Documentation](https://github.com/digitalinteraction/openmovement-axconfig/blob/master/README.md)

4.	The AX device communication is documented, and a software API exists that can be used to create your own software to communicate with the AX devices:
 
    * [AX3 Technical Information](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-technical.md)
    * [AX libOMAPI](https://github.com/digitalinteraction/libomapi/)

5.  Third-party support for configuring, downloading and analysing data is available (Note: the Open Movement project does not necessarily endorse, nor is responsible for, any third-parties):

    * [McRoberts: data collection and processing using Open Movement sensors](https://mcroberts.nl/axivity-services). 

## Wear sites

AX devices are typically mounted at the wrist and this has been shown to have a very high compliance as participants are generally happy to wear a wrist-worn device for long periods of time.  The Open Movement project includes a [silicon wristband](https://github.com/digitalinteraction/openmovement/blob/master/Mechanical/AX3/AX3%20Band/AX3_Band.PDF).  The band and device are designed to be worn for [daily activities](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-faq.md#usage-environments).  There are also designs for smaller [infant fabric bands](https://github.com/digitalinteraction/openmovement/blob/master/Mechanical/AX3/AX3%20Fabric%20Band/README.md), although direct use on infants is not recommended without a sufficiently-sized enclosure design (must not fit within a choke test cylinder) that requires a tool to remove.

Alternative means may be used to attach the device at other sites (such as the hip, thigh or back) using medical tape/patches, or a cohesive bandage.  

One attachment method (Schneller et al, *Measuring children's physical activity: compliance using skin-taped accelerometers*):

1. Clean skin using an alcohol wipe and dry.
2. Take a 30x50mm piece (a little larger than the device) of Fixomull Stretch tape (02036-00; BSN Medical)
3. Take a small, 10x20mm piece of 3M Scotch double-sided adhesive tape (3M, Hair-set) and secure on top of the Fixomull.
4. Mount the AX device on the double adhesive tape.
5. Place the combined AX/Fixomull on the on the clean, dry skin of the subject.
6. Adhere an 80x100mm piece of Opsite Flexifix (Smith & Nephew, #66000041) with rounded corners, on top of the accelerometer.
7. Use firm hand pressure on the tape and the device for the pressure-sensitive adhesives

An alternative attachment method is described as: *two layers of Flexifix (WC690-Smith and Nephew Opsite Flexifix Film Roll 5cmx10m) with the sensor between the layers.*

There have also been third-party purpose-built foam pouches, such as those by [Herpatech AB](http://herpatech.se) (now [nile.se](https://nile.se)).


### Non-human use

The AX devices were originally designed for human movement but, as they are general purpose loggers, they have been used on animals and other locations.

While the logger case is designed for IP68 rating, the device left as it is it could still be damaged by activities outside of normal human movement (e.g. impacts against objects, or animals biting the case), and an unprotected device could pose a choke hazard and includes material that should not be ingested.  Dirt should be prevented from becoming lodged inside the USB connector (and the USB connector should never be used while it is dirty or damp as that might cause abrasion or corrosion to the contacts – it must only be connected once clean and dry).  Additionally, the device was designed to be in contact with a body, so should not be exposed to [extreme environments](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-faq.md#usage-environments) (extreme temperature or moisture).  If the case is covered in additional material, a desiccant bag could be used in with the device to ensure moisture is not trapped with the device for extended periods.

If the device could become dislodged, a brightly-colored outer layer may also aid in locating the device.  If relevant, to dissuade against an animal biting a device, consider (if suitable) something like [Bitrex](https://www.bitrex.com/about-bitrex/what-is-bitrex), or a "prickly" deterrant outer layer, or a true physical barrier (e.g. harder protective outer enclosure).  

Some animal research has attached to limbs using [3M Vetrap](https://www.3m.co.uk/3M/en_GB/p/d/v000106104/).  As another example for sensor attachment, from the paper "Freedom to lie: How farrowing environment affects sow lying behavior assessment using inertial sensors" by Thompson et al. - Section 2.1.2. "Sensor protocol" states: 

> *All sows were shaved and cleaned in two locations: at the rear, just above the tail head and between the hip bones; and at the neck, halfway between the shoulder and the base of the skull, […]. Sensors were wrapped in duct tape to provide further protection should they become dislodged. A coating of Evo-Stik contact adhesive was applied in a 2 cm patch around the sensor which was further secured with a layer of Scapa Sniper tape.*


## Further Reading

For more information on the AX devices, please see:

* [AX Devices - Frequently Asked Questions](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-faq.md)

