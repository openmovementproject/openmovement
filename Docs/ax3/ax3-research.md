# AX Devices for Physical Activity Research

## Introduction

The AX devices record raw, high-frequency data directly from an underlying movement sensor - this ensures the dataset remains useful for any future analysis that you may want to perform.  The devices are implementations of the Open Movement project (https://openmovement.dev) - the electronic designs, device firmware, supporting software and documentation are all open source and freely available.

AX devices have been used in many studies, e.g.: https://scholar.google.com/scholar?q=(%22Open+Movement%22+|+%22Axivity%22)+(%22AX3%22+|+%22AX6%22)


## Collecting Data

A typical study protocol might be to configure a device to record within a specific interval, then give (or mail) the device to be worn by a participant for the specified period.  

The maximum duration of a single recording depends on the device and configuration, but for a typical rate of 100 Hz, this is almost two weeks.  For details, please see: https://github.com/digitalinteraction/openmovement/blob/master/Docs/ax3/ax3-faq.md#maximum-recording-duration

If a longer recording is required, a replacement configured unit could be mailed with instructions to swap them over and to return the first unit.  

Whenever a unit is returned, it can have the data be downloaded, device recharged, reconfigured, and sent out to another participant.  The recommended software for configuring and downloading is OmGui: https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI


## Data Analysis

The raw accelerometer data can be processed to derive metrics, and there are many published algorithms for analysing raw accelerometer data.  

Some simple analysis methods are available through the OmGui configuration tool, as described at: https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI#analysis-and-data-manipulation. 

For a richer analysis, a popular option in research is the open source "GGIR" package, which includes sleep analysis and can also produce more visual reports.  A good introduction to the package is given in: https://journals.humankinetics.com/view/journals/jmpb/2/3/article-p188.xml  ...and a "getting started" guide is available at: https://cran.r-project.org/web/packages/GGIR/vignettes/GGIR.html.  Step counting can be added as an external function (https://cran.r-project.org/web/packages/GGIR/vignettes/ExternalFunction.pdf): https://github.com/ShimmerEngineering/Verisense-Toolbox/tree/master/Verisense_step_algorithm. 

Additionally, any available algorithm designed to process raw data might also be used, and there is broad equivalence between raw accelerometer sensors for physical activity, e.g.:

* Ladha, C., Ladha, K., Jackson, D., & Olivier, P. (2013, June). Shaker table validation of Open Movement AX3 accelerometer. In Ahmerst (ICAMPAM 2013 AMHERST): In 3rd International Conference on Ambulatory Monitoring of Physical Activity and Movement (pp. 69-70).

* Rowlands, A. V., Mirkes, E. M., Yates, T., Clemes, S., Davies, M., Khunti, K., & Edwardson, C. L. (2017). Accelerometer-assessed physical activity in epidemiology: are monitors equivalent?

* Feng, Y., Wong, C. K., Janeja, V., Kuber, R., & Mentis, H. M. (2017). Comparison of tri-axial accelerometers step-count accuracy in slow walking conditions. Gait & posture, 53, 11-16.

* Crowley, P., Skotte, J., Stamatakis, E., Hamer, M., Aadahl, M., Stevens, M. L., ... & Holtermann, A. (2019). Comparison of physical behavior estimates from three different thigh-worn accelerometers brands: a proof-of-concept for the prospective physical activity, sitting, and sleep Consortium (ProPASS). International Journal of Behavioral Nutrition and Physical Activity, 16(1), 1-7.

The raw data can be loaded into many processing environments, such as:

* MATLAB: https://raw.githubusercontent.com/digitalinteraction/openmovement/master/Software/Analysis/Matlab/CWA_readFile.m
* R: https://cran.r-project.org/web/packages/GGIR/index.html
* Python https://github.com/digitalinteraction/openmovement-python/#cwa_load---cwa-file-loader
* C (https://github.com/digitalinteraction/libomapi/
* ...other languages: https://github.com/digitalinteraction/libomapi/tree/master/bindings
* Indirectly via a multi-channel .WAV file: https://github.com/digitalinteraction/omconvert/blob/master/src/omconvert/README.md
* CSV-exported using the software: https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI#export-function
* CSV batch export of raw data using the cross-platform cwa-convert: https://github.com/digitalinteraction/openmovement/tree/master/Software/AX3/cwa-convert/c#readme
* CSV batch export of fixed-rate data using the cross-platform omconvert: https://github.com/digitalinteraction/omconvert/#file-conversion
 
All of the above linked software is freely available, and a sample data file (inside a .ZIP archive) is available at: https://github.com/digitalinteraction/openmovement/raw/master/Downloads/AX3/longitudinal_data.zip

