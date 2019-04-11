/* 
 * Copyright (c) 2009-2013, Newcastle University, UK.
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

// Stream Parser
// Dan Jackson, 2012-2013

// Cross-platform 
#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #define gmtime_r(timer, result) gmtime_s(result, timer)
    #define timegm _mkgmtime
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "StreamParser.h"


/*
    // Non-timezone affected time functions
    time_t tSec;             // Seconds since epoch
    struct tm tParts;        // Time elements (YMDHMS)

    // Pack from YMDHMS
    tSec = timegm(&tParts);

    // Unpack to YMDHMS
    gmtime_r(&tSec, &tParts);

    // Print
    printf("%04d-%02d-%02d %02d:%02d:%02d", 1900 + tParts.tm_year, tParts.tm_mon + 1, tParts.tm_mday, tParts.tm_hour, tParts.tm_min, tParts.tm_sec);
*/



/** Maximum possible packet size */
#define MAX_PACKET_SIZE 0xffff

/** Block size */
#define BLOCK_SIZE 512

/** Maximum number of samples (if 8-bit) */
#define MAX_SAMPLES 480


/** The internal state tracker for a stream within a reader */
typedef struct
{
    // Current block information
    unsigned long long blockStart;
    unsigned long long blockEnd;
    unsigned int sequenceId;
} StreamState;


/** The internal state tracker for a reader */
typedef struct
{
    // File pointer
    FILE *fp;

    // Internal buffers
    filestream_fileheader_t header;
    unsigned char data[BLOCK_SIZE];
    int stream;           // Current buffer's stream

    // Output buffers
    short shorts[MAX_SAMPLES];
    int ints[MAX_SAMPLES];
    float floats[MAX_SAMPLES];

    // Global information
    long fileSize;
    int numDataBlocks;

    // Timing
    unsigned long long firstTime;
    unsigned long long lastTime;

    // Stream state
    StreamState streamState[MAX_STREAMS];
    unsigned char packetType;
    unsigned int numSamples;
    unsigned char channels;
    unsigned char channelPacking;
    unsigned char dataType;
    char dataConversion;
} ReaderState;

static unsigned short getSum(const void *buffer, size_t count)
{
	const unsigned short *p = (const unsigned short *)buffer;
    unsigned short checksum = 0x0000;
	size_t len;
    for (len = count / 2; len; --len) { checksum += *p++; }
    return checksum;
}

static double getSampleRate(const void *buffer)
{
    const unsigned char *p = (const unsigned char *)buffer;
    char sampleRateModifier = p[18];
    double sampleRate = (double)((unsigned short)p[16] | ((unsigned short)p[17] << 8));
    if (sampleRateModifier == 1)        // 1 = Sample rate in Hz
    { ; }
    else if (sampleRateModifier > 1)    // >1 = Sample rate divisor
    {
        sampleRate /= sampleRateModifier;
    }
    else if (sampleRateModifier > 1)    // <-1 = Sample rate multiplier
    {
        sampleRate *= -sampleRateModifier;
    }
    else if (sampleRateModifier == 0)   // 0 = Period in seconds
    {
        if (sampleRate != 0) { sampleRate = 1.0 / sampleRate; }
    }
    else if (sampleRateModifier == -1)  // -1 = Period in minutes
    {
        if (sampleRate != 0) { sampleRate = 1.0 / (sampleRate * 60.0); }
    }
    return sampleRate;
}


static unsigned long long fractionalEpochFromTimestampFractional(const void *buffer)
{
	const unsigned char *p = (const unsigned char *)buffer;

	// timestamp        @0 +4  RTC value
	unsigned long timestamp = (unsigned long)p[0] | ((unsigned long)p[1] << 8) | ((unsigned long)p[2] << 16) | ((unsigned long)p[3] << 24);

	// fractionalTime   @4 +2  Fractional part
	unsigned short fractionalTime = (unsigned short)p[4] | ((unsigned short)p[5] << 8);

	// Calculate the time
	time_t tSec;                            // Seconds since epoch
											// Check if it's a serial time value
	if (timestamp & 0x80000000ul)
	{
		// TODO: Check this is correct
		tSec = (timestamp & 0x7ffffffful) + (10957ul * 24 * 60 * 60);  // Adjust Y2K epoch to unix epoch
	}
	else
	{
		struct tm tParts = { 0 };                 // Time elements (YMDHMS)
		tParts.tm_year = DATETIME_YEAR(timestamp) - 1900;
		tParts.tm_mon = DATETIME_MONTH(timestamp) - 1;
		tParts.tm_mday = DATETIME_DAY(timestamp);
		tParts.tm_hour = DATETIME_HOURS(timestamp);
		tParts.tm_min = DATETIME_MINUTES(timestamp);
		tParts.tm_sec = DATETIME_SECONDS(timestamp);
		tSec = timegm(&tParts);                 // Pack from YMDHMS
	}

	unsigned long long t = ((unsigned long long)tSec << 16) | fractionalTime;   // Shift and add fractional part

	return t;
}


static void getTimestamps(const void *buffer, unsigned long long *blockStartTime, int sampleIndex, unsigned long long *sampleTime)
{
    const unsigned char *p = (const unsigned char *)buffer;
    short timestampOffset;
    unsigned long long t0;
    unsigned long long t;
    double sampleRate;
    unsigned short numSamples;

    numSamples = (unsigned short)p[22] | ((unsigned short)p[23] << 8);
    sampleRate = getSampleRate(buffer);

	// timestamp        @8 +4  Last reported RTC value, 0 = unknown
	// fractionalTime   @12 +2 Fractional part
	t = fractionalEpochFromTimestampFractional(p + 8);

    // timestampOffset  @14 +2  (Signed) relative sample index from the start of the buffer where the whole-second timestamp is valid
    timestampOffset = (short)((unsigned short)p[14] | ((unsigned short)p[15] << 8));

    // Calculate times at start and end of block
    t0 = t;
    if (sampleRate > 0) { t0 = t - (long long)((double)timestampOffset * 0x10000 / sampleRate); }
    if (blockStartTime != NULL) { *blockStartTime = t0; }
    if (sampleIndex <= 0) { sampleIndex = numSamples - -sampleIndex; } else { sampleIndex--; }   // 0 = "end block" time (start of next block), -1 = last sample, 1 = sample 0, etc.
    if (sampleTime != NULL) { *sampleTime = t0 + (unsigned long long)((double)sampleIndex * 0x10000 / sampleRate); }

    return;
}



ReaderHandle ReaderOpen(const char *binaryFilename)
{ 
    ReaderState *state;
    int i;

    // Check parameters
    if (binaryFilename == NULL) { return NULL; }
    if (!binaryFilename[0]) { return NULL; }

    // Allocate state tracker
    state = (ReaderState *)malloc(sizeof(ReaderState));
    if (state == NULL) { return NULL; }    

    // Open source file
    state->fp = fopen(binaryFilename, "rb");
    if (state->fp == NULL) { free(state); return NULL; }    

    // Read file size
    fseek(state->fp, 0, SEEK_END);
    state->fileSize = ftell(state->fp);

    // Read header
    fseek(state->fp, 0, SEEK_SET);
    memset(&state->header, 0, BLOCK_SIZE);
    if (state->fileSize >= BLOCK_SIZE)
    {
        fread(&state->header, 1, BLOCK_SIZE, state->fp);
        //if (state->header->packetType == 'H' && state->header->packetSubType == 'A') // filestream_fileheader_t
    }

    // Read first valid block
    for (i = 0; i < 10; i++)
    {
        if (i * BLOCK_SIZE + BLOCK_SIZE < state->fileSize)
        {
            fseek(state->fp, i * BLOCK_SIZE, SEEK_SET);
            fread(&state->data, 1, BLOCK_SIZE, state->fp);
            if (state->data[0] == 'd' && getSum(&state->data, BLOCK_SIZE) == 0)
            {
                unsigned long long t = 0;
                getTimestamps(state->data, &t, -1, NULL);
                if (state->firstTime == 0 || t < state->firstTime) { state->firstTime = t; }
            }
        }
    }

    // Read last valid block
    for (i = 0; i < 10; i++)
    {
        if (((state->fileSize / BLOCK_SIZE) - i - 1) * BLOCK_SIZE + BLOCK_SIZE < state->fileSize)
        {
            fseek(state->fp, ((state->fileSize / BLOCK_SIZE) - i - 1) * BLOCK_SIZE, SEEK_SET);
            fread(&state->data, 1, BLOCK_SIZE, state->fp);
            if (state->data[0] == 'd' && getSum(&state->data, BLOCK_SIZE) == 0)
            {
                unsigned long long t = 0;
                getTimestamps(state->data, NULL, -1, &t);
                if (state->lastTime == 0 || t > state->lastTime) { state->lastTime = t; }
            }
        }
    }

    // Reset to the start of the file
    fseek(state->fp, 0, SEEK_SET);

    // Calculate the number of data blocks
    state->numDataBlocks = state->fileSize / BLOCK_SIZE;

    // Clear the current event and sequence counter
    memset(state->streamState, 0, MAX_STREAMS * sizeof(StreamState));
    state->stream = -1;

    // Seek ready to read the first data block
    ReaderDataBlockSeek((ReaderHandle)state, 0);

    return (ReaderHandle)state;
}


const filestream_fileheader_t *ReaderMetadata(ReaderHandle reader, unsigned long long *firstTimestamp, unsigned long long *lastTimestamp)
{
    ReaderState *state = (ReaderState *)reader;

    // Check parameter
    if (state == NULL) { return NULL; }

    if (firstTimestamp != NULL) { *firstTimestamp = state->firstTime; } 
    if (lastTimestamp != NULL) { *lastTimestamp = state->lastTime; } 

    if (state->header.packetType == 'H' && state->header.packetSubType == 'A') // filestream_fileheader_t
    {
        return &state->header;
    }
    else
    {
        return NULL;
    }
}

int ReaderLength(ReaderHandle reader)
{
    ReaderState *state = (ReaderState *)reader;

    // Check parameter
    if (state == NULL) { return -1; }

    return state->numDataBlocks; 
}


int ReaderDataBlockPosition(ReaderHandle reader)
{
    int dataBlockNumber;
    ReaderState *state = (ReaderState *)reader;

    // Check parameter
    if (state == NULL) { return -1; }

    // Get the data block position
    dataBlockNumber = ftell(state->fp) / BLOCK_SIZE;

    // Return data block position
    return dataBlockNumber;
}


int ReaderDataBlockSeek(ReaderHandle reader, int dataBlockNumber)
{
    ReaderState *state = (ReaderState *)reader;

    // Check parameter
    if (state == NULL) { return -1; }

    // Check values
    if (dataBlockNumber < 0) { dataBlockNumber = ((state->fileSize / BLOCK_SIZE) + dataBlockNumber); }
    if (dataBlockNumber > (state->fileSize / BLOCK_SIZE)) { return -1; }

    // Seek the file
    fseek(state->fp, dataBlockNumber * BLOCK_SIZE, SEEK_SET);

    // Clear the data buffer and samples
    memset(state->data, 0xff, BLOCK_SIZE);
    memset(state->shorts, 0x00, MAX_SAMPLES * sizeof(short));
    memset(state->ints, 0x00, MAX_SAMPLES * sizeof(int));
    memset(state->floats, 0x00, MAX_SAMPLES * sizeof(float));

    // Clear the buffer tracking values
    state->stream = -1;
    state->numSamples = 0;

    return 0;
}


int ReaderNextBlock(ReaderHandle reader)
{
    unsigned long long previousBlockEnd;
    unsigned int sequenceId;
    unsigned int maxValues;
    double sampleRate;

    // Check parameter
    ReaderState *state = (ReaderState *)reader;
    if (state == NULL) { return -1; }

    // No data unless we find a valid block
    state->stream = -1;
    state->numSamples = 0;
    state->channels = 0;
    state->channelPacking = 0;
    state->dataType = 0;
    state->dataConversion = 0;

    // Check if EOF
    if (feof(state->fp)) { return -1; }                                                                     // End of file

    // Read a block, error on incomplete read
    if (fread(state->data, 1, BLOCK_SIZE, state->fp) != BLOCK_SIZE)
	{
printf("EOF!");
		return -1; 
	}       // Read error

    // Check header and size
    state->packetType = state->data[0];
	if (state->data[0] != 'd' && state->data[0] != 's' && state->data[0] != 'b') { return 0; }                                     // @0 packetHeader
	if (state->data[2] != ((BLOCK_SIZE - 4) & 0xff) || state->data[3] != ((BLOCK_SIZE - 4) >> 8)) { return 0; }   // @2 packetLength

    // Checksum -- 16-bit word-size addition
    if (state->data[510] != 0 || state->data[511] != 0)     // If it's zero, we (probably) don't have a checksum
    {
        unsigned short check = getSum(state->data, BLOCK_SIZE);
        if (check != 0x0000) { return 0; }   // Checksum failed, must discard whole packet
    }

    // Get the stream identifier
    state->stream = state->data[1];

    // We can only cope with blocks with valid stream identifiers
    if (state->stream < 0 || state->stream > MAX_STREAMS) { state->stream = -1; return 0; }

    // Record previous block's 'blockEnd'
    previousBlockEnd = state->streamState[state->stream].blockEnd;

    // We have no data for now
    state->streamState[state->stream].blockStart = 0;
    state->streamState[state->stream].blockEnd = 0;

    // Read sequence number
    sequenceId = (unsigned int)state->data[4] | ((unsigned int)state->data[5] << 8) | ((unsigned int)state->data[6] << 16) | ((unsigned int)state->data[7] << 24);

    // Get sample count
    state->numSamples = (unsigned short)state->data[22] | ((unsigned short)state->data[23] << 8);

// DEBUG: Temporary
if (state->data[0] == 'd' && state->data[1] == 'a' && state->numSamples < 80)
{
	printf("NOTE: Samples / block %d", state->numSamples);
	printf("\n");
}

    // Frequency
    sampleRate = getSampleRate(state->data);

    // Get timestamps
    getTimestamps(state->data, &state->streamState[state->stream].blockStart, 0, &state->streamState[state->stream].blockEnd);

    // If we are reading a data block in sequence
    if (state->data[0] == 'd' && state->streamState[state->stream].sequenceId != (unsigned int)-1 && state->streamState[state->stream].sequenceId + 1 == sequenceId)
    {
        // If the previous block's "blockEnd" is close (+- 5%) to this block's "blockStart", use that value to smooth over any tiny jitter
        // TODO: This is only tuned to low-rate packets
        if (previousBlockEnd != 0 && state->streamState[state->stream].blockStart != 0 && abs((int)(previousBlockEnd - state->streamState[state->stream].blockStart)) < 60000)   // 3276
        {
            state->streamState[state->stream].blockStart = previousBlockEnd;
        }
    }

    // Check that it's a known packing type
    state->channels = 0;
    maxValues = 0;
    state->channelPacking = state->data[21];
    state->dataType = state->data[19];         // @19 [1] Data type [NOT FINALIZED!] (top-bit set indicates "non-standard" conversion; bottom 7-bits: 0x00 = reserved,  0x10-0x13 = accelerometer (g, at +-2,4,8,16g sensitivity), 0x20 = gyroscope (dps), 0x30 = magnetometer (uT/raw?), 0x40 = light (CWA-raw), 0x50 = temperature (CWA-raw), 0x60 = battery (CWA-raw), 0x70 = pressure (raw?))
    state->dataConversion = state->data[20];   // @20 [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))

	if (state->data[0] == 'd')
	{
		if (state->channelPacking == FILESTREAM_PACKING_SPECIAL_DWORD3_10_2) { state->channels = 3; maxValues = 360; }
		if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT32) { state->channels = (state->channelPacking >> 4); maxValues = 480 / 4; }
		//if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_UINT32) { state->channels = (state->channelPacking >> 4); maxValues = 480 / 4; }
		if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT16) { state->channels = (state->channelPacking >> 4); maxValues = 480 / 2; }
		if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_UINT16) { state->channels = (state->channelPacking >> 4); maxValues = 480 / 2; }
		if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT8) { state->channels = (state->channelPacking >> 4); maxValues = 480; }
		//if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_UINT8)  { state->channels = (state->channelPacking >> 4); maxValues = 480; }
	}

    // Handle unknown type or invalid number of values
    if (state->data[0] == 'd' && (state->channels == 0 || state->numSamples * state->channels > maxValues)) { return 0; }

    // Update sequence id
    state->streamState[state->stream].sequenceId = sequenceId;

    // Return the number of samples
    return state->numSamples;
}


int ReaderDataType(ReaderHandle reader, unsigned char *packetType, unsigned char *stream, unsigned char *channels)
{ 
    ReaderState *state = (ReaderState *)reader;
    if (state == NULL) { return 0; }

    if (packetType != NULL) { *packetType = state->packetType; }
    if (stream != NULL) { *stream = state->stream; }
    if (channels != NULL) { *channels = state->channels; }

    return 0;
}

int ReaderChannelPacking(ReaderHandle reader, unsigned char *stream, unsigned char *dataSize)
{ 
    ReaderState *state = (ReaderState *)reader;
    if (state == NULL) { return 0; }

    if (stream != NULL) { *dataSize = state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK; }

    return 0;
}

short *ReaderBufferRawShorts(ReaderHandle reader)
{ 
    ReaderState *state = (ReaderState *)reader;
    if (state == NULL) { return NULL; }

    if (state->channels == 0) { return NULL; }

    // Extract data values
    if (state->channelPacking == FILESTREAM_PACKING_SPECIAL_DWORD3_10_2)
    {
        unsigned int i;

        // Unpack each value
        for (i = 0; i < state->numSamples; i++)
        {
            // Packed accelerometer value - must sign-extend each component value and adjust for exponent
            //        [byte-3] [byte-2] [byte-1] [byte-0]
            //        eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx
            //        10987654 32109876 54321098 76543210
            unsigned int value = (unsigned int)state->data[30 + i * 4] | ((unsigned int)state->data[31 + i * 4] << 8) | ((unsigned int)state->data[32 + i * 4] << 16) | ((unsigned int)state->data[33 + i * 4] << 24);
            state->shorts[3 * i + 0] = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value <<  6)) >> (6 - ((unsigned char)(value >> 30))) );
            state->shorts[3 * i + 1] = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value >>  4)) >> (6 - ((unsigned char)(value >> 30))) );
            state->shorts[3 * i + 2] = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value >> 14)) >> (6 - ((unsigned char)(value >> 30))) );
        }
    }
    else if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT16)
    {
        int i;
        int numValues = state->channels * state->numSamples;

        // Parse each value's bytes -- instead of non-portable:  memcpy(state->samples, state->data + 30, state->numSamples * 3 * sizeof(short));
        for (i = 0; i < numValues; i++)
        {
            state->shorts[i] = (short)((unsigned short)(state->data[24 + (i << 1)] | (((unsigned short)state->data[25 + (i << 1)]) << 8)));
        }
    }
    else if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_UINT16)
    {
        int i;
        int numValues = state->channels * state->numSamples;

        // Parse each value's bytes -- instead of non-portable:  memcpy(state->samples, state->data + 30, state->numSamples * 3 * sizeof(short));
        for (i = 0; i < numValues; i++)
        {
            // HACK: This function will cast results to signed short for now (caller can treat as unsigned)
            state->shorts[i] = (short)((unsigned short)(state->data[24 + (i << 1)] | (((unsigned short)state->data[25 + (i << 1)]) << 8)));
        }
    }
	else if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT8)
    {
        int i;
        int numValues = state->channels * state->numSamples;

        // Parse each value's bytes -- instead of non-portable:  memcpy(state->samples, state->data + 30, state->numSamples * 3 * sizeof(short));
        for (i = 0; i < numValues; i++)
        {

            state->shorts[i] = (short)((short)((char)state->data[24 + (i)]));
        }
    }

    return state->shorts; 
}

int *ReaderBufferRawInts(ReaderHandle reader)
{ 
    ReaderState *state = (ReaderState *)reader;
    if (state == NULL) { return NULL; }

    if (state->channels == 0) { return NULL; }

    // Extract data values
    if (state->channelPacking == FILESTREAM_PACKING_SPECIAL_DWORD3_10_2)
    {
        unsigned int i;

        // Unpack each value
        for (i = 0; i < state->numSamples; i++)
        {
            // Packed accelerometer value - must sign-extend each component value and adjust for exponent
            //        [byte-3] [byte-2] [byte-1] [byte-0]
            //        eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx
            //        10987654 32109876 54321098 76543210
            unsigned int value = (unsigned int)state->data[30 + i * 4] | ((unsigned int)state->data[31 + i * 4] << 8) | ((unsigned int)state->data[32 + i * 4] << 16) | ((unsigned int)state->data[33 + i * 4] << 24);
            state->ints[3 * i + 0] = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value <<  6)) >> (6 - ((unsigned char)(value >> 30))) );
            state->ints[3 * i + 1] = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value >>  4)) >> (6 - ((unsigned char)(value >> 30))) );
            state->ints[3 * i + 2] = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value >> 14)) >> (6 - ((unsigned char)(value >> 30))) );
        }
    }
    else if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT32)
    {
        int i;
        int numValues = state->channels * state->numSamples;

        // Parse each value's bytes -- instead of non-portable:  memcpy(state->samples, state->data + 30, state->numSamples * 3 * sizeof(short));
        for (i = 0; i < numValues; i++)
        {
            state->ints[i] = (int)((unsigned int)(state->data[24 + (i << 2)] | (((unsigned int)state->data[25 + (i << 2)]) << 8) | (((unsigned int)state->data[26 + (i << 2)]) << 16) | (((unsigned int)state->data[27 + (i << 2)]) << 24)));
        }
    }
    else if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT16)
    {
        int i;
        int numValues = state->channels * state->numSamples;

        // Parse each value's bytes -- instead of non-portable:  memcpy(state->samples, state->data + 30, state->numSamples * 3 * sizeof(short));
        for (i = 0; i < numValues; i++)
        {
            // HACK: This function will cast results to signed short for now (caller can treat as unsigned)
            state->ints[i] = (short)((unsigned short)(state->data[24 + (i << 1)] | (((unsigned short)state->data[25 + (i << 1)]) << 8)));
        }
    }
    else if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_UINT16)
    {
        int i;
        int numValues = state->channels * state->numSamples;

        // Parse each value's bytes -- instead of non-portable:  memcpy(state->samples, state->data + 30, state->numSamples * 3 * sizeof(short));
        for (i = 0; i < numValues; i++)
        {
            // HACK: This function will cast results to signed short for now (caller can treat as unsigned)
            state->ints[i] = (short)((unsigned short)(state->data[24 + (i << 1)] | (((unsigned short)state->data[25 + (i << 1)]) << 8)));
        }
    }
	else if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT8)
    {
        int i;
        int numValues = state->channels * state->numSamples;

        // Parse each value's bytes -- instead of non-portable:  memcpy(state->samples, state->data + 30, state->numSamples * 3 * sizeof(short));
        for (i = 0; i < numValues; i++)
        {

            state->ints[i] = (short)((short)((char)state->data[24 + (i)]));
        }
    }

    return state->ints; 
}


float *ReaderBufferFloats(ReaderHandle reader)
{ 
    int *ints;
    int numValues, i;
    float multiply = 1.0f;
    ReaderState *state = (ReaderState *)reader;
    if (state == NULL) { return NULL; }

    if (state->channels == 0) { return NULL; }

    ints = ReaderBufferRawInts(reader);
    if (ints == NULL) { return NULL; }

    // Check data type has a clear top bit (standard conversion)
    if (!(state->dataType & 0x80))
    {
        if (state->dataConversion > 24)
        {
            multiply = (float)state->dataConversion - 24;
        }
        else if (state->dataConversion < -24)
        {
            if (state->dataConversion + 24 == 0) multiply = 0.0f;
            else multiply = 1.0f / (state->dataConversion + 24);
        }
        else
        {
            multiply = (float)pow(2.0f, state->dataConversion);
        }
    }

    // Data-type specific override of conversion
    switch (state->dataType)
    {
        case FILESTREAM_DATATYPE_GYRO_250:      multiply = 70.00f / 1000.0f; break; // Gyro scale at  250 dps: 70 mdps
        case FILESTREAM_DATATYPE_GYRO_500:      multiply = 17.50f / 1000.0f; break; // Gyro scale at  500 dps: 17.5 mdps
        case FILESTREAM_DATATYPE_GYRO_2000:     multiply =  8.75f / 1000.0f; break; // Gyro scale at 2000 dps: 8.75 mdps
        case FILESTREAM_DATATYPE_MAGNETOMETER:  multiply =  0.1f; break;            // Magnetometer scale: 0.10 uT
    }

    // Process each value
    numValues = state->channels * state->numSamples;
    for (i = 0; i < numValues; i++)
    {
        float v;
        v = (float)ints[i] * multiply;

        state->floats[i] = v;
    }

    return state->floats; 
}


double ReaderTimeSerial(ReaderHandle reader, int index)
{
	ReaderState *state = (ReaderState *)reader;
	unsigned long long t;       // Ticks since epoch (1/65536 seconds)

	// Check parameter
	if (state == NULL) { return 0; }
	if (state->stream < 0) { return 0; }

	if (state->packetType == 'b')
	{
		// Invalid block index?
		if (state->channelPacking == 0 || index < 0 || (unsigned int)index >= state->numSamples || index >= (480 / state->channelPacking))
		{
			return 0;
		}
		unsigned long long t = fractionalEpochFromTimestampFractional(state->data + 24 + (index * state->channelPacking));
	}
	else
	{
		// Determine timestamp
		if (state->numSamples == 0) t = state->streamState[state->stream].blockStart;
		else t = state->streamState[state->stream].blockStart + (index * (state->streamState[state->stream].blockEnd - state->streamState[state->stream].blockStart) / state->numSamples);
	}

	// Return value
	return (double)(t / 65536.0); 
}



unsigned long ReaderTimestamp(ReaderHandle reader, int index, unsigned short *fractional)
{ 
    ReaderState *state = (ReaderState *)reader;
    unsigned long long t;       // Ticks since epoch (1/65536 seconds)
    time_t tSec = 0;            // Seconds since epoch
    struct tm tParts;           // Time elements (YMDHMS)
    unsigned long dateTime;     // Output date/time
    unsigned short frac;        // Output fractional value (1/65536 seconds)

    // Check parameter
    if (state == NULL) { return 0; }
    if (state->stream < 0) { return 0; }

    // Determine timestamp
    if (state->numSamples == 0) t = state->streamState[state->stream].blockStart;
    else t = state->streamState[state->stream].blockStart + (index * (state->streamState[state->stream].blockEnd - state->streamState[state->stream].blockStart) / state->numSamples);

    // Calculate date/time value
    tSec = (t >> 16);           // Remove fractional parts of the second
    gmtime_r(&tSec, &tParts);   // Unpack to YMDHMS
    if (tParts.tm_year < 100) { dateTime = 0; }
    else { dateTime = DATETIME_FROM_YMDHMS(1900 + tParts.tm_year, tParts.tm_mon + 1, tParts.tm_mday, tParts.tm_hour, tParts.tm_min, tParts.tm_sec); }

    // Calculate fractional value
    frac = (unsigned short)t;

    // Return values
    if (fractional != NULL) { *fractional = frac; }
    return dateTime; 
}


void *ReaderRawDataPacket(ReaderHandle reader) 
{ 
    if (reader == NULL) { return NULL; }
    return (void *)&((ReaderState *)reader)->data; 
}


void ReaderClose(ReaderHandle reader)
{ 
    ReaderState *state = (ReaderState *)reader;

    if (state == NULL) { return; }
    if (state->fp != NULL)
    {
        fclose(state->fp);
        state->fp = NULL;
    }
    free(state);
    return; 
}

