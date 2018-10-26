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

#ifndef AGFILTER_H
#define AGFILTER_H

//#include <stdint.h>
#include <stdbool.h>
//#include <stdlib.h>
#include <stdio.h>

// AG-filter configuration
typedef struct
{
	char headerCsv;
	char timeCsv;
	char formatCsv;			// 0=own, 1=AG
	double sampleRate;
	const char *filename;
	int secondEpochs;		// Number of second epochs to summarize over
	double startTime;
} agfilter_configuration_t;

#define AG_AXES 3
#define AG_MAX_COEFFICIENTS 30
//#define AG_VM_FLOAT

// AG-filter status
typedef struct
{
	agfilter_configuration_t *configuration;

	FILE *file;
	double decimateAccumulator;						// Input decimation accumulator
	double epochStartTime;		// Start time of current epoch
	int intervalSample;			// Valid seconds within this larger epoch
	int sample;					// Sample number
	int epoch;					// Epoch number

	int integCount;				// Integration count

	// Standard filter values
	double B[AG_MAX_COEFFICIENTS];
	double A[AG_MAX_COEFFICIENTS];
	double z[AG_AXES][AG_MAX_COEFFICIENTS];		// Final/initial condition tracking (per-axis)
	int numCoefficients;

	int axisSum[AG_AXES];	// per-axis second-epoch sum
	int axisTotal[AG_AXES];	// per-axis larger-epoch sum

#ifdef AG_VM_FLOAT
	double vmSum;			// VM second-epoch sum
	double vmTotal;			// VM larger-epoch sum
#else
	int vmSum;				// VM second-epoch sum
	int vmTotal;			// VM larger-epoch sum
#endif

	int written;			// number of written lines
} agfilter_status_t;


// Load data
char AgFilterInit(agfilter_status_t *status, agfilter_configuration_t *configuration);

// Processes the specified value
bool AgFilterAddValue(agfilter_status_t *status, double *value, double temp, bool valid);

// Free data resources
int AgFilterClose(agfilter_status_t *status);

#endif
