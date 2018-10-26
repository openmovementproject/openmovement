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
char CsvInit(csv_status_t *status, csv_configuration_t *configuration, int numChannels)
{
	memset(status, 0, sizeof(csv_status_t));
	status->configuration = configuration;
	status->numChannels = numChannels;

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
			fprintf(stderr, "ERROR: CSV file not opened: %s\n", configuration->filename);
		}
	}

	status->sample = 0;

	// .CSV header
	if (status->file && configuration->headerCsv)
	{
		if (configuration->format == CSV_FORMAT_AG || configuration->format == CSV_FORMAT_AGDT)
		{
			// "ActiGraph(tm) ActiLife"-compatible .CSV export (may need to be at 30Hz to work properly)
			double t = status->configuration->startTime;
			time_t tn = (time_t)t;
			struct tm *tmn = gmtime(&tn);
			fprintf(status->file, "------------ Data File Created By ActiGraph GT3X+ %sActiLife v6.13.3 Firmware v3.0.0 date format dd/MM/yyyy at %d Hz  Filter Normal -----------\n", true?"omconvert ":"", (int)configuration->sampleRate);
			fprintf(status->file, "Serial Number: NEO1DXXXXXXXX\n");
			fprintf(status->file, "Start Time %02d:%02d:%02d\n", tmn->tm_hour, tmn->tm_min, tmn->tm_sec);
			fprintf(status->file, "Start Date %02d/%02d/%04d\n", tmn->tm_mday, tmn->tm_mon + 1, 1900 + tmn->tm_year);
			fprintf(status->file, "Epoch Period (hh:mm:ss) 00:00:00\n");
			fprintf(status->file, "Download Time 00:00:00\n");
			fprintf(status->file, "Download Date 01/01/2000\n");
			fprintf(status->file, "Current Memory Address: 0\n");
			fprintf(status->file, "Current Battery Voltage: 4.22     Mode = 12\n");
			fprintf(status->file, "--------------------------------------------------\n");
			fprintf(status->file, "Accelerometer X,Accelerometer Y,Accelerometer Z\n");

			status->numChannels = 3;	// only accel
		}
		else if (configuration->format == CSV_FORMAT_GA)
		{
			// "GENEActiv(tm) Software"-compatible .CSV export (may need to be at 80Hz to work properly?)
			double t = status->configuration->startTime;
			time_t tn = (time_t)t;
			struct tm *tmn = gmtime(&tn);
			float sec = tmn->tm_sec + (float)(t - (time_t)t);

			// File has 100-line header
			for (int lineNumber = 1; lineNumber <= 100; lineNumber++)
			{
				char line[1024] = "\n";
				switch (lineNumber)
				{
					case  1: sprintf(line, "Device Type,GENEActiv           \n"); break;
					case  2: sprintf(line, "Device Model,1.1\n"); break;
					case  3: sprintf(line, "Device Unique Serial Code,%d\n", 1); break;
					case  4: sprintf(line, "Device Firmware Version,Ver1.30 date 05Aug11\n"); break;
					case  5: sprintf(line, "Calibration Date,2000-01-01 00:00:00:000\n"); break;
					case  6: sprintf(line, "Application name & version ,%s\n", true?"omconvert":"GENEActiv PC Software 2.1"); break;

					case 11: sprintf(line, "Measurement Frequency,%d Hz\n", (int)status->configuration->sampleRate); break;
					case 12: sprintf(line, "Start Time,%04d-%02d-%02d %02d:%02d:%02d:%03d\n", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec, (int)((sec - (int)sec) * 1000)); break;
					case 13: sprintf(line, "Last measurement,\n"); break;
					case 14: sprintf(line, "Device Location Code,\n"); break;
					case 15: sprintf(line, "Time Zone,GMT +00\n"); break;

					case 21: sprintf(line, "Subject Code,Exported\n"); break;
					case 22: sprintf(line, "Date of Birth,01/01/1900\n"); break;
					case 23: sprintf(line, "Sex,\n"); break;
					case 24: sprintf(line, "Height,\n"); break;
					case 25: sprintf(line, "Weight,\n"); break;
					case 26: sprintf(line, "Handedness Code,\n"); break;
					case 27: sprintf(line, "Subject Notes,\n"); break;

					case 31: sprintf(line, "Study Centre,\n"); break;
					case 32: sprintf(line, "Study Code,\n"); break;
					case 33: sprintf(line, "Investigator ID,\n"); break;
					case 34: sprintf(line, "Exercise Type,\n"); break;
					case 35: sprintf(line, "Config Operator ID,\n"); break;
					case 36: sprintf(line, "Config Time,2000-01-01 00:00:00:000:\n"); break;
					case 37: sprintf(line, "Config Notes,Notes\n"); break;
					case 38: sprintf(line, "Extract Operator ID,\n"); break;
					case 39: sprintf(line, "Extract Time,2000-01-01 00:00:00:000\n"); break;
					case 40: sprintf(line, "Extract Notes,\n"); break;

					case 51: sprintf(line, "Sensor type,MEMS accelerometer x-axis\n"); break;
					case 52: sprintf(line, "Range,-8 to 8             \n"); break;
					case 53: sprintf(line, "Resolution,0.0039\n"); break;
					case 54: sprintf(line, "Units,g                   \n"); break;
					case 55: sprintf(line, "Additional information, \n"); break;
					case 56: sprintf(line, "Sensor type,MEMS accelerometer y-axis\n"); break;
					case 57: sprintf(line, "Range,-8 to 8             \n"); break;
					case 58: sprintf(line, "Resolution,0.0039\n"); break;
					case 59: sprintf(line, "Units,g                   \n"); break;
					case 60: sprintf(line, "Additional information, \n"); break;
					case 61: sprintf(line, "Sensor type,MEMS accelerometer z-axis \n"); break;
					case 62: sprintf(line, "Range,-8 to 8             \n"); break;
					case 63: sprintf(line, "Resolution,0.0039\n"); break;
					case 64: sprintf(line, "Units,g                   \n"); break;
					case 65: sprintf(line, "Additional information, \n"); break;
					case 66: sprintf(line, "Sensor type,Lux Photodiode 400nm - 1100nm \n"); break;
					case 67: sprintf(line, "Range,0 to 5000           \n"); break;
					case 68: sprintf(line, "Resolution,5\n"); break;
					case 69: sprintf(line, "Units,lux                 \n"); break;
					case 70: sprintf(line, "Additional information, \n"); break;
					case 71: sprintf(line, "Sensor type,User button event marker\n"); break;
					case 72: sprintf(line, "Range,1 or 0\n"); break;
					case 73: sprintf(line, "Resolution, \n"); break;
					case 74: sprintf(line, "Units, \n"); break;
					case 75: sprintf(line, "Additional information,1=pressed\n"); break;
					case 76: sprintf(line, "Sensor type,Linear active thermistor\n"); break;
					case 77: sprintf(line, "Range,0 to 70             \n"); break;
					case 78: sprintf(line, "Resolution,0.1\n"); break;
					case 79: sprintf(line, "Units,deg. C              \n"); break;
					case 80: sprintf(line, "Additional information, \n"); break;

				}
				fprintf(status->file, "%s", line);
			}
			status->numChannels = 3; // accel only
		}
		else if (configuration->format == CSV_FORMAT_ACCEL)
		{
			fprintf(status->file, "Time,Accel-X (g), Accel-Y (g), Accel-Z (g)%s\n", status->numChannels > 3 ? ", Gyro-X (d/s), Gyro-Y (d/s), Gyro-Z (d/s)" : "");
		}
	}

	return (status->file != NULL) ? 1 : 0;
}

// Processes the specified value
bool CsvAddValue(csv_status_t *status, double* accel, double temp, bool valid)
{
	int c;

	double t = status->configuration->startTime + (status->sample / status->configuration->sampleRate);

	status->sample++;

	if (status->file != NULL)
	{
		bool timestamp = (status->configuration->format != CSV_FORMAT_AG && status->configuration->format != CSV_FORMAT_AGDT);

		if (timestamp)
		{
			// Report SVM epoch
			char timestring[24];	// 2000-01-01 12:00:00.000\0

			time_t tn = (time_t)t;
			struct tm *tmn = gmtime(&tn);
			float sec = tmn->tm_sec + (float)(t - (time_t)t);
			sprintf(timestring, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec, (int)((sec - (int)sec) * 1000));

			if (status->configuration->format == CSV_FORMAT_GA)
			{
				// Export format uses colon to separate decimal part of seconds
				timestring[19] = ':';
			}

			fprintf(status->file, "%s", timestring);
		}

		for (c = 0; c < status->numChannels; c++)
		{
			if (status->configuration->format == CSV_FORMAT_AG || status->configuration->format == CSV_FORMAT_AGDT)
			{
				fprintf(status->file, "%s%.3f", (c > 0) ? "," : "", accel[c]);
			}
			else if (status->configuration->format == CSV_FORMAT_GA)
			{
				fprintf(status->file, ",%.4f", accel[c]);
			}
			else
			{
				fprintf(status->file, "%s%f", (c > 0 || timestamp) ? "," : "", accel[c]);
			}
		}

		if (status->configuration->format == CSV_FORMAT_GA)
		{
			int lux = 0;
			int button = 0;
			float temp = 20.0f;
			fprintf(status->file, ",%d,%d,%.1f", lux, button, temp);
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

