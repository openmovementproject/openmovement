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

// Open Movement CSV writer
// Dan Jackson, 2014

#ifndef CSV_H
#define CSV_H


//#include <stdint.h>
#include <stdbool.h>
//#include <stdlib.h>
#include <stdio.h>


// CSV file header options
#define CSV_FORMAT_ACCEL  0		// CSV export format is just time and acceleration in each axis
#define CSV_FORMAT_AG     1		// CSV export format matches that of "ActiGraph(tm) ActiLife" software
#define CSV_FORMAT_GA     2		// CSV export format matches that of "GENEActiv(tm) Software"
#define CSV_FORMAT_AGDT   3		// CSV export format matches that of "ActiGraph(tm) ActiLife" Data Table format (counts only)


// SVM configuration
typedef struct
{
	char headerCsv;
	int format;				// CSV_FORMAT_*
	double sampleRate;
	const char *filename;
	double startTime;
} csv_configuration_t;


// CSV status
typedef struct
{
	csv_configuration_t *configuration;

	// 
	FILE *file;
	int sample;				// Sample number
	int numChannels;

} csv_status_t;


// Load data
char CsvInit(csv_status_t *status, csv_configuration_t *configuration, int numChannels);

// Processes the specified value
bool CsvAddValue(csv_status_t *status, double *value, double temp, bool valid);

// Free data resources
int CsvClose(csv_status_t *status);

#endif

