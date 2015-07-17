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

// Open Movement Physical Activity and Energy Expenditure Processor
// Dan Jackson, 2014

#ifndef PAEE_H
#define PAEE_H


//#include <stdint.h>
#include <stdbool.h>
//#include <stdlib.h>
#include <stdio.h>


// PAEE configuration
typedef struct
{
	char headerCsv;
	double sampleRate;
	const char *filename;
	char filter;
	//char mode;
	const double *cutPoints;
	int minuteEpochs;		// Number of minute epochs to summarize over
	double startTime;
} paee_configuration_t;

#include "butter.h"

#define PAEE_CUT_POINTS 3

// PAEE status
typedef struct
{
	paee_configuration_t *configuration;

	// Standard PAEE
	FILE *file;
	double epochStartTime;		// Start time of current epoch
	int sample;					// Sample number
	int intervalSample;			// Valid samples within this minute
	int minute;					// Minute number
	double minutesAtLevel[PAEE_CUT_POINTS + 1];	// Minutes at each cut level

	// Standard SVM Filter values
	double B[BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)];
	double A[BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)];
	double z[BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)];		// Final/initial condition tracking
	int numCoefficients;

	double sumSvm;

} paee_status_t;


// Available points
const double paeeCutPointWrist[3];
const double paeeCutPointWristR[3];
const double paeeCutPointWristL[3];
const double paeeCutPointWaist[3];


// Load data
char PaeeInit(paee_status_t *status, paee_configuration_t *configuration);

// Processes the specified value
bool PaeeAddValue(paee_status_t *status, double *value, double temp, bool valid);

// Free data resources
int PaeeClose(paee_status_t *status);

#endif

