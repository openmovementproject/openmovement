// Stream Parser
// Dan Jackson, 2012

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

/** Maximum number of streams */
#define MAX_STREAMS 256


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
    unsigned char data[BLOCK_SIZE];
    int stream;           // Current buffer's stream

    // Output buffers
    short shorts[MAX_SAMPLES];
    float floats[MAX_SAMPLES];

    // Global information
    long fileSize;
    int numDataBlocks;

    // Stream state
    StreamState streamState[MAX_STREAMS];
    unsigned int numSamples;
    unsigned char channels;
    unsigned char channelPacking;
    unsigned char dataType;
	float sampleRateHz;
    char dataConversion;
} ReaderState;


ReaderHandle ReaderOpen(const char *binaryFilename)
{ 
    ReaderState *state;

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
    //TODO: Read headers

    // Calculate the number of data blocks
    state->numDataBlocks = state->fileSize / BLOCK_SIZE;

    // Clear the current event and sequence counter
    memset(state->streamState, 0, MAX_STREAMS * sizeof(StreamState));
    state->stream = -1;

    // Seek ready to read the first data block
    ReaderDataBlockSeek((ReaderHandle)state, 0);

    return (ReaderHandle)state;
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
    if (dataBlockNumber < 0) { return -1; }
    if (dataBlockNumber > (state->fileSize / BLOCK_SIZE)) { return -1; }

    // Seek the file
    fseek(state->fp, dataBlockNumber * BLOCK_SIZE, SEEK_SET);

    // Clear the data buffer and samples
    memset(state->data, 0xff, BLOCK_SIZE);
    memset(state->shorts, 0x00, MAX_SAMPLES * sizeof(short));
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
    if (fread(state->data, 1, BLOCK_SIZE, state->fp) != BLOCK_SIZE) { return -1; }       // Read error

    // Check header and size
    if (state->data[0] != 'd') { return 0; }                                     // @0 packetHeader
    if (state->data[2] != ((BLOCK_SIZE - 4) & 0xff) || state->data[3] != ((BLOCK_SIZE - 4) >> 8)) { return 0; }   // @2 packetLength

    // Checksum -- 16-bit word-size addition
    if (state->data[510] != 0 || state->data[511] != 0)     // If it's zero, we (probably) don't have a checksum
    {
	    unsigned short *p = (unsigned short *)state->data;
        unsigned short checksum = 0x0000;
	    size_t len;
        for (len = BLOCK_SIZE / 2; len; --len) { checksum += *p++; }
        if (checksum != 0x0000) { return 0; }   // Checksum failed, must discard whole packet
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

    // Frequency
    {
        char sampleRateModifier = state->data[18];
        sampleRate = (double)((unsigned short)state->data[16] | ((unsigned short)state->data[17] << 8));
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
    }

    // Extract timestamps
    {
        unsigned long timestamp;
        unsigned short fractionalTime;
        short timestampOffset;
        unsigned long long t;

        // timestamp        @8 +4  Last reported RTC value, 0 = unknown
        timestamp = (unsigned long)state->data[8] | ((unsigned long)state->data[9] << 8) | ((unsigned long)state->data[10] << 16) | ((unsigned long)state->data[11] << 24);

        // fractionalTime   @12 +2 Fractional part
        fractionalTime = (unsigned short)state->data[12] | ((unsigned short)state->data[13] << 8);

        // Calculate the time
        {
            time_t tSec;                            // Seconds since epoch

            // Check if it's a serial time value
            if (timestamp & 0x80000000ul)
            {
                // TODO: Check this is correct
                tSec = (timestamp & 0x7ffffffful) + (10957ul * 24 * 60 * 60);  // Adjust Y2K epoch to unix epoch
            }
            else
            {
                struct tm tParts = {0};                 // Time elements (YMDHMS)
                tParts.tm_year = DATETIME_YEAR(timestamp) - 1900;
                tParts.tm_mon  = DATETIME_MONTH(timestamp) - 1;
                tParts.tm_mday = DATETIME_DAY(timestamp);
                tParts.tm_hour = DATETIME_HOURS(timestamp);
                tParts.tm_min  = DATETIME_MINUTES(timestamp);
                tParts.tm_sec  = DATETIME_SECONDS(timestamp);
                tSec = timegm(&tParts);                 // Pack from YMDHMS
            }
            t = ((unsigned long long)tSec << 16) | fractionalTime;   // Shift and add fractional part
        }

        // timestampOffset  @14 +2  (Signed) relative sample index from the start of the buffer where the whole-second timestamp is valid
        timestampOffset = (short)((unsigned short)state->data[14] | ((unsigned short)state->data[15] << 8));

        // Calculate times at start and end of block
        state->streamState[state->stream].blockStart = t - (long long)((double)timestampOffset * 0x10000 / sampleRate);
        state->streamState[state->stream].blockEnd = state->streamState[state->stream].blockStart + (unsigned long long)((double)state->numSamples * 0x10000 / sampleRate);

        // If we are reading a block in sequence
        if (state->streamState[state->stream].sequenceId != (unsigned int)-1 && state->streamState[state->stream].sequenceId + 1 == sequenceId)
        {
            // If the previous block's "blockEnd" is close (+- 5%) to this block's "blockStart", use that value to smooth over any tiny jitter
            // TODO: This is only tuned to ~1Hz packets
            if (previousBlockEnd != 0 && state->streamState[state->stream].blockStart != 0 && abs((int)(previousBlockEnd - state->streamState[state->stream].blockStart)) < 3276)
            {
                state->streamState[state->stream].blockStart = previousBlockEnd;
            }
        }
    }

    // Check that it's a known packing type
    state->channels = 0;
    maxValues = 0;
	state->sampleRateHz = (unsigned short)state->data[16];
	switch((signed char)state->data[18]){
		case 1 : break;
		case 0 : state->sampleRateHz = 1/state->sampleRateHz;break;			// Period seconds
		case -1: state->sampleRateHz = 1/(60*state->sampleRateHz);break;	// Period minutes
		default : {
			if((signed char)state->data[18] > 1) {state->sampleRateHz /= (signed char)state->data[18];break;}
			else  state->sampleRateHz *= -((signed char)state->data[18]);
			break;
		}
	}
    state->channelPacking = state->data[21];
    state->dataType = state->data[19];         // @19 [1] Data type [NOT FINALIZED!] (top-bit set indicates "non-standard" conversion; bottom 7-bits: 0x00 = reserved,  0x10-0x13 = accelerometer (g, at +-2,4,8,16g sensitivity), 0x20 = gyroscope (dps), 0x30 = magnetometer (uT/raw?), 0x40 = light (CWA-raw), 0x50 = temperature (CWA-raw), 0x60 = battery (CWA-raw), 0x70 = pressure (raw?))
    state->dataConversion = state->data[20];   // @20 [1] Conversion of raw values to units (-24 to 24 = * 2^n; < -24 divide -(n+24); > 24 multiply (n-24))

    if (state->channelPacking == FILESTREAM_PACKING_SPECIAL_DWORD3_10_2) { state->channels = 3; maxValues = 360; }
    if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT16) { state->channels = (state->channelPacking >> 4); maxValues = 480 / 2; }
    if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_UINT16) { state->channels = (state->channelPacking >> 4); maxValues = 480 / 2; }
	if ((state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK) == FILESTREAM_PACKING_SINT8)	{ state->channels = (state->channelPacking >> 4); maxValues = 480; }

    // Handle unknown type or invalid number of values
    if (state->channels == 0) { return 0; }
    if (state->numSamples * state->channels > maxValues) { return 0; }

    // Update sequence id
    state->streamState[state->stream].sequenceId = sequenceId;

    // Return the number of samples
    return state->numSamples;
}


int ReaderDataChannels(ReaderHandle reader, unsigned char *stream, unsigned char *channels)
{ 
    ReaderState *state = (ReaderState *)reader;
    if (state == NULL) { return 0; }

    if (stream != NULL) { *stream = state->stream; }
    if (channels != NULL) { *channels = state->channels; }

    return 0;
}
int ReaderDataType(ReaderHandle reader, unsigned char *stream, unsigned char *dataType)
{ 
    ReaderState *state = (ReaderState *)reader;
    if (state == NULL) { return 0; }

    if (stream != NULL) { *stream = state->stream; }
    if (dataType != NULL) { *dataType = state->dataType; }

    return 0;
}
int ReaderChannelPacking(ReaderHandle reader, unsigned char *stream, unsigned char *dataSize)
{ 
    ReaderState *state = (ReaderState *)reader;
    if (state == NULL) { return 0; }

    if (stream != NULL) { *dataSize = state->channelPacking & FILESTREAM_PACKING_FORMAT_MASK; }

    return 0;
}
int ReaderDataRateHz(ReaderHandle reader, unsigned char *stream, float* rateHz)
{ 
    ReaderState *state = (ReaderState *)reader;
    if (state == NULL) { return 0; }

    if (stream != NULL) { *stream = state->stream; }
	if (rateHz != NULL) { *rateHz = state->sampleRateHz; }

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

            state->shorts[i] = (short)(state->data[24 + (i)]);
        }
    }

    return state->shorts; 
}


float *ReaderBufferFloats(ReaderHandle reader)
{ 
    short *shorts;
    int numValues, i;
    float multiply = 1.0f;
    ReaderState *state = (ReaderState *)reader;
    if (state == NULL) { return NULL; }

    if (state->channels == 0) { return NULL; }

    shorts = ReaderBufferRawShorts(reader);
    if (shorts == NULL) { return NULL; }

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

    // Process each value
    numValues = state->channels * state->numSamples;
    for (i = 0; i < numValues; i++)
    {
        float v;
        v = (float)shorts[i] * multiply;

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

	// Determine timestamp
	if (state->numSamples == 0) t = state->streamState[state->stream].blockStart;
	else t = state->streamState[state->stream].blockStart + (index * (state->streamState[state->stream].blockEnd - state->streamState[state->stream].blockStart) / state->numSamples);

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

