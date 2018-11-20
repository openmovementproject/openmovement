// Audio Stream
// 2012

#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#if defined(__PIC24FJ256DA206__) && !defined(USE_EDS)
#warning "EDS Expected"
#endif

#ifdef USE_EDS
#define AUDIO_EDS __eds__
#else
#define AUDIO_EDS
#endif

#include "Utils/Fifo.h"
#include "Utils/Wav.h"
#include "MDD File System/FSIO.h"
#include "Settings.h"
#include "Peripherals/Rtc.h"

// Constants
#define AUDIO_SECTOR_SIZE MEDIA_SECTOR_SIZE
#define AUDIO_SAMPLES_PER_SECTOR (AUDIO_SECTOR_SIZE/2)
#define AUDIO_MAX_SECTORS 40LU		// MUST BE EVEN.  32 = 1s, 40 = 1.28s
#define MIN_SECTORS_TO_WRITE 20		// For even file lengths, please choose from divisors of 112500 sectors/hour (@31.25 sectors/sec):  1, 2, 3, 4, 5, 6, 9, 10, 12, 15, 18, 20, 25, 30, 36, 45, 50, 60, 75, 90, 100, 125
#define AUDIOSTREAM_FILE_RETRY (1 * 60)		// Retry delay before trying to open a new file after a failed open or write

#if ((112500ul % MIN_SECTORS_TO_WRITE) != 0)
//#warning "File lengths may not all be even."
#endif

typedef struct
{
	// File variables
	FSFILE *fp;
	fifo_t fifo;
	WavInfo wavInfoWrite;
	DateTime openDateTime;
	DateTime errorDateTime;
	// Sampling state
	short count;
	short AUDIO_EDS *buffer;
#ifdef CHECK_AUDIO
	int emptySectors;		// <0 = no checks, >=0 check for empty sectors
#endif
} recorder_t;

extern recorder_t audioStreamRecorder;

#ifdef CHECK_AUDIO
#define AudioStreamNumEmptySectors() (audioStreamRecorder.emptySectors)
#endif

void AudioStreamInit(void); 						// Initialize audio stream
void AudioStreamCodecHasBeenReset(void);			// Informs the audio stream that the codec has been reset
unsigned char AudioStreamIsRecording(void);			// Whether audio stream has a file open now?

// Backwards compatibility
#define AudioStreamStartRecording() (AudioStreamInit(), AudioStreamRecordingTasks(), AudioStreamIsRecording()) 		// Deprecated: AudioStreamInit() & AudioStreamRecordingTasks(), and returns TRUE if the first file was opened successfully

unsigned int AudioStreamRecordingTaskLength(void);	// Number of sectors available to write
int AudioStreamRecordingTasks(int maxSectors); 		// Returns <0 on error (should stop)
void AudioStreamStopRecording(void); 				// Stops recording (safe if not started)
void AudioStreamSample(void);


#endif
