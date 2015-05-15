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

// Open Movement Wear-Time Validation Processor
// Dan Jackson, 2014

#ifndef WTV_H
#define WTV_H


//#include <stdint.h>
#include <stdbool.h>
//#include <stdlib.h>
#include <stdio.h>


// WTV configuration
typedef struct
{
	char headerCsv;
	double sampleRate;
	const char *filename;
	//double epoch;
	double startTime;
	int halfHourEpochs;

	double wtvStdCutoff;	// Non-wear if std-dev < 3.0 mg (for at least 2 out of the 3 axes)
	double wtvRangeCutoff;	// or, non-wear if range < 50 mg (for at least 2 out of the 3 axes)
} wtv_configuration_t;


// WTV status
typedef struct
{
	wtv_configuration_t *configuration;

	// Standard WTV
	FILE *file;
	double epochStartTime;	// Start time of current epoch
	int sample;				// Sample number
	int intervalSample;		// Valid samples within this interval
	int halfHourEpochs;		// Number of 30-minute epochs to summarize over

	// StdDev & range
	double axisSum[3];
	double axisSumSquared[3];
	double axisMin[3];
	double axisMax[3];

	int numWindows;			// Number of 30-minute windows assessed
	int totalWorn;			// Count of windows that the device was worn

} wtv_status_t;


// Load data
char WtvInit(wtv_status_t *status, wtv_configuration_t *configuration);

// Processes the specified value
bool WtvAddValue(wtv_status_t *status, double *value, double temp, bool valid);

// Free data resources
int WtvClose(wtv_status_t *status);


#endif

