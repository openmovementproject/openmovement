/* 
 * Copyright (c) 2011-2012, Newcastle University, UK.
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

// WAV Header Reader/Writer
// Dan Jackson, 2011-2012

#ifndef WAV_H
#define WAV_H

#include <stdio.h>
#define WAV_FILE FILE

// Flags
#define WAV_FLAGS_NONE			0x00
#define WAV_FLAGS_CUSTOM_HEADER	0x01

// WavInfo struct - IMPORTANT: zero unused entries before calling WavWrite
typedef struct 
{
    char bytesPerChannel;       // Bytes per channel-sample:  0 = non-PCM data; 1 = 8-bit unsigned; 2 = 16-bit signed; other = non-playable
    char chans;                 // Number of channels:        1 = mono; 2 = stereo; other = non-playable
    unsigned int freq;          // PCM Frequency (0 = non-PCM)
    unsigned long offset;       // Seek offset (in bytes) into the file to the start of the PCM data
    unsigned long numSamples;   // Number of samples in the file 
                                // Length of data in bytes: numSamples * freq * chans * bytesPerChannel
    // Metadata strings (saving only, for now)
    char *infoName;
    char *infoArtist;
    char *infoComment;
    char *infoDate;

	// Customized Flags    
    unsigned int flags;			// Customized flags
    void *pointer;        		// Customized data
} WavInfo;


#define WAV_META_LENGTH 16384      // Buffer size required for meta-data string reading (if not-NULL)

// 'bytesPerChannel' values
#define WAV_BPC_8BIT     1
#define WAV_BPC_16BIT    2

// 'chans' values
#define WAV_CHAN_MONO    1
#define WAV_CHAN_STEREO  2

// 'freq' common values
#define WAV_FREQ_8K   8000
#define WAV_FREQ_11K 11025
#define WAV_FREQ_16K 16000
#define WAV_FREQ_22K 22050
#define WAV_FREQ_24K 24000
#define WAV_FREQ_32K 32000
#define WAV_FREQ_44K 44100
#define WAV_FREQ_48K 48000


// WavWrite - Writes to the specified file pointer to emit the WavInfo information (bytesPerChannel, chans, freq, offset, numSamples).
// - Returns the number of bytes written.
unsigned long WavWrite(WavInfo *wavInfo, WAV_FILE *ofp);

// WavUpdate - Updates the WAV file header of the specified file pointer to reflect the current file length.
// - Returns zero if not possible, non-zero if successful.
char WavUpdate(unsigned long startOffset, WAV_FILE *ofp);


// WavRead - Reads the specified file pointer to retrieve the WavInfo information (bytesPerChannel, chans, freq, offset, numSamples).
// - Returns non-zero if successfully read a PCM WAV file details, leaving the file pointer at the start of the sound data.
// - Returns zero if it cannot read a PCM WAV file, leaving the file pointer at an undefined location.
char WavRead(WavInfo *wavInfo, WAV_FILE *fp);


// WavFillBuffer16bitMono - Reads the specified file pointer to fill the buffer with 16-bit mono samples (applying any conversion required)
// NOTE: capacitySamples is in samples (i.e. half the number of bytes available)
// Returns the number of samples filled.
unsigned int WavFillBuffer16bitMono(short *buffer, unsigned int capacitySamples, WavInfo *wavInfo, WAV_FILE *fp);


// Check if a file appears to be a WAV file
char WavCheckFile(const char *filename);


#endif

