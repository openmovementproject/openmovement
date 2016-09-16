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

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wav.h"

#define PRINT(_s) fprintf(stderr, _s)


// Endian-independent short/long read/write
static short fgetshort(FILE *fp) { unsigned short v = 0; v |= ((unsigned short)fgetc(fp)); v |= (((unsigned short)fgetc(fp)) << 8); return (short)v; }
static long fgetlong(FILE *fp) { unsigned long v = 0; v |= ((unsigned long)fgetc(fp)); v |= (((unsigned long)fgetc(fp)) << 8); v |= (((unsigned long)fgetc(fp)) << 16); v |= (((unsigned long)fgetc(fp)) << 24); return (long)v; }
static void fputshort(unsigned short v, FILE *fp) { fputc((unsigned char)((v >> 0) & 0xff), fp); fputc((unsigned char)((v >> 8) & 0xff), fp); }
static void fputlong(unsigned long v, FILE *fp) { fputc((unsigned char)((v >> 0) & 0xff), fp); fputc((unsigned char)((v >> 8) & 0xff), fp); fputc((unsigned char)((v >> 16) & 0xff), fp); fputc((unsigned char)((v >> 24) & 0xff), fp); }

// WAV-file values
#define WAVE_FORMAT_UNKNOWN     0x0000
#define WAVE_FORMAT_PCM         0x0001
//#define WAVE_FORMAT_ADPCM       0x0002
#define WAVE_FORMAT_IEEE_FLOAT  0x0003
//#define WAVE_FORMAT_ALAW        0x0006
//#define WAVE_FORMAT_MULAW       0x0007
//#define  WAVE_FORMAT_MPEGLAYER3 0x0055
#define WAVE_FORMAT_EXTENSIBLE  0xFFFE


// WavRead - Reads the specified file pointer to retrieve the WavInfo information (bytesPerChannel, chans, freq, offset, numSamples).
// - Returns non-zero if successfully read a PCM WAV file details, leaving the file pointer at the start of the sound data (returns positive if standard mono/stereo 8-/16-bit sound, negative if another format)
// - Returns zero if it cannot read a PCM WAV file, leaving the file pointer at an undefined location.
char WavRead(WavInfo *wavInfo, FILE *fp)
{
    // static so not on the stack
    unsigned char buffer[16];
    unsigned long trueFileLength;
    unsigned long riffSize;
    unsigned long chunkSize;
    char headerOk = 0;
	char error = 0;

    // Clear values for return structure
    if (wavInfo == NULL) { return 0; }
    //memset(wavInfo, 0, sizeof(WavInfo));
    wavInfo->bytesPerChannel = 0;
    wavInfo->chans = 0;
    wavInfo->freq = 0;
    wavInfo->offset = 0;
    wavInfo->numSamples = 0;

    // Get the file length
    if (fp == NULL) 
    {
        PRINT("ERROR: WAV file not passed.\n");
        return 0; 
    }
    fseek(fp, 0, SEEK_END);
    trueFileLength = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Check minimum header size
    if (trueFileLength < 28) 
    { 
        PRINT("ERROR: WAV file too small to be valid.\n");
        return 0; 
    }

    // Check RIFF header
	error |= 4 != fread(buffer, 1, 4, fp);            // [0-3]
    headerOk = 0;
    if (buffer[0] == 'R' && buffer[1] == 'I' && buffer[2] == 'F' && buffer[3] == 'F') { headerOk = 1; }
	if (wavInfo->flags & WAV_FLAGS_CUSTOM_HEADER && wavInfo->pointer != NULL)
	{
		// Check non-standard header
		const char *p = (const char *)wavInfo->pointer;
	    if (buffer[0] == p[0] && buffer[1] == p[1] && buffer[2] == p[2] && buffer[3] == p[3]) { headerOk = 1; }
	}    
    
    if (!headerOk)
    {
        PRINT("ERROR: Not a RIFF file.\n");
        return 0;
    }
    riffSize = fgetlong(fp);            // [4-7]
    if (riffSize + 8 != trueFileLength) { PRINT("WARNING: RIFF file size not as would be expected from file size.\n"); }

    // Check WAVE header
    error |= 4 != fread(buffer, 1, 4, fp);            // [8-11]
    if (buffer[0] != 'W' || buffer[1] != 'A' || buffer[2] != 'V' || buffer[3] != 'E')
    {
        PRINT("ERROR: Not a WAVE RIFF file.\n");
        return 0;
    }

    // Read RIFF WAVE chunks from file...
    while (!feof(fp))
    {
        // Read chunk type and size
		if (fread(buffer, 1, 4, fp) != 4) { error |= 1; break; }    // [12-15]
        chunkSize = fgetlong(fp);                        // [16-19]

        // Check for fmt header (expected as first chunk)
        if (buffer[0] == 'f' && buffer[1] == 'm' && buffer[2] == 't' && buffer[3] == ' ')
        { 
            // WAV header values (static so not on the stack)
            static unsigned short wFormatTagOriginal, wFormatTag, nChannels; 
            static unsigned long nSamplesPerSec, nAvgBytesPerSec;
            static unsigned short nBlockAlign, wBitsPerSample, cbSize; 

            if (chunkSize < 16)
            { 
                PRINT("ERROR: fmt chunk size is too small to be a WAVEFORMATEX structure.\n");
                return 0;
            }

            // Read WAVEFORMATEX structure
            wFormatTag = fgetshort(fp);            // [20] WORD  wFormatTag; 
            nChannels = fgetshort(fp);            // [22] WORD  nChannels; 
            nSamplesPerSec = fgetlong(fp);        // [24] DWORD nSamplesPerSec; 
            nAvgBytesPerSec = fgetlong(fp);        // [28] DWORD nAvgBytesPerSec; 
            nBlockAlign = fgetshort(fp);        // [32] WORD  nBlockAlign; 
            wBitsPerSample = fgetshort(fp);        // [34] WORD  wBitsPerSample; 

            // Check cbSize and expected chunkSize
            cbSize = 0;
            if (chunkSize >= 18)                // [36] WORD  cbSize; 
            { 
                cbSize = fgetshort(fp); 
                if (18 + cbSize != chunkSize) { PRINT("WARNING: fmt chunk size doesn't appear consistent with cbSize.\n"); }
            }
            if (wFormatTag != WAVE_FORMAT_EXTENSIBLE && chunkSize != 16 && chunkSize != 18) { PRINT("WARNING: fmt chunk size is not an expected length for PCM data (16 or 18 bytes).\n"); }
            if (wFormatTag == WAVE_FORMAT_EXTENSIBLE && chunkSize != 40) { PRINT("WARNING: fmt chunk size is not an expected length for WAVE_FORMAT_EXTENSIBLE PCM data (40 bytes).\n"); }

            // Read WAVEFORMATEXTENSIBLE structure
            wFormatTagOriginal = wFormatTag;
            if (wFormatTag == WAVE_FORMAT_EXTENSIBLE && cbSize >= 22 && chunkSize >= 40)
            {
                fgetshort(fp);                        // [38] WORD wValidBitsPerSample; (or wSamplesPerBlock if wBitsPerSample==0, or wReserved)
                fgetlong(fp);                        // [50] DWORD dwChannelMask;
				error |= 16 != fread(buffer, 1, 16, fp);            // [54-69] GUID SubFormat

                // Check bytes 2-15 of GUID are bytes 2-15 of _KSDATAFORMAT_SUBTYPE_PCM[16] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 };
                if (                                            buffer[ 2] != 0x00 || buffer[ 3] != 0x00 || 
                    buffer[ 4] != 0x00 || buffer[ 5] != 0x00 || buffer[ 6] != 0x10 || buffer[ 7] != 0x00 || 
                    buffer[ 8] != 0x80 || buffer[ 9] != 0x00 || buffer[10] != 0x00 || buffer[11] != 0xAA || 
                    buffer[12] != 0x00 || buffer[13] != 0x38 || buffer[14] != 0x9B || buffer[15] != 0x71)
                {
                    PRINT("WARNING: GUID bytes not WAVE_FORMAT_EXTENSIBLE.\n");
                }

                // The first two bytes of SubFormat are the actual format tag
                wFormatTag = (unsigned short)buffer[0] | ((unsigned short)buffer[1] << 8);
            }

            // Skip any remaining bytes in the chunk
            chunkSize -= 16;
            if (chunkSize >= 2) { chunkSize -= 2;  }
            if (wFormatTagOriginal == WAVE_FORMAT_EXTENSIBLE && cbSize >= 22 && chunkSize >= 22) { chunkSize -= 22; }
            if (chunkSize > 0) { fseek(fp, chunkSize, SEEK_CUR); }

            // Check format
            if (wFormatTag != WAVE_FORMAT_PCM) { PRINT("ERROR: Only PCM .WAV files will work ('WAVE_FORMAT_PCM').\n"); return 0; }
            if ((nChannels & 0xff) <= 0) { PRINT("ERROR: No audio channels found ('nChannels').\n"); return 0; }
            if (nSamplesPerSec <= 0 || nSamplesPerSec > 0xffff) { PRINT("ERROR: Invalid frequency ('nSamplesPerSec').\n");  return 0; }
            if (wBitsPerSample <= 0) { PRINT("ERROR: No bits per sample given ('wBitsPerSample').\n");  return 0; }
            if ((wBitsPerSample & 0x7) != 0) { PRINT("WARNING: Bits-per-sample 'wBitsPerSample' is not a whole number of bytes -- rounding up.\n"); }
            if (nBlockAlign != (nChannels * ((wBitsPerSample + 7) >> 3))) { PRINT("WARNING: 'nBlockAlign' is not the expected number for the given number of channels and bytes per sample.\n"); }
            if (nAvgBytesPerSec != (nSamplesPerSec * nBlockAlign)) { PRINT("WARNING: 'nAvgBytesPerSec' is not the expected number for the frequency, channels and bytes-per-sample.\n"); }

            // Set output values
            wavInfo->bytesPerChannel = (wBitsPerSample + 7)  >> 3;
            wavInfo->chans = (char)nChannels;
            wavInfo->freq = (int)nSamplesPerSec;

        }
        else if (buffer[0] == 'd' && buffer[1] == 'a' && buffer[2] == 't' && buffer[3] == 'a')                // 'data' chunk
        {
            if (wavInfo->bytesPerChannel <= 0 && wavInfo->chans <= 0)
            {
                PRINT("ERROR: Cannot use data chunk, it must appear after a valid fmt chunk.\n");
                return 0;
            }

            // Store offset and length of data chunk
            wavInfo->offset = ftell(fp);
            // Verify data chunk size
            if (chunkSize % (wavInfo->bytesPerChannel * wavInfo->chans) != 0)
            {
                PRINT("WARNING: data chunk size not a whole number of sample blocks - truncating last (partial) sample.\n"); 
            }
            if (wavInfo->offset + chunkSize > trueFileLength) 
            {
                PRINT("WARNING: data chunk size larger than remaining file size - truncating sample length to file size.\n"); 
                chunkSize = trueFileLength - wavInfo->offset;
            }
            wavInfo->numSamples = chunkSize / (wavInfo->bytesPerChannel * wavInfo->chans);

            // 'data' must be the last chunk in the sound file
            break;
        }
        else if (buffer[0] == 'L' && buffer[1] == 'I' && buffer[2] == 'S' && buffer[3] == 'T')                // 'LIST' chunk
        {
            long afterList = ftell(fp) + chunkSize;
            if (chunkSize < 4)
            {
                PRINT("WARNING: Skipping LIST chunk with no type.\n");
                fseek(fp, chunkSize, SEEK_CUR);
            }
            else
            {
                // Read LIST type
				error |= 4 != fread(buffer, 1, 4, fp);

                if (buffer[0] == 'I' && buffer[1] == 'N' && buffer[2] == 'F' && buffer[3] == 'O')                // 'INFO' LIST type
                {
                    // Read LIST-INFO sub-chunks
                    while (ftell(fp) < afterList)
                    {
                        long afterListSubChunk;

						if (fread(buffer, 1, 4, fp) != 4) { error |= 1; break; }
                        chunkSize = fgetlong(fp);
                        afterListSubChunk = ftell(fp) + chunkSize;

                        if (buffer[0] == 'I' && buffer[1] == 'N' && buffer[2] == 'A' && buffer[3] == 'M')                // 'INAM' LIST-INFO sub-chunk - Name (Track Title)
                        {
                            // Read INAM chunk - Name (Track Title)
                            if (wavInfo->infoName != NULL)
                            {
                                if (chunkSize > WAV_META_LENGTH - 1) { chunkSize = WAV_META_LENGTH - 1; }
                                error |= chunkSize != fread(wavInfo->infoName, 1, chunkSize, fp);
                                wavInfo->infoName[chunkSize] = '\0';
                            }
                        }
                        else if (buffer[0] == 'I' && buffer[1] == 'A' && buffer[2] == 'R' && buffer[3] == 'T')            // 'IART' LIST-INFO sub-chunk - Artist Name
                        {
                            // Read IART chunk - Artist Name
                            if (wavInfo->infoArtist != NULL)
                            {
                                if (chunkSize > WAV_META_LENGTH - 1) { chunkSize = WAV_META_LENGTH - 1; }
								error |= chunkSize != fread(wavInfo->infoArtist, 1, chunkSize, fp);
                                wavInfo->infoArtist[chunkSize] = '\0';
                            }
                        }
                        else if (buffer[0] == 'I' && buffer[1] == 'C' && buffer[2] == 'M' && buffer[3] == 'T')            // 'ICMT' LIST-INFO sub-chunk - Comments
                        {
                            // Read ICMT chunk - Comments
                            if (wavInfo->infoComment != NULL)
                            {
                                if (chunkSize > WAV_META_LENGTH - 1) { chunkSize = WAV_META_LENGTH - 1; }
								error |= chunkSize != fread(wavInfo->infoComment, 1, chunkSize, fp);
                                wavInfo->infoComment[chunkSize] = '\0';
                            }
                        }
                        else if (buffer[0] == 'I' && buffer[1] == 'C' && buffer[2] == 'R' && buffer[3] == 'D')            // 'ICRD' LIST-INFO sub-chunk - Creation Date
                        {
                            // Read ICRD chunk - Creation Date
                            if (wavInfo->infoDate != NULL)
                            {
                                if (chunkSize > WAV_META_LENGTH - 1) { chunkSize = WAV_META_LENGTH - 1; }
								error |= chunkSize != fread(wavInfo->infoDate, 1, chunkSize, fp);
                                wavInfo->infoDate[chunkSize] = '\0';
                            }
                        }
                        else if (buffer[0] >= 32 && buffer[0] < 127 && buffer[1] >= 32 && buffer[1] < 127 && buffer[2] >= 32 && buffer[2] < 127 && buffer[3] >= 32 && buffer[3] < 127)
                        {
                            PRINT("WARNING: Skipping unrecognized LIST-INFO sub-chunk.\n");
                        }
                        else
                        {
                            PRINT("ERROR: Found seemingly invalid LIST-INFO sub-chunk type.\n");
                            //break;
                        }

                        // Set to after the LIST sub-chunk contents
                        fseek(fp, afterListSubChunk, SEEK_SET);
                    }

                }
                else if (buffer[0] >= 32 && buffer[0] < 127 && buffer[1] >= 32 && buffer[1] < 127 && buffer[2] >= 32 && buffer[2] < 127 && buffer[3] >= 32 && buffer[3] < 127)
                {
                    PRINT("WARNING: Skipping unrecognized LIST type.\n");
                }
                else
                {
                    PRINT("ERROR: Found seemingly invalid LIST type.\n");
                }
            }

            // Set to after the LIST chunk contents
            fseek(fp, afterList, SEEK_SET);
        }
        else
        {
            if (buffer[0] == 'f' && buffer[1] == 'a' && buffer[2] == 'c' && buffer[3] == 't')                // 'fact' chunk
            { ; }    // skip
            else if (buffer[0] == 'P' && buffer[1] == 'E' && buffer[2] == 'A' && buffer[3] == 'K')            // 'PEAK' chunk
            { ; }    // skip
            else if (buffer[0] == 'J' && buffer[1] == 'U' && buffer[2] == 'N' && buffer[3] == 'K')            // 'JUNK' chunk
            { ; }    // skip
            else if (buffer[0] >= 32 && buffer[0] < 127 && buffer[1] >= 32 && buffer[1] < 127 && buffer[2] >= 32 && buffer[2] < 127 && buffer[3] >= 32 && buffer[3] < 127)
            {
                PRINT("WARNING: Skipping unrecognized chunk.\n");
            }
            else
            {
                PRINT("ERROR: Found seemingly invalid chunk type.\n");
                return 0;
            }

            // Skip entire chunk contents
            fseek(fp, chunkSize, SEEK_CUR);
        }
    }

    // Final checks for compatibility
    if ((wavInfo->bytesPerChannel != 1 && wavInfo->bytesPerChannel != 2) || (wavInfo->chans != 1 && wavInfo->chans != 2))
    {
        //PRINT("WARNING: Not 8- or 16-bit mono or stereo PCM sound.\n");
        return -1;
    }

    return 1;
}


// Fill buffer scratch area
#define WAV_SCRATCH_SIZE 64
static unsigned char wavScratch[WAV_SCRATCH_SIZE];

// WavFillBuffer16bitMono - Reads the specified file pointer to fill the buffer with 16-bit mono samples (applying any conversion required)
// Returns the number of samples filled.
unsigned int WavFillBuffer16bitMono(short *buffer, unsigned int capacitySamples, WavInfo *wavInfo, FILE *fp)
{
    size_t totalRead;

    totalRead = 0;
    if (fp != NULL && wavInfo != NULL)
    {
        if (wavInfo->chans == 1)
        {
            if (wavInfo->bytesPerChannel == 1)                          // 8-bit mono -- copy directly to first half of buffer, then iterate backwards expanding to 16-bits
            {
                size_t i;
                unsigned char *src;
                short *dst;

                // Copy directly to first half of buffer, then iterate backwards expanding 8-bits to 16-bits
                totalRead = fread(buffer, 1, capacitySamples, fp);      // Reading 1-byte sample (8-bit mono)

                // Convert unsigned 8-bit mono to 16-bit signed mono
                src = (unsigned char *)buffer + capacitySamples - 1;
                dst = buffer + capacitySamples - 1;
                for (i = 0; i < totalRead; i++)
                { 
                    *dst-- = (((short)((unsigned char)0x80 ^ (*src--))) << 8); 
                }

            }
            else if (wavInfo->bytesPerChannel == 2)                     // 16-bit mono -- copy directly to buffer
            {
                totalRead = fread(buffer, 2, capacitySamples, fp);      // Reading 2-byte samples (16-bit mono)
            }
        }
        else if (wavInfo->chans == 2)
        {
            if (wavInfo->bytesPerChannel == 1)                          // 8-bit stereo -- copy directly to buffer, then convert each pair of 8-bit samples into a single 16-bit sample
            {
                size_t i;
                unsigned char *src;

                // Copy directly to buffer
                totalRead = fread(buffer, 2, capacitySamples, fp);      // Reading 2-byte samples (8-bit stereo)

                // Convert unsigned 8-bit stereo to 16-bit signed mono
                src = (unsigned char *)buffer;
                for (i = 0; i < totalRead; i++)
                { 
                    short v;
                    v = (short)(char)((unsigned char)0x80 ^ (*src++));  // left
                    v += (short)(char)((unsigned char)0x80 ^ (*src++)); // add right
                    *buffer++ = (v << 7);  // bit-shift the 9-bit number up 7
                }


            }
            else if (wavInfo->bytesPerChannel == 2)                     // 16-bit stereo -- load to scratch area, then write to actual buffer
            {
				size_t read, i;
                short *src;
                while (!feof(fp))
                {
                    read = capacitySamples - totalRead;
                    if (read > (WAV_SCRATCH_SIZE >> 2)) { read = (WAV_SCRATCH_SIZE >> 2); }
                    if (read <= 0) { break; }
                    read = fread(wavScratch, 4, read, fp);              // Reading 4-byte samples (16-bit stereo)
                    if (read <= 0) { break; }
                    src = (short *)wavScratch;
                    for (i = 0; i < read; i++)
                    {
                        short v;
                        v = ((*src++) >> 1);
                        v += ((*src++) >> 1);
                        *buffer++ = v;    // 16-bit stereo to 16-bit mono conversion
                    }
                    totalRead += read;
                }
            }
        }
    }

    // If the buffer wasn't filled, zero any remaining space
    if (totalRead < capacitySamples)
    {
        memset(buffer + totalRead, 0, (capacitySamples - totalRead) << 1);
    }

    return (unsigned int)totalRead;
}


// Write 1- or 2-channel WAVE_FORMAT_PCM, or an n-channel 'WAVE_FORMAT_EXTENSIBLE' .WAV header
unsigned long WavWrite(WavInfo *wavInfo, FILE *ofp)
{
    unsigned long  nSamplesPerSec;
    unsigned short nChannels;
    unsigned short wBitsPerSample;
    unsigned short wSubFormatTag;
    unsigned short nBlockAlign;
    unsigned long  nAvgBytesPerSec;
    unsigned long expectedLength;
    unsigned short wFormatTag;
    unsigned short formatSize;
    unsigned long i;
    unsigned long listInfoSize;
    unsigned long junkSize;

    nSamplesPerSec = wavInfo->freq;
    nChannels = wavInfo->chans;
    wBitsPerSample = wavInfo->bytesPerChannel * 8;
    wSubFormatTag = 1;     // From KSDATAFORMAT_SUBTYPE_PCM
    nBlockAlign = nChannels * ((wBitsPerSample + 7) / 8);
    nAvgBytesPerSec = nSamplesPerSec * nBlockAlign;
    expectedLength = wavInfo->numSamples * wavInfo->chans * wavInfo->bytesPerChannel;
    wFormatTag = (wavInfo->chans <= 2) ? WAVE_FORMAT_PCM : WAVE_FORMAT_EXTENSIBLE;
    formatSize = (wFormatTag == WAVE_FORMAT_EXTENSIBLE) ? 40 : 18;

    // Calculate LIST-INFO packet
    listInfoSize = 0;
    {
        // Get string lengths
        int infoNameSize    = (wavInfo->infoName    != NULL) ? (int)strlen(wavInfo->infoName)    : 0;
        int infoArtistSize  = (wavInfo->infoArtist  != NULL) ? (int)strlen(wavInfo->infoArtist)  : 0;
        int infoCommentSize = (wavInfo->infoComment != NULL) ? (int)strlen(wavInfo->infoComment) : 0;
        int infoDateSize    = (wavInfo->infoDate    != NULL) ? (int)strlen(wavInfo->infoDate)    : 0;

        // Add NULL byte and padding to even number of bytes; sum sub-chunks
        if (infoNameSize > 0)    { listInfoSize += infoNameSize    + 2 - (infoNameSize    & 1) + 8; }   // "INAM<sz>"
        if (infoArtistSize > 0)  { listInfoSize += infoArtistSize  + 2 - (infoArtistSize  & 1) + 8; }   // "IART<sz>"
        if (infoCommentSize > 0) { listInfoSize += infoCommentSize + 2 - (infoCommentSize & 1) + 8; }   // "ICMT<sz>"
        if (infoDateSize > 0)    { listInfoSize += infoDateSize    + 2 - (infoDateSize    & 1) + 8; }   // "ICRD<sz>"

        // If we have a non-empty LIST-INFO structure, add chunk overhead
        if (listInfoSize > 0)    { listInfoSize    += 12; }                             // "LIST<sz>INFO"
    }

    // Calculate JUNK packet
    if (wavInfo->offset >= 76 + listInfoSize)
    {
        junkSize = wavInfo->offset - 28 - formatSize - listInfoSize;
    }
    else
    {
        junkSize = 0;
    }

    // Calculate actual start of data
    wavInfo->offset = 28 + formatSize + junkSize + listInfoSize;

	if (wavInfo->flags & WAV_FLAGS_CUSTOM_HEADER && wavInfo->pointer != NULL)
	{
		const char *p = (const char *)wavInfo->pointer;
		// Non-standard header
	    fputc(p[0], ofp); fputc(p[1], ofp); fputc(p[2], ofp); fputc(p[3], ofp); 
	}
	else
	{
	    //  0, 1, 2, 3 = 'RIFF'
	    fputc('R', ofp); fputc('I', ofp); fputc('F', ofp); fputc('F', ofp); 
	}

    //  4, 5, 6, 7 = (file size - 8 bytes header) = (data size + 68 - 8)
    fputlong(expectedLength + wavInfo->offset - 8, ofp);

    //  8, 9,10,11 = 'WAVE'
    fputc('W', ofp); fputc('A', ofp); fputc('V', ofp); fputc('E', ofp); 

    // 12,13,14,15 = 'fmt '
    fputc('f', ofp); fputc('m', ofp); fputc('t', ofp); fputc(' ', ofp); 

    // 16,17,18,19 = format size
    fputlong(formatSize, ofp);      // WAVE_FORMAT_EXTENSIBLE is 40, WAVE_FORMAT_PCM is 18

    // WAVEFORMATEX
    fputshort(wFormatTag, ofp);     // 20 WORD  wFormatTag = 0xFFFE (WAVE_FORMAT_EXTENSIBLE) or 0x0001 (WAVE_FORMAT_PCM)
    fputshort(nChannels, ofp);      // 22 WORD  nChannels; 
    fputlong(nSamplesPerSec, ofp);  // 24 DWORD nSamplesPerSec; 
    fputlong(nAvgBytesPerSec, ofp); // 28 DWORD nAvgBytesPerSec = nSamplesPerSec * nBlockAlign; 
    fputshort(nBlockAlign, ofp);    // 32 WORD  nBlockAlign = nChannels * ((wBitsPerSample + 7) / 8);
    fputshort(wBitsPerSample, ofp); // 34 WORD  wBitsPerSample; 
    fputshort(formatSize - 18, ofp);// 36 WORD  cbSize = formatSize - 18 = 22 (WAVE_FORMAT_EXTENSIBLE) or 0 (WAVE_FORMAT_PCM);

    if (wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        fputshort(wBitsPerSample, ofp); // 38 WORD  wValidBitsPerSample; (or wSamplesPerBlock if wBitsPerSample==0, or wReserved
        fputlong(0, ofp);               // 40 DWORD dwChannelMask;

        // 44 GUID SubFormat = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 }; (KSDATAFORMAT_SUBTYPE_PCM)
        fputc((wSubFormatTag & 0xff), ofp); fputc(((wSubFormatTag >> 8) & 0xff), ofp); fputc(0x00, ofp); fputc(0x00, ofp); 
        fputc(0x00, ofp); fputc(0x00, ofp); fputc(0x10, ofp); fputc(0x00, ofp); 
        fputc(0x80, ofp); fputc(0x00, ofp); fputc(0x00, ofp); fputc(0xAA, ofp); 
        fputc(0x00, ofp); fputc(0x38, ofp); fputc(0x9B, ofp); fputc(0x71, ofp); 
    }

    // Optional LIST-INFO metadata
    if (listInfoSize >= 12)
    {
        // (All fields even length, NULL-padded.)
        // LIST<sz>INFO
        //  INAM<sz>Track Title\0
        //  IART<sz>Artist Name\0
        //  ICMT<sz>Comments\0\0
        //  ICRD<sz>2012-06-12 11:23:00.000\0

        // 60,61,62,63 = 'LIST'
        fputc('L', ofp); fputc('I', ofp); fputc('S', ofp); fputc('T', ofp); 

        // 64,65,66,67 = LIST-INFO size
        fputlong(listInfoSize - 8, ofp);

        // INFO list type
        fputc('I', ofp); fputc('N', ofp); fputc('F', ofp); fputc('O', ofp); 

        // Write INAM chunk - Name (Track Title)
        if (wavInfo->infoName != NULL && wavInfo->infoName[0] != '\0')
        {
            size_t len = strlen(wavInfo->infoName);
            fputc('I', ofp); fputc('N', ofp); fputc('A', ofp); fputc('M', ofp); 
            fputlong((unsigned long)(len + 2 - (len & 1)), ofp);         // Length of string plus NULL byte and any padding byte to make even
            fwrite(wavInfo->infoName, 1, len + 1, ofp); // Write string plus NULL byte
            if (!(len & 1)) { fputc('\0', ofp); }       // Additional padding byte to make even
        }

        // Write IART chunk - Artist Name
        if (wavInfo->infoArtist != NULL && wavInfo->infoArtist[0] != '\0')
        {
			size_t len = strlen(wavInfo->infoArtist);
            fputc('I', ofp); fputc('A', ofp); fputc('R', ofp); fputc('T', ofp); 
            fputlong((unsigned long)(len + 2 - (len & 1)), ofp);         // Length of string plus NULL byte and any padding byte to make even
            fwrite(wavInfo->infoArtist, 1, len + 1, ofp); // Write string plus NULL byte
            if (!(len & 1)) { fputc('\0', ofp); }       // Additional padding byte to make even
        }

        // Write ICMT chunk - Comments
        if (wavInfo->infoComment != NULL && wavInfo->infoComment[0] != '\0')
        {
			size_t len = strlen(wavInfo->infoComment);
            fputc('I', ofp); fputc('C', ofp); fputc('M', ofp); fputc('T', ofp); 
            fputlong((unsigned long)(len + 2 - (len & 1)), ofp);         // Length of string plus NULL byte and any padding byte to make even
            fwrite(wavInfo->infoComment, 1, len + 1, ofp); // Write string plus NULL byte
            if (!(len & 1)) { fputc('\0', ofp); }       // Additional padding byte to make even
        }

        // Write ICRD chunk - Creation Date
        if (wavInfo->infoDate != NULL && wavInfo->infoDate[0] != '\0')
        {
			size_t len = strlen(wavInfo->infoDate);
            fputc('I', ofp); fputc('C', ofp); fputc('R', ofp); fputc('D', ofp); 
            fputlong((unsigned long)(len + 2 - (len & 1)), ofp);         // Length of string plus NULL byte and any padding byte to make even
            fwrite(wavInfo->infoDate, 1, len + 1, ofp); // Write string plus NULL byte
            if (!(len & 1)) { fputc('\0', ofp); }       // Additional padding byte to make even
        }
    }

    // Optional padding JUNK chunk
    if (junkSize >= 8)
    {
        // 60,61,62,63 = 'JUNK'
        fputc('J', ofp); fputc('U', ofp); fputc('N', ofp); fputc('K', ofp); 

        // 64,65,66,67 = JUNK size
        fputlong(junkSize - 8, ofp);

        // JUNK data
        for (i = 0; i < junkSize - 8; i++)
        {
            fputc(0x00, ofp); 
        }
    }

    // 60,61,62,63 = 'data'
    fputc('d', ofp); fputc('a', ofp); fputc('t', ofp); fputc('a', ofp); 

    // 64,65,66,67 = data size
    fputlong(expectedLength, ofp);

    return wavInfo->offset;  // + expectedLength
}


// WavUpdate - Updates the WAV file header of the specified file pointer to reflect the current file length.
// - Returns zero if not possible, non-zero if successful.
char WavUpdate(unsigned long startOffset, WAV_FILE *ofp)
{
    unsigned long original;
    unsigned long length;
    
    if (ofp == NULL) { return 0; }          // File pointer not specified
    if (startOffset < 46) { return 0; }     // Start offset smaller than possible

    // Get current position
    original = ftell(ofp);

    // Seek to end to find length
    fseek(ofp, 0, SEEK_END);
    length = ftell(ofp);

    // Check length is at least as large as the offset
    if (length < startOffset)
    {
        fseek(ofp, original, SEEK_SET);     // Seek to original location
        return 0;                           // Start offset after the file length
    }

    // Update data length
    fseek(ofp, startOffset - 4, SEEK_SET);
    fputlong(length - startOffset, ofp);

    // Update WAVE length
    fseek(ofp, 4, SEEK_SET);
    fputlong(length - 8, ofp);

    // Seek to original location
    fseek(ofp, original, SEEK_SET);

    return 1;
}


// Check if a file appears to be a WAV file
char WavCheckFile(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) { return 0; }

	char buffer[5] = { 0 };
	if (fread(buffer, 1, sizeof(buffer) - 1, fp) != sizeof(buffer) - 1) { fclose(fp); return 0; }
	fclose(fp);

	if (strcmp(buffer, "RIFF") == 0) { return 1; }

	return 0;
}

