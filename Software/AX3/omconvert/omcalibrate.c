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


#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "omdata.h"
#include "omconvert.h"
#include "omcalibrate.h"
#include "linearregression.h"


#define ALT_SD	// More stable - see Knuth TAOCP vol 2, 3rd edition, page 232


// (Internal) Find stationary points (using either a player or direct data)
static omcalibrate_stationary_points_t *OmCalibrateFindStationaryPoints(omcalibrate_config_t *config, omdata_t *data, om_convert_player_t *player)
{
	double sampleRate, startTime;
	double firstSampleTime = 0;
	int lastWindow = -1;
	int numSamples;
	int samplesInPreviousSegments = 0;
	omdata_segment_t *dataSegment = NULL;

	if (player != NULL)
	{
		sampleRate = player->sampleRate;
		startTime = player->arrangement->startTime;
		numSamples = player->numSamples;
	}
	else if (data != NULL)
	{
		omdata_stream_t *stream = &data->stream['a'];
		if (!stream->inUse) 
		{
			fprintf(stderr, "ERROR: Calibration failed as accelerometer stream not found.\n");
			return NULL;
		}
		dataSegment = stream->segmentFirst;

		sampleRate = dataSegment->sampleRate;
		startTime = dataSegment->startTime;

		// Determine total number of samples
		numSamples = 0;
		omdata_segment_t *seg;
		for (seg = dataSegment; seg != NULL; seg = seg->segmentNext)
		{
			numSamples += seg->numSamples;
		}
	}
	else
	{
		return NULL;
	}

	omcalibrate_stationary_points_t *stationaryPoints = (omcalibrate_stationary_points_t *)malloc(sizeof(omcalibrate_stationary_points_t));
	memset(stationaryPoints, 0, sizeof(omcalibrate_stationary_points_t));

	// For 'ignore repeated' option
	double initialMean[OMCALIBRATE_AXES] = { 0 };
	double initialTemp = 0.0;
	int consecutive = 0;

	// Trackers
#ifdef ALT_SD
	// See Knuth TAOCP vol 2, 3rd edition, page 232
	double oldM[OMCALIBRATE_AXES] = { 0 };
	double newM[OMCALIBRATE_AXES] = { 0 };
	double oldS[OMCALIBRATE_AXES] = { 0 };
	double newS[OMCALIBRATE_AXES] = { 0 };
	int n[OMCALIBRATE_AXES] = { 0 };
#else
	double axisSum[OMCALIBRATE_AXES] = { 0 };
	double axisSumSquared[OMCALIBRATE_AXES] = { 0 };
#endif
	double tempSum = 0;

	int sampleCount = 0;

	// Time interpolation for direct data
	int lastSectorIndex = -1;
	int nextTimestampSample = -1, lastTimestampSample = -1;
	double nextTimestampValue = 0, lastTimestampValue = 0;

	int sample;
	for (sample = 0; sample < numSamples; sample++)
	{
		int c;
		double values[OMCALIBRATE_AXES];
		double temp;
		double currentTime;
		bool windowFilled = false;

		if (player != NULL)
		{
			OmConvertPlayerSeek(player, sample);
			if (!player->valid) { sampleCount = 0; continue; }
			// Convert to units
			for (c = 0; c < OMCALIBRATE_AXES; c++)	// player.arrangement->numChannels
			{
				values[c] = player->scale[c] * player->values[c];
			}
			temp = player->temp;
			currentTime = ((double)sample / sampleRate) + startTime;

			// Have we filled a window?
			int numStationary = (int)(config->stationaryTime * sampleRate + 0.5);
			if (sampleCount >= numStationary) { windowFilled = true; }
		}
		else if (data != NULL)
		{
			int sampleWithinSegment = sample - samplesInPreviousSegments;

			// Check we have data
			if (dataSegment == NULL)
			{
				fprintf(stderr, "WARNING: Less data than was expected.\n");
				break;
			}

			int sectorWithinSegment = sampleWithinSegment / dataSegment->samplesPerSector;
			if (sectorWithinSegment >= dataSegment->sectorCount)
			{
				fprintf(stderr, "WARNING: Invalid sector within segment.\n");
				break;
			}

			// Advance to next segment?
			if (sampleWithinSegment >= dataSegment->numSamples)
			{
				dataSegment = dataSegment->segmentNext;
				samplesInPreviousSegments = sample;
				sampleWithinSegment = 0;

				// Update rates/times
				if (dataSegment != NULL)
				{
					sampleRate = dataSegment->sampleRate;
					startTime = dataSegment->startTime;
				}

				// Reset accumulator on segment change (break in sample stream)
				sampleCount = 0; 
				firstSampleTime = 0;	// Trigger time reset
				lastSectorIndex = -1;	// Force no time interpolation
				nextTimestampSample = -1;	// Force restart of time interpolation
				lastWindow = -1;
				continue;
			}

			// Sector index
			int sectorIndex = dataSegment->sectorIndex[sectorWithinSegment];

#if 1

			if (sectorIndex != lastSectorIndex)
			{
				lastSectorIndex = sectorIndex;

				// Get current sector time
				int sampleIndexOffset = 0;
				double newTimestampValue = OmDataTimestampForSector(data, sectorIndex, &sampleIndexOffset);
				int newTimestampSample = sample + sampleIndexOffset;

				// Replace 'next' timestamp if different
				if (newTimestampSample != nextTimestampSample)
				{
					if (nextTimestampSample < 0) 
					{
						nextTimestampSample = newTimestampSample;
						nextTimestampValue = newTimestampValue;
					}
					lastTimestampSample = nextTimestampSample;
					lastTimestampValue = nextTimestampValue;
					nextTimestampSample = newTimestampSample;
					nextTimestampValue = newTimestampValue;
				}
			}
			
			// If we only have one timestamp to estimate from
			int elapsedSamples = sample - lastTimestampSample + 1;
			if (nextTimestampSample <= lastTimestampSample)
			{
				currentTime = lastTimestampValue + (elapsedSamples / sampleRate);
			}
			else
			{
				currentTime = lastTimestampValue + (elapsedSamples * (nextTimestampValue - lastTimestampValue) / (nextTimestampSample - lastTimestampSample));
			}

#else
			// HACK: This is a bogus time as the sample rate is not fixed
			currentTime = ((double)sampleWithinSegment / sampleRate) + startTime;
#endif

#if 0
			// Debug out of time delta
			static double lastTime = -1;
			if (lastTime < 0) { lastTime = currentTime; }
			double deltaTime = currentTime - lastTime;
			printf("%f\n", deltaTime);
			lastTime = currentTime;
#endif

			// Get samples
			int16_t intvalues[OMCALIBRATE_AXES];
			OmDataGetValues(data, dataSegment, sampleWithinSegment, intvalues);

			// Get temperature
			temp = 0;
			if (dataSegment->offset == 30)
			{
				const unsigned char *p = (const unsigned char *)data->buffer + (OMDATA_SECTOR_SIZE * sectorIndex);
				int16_t inttemp = p[20] | ((int16_t)p[21] << 8);		// @20 WORD Temperature
				// Convert
				temp = ((int)inttemp * 150 - 20500) / 1000.0;
				//temp = (double)inttemp * 75 / 256.0 - 50;
			}

			// Scale values
			for (c = 0; c < OMCALIBRATE_AXES; c++)
			{
				values[c] = intvalues[c] * dataSegment->scaling;
			}

			// Check whether a window is filled
			if (firstSampleTime <= 0) { firstSampleTime = currentTime; lastWindow = -1; }
			int currentWindow = (int)((currentTime - firstSampleTime) / config->stationaryTime);
			if (lastWindow < 0) { lastWindow = currentWindow; }
			if (currentWindow != lastWindow)
			{
				windowFilled = true; 
				lastWindow = currentWindow;
			}

		}
		else 
		{
			break;
		}

		// Filled window
		if (windowFilled)
		{
			bool stationary = true;
			if (sampleCount <= 0) { sampleCount = 1; stationary = false; }

			// Check whether stationary
			for (c = 0; c < OMCALIBRATE_AXES; c++)
			{
#ifdef ALT_SD
				//double count = n[c];
				//double mean = (n[c] > 0) ? newM[c] : 0.0;
				double variance = (n[c] > 1) ? newS[c] / (n[c] - 1) : 0.0;
				double standardDeviation = sqrt(variance);
#else
				double mean = axisSum[c] / sampleCount;
				double squareOfMean = mean * mean;
				double averageOfSquares = (axisSumSquared[c] / sampleCount);
				double standardDeviation = sqrt(averageOfSquares - squareOfMean);

				//double standardDeviation = sqrt(sampleCount * axisSumSquared[c] - (axisSum[c] * axisSum[c])) / sampleCount;
#endif
				if (standardDeviation > config->stationaryMaxDeviation)
				{
					stationary = false;
				}
			}

			// Calculate mean values
			double time = currentTime - ((((double)sampleCount / 2)) / sampleRate);
			double mean[OMCALIBRATE_AXES];
			for (c = 0; c < OMCALIBRATE_AXES; c++)
			{
#ifdef ALT_SD
				mean[c] = (n[c] > 0) ? newM[c] : 0.0;
#else
				mean[c] = axisSum[c] / sampleCount;
#endif
			}

			// Check if this is a repeat
			if (stationary && config->stationaryRepeated)
			{
				if (consecutive == 0)
				{
					// Update 'initial' values
					for (c = 0; c < OMCALIBRATE_AXES; c++) { initialMean[c] = mean[c]; }
					initialTemp = temp;
					consecutive = 1;
				}
				else
				{
					// Compare 'initial' values
					double diff = 0;
					for (c = 0; c < OMCALIBRATE_AXES; c++) { diff += (initialMean[c] - mean[c]) * (initialMean[c] - mean[c]); }
					diff = sqrt(diff);
					if (diff < config->stationaryRepeatedAccel && fabs(initialTemp - temp) <= config->stationaryRepeatedTemp)
					{
						consecutive++;
						stationary = false;
					}
					else
					{
						consecutive = 0;
					}
				}
			}
			else
			{
				consecutive = 0;
			}

#if 0
			// Debug out of time delta
			static int windowNumber = 0;
			static double lastTime = -1;
			if (lastTime < 0) { lastTime = currentTime; }
			double deltaTime = currentTime - lastTime;
			if (stationary)
				printf("WINDOW: %d, from=%.3f, to=%.3f, period=%.3f\n", windowNumber++, lastTime, currentTime, deltaTime);
			lastTime = currentTime;
			windowNumber++;
#endif

			// Create new point
			if (stationary)
			{
				omcalibrate_point_t point;
				point.time = time;
				for (c = 0; c < OMCALIBRATE_AXES; c++)
				{
					point.mean[c] = mean[c];
				}
				point.actualTemperature = tempSum / sampleCount;
				//point.temperature = 0;

				// Check whether we have to grow the buffer
				if (stationaryPoints->numValues >= stationaryPoints->capacity)
				{
					stationaryPoints->capacity = (15 * stationaryPoints->capacity / 10) + 1;
					stationaryPoints->values = (omcalibrate_point_t *)realloc(stationaryPoints->values, stationaryPoints->capacity * sizeof(omcalibrate_point_t));
				}
				stationaryPoints->values[stationaryPoints->numValues] = point;
				stationaryPoints->numValues++;
			}

			// Trigger reset of accumulators
			sampleCount = 0;
		}




		// If the accumulators need to be reset...
		if (sampleCount == 0)
		{
			if (firstSampleTime <= 0) { firstSampleTime = currentTime; }

			// Clear accumulators
			for (c = 0; c < OMCALIBRATE_AXES; c++)
			{
#ifdef ALT_SD
				oldM[c] = newM[c] = oldS[c] = newS[c] = 0;
				n[c] = 0;
#else
				axisSum[c] = 0;
				axisSumSquared[c] = 0;
#endif
			}
			tempSum = 0;
		}
		sampleCount++;

		// Accumulate
		for (c = 0; c < OMCALIBRATE_AXES; c++)	// player.arrangement->numChannels
		{
			double x = values[c];
#ifdef ALT_SD
			n[c]++;
			if (n[c] == 1)
			{
				oldM[c] = newM[c] = x;
				oldS[c] = 0.0;
			}
			else
			{
				newM[c] = oldM[c] + (x - oldM[c]) / n[c];
				newS[c] = oldS[c] + (x - oldM[c]) * (x - newM[c]);
				oldM[c] = newM[c];
				oldS[c] = newS[c];
			}
#else
			axisSum[c] += x;
			axisSumSquared[c] += x * x;
#endif
		}
		tempSum += temp;


	}

	return stationaryPoints;
}


// Find stationary points (using an interpolating player)
omcalibrate_stationary_points_t *OmCalibrateFindStationaryPointsFromPlayer(omcalibrate_config_t *config, om_convert_player_t *player)
{
	return OmCalibrateFindStationaryPoints(config, NULL, player);
}

// Find stationary points (using direct data)
omcalibrate_stationary_points_t *OmCalibrateFindStationaryPointsFromData(omcalibrate_config_t *config, omdata_t *data)
{
	return OmCalibrateFindStationaryPoints(config, data, NULL);
}


// Free stationary points
void OmCalibrateFreeStationaryPoints(omcalibrate_stationary_points_t *stationaryPoints)
{
	if (stationaryPoints != NULL)
	{
		if (stationaryPoints->values != NULL)
		{
			free(stationaryPoints->values);
			stationaryPoints->values = NULL;
			stationaryPoints->capacity = 0;
			stationaryPoints->numValues = 0;
		}
		free(stationaryPoints);
	}
}


// Create a default (null) calibration
void OmCalibrateInit(omcalibrate_calibration_t *calibration)
{
	int i;
	memset(calibration, 0, sizeof(omcalibrate_calibration_t));
	for (i = 0; i < OMCALIBRATE_AXES; i++)
	{
		calibration->scale[i] = 1.0f;
		calibration->offset[i] = 0.0f;
		calibration->tempOffset[i] = 0.0f;
	}
	calibration->referenceTemperature = 0;
	//memset(calibration->referenceTemperatureQuantiles, 0, sizeof(calibration->referenceTemperatureQuantiles));
	calibration->numAxes = 0;
	calibration->errorCode = 0;
	return;
}


// Copy a calibration
void OmCalibrateCopy(omcalibrate_calibration_t *calibration, omcalibrate_calibration_t *source)
{
	if (source == NULL)
	{
		OmCalibrateInit(calibration);
	}
	else
	{
		memcpy(calibration, source, sizeof(omcalibrate_calibration_t));
	}
}


// Mean svm of stationary points with the calibration
double OmCalibrateMeanSvmError(omcalibrate_calibration_t *calibration, omcalibrate_stationary_points_t *stationaryPoints)
{
	// Evalulate mean svm
	int i;
	double sumSvm = 0;
	double sumSvmSquared = 0;
	for (i = 0; i < stationaryPoints->numValues; i++)
	{
		double temp = stationaryPoints->values[i].actualTemperature;
		double values[OMCALIBRATE_AXES];
		int c;
		for (c = 0; c < OMCALIBRATE_AXES; c++)
		{
			double v = stationaryPoints->values[i].mean[c];
			// Rescaling is:  v = (v + offset) * scale + (temp - referenceTemperature) * tempOffset
			v = (v + calibration->offset[c]) * calibration->scale[c] + (temp - calibration->referenceTemperature) * calibration->tempOffset[c];
			values[c] = v;
		}

		// SVM
		double svm = fabs(sqrt(values[0] * values[0] + values[1] * values[1] + values[2] * values[2]) - 1.0);
		sumSvm += svm;
		sumSvmSquared += svm * svm;
	}
	double meanSvm = 0;
	if (stationaryPoints->numValues > 0)
	{
		meanSvm = sumSvm / stationaryPoints->numValues;
	}
	return meanSvm;
}


// Print out calibration
void OmCalibrateDump(omcalibrate_calibration_t *calibration, omcalibrate_stationary_points_t *stationaryPoints, char finalResult)
{
	int c;


	if (finalResult)
	{
		// Nils 'CAL:' format
		printf("CAL:%.10f;%.10f;%.10f;%.10f;%.10f;%.10f;%.10f;%.10f;%.10f;%d;%d;\n",
			calibration->scale[0], calibration->scale[1], calibration->scale[2], 
			calibration->offset[0], calibration->offset[1], calibration->offset[2], 
			calibration->tempOffset[0], calibration->tempOffset[1], calibration->tempOffset[2], 
			calibration->errorCode, calibration->numAxes);
	}


	// JSON Format
	printf("calibration = {\n");

	printf("    \"scale\": [");
	for (c = 0; c < OMCALIBRATE_AXES; c++) { printf("%s%f", c > 0 ? ", " : "", calibration->scale[c]); }
	printf("],\n");

	printf("    \"offset\": [");
	for (c = 0; c < OMCALIBRATE_AXES; c++) { printf("%s%f", c > 0 ? ", " : "", calibration->offset[c]); }
	printf("],\n");

	printf("    \"tempOffset\": [");
	for (c = 0; c < OMCALIBRATE_AXES; c++) { printf("%s%f", c > 0 ? ", " : "", calibration->tempOffset[c]); }
	printf("],\n");

	printf("    \"referenceTemperature\": %f\n", calibration->referenceTemperature);

	//printf("    \"referenceTemperatureQuantiles\": [%f, %f]\n", calibration->referenceTemperatureQuantiles[0], calibration->referenceTemperatureQuantiles[1]);

	printf("};\n");

	double meanSvm = OmCalibrateMeanSvmError(calibration, stationaryPoints);
	printf("// meanSvm = %f; numPoints = %d; errorCode = %d; numAxes = %d; \n", meanSvm, stationaryPoints->numValues, calibration->errorCode, calibration->numAxes);

	return;
}


void OmCalibrateConfigInit(omcalibrate_config_t *calibrateConfig)
{
	memset(calibrateConfig, 0, sizeof(omcalibrate_config_t));
	calibrateConfig->stationaryTime = 10.0;				// 10.0
	calibrateConfig->stationaryMaxDeviation = 0.013;	// 0.013
	calibrateConfig->useTemp = 1;						// use temperature
	calibrateConfig->maxIter = 1000;					// 100
	calibrateConfig->convCrit = 0.000001;				// 0.0000000001; 0.001
	calibrateConfig->stationaryRepeated = 0;			// include
	calibrateConfig->stationaryRepeatedAccel = 0.032;	// Accel is 0.015625 g/bit
	calibrateConfig->stationaryRepeatedTemp = 0.16;		// Temp is 0.15^C/bit
	calibrateConfig->axisRange = 0.3;					// Required per-axis range in stationary points (0.3)
	calibrateConfig->maximumScaleDiff = 0.2;			// Maximum amount of per-axis scale (absolute difference from 1)
	calibrateConfig->maximumOffsetDiff = 0.41;			// !!! THIS IS TOO LARGE !!! Maximum amount of per-axis offset
	calibrateConfig->maximumTempOffsetDiff = 0.02;		// Maximum amount of per-axis temperature offset
	
	// Sx, Sy, Sz, Ox, Oy, Oz, Tx, Ty, Tz
	// 1.0371, 1.1288, 1.0641,  0.1052,  0.1282,  0.4094,  0.0084,  0.0159,  0.0172, // max
	// 0.9310, 0.8334, 0.9451, -0.1002, -0.1052, -0.1262, -0.0088, -0.0136, -0.0100, // min
}


// Find calibration
int OmCalibrateFindAutoCalibration(omcalibrate_config_t *config, omcalibrate_stationary_points_t *stationaryPoints, omcalibrate_calibration_t *calibration)
{
	int i;
	int c;

	// Reset calibration
	OmCalibrateInit(calibration);

	// Calcuate mean temperature
	if (config->useTemp)
	{
		double tempSum = 0;
		for (i = 0; i < stationaryPoints->numValues; i++)
		{
			tempSum += stationaryPoints->values[i].actualTemperature;
		}
		calibration->referenceTemperature = (stationaryPoints->numValues > 0) ? (tempSum / stationaryPoints->numValues) : 0.0;
	}
	else
	{
		calibration->referenceTemperature = 0.0;
	}

	// Check number of stationary points
	if (stationaryPoints->numValues < 4)
	{
		const char *msg = "CALIBRATE: Fewer than 4 stationary points for calibration estimation.\n";
		fprintf(stderr, "%s", msg);
		fprintf(stdout, "%s", msg);
		if (calibration->errorCode == 0) { calibration->errorCode = -1; }
	}
	else if (stationaryPoints->numValues < 10)
	{
		const char *msg = "CALIBRATE: Fewer than 10 stationary points for calibration estimation.\n";
		fprintf(stderr, "%s", msg);
		fprintf(stdout, "%s", msg);
	}

	// Measure per-axis min/max range
	double axisMin[OMCALIBRATE_AXES] = { 0 };
	double axisMax[OMCALIBRATE_AXES] = { 0 };
	for (c = 0; c < OMCALIBRATE_AXES; c++)
	{
		for (i = 0; i < stationaryPoints->numValues; i++)
		{
			double v = stationaryPoints->values[i].mean[c];
			if (i == 0 || v < axisMin[c]) { axisMin[c] = v; }
			if (i == 0 || v > axisMax[c]) { axisMax[c] = v; }
		}
	}

	// Check range on each axis
	calibration->numAxes = 0;
	for (c = 0; c < OMCALIBRATE_AXES; c++)
	{
		if (axisMin[c] <= -config->axisRange && axisMax[c] >= config->axisRange)
		{
			calibration->numAxes++;
		}
	}

	// Warn if not distributed on either side of each axis (+/- 0.3 g)
	if (calibration->numAxes <= 0) { const char *msg = "CALIBRATE: Unit sphere - no axis fits criterion.\n"; fprintf(stderr, "%s", msg); fprintf(stdout, "%s", msg); if (calibration->errorCode == 0) { calibration->errorCode = -2; } }
	else if (calibration->numAxes <= 1) { const char *msg = "CALIBRATE: Unit sphere - one axis fits criterion.\n"; fprintf(stderr, "%s", msg); fprintf(stdout, "%s", msg); if (calibration->errorCode == 0) { calibration->errorCode = -3; } }
	else if (calibration->numAxes <= 2) { const char *msg = "CALIBRATE: Unit sphere - two axes fulfill criterion.\n"; fprintf(stderr, "%s", msg); fprintf(stdout, "%s", msg); if (calibration->errorCode == 0) { calibration->errorCode = -4; } }

	// ---------- Auto-calibration ----------
	int numPoints = stationaryPoints->numValues;
	double *temp = (double *)malloc(sizeof(double) * numPoints);
	double *weights = (double *)malloc(sizeof(double) * numPoints);
	double *values[OMCALIBRATE_AXES];
	double *target[OMCALIBRATE_AXES];
	for (c = 0; c < OMCALIBRATE_AXES; c++)
	{
		values[c] = (double *)malloc(sizeof(double) * numPoints);
		target[c] = (double *)malloc(sizeof(double) * numPoints);
	}

	// Initialize weights to 1
	for (i = 0; i < numPoints; i++)
	{
		weights[i] = 1.0;
	}

	// Set the temperature relative to the reference temperature
	for (i = 0; i < numPoints; i++)
	{
		if (config->useTemp)
		{
			temp[i] = stationaryPoints->values[i].actualTemperature - calibration->referenceTemperature;
		}
		else
		{
			temp[i] = 0.0;
		}
	}


	// Main loop to estimate unit sphere
	int iter;
	for (iter = 0; iter < config->maxIter; iter++)
	{
		int c;

		// Scale input data with current parameters
		// model: (offset + D_in) * scale + T * tempOffset)
		//	D  = (repmat(offset,N,1) + D_in) .* repmat(scale,N,1) + repmat(temp,1,3) .* repmat(tempOffset,N,1);

		for (c = 0; c < OMCALIBRATE_AXES; c++)
		{
			for (i = 0; i < numPoints; i++)
			{
				values[c][i] = (calibration->offset[c] + stationaryPoints->values[i].mean[c]) * calibration->scale[c] + temp[i] * calibration->tempOffset[c];
			}
		}

		// targets: points on unit sphere
		// target = D ./ repmat(sqrt(sum(D.^2,2)),1,size(D,2));
		for (i = 0; i < numPoints; i++)
		{
			double sumSquares = 0;
			for (c = 0; c < OMCALIBRATE_AXES; c++)
			{
				sumSquares += values[c][i] * values[c][i];
			}
			double vectorLength = sqrt(sumSquares);
			if (vectorLength == 0) { vectorLength = 1.0; }
			for (c = 0; c < OMCALIBRATE_AXES; c++)
			{
				target[c][i] = values[c][i] / vectorLength;
			}
		}

		// Initialise vars for optimisation
		double gradient[OMCALIBRATE_AXES], off[OMCALIBRATE_AXES], tOff[OMCALIBRATE_AXES];

		// Do linear regression per input axis to estimate scale offset (and tempOffset)
		for (c = 0; c < OMCALIBRATE_AXES; c++)
		{
			gradient[c] = 1;
			off[c] = 0;
			tOff[c] = 0;

			double *coef;
			if (config->useTemp)
			{
				//	mdl = LinearModel.fit([D(:,j) temp], target(:,j), 'linear', 'Weights', weights);
				//coef = LinearModelFitTwoIndependent(numPoints, target[c], values[c], temp);
#ifdef ENABLE_GSL
				coef = LinearModelFitTwoIndependentWeighted(numPoints, target[c], values[c], temp, weights);
#else
				coef = LinearModelFitTwoIndependent(numPoints, target[c], values[c], temp);
				//coef = LinearModelFitTwoIndependentWeightedApproximately(numPoints, target[c], values[c], temp, weights);
#endif
			}
			else
			{
				//	mdl = LinearModel.fit([D(:,j)], target(:,j), 'linear', 'Weights', weights);
				coef = LinearModelFitOneIndependent(numPoints, target[c], values[c]);
			}

			off[c] = coef[0];			// offset		= intersect
			if (coef[1] != 0.0) { gradient[c] = coef[1]; }		// scale		= gradient
			if (config->useTemp)
				tOff[c] = coef[2];		// tempOffset	= last coeff.
			else
				tOff[c] = 0;
		}

		// Change current parameters

		// sc = scale;
		double sc[OMCALIBRATE_AXES];	// previous values (saved for convergence comparison)
		for (c = 0; c < OMCALIBRATE_AXES; c++) { sc[c] = calibration->scale[c]; }

		// adapt offset: offset = offset + off . / (scale.*gradient); 
		for (c = 0; c < OMCALIBRATE_AXES; c++) 
		{ 
			double div = calibration->scale[c] * gradient[c];
			if (div == 0.0) { div = 1; }
			calibration->offset[c] = calibration->offset[c] + off[c] / div;
		}

		// adapt scaling: scale = scale.*gradient;
		for (c = 0; c < OMCALIBRATE_AXES; c++) 
		{ 
			calibration->scale[c] = calibration->scale[c] * gradient[c]; 
		} 

		// Apply temperature offset 
		// if p.useTemp
		if (config->useTemp)
		{
			// tempOffset = tempOffset .* gradient + tOff; 
			for (c = 0; c < OMCALIBRATE_AXES; c++) { calibration->tempOffset[c] = calibration->tempOffset[c] * gradient[c] + tOff[c]; }
		}

		// Update weightings for linear regression (ignores outliers, overall limited to a maximum of 100)
		// weights = min([1 ./ sqrt(sum((D-target).^2,2)), repmat(100,N,1)],[],2);
		for (i = 0; i < numPoints; i++)
		{
			double rowSum = 0;
			for (c = 0; c < OMCALIBRATE_AXES; c++)
			{
				double v = values[c][i] - target[c][i];		// (D-target)
				rowSum += (v * v);							// sum((D-target).^2)
			}
			double vlen = sqrt(rowSum);
			double vv = vlen != 0 ? 1 / vlen : 0;
			if (vv > 100) { vv = 100; }
			weights[i] = vv;
		}

		// no more scaling change -> assume it has converged
		double cE = 0;
		for (c = 0; c < OMCALIBRATE_AXES; c++) { cE += fabs(calibration->scale[c] - sc[c]); }
		bool converged = cE < config->convCrit;

		// RMS error to unit sphere
		// E = sqrt(mean(sum((D - target). ^ 2, 2)));
		double colSum = 0;
		for (i = 0; i < numPoints; i++)
		{
			double rowSum = 0;
			for (c = 0; c < OMCALIBRATE_AXES; c++)
			{
				double v = values[c][i] - target[c][i];		// (D-target)
				rowSum += (v * v);							// sum((D-target).^2)
			}
			colSum += rowSum;
		}
		double meanSum = numPoints > 0 ? colSum / numPoints : 0;
		double E = sqrt(meanSum);

		// Debug progress
		if (iter == 0 || iter >= config->maxIter - 1 || converged)
		{
			fprintf(stderr, "CALIBRATE: Iteration %d, error: %.4f, convergence: %.6f %s\n", iter + 1, E, cE, converged ? "CONVERGED" : "");
		}

// Check for convergence
if (converged) { break; }

		// Warning if no convergence
		if (!converged && iter + 1 >= config->maxIter)
		{
			const char *msg = "WARNING: Maximum number of iterations reached without convergence.\n";
			fprintf(stderr, "%s", msg);
			fprintf(stdout, "%s", msg);
		}

	}

	// Free resources
	free(temp);
	free(weights);
	for (c = 0; c < OMCALIBRATE_AXES; c++)
	{
		free(values[c]);
		free(target[c]);
	}

	// Sanity check the calibration
	char axisFailed = 0;
	for (c = 0; c < OMCALIBRATE_AXES; c++)
	{
		double scaleDiff = fabs(calibration->scale[c] - 1.0);
		double offsetDiff = calibration->offset[c];
		double tempOffsetDiff = calibration->tempOffset[c];
		if ((config->maximumScaleDiff > 0 && scaleDiff > config->maximumScaleDiff) || (config->maximumOffsetDiff > 0 && offsetDiff > config->maximumOffsetDiff) || (config->maximumTempOffsetDiff > 0 && tempOffsetDiff > config->maximumTempOffsetDiff))
		{
			axisFailed++;
		}
	}
	if (axisFailed > 0)
	{
		char msg[128];
		sprintf(msg, "CALIBRATE: Range check on %d axes unmet.\n", axisFailed);
		fprintf(stderr, "%s", msg);
		fprintf(stdout, "%s", msg);
		if (calibration->errorCode == 0) calibration->errorCode = -5;
	}

	return calibration->errorCode;
}



