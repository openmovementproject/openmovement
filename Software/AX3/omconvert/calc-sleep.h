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

#ifndef SLEEP_H
#define SLEEP_H


//#include <stdint.h>
#include <stdbool.h>
//#include <stdlib.h>
#include <stdio.h>


// Sleep configuration
typedef struct
{
	char headerCsv;
	char timeCsv;
	double sampleRate;
	const char *filename;
	int summaryEpochs;			// Number of (1-second) epochs to summarize over
	double startTime;
} sleep_configuration_t;


// Sleep status
typedef struct
{
	sleep_configuration_t *configuration;
	FILE *file;
	int sample;					// Sample index (from start)

	// Epoch tracking
	int intervalSample;			// Count of valid samples within this epoch
	double axisSum;				// Sum of axis values within this epoch
	double axisSumSquared;		// Sum of squared axis values within this epoch

	// Segment tracking
	double sleepStartTime;		// First sample timestamp in current segment
	int epochsSleeping;			// Contiguous epochs sleeping (the current segment)

} sleep_status_t;

// Load data
char SleepInit(sleep_status_t *status, sleep_configuration_t *configuration);

// Processes the specified value
bool SleepAddValue(sleep_status_t *status, double *value, double temp, bool valid);

// Free data resources
int SleepClose(sleep_status_t *status);

#endif

