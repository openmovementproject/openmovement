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

// Open Movement SVM Processor
// Dan Jackson, 2014


/*
Cut-point calculation
"Validation of the GENEA Accelerometer" by Esliger et al.

  SVMgs = SUM(abs(sqrt(x^2 + y^2 + z^2) – 1))

Where sum is for 60 seconds at 80Hz, so divide all cut-points below by 4800 to work with any average SVM (???)

									GENEA	R/Dom	L/N-D	Waist	
Sedentary	 < 1.5 METS										
Light   	>= 1.5 METS	< 4 METS	 386	 386	 217	  77	
Moderate	>= 4.0 METS	< 7 METS	 542	 440	 645	 220	
Vigorous	>= 7.0 METS	        	1811	2099	1811	2057	

// Original SVMgs = SUM(abs(sqrt(x ^ 2 + y ^ 2 + z ^ 2) – 1)) at 80Hz (from "Validation of the GENEA Accelerometer" by Esliger et al.) so these cut-points * 1/(60*80) for average SVM?
// Senentary, Light (>= 1.5 METS, < 4 METS), Moderate (>= 4.0 METS, < 7 METS), Vigorous (>= 7.0 METS)
*/

/*
From Phillips et al ("Calibration of the GENEA accelerometer for assessment of physical activity intensity in children")
Author, Site (Rate), Sedentary/Light/Moderate/Vigorous (g/s)
Phillips et al, Left Wrist, 80Hz,  <7, 7-19, 20-60, >60
Phillips et al, Right Wrist, 80Hz, <6, 6-21, 22-56, >56
*/

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "calc-paee.h"


#define PAEE_NORMALIZE (1.0 / (60.0 * 80.0))		// Normalize as figures in paper were direct sum of abs(SVM-1) for 60 seconds at 80Hz
const double paeeCutPointWrist[PAEE_CUT_POINTS]  = { 386.0 * PAEE_NORMALIZE, 542.0 * PAEE_NORMALIZE, 1811.0 * PAEE_NORMALIZE };	// In GENEA analysis
const double paeeCutPointWristR[PAEE_CUT_POINTS] = { 386.0 * PAEE_NORMALIZE, 440.0 * PAEE_NORMALIZE, 2099.0 * PAEE_NORMALIZE };
const double paeeCutPointWristL[PAEE_CUT_POINTS] = { 217.0 * PAEE_NORMALIZE, 645.0 * PAEE_NORMALIZE, 1811.0 * PAEE_NORMALIZE };
const double paeeCutPointWaist[PAEE_CUT_POINTS]  = {  77.0 * PAEE_NORMALIZE, 220.0 * PAEE_NORMALIZE, 2057.0 * PAEE_NORMALIZE };

#define AXES 3


// Load data
char PaeeInit(paee_status_t *status, paee_configuration_t *configuration)
{
	memset(status, 0, sizeof(paee_status_t));
	status->configuration = configuration;

	if (configuration->minuteEpochs <= 0) { configuration->minuteEpochs = 1; }

	if (status->configuration->sampleRate <= 0.0)
	{
		fprintf(stderr, "ERROR: PAEE sample rate not specified.\n");
		return 0;
	}

	if (status->configuration->cutPoints == NULL)
	{
		fprintf(stderr, "ERROR: PAEE cut points rate not specified.\n");
		return 0;
	}

	status->file = NULL;
	if (configuration->filename != NULL && strlen(configuration->filename) > 0)
	{
		status->file = fopen(configuration->filename, "wt");
		if (status->file == NULL)
		{
			fprintf(stderr, "ERROR: PAEE file not opened.\n");
		}
	}

	// .CSV header
	if (status->file && configuration->headerCsv)
	{
		fprintf(status->file, "Time,Sedentary (mins),Light (mins),Moderate (mins),Vigorous (mins)\n");
	}

	// Filter parameters
	int order = 4;
	double Fc1 = 0.5;
	double Fc2 = 20;			// 15
	double Fs = status->configuration->sampleRate;
	if (Fc2 >= Fs / 2) { Fc2 = -1.0; }				// High-pass filter instead (upper band cannot exceed Nyquist limit)

	// Calculate normalized cut-offs
	double W1 = Fc1 / (Fs / 2);
	double W2 = Fc2 / (Fs / 2);

	// Calculate coefficients
	status->numCoefficients = CoefficientsButterworth(order, W1, W2, status->B, status->A);

	/*
	// Display coefficients
	int i;
	printf("B: ");
	for (i = 0; i < status->numCoefficients; i++) { printf("%f ", status->B[i]); }
	printf("\n");
	printf("A: ");
	for (i = 0; i < status->numCoefficients; i++) { printf("%f ", status->A[i]); }
	printf("\n");
	*/

	// Reset
	status->epochStartTime = 0;
	status->sample = 0;
status->sumSvm = 0;

	return (status->file != NULL) ? 1 : 0;
}


void PaeePrint(paee_status_t *status)
{
	int c;
	if (status->file != NULL)
	{
		char timestring[24];	// 2000-01-01 12:00:00.000\0

		time_t tn = (time_t)status->epochStartTime;
		struct tm *tmn = gmtime(&tn);
		float sec = tmn->tm_sec + (float)(status->epochStartTime - (time_t)status->epochStartTime);
		sprintf(timestring, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec);	// (int)((sec - (int)sec) * 1000)

		fprintf(status->file, "%s", timestring);
		for (c = 0; c < PAEE_CUT_POINTS + 1; c++)
		{
			fprintf(status->file, ",%u", (int)(status->minutesAtLevel[c] + 0.5));
		}
		fprintf(status->file, "\n");

	}
}


// Processes the specified value
bool PaeeAddValue(paee_status_t *status, double* accel, double temp, bool valid)
{
	int c;

	if (status->epochStartTime == 0)
	{
		status->epochStartTime = status->configuration->startTime + (status->sample / status->configuration->sampleRate);
	}

	// SVM
	double sumSquared = 0;
	for (c = 0; c < AXES; c++)
	{
		double v = accel[c];
		sumSquared += v * v;
	}
	double svm = sqrt(sumSquared);

#if 0
	if (!(status->configuration->mode < 4))	// Mode 0/1 are SVM-1, modes 2-4 are SVM
#endif
	{
		svm -= 1;
	}

	if (status->configuration->filter)
	{
		filter(status->numCoefficients, status->B, status->A, &svm, &svm, 1, status->z);
	}

#if 0
	// SVM mode (must be after filtering)
	switch (status->configuration->mode & 3)
	{
		case 0: 
#endif
			svm = fabs(svm); 
#if 0
			break;					// Standard abs(v) mode
		case 1: if (svm < 0) { svm = 0.0; } break;		// Clamp max(0,v) mode
		case 2: break;									// Pass-through mode (sum will include values <0 !)
		case 3:	break;									// (reserved)
	}
#endif

	if (valid)
	{
		status->sumSvm += svm;
		status->intervalSample++;
	}

	status->sample++;

	// Report PAEE epoch
	int interval = ((int)(status->configuration->sampleRate * 60 + 0.5));
	if (status->sample > 0 && status->sample % interval == 0)
	{
		double meanSvm = 0;
		if (status->intervalSample > 0) { meanSvm = status->sumSvm / status->intervalSample; }

		// Add to the correct cut points
		for (c = PAEE_CUT_POINTS; c >= 0; c--)
		{
			if (c == 0 || meanSvm >= status->configuration->cutPoints[c - 1])
			{
				status->minutesAtLevel[c]++;
				break;
			}
		}

		status->minute++;

		if (status->minute >= status->configuration->minuteEpochs)
		{
			PaeePrint(status);

			status->minute = 0;
			for (c = 0; c < PAEE_CUT_POINTS + 1; c++) { status->minutesAtLevel[c] = 0; }
			status->epochStartTime = 0;
		}

		status->sumSvm = 0;
		status->intervalSample = 0;
	}

	return true;
}

// Free data resources
int PaeeClose(paee_status_t *status)
{
	if (status->minute > 0)
	{
		PaeePrint(status);	// Print PAEE for last block if it has at least one minute in
	}
	if (status->file != NULL)
	{ 
		fclose(status->file);
	}
	return 0;
}
