// Fix for some PCs: C/C++; Code Generation; Enable Enhanced Instruction Set; Streaming SIMD Extensions (/arch:SSE)

// DONE: Fixed an issue where the upper band-pass limit for SVM and PAEE (cut point) calculations was greater than the Nyquist limit for input data <= 40Hz -- now switches to a high-pass filter instead.
// DONE: Occasional x / y / z std values were NaN (should now be 0 instead)
// DONE: For interrupts, where there are no valid samples in the epoch, NaN can be emitted for the values:  use the parameter "-svm-extended", where 1=current behaviour of zero, 2=empty cell, 3="nan".
// DONE: temperature field formatted to 2 decimal places
// DONE: calibration parameters can be specified (combine with "-calibrate 0" to forcefully use them, otherwise will be used only if auto-calibration fails); the format is:  -calibration "scaleX,scaleY,scaleZ,offsetX,offsetY,offsetZ,tempX,tempY,tempZ,referenceTemp" (comma-, space- or semicolon- separated values, must use quotes if space-separated)
// DONE: Fixed that clipped input/output values were overestimated (limits assumed 2G data)
// DONE: SVM additional column to report the number of raw accelerometer samples (before interpolation).
// DONE: Check other output range scaling other than +/- 8g (for CWA & OMX)

// CONSIDER: Re-arranging to perform calibration accross all sessions and at native sampling rate (not interpolated)

// LATER: Sleep analysis algorithm (te Lindert et al 2013 in the journal SLEEP, volume 36, issue 5, page 781)
// LATER: Check what output we want with multiple 'sessions' - how to override for normal 7-day data (up to 1 week missing)?
// LATER: Determine and apply calibration even if accelerometer X/Y/Z is not first three output channels
// LATER: Other conversion scaling other than just 1/256 (e.g. for OMX)
// LATER: Other temperature conversions (OMX)



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

// Open Movement Converter Main Code
// Dan Jackson, 2014

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#define strcasecmp _stricmp
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "omconvert.h"
#include "omcalibrate.h"
#include "exits.h"


int main(int argc, char *argv[])
{
	int i;
	char help = 0;
	int positional = 0;
	int ret;
	omconvert_settings_t settings = { 0 };

#ifdef _WIN32
#if 0
	static char stdoutbuf[20];
	static char stderrbuf[20];
	setvbuf(stdout, stdoutbuf, _IOFBF, sizeof(2));
	setvbuf(stderr, stderrbuf, _IOFBF, sizeof(2));
#endif
#endif

	// Default settings
	memset(&settings, 0, sizeof(settings));
	settings.sampleRate = -1;
	settings.interpolate = 3;
	settings.auxChannel = 1;
	settings.headerCsv = -1;
	settings.calibrate = -1;
	settings.stationaryTime = 10.0;
	settings.defaultCalibration = (omcalibrate_calibration_t *)malloc(sizeof(omcalibrate_calibration_t));
	OmCalibrateInit(settings.defaultCalibration);
	settings.svmEpoch = 60;
	settings.svmFilter = -1;
	settings.svmMode = 0;
	settings.wtvEpoch = 1;	// measured in 30 minute windows
	settings.paeeEpoch = 1;	// measured in 1 minute windows
	settings.paeeFilter = -1;
	settings.paeeModel = "";	// default
	settings.agfilterEpoch = 1;
	settings.stepEpoch = 60;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0) { help = 1; }

		else if (strcmp(argv[i], "-out") == 0) { settings.outFilename = argv[++i]; }
		else if (strcmp(argv[i], "-resample") == 0) { settings.sampleRate = atof(argv[++i]); }
		else if (strcmp(argv[i], "-interpolate-mode") == 0) { settings.interpolate = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-aux-channel") == 0) { settings.auxChannel = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-info") == 0) { settings.infoFilename = argv[++i]; }
		else if (strcmp(argv[i], "-stationary") == 0) { settings.stationaryFilename = argv[++i]; }
		else if (strcmp(argv[i], "-header-csv") == 0) { settings.headerCsv = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-time") == 0) { settings.timeCsv = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-forceaccept") == 0) { settings.forceAccept = true; }

		else if (strcmp(argv[i], "-calibrate") == 0) { settings.calibrate = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-calibrate-repeated") == 0) { settings.repeatedStationary = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-calibrate-stationary") == 0) { settings.stationaryTime = atof(argv[++i]); }

		else if (strcmp(argv[i], "-calibration") == 0) {
			char *calibrationString = argv[++i];
			double values[10] = { 0 };
			int tokenCount = 0;
			const char *token;
			for (token = strtok(calibrationString, " ,;"); token != NULL; token = strtok(NULL, " ,;"), tokenCount++)
			{
				if (tokenCount < sizeof(values) / sizeof(values[0])) {
					values[tokenCount] = atof(token);
				}
			}

			if (tokenCount != sizeof(values) / sizeof(values[0]))
			{
				fprintf(stderr, "Cannot use calibration with invalid number of values (%d given but expected %d)\n", (int)tokenCount, (int)(sizeof(values) / sizeof(values[0])));
				help = 1;
			}
			else
			{
				settings.defaultCalibration->scale[0] = values[0];
				settings.defaultCalibration->scale[1] = values[1];
				settings.defaultCalibration->scale[2] = values[2];
				settings.defaultCalibration->offset[0] = values[3];
				settings.defaultCalibration->offset[1] = values[4];
				settings.defaultCalibration->offset[2] = values[5];
				settings.defaultCalibration->tempOffset[0] = values[6];
				settings.defaultCalibration->tempOffset[1] = values[7];
				settings.defaultCalibration->tempOffset[2] = values[8];
				settings.defaultCalibration->referenceTemperature = values[9];
			}
		}

		else if (strcmp(argv[i], "-csv-file") == 0) { settings.csvFilename = argv[++i]; }
		else if (strcmp(argv[i], "-csv-format") == 0) { settings.csvFormat = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-csv-format:accel") == 0) { settings.csvFormat = CSV_FORMAT_ACCEL; }
		else if (strcmp(argv[i], "-csv-format:ag") == 0) { settings.csvFormat = CSV_FORMAT_AG; }
		else if (strcmp(argv[i], "-csv-format:ga") == 0) { settings.csvFormat = CSV_FORMAT_GA; }
		else if (strcmp(argv[i], "-csv-format:agdt") == 0) { settings.csvFormat = CSV_FORMAT_AGDT; }

		else if (strcmp(argv[i], "-svm-file") == 0) { settings.svmFilename = argv[++i]; }
		else if (strcmp(argv[i], "-svm-epoch") == 0) { settings.svmEpoch = atof(argv[++i]); }
		else if (strcmp(argv[i], "-svm-filter") == 0) { settings.svmFilter = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-svm-mode") == 0) { settings.svmMode = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-svm-extended") == 0) { settings.svmExtended = atoi(argv[++i]); }

		else if (strcmp(argv[i], "-wtv-file") == 0) { settings.wtvFilename = argv[++i]; }
		else if (strcmp(argv[i], "-wtv-epoch") == 0) { settings.wtvEpoch = atoi(argv[++i]); }

		else if (strcmp(argv[i], "-paee-file") == 0) { settings.paeeFilename = argv[++i]; }
		else if (strcmp(argv[i], "-paee-model") == 0) { settings.paeeModel = argv[++i]; }
		else if (strcmp(argv[i], "-paee-epoch") == 0) { settings.paeeEpoch = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-paee-filter") == 0) { settings.paeeFilter = atoi(argv[++i]); }

		else if (strcmp(argv[i], "-sleep-file") == 0) { settings.sleepFilename = argv[++i]; }

		else if (strcmp(argv[i], "-counts-file") == 0) { settings.agfilterFilename = argv[++i]; }
		else if (strcmp(argv[i], "-counts-epoch") == 0) { settings.agfilterEpoch = atoi(argv[++i]); }

		else if (strcmp(argv[i], "-step-file") == 0) { settings.stepFilename = argv[++i]; }
		else if (strcmp(argv[i], "-step-epoch") == 0) { settings.stepEpoch = atoi(argv[++i]); }

		else if (argv[i][0] == '-')
		{
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			help = 1;
		}
		else
		{
			if (positional == 0)
			{
				settings.filename = argv[i];
			}
			else
			{
				fprintf(stderr, "Unknown positional parameter (%d): %s\n", positional + 1, argv[i]);
				help = 1;
			}
			positional++;
		}
	}


	if (settings.filename == NULL) { fprintf(stderr, "ERROR: Input file not specified.\n"); help = 1; }
	//if (settings.outFilename == NULL && settings.svmFilename == NULL) { fprintf(stderr, "ERROR: Output/SVM file not specified.\n"); help = 1; }

	if (help)
	{
		fprintf(stderr, "OMCONVERT\n");
		fprintf(stderr, "V1.22\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage: omconvert <filename.cwa> [<options>...]\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Where <options> are:\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-out <filename.wav>\n");
		fprintf(stderr, "\t-resample <rate (default from input configuration)>\n");
		fprintf(stderr, "\t-interpolate-mode <1=nearest, 2=linear, 3=cubic (default)>\n");
//		fprintf(stderr, "\t-aux-channel <0=ignore, 1=include (default)>\n");
		fprintf(stderr, "\t-info <filename.txt>\n");
		fprintf(stderr, "\t-stationary <filename.csv>\n");
		fprintf(stderr, "\t-header-csv <0=none, 1=header in first row (default)>\n");
		fprintf(stderr, "\t-time <0=absolute (default), 1=UNIX epoch>\n");
		fprintf(stderr, "\t-forceaccept");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-calibrate <0=off, 1=auto (default)>\n");	// 2=auto (force interpolator)
		fprintf(stderr, "\t-calibrate-repeated <0=include (default), 1=ignore>\n");
		fprintf(stderr, "\t-calibrate-stationary <time (default 10 seconds)>\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-csv-file <filename.csv>\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-svm-file <filename.svm.csv>\n");
		fprintf(stderr, "\t-svm-epoch <time (default 60 seconds)>\n");
		fprintf(stderr, "\t-svm-filter <0=off, 1=BP 0.5-20 Hz (default)>\n");
		fprintf(stderr, "\t-svm-extended <0=off (default), 1=zero invalid, 2=empty invalid, 3='nan' invalid>\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-wtv-file <filename.wtv.csv>\n");
		fprintf(stderr, "\t-wtv-epoch <number of 30-minute windows (default 1)>\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-paee-file <filename.paee.csv>\n");
		fprintf(stderr, "\t-paee-model <0=wrist, 1=right wrist, 2=left wrist, 3=waist>\n");
		fprintf(stderr, "\t-paee-epoch <minutes (default 1)>\n");
		fprintf(stderr, "\t-paee-filter <0=off, 1=BP 0.5-20 Hz (default)>\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-sleep-file <filename.sleep.csv>\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-counts-file <filename.counts.csv>\n");
		fprintf(stderr, "\t-counts-epoch <seconds (default 1)>\n");		
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-step-file <filename.step.csv>\n");
		fprintf(stderr, "\t-step-epoch <seconds (default 60)>\n");		
		fprintf(stderr, "\n");

		ret = EXIT_USAGE;
	}
	else
	{
		// Run converter
		ret = OmConvertRun(&settings);
	}

#if defined(_WIN32) && defined(_DEBUG)
	if (IsDebuggerPresent()) { fprintf(stderr, "\nPress [enter] to exit <%d>....", ret); getc(stdin); }
#endif

	return ret;
}

