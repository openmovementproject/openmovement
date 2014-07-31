/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
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

// Data Logger
// Dan Jackson, K Ladha, 2010-2012


#ifndef RECORDER_H
#define RECORDER_H


// Logger state types
// === LOGGER STATES ===

// Schedule: record automatically when within interval, record never, always record
typedef enum { SCHEDULE_INTERVAL = -1, SCHEDULE_NEVER = 0, SCHEDULE_ALWAYS = 1 } StateSchedule;

// Interval: none set, after interval, waiting before interval, within interval, always within interval
typedef enum { INTERVAL_WAITING, INTERVAL_NONE, INTERVAL_AFTER, INTERVAL_WITHIN, INTERVAL_ALWAYS } StateInterval;

// Recording: not currently recording to file, currently recording to file
typedef enum { RECORDING_OFF, RECORDING_ON } StateRecording;

// Streaming: no streaming, stream over fixed connection (when connected), stream over wireless (when connected), stream over both (when connected)
typedef enum { STREAM_NONE = 0, STREAM_WIRELESS = 1, STREAM_USB = 2, STREAM_BOTH = 3 } StateStream;

// Sampling: not currently sampling peripherals, currently sampling peripherals
typedef enum { SAMPLING_OFF, SAMPLING_ON } StateSampling;


// State changing functions
char StateSetRecording(StateRecording newState, unsigned short stopReason);
char StateSetStream(StateStream newState);
char StateSetSampling(StateSampling newState, char force);


// Logger state
typedef struct
{
    // Settings
    StateSchedule schedule;
    StateStream stream;

    // State
    StateInterval interval;
    StateRecording recording;
    StateSampling sampling;

    // Status: logging
    unsigned char debugFlashCount;      // Remaining number of debug flashes
    char filename[16];                  // Current data file name

	// Sort these out...
    StateStream fileReadStream;
	void *readFile;
	unsigned long readPosition;
	unsigned long readLength;
} logger_t;

extern logger_t logger;




// Initialize the recorder code
void RecorderInit(void);

// Stop recording
void RecorderStop(unsigned short stopReason);


// Privates
// Start the data logger
//void RunLogging(void);

// Called from the RTC interrupt at 1Hz
void LoggerTimerTasks(void);

// Called from the timer1 interrupt if enabled
void LoggerRTCTasks(void);

// Read metadata settings from a binary file
char LoggerReadMetadata(const char *filename);

// Write metadata settings to a binary file
char LoggerWriteMetadata(const char *filename);

// Numbered file utility functions
int LoggerGetFilenameNumber(const char *filespec, const char *filename);
int LoggerSetFilenameNumber(const char *filespec, char *filename, int number);
int LoggerFindCurrentFile(const char *filespec, char *filename, unsigned long *size);

char RecorderAnnotate(const void *annotation, unsigned short length);

// Logger tasks
void RecordingTasks(void);


#endif
