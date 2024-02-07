# AX Devices for Physical Activity Research

## Introduction

The AX devices record raw, high-frequency data directly from an underlying movement sensor - this ensures the dataset remains useful for any future analysis that you may want to perform.  The devices are implementations of the Open Movement project ([openmovement.dev](https://openmovement.dev)), and the electronic designs, device firmware, supporting software, and documentation are all open source and freely available.

Raw movement data from an open source device, unlike proprietary solutions, is entirely open to inspection.  Published algorithms can be applied to process the raw data to derive metrics (see below).

AX devices have been used in many studies, for example: [Google Scholar: Open Movement AX3/AX6](https://scholar.google.com/scholar?q=%28%22Open+Movement%22+|+%22Axivity%22%29+%28%22AX3%22+|+%22AX6%22%29)


## Collecting Data

A typical study protocol might be to configure a device to record within a specific interval, then give/mail the device to be worn by a participant for the specified period.  Typically the device is mounted within a wrist strap for the best compliance, but could be attached at other sites.

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

The raw data can be loaded into many languages/processing environments, such as:

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


## Further Reading

For more information on the AX devices, please see:

* [AX Devices - Frequently Asked Questions](https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-faq.md)

