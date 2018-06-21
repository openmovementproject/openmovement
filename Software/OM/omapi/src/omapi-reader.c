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

// Open Movement API - Binary File Reader Functions
// Dan Jackson, 2011-2012

#include "omapi-internal.h"


/*
    // Non-timezone-affected time functions
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
#define OM_MAX_PACKET_SIZE 0xffff



/** The internal state tracker for a reader */
typedef struct
{
    // File pointer
    FILE *fp;

    // Buffers
    unsigned char header[OM_MAX_PACKET_SIZE];
    unsigned char data[OM_BLOCK_SIZE];
    short samples[OM_MAX_SAMPLES * 3];

    // Global information
    long dataOffset;
    long fileSize;
    int numDataBlocks;
    OM_DATETIME firstStartTime;
    OM_DATETIME lastEndTime;

    // Current block information
    unsigned int numAxes;		// Synchronous axes are [GxGyGz]AxAyAz[[MxMyMz]], 3=A, 6=GA, 9=GAM
	int accelScale;				// Scaling: number of units for 1g: CWA=256, AX6=2048 (+/-16g), 4096 (+/-8g), 8192 (+/-4g), 16384 (+/-2g)
	int gyroScale;				// Scaling: number of degrees per second that (2^15=)32768 represents: AX6= 2000, 1000, 500, 250, 125, 0=off.
    unsigned int numSamples;
    unsigned long long blockStart;
    unsigned long long blockEnd;
    unsigned int sequenceId;
    unsigned char events;
	
    // Output values
    unsigned short deviceId;
    unsigned int sessionId;
    unsigned char metaData[OM_METADATA_SIZE + 1];

} OmReaderState;


OmReaderHandle OmReaderOpen(const char *binaryFilename)
{ 
    OmReaderState *state;
    int initialReadSize;

    // Check parameters
    if (binaryFilename == NULL) { return NULL; }
    if (!strlen(binaryFilename)) { return NULL; }

    // Allocate state tracker
    state = (OmReaderState *)malloc(sizeof(OmReaderState));
    if (state == NULL) { return NULL; }    

    // Open source file
    state->fp = fopen(binaryFilename, "rb");
    if (state->fp == NULL) { free(state); return NULL; }    

    // Read file size
    fseek(state->fp, 0, SEEK_END);
    state->fileSize = ftell(state->fp);

    // Read header
    fseek(state->fp, 0, SEEK_SET);
    initialReadSize = (int)fread(state->header, 1, OM_MAX_PACKET_SIZE, state->fp);

    // Check header (at least as big as the header packet, actual length is an integer multiple of the size of each data block)
    if (initialReadSize < OM_BLOCK_SIZE) { fclose(state->fp); free(state); return NULL; }
    state->dataOffset = state->header[2] + (state->header[3] << 8) + 4;
    if (state->header[0] != 0x4d || state->header[1] != 0x44 || (state->dataOffset & (sizeof(OM_READER_DATA_PACKET) - 1)) != 0)
    {
        fclose(state->fp); free(state); return NULL;
    }

    // Extract the metadata from the header
    state->deviceId = (unsigned short)(state->header[5]) | ((unsigned short)state->header[6] << 8);
    state->sessionId = (unsigned int)(state->header[7]) | ((unsigned int)state->header[8] << 8) | ((unsigned int)state->header[9] << 16) | ((unsigned int)state->header[10] << 24);
    memcpy(state->metaData, &(state->header[64]), OM_METADATA_SIZE);
    state->metaData[OM_METADATA_SIZE] = '\0';

    // Calculate the number of data blocks
    state->numDataBlocks = (state->fileSize - state->dataOffset) / OM_BLOCK_SIZE;

    // Clear the current event and sequence counter
    state->events = 0;
    state->sequenceId = (unsigned int)-1;

    // Determine the start time from the first readable data block
    state->firstStartTime = OM_DATETIME_ZERO;
    if (state->numDataBlocks > 0)
    {
        int retry = 0;
        for (retry = 0; retry < state->numDataBlocks; retry++)
        {
            int values;
            if (retry > 16) { break; }      // Give up after 16 blocks
            OmReaderDataBlockSeek((OmReaderHandle)state, 0 + retry);
            values = OmReaderNextBlock((OmReaderHandle)state);
            if (values > 0)
            {
                state->firstStartTime = OmReaderTimestamp((OmReaderHandle)state, 0, NULL);
                break;
            }
        }
    }

    // Determine the end time from the last readable data block
    state->lastEndTime = OM_DATETIME_ZERO;
    if (state->numDataBlocks > 0)
    {
        int retry = 0;
        for (retry = 0; retry < state->numDataBlocks; retry++)
        {
            int values;
            if (retry > 16) { break; }      // Give up after 16 blocks
            OmReaderDataBlockSeek((OmReaderHandle)state, (state->fileSize / OM_BLOCK_SIZE) - 1 - retry);
            values = OmReaderNextBlock((OmReaderHandle)state);
            if (values > 0)
            {
                state->lastEndTime = OmReaderTimestamp((OmReaderHandle)state, values - 1, NULL);
                break;
            }
        }
    }

    // Seek ready to read the first data block
    OmReaderDataBlockSeek((OmReaderHandle)state, 0);

    return (OmReaderHandle)state;
}


int OmReaderDataRange(OmReaderHandle reader, int *dataBlockSize, int *dataOffsetBlocks, int *dataNumBlocks, OM_DATETIME *startTime, OM_DATETIME *endTime)
{
    OmReaderState *state = (OmReaderState *)reader;

    // Check parameter
    if (state == NULL) { return OM_E_POINTER; }

    // Output values
    if (dataBlockSize != NULL) { *dataBlockSize = OM_BLOCK_SIZE; }
    if (dataOffsetBlocks != NULL) { *dataOffsetBlocks = state->dataOffset / OM_BLOCK_SIZE; }
    if (dataNumBlocks != NULL) { *dataNumBlocks = state->numDataBlocks; }
    if (startTime != NULL) { *startTime = state->firstStartTime; }
    if (endTime != NULL) { *endTime = state->lastEndTime; }

    return OM_OK; 
}


const char *OmReaderMetadata(OmReaderHandle reader, int *deviceId, unsigned int *sessionId)
{
    OmReaderState *state = (OmReaderState *)reader;

    // Check parameter
    if (state == NULL) { return NULL; }

    // Output values
    if (deviceId != NULL) { *deviceId = state->deviceId; }
    if (sessionId != NULL) { *sessionId = state->sessionId; }

    return (const char *)state->metaData; 
}


int OmReaderDataBlockPosition(OmReaderHandle reader)
{
    int dataBlockNumber;
    OmReaderState *state = (OmReaderState *)reader;

    // Check parameter
    if (state == NULL) { return OM_E_POINTER; }

    // Get the data block position
    dataBlockNumber = (ftell(state->fp) - state->dataOffset) / OM_BLOCK_SIZE;

    // Return data block position
    return dataBlockNumber;
}


int OmReaderDataBlockSeek(OmReaderHandle reader, int dataBlockNumber)
{
    OmReaderState *state = (OmReaderState *)reader;

    // Check parameter
    if (state == NULL) { return OM_E_POINTER; }

    // Check values
    //if (dataBlockNumber < 0) { dataBlockNumber += state->numDataBlocks; }       // Negative values from end of file
    if (dataBlockNumber < -(state->dataOffset / OM_BLOCK_SIZE)) { return OM_E_FAIL; }
    if (dataBlockNumber > (state->fileSize / OM_BLOCK_SIZE)) { return OM_E_FAIL; }

    // Seek the file
    fseek(state->fp, state->dataOffset + dataBlockNumber * OM_BLOCK_SIZE, SEEK_SET);

    // Clear the data buffer and samples
    memset(state->data, 0xff, OM_BLOCK_SIZE);
    memset(state->samples, 0x00, OM_MAX_SAMPLES * 3 * sizeof(short));

    // Clear the sequence and time-tracking values
    state->sequenceId = (unsigned int)-1;
    state->events = 0;
    state->numSamples = 0;
    state->blockStart = 0;
    state->blockEnd = 0;

    return OM_OK;
}


int OmReaderNextBlock(OmReaderHandle reader)
{
    // unsigned long long previousBlockStart;
    unsigned long long previousBlockEnd;
    unsigned int sequenceId;
    char bytesPerSample;
    int sampleRate;
    int len;

    // Check parameter
    OmReaderState *state = (OmReaderState *)reader;
    if (state == NULL) { return OM_E_POINTER; }

    // Record previous block's 'blockStart' and 'blockEnd'
    // previousBlockStart = state->blockStart;
    previousBlockEnd = state->blockEnd;

    // No data unless we find a valid block
	state->numAxes = 0;
	state->accelScale = 256;
	state->gyroScale = 0;
    state->numSamples = 0;
    state->blockStart = 0;
    state->blockEnd = 0;
	
    // Read a block (if not EOF)
    len = -1;
    if (!feof(state->fp))
    {
        len = (int)fread(state->data, 1, OM_BLOCK_SIZE, state->fp);
    }

    if (len != OM_BLOCK_SIZE)
    {
        // Check if EOF
        if (len == 0 || len == -1)
        {
            if (ftell(state->fp) == state->fileSize)
            {
                return OM_E_FAIL;       // End-of-file as expected
            }
            return OM_E_UNEXPECTED;     // Unanticipated end-of-file (could events like device removal/network error/external file truncation cause this?)
        }
        // Other read error
        return OM_E_ACCESS_DENIED; 
    }

    // Check header and size
    if (state->data[0] != 0x41 || state->data[1] != 0x58) { return 0; }                                     // @0 packetHeader
    if (state->data[2] != ((OM_BLOCK_SIZE - 4) & 0xff) || state->data[3] != ((OM_BLOCK_SIZE - 4) >> 8)) { return 0; }   // @2 packetLength

    // Checksum -- 16-bit word-size addition
    {
	    unsigned short *p = (unsigned short *)state->data;
        unsigned short checksum = 0x0000;
	    size_t len;
        for (len = OM_BLOCK_SIZE / 2; len; --len) { checksum += *p++; }
        if (checksum != 0x0000) { return 0; }
    }

	state->numAxes = (state->data[25] >> 4);
	
	// light is least significant 10 bits, accel scale 3-MSB, gyro scale next 3 bits: AAAGGGLLLLLLLLLL
	unsigned short light = state->data[18] | (state->data[19] << 8);
	state->accelScale = 1 << (8 + ((light >> 13) & 0x03));
	state->gyroScale = 8000 / (1 << ((light >> 10) & 0x03));

    // Check bytes per sample
    if ((state->data[25] & 0x0f) == 0)
	{
		// Check number of axes (must be 3 for packed data)
		if (state->numAxes != 3) { return 0; }                                                          // @0 numAxesBPS:H
		bytesPerSample = 4;		// @0 numAxesBPS:L == 0 -- 3x 10-bit signed + 2-bit exponent
	}
    else if ((state->data[25] & 0x0f) == 2)
	{
		bytesPerSample = state->numAxes * 2;		// @0 numAxesBPS:L == 2 -- 3x 16-bit signed
	}
    else { return 0; }

    // Read sequence number and events
    sequenceId = ((unsigned int)state->data[10] << 0) | ((unsigned int)state->data[11] << 8) | ((unsigned int)state->data[12] << 16) | ((unsigned int)state->data[13] << 24);
    state->events = state->data[22];

    // Extract data values
    if (bytesPerSample == 4)
    {
        unsigned int i;
		
        // Check sample count matches expected number
        state->numSamples = (OM_BLOCK_SIZE - 32) / bytesPerSample;      // 120
        if (state->data[28] != (unsigned char)state->numSamples || state->data[29] != (unsigned char)(state->numSamples >> 8)) { return 0; }    // @28 sampleCount  

        // Unpack each value
        for (i = 0; i < state->numSamples; i++)
        {
            // Packed accelerometer value - must sign-extend each component value and adjust for exponent
            //        [byte-3] [byte-2] [byte-1] [byte-0]
            //        eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx
            //        10987654 32109876 54321098 76543210
            unsigned int value = (unsigned int)state->data[30 + i * 4] | ((unsigned int)state->data[31 + i * 4] << 8) | ((unsigned int)state->data[32 + i * 4] << 16) | ((unsigned int)state->data[33 + i * 4] << 24);
            state->samples[3 * i + 0] = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value <<  6)) >> (6 - ((unsigned char)(value >> 30))) );
            state->samples[3 * i + 1] = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value >>  4)) >> (6 - ((unsigned char)(value >> 30))) );
            state->samples[3 * i + 2] = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value >> 14)) >> (6 - ((unsigned char)(value >> 30))) );
        }
    }
    else	// 16-bit signed values
    {
        unsigned int i;

        // Check sample count matches expected number
        unsigned int maxSamples = (OM_BLOCK_SIZE - 32) / bytesPerSample;      // 80
		state->numSamples = state->data[28] | (state->data[29] << 8);	// @28 sampleCount  
		if (state->numSamples > maxSamples) { state->numSamples = maxSamples; }	// error instead?
		if (state->numSamples < 0) { state->numSamples = 0; }	// error instead?

        // Parse each value's bytes for portability
        for (i = 0; i < maxSamples; i++)
        {
			for (unsigned int j  = 0; j < state->numAxes; j++)
			{
				int value = 0;
				if (i < state->numSamples)
				{
					value = (short)((unsigned short)(state->data[30 + (i * state->numAxes + j) * 2] | (((unsigned short)state->data[31 + (i * state->numAxes + j) * 2]) << 8)));
				}
				state->samples[state->numAxes * i + j] = value;
			}
        }
    }

    // Frequency
// TODO: This method only works down to 25 Hz, use float to support 12.5, 6.25 rates
    sampleRate = 3200 / ((unsigned short)1 << (15 - (state->data[24] & 0x0f)));     // @24 sampleRate -- (3200/(1<<(15-(rate & 0x0f)))) Hz

    // Extract timestamps
    {
        OM_DATETIME timestamp;
        short timestampOffset;
        unsigned long long t;
        unsigned short fractionalPart;

        // timestamp        @14 +4  Last reported RTC value, 0 = unknown
        timestamp = (unsigned long)state->data[14] | ((unsigned long)state->data[15] << 8) | ((unsigned long)state->data[16] << 16) | ((unsigned long)state->data[17] << 24);
        fractionalPart = 0;

        // timestampOffset  @26 +2  (Signed) relative sample index from the start of the buffer where the whole-second timestamp is valid
        timestampOffset = (short)((unsigned short)state->data[26] | ((unsigned short)state->data[27] << 8));

#if 0
        {
            unsigned short deviceFractional = (unsigned short)((unsigned short)state->data[4] | ((unsigned short)state->data[5] << 8));
            // If we have a fractional offset
            if (deviceFractional & 0x8000)
            {
                // Need to undo backwards-compatible shim: Take into account how many whole samples the fractional part of timestamp accounts for:  relativeOffset = fifoLength - (short)(((unsigned long)timeFractional * AccelFrequency()) >> 16);
                // relativeOffset = fifoLength - (short)(((unsigned long)timeFractional * AccelFrequency()) >> 16);
                //                         nearest whole sample
                //          whole-sec       | /fifo-pos@time
                //           |              |/
                // [0][1][2][3][4][5][6][7][8][9]
                fractionalPart = ((deviceFractional & 0x7fff) << 1);	// use 15-bits as 16-bit fractional time
                // Remove the "ideal sample" offset that was estimated (for the whole part of the timestamp), now the offset will be for the full timestamp
                timestampOffset += (short)(((unsigned long)fractionalPart * (unsigned short)(sampleRate)) >> 16);
            }
        }
#endif

        // Calculate the time in ticks
        {
            time_t tSec;                            // Seconds since epoch
			// Time elements (YMDHMS)
            struct tm tParts =
			#ifdef _WIN32
				{0};
			#else
				{};
			#endif
            tParts.tm_year = OM_DATETIME_YEAR(timestamp) - 1900;
            tParts.tm_mon = OM_DATETIME_MONTH(timestamp) - 1;
            tParts.tm_mday = OM_DATETIME_DAY(timestamp);
            tParts.tm_hour = OM_DATETIME_HOURS(timestamp);
            tParts.tm_min = OM_DATETIME_MINUTES(timestamp);
            tParts.tm_sec = OM_DATETIME_SECONDS(timestamp);
            tSec = timegm(&tParts);                 // Pack from YMDHMS
            t = (unsigned long long)tSec << 16;     // Shift for fractional part
            t += fractionalPart;
        }

        // Calculate times at start and end of block
        // NOTE: This assumes the ideal sample rate
        state->blockStart = t - ((long long)timestampOffset * 0x10000 / sampleRate);
        state->blockEnd = state->blockStart + (unsigned long long)state->numSamples * 0x10000 / sampleRate;

        // If we are reading a block in sequence
        if (state->sequenceId != (unsigned int)-1 && state->sequenceId + 1 == sequenceId)
        {
            // If the previous block's "blockEnd" is close (+- 8%) to this block's "blockStart", use that value to smooth over any tiny jitter
            if (previousBlockEnd != 0 && state->blockStart != 0 && abs((int)(previousBlockEnd - state->blockStart)) < 8000)
            {
                state->blockStart = previousBlockEnd;
            }
        }
    }

    // Update sequence id
    state->sequenceId = sequenceId;

    return state->numSamples;
}


short *OmReaderBuffer(OmReaderHandle reader)
{ 
    OmReaderState *state = (OmReaderState *)reader;
    if (state == NULL) { return NULL; }
    return state->samples; 
}


OM_DATETIME OmReaderTimestamp(OmReaderHandle reader, int index, unsigned short *fractional)
{ 
    OmReaderState *state = (OmReaderState *)reader;
    unsigned long long t;       // Ticks since epoch (1/65536 seconds)
    time_t tSec = 0;            // Seconds since epoch
    struct tm tParts;           // Time elements (YMDHMS)
    OM_DATETIME dateTime;       // Output date/time
    unsigned short frac;        // Output fractional value (1/65536 seconds)

    // Check parameter
    if (state == NULL) { return OM_DATETIME_ZERO; }

    // Determine timestamp
    if (state->numSamples == 0) t = state->blockStart;
    else t = state->blockStart + (index * (state->blockEnd - state->blockStart) / state->numSamples);

    // Calculate date/time value
    tSec = (t >> 16);           // Remove fractional parts of the second
    gmtime_r(&tSec, &tParts);   // Unpack to YMDHMS
    if (tParts.tm_year < 100) { dateTime = OM_DATETIME_ZERO; }
    else { dateTime = OM_DATETIME_FROM_YMDHMS(1900 + tParts.tm_year, tParts.tm_mon + 1, tParts.tm_mday, tParts.tm_hour, tParts.tm_min, tParts.tm_sec); }

    // Calculate fractional value
    frac = (unsigned short)t;

    // Return values
    if (fractional != NULL) { *fractional = frac; }
    return dateTime; 
}


static unsigned int AdcBattToPercent(unsigned int Vbat)
{
    #define BATT_CHARGE_ZERO 614
    #define BATT_CHARGE_FULL 708
	#define BATT_FIT_CONST_1	666LU
	#define BATT_FIT_CONST_2	150LU
	#define BATT_FIT_CONST_3	538LU	
	#define BATT_FIT_CONST_4	8
	#define BATT_FIT_CONST_5	614LU
	#define BATT_FIT_CONST_6	375LU
	#define BATT_FIT_CONST_7	614LU	
	#define BATT_FIT_CONST_8	8
    #define USB_BUS_SENSE 0

	unsigned long temp; 
	
	// Compensate for charging current
	if (USB_BUS_SENSE && (Vbat>12)) Vbat -= 12; 
 
	// Early out functions for full and zero charge
	if (Vbat > BATT_CHARGE_FULL) return 100;
    if (Vbat < BATT_CHARGE_ZERO) return 0;

	// Calculations for curve fit
	if (Vbat>BATT_FIT_CONST_1)
	{
		temp = (BATT_FIT_CONST_2 * (Vbat - BATT_FIT_CONST_3))>>BATT_FIT_CONST_4;
	}
	else if (Vbat>BATT_FIT_CONST_5)
	{
		temp = (BATT_FIT_CONST_6 * (Vbat - BATT_FIT_CONST_7))>>BATT_FIT_CONST_8;
	}
	else 
	{
		temp = 0;
	}
    return (unsigned int)temp;
}


int OmReaderGetValue(OmReaderHandle reader, OM_READER_VALUE_TYPE valueType)
{ 
    OmReaderState *state = (OmReaderState *)reader;
    OM_READER_DATA_PACKET *dataPacket;

    // Check parameter
    if (state == NULL) { return -1; }

    //if (state->numSamples == 0)

    dataPacket = OmReaderRawDataPacket(reader);
    if (dataPacket == NULL) { return -1; }

    switch (valueType)
    {
		// TODO: WARNING: This is not endian-agnostic.
		
        // Raw values
        case OM_VALUE_DEVICEID:         if (dataPacket->deviceFractional & 0x8000) { return 0; } else { return dataPacket->deviceFractional; }
        case OM_VALUE_SESSIONID:        return dataPacket->sessionId;
        case OM_VALUE_SEQUENCEID:       return dataPacket->sequenceId;
        case OM_VALUE_LIGHT:            return dataPacket->light & 0x03ff;  		// Bottom 10 bits are light
        case OM_VALUE_TEMPERATURE:      return dataPacket->temperature & 0x03ff;	// Bottom 10 bits are temperature
        case OM_VALUE_EVENTS:           return dataPacket->events;
        case OM_VALUE_BATTERY:          return dataPacket->battery;
        case OM_VALUE_SAMPLERATE:       return dataPacket->sampleRate;
		
		// Calculated in OmReaderNextBlock() -- could recalculate here
        case OM_VALUE_AXES:             return state->numAxes;			// Synchronous axes are [GxGyGz]AxAyAz[[MxMyMz]], 3=A, 6=GA, 9=GAM
        case OM_VALUE_SCALE_ACCEL:      return state->accelScale;		// Scaling: number of units for 1g: CWA=256, AX6=2048 (+/-16g), 4096 (+/-8g), 8192 (+/-4g), 16384 (+/-2g)
        case OM_VALUE_SCALE_GYRO:       return state->gyroScale;		// Scaling: number of degrees per second that (2^15=)32768 represents: AX6= 2000, 1000, 500, 250, 125, 0=off.

        // Cooked values
        case OM_VALUE_LIGHT_LOG10LUXTIMES10POWER3: return (((dataPacket->light & 0x03ff) + 512) * 6000 / 1024); // log10(lux) * 10^3   therefore   lux = pow(10.0, log10LuxTimes10Power3 / 1000.0)
		case OM_VALUE_TEMPERATURE_MC:   return (int)(dataPacket->temperature & 0x03ff) * 75000 / 256 - 50000; // For MCP9700 // ((dataPacket->temperature & 0x03ff) * 150 - 20500);     // Scaled to millicentigrade from the 0.1 dC conversion for MCP9701 in Analog.c: (value * 3 / 2) - 205
        case OM_VALUE_BATTERY_MV:       return ((dataPacket->battery + 512) * 6000 / 1024); // Conversion to millivolts:  Vref = 3V, Vbat = 6V * value / 1024
        case OM_VALUE_BATTERY_PERCENT:  return AdcBattToPercent(dataPacket->battery + 512); // Conversion to percentage

        // Default
        default:                    return OM_E_FAIL;
    }
}


OM_READER_HEADER_PACKET *OmReaderRawHeaderPacket(OmReaderHandle reader) 
{
    if (reader == NULL) { return NULL; }
    return (OM_READER_HEADER_PACKET *)&(((OmReaderState *)reader)->header[0]); 
}


OM_READER_DATA_PACKET *OmReaderRawDataPacket(OmReaderHandle reader) 
{ 
    if (reader == NULL) { return NULL; }
    return (OM_READER_DATA_PACKET *)&(((OmReaderState *)reader)->data[0]); 
}


void OmReaderClose(OmReaderHandle reader)
{ 
    OmReaderState *state = (OmReaderState *)reader;

    if (state == NULL) { return; }
    if (state->fp != NULL)
    {
        fclose(state->fp);
        state->fp = NULL;
    }
    free(state);
    return; 
}

