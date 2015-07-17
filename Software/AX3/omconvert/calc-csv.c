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
#include "calc-csv.h"


#define AXES 3


// Load data
char CsvInit(csv_status_t *status, csv_configuration_t *configuration)
{
	memset(status, 0, sizeof(csv_status_t));
	status->configuration = configuration;

	if (status->configuration->sampleRate <= 0.0)
	{
		fprintf(stderr, "ERROR: CSV sample rate not specified.\n");
		return 0;
	}

	status->file = NULL;
	if (configuration->filename != NULL && strlen(configuration->filename) > 0)
	{
		status->file = fopen(configuration->filename, "wt");
		if (status->file == NULL)
		{
			fprintf(stderr, "ERROR: CSV file not opened.\n");
		}
	}

	status->sample = 0;

	// .CSV header
	if (status->file && configuration->headerCsv)
	{
		fprintf(status->file, "Time,Accel-X (g), Accel-Y (g), Accel-Z (g)\n");
	}

	return (status->file != NULL) ? 1 : 0;
}

// Processes the specified value
bool CsvAddValue(csv_status_t *status, double* accel, double temp, bool valid)
{
	int c;

	double t = status->configuration->startTime + (status->sample / status->configuration->sampleRate);

	status->sample++;

	// Report SVM epoch
	char timestring[24];	// 2000-01-01 12:00:00.000\0

	time_t tn = (time_t)t;
	struct tm *tmn = gmtime(&tn);
	float sec = tmn->tm_sec + (float)(t - (time_t)t);
	sprintf(timestring, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec, (int)((sec - (int)sec) * 1000));

	if (status->file != NULL)
	{
		fprintf(status->file, "%s", timestring);
		for (c = 0; c < AXES; c++)
		{
			fprintf(status->file, ",%f", accel[c]);
		}
		fprintf(status->file, "\n");
	}

	return true;
}

// Free data resources
int CsvClose(csv_status_t *status)
{
	if (status->file != NULL) 
	{ 
		fclose(status->file);
	}
	return 0;
}

