/*
* Copyright (c) 2015, Newcastle University, UK.
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

// Open Movement - ESS Sleep Calculation
// Dan Jackson, 2015


/*
ESS Sleep calculation

Implementation of the "Estimation of Stationary Sleep-segments" approach from:

  Marko Borazio, Eugen Berlin, Nagihan Kucukyildiz, Philipp M. Scholl and Kristof Van Laerhoven:
  "Towards a Benchmark for Wearable Sleep Analysis with Inertial Wrist-worn Sensing Units", 
  ICHI 2014, Verona, Italy, IEEE Press, 2014.
  
Algorithm:
  
* Using just the Z axis which, from the hardware specification, is "out" of the wrist.
* Standard deviation over each 1 second epoch window:  STD = sqrt(1/N * SUM( (z[i] - MEAN(z))^2 ))
* A segment is extended by one 1 second epoch where:  STD < delta,  delta = 6.  The paper doesn't specify units, but from the data, and the way it is used by the code, this appears to be in untis of 1/32 G
* Segments must be of a minimum length to count: 600 epochs => 600 seconds.

*/


#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "calc-sleep.h"


// Sleep parameters
#define AXIS 2						// Z-axis
#define EPOCH 1						// 1 second epoch
#define DELTA (6 / 32.0)			// 6 (apparently in units of 1/32 G)
#define MIN_LENGTH (600 / EPOCH)	// Minimum length in epochs (600 * 1-second epochs)


#define MAX_TIME_STRING 26
static const char *TimeString(char timeCsv, double t, char *buff)
{
	static char staticBuffer[MAX_TIME_STRING] = { 0 };	// 2000-01-01 20:00:00.000|
	if (buff == NULL) { buff = staticBuffer; }
	time_t tn = (time_t)t;
	struct tm *tmn = gmtime(&tn);
	float sec = tmn->tm_sec + (float)(t - (time_t)t);
	if (timeCsv == 0)
	{
		sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d" /*".%03d"*/, 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec /*, (int)((sec - (int)sec) * 1000) */);
	}
	else if (timeCsv == 42) { sprintf(buff, "%d", (int)t-1); }		// // DELME: Special mode for algorithm comparison
	else
	{
		sprintf(buff, "%.3f", t);
	}
	return buff;
}


// Load data
char SleepInit(sleep_status_t *status, sleep_configuration_t *configuration)
{
	memset(status, 0, sizeof(sleep_status_t));
	status->configuration = configuration;

	if (configuration->summaryEpochs <= 0) { configuration->summaryEpochs = 1; }

	if (status->configuration->sampleRate <= 0.0)
	{
		fprintf(stderr, "ERROR: Sleep sample rate not specified.\n");
		return 0;
	}

	status->file = NULL;
	if (configuration->filename != NULL && strlen(configuration->filename) > 0)
	{
		status->file = fopen(configuration->filename, "wt");
		if (status->file == NULL)
		{
			fprintf(stderr, "ERROR: Sleep file not opened.\n");
		}
	}

	// .CSV header
	if (status->file && configuration->headerCsv)
	{
		fprintf(status->file, "Start,End,Duration(s)\n");
	}

	// Reset
	status->sample = 0;

	status->intervalSample = 0;
	status->axisSum = 0;
	status->axisSumSquared = 0;

	status->sleepStartTime = 0;
	status->epochsSleeping = 0;

	return (status->file != NULL) ? 1 : 0;
}


void SleepEndSegment(sleep_status_t *status)
{
	// Long enough to count?
	if (status->epochsSleeping >= MIN_LENGTH)
	{
		if (status->file != NULL)
		{
			fprintf(status->file, "%s", TimeString(status->configuration->timeCsv, status->sleepStartTime, NULL));
			fprintf(status->file, ",%s", TimeString(status->configuration->timeCsv, status->sleepStartTime + status->epochsSleeping, NULL));
//if (status->configuration->timeCsv != 42)		// DELME: Special mode for algorithm comparison
			fprintf(status->file, ",%u", status->epochsSleeping);
			fprintf(status->file, "\n");
		}
	}

	// End segment
	status->epochsSleeping = 0;
}


// Processes the specified value
bool SleepAddValue(sleep_status_t *status, double* accel, double temp, bool valid)
{
	// Axis value
	double v = accel[AXIS];		// Just Z-axis

	// Only add valid samples
	if (valid)
	{
		status->axisSum += v;
		status->axisSumSquared += v * v;
		status->intervalSample++;
	}

	// Next sample
	status->sample++;

	// Epoch
	int interval = ((int)(status->configuration->sampleRate * EPOCH + 0.5));
	if (status->sample > 0 && status->sample % interval == 0)
	{
		// Calculate standard deviation on the axis
		double variance = 0;
		if (status->intervalSample > 1)
		{
			variance = (status->axisSumSquared - ((status->axisSum * status->axisSum) / status->intervalSample)) / (status->intervalSample - 1);
		}
		double standardDeviation = sqrt(variance);

//printf("#%d, t=%f, VAR=%f, STD=%f, delta=%f\n", status->sample, status->configuration->startTime + (status->sample / status->configuration->sampleRate), variance, standardDeviation, (double)DELTA);

		// Add to current segment
		if (standardDeviation < DELTA)
		{
			// New segment?
			if (status->epochsSleeping <= 0)
			{
				status->sleepStartTime = status->configuration->startTime + (status->sample / status->configuration->sampleRate);
			}

			status->epochsSleeping++;
		}
		else
		{
			SleepEndSegment(status);
		}

		// Start new epoch
		status->intervalSample = 0;
		status->axisSum = 0;
		status->axisSumSquared = 0;
	}

	return true;
}

// Free data resources
int SleepClose(sleep_status_t *status)
{
	if (status->intervalSample > 0)
	{
		SleepEndSegment(status);
	}
	if (status->file != NULL)
	{ 
		fclose(status->file);
	}
	return 0;
}
