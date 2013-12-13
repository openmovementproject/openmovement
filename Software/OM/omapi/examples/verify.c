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

/** 
 *  @file verify.c
 *  @brief     Open Movement API Example: Verify the contents of a binary data file
 *  @author    -
 *  @date      2011-2012
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2012, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  A command-line tool to verify a specified binary data file contains sensible data.
 *
 *  @remarks Makes use of \ref reader
 */



/* Headers */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>

#ifdef _WIN32
#define gmtime_r(timer, result) gmtime_s(result, timer)
#define timegm _mkgmtime
#define tzset _tzset
#endif



/* API header */
#include "omapi.h"


/* LEDs */
#define LED_PROCESSING  OM_LED_YELLOW
#define LED_OK          OM_LED_MAGENTA
#define LED_WARNING     OM_LED_MAGENTA
#define LED_FAILED      OM_LED_BLUE
#define LED_ERROR_COMMS OM_LED_CYAN

/* Options */
#define VERIFY_OPTION_ALL               0x01
#define VERIFY_OPTION_NO_CHECK_STOP     0x02
#define VERIFY_OPTION_OUTPUT_NEW        0x04

/* Error measures */
#define STUCK_COUNT (12 * 120)
#define AVERAGE_FACTOR 0.00001
#define AVERAGE_RANGE_MAX 0.400
#define AVERAGE_RANGE_OFF 0.300

#define IGNORE_RECENT_RESTARTS (6*60*60)        // Enable this if the devices have been connected/disconnected recently (causing a break in the data)



#define ID_NAND

typedef struct
{
    int memoryHealth;
#ifdef ID_NAND
    unsigned char nandId[6];
    int nandType;
#endif
    char *filename;
    // ???!!!
} download_t;

static int globalOptions = 0;
static int globalAllowedRestarts = 0;

#ifdef ID_NAND
// "NANDID=%02x:%02x:%02x:%02x:%02x:%02x,%d\r\n", id[0], id[1], id[2], id[3], id[4], id[5], nandPresent
static const char NAND_DEVICE_DONT_CARE[6] = 	  {0x00};
static const char NAND_DEVICE_HY27UF084G2x[6] = {0xAD,0xDC,0x00};
static const char NAND_DEVICE_HY27UF084G2B[6] = {0xAD,0xDC,0x10,0x95,0x54,0x00};
static const char NAND_DEVICE_HY27UF084G2M[6] = {0xAD,0xDC,0x80,0x95,0xAD,0x00};

static int OmGetNandId(int deviceId, unsigned char *id, int *present, int *identified)
{
    int status;
    char response[256], *parts[16] = {0};
    unsigned char localId[6] = {0};
    int tempId[6] = {0};
    int i;

    status = OmCommand(deviceId, "\r\nSTATUS 7\r\n", response, 255, "NANDID=", 2000, parts, 15);
    if (OM_FAILED(status)) return status;
    // "NANDID=0"
    if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    if (sscanf(parts[1], "%x:%x:%x:%x:%x:%x", &tempId[0], &tempId[1], &tempId[2], &tempId[3], &tempId[4], &tempId[5]) != 6) { return OM_E_UNEXPECTED_RESPONSE; }
    for (i = 0; i < 6; i++) { localId[i] = (unsigned char)tempId[i]; }
    if (id != NULL) { memcpy(id, localId, 6); }
    if (identified != NULL) 
    {
        *identified = -2;
        if (strcmp(NAND_DEVICE_HY27UF084G2B, (char *)localId) == 0) { *identified = 1; }
        else if (strcmp(NAND_DEVICE_HY27UF084G2M, (char *)localId) == 0) { *identified = 2; }
    }
    if (parts[2] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    if (present != NULL) 
    {
        *present = atoi(parts[2]);
    }
    return OM_OK;
}
#endif


FILE *outfile;

// Calculate epoch the time in seconds from a packed time
time_t TimeSerial(OM_DATETIME timestamp)
{
    time_t tSec;                            // Seconds since epoch
    struct tm tParts = {0};                 // Time elements (YMDHMS)
    tParts.tm_year = OM_DATETIME_YEAR(timestamp) - 1900;
    tParts.tm_mon = OM_DATETIME_MONTH(timestamp) - 1;
    tParts.tm_mday = OM_DATETIME_DAY(timestamp);
    tParts.tm_hour = OM_DATETIME_HOURS(timestamp);
    tParts.tm_min = OM_DATETIME_MINUTES(timestamp);
    tParts.tm_sec = OM_DATETIME_SECONDS(timestamp);
    tSec = timegm(&tParts);                 // Pack from YMDHMS
    return tSec;
}


// Calculate the time in 1/65536th seconds from a packed time
unsigned long long Ticks(OM_DATETIME timestamp, unsigned short fractional)
{
    time_t tSec = TimeSerial(timestamp);
    return ((unsigned long long)tSec << 16) + fractional;
}

// Returns the number of milliseconds since the epoch
unsigned long long now(void)
{
    struct timeb tp;
    ftime(&tp);
    return (unsigned long long)tp.time * 1000 + tp.millitm;
}

// Returns a formatted date/time string for the specific number of milliseconds since the epoch
const char *formattedtime(unsigned long long milliseconds)
{
	static char output[] = "YYYY-MM-DD HH:MM:SS.fff";
	struct tm *today;
	struct timeb tp = {0};
	tp.time = (time_t)(milliseconds / 1000);
	tp.millitm = (unsigned short)(milliseconds % 1000);
	tzset();
	today = localtime(&(tp.time));
	sprintf(output, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 1900 + today->tm_year, today->tm_mon + 1, today->tm_mday, today->tm_hour, today->tm_min, today->tm_sec, tp.millitm);
    return output;
}



/* Conversion function */
int verify_process(int id, const char *infile, download_t *download, int globalOptions)
{
    char output = 0;
    OmReaderHandle reader;
    int batteryStartPercent = 0, batteryEndPercent = 0;
    int batteryMaxPercent = -1, batteryMinPercent = -1;
    unsigned long long veryFirstTime = 0;                   // First time in file
    unsigned long long firstTime = 0, lastTime = 0;         // First & last time in current recording block
    unsigned long long blockStart = 0;
    unsigned long long previousBlockEnd = 0;
    unsigned int minLight = 0xffff;
    unsigned int previousSequenceId = -1;
    int block;
    int day = 0;
    unsigned int totalSamples = 0;
    unsigned int errorFile = 0, errorEvent = 0, errorStuck = 0, errorRange = 0, errorBreaks = 0, errorRate = 0;
    double maxAv = 0.0, peakAv = 0.0;
    char first = 1;
    unsigned long long minInterval = 0, maxInterval = 0;
    unsigned long long lastBlockStart = 0;
    unsigned long long totalDuration = 0;
    float breakTime = 0.0f;
    int firstPacket;
    int lastSecond;
    int restarts = 0;
    OM_READER_HEADER_PACKET *hp;
    char outOfRange = 0;
    char label[256];
    char line[768];
    int retval;
    float percentPerHour = 0;
    // old statics
    short lx = 0, ly = 0, lz = 0, stuck = 0;
    char timeString[25];  /* "YYYY-MM-DD hh:mm:ss.000"; */
    int seconds, packetCount = 0;
    double av = 0.0;
    int lastHour = -1;
    char description[1024] = "";

    if (infile == NULL || infile[0] == '\0')
    {
        fprintf(stderr, "ERROR: File not specified\n");
        return -2;
    }

    fprintf(stderr, "FILE: %s\n", infile);
    sprintf(label, infile);
    if (id >= 0) { sprintf(label, "%d", id); }

    /* Open the binary file reader on the input file */
    reader = OmReaderOpen(infile);
    if (reader == NULL)
    {
        fprintf(stderr, "ERROR: Problem opening file: %s\n", infile);
        return -1;
    }

    /* Iterate over all of the blocks of the file */
    firstPacket = 1;
    lastSecond = -1;
    for (block = 0; ; block++)
    {
        OM_READER_DATA_PACKET *dp;
        unsigned long long interval;
        int numSamples;
        short *buffer;
        int i;

        /* Report progress: minute */
        if (block != 0 && block %    50 == 0) { fprintf(stderr, "."); }						/* Approx. 1 minute */
        //if (block != 0 && block %  3000 == 0) { fprintf(stderr, "|\n"); }					/* Approx. 1 hour */
        //if (block != 0 && block % 72000 == 0) { fprintf(stderr, "===\n"); }				/* Approx. 1 day */

        /* Read the next block */
        numSamples = OmReaderNextBlock(reader);

        /* Successful end of file */
        if (numSamples == OM_E_FAIL) { break; }

        /* Problem reading the file */
        if (numSamples < 0)
        {
            fprintf(stderr, "[ERROR: %d - %s]\n", numSamples, OmErrorString(numSamples)); 
            errorFile++;
            break;
        }

        /* Block has no valid data */
        if (numSamples == 0)
        {
            fprintf(stderr, "[WARNING: Missing packet -- probably checksum failed?]\n"); 
            errorFile++;
            continue;
        }

        /* Get the raw packet handle */
        dp = OmReaderRawDataPacket(reader);
        if (dp == NULL)
        {
            fprintf(stderr, "[ERROR: Cannot get raw data packet]\n"); 
            errorFile++;
            continue;
        }

        /* Check events mask */
        if (dp->events & 0xf0)
        {
            fprintf(stderr, "[EVENT-ERROR: 0x%02x]\n", dp->events); 
            errorEvent++;
        }

        // Read the block start time
        {
            unsigned int timestamp;
            unsigned short fractional;
            timestamp = OmReaderTimestamp(reader, 0, &fractional);
            blockStart = Ticks(timestamp, fractional);
        }

        {
            unsigned int light = OmReaderGetValue(reader, OM_VALUE_LIGHT);
            if (light < minLight) { minLight = light; }
        }

        /*
        fprintf(stderr, "\n[timestampOffset %04d-%02d-%02d %02d:%02d:%02d %+d * %.3f]", 
            OM_DATETIME_YEAR(timestamp), OM_DATETIME_MONTH(timestamp), OM_DATETIME_DAY(timestamp),
            OM_DATETIME_HOURS(timestamp), OM_DATETIME_MINUTES(timestamp), OM_DATETIME_SECONDS(timestamp),
            -timestampOffset, 1.0f / sampleRate);
            */

        // If we read a block out-of-sequence
        if (previousSequenceId != (unsigned int)-1 && previousSequenceId + 1 != dp->sequenceId)
        {
            if (dp->sequenceId != 0)
            {
                fprintf(stderr, "WARNING: Sequence break %u -> %u\n", previousSequenceId, dp->sequenceId);
            }
            else
            {
                long long recordingLength = (long long)(lastTime - firstTime);
                long long diff = (long long)(blockStart - previousBlockEnd);

#ifdef IGNORE_RECENT_RESTARTS
                time_t allowed = time(NULL) - IGNORE_RECENT_RESTARTS;
                struct tm *tm = localtime(&allowed);
                OM_DATETIME allowedRestartTime = OM_DATETIME_FROM_YMDHMS(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

                if (dp->timestamp >= allowedRestartTime)
                {
                    // Recalculate allowed restart time

                    fprintf(stderr, "NOTE: Permitted recording sequence restarted @%u, length of %+.2fs (gap of %+.2fs)\n", previousSequenceId, (float)recordingLength / 65536.0f, (float)diff / 65536.0f);
                }
                else
#endif
                {
                    fprintf(stderr, "NOTE: Recording sequence restarted @%u, length of %+.2fs (gap of %+.2fs)\n", previousSequenceId, (float)recordingLength / 65536.0f, (float)diff / 65536.0f);
                    restarts++;
                    breakTime += (float)diff / 65536.0f;
                }

                totalDuration += recordingLength;
                lastBlockStart = 0;
                firstPacket = 1;
                lastSecond = -1;
                firstTime = 0;           // Start of a new block
            }
        }

        if (firstTime == 0)
        {
            firstTime = blockStart;
            if (veryFirstTime == 0) { veryFirstTime = firstTime; }
            lastTime = firstTime;
            batteryStartPercent = OmReaderGetValue(reader, OM_VALUE_BATTERY_PERCENT);
            batteryEndPercent = batteryStartPercent;
            if (batteryMaxPercent < 0) { batteryMaxPercent = batteryStartPercent; }
            if (batteryMinPercent < 0) { batteryMinPercent = batteryStartPercent; }
        }

        if (lastBlockStart > 0)
        {
            // Interval
            interval = blockStart - lastBlockStart;

            // If the previous block's "blockEnd" is not close (+- 5%) to this block's "blockStart", we have a time discontinuity
            if (previousBlockEnd != 0 && blockStart != 0 && abs((int)(previousBlockEnd - blockStart)) >= 5000)
            {
                long long diff = (long long)(blockStart - previousBlockEnd);
                fprintf(stderr, "WARNING: Time break in sequence by %+.2f seconds (last: %f, curr: %f)\n", (float)diff / 65536.0f, lastBlockStart / 65536.0f, blockStart / 65536.0f);
                fprintf(stderr, "\n");
                breakTime += (float)diff / 65536.0f;
            }
            else
            {
                // Min/max interval
                if (first) { first = 0; minInterval = interval; maxInterval = interval; }
                if (interval > maxInterval) { maxInterval = interval; }
                if (interval < minInterval) { minInterval = interval; }
            }
        }

        lastBlockStart = blockStart;

        buffer = OmReaderBuffer(reader);
        for (i = 0; i < numSamples; i++)
        {
            OM_DATETIME dateTime;
            unsigned short fractional;
            short x, y, z;
            double v;

            totalSamples++;

            /* Get the date/time value for this sample, and the 1/65536th fractions of a second */
            dateTime = OmReaderTimestamp(reader, i, &fractional);
            sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 
                    OM_DATETIME_YEAR(dateTime), OM_DATETIME_MONTH(dateTime), OM_DATETIME_DAY(dateTime), 
                    OM_DATETIME_HOURS(dateTime), OM_DATETIME_MINUTES(dateTime), OM_DATETIME_SECONDS(dateTime), 
                    (int)fractional * 1000 / 0xffff);

            /* Get the x/y/z/ values */
            x = buffer[3 * i + 0];
            y = buffer[3 * i + 1];
            z = buffer[3 * i + 2];

            /* Detect stuck values */
            if (x == lx && y == ly && z == lz)
            { 
                stuck++;
                if (stuck == STUCK_COUNT)
                {
                    fprintf(stderr, "\nERROR: Readings could be stuck at (%d, %d, %d); has been for %d consecutive readings. ", lx, ly, lz, stuck);
                    errorStuck++;
                }
            }
            else
            {
                if (stuck >= STUCK_COUNT)
                {
                    fprintf(stderr, "\nNOTE: Readings now changed -- were at (%d, %d, %d) for total of %d consecutive readings. ", lx, ly, lz, stuck);
                }
                lx = x; ly = y; lz = z;
                stuck = 0;
            }

            /* Get the SVM - 1 */
            v = sqrt((x / 256.0) * (x / 256.0) + (y / 256.0) * (y / 256.0) + (z / 256.0) * (z / 256.0)) - 1.0;
            av = ((1.0 - AVERAGE_FACTOR) * av) + (AVERAGE_FACTOR * v);
            if (fabs(av) > peakAv) { peakAv = fabs(av); }
            if (fabs(av) > maxAv) { maxAv = fabs(av); }
            if (fabs(av) > AVERAGE_RANGE_MAX)
            {
                if (!outOfRange)
                {
                    errorRange++;
                    outOfRange = 1;
                    peakAv = fabs(av);
                    fprintf(stderr, "WARNING: Average SVM gone out-of-normal-range abs(avg(svm-1)): %0.3f\n", fabs(av));
                }
            } 
            else if (fabs(av) < AVERAGE_RANGE_OFF && outOfRange) 
            { 
                outOfRange = 0; 
                fprintf(stderr, "WARNING: Average SVM back in normal range, peak was abs(avg(svm-1)): %0.3f\n", peakAv);
            }

            /* Output the data */
//            if (output) { fprintf(stdout, "%s,%d,%d,%d\n", timeString, x, y, z); }

            {
                int hour = OM_DATETIME_HOURS(dateTime);
                if (hour != lastHour)
                {
                    fprintf(stderr, "\n%02d-%02d %02d:%02d:%02d.%02d", /*OM_DATETIME_YEAR(dateTime) % 100, */ OM_DATETIME_MONTH(dateTime), OM_DATETIME_DAY(dateTime), OM_DATETIME_HOURS(dateTime), OM_DATETIME_MINUTES(dateTime), OM_DATETIME_SECONDS(dateTime), (int)fractional * 100 / 0xffff);
                    lastHour = hour;
                }
            }

            seconds = OM_DATETIME_SECONDS(dateTime);
            if (lastSecond == -1) { lastSecond = seconds; };
            if (seconds != lastSecond)
            {
                if (firstPacket) { /* fprintf(stderr, "!"); */ firstPacket = 0; }
                else if (packetCount >= 88 && packetCount <= 112) { /* fprintf(stderr, "#"); */ }
                else
                { 
                    if (seconds == lastSecond + 1 || (seconds == 0 && lastSecond == 59))
                    {
                        errorRate++;
                        fprintf(stderr, "WARNING: 'Out of range' %d samples in second :%02d before %s]\n", packetCount, lastSecond, timeString);
                    }
                    else
                    {
                        errorBreaks++;
                        fprintf(stderr, "WARNING: Non-sequential second jump (%d samples in second :%02d before %s)]\n", packetCount, lastSecond, timeString);
                    }
                }
                lastSecond = seconds;
                packetCount = 0;
            }
            packetCount++;

        }

        // Store the current sequence id
        previousSequenceId = dp->sequenceId;

        // Read the block end time
        {
            unsigned int timestamp;
            unsigned short fractional;
            timestamp = OmReaderTimestamp(reader, numSamples, &fractional);
            previousBlockEnd = Ticks(timestamp, fractional);
        }

        lastTime = previousBlockEnd;
        batteryEndPercent = OmReaderGetValue(reader, OM_VALUE_BATTERY_PERCENT);
        if (batteryEndPercent > batteryMaxPercent) { batteryMaxPercent = batteryEndPercent; }
        if (batteryEndPercent < batteryMinPercent) { batteryMinPercent = batteryEndPercent; }

    }

    /* Last session's duration */
    {
        long long recordingLength = (long long)(lastTime - firstTime);
        totalDuration += recordingLength;
    }

    /* Summary */
    {
        int startStopFail = 0;

        hp = OmReaderRawHeaderPacket(reader);

        fprintf(stderr, "\n");

        if (hp == NULL)
        {
            fprintf(stderr, "ERROR: Unable to access file header for start/stop times.\n");
            startStopFail += 4;
        }
        else
        {
            if (hp->loggingStartTime >= OM_DATETIME_MIN_VALID && hp->loggingStartTime <= OM_DATETIME_MAX_VALID && hp->loggingEndTime > hp->loggingStartTime)
            {
                unsigned long fStart = (unsigned long)TimeSerial(hp->loggingStartTime);
                unsigned long aStart = (unsigned long)(veryFirstTime >> 16);
                int startDiff = (int)(aStart - fStart);
                if (abs(startDiff) < 80)
                { 
                    fprintf(stderr, "NOTE: Data started near recording start time (%ds).\n", startDiff);
                }
                else
                {
                    fprintf(stderr, "ERROR: Data did not start near recording start time (%ds).\n", startDiff);
                    startStopFail += 2;
                }
            }
            else
            {
                fprintf(stderr, "NOTE: Recording has no start time (not verifying).\n");
            }
            
            
            if (hp->loggingEndTime >= OM_DATETIME_MIN_VALID && hp->loggingEndTime <= OM_DATETIME_MAX_VALID && hp->loggingStartTime < hp->loggingEndTime)
            {
                unsigned long fEnd = (unsigned long)TimeSerial(hp->loggingEndTime);
                unsigned long aEnd = (unsigned long)(lastTime >> 16);
                int stopDiff = (int)(aEnd - fEnd);
                if (globalOptions & VERIFY_OPTION_NO_CHECK_STOP)
                {
                    fprintf(stderr, "NOTE: Ignoring whether data stopped near recording stop time (difference was %ds).\n", stopDiff);
                }
                else if (abs(stopDiff) < 80)
                { 
                    fprintf(stderr, "NOTE: Data stopped near recording stop time (%ds).\n", stopDiff);
                }
                else
                {
                    fprintf(stderr, "ERROR: Data did not stop near recording stop time (%ds).\n", stopDiff);
                    startStopFail += 1;
                }
            }
            else
            {
                fprintf(stderr, "NOTE: Recording has no stop time (not verifying).\n");
            }
        }

// Error mask
#define CODE_ERROR_MASK         0xfffff000
#define CODE_WARNING_MASK       0x00000fff

// FILE - Read file issue (sector incorrect or checksum mismatch)
#define CODE_WARNING_FILE       0x000001
#define CODE_ERROR_FILE         0x001000

// EVENT - System logged event error (e.g. FIFO overflow)
#define CODE_WARNING_EVENT      0x000002
#define CODE_ERROR_EVENT        0x002000

// STUCK - Where the accelerometer appears to be stuck at the same value 
#define CODE_WARNING_STUCK      0x000004
#define CODE_ERROR_STUCK        0x004000

// RANGE - Where the accelerometer doesn't seem to average to 1G
#define CODE_WARNING_RANGE      0x000008
#define CODE_ERROR_RANGE        0x008000

// RATE - Where the rate is too far off 100Hz
#define CODE_WARNING_RATE       0x000010
#define CODE_ERROR_RATE         0x010000

// BREAKS - Where there is a gap in the data
#define CODE_WARNING_BREAKS     0x000020
#define CODE_ERROR_BREAKS       0x020000

// RESTARTS - Where the device has restarted
#define CODE_WARNING_RESTARTS   0x000040
#define CODE_ERROR_RESTARTS     0x040000

// LIGHT - Where the light level reading is far lower than normal (seems to indicate a failure)
#define CODE_WARNING_LIGHT      0x000080
#define CODE_ERROR_LIGHT        0x080000

// BATT - Where the battery discharge is more rapid than usual
#define CODE_WARNING_BATT       0x000100
#define CODE_ERROR_BATT         0x100000

// STARTSTOP - Where the configured start/stop times are not met
#define CODE_WARNING_STARTSTOP  0x000200
#define CODE_ERROR_STARTSTOP    0x200000

// NANDHEALTH - Where the number of reported spare blocks are low
#define CODE_WARNING_NANDHEALTH 0x000400
#define CODE_ERROR_NANDHEALTH   0x400000

// NANDID - Where the NAND id is unexpected
#define CODE_WARNING_NANDID     0x000800
#define CODE_ERROR_NANDID       0x800000


retval = 0;
if (errorFile   > 0)  { retval |= CODE_ERROR_FILE; }     else if (errorFile > 0)   { retval |= CODE_WARNING_FILE; }
if (errorEvent  > 0)  { retval |= CODE_ERROR_EVENT; }    else if (errorEvent > 0)  { retval |= CODE_WARNING_EVENT; }
if (errorStuck  > 0)  { retval |= CODE_ERROR_STUCK; }    else if (errorStuck > 0)  { retval |= CODE_WARNING_STUCK; }
if (errorRange  > 0)  { retval |= CODE_ERROR_RANGE; }    else if (errorRange > 0)  { retval |= CODE_WARNING_RANGE; }
if (errorRate   > 0)  { retval |= CODE_ERROR_RATE; }     else if (errorRate > 0)   { retval |= CODE_WARNING_RATE; }
if (errorBreaks > 0)  { retval |= CODE_ERROR_BREAKS; }   else if (errorBreaks > 0) { retval |= CODE_WARNING_BREAKS; }
if (restarts    > globalAllowedRestarts) { retval |= CODE_ERROR_RESTARTS; } else if (restarts > 0) { retval |= CODE_WARNING_RESTARTS; }
if (minLight    < 90) { retval |= CODE_ERROR_LIGHT; }    else if (minLight < 140)  { retval |= CODE_WARNING_LIGHT; }
// Discharge
{
    float hours = ((totalDuration >> 16) / 60.0f / 60.0f);
    float percentLoss = (float)batteryMaxPercent - batteryMinPercent;
    percentPerHour = 0;
    if (hours > 0) { percentPerHour = percentLoss / hours; } else { percentPerHour = 0; }
    if (percentPerHour >= 0.26f) { retval |= CODE_ERROR_BATT; } else if (percentPerHour >= 0.25f)  { retval |= CODE_WARNING_BATT; }
}
// Start/stop
if (startStopFail) { retval |= CODE_ERROR_STARTSTOP; } 

        fprintf(stderr, "---\n");
        fprintf(stderr, "Input file,%d,\"%s\",%d\n", id, infile, retval);
        fprintf(stderr, "Summary errors: file=%d, event=%d, stuck=%d, range=%d, rate=%d, breaks=%d\n", errorFile, errorEvent, errorStuck, errorRange, errorRate, errorBreaks);
        fprintf(stderr, "Summary info-1: restart=%d, breakTime=%0.1fs, maxAv=%f\n", restarts, breakTime, maxAv);
        fprintf(stderr, "Summary info-2: minInterval=%0.3f, maxInterval=%0.3f, duration=%0.4fh\n", minInterval / 65536.0f, maxInterval / 65536.0f, ((totalDuration >> 16) / 60.0f / 60.0f));
        fprintf(stderr, "Summary info-3: minLight=%d, Bmax=%d%%, Bmin=%d%%, intervalFail=%d\n", minLight, batteryMaxPercent, batteryMinPercent, startStopFail);

if (download != NULL)
{
    // Spare blocks
    if (download->memoryHealth < OM_MEMORY_HEALTH_ERROR)        { retval |= CODE_ERROR_NANDHEALTH; }    // ERROR: No spare blocks
    else if (download->memoryHealth < OM_MEMORY_HEALTH_WARNING) { retval |= CODE_WARNING_NANDHEALTH; }  // WARNING: Very few spare blocks

#ifdef ID_NAND
    // NAND ID
    if (download->nandType == -1) { ; }                                                 // Firmware doesn't support nand Id
    else if (download->nandType != 1 && download->nandType != 2) { retval |= CODE_ERROR_NANDID; }   // ERROR: Not primary or secondary type
    //else if (download->nandType != 1) { retval |= CODE_WARNING_NANDID; }                            // WARNING: Not primary type

    fprintf(stderr, "NAND #%d (%d spare)\n", download->nandType, download->memoryHealth);
#endif
}

        fprintf(stderr, "---\n");

#define HEADER        "VERIFY," "id,"  "summary,"  "file,"    "event,"    "stuck,"    "range,"    "rate,"    "breaks,"    "restarts," "breakTime," "maxAv,"    "minInterval,"          "maxInterval,"           "duration,"                             "minLight," "batteryMaxPercent," "batteryMinPercent," "intervalFail," "percentLoss," "description\n"
        {
			description[0] = '\0';

            if (retval & CODE_WARNING_FILE      ) { strcat(description, "W:File;"); }
            if (retval & CODE_WARNING_EVENT     ) { strcat(description, "W:Event;"); }
            if (retval & CODE_WARNING_STUCK     ) { strcat(description, "W:Stuck;"); }
            if (retval & CODE_WARNING_RANGE     ) { strcat(description, "W:Range;"); }
            if (retval & CODE_WARNING_RATE      ) { strcat(description, "W:Rate;"); }
            if (retval & CODE_WARNING_BREAKS    ) { strcat(description, "W:Breaks;"); }
            if (retval & CODE_WARNING_RESTARTS  ) { strcat(description, "W:Restarts;"); }
            if (retval & CODE_WARNING_LIGHT     ) { char temp[32]; sprintf(temp, "W:Light(%d);", minLight); strcat(description, temp); }
            if (retval & CODE_WARNING_BATT      ) { char temp[32]; sprintf(temp, "W:Batt(%2.2f);", percentPerHour); strcat(description, temp); }
            if (retval & CODE_WARNING_STARTSTOP ) { strcat(description, "W:StartStop;"); }
            if (retval & CODE_WARNING_NANDHEALTH) { strcat(description, "W:NandHealth;"); }
            if (retval & CODE_WARNING_NANDID    ) { strcat(description, "W:NandId;"); }
            if (retval & CODE_ERROR_FILE        ) { strcat(description, "E:File;"); }
            if (retval & CODE_ERROR_EVENT       ) { strcat(description, "E:Event;"); }
            if (retval & CODE_ERROR_STUCK       ) { strcat(description, "E:Stuck;"); }
            if (retval & CODE_ERROR_RANGE       ) { strcat(description, "E:Range;"); }
            if (retval & CODE_ERROR_RATE        ) { strcat(description, "E:Rate;"); }
            if (retval & CODE_ERROR_BREAKS      ) { strcat(description, "E:Breaks;"); }
            if (retval & CODE_ERROR_RESTARTS    ) { strcat(description, "E:Restarts;"); }
            if (retval & CODE_ERROR_LIGHT       ) { char temp[32]; sprintf(temp, "E:Light(%d);", minLight); strcat(description, temp); }
            if (retval & CODE_ERROR_BATT        ) { char temp[32]; sprintf(temp, "E:Batt(%2.2f);", percentPerHour); strcat(description, temp); }
            if (retval & CODE_ERROR_STARTSTOP   ) { strcat(description, "E:StartStop;"); }
            if (retval & CODE_ERROR_NANDHEALTH  ) { strcat(description, "E:NandHealth;"); }
            if (retval & CODE_ERROR_NANDID      ) { strcat(description, "E:NandId;"); }

        sprintf(line, "VERIFYX," "%s,"  "%d,"       "%d,"      "%d,"       "%d,"       "%d,"       "%d,"      "%d,"        "%d,"       "%.1f,"      "%.4f,"     "%.3f,"                 "%.3f,"                  "%.4f,"                                 "%d,"       "%d,"                "%d,"                "%d,"           "%f,"          "%s\n",
                                 label, retval,     errorFile, errorEvent, errorStuck, errorRange, errorRate, errorBreaks, restarts,   breakTime,   maxAv,       minInterval / 65536.0f, maxInterval / 65536.0f, ((totalDuration >> 16) / 60.0f / 60.0f), minLight,   batteryMaxPercent,   batteryMinPercent,   startStopFail, percentPerHour, description);
        }

        fprintf(stdout, line);
        fprintf(stderr, line);
        if (outfile != NULL)
        { 
			// New output format
			if (globalOptions & VERIFY_OPTION_OUTPUT_NEW)
			{
				int passed = ((retval & CODE_ERROR_MASK) == 0) ? 1 : 0;
				// "VERIFY,YYYY-MM-DD hh:mm:ss.000,12345,1,260,W:Batt;W:Stuck;"
				sprintf(line, "VERIFY,%s,%s,%d,%d,%s\n", formattedtime(now()), label, passed, retval, description);

		        fprintf(stderr, line);
			}

			fprintf(stdout, line);

            fprintf(outfile, line); 
            fflush(outfile);
        }
        printf(line);
    }

    /* Close the files */
    OmReaderClose(reader);
    return retval;
}





/* Download updated */
static void verify_DownloadCallback(void *reference, int deviceId, OM_DOWNLOAD_STATUS status, int value)
{
    download_t *download = (download_t *)reference;

    if (status == OM_DOWNLOAD_PROGRESS)
    { 
        //printf("VERIFY #%d: Progress %d%%.\n", deviceId, value);
        if ((value % 5) == 0) { printf("<#%d@%d%%>", deviceId, value); } // report every 5%
    }
    else if (status == OM_DOWNLOAD_COMPLETE)
    { 
        const char *file = download->filename;
        int verifyResult;
        int result;

        printf("VERIFY #%d: Download complete, verify starting... (options 0x%04x)\n", deviceId, globalOptions);

        verifyResult = verify_process(deviceId, file, download, globalOptions);

        if (verifyResult < 0)								
		{ 
			char line[128];
			sprintf(line, "#ERROR,%s,%d,Verify failed (%d)\n", formattedtime(now()), deviceId, verifyResult);
			fprintf(stderr, line);
			fprintf(stdout, line);
			if (outfile != NULL) { fprintf(outfile, line); fflush(outfile); }
			OmSetLed(deviceId, LED_ERROR_COMMS);
		}
        else if (verifyResult & CODE_ERROR_MASK)
		{ 
			OmSetLed(deviceId, LED_FAILED);					// Looks like a problem
		}
        else 
		{
			/* Set the session id (use the deviceId) */
			result = OmSetSessionId(deviceId, 0);
			fprintf(stderr, "VERIFY #%d: Setting session id: %u\n", deviceId, 0);
			if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmSetSessionId() %s\n", OmErrorString(result)); }

			/* Set the delayed start/stop times */
			fprintf(stderr, "VERIFY #%d: Setting start/stop: INFINITY/ZERO\n", deviceId);
			result = OmSetDelays(deviceId, OM_DATETIME_INFINITE, OM_DATETIME_ZERO);
			if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmSetDelays() %s\n", OmErrorString(result)); }

			/* Commit the new settings */
			fprintf(stderr, "VERIFY #%d: Committing new settings...\n", deviceId);
			result = OmEraseDataAndCommit(deviceId, OM_ERASE_QUICKFORMAT);
			if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmEraseDataAndCommit() %s\n", OmErrorString(result)); }

			// Set LEDs
			if (verifyResult & CODE_WARNING_MASK) { OmSetLed(deviceId, LED_WARNING); }            // Warning
			else { OmSetLed(deviceId, LED_OK); }                                           // Everything ok
		}

    }
    else if (status == OM_DOWNLOAD_CANCELLED)
    { 
		char line[128];
		sprintf(line, "#ERROR,%s,%d,Download cancelled\n", formattedtime(now()), deviceId);
		fprintf(stderr, line);
		fprintf(stdout, line);
		if (outfile != NULL) { fprintf(outfile, line); fflush(outfile); }
        OmSetLed(deviceId, LED_ERROR_COMMS);
    }
    else if (status == OM_DOWNLOAD_ERROR) 
    { 
		char line[128];
		sprintf(line, "#ERROR,%s,%d,Download error (diagnostic 0x%04x)\n", formattedtime(now()), deviceId, value);
		fprintf(stderr, line);
		fprintf(stdout, line);
		if (outfile != NULL) { fprintf(outfile, line); fflush(outfile); }
        OmSetLed(deviceId, LED_ERROR_COMMS);
    }
    else
    {
		char line[128];
		sprintf(line, "#ERROR,%s,%d,Unexpected status %d / 0x%04x\n", formattedtime(now()), deviceId, status, value);
		fprintf(stderr, line);
		fprintf(stdout, line);
		if (outfile != NULL) { fprintf(outfile, line); fflush(outfile); }
        OmSetLed(deviceId, LED_ERROR_COMMS);
    }


	// On any download event other than progress, alert the user
    if (status != OM_DOWNLOAD_PROGRESS)
	{
#ifdef _WIN32
		{
			char oldTitle[2048];
			char newTitle[] = "[ALERT]";
			HWND hWnd = NULL;
			FLASHWINFO fwi = {0};
			GetConsoleTitle(oldTitle, sizeof(oldTitle));
			SetConsoleTitle(newTitle);

			fprintf(stderr, "\a");
			MessageBeep(0xffffffff);
			Sleep(40);

			hWnd = FindWindow(NULL, newTitle);
		    SetConsoleTitle(oldTitle);
			if (hWnd != NULL)
			{
				fwi.cbSize = sizeof(fwi);
				fwi.hwnd = hWnd;
				fwi.dwFlags = FLASHW_ALL;
				fwi.uCount = 3;
				fwi.dwTimeout = 0;
				FlashWindowEx(&fwi);
			}
		}
#else
		fprintf(stderr, "\a");
#endif
	}

    return;
}


/* Device updated */
static void verify_DeviceCallback(void *reference, int deviceId, OM_DEVICE_STATUS status)
{
    if (status == OM_DEVICE_CONNECTED)
    {
        int result;
        int dataBlockSize = 0, dataOffsetBlocks = 0, dataNumBlocks = 0;
        OM_DATETIME startTime = 0, endTime = 0;

        printf("VERIFY #%d: Device CONNECTED (verify options 0x%04x)\n", deviceId, globalOptions);

        /* Get the data range */
        result = OmGetDataRange(deviceId, &dataBlockSize, &dataOffsetBlocks, &dataNumBlocks, &startTime, &endTime);
        if (OM_FAILED(result)) { printf("ERROR: OmGetDataRange() %s\n", OmErrorString(result)); return; }
        printf("VERIFY #%d: %d blocks data, at offset %d blocks (1 block = %d bytes)\n", deviceId, dataNumBlocks, dataOffsetBlocks, dataBlockSize);
        /* Display the data start and end times */
        {
            char startString[OM_DATETIME_BUFFER_SIZE], endString[OM_DATETIME_BUFFER_SIZE];
            OmDateTimeToString(startTime, startString);
            OmDateTimeToString(endTime, endString);
            printf("VERIFY #%d: ... %s --> %s\n", deviceId, startString, endString);
        }

        /* Check if there's any data blocks stored (not just the headers) */
        if (dataNumBlocks - dataOffsetBlocks <= 0)
        {
            printf("VERIFY #%d: Ignoring - no data stored.\n");
            OmSetLed(deviceId, LED_ERROR_COMMS);                   // Error accessing data
        }
        else
        {
            const char *downloadPath = ".";
            /* Create reference handle */
            download_t *download = (download_t *)malloc(sizeof(download_t));

            /* Get NAND information */
            download->memoryHealth = OmGetMemoryHealth(deviceId);
#ifdef ID_NAND
            download->nandType = -1;
            memset(download->nandId, 0, sizeof(download->nandId));
            result = OmGetNandId(deviceId, download->nandId, NULL, &download->nandType);
            if (result == OM_E_NOT_IMPLEMENTED) { fprintf(stderr, "NOTE: This firmware doesn't support NANDID command\n"); }
            else if (result == OM_E_UNEXPECTED_RESPONSE) { fprintf(stderr, "NOTE: Unexpected NANDID response (firmware probably doesn't support NANDID command)\n"); }
#endif

            /* Allocate filename string */
            download->filename = (char *)malloc(strlen(downloadPath) + 32);  /* downloadPath + "/4294967295-65535.cwa" + 1 (NULL-terminated) */

            /* Copy path, and platform-specific path separator char */
            strcpy(download->filename, downloadPath);
            #ifdef _WIN32
                if (download->filename[strlen(download->filename) - 1] != '\\') strcat(download->filename, "\\");
            #else
                if (download->filename[strlen(download->filename) - 1] != '/') strcat(download->filename, "/");
            #endif

            /* Append session-id and device-id as part of the filename */
            sprintf(download->filename + strlen(download->filename), "CWA-%04u.CWA", deviceId);

            /* Setup */
            OmSetLed(deviceId, LED_PROCESSING);

            /* Begin download */
            printf("VERIFY #%d: Starting download to file: %s\n", deviceId, download->filename);
            {
                result = OmBeginDownloadingReference(deviceId, 0, -1, download->filename, download);
            }
            if (OM_FAILED(result)) { printf("ERROR: OmBeginDownloading() %s\n", OmErrorString(result)); }

            /* Leave filename string for download complete to free... */
        }
    }
    else if (status == OM_DEVICE_REMOVED)
    {
        printf("VERIFY #%d: Device REMOVED\n", deviceId);
        /* The download will have already been cancelled in the event of a device removal */
        /*OmCancelDownload(deviceId);*/
    }
    else
    {
        printf("VERIFY #%d: Error, unexpected status %d\n", deviceId, status);
        OmSetLed(deviceId, LED_ERROR_COMMS);                   // Error accessing data
    }

    return;
}



/* Record function */
int verify(void)
{
    int result;

    /* Set device callback before API startup to get initially-connected devices through the callback */
    OmSetDeviceCallback(verify_DeviceCallback, NULL);

    /* Set download callback */
    OmSetDownloadCallback(verify_DownloadCallback, NULL);

    /* Start the API */
    result = OmStartup(OM_VERSION);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmStartup() %s\n", OmErrorString(result)); return -1; }

    for (;;)
    {
        /* Wait 5 seconds */
        Sleep(60000);
        fprintf(stderr, "<.>");
    }

    /* Shutdown the API */
    result = OmShutdown();
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmShutdown() %s\n", OmErrorString(result)); return -1; }

    /* Close the input and output files */
    if (outfile != NULL) { fclose(outfile); }
}


/* Main function */
int verify_main(int argc, char *argv[])
{
    const char *outfilename = NULL;
    int ret = -1;
    int i;
    fprintf(stderr, "VERIFY: verify a specified binary data file contains sensible data.\n");
    fprintf(stderr, "\n");
    globalOptions = VERIFY_OPTION_OUTPUT_NEW;
    if (argc > 1)
    {
        const char *infile = NULL;
        //char output = 0;

        for (i = 1; i < argc; i++)
        {
            if (!strcmp(argv[i], "-headeronly"))
            {
                fprintf(stdout, HEADER);
                return -2;
            }
            else if (!strcmp(argv[i], "-stop-clear-all")) { fprintf(stderr, "VERIFY: Option -stop-clear-all\n");    globalOptions |= VERIFY_OPTION_ALL; }
            else if (!strcmp(argv[i], "-no-check-stop"))  { fprintf(stderr, "VERIFY: Option -no-check-stop\n");     globalOptions |= VERIFY_OPTION_NO_CHECK_STOP; }
            else if (!strcmp(argv[i], "-output:new"))     { fprintf(stderr, "VERIFY: Option -output:new\n");        globalOptions |= VERIFY_OPTION_OUTPUT_NEW; }
            else if (!strcmp(argv[i], "-output:old"))     { fprintf(stderr, "VERIFY: Option -output:old\n");        globalOptions &= ~VERIFY_OPTION_OUTPUT_NEW; }
            else if (!strcmp(argv[i], "-allow-restarts")) { globalAllowedRestarts = atoi(argv[++i]); fprintf(stderr, "VERIFY: Option -allow-restarts %d\n", globalAllowedRestarts); }
            else if (argv[i][0] == '-')
            {
                fprintf(stdout, "ERROR: Unrecognized option %s\n", argv[i]);
                return -3;
            }
            else if (infile == NULL && !(globalOptions & VERIFY_OPTION_ALL))
            {
                infile = argv[i];
            }
            else if (outfilename == NULL)
            {
                outfilename = argv[i];
            }
            else
            {
                fprintf(stdout, "ERROR: Unexpected parameter %s\n", argv[i]);
                return -3;
            }
        }

        /* Open the input and output files */
        if (outfilename != NULL)
        {
            outfile = fopen(outfilename, "at");
        }

        //if (argc > 2 && !strcmp(argv[2], "-output")) { output = 1; }
        if ((globalOptions & VERIFY_OPTION_ALL) == 0)
        {
            ret = verify_process(-1, infile, NULL, globalOptions);
        }
        else
        {
            verify();
        }

        if (outfile != NULL) { fclose(outfile); }
    }
    else
    {
        fprintf(stderr, "Usage: verify <<binary-input-file> | <-stop-clear-all> [outfile.csv] | <-headeronly>> [-no-check-stop] [-allow-restarts <n>]\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Where: binary-input-file: the name of the binary file to verify.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Example: verify data.cwa\n");
        fprintf(stderr, "\n");
    }

#if defined(_WIN32) && defined(_DEBUG)
    if (IsDebuggerPresent()) { fprintf(stderr, "Press [enter] to exit..."); getc(stdin); }
#endif

    return ret;
}

