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

// Step Count
// Dan Jackson

/*
# Wrist-Worn Step Count Algorithm

## Overview

A step count algorithm for a wrist-worn triaxial accelerometer based on:

> "Design and Implementation of Practical Step Detection Algorithm for Wrist-Worn Devices"
> by Yunhoon Cho, Hyuntae Cho, and Chong-Min Kyung.
> IEEE Sensors Journal, vol. 16, no. 21, pp. 7720-7730, Nov.1, 2016.
> doi: 10.1109/JSEN.2016.2603163
> Contact: "Yunhoon Cho" <yhcho@vslab.kaist.ac.kr>; "Hyuntae Cho" <phd.marine@kaist.ac.kr>; "Chong-Min Kyung" <kyung@kaist.ac.kr>; 


## Summary of algorithm

### Process 1: Data Preprocessing - Norm. calculation, DC blocking

Note: Paper does not specify precision -- assuming doubles (rather than, e.g., an embedded system's integers).

1. Euclidean norm signal (paper's example input was clamped +/-2g, units not specified but 'g' shown in graphs):
    U[n] = sqrt(Ax[n]^2 + Ay[n]^2 + Az[n]^2)
2. Remove DC determined by a box filter of the last 1 second (paper says unspecified rate and 20 samples, but assume this is at the 20 Hz sample collection rate mentioned later).
		X[n] = U[n] - (SUM(U[n]...U[n-19]) / 20)

### Process 2: Data Filtering - Simple low-pass filtering

3. Low-pass filter with a cutoff frequency of 20Hz.  The paper does not specify the data rate, have to assume this is at the 20 Hz sample data collection rate mentioned later (although it would need at least 40Hz input to perform the claimed filtering?):
   Y[n] = (X[n] + 2*X[n-1] + 3*X[x-2] + 4*X[n-3] + 3*X[n-4] + 2*X[n-5] + X[n-6]) / 16

### Process 3: Peak Detection - Sign-of-slope with average threshold

4. A moving average box filter is taken of the last 0.5 seconds (paper only specifies 10 samples, but assume this is at the 20 Hz sample collection rate mentioned later).
 		A[n] = (SUM(Y[n]...Y[n-9]) / 10)
5. The sign-of-slope is taken to identify candidates for local maximum (1,-1) and minimum (-1,1):
    S[n] = sgn(Y[n] - Y[n-1])
		if (S[n] < S[n-1]) P[n] = 1;
		else if (S[n] > S[n-1]) P[n] = -1;
		else P[n] = 0;
6. Retain a local maximum candidate only if it comes after a true local minimum;
   retain a local minimum candidate only if it comes after a true local maximum.
	   if (P[n] == P[n-1]) P[n] = 0;
7. Retain a local maximum candidate only if it is greater than the average threshold;
   retain a local minimum candidate only if it is less than the average threshold.
	   if (P[n] > 0 && Y[n] <= A[n]) P[n] = 0;
	   if (P[n] < 0 && Y[n] >= A[n]) P[n] = 0;
8. Retained local maximum/minimum candidates become the true local maximum/minimum.
9. The number of steps is determined by the counting the number of pairs of the true local maximum and minimum.
	   if (P[n] != 0) halfSteps++;
*/


#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

//#define STEP_DEBUG_DUMP	// dump per-sample trace values

#if 1
// Additional filtering on peak/trough-to-peak/trough values
#define STEP_CHECK_INTERPEAK_TIME 1500
#define STEP_CHECK_INTERPEAK_RANGE 0.08 //0.08
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "calc-step.h"


// Load data
char StepInit(step_status_t *status, step_configuration_t *configuration)
{
	memset(status, 0, sizeof(step_status_t));
	status->configuration = configuration;

	// Check sample rate
	if (status->configuration->sampleRate <= 0.0)
	{
		fprintf(stderr, "ERROR: Step sample rate not specified.\n");
		return 0;
	}
	status->decimateAccumulator = status->configuration->sampleRate - STEP_RATE;
	if (status->configuration->sampleRate != STEP_RATE)
	{
		fprintf(stderr, "WARNING: Step sample rate should be %d Hz (use option: -resample %d) - will decimate input sample rate %d:%.2f.\n", STEP_RATE, STEP_RATE, STEP_RATE, status->configuration->sampleRate);
	}

	status->file = NULL;
	if (configuration->filename != NULL && strlen(configuration->filename) > 0)
	{
		status->file = fopen(configuration->filename, "wt");
		if (status->file == NULL)
		{
			fprintf(stderr, "ERROR: Step file not opened.\n");
		}
	}

	// .CSV header
	if (status->file && configuration->headerCsv)
	{
		fprintf(status->file, "Time,Steps,Cumulative");
		fprintf(status->file, "\n");
	}

	status->sample = 0;
	status->halfStepsInEpoch = 0;
	status->epochStartTime = 0;		// First sample will start the next epoch
	status->lastEpoch = 0;				// Begin in the first epoch

	return 1; // (status->file != NULL) ? 1 : 0;
}


static void StepPrint(step_status_t *status)
{
	if (status->file != NULL)
	{
		char timestring[24];	// 2000-01-01 12:00:00.000\0
		time_t tn = (time_t)status->epochStartTime;
		struct tm *tmn = gmtime(&tn);
		float sec = tmn->tm_sec + (float)(status->epochStartTime - (time_t)status->epochStartTime);
		int reportedSteps = (int)(status->halfStepsInEpoch / 2);
		status->cumulativeStepsReported += reportedSteps;
		status->halfStepsInEpoch = status->halfStepsInEpoch % 2;		// Carry up to one half step (full steps are reported)
		sprintf(timestring, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec);	// (int)((sec - (int)sec) * 1000)
		fprintf(status->file, "%s,%d,%d\n", timestring, reportedSteps, status->cumulativeStepsReported);
		status->written++;
#ifdef _DEBUG
		fflush(status->file);		// !!!!???? HACK: Only for debugging, remove
#endif
	}
}


// Processes the specified value
bool StepAddValueInternal(step_status_t *status, double *value, double temp, bool valid)
{
	const int effectiveRate = STEP_RATE; // status->configuration->sampleRate
	int i;

	// Report from the last epoch
	unsigned int currentEpoch = (unsigned int)(status->sample / status->configuration->secondEpochs / effectiveRate);
	if (currentEpoch != status->lastEpoch)
	{
		status->lastEpoch = currentEpoch;
		StepPrint(status);
		status->epochStartTime = 0;	// Next sample will begin a new epoch
	}

	// New epoch? Reset epoch state
	if (status->epochStartTime == 0)
	{
		status->epochStartTime = status->configuration->startTime + (status->sample / effectiveRate);
	}

	// Step 1. Euclidean norm signal (paper's example input was clamped +/-2g, units not specified but 'g' shown in graphs):
	//    U[n] = sqrt(Ax[n]^2 + Ay[n]^2 + Az[n]^2)
	#if (STEP_AXES != 3)
		#error "STEP_AXES != 3"
	#endif
	double ax = value[0]; if (ax < -STEP_G_RANGE) { ax = -STEP_G_RANGE; } else if (ax > STEP_G_RANGE) { ax = STEP_G_RANGE; }
	double ay = value[1]; if (ay < -STEP_G_RANGE) { ay = -STEP_G_RANGE; } else if (ay > STEP_G_RANGE) { ay = STEP_G_RANGE; }
	double az = value[2]; if (az < -STEP_G_RANGE) { az = -STEP_G_RANGE; } else if (az > STEP_G_RANGE) { az = STEP_G_RANGE; }
	if (STEP_PRESCALE != 1)
	{
		ax *= STEP_PRESCALE; 
		ay *= STEP_PRESCALE; 
		az *= STEP_PRESCALE; 
	}
	double u = sqrt(ax * ax + ay * ay + az * az);

	// Step 2. Remove DC determined by a box filter of the last 1 second (paper says unspecified rate and 20 samples, but assume this is at the 20 Hz sample collection rate mentioned later).
	// 		X[n] = U[n] - (SUM(U[n]...U[n-19]) / 20)
	// Place in to box filter
	status->dcFilter[status->sample % STEP_DC_FILTER_SIZE] = u;
	double meanU = 0.0;
	for (i = 0; i < STEP_DC_FILTER_SIZE; i++) { meanU += status->dcFilter[i]; }
	meanU /= STEP_DC_FILTER_SIZE;
	double x = u - meanU;

	// Step 3. Low-pass filter with a cutoff frequency of 20Hz.  The paper does not specify the data rate, have to assume this is at the 20 Hz sample data collection rate mentioned later (although it would need at least 40Hz input to perform the claimed filtering?):
	//    Y[n] = (X[n] + 2*X[n-1] + 3*X[x-2] + 4*X[n-3] + 3*X[n-4] + 2*X[n-5] + X[n-6]) / 16
	#if STEP_LP_FILTER_SIZE < 7
	#error STEP_LP_FILTER_SIZE must be 7
	#endif
	memmove(status->lpFilter + 1, status->lpFilter + 0, sizeof(double) * (STEP_LP_FILTER_SIZE - 1));
	status->lpFilter[0] = x;
	double y = (status->lpFilter[0] + 2*status->lpFilter[1] + 3*status->lpFilter[2] + 4*status->lpFilter[3]
							+ 3*status->lpFilter[4] + 2*status->lpFilter[5] + status->lpFilter[6]) / 16;

	// Step 4. A moving average box filter is taken of the last 0.5 seconds (paper only specifies 10 samples, but assume this is at the 20 Hz sample collection rate mentioned later).
	// 		A[n] = (SUM(Y[n]...Y[n-9]) / 10)
	status->average[status->sample % STEP_AVERAGE_SIZE] = y;
	double a = 0.0;
	for (i = 0; i < STEP_AVERAGE_SIZE; i++) { a += status->average[i]; }
	a /= STEP_AVERAGE_SIZE;

	// Step 5. The sign-of-slope is taken to identify candidates for local maximum (1,-1) and minimum (-1,1):
	//     S[n] = sgn(Y[n] - Y[n-1])
	//     if (S[n] < S[n-1]) P[n] = 1;
	//     else if (S[n] > S[n-1]) P[n] = -1;
	//     else P[n] = 0;
	int s = (y >= status->lastY) ? 1 : -1;
	status->lastY = y;
	int peak = (s < status->lastS) ? 1 : ((s > status->lastS) ? -1 : 0);
	status->lastS = s;
	int truePeak = peak;

	// Step 6. Retain a local maximum candidate only if it comes after a true local minimum;
	//    retain a local minimum candidate only if it comes after a true local maximum.
	//    if (P[n] == P[n-1]) P[n] = 0;
	if (truePeak != 0 && truePeak == status->lastPeak) {
		// Min/max candidate: ignore if same as last true peak
		truePeak = 0;
	}

	// Step 7. Retain a local maximum candidate only if it is greater than the average threshold;
	//    retain a local minimum candidate only if it is less than the average threshold.
	//    if (P[n] > 0 && Y[n] <= A[n]) P[n] = 0;
	//    if (P[n] < 0 && Y[n] >= A[n]) P[n] = 0;
	if (truePeak > 0) {
		// Maximum candidate: ignore if below average
		if (y <= a) {
			truePeak = 0;
		}
	} else if (truePeak < 0) {
		// Minimum candidate: ignore if above average
		if (y >= a) {
			truePeak = 0;
		}
	}

	if (truePeak != 0) {
		char hasPrevious = status->lastPeak != 0;

#ifdef STEP_CHECK_INTERPEAK_TIME
		if (hasPrevious && STEP_CHECK_INTERPEAK_TIME > 0)
		{
			// If too long has elapsed since last peak, forget it
			int elapsed = (int)(1000 * (status->sample - status->lastPeakSample) / effectiveRate);
			if (elapsed >= STEP_CHECK_INTERPEAK_TIME)
			{
				hasPrevious = 0;
			}
		}
#endif

#ifdef STEP_CHECK_INTERPEAK_RANGE
		if (hasPrevious && STEP_CHECK_INTERPEAK_RANGE > 0)
		{
			if (fabs(y - status->lastPeakValue) < STEP_CHECK_INTERPEAK_RANGE)
			{
				hasPrevious = 0;
			}
		}
#endif

		// Step 8. Retained local maximum/minimum candidates become the true local maximum/minimum.
		status->lastPeak = truePeak;
		status->lastPeakSample = status->sample;
		status->lastPeakValue = y;

		// Step 9. The number of steps is determined by the counting the number of pairs of the true local maximum and minimum.
		if (hasPrevious)
		{
			status->halfStepsInEpoch++;
		}
	}

#ifdef STEP_DEBUG_DUMP
	// Debug info
	if (status->sample == 0)
		printf("sample,epoch,u,meanU,x,y,a,s,peak,truePeak,halfStepsInEpoch,cumulative steps\n");
	printf("%d,%d,%f,%f,%f,%f,%f,%d,%d,%d,%d,%d\n", status->sample, status->lastEpoch, u, meanU, x, y, a, s, peak, truePeak, status->halfStepsInEpoch, status->cumulativeStepsReported);
#endif

	// Increment sample number
	status->sample++;

	return true;
}

// Free data resources
int StepClose(step_status_t *status)
{
	// Print partial result
	if (status->epochStartTime != 0)
	{
		StepPrint(status);		// Print for last, incomplete block, if it has at least one valid second in
	}

	if (status->file != NULL)
	{
		fclose(status->file);
	}
	return 0;
}

bool StepAddValue(step_status_t *status, double *value, double temp, bool valid)
{
	bool result = true;
	for (status->decimateAccumulator += STEP_RATE; status->decimateAccumulator >= status->configuration->sampleRate; status->decimateAccumulator -= status->configuration->sampleRate) {
		result &= StepAddValueInternal(status, value, temp, valid);
	}
	return result;
}
