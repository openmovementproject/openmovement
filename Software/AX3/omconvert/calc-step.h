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

// Step Counter
// Dan Jackson

#ifndef STEP_H
#define STEP_H

//#include <stdint.h>
#include <stdbool.h>
//#include <stdlib.h>
#include <stdio.h>

// Consts
#define STEP_AXES 3							// triaxial input
#define STEP_G_RANGE 2.0					// clamp input range +/- 2g
#define STEP_PRESCALE 1						// input units not specified (1=g, 9.81=m/s/s, 1000=1mg/LSB - but g shown in graphs)
#define STEP_RATE 20						// 20 Hz
#define STEP_DC_FILTER_SIZE (STEP_RATE)		// 20 samples = 1 second
#define STEP_LP_FILTER_SIZE (7)				//  7 samples
#define STEP_AVERAGE_SIZE (STEP_RATE / 2)	// 10 samples = 0.5 seconds

// Step counter configuration
typedef struct
{
	char headerCsv;
	char timeCsv;
	char formatCsv;				// 0=own
	double sampleRate;
	const char *filename;
	int secondEpochs;			// Number of second epochs to summarize over
	double startTime;
} step_configuration_t;


// Step status
typedef struct
{
	step_configuration_t *configuration;

	FILE *file;
	double decimateAccumulator;						// Input decimation accumulator
	double epochStartTime;								// Start time of current epoch (0=not started)
	unsigned int sample;									// Sample number
	unsigned int lastEpoch;								// Last epoch number reported

	double dcFilter[STEP_DC_FILTER_SIZE];	// DC box filter to subtract from SVM
	double lpFilter[STEP_LP_FILTER_SIZE];	// Low-pass filter to extract from DC-filtered
	double average[STEP_AVERAGE_SIZE];		// Moving average to exclude false peaks
	double lastY;													// Previous value from the low-pass filter
	int lastS;														// Previous sign value
	int lastPeak;													// Previous true peak type
	double lastPeakValue;									// Previous true peak value
	unsigned int lastPeakSample;					// Previous true peak sample index

	int halfStepsInEpoch;									// Current half-step count in this epoch (max 1 carry)

	int cumulativeStepsReported;					// Total steps reported

	int written;													// number of written lines
} step_status_t;


// Load data
char StepInit(step_status_t *status, step_configuration_t *configuration);

// Processes the specified value
bool StepAddValue(step_status_t *status, double *value, double temp, bool valid);

// Free data resources
int StepClose(step_status_t *status);

#endif
