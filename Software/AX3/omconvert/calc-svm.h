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

#ifndef SVM_H
#define SVM_H


//#include <stdint.h>
#include <stdbool.h>
//#include <stdlib.h>
#include <stdio.h>


// SVM Mode
enum SVM_MODE {
	SVMMO_ABS = 0,			// 0: abs(SVM-1)
	SVMMO_CLAMP = 1,		// 1: max(SVM-1, 0)
	SVMMO = 2,				// 2: SVM-1
	_SVMMO_RESERVED = 3,	// 3: (reserved)
	SVM_ABS = 4,			// 4: abs(SVM)			(VM input won't be below zero, but filtered output can be)
	SVM_CLAMP = 5,			// 5: max(SVM, 0)		(VM input won't be below zero, but filtered output can be)
	SVM = 6,				// 6: SVM				(VM input won't be below zero, but filtered output can be)
	_SVM_RESERVED = 7,		// 7: (reserved)
};


// SVM configuration
typedef struct
{
	char headerCsv;
	double sampleRate;
	const char *filename;
	char filter;
	char mode;
	char extended;		// Extended reporting (range, std, etc)
	double epoch;
	double startTime;
} svm_configuration_t;


#include "butter.h"

// SVM status
typedef struct
{
	svm_configuration_t *configuration;

	// Standard SVM
	FILE *file;
	double epochStartTime;	// Start time of current epoch
	int sample;				// Sample number
	int intervalSample;		// Valid samples within this interval

	// Standard SVM Filter values
	double B[BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)];
	double A[BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)];
	double z[BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)];		// Final/initial condition tracking
	int numCoefficients;


	// For average SVM
	double sumSvm;
	double sumTemperature;

	// StdDev & range
	double axisSum[3];
	double axisSumSquared[3];
	double axisMin[3];
	double axisMax[3];

	// Per-ecoch stats
	int countInvalid;
	int countClipped;
	int countClippedInput;
	int countClippedOutput;

	int rawIndex;
	int countRaw;

} svm_status_t;


// Load data
char SvmInit(svm_status_t *status, svm_configuration_t *configuration);

// Processes the specified value
bool SvmAddValue(svm_status_t *status, double *value, double temp, char validity, int rawIndex);

// Free data resources
int SvmClose(svm_status_t *status);

#endif

