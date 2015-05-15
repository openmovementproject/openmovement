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


#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "butter4bp.h"
#include "calc-svm.h"


#define AXES 3


// Load data
char SvmInit(svm_status_t *status, svm_configuration_t *configuration)
{
	memset(status, 0, sizeof(svm_status_t));
	status->configuration = configuration;

	if (status->configuration->sampleRate <= 0.0)
	{
		fprintf(stderr, "ERROR: SVM sample rate not specified.\n");
		return 0;
	}

	status->file = NULL;
	if (configuration->filename != NULL && strlen(configuration->filename) > 0)
	{
		status->file = fopen(configuration->filename, "wt");
		if (status->file == NULL)
		{
			fprintf(stderr, "ERROR: SVM file not opened.\n");
		}
	}

	// .CSV header
	if (status->file && configuration->headerCsv)
	{
		fprintf(status->file, "Time,Mean SVM (g)\n");
	}

	// Filter parameters
	double Fc1 = 0.5;			// -> 0.2
	double Fc2 = 20;			// 15
	double Fs = status->configuration->sampleRate;

	// Calculate normalized cut-offs
	double W1 = Fc1 / (Fs / 2);
	double W2 = Fc2 / (Fs / 2);

	// Calculate coefficients
	CoefficientsButterworth4BP(W1, W2, status->B, status->A);

	/*
	// Display coefficients
	printf("B: ");
	for (i = 0; i < BUTTERWORTH4_NUM_COEFFICIENTS; i++)
	{
	printf("%f ", B[i]);
	}
	printf("\n");
	printf("A: ");
	for (i = 0; i < BUTTERWORTH4_NUM_COEFFICIENTS; i++)
	{
	printf("%f ", A[i]);
	}
	printf("\n");
	*/

	status->sample = 0;

	return 1; // (status->file != NULL) ? 1 : 0;
}


static void SvmPrint(svm_status_t *status)
{
	double meanSvm = 0;
	if (status->intervalSample > 0) { meanSvm = status->sumSvm / status->intervalSample; }
	if (status->file != NULL)
	{
		char timestring[24];	// 2000-01-01 12:00:00.000\0

		time_t tn = (time_t)status->epochStartTime;
		struct tm *tmn = gmtime(&tn);
		float sec = tmn->tm_sec + (float)(status->epochStartTime - (time_t)status->epochStartTime);
		sprintf(timestring, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec);	// (int)((sec - (int)sec) * 1000)

		fprintf(status->file, "%s,%f\n", timestring, meanSvm);
	}
}

// Processes the specified value
bool SvmAddValue(svm_status_t *status, double* accel, double temp, bool valid)
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
	double svm = sqrt(sumSquared) - 1;

	if (status->configuration->filter)
	{
		filter(status->B, status->A, &svm, 1, status->z);
	}

	// SVM mode (must be after filtering)
	if (status->configuration->mode == 1)		// Clamp mode
	{
		if (svm < 0) { svm = 0.0; }
	}
	else
	{
		svm = fabs(svm);			// Standard abs() mode
	}

	if (valid)
	{
		status->sumSvm += svm;
		status->intervalSample++;
	}

	status->sample++;

	// Report SVM epoch
	int interval = ((int)(status->configuration->sampleRate * status->configuration->epoch + 0.5));
	if (status->sample > 0 && status->sample % interval == 0)
	{
		SvmPrint(status);
		status->intervalSample = 0;
		status->sumSvm = 0;
		status->epochStartTime = 0;
	}

	return true;
}

// Free data resources
int SvmClose(svm_status_t *status)
{
	// Print partial result
	if (status->intervalSample > 0)
	{
		SvmPrint(status);		// Print SVM for last, incomplete block, if it has at least one valid sample in
	}

	if (status->file != NULL) 
	{ 
		fclose(status->file);
	}
	return 0;
}

