/*
* Copyright (c) Newcastle University, UK.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

// "AG aggregation method and band-pass filter" coefficients
// Dan Jackson

// Filter coefficients from: 
//   "Frequency filtering and the aggregation of raw accelerometry into Actigraph counts."
//   Jan Brond, Daniel Arvidsson, Lars Bo Andersen. 
//   5th International Conference on Ambulatory Monitoring of Physical Activity and Movement (ICAMPAM) June 2017. 

// Sample frequency (30 Hz)
const int agSf = 30;

// Coefficients are derived from best fit (1st order Butterworth was tried with cut-offs 0.29 and 1.66, but not accurate enough)
// (Filtering coefficients are approximately 0.1 to 3.5Hz?)

// Filter order is 21
#define agN 21

// A-coefficients
const double agA[agN] =
{
    1, -4.16372602554363, 7.57115309014007, -7.98046902509111, 
    5.38501191026769, -2.46356271321257, 0.892381422717250, 0.0636099868633388, 
    -1.34810512714077, 2.47338133053049, -2.92571735841718, 2.92983230386598, 
    -2.78159062882719, 2.47767353571210, -1.68473849390463, 0.464828627239016, 
    0.465652889035618, -0.673118967429961, 0.416203225759379, -0.138323223919610, 
    0.0198517159761605
};

// B-coefficients
const double agB[agN] = 
{
    0.0491089825140489, -0.122841835307157, 0.143557884896153, -0.112693989220238, 
    0.0538037410952924, -0.0202302738400010, 0.00637784647673757, 0.0185125409235852, 
    -0.0381541058906574, 0.0487265187117185, -0.0525772146919336, 0.0478471380895460, 
    -0.0460148280299714, 0.0362833364868511, -0.0129768121654561, -0.00462621079355692, 
    0.0128353970741233, -0.00937622141658307, 0.00344850106651387, -0.000809720155277696, 
    -0.000196225290878896
};

// Filter gain (for B-coefficients) that gave the smallest error
const double agGain = 0.965;

// Decimated by 3: 2/3 of the samples are dropped and when sub-sampling to 10Hz (resample was tried, but it did not give accurate results)
const int agDownsample = 3;

// Sensor range is +/- 2.13 g
const double agPeakThreshold = 2.13;

// Dead-band removes anything < 0.068 g
const double agDeadband = 0.068;

// ADC resolution (1/) -- this resolution and gain gave the smallest error, but https://actigraph.desk.com/customer/en/portal/articles/2515580-what-are-counts- lists the original 8-bit sensor as 1/256 of a +/-2.13g/sec which produces: 4.26/256 = 0.016640625
const double agAdcResolution = 0.0164;

// Summation period is 10 samples, creating 1 second epochs
const int agIntegN = 10;

