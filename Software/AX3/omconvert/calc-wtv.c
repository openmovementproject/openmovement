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

// Open Movement Wear-Time Validation
// Dan Jackson, 2014


/*
Based on the method by van Hees et al in PLos ONE 2011 6(7), 
"Estimation of Daily Energy Expenditure in Pregnant and Non-Pregnant Women Using a Wrist-Worn Tri-Axial Accelerometer".
Accelerometer non-wear time is estimated from the standard deviation and range of each accelerometer axis, 
calculated for consecutive blocks of 30 minutes.
A block was classified as non-wear time if the standard deviation was less than 3.0 mg 
(1 mg = 0.00981 m*s-2) for at least two out of the three axes,
or if the value range, for at least two out of three axes, was less than 50 mg.
*/



#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "butter.h"
#include "calc-wtv.h"


#define AXES 3


// Load data
char WtvInit(wtv_status_t *status, wtv_configuration_t *configuration)
{
	memset(status, 0, sizeof(wtv_status_t));
	status->configuration = configuration;

	if (status->configuration->wtvStdCutoff <= 0.0)
	{
		status->configuration->wtvStdCutoff = 0.003;		// Non-wear if std-dev < 3.0 mg (for at least 2 out of the 3 axes)
	}
	if (status->configuration->wtvRangeCutoff <= 0.0)
	{
		status->configuration->wtvRangeCutoff = 0.050;		// or, non-wear if range < 50 mg (for at least 2 out of the 3 axes)
	}

	if (status->configuration->sampleRate <= 0.0)
	{
		fprintf(stderr, "ERROR: WTV sample rate not specified.\n");
		return 0;
	}

	status->file = NULL;
	if (configuration->filename != NULL && strlen(configuration->filename) > 0)
	{
		status->file = fopen(configuration->filename, "wt");
		if (status->file == NULL)
		{
			fprintf(stderr, "ERROR: WTV file not opened.\n");
		}
	}

	// .CSV header
	if (status->file && configuration->headerCsv)
	{
		fprintf(status->file, "Time,Wear time (30 mins)\n");
	}

	// Reset counters
	int c;
	for (c = 0; c < AXES; c++) 
	{ 
		status->axisSum[c] = 0; 
		status->axisSumSquared[c] = 0; 
		status->axisMax[c] = 0;
		status->axisMin[c] = 0;
	}
	status->sample = 0;

	return (status->file != NULL) ? 1 : 0;
}


void WtvPrint(wtv_status_t *status)
{
	if (status->file != NULL)
	{
		char timestring[24];	// 2000-01-01 12:00:00.000\0

		time_t tn = (time_t)status->epochStartTime;
		struct tm *tmn = gmtime(&tn);
		float sec = tmn->tm_sec + (float)(status->epochStartTime - (time_t)status->epochStartTime);
		sprintf(timestring, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec);	// (int)((sec - (int)sec) * 1000)
		//sprintf(timestring, "==> %04d-%02d-%02d %02d:%02d:%02d.%03d", status->totalWorn);

		fprintf(status->file, "%s,%d\n", timestring, status->totalWorn);	// Number of half-hour epochs that were worn
	}
}


// Processes the specified value
bool WtvAddValue(wtv_status_t *status, double* accel, double temp, bool valid)
{
	int c;

	if (status->epochStartTime == 0)
	{
		status->epochStartTime = status->configuration->startTime + (status->sample / status->configuration->sampleRate);
		for (c = 0; c < AXES; c++)
		{
			status->axisSum[c] = 0;
			status->axisSumSquared[c] = 0;
			status->axisMax[c] = accel[c];
			status->axisMin[c] = accel[c];
		}
	}

	/*
	// SVM
	double sumSquared = 0;
	for (c = 0; c < AXES; c++)
	{
	double v = accel[c];
	sumSquared += v * v;
	}
	double svm = sqrt(sumSquared) - 1;
	//if (svm < 0) { svm = 0.0; }		// Clamp mode
	svm = fabs(svm);			// Standard abs() mode
	status->sumSvm += svm;
	*/

	// Axis StdDev & range
	if (valid)
	{
		for (c = 0; c < AXES; c++)
		{
			double v = accel[c];
			status->axisSum[c] += v;
			status->axisSumSquared[c] += v * v;
			if (status->intervalSample == 0 || accel[c] < status->axisMin[c]) { status->axisMin[c] = accel[c]; }
			if (status->intervalSample == 0 || accel[c] > status->axisMax[c]) { status->axisMax[c] = accel[c]; }
		}
		status->intervalSample++;
	}

	status->sample++;

	// Report WTV epoch
	int epoch = 30 * 60;	// 30 minutes
	int interval = ((int)(status->configuration->sampleRate * epoch + 0.5));
	if (status->sample > 0 && status->sample % interval == 0)
	{
		//double meanSvm = status->sumSvm / interval;

		// Per-axis StdDev and range
		double stddev[AXES];
		double range[AXES];
		for (c = 0; c < AXES; c++)
		{
			double mean = status->intervalSample == 0 ? 0 : status->axisSum[c] / status->intervalSample;
			double squareOfMean = mean * mean;
			double averageOfSquares = status->intervalSample == 0 ? 0 : (status->axisSumSquared[c] / status->intervalSample);
			double standardDeviation = sqrt(averageOfSquares - squareOfMean);
			stddev[c] = standardDeviation;
			range[c] = status->axisMax[c] - status->axisMin[c];
		}

		// Determine if the StdDev or range is low on each axis
		int stdDevLow = 0, rangeLow = 0;
		for (c = 0; c < AXES; c++)
		{
			if (stddev[c] < status->configuration->wtvStdCutoff) { stdDevLow++; }	// Non-wear if std-dev < 3.0 mg (for at least 2 out of the 3 axes)
			if (range[c] < status->configuration->wtvRangeCutoff) { rangeLow++; }	// or, non-wear if range < 50 mg (for at least 2 out of the 3 axes)
		}

		// Determine if a non-wear chunk
		// - non-wear if std-dev < 3.0 mg (for at least 2 out of the 3 axes), or if range < 50 mg (for at least 2 out of the 3 axes)
		int worn;
		if (stdDevLow >= 2 || rangeLow >= 2) { worn = 0; }
		else { worn = 1; }

		status->totalWorn += worn;
		status->numWindows++;

		if (status->numWindows >= status->configuration->halfHourEpochs)
		{
			WtvPrint(status);
			status->totalWorn = 0;
			status->numWindows = 0;
		}

		// Reset counters
		status->epochStartTime = 0;
	}

	return true;
}

// Free data resources
int WtvClose(wtv_status_t *status)
{
	//WtvPrint(status);		// Don't print wear-time for last, incomplete block (it's not even been calculated here anyway)

	if (status->file != NULL)
	{ 
		fclose(status->file);
	}
	return 0;
}
