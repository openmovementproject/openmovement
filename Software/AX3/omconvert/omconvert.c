// TODO
// SVM filtering in cut-points?
// Header in CSV files (perhaps metadata)
// FIX: Accelerometer range 
// TODO: Break this in to smaller parts and improve API (e.g. few more enums rather than special values).
// TODO: Add a player type for .WAV files
// TODO: Add a player type for raw data (and port the find-stationary-points algorithm to that, consider .WAV at the same time)


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

// Open Movement Converter
// Dan Jackson, 2014

#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#define timegm _mkgmtime
#else
	#define _BSD_SOURCE		// Both of these lines
	#include <features.h>	// ...needed for timegm() in time.h on Linux
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/timeb.h>

#include "omconvert.h"
#include "exits.h"
#include "omdata.h"
#include "omcalibrate.h"
#include "wav.h"

#define CONVERT_VERSION 1

// Calculations
#include "calc-csv.h"
#include "calc-svm.h"
#include "calc-wtv.h"
#include "calc-paee.h"
#include "calc-sleep.h"

// Calculation state
typedef struct
{
	// CSV
	csv_configuration_t csvConfiguration;
	csv_status_t csvStatus;
	int csvOk;

	// SVM
	svm_configuration_t svmConfiguration;
	svm_status_t svmStatus;
	int svmOk;

	// WTV
	wtv_configuration_t wtvConfiguration;
	wtv_status_t wtvStatus;
	int wtvOk;

	// PAEE
	paee_configuration_t paeeConfiguration;
	paee_status_t paeeStatus;
	int paeeOk;

	// Sleep
	sleep_configuration_t sleepConfiguration;
	sleep_status_t sleepStatus;
	int sleepOk;

	// Overall stats
	int countInvalid;
	int countClipped;
	int countClippedInput;
	int countClippedOutput;

} calc_t;


static void CalcCreate(calc_t *calc, omconvert_settings_t *settings)
{
	// Clear
	memset(calc, 0, sizeof(calc_t));

	// CSV status
	memset(&calc->csvConfiguration, 0, sizeof(csv_configuration_t));
	calc->csvConfiguration.headerCsv = settings->headerCsv;
	calc->csvConfiguration.filename = settings->csvFilename;

	// SVM status
	memset(&calc->svmConfiguration, 0, sizeof(svm_configuration_t));
	calc->svmConfiguration.headerCsv = settings->headerCsv;
	calc->svmConfiguration.filename = settings->svmFilename;
	calc->svmConfiguration.filter = settings->svmFilter;
	calc->svmConfiguration.mode = settings->svmMode;
	calc->svmConfiguration.epoch = settings->svmEpoch;
	calc->svmConfiguration.extended = settings->svmExtended;

	// WTV status
	memset(&calc->wtvConfiguration, 0, sizeof(wtv_configuration_t));
	calc->wtvConfiguration.headerCsv = settings->headerCsv;
	calc->wtvConfiguration.filename = settings->wtvFilename;
	calc->wtvConfiguration.halfHourEpochs = settings->wtvEpoch;

	// PAEE status
	memset(&calc->paeeConfiguration, 0, sizeof(paee_configuration_t));
	calc->paeeConfiguration.headerCsv = settings->headerCsv;
	calc->paeeConfiguration.filename = settings->paeeFilename;
	if (settings->paeeCutPoints == 0) { calc->paeeConfiguration.cutPoints = paeeCutPointWrist; }
	else if (settings->paeeCutPoints == 1) { calc->paeeConfiguration.cutPoints = paeeCutPointWristR; }
	else if (settings->paeeCutPoints == 2) { calc->paeeConfiguration.cutPoints = paeeCutPointWristL; }
	else if (settings->paeeCutPoints == 3) { calc->paeeConfiguration.cutPoints = paeeCutPointWaist; }
	calc->paeeConfiguration.minuteEpochs = settings->paeeEpoch;
	calc->paeeConfiguration.filter = settings->paeeFilter;

	// Sleep status
	memset(&calc->sleepConfiguration, 0, sizeof(sleep_configuration_t));
	calc->sleepConfiguration.headerCsv = settings->headerCsv;
	calc->sleepConfiguration.timeCsv = settings->timeCsv;
	calc->sleepConfiguration.filename = settings->sleepFilename;

	return;
}

static int CalcInit(calc_t *calc, double sampleRate, double startTime)
{
	int ok = 0;

	// Do not clear structure here, this is done in CalcCreate()

	// Init. CSV
	memset(&calc->csvStatus, 0, sizeof(csv_status_t));
	calc->csvConfiguration.sampleRate = sampleRate;
	calc->csvConfiguration.startTime = startTime;
	calc->csvOk = CsvInit(&calc->csvStatus, &calc->csvConfiguration);
	ok |= calc->csvOk;

	// Init. SVM
	memset(&calc->svmStatus, 0, sizeof(svm_status_t));
	calc->svmConfiguration.sampleRate = sampleRate;
	calc->svmConfiguration.startTime = startTime;
	calc->svmOk = SvmInit(&calc->svmStatus, &calc->svmConfiguration);
	ok |= calc->svmOk;

	// Init. WTV
	memset(&calc->wtvStatus, 0, sizeof(wtv_status_t));
	calc->wtvConfiguration.sampleRate = sampleRate;
	calc->wtvConfiguration.startTime = startTime;
	calc->wtvOk = WtvInit(&calc->wtvStatus, &calc->wtvConfiguration);
	ok |= calc->wtvOk;

	// Init. PAEE
	memset(&calc->paeeStatus, 0, sizeof(paee_status_t));
	calc->paeeConfiguration.sampleRate = sampleRate;
	calc->paeeConfiguration.startTime = startTime;
	calc->paeeOk = PaeeInit(&calc->paeeStatus, &calc->paeeConfiguration);
	ok |= calc->paeeOk;

	// Init. Sleep
	memset(&calc->sleepStatus, 0, sizeof(sleep_status_t));
	calc->sleepConfiguration.sampleRate = sampleRate;
	calc->sleepConfiguration.startTime = startTime;
	calc->sleepOk = SleepInit(&calc->sleepStatus, &calc->sleepConfiguration);
	ok |= calc->sleepOk;


	// Clear stats
	calc->countInvalid = 0;
	calc->countClippedInput = 0;
	calc->countClippedOutput = 0;
	calc->countClipped = 0;

	return ok;		// Whether any processing outputs are used
}


static bool CalcAddValue(calc_t *calc, double* accel, double temp, char validity, int rawIndex)
{
	bool ok = true;
	bool valid = (validity & 1) ? false : true;		 // Valid if not invalid(!)

	if (calc->svmOk) { ok &= SvmAddValue(&calc->svmStatus, accel, temp, validity, rawIndex); }
	if (calc->wtvOk) { ok &= WtvAddValue(&calc->wtvStatus, accel, temp, valid); }
	if (calc->paeeOk) { ok &= PaeeAddValue(&calc->paeeStatus, accel, temp, valid); }
	if (calc->csvOk) { ok &= CsvAddValue(&calc->csvStatus, accel, temp, valid); }
	if (calc->sleepOk) { ok &= SleepAddValue(&calc->sleepStatus, accel, temp, valid); }

	// Overall stats
	if (validity & 0x01) { calc->countInvalid++; }
	if (validity & 0x02) { calc->countClippedInput++; }
	if (validity & 0x04) { calc->countClippedOutput++; }
	if ((validity & 0x02) || (validity & 0x04)) { calc->countClipped++; }

	return ok;
}


static void CalcClose(calc_t *calc)
{
	if (calc->svmOk) { SvmClose(&calc->svmStatus); }
	if (calc->wtvOk) { WtvClose(&calc->wtvStatus); }
	if (calc->paeeOk) { PaeeClose(&calc->paeeStatus); }
	if (calc->csvOk) { CsvClose(&calc->csvStatus); }
	if (calc->sleepOk) { SleepClose(&calc->sleepStatus); }
}


/*
The comment format is modelled on (and a compatible subset of) other key-value pair formats such as common internet headers (HTTP/SMTP), YAML and Java properties files.

Device (Artist "IART" WAV chunk): to specify metadata about the device that made the recording: device id, device type, revision, firmware version.
Id: 1234
Device: CWA
Revision: 17
Firmware: 44

Recording (Title "INAM" WAV chunk): to specify metadata about the recording itself: session identifier, recording start time, recording stop time, sensor sample rate and sensitivity, user metadata
Session: 1
Start: 2014-08-01 10:00:00
Stop: 2014-12-08 10:00:00
Rate-A: 100,8
Metadata:

Formatting (Comment "ICMT" WAV chunk): to specify metadata about this WAV file: time of the first sample, channel mapping, ranges
Time: 2014-08-01 10:00:00.000
Channel-1: Accel-X
Scale-1: 8
Channel-2: Accel-Y
Scale-2: 8
Channel-3: Accel-Z
Scale-3: 8
Channel-4: Aux

Creation date ("ICRD" WAV chunk): to specify the time of the first sample (this is also in the comment as Matlab doesn't read the ICRD chunk)
2014-08-01 10:00:00.000

*/

//
// Auxilliary channel format:  ncttttuu vvvvvvvv   
//                             n=0:   data available
//                             n=1:   data not available
//                             c=0:   no data channels clipped
//                             c=1:   some data channels clipped       [reserve nc=11, perhaps for in-data scaling information?]
//                             tttt=0000: metadata:
//                                      uu=00: other comment
//                                      uu=01: 'artist' file metadata
//                                      uu=10: 'title' file metadata
//                                      uu=11: 'comment' file metadata
//                             tttt=0001: sensor - battery (10 bits, u+v)       [reserve tttt=01uu, perhaps for 12-bit battery]
//                             tttt=0010: sensor - light (10 bits, u+v)         [reserve tttt=10uu, perhaps for 12-bit light]
//                             tttt=0011: sensor - temperature (10 bits, u+v)   [reserve tttt=11uu, perhaps for 12-bit temperature]
//

#define WAV_AUX_UNAVAILABLE			0x8000		// u------- --------    Data not available on one or more channels (1/0)
#define WAV_AUX_CLIPPING			0x4000		// -c------ --------    Data clipped on one or more channels (1/0)
#define WAV_AUX_METADATA_OTHER		0x0000		// --000000 vvvvvvvv    Metadata - other comment
#define WAV_AUX_METADATA_ARTIST		0x0100		// --000001 vvvvvvvv    Metadata - artist
#define WAV_AUX_METADATA_TITLE		0x0200		// --000010 vvvvvvvv    Metadata - title
#define WAV_AUX_METADATA_COMMENT	0x0300		// --000011 vvvvvvvv    Metadata - comment
#define WAV_AUX_SENSOR_BATTERY		0x0400		// --0001vv vvvvvvvv    Sensor - battery
#define WAV_AUX_SENSOR_LIGHT		0x0800		// --0010vv vvvvvvvv    Sensor - light
#define WAV_AUX_SENSOR_TEMPERATURE	0x0c00		// --0011vv vvvvvvvv    Sensor - temperature



// Desired mapping of channels
static om_convert_channel_t defaultChannelPriority[] =
{
	//{ 'x', 0 }, { 'x', 1 }, { 'x', 2 }, { 'x', 3 }, { 'x', 4 }, { 'x', 5 }, { 'x', 6 }, { 'x', 7 }, { 'x', 8 }, // "All-axis" channels
	{ 'a', 0 }, { 'a', 1 }, { 'a', 2 },		// Accelerometer
	{ 'g', 0 }, { 'g', 1 }, { 'g', 2 },		// Gyroscope
	{ 'm', 0 }, { 'm', 1 }, { 'm', 2 },		// Magnetometer
	{ 0, 0 }								// END
};




static double NearestInterpolate(double v1, double v2, double x)
{
	return (x < 0.5) ? v1 : v2;
}

static double LinearInterpolate(double v1, double v2, double x)
{
	return ((v2 - v1) * x) + v1;
}

static double CubicInterpolate(double v0, double v1, double v2, double v3, double x)
{
	// v0 = point before A, v1 = point A, v2 = point B, v3 = point after B, x = position between A/B.
	double p = (v3 - v2) - (v0 - v1);
	double q = (v0 - v1) - p;
	double r = v2 - v0;
	double s = v1;
	return (p * x * x * x) + (q * x * x) + (r * x) + s;
}





void InterpolatorInit(interpolator_t *interpolator, char mode, omdata_t *data, omdata_session_t *session, int streamIndex)
{
	memset(interpolator, 0, sizeof(interpolator_t));
	interpolator->mode = mode;
	interpolator->data = data;
	interpolator->streamIndex = streamIndex;
	interpolator->seg = session->stream[streamIndex].segmentFirst;
	interpolator->timeIndex = -1;
	interpolator->scale = 0;
	if (interpolator->seg != NULL)
	{
		interpolator->scale = interpolator->seg->scaling;
	}
}

// TODO: Currently this can only advance forwards (never backwards) -- as it's sorted, we could do a binary search for the nearest time
void InterpolatorSeek(interpolator_t *interpolator, double t)
{
	interpolator->clipped = false;
	interpolator->valid = true;

	// Skip segment if needed
	while (interpolator->seg != NULL && t > interpolator->seg->endTime)
	{
		interpolator->previousSegmentSamples += interpolator->seg->numSamples;
		interpolator->seg = interpolator->seg->segmentNext;
		interpolator->timeIndex = -1;
		if (interpolator->seg != NULL)
		{
			interpolator->scale = interpolator->seg->scaling;
		}
	}

	if (interpolator->seg != NULL && t >= interpolator->seg->startTime)
	{
		// Skip time indices if needed
		while (interpolator->timeIndex + 1 < interpolator->seg->timestampCount && t >= interpolator->seg->timestamps[interpolator->timeIndex + 1].timestamp)
		{
			interpolator->timeIndex++;
		}

		// Check we're between two time indices
		if (interpolator->seg->numSamples > 0)
		{
			int i1, i2;
			double t1, t2;

			if (interpolator->timeIndex >= 0 && interpolator->timeIndex < interpolator->seg->timestampCount)
			{
				i1 = interpolator->seg->timestamps[interpolator->timeIndex].sample;
				t1 = interpolator->seg->timestamps[interpolator->timeIndex].timestamp;
			}
			else
			{
				i1 = 0;
				t1 = interpolator->seg->startTime;
			}

			if (interpolator->timeIndex + 1 < interpolator->seg->timestampCount)
			{
				i2 = interpolator->seg->timestamps[interpolator->timeIndex + 1].sample;
				t2 = interpolator->seg->timestamps[interpolator->timeIndex + 1].timestamp;
			}
			else
			{
				i2 = interpolator->seg->numSamples - 1;
				t2 = interpolator->seg->endTime;
			}

			// Interpolate the timestamps here (don't think cubic is valid as the source points should be equidistant for that to be valid, so use linear)
			double timeProp = (t2 - t1) != 0.0 ? (t - t1) / (t2 - t1) : 0.0;		// Linear interpolate in time (see note above)
			double index = (i2 - i1) * timeProp + i1;	// Fractional index at that position
			interpolator->sampleIndex = (int)index;		// Actual index (v1)
			interpolator->prop = index - (int)index;	// Offset towards next value (v2)

//printf(">>> %f => %d . %f\n", index, interpolator->sampleIndex, interpolator->prop);

			// Have we got enough for (-1, 0, 1, 2)?
			int idx[4];

			idx[1] = interpolator->sampleIndex;				// v1 (@0)

			if (interpolator->sampleIndex >= 1)
			{
				idx[0] = interpolator->sampleIndex - 1;		// v0 (@-1)
			}
			else
			{
				idx[0] = idx[1];
			}

			if (interpolator->sampleIndex + 1 < interpolator->seg->numSamples)
			{
				idx[2] = interpolator->sampleIndex + 1;		// v2 (@1)
			}
			else
			{
				idx[2] = idx[1];
			}

			if (interpolator->sampleIndex + 2 < interpolator->seg->numSamples)
			{
				idx[3] = interpolator->sampleIndex + 2;		// v3 (@2)
			}
			else
			{
				idx[3] = idx[2];
			}


			// For each index (-1, 0, 1, 2), cache the underlying values (for the interpolator to work over)
			int z;
			for (z = 0; z < 4; z++)
			{
				char clipped = OmDataGetValues(interpolator->data, interpolator->seg, idx[z], interpolator->values[z]);
				if (z == 1 || z == 2) { interpolator->clipped |= clipped; }
			}

			return;
		}
	}

	// Invalid
	interpolator->valid = false;
}

double InterpolatorValue(interpolator_t *interpolator, int subchannel, char *valid)
{
	// Check for invalid
	if (!interpolator->valid || interpolator->seg == NULL || subchannel < 0 || subchannel >= interpolator->seg->channels)
	{
		if (valid != NULL) { 
			*valid = 0; 
		}
		return 0.0;
	}

	//if (t >= interpolator->seg->startTime)
	double val;
	
	if (interpolator->mode == 1) { val = NearestInterpolate(interpolator->values[1][subchannel], interpolator->values[2][subchannel], interpolator->prop); }
	else if (interpolator->mode == 2) { val = LinearInterpolate(interpolator->values[1][subchannel], interpolator->values[2][subchannel], interpolator->prop); }
	else if (interpolator->mode == 3) { val = CubicInterpolate(interpolator->values[0][subchannel], interpolator->values[1][subchannel], interpolator->values[2][subchannel], interpolator->values[3][subchannel], interpolator->prop); }
	else { val = 0.0; }

	if (valid != NULL) { *valid = 1; }

	return val;
}

// Returns the number of seconds since the epoch
double TimeNow()
{
	struct timeb tp;
	ftime(&tp);
	return ((unsigned long long)tp.time * 1000 + tp.millitm) / 1000.0;
}

#define MAX_TIME_STRING 26
const char *TimeString(double t, char *buff)
{
	static char staticBuffer[MAX_TIME_STRING] = { 0 };	// 2000-01-01 20:00:00.000|
	if (buff == NULL) { buff = staticBuffer; }
	time_t tn = (time_t)t;
	struct tm *tmn = gmtime(&tn);
	float sec = tmn->tm_sec + (float)(t - (time_t)t);
	sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec, (int)((sec - (int)sec) * 1000));
	return buff;
}







int OmConvertFindArrangement(om_convert_arrangement_t *arrangement, omconvert_settings_t *settings, omdata_t *omdata, omdata_session_t *session, om_convert_channel_t *channelPriority)
{
	memset(arrangement, 0, sizeof(om_convert_arrangement_t));
	arrangement->data = omdata;
	arrangement->session = session;
	arrangement->numChannels = 0;
	arrangement->numStreamIndexes = 0;
	arrangement->defaultRate = 1;

	// TODO: If the session contains an 'all axis' (synchronous) source, use that for timing for those regions (however, need to patch any holes and before/after values)

	// Determine the stream ordering
	arrangement->defaultRate = 1;
	int cpi;
	for (cpi = 0; channelPriority[cpi].stream != 0; cpi++)
	{
		int streamIndex = channelPriority[cpi].stream;
		int subchannel = channelPriority[cpi].subchannel;

//fprintf(stderr, "DEBUG: Finding stream %c:%d.\n", streamIndex, subchannel); 

		if (streamIndex < 0 || streamIndex >= OMDATA_MAX_STREAM) { fprintf(stderr, "WARNING: Ignoring stream index %d (0x%02x = %c).\n", streamIndex, streamIndex, streamIndex); continue; }
		if (subchannel < 0 || subchannel > 9) { fprintf(stderr, "WARNING: Ignoring invalid sub-channel index %d.\n", subchannel); continue; }

		// Check each session
		omdata_stream_t *stream = &arrangement->session->stream[streamIndex];
		if (!stream->inUse) { fprintf(stderr, "DEBUG: Stream %c not found...\n", streamIndex); continue; }
		if (!stream->segmentFirst || !stream->segmentLast) { fprintf(stderr, "DEBUG: Stream %c not first/last segment.\n", streamIndex); continue; }

		// Check each segment for the maximum number of sub-channels
		omdata_segment_t *seg;
		int numSubChannels = 0;
		for (seg = stream->segmentFirst; seg != NULL; seg = seg->segmentNext)
		{
			if (seg->channels > numSubChannels) { numSubChannels = seg->channels; }
			if (seg->sampleRate > arrangement->defaultRate) { arrangement->defaultRate = seg->sampleRate; }
		}

		// Check we have the subchannel
		if (subchannel >= numSubChannels) { fprintf(stderr, "DEBUG: Stream %c cannot use sub-channel index %d (has %d sub-channels).\n", streamIndex, subchannel, numSubChannels); continue; }
		
		// Check not too many channels
		if (arrangement->numChannels >= OMDATA_MAX_CHANNELS)
		{
			fprintf(stderr, "DEBUG: Stream %c:%d cannot be assigned as exceeded %d maximum number of channels.\n", streamIndex, subchannel, OMDATA_MAX_CHANNELS); 
			continue;
		}

		// Assign channel
		arrangement->channelAssignment[arrangement->numChannels].stream = streamIndex;
		arrangement->channelAssignment[arrangement->numChannels].subchannel = subchannel;

		// Track which streams need updating
		bool found = false;
		int j;
		for (j = 0; j < arrangement->numStreamIndexes; j++)
		{
			if (arrangement->streamIndexes[j] == streamIndex) { found = true; break; }
		}
		if (!found && arrangement->numStreamIndexes < OMDATA_MAX_CHANNELS)
		{
			arrangement->streamIndexes[arrangement->numStreamIndexes] = streamIndex;
			arrangement->numStreamIndexes++;
		}


//		fprintf(stderr, "DEBUG: Output channel %d is stream '%c' channel %d...\n", arrangement->numChannels, streamIndex, subchannel);

		arrangement->numChannels++;
	}

	arrangement->startTime = session->startTime;
	arrangement->endTime = session->endTime;

	// Clamp to record time if close
	double limit = 15.000;
	if (omdata->metadata.recordingStart < omdata->metadata.recordingStop && (omdata->metadata.recordingStop - omdata->metadata.recordingStart) > 2 * limit)
	{
		if (fabs(arrangement->startTime - omdata->metadata.recordingStart) < limit) 
		{ 
			arrangement->startTime = omdata->metadata.recordingStart; 
			fprintf(stderr, "Clamping session to recording start time (%s)...\n", TimeString(omdata->metadata.recordingStart, NULL)); 
		}
		if (fabs(arrangement->endTime - omdata->metadata.recordingStop) < limit)
		{
			arrangement->endTime = omdata->metadata.recordingStop; 
			fprintf(stderr, "Clamping session to recording stop time (%s)...\n", TimeString(omdata->metadata.recordingStop, NULL));
		}
	}

	arrangement->duration = arrangement->endTime - arrangement->startTime;

	return 0;
}


void OmConvertPlayerInitialize(om_convert_player_t *player, om_convert_arrangement_t *arrangement, double sampleRate, char interpolate)
{
	omdata_session_t *session = arrangement->session;
	player->arrangement = arrangement;

	player->interpolate = interpolate;

	// If not interpolating...
	if (player->interpolate < 0)
	{
		player->sampleRate = arrangement->defaultRate;
		player->numSamples = -1;
		fprintf(stderr, "ERROR: Non-interpolated player not supported.\n");
	}
	else
	{
		// Calculate number of samples
		if (sampleRate > 0)
		{
			player->sampleRate = sampleRate;
		}
		else
		{
			player->sampleRate = arrangement->defaultRate;
		}

		player->numSamples = (int)(arrangement->duration * player->sampleRate + 0.5);
	}

	fprintf(stderr, "DEBUG: Session between t0=%f, t1=%f  ==>  %f seconds at %f Hz  ==> %d samples * %d channels\n", arrangement->startTime, arrangement->endTime, arrangement->duration, player->sampleRate, player->numSamples, arrangement->numChannels);

	// Start tracking each stream with an omdata_interpolator_t over each segment... (tracks up to four different index-timestamp pairs, advance converts time a fractional index point, then sample each sub-channel at that point)
	int j;
	for (j = 0; j < arrangement->numStreamIndexes; j++)
	{
		int si = arrangement->streamIndexes[j];
		InterpolatorInit(&player->segmentInterpolators[si], player->interpolate, arrangement->data, session, si);
	}

	// ADC interpolator
	InterpolatorInit(&player->adcInterpolator, 3, arrangement->data, session, 'l');


	// Record the scale
	int c;
	for (c = 0; c < player->arrangement->numChannels; c++)
	{
		int si = player->arrangement->channelAssignment[c].stream;
		//int subchannel = player->arrangement->channelAssignment[c].subchannel;
		player->scale[c] = player->segmentInterpolators[si].scale;
	}

	return;
}

int OmConvertPlayerRawIndexWithinSegment(om_convert_player_t *player, char channel)
{
	// Update the interpolator for each stream to the current time
	int j;
	for (j = 0; j < player->arrangement->numStreamIndexes; j++)
	{
		int si = player->arrangement->streamIndexes[j];
		if (si == channel)
		{
			return player->segmentInterpolators[si].previousSegmentSamples + player->segmentInterpolators[si].sampleIndex;
		}
	}
	return -1;
}

void OmConvertPlayerSeek(om_convert_player_t *player, int sample)
{
	double t = player->arrangement->startTime + (sample / player->sampleRate);

	// Update the interpolator for each stream to the current time
	int j;
	for (j = 0; j < player->arrangement->numStreamIndexes; j++)
	{
		int si = player->arrangement->streamIndexes[j];
		InterpolatorSeek(&player->segmentInterpolators[si], t);
	}
	InterpolatorSeek(&player->adcInterpolator, t);

	// Sample the sub-channels
	int c;
	player->valid = 1;
	player->clipped = 0;
	for (c = 0; c < player->arrangement->numChannels; c++)
	{
		int si = player->arrangement->channelAssignment[c].stream;
		int subchannel = player->arrangement->channelAssignment[c].subchannel;
		char valid = 0;
		double val = InterpolatorValue(&player->segmentInterpolators[si], subchannel, &valid);
		player->values[c] = val;
		player->valid &= valid;
		player->clipped |= player->segmentInterpolators[si].clipped;
	}

	// Aux channel
	int z;
	for (z = 0; z < 3; z++)
	{
		player->aux[z] = (short)(InterpolatorValue(&player->adcInterpolator, z, NULL));
	}

	// TODO: Cope with other temperature conversions
	player->temp = ((int)player->aux[2] * 150 - 20500) / 1000.0;
	//player->temp = (double)player->aux[2] * 75 / 256.0 - 50;

	return;
}

double ParseTime(char *tstr)
{
	int index = 0;
	char *token = NULL;
	char *p;
	char end = 0;
	struct tm tm0 = { 0 };
	int milli = 0;

	memset(&tm0, 0, sizeof(tm0));

	for (p = tstr; !end; p++)
	{
		end = (*p == '\0');
		if (token == NULL && *p >= '0' && *p <= '9')
		{
			token = p;
		}
		if (token != NULL && (*p < '0' || *p > '9'))
		{
			int value;
			*p = '\0';
			value = atoi(token);
			token = NULL;

			if (index == 0) { tm0.tm_year = value - 1900; }		// since 1900
			else if (index == 1) { tm0.tm_mon = value - 1; }	// 0-11
			else if (index == 2) { tm0.tm_mday = value; }		// 1-31
			else if (index == 3) { tm0.tm_hour = value; }		// 
			else if (index == 4) { tm0.tm_min = value; }		// 
			else if (index == 5) { tm0.tm_sec = value; }		// 
			else if (index == 6) { milli = value; }				// 

			index++;
		}
	}

	if (index < 6) { return 0; }
	double t = (double)timegm(&tm0);
	t += milli / 1000.0;

	return t; 
}




int OmConvertRunWav(omconvert_settings_t *settings, calc_t *calc)
{
	int retVal = EXIT_OK;
	int i;
	FILE *fp;
	WavInfo wavInfo = { 0 };
	char infoArtist[WAV_META_LENGTH] = { 0 };
	char infoName[WAV_META_LENGTH] = { 0 };
	char infoComment[WAV_META_LENGTH] = { 0 };
	char infoDate[WAV_META_LENGTH] = { 0 };
	#define MAX_CHANNELS (3 + 3 + 3 + 4 + 1)
	double scale[MAX_CHANNELS] = { 8.0 / 32768.0, 8.0 / 32768.0, 8.0 / 32768.0 };

	// Check config.
	if (settings->outFilename != NULL) { fprintf(stderr, "ERROR: Cannot output to WAV when input is WAV.\n"); return EXIT_CONFIG; }
	if (settings->infoFilename != NULL) { fprintf(stderr, "ERROR: Cannot output to info file when input is WAV.\n"); return EXIT_CONFIG; }
	if (settings->stationaryFilename != NULL) { fprintf(stderr, "ERROR: Cannot output to stationary points file when input is WAV.\n"); return EXIT_CONFIG; }

	fp = fopen(settings->filename, "rb");
	if (fp == NULL) { fprintf(stderr, "ERROR: Cannot open WAV file.\n"); return EXIT_NOINPUT; }

	wavInfo.infoArtist = infoArtist;
	wavInfo.infoName = infoName;
	wavInfo.infoComment = infoComment;
	wavInfo.infoDate = infoDate;

	if (!WavRead(&wavInfo, fp)) { fprintf(stderr, "ERROR: Problem reading WAV file format.\n"); fclose(fp); return EXIT_DATAERR; }
	if (wavInfo.bytesPerChannel != 2) { fprintf(stderr, "ERROR: WAV file format not supported (%d bytes/channel, expected 2).\n", wavInfo.bytesPerChannel); fclose(fp); return EXIT_DATAERR; }
	if (wavInfo.chans < 3 || wavInfo.chans > 16) { fprintf(stderr, "ERROR: WAV file format not supported (%d channels, expected at least 3 and no more than 16).\n", wavInfo.chans); fclose(fp); return EXIT_DATAERR; }
	if (wavInfo.freq < 1 || wavInfo.freq > 48000) { fprintf(stderr, "ERROR: WAV file format not supported (%d frequency).\n", wavInfo.freq); fclose(fp); return EXIT_DATAERR; }

	// Extract metadata
	char *line;
	#define MAX_FIELDS 32
	//char *artistLines[MAX_FIELDS]; int numArtistLines = 0;
	//for (line = strtok(wavInfo.infoArtist, "\n"); line != NULL; line = strtok(NULL, "\n")) { if (numArtistLines < MAX_FIELDS) { artistLines[numArtistLines++] = line; } }
	//char *nameLines[MAX_FIELDS]; int numNameLines = 0;
	//for (line = strtok(wavInfo.infoName, "\n"); line != NULL; line = strtok(NULL, "\n")) { if (numNameLines < MAX_FIELDS) { nameLines[numNameLines++] = line; } }
	char *commentLines[MAX_FIELDS]; int numCommentLines = 0;
	for (line = strtok(wavInfo.infoComment, "\n"); line != NULL; line = strtok(NULL, "\n")) { if (numCommentLines < MAX_FIELDS) { commentLines[numCommentLines++] = line; } }

	// Parse headers
	bool parsedTime = false;
	bool parsedScale[MAX_CHANNELS] = { 0 };
	double startTime = 0;
	for (i = 0; i < numCommentLines; i++)
	{
		if (strncmp(commentLines[i], "Time:", 5) == 0) 
		{
			startTime = ParseTime(commentLines[i] + 5);
			fprintf(stderr, "Time: %s\n", TimeString(startTime, NULL));
			if (startTime > 0) { parsedTime = true; }
		}
		else if (strncmp(commentLines[i], "Scale-", 6) == 0 && (commentLines[i][6] >= '1' && commentLines[i][6] <= '9') && commentLines[i][7] == ':')
		{
			int chan = commentLines[i][6] - '1';
			double val = atof(commentLines[i] + 8);
			scale[chan] = val / 32768.0;
			fprintf(stderr, "Scale-%d: %f (scale[%d] = %f)\n", chan + 1, val, chan, scale[chan]);
			if (scale[chan] > 0) { parsedScale[chan] = true; }
		}
	}

	// Check we parsed the headers we need
	if (!parsedTime) { fprintf(stderr, "WARNING: Didn't successfully parse a 'Time' header (using zero).\n"); }
	for (i = 0; i < 3; i++)
	{
		if (!parsedScale[i]) { fprintf(stderr, "WARNING: Didn't successfully parse a 'Scale-%d' header (using defaults).\n", i + 1); }
	}

	int bufferSamples = wavInfo.freq * 60 * 60;	// 1 hour buffer
	short *buffer = malloc(sizeof(short) * wavInfo.chans * bufferSamples);
	if (buffer == NULL) { fprintf(stderr, "ERROR: Problem allocating buffer for WAV file (%d samples).\n", bufferSamples); fclose(fp); return EXIT_SOFTWARE; }

	// Init. CSV/SVM/WTV/PAEE
	int outputOk = CalcInit(calc, wavInfo.freq, startTime);
	if (!outputOk)
	{
		fprintf(stderr, "ERROR: No outputs.\n");
		retVal = EXIT_CONFIG;
	}
	else
	{
		fprintf(stderr, "Working...\n");
		unsigned long samplesOffset = 0;
		unsigned long samplesRemaining = wavInfo.numSamples;
		double temp = 0.0;
		while (!feof(fp))
		{
			long offset = wavInfo.offset + ((sizeof(short) * wavInfo.chans) * samplesOffset);
			unsigned long samplesToRead = bufferSamples;
			if (samplesToRead > samplesRemaining) { samplesToRead = samplesRemaining; }
			if (samplesToRead <= 0) { break; }
			fseek(fp, offset, SEEK_SET);
			unsigned long samplesRead = fread(buffer, sizeof(short) * wavInfo.chans, samplesToRead, fp);
			if (samplesRead <= 0) { break; }
			samplesOffset += samplesRead;
			samplesRemaining -= samplesRead;

			double values[3];
			for (i = 0; i < (int)samplesRead; i++)
			{
				const short *v = buffer + i * wavInfo.chans;

				// Auxilliary channel is last channel
				int validity = 0;
				if (wavInfo.chans > 3)
				{
					uint16_t aux = v[wavInfo.chans - 1];
					if (aux & WAV_AUX_UNAVAILABLE) { validity |= 0x01; }		// Invalid sample
					if (aux & WAV_AUX_CLIPPING)
					{ 
						if (v[0] > -32768 && v[0] < 32767 && v[1] > -32768 && v[1] < 32767 && v[2] > -32768 && v[2] < 32767)
						{
							validity |= 0x02;	// Not clipped after calibration, must have been clipped before calibration
						}
						else
						{
							validity |= 0x04;	// Clipped after calibration, may have also been clipped before calibration.
						}
					}
					// TODO: Update temperature 'temp'
				}

				// Scaling from metadata
				values[0] = v[0] * scale[0];
				values[1] = v[1] * scale[1];
				values[2] = v[2] * scale[2];
				if (!CalcAddValue(calc, values, temp, validity, samplesOffset + i))
				{
					fprintf(stderr, "ERROR: Problem writing calculations.\n");
					retVal = EXIT_IOERR;
					break;
				}
			}
		}
	}

	free(buffer);
	fclose(fp);

	CalcClose(calc);

	return retVal;
}


int OmConvertRunConvert(omconvert_settings_t *settings, calc_t *calc)
{
	int retVal = EXIT_OK;
	omdata_t omdata = { 0 };
	om_convert_arrangement_t arrangement = { 0 };

	if (!settings->calibrate && settings->stationaryFilename != NULL) { fprintf(stderr, "ERROR: Cannot output to stationary points file when not auto-calibrating.\n"); return EXIT_CONFIG; }

	// Output information file
	FILE *infofp = NULL;
	if (settings->infoFilename != NULL)
	{
		infofp = fopen(settings->infoFilename, "wt");
		if (infofp == NULL)
		{
			fprintf(stderr, "ERROR: Cannot open output information file: %s\n", settings->infoFilename);
			return EXIT_CANTCREAT;
		}
	}
		
	// Load input data
	if (!OmDataLoad(&omdata, settings->filename))
	{
		const char *msg = "ERROR: Problem loading file.\n";
		fprintf(stderr, "%s", msg);
		fprintf(stdout, "%s", msg);
		return EXIT_DATAERR;
	}
	fprintf(stderr, "Data loaded!\n");

	OmDataDump(&omdata);


	// Calibration configuration
	omcalibrate_config_t calibrateConfig = { 0 };
	OmCalibrateConfigInit(&calibrateConfig);
	calibrateConfig.stationaryTime = settings->stationaryTime; // 10.0;
	calibrateConfig.stationaryRepeated = settings->repeatedStationary;
	bool doneCalibration = false;
	double errorBeforeCalibration = 0.0;
	double errorAfterCalibration = 0.0;
	double stationaryMin[3] = { 0 }, stationaryMax[3] = { 0 };
	int stationaryCount = 0;

	// Initialize identity calibration
	omcalibrate_calibration_t calibration;
	OmCalibrateCopy(&calibration, settings->defaultCalibration);

	// For each session:
	omdata_session_t *session;
	int sessionCount = 0;
	for (session = omdata.firstSession; session != NULL; session = session->sessionNext)
	{
		sessionCount++;
		fprintf(stderr, "=== SESSION %d ===\n", sessionCount);

		if (sessionCount > 1)
		{
			fprintf(stderr, "NOTE: Skipping session %d...\n", sessionCount);
			continue;
		}

		// Find a configuration
		OmConvertFindArrangement(&arrangement, settings, &omdata, session, defaultChannelPriority);

		// Calibrate now?
		if (!doneCalibration && settings->calibrate)
		{
			doneCalibration = true;

			// Find stationary points
			// - If this is a CWA file with co-located temperature and accelerometer readings, use the data directly,
			// - otherwise, use a 'player' to interpolate over the data.
			omcalibrate_stationary_points_t *stationaryPoints;
			bool calibrateFromData = (settings->calibrate != 0 && settings->calibrate != 2);
			if (calibrateFromData && (!omdata.stream['a'].inUse || omdata.stream['a'].segmentFirst->offset != 30))
			{
				calibrateFromData = false;
				fprintf(stderr, "NOTE: Calibration requested directly from data, but an interpolater must be used instead.\n");
			}

			if (calibrateFromData)
			{
				fprintf(stderr, "Finding stationary points from data...\n");
				stationaryPoints = OmCalibrateFindStationaryPointsFromData(&calibrateConfig, &omdata);
			}
			else
			{
				// Start a player
				om_convert_player_t calibrationPlayer = { 0 };
				OmConvertPlayerInitialize(&calibrationPlayer, &arrangement, settings->sampleRate, settings->interpolate);	// Initialize here for find stationary points
				fprintf(stderr, "Finding stationary points from player...\n");
				stationaryPoints = OmCalibrateFindStationaryPointsFromPlayer(&calibrateConfig, &calibrationPlayer);		// Player already initialized
			}

			fprintf(stderr, "Found stationary points: %d\n", stationaryPoints->numValues);
			errorBeforeCalibration = OmCalibrateMeanSvmError(&calibration, stationaryPoints);

			// Dump no calibration
			OmCalibrateDump(&calibration, stationaryPoints, 0);

			// Auto-calibrate
			fprintf(stderr, "Auto-calibrating...\n");
			int calibrationResult = OmCalibrateFindAutoCalibration(&calibrateConfig, stationaryPoints, &calibration);
			errorAfterCalibration = OmCalibrateMeanSvmError(&calibration, stationaryPoints);
			OmCalibrateDump(&calibration, stationaryPoints, 1);
			if (calibrationResult < 0)
			{
				fprintf(stderr, "Auto-calibration: using default calibration...\n");
				int ec = calibration.errorCode;		// Copy error code
				int na = calibration.numAxes;		// ...and num-axes
				OmCalibrateCopy(&calibration, settings->defaultCalibration);
				calibration.errorCode = ec;			// Copy error code to identity calibration
				calibration.numAxes = na;			// ...and num-axes
			}

			// Find min/max x/y/z
			stationaryCount = stationaryPoints->numValues;
			FILE *sfp = NULL;
			if (settings->stationaryFilename != NULL)
			{
				sfp = fopen(settings->stationaryFilename, "wt");
				if (sfp == NULL) { fprintf(stderr, "WARNING: Couldn't write stationary points to file: %s\n", settings->stationaryFilename); }
			}
			int i;
			for (i = 0; i < stationaryPoints->numValues; i++)
			{
				char timestring[24];	// 2000-01-01 12:00:00.000\0
				time_t tn = (time_t)stationaryPoints->values[i].time;
				struct tm *tmn = gmtime(&tn);
				float sec = tmn->tm_sec + (float)(stationaryPoints->values[i].time - (time_t)stationaryPoints->values[i].time);
				sprintf(timestring, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec);	// (int)((sec - (int)sec) * 1000)

				//double temp = stationaryPoints->values[i].actualTemperature;
				int c;
				if (sfp != NULL) { fprintf(sfp, "%s", timestring); }
				for (c = 0; c < OMCALIBRATE_AXES; c++)
				{
					double v = stationaryPoints->values[i].mean[c];
					if (sfp != NULL) { fprintf(sfp, ",%.10f", v); }
					if (i == 0 || v < stationaryMin[c]) { stationaryMin[c] = v; }
					if (i == 0 || v > stationaryMax[c]) { stationaryMax[c] = v; }
				}
				if (sfp != NULL) { fprintf(sfp, ",%.10f", stationaryPoints->values[i].actualTemperature); }
				if (sfp != NULL) { fprintf(sfp, "\n"); }
			}
			if (sfp != NULL) { fclose(sfp); }

			// Free stationary points
			OmCalibrateFreeStationaryPoints(stationaryPoints);
		}

		// Player for the session
		om_convert_player_t player = { 0 };
		OmConvertPlayerInitialize(&player, &arrangement, settings->sampleRate, settings->interpolate);

		// Channels, rate, samples
		int outputChannels = arrangement.numChannels + 1;
		int outputRate = (int)(player.sampleRate + 0.5);
		int outputSamples = player.numSamples;

		// Metadata - [Artist "IART" WAV chunk] Data about the device that made the recording
		char artist[WAV_META_LENGTH] = { 0 };
		sprintf(artist + strlen(artist), "Id: %u\n", omdata.metadata.deviceId);
		sprintf(artist + strlen(artist), "Device: %s\n", omdata.metadata.deviceTypeString);
		sprintf(artist + strlen(artist), "Revision: %d\n", omdata.metadata.deviceVersion);
		sprintf(artist + strlen(artist), "Firmware: %d\n", omdata.metadata.firmwareVer);

		// Metadata - [Title "INAM" WAV chunk] Data about the recording configuration
		char startTime[MAX_TIME_STRING] = { 0 };	// 2000-01-01 20:00:00.000|
		char stopTime[MAX_TIME_STRING] = { 0 };	// 2000-01-01 20:00:00.000|
		char name[WAV_META_LENGTH] = { 0 };
		sprintf(name + strlen(name), "Session: %u\n", (unsigned int)omdata.metadata.sessionId);
		sprintf(name + strlen(name), "Start: %s\n", TimeString(omdata.metadata.recordingStart, startTime));
		sprintf(name + strlen(name), "Stop: %s\n", TimeString(omdata.metadata.recordingStop, stopTime));
		sprintf(name + strlen(name), "Config-A: %d,%d\n", omdata.metadata.configAccel.frequency, omdata.metadata.configAccel.sensitivity);
		sprintf(name + strlen(name), "Metadata: %s\n", omdata.metadata.metadata);

		// Metadata - [Creation date "ICRD" WAV chunk] - Specify the time of the first sample (also in the comment for Matlab)
		char datetime[WAV_META_LENGTH] = { 0 };
		sprintf(datetime, "%s", TimeString(arrangement.startTime, NULL));

		// Metadata - [Comment "ICMT" WAV chunk] Data about this file representation
		char comment[WAV_META_LENGTH] = { 0 };
		sprintf(comment + strlen(comment), "Time: %s\n", TimeString(arrangement.startTime, NULL));

		// Output scaling
		float outputScale[MAX_CHANNELS] = { 0 };

		// Sensor output range scalings 
		int outputAccelRange = 8;								// TODO: Possibly allow for +/- 16 outputs (currently always +/-8g -> 16-bit signed)?
		if (outputAccelRange < 8) { outputAccelRange = 8; }		// Minimum of +/-2, +/-4, +/-8 all get output coded as +/-8
		int outputGyroRange = 2000;
		int outputMagRange = 3277;

		// Metadata - channel assignment and scale
		int chan = 0;
		int axis;
		for (axis = 0; axis < arrangement.numChannels; axis++)
		{
			char label[32] = "";
			int range = 1;

			if (arrangement.channelAssignment[axis].stream == 'a') { 
				sprintf(label, "Accel-%c", 'X' + arrangement.channelAssignment[axis].subchannel); 
				range = outputAccelRange; 
			} else if (arrangement.channelAssignment[axis].stream == 'g') {
				sprintf(label, "Gyro-%c", 'X' + arrangement.channelAssignment[axis].subchannel);
				range = outputGyroRange;
			} else if (arrangement.channelAssignment[axis].stream == 'm') {
				sprintf(label, "Mag-%c", 'X' + arrangement.channelAssignment[axis].subchannel);
				range = outputMagRange;
			} else {
				sprintf(label, "%c%d", arrangement.channelAssignment[axis].stream, arrangement.channelAssignment[axis].subchannel);
			}

			sprintf(comment + strlen(comment), "Channel-%d: %s\nScale-%d: %d\n", chan + 1, label, chan + 1, range);

			outputScale[chan] = 65536.0f / (2 * range);
			chan++;
		}

		// Other axes
fprintf(stderr, "COMMENT: %s\n", comment);
//		???
//Accelerometer scaling...

		// Create output WAV file
		FILE *ofp = NULL;
		if (settings->outFilename != NULL && strlen(settings->outFilename) > 0)
		{
			fprintf(stderr, "Generating WAV file: %s\n", settings->outFilename);
			ofp = fopen(settings->outFilename, "wb");
			if (ofp == NULL)
			{
				fprintf(stderr, "Cannot open output WAV file: %s\n", settings->outFilename);
				retVal = EXIT_CANTCREAT;
				break;
			}

			WavInfo wavInfo = { 0 };
			wavInfo.bytesPerChannel = 2;
			wavInfo.chans = outputChannels;
			wavInfo.freq = outputRate;
			wavInfo.numSamples = outputSamples;
			wavInfo.infoArtist = artist;
			wavInfo.infoName = name;
			wavInfo.infoDate = datetime;
			wavInfo.infoComment = comment;

			// Try to start the data at 1k offset (create a dummy 'JUNK' header)
			wavInfo.offset = 1024;

			if (WavWrite(&wavInfo, ofp) <= 0)
			{
				fprintf(stderr, "ERROR: Problem writing WAV file.\n");
				retVal = EXIT_IOERR;
				break;
			}
		}


		int outputOk = CalcInit(calc, player.sampleRate, player.arrangement->startTime);		// Whether any processing outputs are used

		// Calculate each output sample between the start/end time of session
		if (!outputOk && ofp == NULL)
		{
			fprintf(stderr, "ERROR: No output.\n");
			retVal = EXIT_CONFIG;
			break;
		}
		else
		{

			// Write other information to info file
			if (infofp != NULL)
			{
				fprintf(infofp, ":\n");
				fprintf(infofp, "::: Data about the conversion process\n");
				fprintf(infofp, "Result-file-version: %d\n", 1);
				fprintf(infofp, "Convert-version: %d\n", CONVERT_VERSION);
				fprintf(infofp, "Processed: %s\n", TimeString(TimeNow(), NULL));
				fprintf(infofp, "File-input: %s\n", settings->filename);
				fprintf(infofp, "File-output: %s\n", settings->outFilename);
				fprintf(infofp, "Results-output: %s\n", settings->infoFilename);

				fprintf(infofp, "Auto-calibration: %d\n", settings->calibrate);
				fprintf(infofp, "Calibration-Result: %d\n", calibration.errorCode);
				fprintf(infofp, "Calibration: %.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f\n",
					calibration.scale[0], calibration.scale[1], calibration.scale[2],
					calibration.offset[0], calibration.offset[1], calibration.offset[2],
					calibration.tempOffset[0], calibration.tempOffset[1], calibration.tempOffset[2],
					calibration.referenceTemperature);
				fprintf(infofp, "Calibration-Stationary-Count: %d\n", stationaryCount);
				fprintf(infofp, "Calibration-Stationary-Min: %.10f,%.10f,%.10f\n", stationaryMin[0], stationaryMin[1], stationaryMin[2]);
				fprintf(infofp, "Calibration-Stationary-Max: %.10f,%.10f,%.10f\n", stationaryMax[0], stationaryMax[1], stationaryMax[2]);
				fprintf(infofp, "Calibration-Stationary-Error-Pre: %.10f\n", errorBeforeCalibration);
				fprintf(infofp, "Calibration-Stationary-Error-Post: %.10f\n", errorAfterCalibration);

				fprintf(infofp, "Input-sectors-total: %d\n", omdata.statsTotalSectors);
				fprintf(infofp, "Input-sectors-data: %d\n", omdata.statsDataSectors);
				fprintf(infofp, "Input-sectors-bad: %d\n", omdata.statsBadSectors);

				fprintf(infofp, "Output-rate: %d\n", outputRate);
				fprintf(infofp, "Output-channels: %d\n", outputChannels);
				fprintf(infofp, "Output-duration: %f\n", (float)outputSamples / outputRate);
				fprintf(infofp, "Output-samples: %d\n", outputSamples);
				fprintf(infofp, "Output-duration-invalid: %f\n", (float)calc->countInvalid / outputRate);
				fprintf(infofp, "Output-samples-invalid: %d\n", calc->countInvalid);
				fprintf(infofp, "Output-duration-clipped: %f\n", (float)calc->countClipped / outputRate);
				fprintf(infofp, "Output-samples-clipped: %d\n", calc->countClipped);
				fprintf(infofp, "Output-samples-clipped-before-calibration: %d\n", calc->countClippedInput);
				fprintf(infofp, "Output-samples-clipped-after-calibration: %d\n", calc->countClippedOutput);


				fprintf(infofp, ":\n");
				fprintf(infofp, "::: Data about the device that made the recording\n");
				fprintf(infofp, "%s", artist);
				fprintf(infofp, ":\n");
				fprintf(infofp, "::: Data about the recording itself\n");
				fprintf(infofp, "%s", name);
				fprintf(infofp, ":\n");
				fprintf(infofp, "::: Data about this file representation\n");
				fprintf(infofp, "%s", comment);
			}

			signed short values[OMDATA_MAX_CHANNELS + 1];
			int sample;
			for (sample = 0; sample < outputSamples; sample++)
			{
				int rawIndex = 0;
				OmConvertPlayerSeek(&player, sample);
				rawIndex = OmConvertPlayerRawIndexWithinSegment(&player, 'a');

				// Convert to integers
				int c;
				double temp = player.temp;
				char validity = 0;
				if (!player.valid) { validity |= 0x01; }		// Invalid
				if (player.clipped) { validity |= 0x02; }		// Input clipped

				double accel[OMCALIBRATE_AXES];
				for (c = 0; c < player.arrangement->numChannels; c++)
				{
					double interpVal = player.values[c];
					double v = player.scale[c] * interpVal;

					// Apply calibration
					if (c < OMCALIBRATE_AXES)
					{
						// Rescaling is:  v = (v + offset) * scale + (temp - referenceTemperature) * tempOffset
						v = (v + calibration.offset[c]) * calibration.scale[c] + (temp - calibration.referenceTemperature) * calibration.tempOffset[c];
					}

					if (c < OMCALIBRATE_AXES)
					{
						accel[c] = v;
					}

					// Output range scaled
					double ov = v * outputScale[c];

					// Saturate
					if (ov <= -32768.0) { ov = -32768.0; validity |= 0x04; }	// Output clipped
					if (ov >= 32767.0) { ov = 32767.0; validity |= 0x04; }	// Output clipped

					// Save
					values[c] = (signed short)(ov);
				}


				// Auxilliary channel
				uint16_t aux = 0;
				if (validity & 0x01) { aux |= WAV_AUX_UNAVAILABLE; }
				if (validity & 0x06) { aux |= WAV_AUX_CLIPPING; }

				int cycle = sample % (int)player.sampleRate;
				if (cycle == 0) { aux |= WAV_AUX_SENSOR_BATTERY | (player.aux[0] & 0x3ff); }
				if (cycle == 1) { aux |= WAV_AUX_SENSOR_LIGHT | (player.aux[1] & 0x3ff); }
				if (cycle == 2) { aux |= WAV_AUX_SENSOR_TEMPERATURE | (player.aux[2] & 0x3ff); }

				//player.ettings.auxChannel

				values[player.arrangement->numChannels] = aux;

				if (!CalcAddValue(calc, accel, temp, validity, rawIndex))
				{
					fprintf(stderr, "ERROR: Problem writing calculations.\n");
					retVal = EXIT_IOERR;
					break;
				}

#if 0
				// TEMPORARY: Write SVM (before filtering) to fourth channel
				double outSvm = svm * 4096.0;
				if (outSvm < -32768.0) { outSvm = -32768.0; }
				if (outSvm > 32767.0) { outSvm = 32767.0; }
				values[player.arrangement->numChannels] = (signed short)outSvm;
#endif

				// Output
				//for (c = 0; c < numChannels + 1; c++) { printf("%s%d", (c > 0) ? "," : "", values[c]); }
				//printf("\n");

				if (ofp != NULL)
				{
					int bytesToWrite = sizeof(int16_t) * (arrangement.numChannels + 1);
					static unsigned char cache[1024 * 1024];		// TODO: Don't do this - not thread safe
					static int cachePosition = 0;					// ...or this...

					memcpy(cache + cachePosition, values, bytesToWrite);
					cachePosition += bytesToWrite;
					if (cachePosition + bytesToWrite >= sizeof(cache) || sample + 1 >= outputSamples)
					{
						if (fwrite(cache, 1, cachePosition, ofp) != cachePosition)
						{
							fprintf(stderr, "ERROR: Problem writing output.\n");
							retVal = EXIT_IOERR;
							break;
						}
						cachePosition = 0;
						fprintf(stderr, ".");
					}
				}
			}

		}

		if (ofp != NULL) { fclose(ofp); }

		CalcClose(calc);

		fprintf(stderr, "\n");
		fprintf(stderr, "Finished.\n");

	}

	OmDataFree(&omdata);

	if (sessionCount < 1)
	{
		fprintf(stderr, "ERROR: No sessions to write.\n");
		retVal = EXIT_DATAERR;
	}

	if (infofp != NULL)
	{
		// Write other information to info file
		fprintf(infofp, ":\n");
		fprintf(infofp, "::: Data about the final state\n");
		fprintf(infofp, "Exit: %d\n", retVal);

		fclose(infofp);
		infofp = NULL;
	}

	return retVal;
}


int OmConvertRun(omconvert_settings_t *settings)
{
	calc_t calc;
	CalcCreate(&calc, settings);

	// Check file exists and is readable
	FILE *fp = fopen(settings->filename, "rb");
	if (fp == NULL) { fprintf(stderr, "NOTE: Input file not found.\n\n"); return EXIT_NOINPUT; }
	fclose(fp);

	// Check if it's a WAV file
	if (WavCheckFile(settings->filename))
	{
		fprintf(stderr, "NOTE: WAV file detected, loading...\n\n");
		return OmConvertRunWav(settings, &calc);
	}

	// Check if we can load it
	if (!OmDataCanLoad(settings->filename))
	{
		const char *msg = "ERROR: File not supported (not WAV or CWA/OMX).\n";
		fprintf(stderr, "%s", msg);
		fprintf(stdout, "%s", msg);
		return EXIT_DATAERR;
	}

	return OmConvertRunConvert(settings, &calc);
}

