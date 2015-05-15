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


// SVM configuration
typedef struct
{
	char headerCsv;
	double sampleRate;
	const char *filename;
	char filter;
	char mode;
	double epoch;
	double startTime;
} svm_configuration_t;


#include "butter4bp.h"

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
	double B[BUTTERWORTH4_NUM_COEFFICIENTS];
	double A[BUTTERWORTH4_NUM_COEFFICIENTS];
	double z[BUTTERWORTH4_NUM_COEFFICIENTS];		// Final/initial condition tracking

	double sumSvm;

} svm_status_t;


// Load data
char SvmInit(svm_status_t *status, svm_configuration_t *configuration);

// Processes the specified value
bool SvmAddValue(svm_status_t *status, double *value, double temp, bool valid);

// Free data resources
int SvmClose(svm_status_t *status);

#endif

