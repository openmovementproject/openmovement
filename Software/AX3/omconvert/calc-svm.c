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

#include "butter.h"
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
		fprintf(status->file, "Time,Mean SVM (g)");
		if (configuration->extended >= 1)
		{
			// Extended header
			fprintf(status->file, ",Range X (g),Range Y (g),Range Z (g),STD X (g),STD Y (g),STD Z (g),Temperature (C),Num Samples,Invalid Samples,Clipped Input,Clipped Output,Raw Samples");
		}
		fprintf(status->file, "\n");
	}

	// Filter parameters
	int order = 4;
	double Fc1 = 0.5;			// -> 0.2
	double Fc2 = 20;			// 15
	double Fs = status->configuration->sampleRate;

	if (status->configuration->filter == 2) { Fc1 = -1; }		// Low-pass only
	if (Fc2 >= Fs / 2) { Fc2 = -1.0; }				// High-pass filter instead (upper band cannot exceed Nyquist limit)

	// Calculate normalized cut-offs
	double W1 = Fc1 / (Fs / 2);
	double W2 = Fc2 / (Fs / 2);

	// Calculate coefficients
	status->numCoefficients = CoefficientsButterworth(order, W1, W2, status->B, status->A);

#if 0
	// # Fc1 = 0.5; Fc2 = 20; Fs = 100; order = 4; [b, a] = butter(4, [Fc1, Fc2] . / (Fs / 2))
	// # b = 0.0430         0 -0.1720         0    0.2580         0 -0.1720         0    0.0430
	// # a = 1.0000 -4.7509    9.7480 -11.6172    9.1051 -4.8601    1.6715 -0.3294    0.0329

	// Display coefficients
	int i;
	printf("B: "); for (i = 0; i < status->numCoefficients; i++) { printf("%f ", status->B[i]); } printf("\n");
	printf("A: "); for (i = 0; i < status->numCoefficients; i++) { printf("%f ", status->A[i]); } printf("\n");
#endif

	status->sample = 0;

	return 1; // (status->file != NULL) ? 1 : 0;
}


static void SvmPrint(svm_status_t *status)
{
	// Resulting mean and StdDev
	double meanSvm = 0;
//	double resultMean[3] = { 0 };
	double resultRange[3] = { 0 };
	double resultStdDev[3] = { 0 };
	double resultTemperature = 0.0;

	if (status->intervalSample > 0)
	{ 
		int c;
		meanSvm = status->sumSvm / status->intervalSample; 
		// Per-axis StdDev and range
		for (c = 0; c < AXES; c++)
		{
			double mean = status->intervalSample == 0 ? 0 : status->axisSum[c] / status->intervalSample;
			double squareOfMean = mean * mean;
			double averageOfSquares = status->intervalSample == 0 ? 0 : (status->axisSumSquared[c] / status->intervalSample);
			double standardDeviation = sqrt(averageOfSquares - squareOfMean);
			if (isnan(standardDeviation)) { standardDeviation = 0; }
//			resultMean[c] = mean;
			resultStdDev[c] = standardDeviation;
			resultRange[c] = status->axisMax[c] - status->axisMin[c];
		}
		resultTemperature = status->intervalSample == 0 ? 0 : status->sumTemperature / status->intervalSample;
	}

	if (status->file != NULL)
	{
		char timestring[24];	// 2000-01-01 12:00:00.000\0
		const char *none = NULL;
		if (status->intervalSample == 0 && (status->configuration->extended > 1 || status->configuration->extended < 0))
		{
			switch (status->configuration->extended) 
			{
				case -2:
				case 2: none = ""; break;
				case -3:
				case 3: none = "nan"; break;
				case -4:
				case 4: none = "0.0"; break;
				case -5:
				case 5: none = "0"; break;
				case -6:
				case 6: none = "-1"; break;
				default: none = "0.0"; break;
			}
		}

		time_t tn = (time_t)status->epochStartTime;
		struct tm *tmn = gmtime(&tn);
		float sec = tmn->tm_sec + (float)(status->epochStartTime - (time_t)status->epochStartTime);
		sprintf(timestring, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec);	// (int)((sec - (int)sec) * 1000)

		// "Time, Mean SVM (g)"
		fprintf(status->file, "%s", timestring);
		if (status->intervalSample == 0 && none != NULL)
		{
			fprintf(status->file, ",%s", none);
		}
		else 
		{
			fprintf(status->file, ",%f", meanSvm);
		}

		if (status->configuration->extended >= 1)
		{
			// "Range X (g),Range Y (g),Range Z (g),STD X (g),STD Y (g),STD Z (g),Temperature (C),Total Samples,Invalid Samples,Clipped Input,Clipped Output,Raw Samples"

			if (status->intervalSample == 0 && none != NULL)
			{
				fprintf(status->file, ",%s,%s,%s,%s,%s,%s,%s", none, none, none, none, none, none, none);
			}
			else
			{
				fprintf(status->file, ",%f,%f,%f", resultRange[0], resultRange[1], resultRange[2]);
				fprintf(status->file, ",%f,%f,%f", resultStdDev[0], resultStdDev[1], resultStdDev[2]);
				fprintf(status->file, ",%.2f", resultTemperature);
			}

			fprintf(status->file, ",%d", status->intervalSample);
			fprintf(status->file, ",%d", status->countInvalid);
			fprintf(status->file, ",%d", status->countClippedInput);
			fprintf(status->file, ",%d", status->countClipped);			// Clipped input or output
			fprintf(status->file, ",%d", status->countRaw);				// Raw samples
		}
		fprintf(status->file, "\n");

#ifdef _DEBUG
	fflush(status->file);		// !!!!???? HACK: Only for debugging, remove
#endif
	}
}


// Processes the specified value
bool SvmAddValue(svm_status_t *status, double* accel, double temp, char validity, int rawIndex)
{
	int countRaw = 0;
	int c;

	if (rawIndex >= status->rawIndex) {
		countRaw = rawIndex - status->rawIndex;
	}
	status->rawIndex = rawIndex;

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

	// SVM
	double sumSquared = 0;
	for (c = 0; c < AXES; c++)
	{
		double v = accel[c];
		sumSquared += v * v;
	}
	double svm = sqrt(sumSquared);

	if (status->configuration->mode < 4)	// Mode 0-3 are SVM-1, modes 4-7 are straight SVM
	{
		svm -= 1;
	}
	if (status->configuration->filter != 0)
	{
		filter(status->numCoefficients, status->B, status->A, &svm, &svm, 1, status->z);
	}

	// SVM mode (must be after filtering)
	switch (status->configuration->mode & 3)
	{
		case 0: svm = fabs(svm); break;					// Standard abs(v) mode
		case 1: if (svm < 0) { svm = 0.0; } break;		// Clamp max(0,v) mode
		case 2: break;									// Pass-through mode (sum will include values <0 !)
		case 3:	break;									// (reserved)
	}

	// Invalid?
	if (validity & 0x01) 
	{ 
		status->countInvalid++; 
	}
	else
	{
		// Count data if valid

		// Axis StdDev & range
		for (c = 0; c < AXES; c++)
		{
			double v = accel[c];
			status->axisSum[c] += v;
			status->axisSumSquared[c] += v * v;
			if (status->intervalSample == 0 || accel[c] < status->axisMin[c]) { status->axisMin[c] = accel[c]; }
			if (status->intervalSample == 0 || accel[c] > status->axisMax[c]) { status->axisMax[c] = accel[c]; }
		}

		// Mean SVM
		status->sumSvm += svm;
		status->sumTemperature += temp;

		status->intervalSample++;

		status->countRaw += countRaw;
	}

	// Sum number of clipped samples
	if (validity & 0x02) { status->countClippedInput++; }
	if (validity & 0x04) { status->countClippedOutput++; }
	if ((validity & 0x02) || (validity & 0x04)) { status->countClipped++; }

	status->sample++;

	// Report SVM epoch
	int interval = ((int)(status->configuration->sampleRate * status->configuration->epoch + 0.5));
	if (status->sample > 0 && interval > 0 && status->sample % interval == 0)
	{
		SvmPrint(status);
		status->intervalSample = 0;
		status->sumSvm = 0;
		status->sumTemperature = 0;
		status->epochStartTime = 0;

		status->countInvalid = 0;
		status->countClippedInput = 0;
		status->countClippedOutput = 0;
		status->countClipped = 0;

		status->countRaw = 0;
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

