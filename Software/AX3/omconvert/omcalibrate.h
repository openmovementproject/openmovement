/*
* Copyright (c) 2014, Newcastle University, UK.
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

// Open Movement Calibration
// Dan Jackson, Nils Hammerla, 2014
// Based on a Matlab implementation by Nils Hammerla 2014 <nils.hammerla@ncl.ac.uk>
// which was inspired by the GGIR package (http://cran.r-project.org/web/packages/GGIR/) by Vincent T van Hees, Zhou Fang, Jing Hua Zhao.


#ifndef OMCALIBRATE_H
#define OMCALIBRATE_H


#include "omconvert.h"


#define OMCALIBRATE_AXES 3


// Auto-calibration configuration
typedef struct
{
	char useTemp;							// Whether to use the temperature for estimation (1)
	int maxIter;							// Maximum number of iterations for optimization (100)
	double convCrit;						// Convergence criterion (derivative of scale-factors in subsequent iterations) (0.001)
	double stationaryTime;					// Stationary period time in seconds (10)
	double stationaryMaxDeviation;			// Maximum standard deviation per channel for stationary periods (0.013)
	char stationaryRepeated;				// 0=include repeated measurements, 1=ignore repeated measurements, (future: 2=combine repeated measurements?)
	double stationaryRepeatedAccel;			// Acceleration vector length critera for "repeated" stationary measurements
	double stationaryRepeatedTemp;			// Temperature difference critera for "repeated" stationary measurements
	double axisRange;						// Required per-axis range in stationary points (0.3)
	double maximumScaleDiff;				// Maximum amount of per-axis scale (absolute difference from 1)
	double maximumOffsetDiff;				// Maximum amount of per-axis offset
	double maximumTempOffsetDiff;			// Maximum amount of per-axis temperature offset
} omcalibrate_config_t;



// A stationary point
typedef struct
{
	double time;					// Time (at end)
	double mean[OMCALIBRATE_AXES];	// Mean value on each axis
	double actualTemperature;		// As recorded in the data
} omcalibrate_point_t;

// Set of stationary points
typedef struct
{
	omcalibrate_point_t *values;
	int numValues;
	int capacity;
} omcalibrate_stationary_points_t;

// Find stationary points (using an interpolating player)
omcalibrate_stationary_points_t *OmCalibrateFindStationaryPointsFromPlayer(omcalibrate_config_t *config, om_convert_player_t *player);

// Find stationary points (using direct data)
omcalibrate_stationary_points_t *OmCalibrateFindStationaryPointsFromData(omcalibrate_config_t *config, omdata_t *data);

// Free stationary points
void OmCalibrateFreeStationaryPoints(omcalibrate_stationary_points_t *stationaryPoints);


// Calibration
// NOTE: tempOffset (from original calibration code) should probably be called tempScale?
// Rescaling is:  v = (v + offset) * scale + (temp - referenceTemperature) * tempOffset
typedef struct omcalibrate_calibration_tag
{
	double scale[OMCALIBRATE_AXES];
	double offset[OMCALIBRATE_AXES];
	double tempOffset[OMCALIBRATE_AXES];
	double referenceTemperature;				// Mean temperature
	//double referenceTemperatureQuantiles[2];	// 10th & 90th percentile of (temperature - meanTemperature)
	int numAxes;								// Num axes
	int errorCode;								// Error code
} omcalibrate_calibration_t;

// Create a default (null) calibration
void OmCalibrateInit(omcalibrate_calibration_t *calibration);

// Copy a calibration
void OmCalibrateCopy(omcalibrate_calibration_t *calibration, omcalibrate_calibration_t *source);

// Create a default calibration configuration
void OmCalibrateConfigInit(omcalibrate_config_t *calibrateConfig);

// Calculate the mean SVM of the stationary points with the specified calibration
double OmCalibrateMeanSvmError(omcalibrate_calibration_t *calibration, omcalibrate_stationary_points_t *stationaryPoints);

// Print out calibration
void OmCalibrateDump(omcalibrate_calibration_t *calibration, omcalibrate_stationary_points_t *stationaryPoints, char finalResult);

// Find calibration
int OmCalibrateFindAutoCalibration(omcalibrate_config_t *config, omcalibrate_stationary_points_t *stationaryPoints, omcalibrate_calibration_t *calibration);



#endif

