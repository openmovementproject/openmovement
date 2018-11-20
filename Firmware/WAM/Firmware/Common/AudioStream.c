// Audio Stream
// 2012

#include <Compiler.h>
#include "HardwareProfile.h"
//#include <TimeDelay.h>

#include "MDD File System/FSIO.h"

#include "AudioStream.h"
#include "Utils/Fifo.h"
#include "Utils/Wav.h"
#include "Codec AIC111.h"
#include "Utils/FSUtils.h"
#include "Peripherals/Rtc.h"

#define SAFETY_WRITE


// Globals
#ifdef USE_EDS
__eds__ static short  __attribute__((section("eds1"),space(eds)))  audioBuffer[AUDIO_MAX_SECTORS * AUDIO_SAMPLES_PER_SECTOR];
#else
	#if __DEBUG
		#warning "Visible audio buffer"
		short audioBuffer[AUDIO_MAX_SECTORS * AUDIO_SAMPLES_PER_SECTOR];
	#else
		static short audioBuffer[AUDIO_MAX_SECTORS * AUDIO_SAMPLES_PER_SECTOR];
	#endif
#endif

#ifdef ENABLE_ENCRYPT
// RC4 Encryption
#include "Utils/Encrypt.h"
rc4_t rc4 = {{0}};
#endif

extern volatile unsigned short inactivity_timer;	// Start as "inactive"


recorder_t audioStreamRecorder = {0};

// Define: Data path (maximum root entries are limited in FAT12/FAT16)
//#define AUDIO_PATH "\\data"
#ifdef AUDIO_PATH
#warning "FSchdir() is currently suspected of causing problems..."
#endif

// Define: File duration mask           // YYYY YYMM MMDD DDDh hhhh mmmm mmss ssss
#ifndef AUDIO_TIME_MASK
	#warning "Using default 1hour setting for file length"
    #define AUDIO_TIME_MASK 0xfffff000      // Every hour   (57.6 MB @8kHz)

// Choose one of
//#define AUDIO_TIME_MASK 0xfffff000      // Every hour   (57.6 MB @8kHz)
//#define AUDIO_TIME_MASK 0xffffffc0      // Every minute (0.96 MB @8kHz)
//#define AUDIO_TIME_MASK 0xffffff00      // Every 4 minutes (~4 MB @8kHz)
//#define AUDIO_TIME_MASK 0xfffe0000      // Every day    (1382.4 MB @8kHz)

#endif

#if (AUDIO_TIME_MASK != 0xfffff000)
    #warning "Warning - this build is not set for one new file per hour."
#endif


static char filename[13] = "";

// (Internal method) Open a new file
static char AudioStreamStartNewFile(DateTime dateTime)		// RtcNow()
{
	char artist[16];
	char comment[50];
	short time;
	int maxLoops;
	
	// Save this first
	audioStreamRecorder.openDateTime = dateTime;
	
#ifdef AUDIO_PATH
	// Attempt to enter the audio path, make directory if doesn't exist
	if (FSchdir(AUDIO_PATH) != 0)
	{
		FSmkdir(AUDIO_PATH);
		if (FSchdir(AUDIO_PATH) != 0)
		{
			;	// For now, ignore failure to make the directory and just use the current directory
		}	
	}
#endif
	
	// Calculate the character to use as a device identifier
	filename[0] = '_';
	if (settings.deviceId != 0 && settings.deviceId != 0xffff)
	{
		filename[0] = 'A' + (settings.deviceId - 1) % 26;
	}
	
	// Calculate the day of the year (files recorded within a year will have the correct interval)
	{
		const static short daysBeforeMonth[16] = { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 356, 356, 356 }; // 4-bit month lookup (safe for invalid months 0, 13, 14 & 15).
		short day = daysBeforeMonth[DATETIME_MONTH(dateTime)] + DATETIME_DAY(dateTime) + ((((DATETIME_YEAR(dateTime) & 0x03) == 0) && (DATETIME_MONTH(dateTime) >= 3)) ? 1 : 0);
		filename[1] = '0' + ((day / 100) % 10);
		filename[2] = '0' + ((day /  10) % 10);
		filename[3] = '0' + ((day      ) % 10);
	}	
	
	// Filename extension and NULL-termination
	filename[8] = '.';
	if (settings.scramble & SCRAMBLE_EXTENSION)
	{
		filename[9] = 'E';
		filename[10] = 'W';
		filename[11] = 'V';
	}
	else
	{
		filename[9] = 'W';
		filename[10] = 'A';
		filename[11] = 'V';
	}	
	filename[12] = '\0';
	
	// Calculate a 'decimalized' time (hour * 100 + minute), we will use this as our preferred filename
	time = DATETIME_HOURS(dateTime) * 100 + DATETIME_MINUTES(dateTime);
	
	// Loop to try and find an available filename, starting with the preferred name ('HHMM')
	maxLoops = 20;			// Maximum number of attempts
	while (maxLoops-- > 0)		// time < 10000
	{
		FSFILE *fp;

		// Fill-in the 'time' part
		filename[4] = '0' + ((time / 1000) % 10);
		filename[5] = '0' + ((time /  100) % 10);
		filename[6] = '0' + ((time /   10) % 10);
		filename[7] = '0' + ((time       ) % 10);
		
		// See if the file exists
		fp = FSfopen(filename, "r");
		if (fp == NULL) { break; }		// Successfully found a free filename
		FSfclose(fp);					// Otherwise, close and try again
		
		// Try the next available filename
		time++;
	}

	// Check for failure	
	if (maxLoops <= 0) { return FALSE; }
	
	// Open the file for writing
	audioStreamRecorder.fp = FSfopen(filename, "wb");
    if (audioStreamRecorder.fp == NULL) { return FALSE; }

    // Set header information
    memset(&audioStreamRecorder.wavInfoWrite, 0, sizeof(WavInfo));
    audioStreamRecorder.wavInfoWrite.bytesPerChannel = 2;   // 16-bit
    audioStreamRecorder.wavInfoWrite.chans = 1;             // Mono
    if (settings.audioRate == 1)
    {
	    audioStreamRecorder.wavInfoWrite.freq = 16384ul;           // 16kHz
	}
	else
	{
	    audioStreamRecorder.wavInfoWrite.freq = 8192ul;           // 8kHz
	}     
    
	if (settings.scramble & SCRAMBLE_HIDDEN)
	{
	    // Ensure archive attribute set to indicate file updated (updated when the file is closed)
	    audioStreamRecorder.fp->attributes |= (ATTR_HIDDEN | ATTR_SYSTEM);
	}

	if (settings.scramble & SCRAMBLE_HEADER)
	{
		// Non-standard header
	    audioStreamRecorder.wavInfoWrite.flags = WAV_FLAGS_CUSTOM_HEADER;
	    audioStreamRecorder.wavInfoWrite.pointer = "riff";
	}

    // (Optional) Set number of samples if known in advance
    audioStreamRecorder.wavInfoWrite.numSamples = 0 * audioStreamRecorder.wavInfoWrite.freq; // 10 second at 8kHz

    // Comment
    comment[0] = '\0';  // Empty comment
    
#ifdef ENABLE_ENCRYPT
    if (settings.key[0] != '\0')
    {
        const unsigned char *iv = (const unsigned char *)filename;	// use filename as initialization vector
        RC4Init(&rc4, (const unsigned char *)settings.key, RC4_BUFFER_IS_STRING, iv, RC4_BUFFER_IS_STRING, RC4_KSA_LOOPS_DEFAULT, RC4_DROP_DEFAULT);
        strcat(comment, strlen(comment) ? "&" : "?");
        sprintf(comment + strlen(comment), "encrypt=RC4&loops=%d&drop=%d&iv=%s", RC4_KSA_LOOPS_DEFAULT, RC4_DROP_DEFAULT, iv);
    }
#endif
#ifdef ENABLE_SCRAMBLE
	if (settings.scramble != 0)
	{
        strcat(comment, strlen(comment) ? "&" : "?");
        sprintf(comment + strlen(comment), "scr=%d", settings.scramble);
	}
	if (settings.dummy != 0)
	{
        strcat(comment, strlen(comment) ? "&" : "?");
        sprintf(comment + strlen(comment), "dummy=%d", settings.dummy);
	}
#endif
    
    // (Optional) Set metadata strings (INFO-LIST chunk)
    audioStreamRecorder.wavInfoWrite.infoName = filename;
    sprintf(artist, "WAM_%05d", settings.deviceId);
    audioStreamRecorder.wavInfoWrite.infoArtist = artist;
    audioStreamRecorder.wavInfoWrite.infoComment = comment;
    audioStreamRecorder.wavInfoWrite.infoDate = (char*)RtcToString(dateTime);    

    // (Optional) Set desired offset of data start (JUNK chunk)
    audioStreamRecorder.wavInfoWrite.offset = AUDIO_SECTOR_SIZE;  // Start data at sector 1
    
    // Write wave header
    WavWrite(&audioStreamRecorder.wavInfoWrite, audioStreamRecorder.fp);
    
#ifdef SAFETY_WRITE
	// Close, re-open for patching header (then close again))
    FSfclose(audioStreamRecorder.fp);
	audioStreamRecorder.fp = FSfopen(filename, "ab");
	if (audioStreamRecorder.fp == NULL) { return FALSE; }
	FSfseek(audioStreamRecorder.fp, AUDIO_SECTOR_SIZE, SEEK_SET);
#endif

#ifdef OFFSET_LOGGING
	// Record filename as 32-bit BCD
	{
		int j;
		status.filenameBCD = 0;
		for (j = 0; j < 8; j++)
		{
			char c = filename[j];
			if (c == '\0') { break; }
			if (c >= '0' && c <= '9') { c = c - '0'; }
			else if (c >= 'a' && c <= 'z') { c = (c - 'a' + 10) & 0x0f; }
			else if (c >= 'A' && c <= 'Z') { c = (c - 'A' + 10) & 0x0f; }
			else { c = 15; }
			status.filenameBCD |= (unsigned long)c << ((7 - j) * 4);				
		}
	}
#endif
    
	return TRUE;
}


unsigned char AudioStreamIsRecording(void)
{
	return (audioStreamRecorder.fp != NULL);
}


void AudioStreamInit(void) 						// Initialize audio stream
{
	// Clear current state
	memset(&audioStreamRecorder, 0, sizeof(audioStreamRecorder));

	// Clear FIFO
	FifoInit(&audioStreamRecorder.fifo, AUDIO_SECTOR_SIZE, AUDIO_MAX_SECTORS, audioBuffer);
	
	// Buffer
	audioStreamRecorder.buffer = FifoContiguousSpaces2(&audioStreamRecorder.fifo, NULL);// Init audioStreamRecorder.buffer
	audioStreamRecorder.count = AUDIO_SAMPLES_PER_SECTOR; 								// Init sector count
	
	// 
	AudioStreamCodecHasBeenReset();

	// Attempt to start a new file
	//if (!AudioStreamStartNewFile(RtcNow()))
	//{
	//	return FALSE;
	//}

	// Start filling buffer - enable software interrupt
	/*
	IPC1bits.IC1IP = FIFO_INTERRUPT_PRIORITY;
	IFS0CLR = _IFS0_IC1IF_MASK;
	IEC0SET = _IEC0_IC1IE_MASK;
	*/
	//gISR_enable_flags.callAudioVector = TRUE;

	return; //return TRUE;
}

void AudioStreamCodecHasBeenReset(void)
{
	//FifoInit(&audioStreamRecorder.fifo, AUDIO_SECTOR_SIZE, AUDIO_MAX_SECTORS, audioBuffer);
#ifdef CHECK_AUDIO
	audioStreamRecorder.emptySectors = 0;		// Start checking empty sectors again
#endif
}

unsigned int AudioStreamRecordingTaskLength(void)
{
	// Check length
	return FifoLength(&audioStreamRecorder.fifo); 
}

#ifdef CHECK_AUDIO
static int CountContiguousEmptySectors(const void *buffer, int numSectors)
{
	int sector;
	for (sector = 0; sector < numSectors; sector++)
	{
		// Point to start of sector
		const unsigned short *p = (const unsigned short *)((const unsigned char *)buffer + (AUDIO_SECTOR_SIZE * numSectors));
		unsigned short firstValue = *p++;
		unsigned char wordsLeft = AUDIO_SAMPLES_PER_SECTOR - 1;
		do
		{
			if (*p++ != firstValue) 
			{ 
				return sector; 		// If we found a difference, return the number of no-content sectors before this one
			}
		}
		while (--wordsLeft);
	}
	return numSectors;				// All of the sectors had no content
}
#endif

int AudioStreamRecordingTasks(int maxSectors)
{
	DateTime dateTime;
	int written;
	int loop;
	char recording;

	// If we're within the interval and the hour mask is set for this hour
	dateTime = RtcNow();
	if (dateTime >= settings.loggingStartTime && dateTime < settings.loggingEndTime && (settings.loggingHours && (1ul << DATETIME_HOURS(dateTime))) && (settings.inactivity == 0 || inactivity_timer <= settings.inactivity))
	{
		recording = 1;		// we should be recording
	}
	else
	{
		recording = 0;		// we should not be recording
	}	
	
	// If we are recording and the hour mask has changed, we should change file
	if (recording == 1 && (dateTime & AUDIO_TIME_MASK) != (audioStreamRecorder.openDateTime & AUDIO_TIME_MASK)) { recording = 2; }	// Flag up a change of file
	
	// If we've got a file open and we should stop recording or start a new file, close the file
	if (audioStreamRecorder.fp != NULL && (recording == 0 || recording == 2)) 
	{
	    // Update the WAV file to reflect the actual number of samples written
	    WavUpdate(audioStreamRecorder.wavInfoWrite.offset, audioStreamRecorder.fp);
	
		// Close file
	    FSfclose(audioStreamRecorder.fp);
		audioStreamRecorder.fp = NULL;
	}	
		
	// If we've not got a file open and we are recording, open a file
	if (audioStreamRecorder.fp == NULL && recording)
	{
		// Only if there wasn't a recent failure to open a file
		if (audioStreamRecorder.errorDateTime == 0 || dateTime > audioStreamRecorder.errorDateTime + AUDIOSTREAM_FILE_RETRY)
		{
			// Start a new file
			if (!AudioStreamStartNewFile(dateTime))
			{
				// Set the error time so we don't retry in a very tight loop
				audioStreamRecorder.errorDateTime = dateTime;
			}
		}	
	}	

	// If we shouldn't be recording, exit now
	if (!recording) { return -1; }
	
	// We should be recording, but don't have a file open
	if (audioStreamRecorder.fp == NULL) { return -1; }
	
	// Special case: we opened a new file while we were recording
	if (recording == 2) { return -2; }

	//
	written = 0;
	if (maxSectors != 0)
	{
		for (loop = 0; loop < 2; loop++)
		{
			unsigned int numSectors;
			void AUDIO_EDS * buffer;
	
			// Find out how many we could write at once
			buffer = FifoContiguousEntries2(&audioStreamRecorder.fifo, &numSectors);
	
			// If we're limiting to a maximum number at once, and we're going to exceed that, limit to that amount
			if (maxSectors > 0 && written + numSectors > maxSectors)
			{
				numSectors = maxSectors - written;
			}
	
			// Stop if nothing more to write
			if (numSectors <= 0) { break; }
	
#ifdef ENABLE_ENCRYPT
			if (settings.key[0] != '\0')
			{
		        // Encrypt the data before writing to disk
		        RC4Process(&rc4, buffer, numSectors * AUDIO_SECTOR_SIZE);
		 	}
#endif
        
#ifdef CHECK_AUDIO
			// If we're counting empty sectors
			if (audioStreamRecorder.emptySectors >= 0)
			{
				// If they're all empty, add to the count
				if (CountContiguousEmptySectors((unsigned short *)buffer, numSectors) == numSectors)
				{
					audioStreamRecorder.emptySectors += numSectors;
				}	
				else
				{
					// Otherwise, we've found a non-empty sector, stop counting
					audioStreamRecorder.emptySectors = -1;
				}	
			}	
#endif
        
			// Write the sectors
#ifdef ENABLE_SCRAMBLE
gFSScramble = (settings.scramble & SCRAMBLE_REVERSE) ? 1 : 0; 
if (settings.dummy == 1) { gFSScramble = 2; }
#endif
			if (!FSfwriteMultipleSectors(buffer, audioStreamRecorder.fp, numSectors, 0))
			{
#ifdef ENABLE_SCRAMBLE
gFSScramble = 0; 
#endif
				// Failure to write, remember error time and close file
				audioStreamRecorder.errorDateTime = dateTime;
			    // Update the WAV file to reflect the actual number of samples written
			    WavUpdate(audioStreamRecorder.wavInfoWrite.offset, audioStreamRecorder.fp);
				// Close file
			    FSfclose(audioStreamRecorder.fp);
				audioStreamRecorder.fp = NULL;
				
#ifdef OFFSET_LOGGING
				status.filenameBCD = 0xfffffffful;
				status.offset = 0;
#endif
				return -1;
			}
#ifdef ENABLE_SCRAMBLE
gFSScramble = 0; 
#endif
			
			// Update FIFO
			FifoExternallyRemoved(&audioStreamRecorder.fifo, numSectors);
			
#ifdef OFFSET_LOGGING
			//status.offset += numSectors * 512;
			{
				unsigned long offset = FSftell(audioStreamRecorder.fp);
				if (offset < 512) { offset = 0; } else { offset -= 512; }				// Exclude header
				offset += (unsigned long)FifoLength(&audioStreamRecorder.fifo) << 9;	// Add any unwritten sectors
				offset >>= 1;															// Halve for sample index rather than byte-offset
				status.offset = offset;
			}	
#endif

			// Update number written
			written += numSectors;
		}
	}	

	return written;
}


void AudioStreamStopRecording(void)
{
	// Stop capture by disabling software interrupt
	/* IEC0CLR = _IEC0_IC1IE_MASK; */
	//gISR_enable_flags.callAudioVector = FALSE;

	// Finalize open file
	if (audioStreamRecorder.fp != NULL)
	{
		// Writes remaining FIFO data to file
		AudioStreamRecordingTasks(-1);

#ifdef SAFETY_WRITE
		// Close, re-open for patching header (then close again))
	    FSfclose(audioStreamRecorder.fp);
		audioStreamRecorder.fp = FSfopen(filename, "ab");
		if (audioStreamRecorder.fp == NULL) { return; }
#endif
	
	    // Update the WAV file to reflect the actual number of samples written
	    WavUpdate(audioStreamRecorder.wavInfoWrite.offset, audioStreamRecorder.fp);
	
		// Close file
	    FSfclose(audioStreamRecorder.fp);
		audioStreamRecorder.fp = NULL;
	}	

	return;
}

// Stream sample (NOT USED IN NORMAL MODE)
void AudioStreamSample(void)
{	
	static unsigned int audiostream_accumulator = 0; 
	if ((audiostream_accumulator += 3) >= 128) { audiostream_accumulator -= 128; return;} // Compensate the 8192hz to 8khz (or 16384 to 16kHz) exactly by skipping samples periodically
	
	// Check we've got somewhere to write, start a new sector if needed
	if (audioStreamRecorder.count == 0) 
	{ 
		static unsigned int count;
		count = 0; 
		audioStreamRecorder.buffer = FifoContiguousSpaces2(&audioStreamRecorder.fifo, &count); 
		if (count <= 0) {return; } 
		//if (audioStreamRecorder.buffer == NULL) {return; } 		
		audioStreamRecorder.count = AUDIO_SAMPLES_PER_SECTOR; 
	} 

	// Sample into the sector
	*audioStreamRecorder.buffer++ = GetSampleInline(); 

	// Increment count, if finished sector inform FIFO
	if (!--audioStreamRecorder.count) 
	{ 
		FifoExternallyAdded(&audioStreamRecorder.fifo, 1); 
	} 
	
}


//EOF



