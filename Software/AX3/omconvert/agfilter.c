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

// "AG aggregation method and band-pass filter"
// Dan Jackson

// Algorithm implementation based on:
//   "Frequency filtering and the aggregation of raw accelerometry into Actigraph counts."
//   Jan Brond, Daniel Arvidsson, Lars Bo Andersen. 
//   5th International Conference on Ambulatory Monitoring of Physical Activity and Movement (ICAMPAM) June 2017. 

// See also:
//   "Exploring the ActiLife filtration algorithm: converting raw acceleration data to counts", D Peach, J Van Hoomissen, and H L Callender.
//   Physiol. Meas. 35 (2014) 2359-2367. http://iopscience.iop.org/article/10.1088/0967-3334/35/12/2359/pdf
// ...which says:
// Frequency filtration: a peak frequency of 0.77 Hz, with 50% cut-offs at 0.13 Hz and 2.73 Hz. 
// Compresses 30 Hz `x'[n]`  by computing the Euclidean norm of every 30 consecutive data points.  

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "agfilter.h"
#include "butter.h"

// Coefficients are derived from best fit (1st order Butterworth was tried with cut-offs 0.29 and 1.66, but not accurate enough)
#include "agcoefficients.h"     // AgN, AgB, AgA, etc...

/*
Summary:
Step 1. Each axis is treated separately:
Step 2. Input data (units of g) is resampled to 30 Hz...
Step 3. ...filtered using the (fitted, optimal band-pass filter) coefficients (where B is scaled by the gain of 0.965), (documented filter should have a center frequency of 0.75Hz)
Step 4. ...decimated to 1 in downsample=3 samples (10 Hz)
Step 5. ...has the abs() absolute value taken
Step 6. ...clamped/saturated to +/- peakThreshold=2.13 (from the documented full range of 4.26g)
Step 7. ...any value less than deadband=0.068 is zeroed
Step 8. ...divided by the adcResolution=0.0164 (but documented as 4.26/256 = 0.016640625, without the gain?)
Step 9. ...integer floor taken
Step 10. ...summed in blocks of integN=10 (1 second epochs)
Step 11. ...and larger epochs are accumulated from the second epochs.
Step 12. VM is performed afterwards on the final axis counts (regardless of epoch duration or pre-fitered values)
*/


// Load data
char AgFilterInit(agfilter_status_t *status, agfilter_configuration_t *configuration)
{
	memset(status, 0, sizeof(agfilter_status_t));
	status->configuration = configuration;

	// Step 1. Each axis is treated separately.
	// Copy coefficients
	int i;
	status->numCoefficients = agN;
	if (status->numCoefficients > AG_MAX_COEFFICIENTS)
	{
		fprintf(stderr, "WARNING: AgFilter number of coefficients too large.\n");
		status->numCoefficients = AG_MAX_COEFFICIENTS;
	}
	for (i = 0; i < status->numCoefficients; i++)
	{
		status->A[i] = agA[i];
		status->B[i] = agGain * agB[i];
	}

	// Status tracking (per-axis)
	int c;
	for (c = 0; c < AG_AXES; c++)
	{
		memset(status->z[c], 0, status->numCoefficients * sizeof(double));
	}


	// Step 2. Input data(units of g) is resampled to 30 Hz.
	if (status->configuration->sampleRate <= 0.0)
	{
		fprintf(stderr, "ERROR: AgFilter sample rate not specified.\n");
		return 0;
	}
	status->decimateAccumulator = status->configuration->sampleRate - agSf;	
	if (status->configuration->sampleRate != agSf)
	{
		fprintf(stderr, "WARNING: AgFilter sample rate must be %d Hz (use option: -resample %d) - will decimate input sample rate %d:%.2f.\n", agSf, agSf, agSf, status->configuration->sampleRate);
	}

	status->file = NULL;
	if (configuration->filename != NULL && strlen(configuration->filename) > 0)
	{
		status->file = fopen(configuration->filename, "wt");
		if (status->file == NULL)
		{
			fprintf(stderr, "ERROR: AgFilter file not opened.\n");
		}
	}

	// .CSV header
	if (status->file && configuration->headerCsv && status->configuration->formatCsv != 1 && status->configuration->formatCsv != 3)
	{
		fprintf(status->file, "Time,CountsX,CountsY,CountsZ,CountsVM");
		fprintf(status->file, "\n");
	}

	status->sample = 0;
	status->integCount = 0;

	return 1; // (status->file != NULL) ? 1 : 0;
}


static void AgFilterPrint(agfilter_status_t *status)
{
	if (status->file != NULL)
	{
		char timestring[24];	// 2000-01-01 12:00:00.000\0

		time_t tn = (time_t)status->epochStartTime;
		struct tm *tmn = gmtime(&tn);
		float sec = tmn->tm_sec + (float)(status->epochStartTime - (time_t)status->epochStartTime);

		if (status->configuration->formatCsv == 1 || status->configuration->formatCsv == 3)
		{
			sprintf(timestring, "%02d/%02d/%04d,%02d:%02d:%02d", tmn->tm_mday, tmn->tm_mon + 1, 1900 + tmn->tm_year, tmn->tm_hour, tmn->tm_min, (int)sec);	// (int)((sec - (int)sec) * 1000)
		}
		else
		{
			sprintf(timestring, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec);	// (int)((sec - (int)sec) * 1000)
		}

		if (status->configuration->headerCsv && status->written <= 0 && (status->configuration->formatCsv == 1 || status->configuration->formatCsv == 3))
		{
			if (status->configuration->formatCsv == 3)
			{
				fprintf(status->file, "------------ Data Table File Created By OpenMvmnt XXXXXXXX omconvrt v9.99.9 Firmware v9.9.9 date format dd/MM/yyyy Filter Normal -----------\n");
			}
			else 
			{
				fprintf(status->file, "------------ Data File Created By OpenMvmnt XXXXX omconvrt v9.99.9 Firmware v9.9.9 date format dd/MM/yyyy Filter Normal -----------\n");
			}
			fprintf(status->file, "Serial Number: TAS1E999%05d\n", 99999);
			fprintf(status->file, "Start Time %02d:%02d:%02d\n", tmn->tm_hour, tmn->tm_min, (int)sec);
			fprintf(status->file, "Start Date %02d/%02d/%04d\n", tmn->tm_mday, tmn->tm_mon + 1, 1900 + tmn->tm_year);
			fprintf(status->file, "Epoch Period (hh:mm:ss) %02d:%02d:%02d\n", status->configuration->secondEpochs / 60 / 60, (status->configuration->secondEpochs / 60) % 60, status->configuration->secondEpochs % 60);
			fprintf(status->file, "Download Time 00:00:00\n");
			fprintf(status->file, "Download Date 01/01/2000\n");
			fprintf(status->file, "Current Memory Address: 0\n");
			fprintf(status->file, "Current Battery Voltage: 4.20     Mode = %d\n", (status->configuration->formatCsv == 3) ? 61 : 12);
			fprintf(status->file, "--------------------------------------------------\n");
			// Data Table file has header:
			//fprintf(status->file, "Date,Time,Axis1,Axis2,Axis3,Steps,Lux,Inclinometer Off,Inclinometer Standing,Inclinometer Sitting,Inclinometer Lying,Vector Magnitude\n");
			if (status->configuration->formatCsv == 3)
			{
				fprintf(status->file, "Date,Time,Axis1,Axis2,Axis3,Vector Magnitude\n");
			}
			//fprintf(status->file, "Axis1,Axis2,Axis3,Vector Magnitude\n");
			//fprintf(status->file, "Axis1,Axis2,Axis3\n");
		}

		if (status->configuration->formatCsv != 1)
		{
			fprintf(status->file, "%s,", timestring);
		}

		if (status->configuration->formatCsv == 1 || status->configuration->formatCsv == 3)
		{
			// AG exports have raw data first two columns swapped (YXZ)
			fprintf(status->file, "%d,%d,%d", status->axisTotal[1], status->axisTotal[0], status->axisTotal[2]);
		}
		else
		{
			int c;
			for (c = 0; c < AG_AXES; c++)
			{
				fprintf(status->file, "%s%d", c > 0 ? "," : "", status->axisTotal[c]);
			}
		}

		if (status->configuration->formatCsv != 1)
		{
#if 1
			// Unusually AG's "VM" counts are the vector magnitude of the final axis-counts (after any epoch duration, and without regard to the true VM of the values before aggregation)
			double vm = 0.0;
			int c;
			for (c = 0; c < AG_AXES; c++)
			{
				vm += status->axisTotal[c] * status->axisTotal[c];
			}
			vm = sqrt(vm);
			fprintf(status->file, ",%f", vm);
#else
			#ifdef AG_VM_FLOAT
				fprintf(status->file, ",%f", status->vmTotal);
			#else
				fprintf(status->file, ",%d", status->vmTotal);
			#endif
#endif
		}

		fprintf(status->file, "\n");

		status->written++;

#ifdef _DEBUG
		fflush(status->file);		// !!!!???? HACK: Only for debugging, remove
#endif
	}
}


// Processes the specified value
bool AgFilterAddValueInternal(agfilter_status_t *status, double *value, double temp, bool valid)
{
	const int effectiveRate = agSf; // status->configuration->sampleRate
	int c;

	if (status->epochStartTime == 0)
	{
		status->epochStartTime = status->configuration->startTime + (status->sample / effectiveRate);
		for (c = 0; c < AG_AXES; c++)
		{
			status->axisSum[c] = 0;		// within second
			status->axisTotal[c] = 0;	// within larger epoch of multiple seconds
		}
		status->vmSum = 0;
		status->vmTotal = 0;
	}

	// Decimate
	bool decimate = (status->sample % agDownsample == 0);

	// Per-axis
	for (c = 0; c < AG_AXES; c++)
	{
		double v = value[c];

		// Step 3. Data is filtered using the coefficients (where B is scaled by the gain of 0.965).
		filter(status->numCoefficients, status->B, status->A, &v, &v, 1, status->z[c]);

		// Step 4. Data is decimated to 1 in 'downsample' = 3 samples(10 Hz).
		if (decimate)
		{
			// Step 5. Data is clamped / saturated to + / -peakThreshold = 2.13 (4.26g range).
			if (v < -agPeakThreshold) { v = -agPeakThreshold; }
			else if (v > agPeakThreshold) { v = agPeakThreshold; }

			// Step 6. Data has the abs() absolute value taken.
			v = fabs(v);

			// Step 7. Data less than deadband = 0.068 is zeroed.
			if (v < agDeadband) { v = 0.0; }

			// Step 8. Data is divided by the adcResolution = 0.0164.
			v /= agAdcResolution;

			// Step 9. Data has the integer floor taken.
			int iv = (int)v;

			// Step 10. Data is summed in blocks of integN = 10 (1 second epochs)
			status->axisSum[c] += iv;
		}
	}


	// Step 10. Data is summed in blocks of integN = 10 (1 second epochs)
	if (decimate)
	{
		status->integCount++;
		if (status->integCount >= agIntegN)
		{
			double sumSquare = 0.0;
			for (c = 0; c < AG_AXES; c++)
			{
				sumSquare += status->axisSum[c] * status->axisSum[c];
			}
			#ifdef AG_VM_FLOAT
				status->vmSum = sqrt(sumSquare);
			#else
				status->vmSum = (int)sqrt(sumSquare);
			#endif

			// Step 11. Larger epochs are accumulated from the second epochs.
			for (c = 0; c < AG_AXES; c++)
			{
				status->axisTotal[c] += status->axisSum[c];
			}
			status->vmTotal += status->vmSum;
			status->intervalSample++;

			// Start interval again
			status->integCount = 0;
			for (c = 0; c < AG_AXES; c++)
			{
				status->axisSum[c] = 0;
			}
			status->vmSum = 0;

			// Report AgFilter epoch
			if (status->intervalSample >= status->configuration->secondEpochs)
			{
				AgFilterPrint(status);
				status->intervalSample = 0;
				status->epochStartTime = 0;
			}

		}
	}

	status->sample++;

	return true;
}

// Free data resources
int AgFilterClose(agfilter_status_t *status)
{
	// Print partial result
	if (status->intervalSample > 0)
	{
		AgFilterPrint(status);		// Print for last, incomplete block, if it has at least one valid second in
	}

	if (status->file != NULL)
	{
		fclose(status->file);
	}
	return 0;
}

bool AgFilterAddValue(agfilter_status_t *status, double *value, double temp, bool valid)
{
	bool result = true;
	for (status->decimateAccumulator += agSf; status->decimateAccumulator >= status->configuration->sampleRate; status->decimateAccumulator -= status->configuration->sampleRate) {
		result &= AgFilterAddValueInternal(status, value, temp, valid);
	}
	return result;
}
