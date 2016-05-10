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


#ifndef OMCONVERT_H
#define OMCONVERT_H


#include "omdata.h"

struct omcalibrate_calibration_tag;


typedef struct
{
	const char *filename;

	// Re-sample
	const char *outFilename;
	double sampleRate;
	int auxChannel;
	char interpolate;					// 1=nearest, 2=linear, 3=cubic
	const char *infoFilename;			// Information file name
	const char *stationaryFilename;		// Stationary points file name
	char headerCsv;						// 0=off, 1=on
	char timeCsv;						// 0=absolute, 1=relative

	// Calibrate
	char calibrate;				// 0=off, 1=auto (prefer from data), 2=auto (always use interpolated player)
	double stationaryTime;
	char repeatedStationary;	// 0=use, 1=ignore (future: 2=combine?)

	// Calibration fallback (if no calibration or if auto-calibration fails)
	struct omcalibrate_calibration_tag *defaultCalibration;

	// CSV
	const char *csvFilename;
	//int csvFormat;

	// SVM
	const char *svmFilename;
	double svmEpoch;
	char svmFilter;				// 0=off, 1=band-pass (0.2-50 Hz)
	char svmMode;				// 0=abs(), 1=clamp-zero
	char svmExtended;			// 0=standard values, 1=extended (std, range, etc.)

	// WTV
	const char *wtvFilename;
	int wtvEpoch;			// in 0.5 minutes

	// PAEE
	const char *paeeFilename;
	int paeeCutPoints;
	int paeeEpoch;			// in minutes
	char paeeFilter;		// 0=off, 1=band-pass (0.2-50 Hz)

	// Sleep
	const char *sleepFilename;

} omconvert_settings_t;


// A channel (a specified stream and sub-channel index)
typedef struct
{
	char stream;		// 'x', 'a', 'g', 'm'
	char subchannel;	// Sub-channel number within the stream
} om_convert_channel_t;


// Arrangement (channel configuration)
typedef struct
{
	omdata_t *data;
	omdata_session_t *session;
	int numChannels;
	om_convert_channel_t channelAssignment[OMDATA_MAX_CHANNELS];
	int numStreamIndexes;
	char streamIndexes[OMDATA_MAX_CHANNELS];
	double defaultRate;
	double startTime;
	double endTime;
	double duration;
} om_convert_arrangement_t;


// Interpolator over samples
typedef struct
{
	char mode;
	omdata_t *data;
	int streamIndex;
	omdata_segment_t *seg;
	int timeIndex;

	// Sample index for "v1"
	int sampleIndex;
	int previousSegmentSamples;

	// Values cached after seek
	double prop;						// Proportion between v1-v2
	int16_t values[4][OMDATA_MAX_CHANNELS];	// Cache seeked values, for each channel, at indices (-1, 0, 1, 2) -- enough for cubic interpolation
	bool clipped;
	bool valid;
	double scale;
} interpolator_t;


// Player (resample over multiple channels)
typedef struct om_convert_player_tag
{
	om_convert_arrangement_t *arrangement;
	double sampleRate;
	char interpolate;
	int numSamples;
	interpolator_t segmentInterpolators[OMDATA_MAX_STREAM];
	interpolator_t adcInterpolator;
	double values[OMDATA_MAX_CHANNELS + 1];
	double scale[OMDATA_MAX_CHANNELS + 1];
	short aux[3];
	double temp;
	char valid;
	bool clipped;
} om_convert_player_t;



void OmConvertPlayerInitialize(om_convert_player_t *player, om_convert_arrangement_t *arrangement, double sampleRate, char interpolate);
void OmConvertPlayerSeek(om_convert_player_t *player, int sample);

int OmConvertRun(omconvert_settings_t *settings);

#endif

