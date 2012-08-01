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
#define gmtime_r(timer, result) gmtime_s(result, timer)
#define timegm _mkgmtime
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>



/* API header */
#include "omapi.h"


/* Error measures */
#define STUCK_COUNT (5 * 120)
#define AVERAGE_FACTOR 0.00001
#define AVERAGE_RANGE_MAX 0.400
#define AVERAGE_RANGE_OFF 0.300


// Calculate the time in ticks from a packed time
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


// Calculate the time in ticks from a packed time
unsigned long long Ticks(OM_DATETIME timestamp, unsigned short fractional)
{
    time_t tSec = TimeSerial(timestamp);
    return ((unsigned long long)tSec << 16) + fractional;
}



/* Conversion function */
int verify(const char *infile, char output)
{
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

    fprintf(stderr, "FILE: %s\n", infile);

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
        if (block != 0 && block %    50 == 0) { fprintf(stderr, "."); }   /* Approx. 1 minute */
        /*if (block != 0 && block %  3000 == 0) { fprintf(stderr, "|\n"); }     */          /* Approx. 1 hour */
        /*if (block != 0 && block % 72000 == 0) { fprintf(stderr, "===\n"); }   */          /* Approx. 1 day */

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
                fprintf(stderr, "NOTE: Recording sequence restarted @%u, length of %+.2fs (gap of %+.2fs)\n", previousSequenceId, (float)recordingLength / 65536.0f, (float)diff / 65536.0f);
                restarts++;
                totalDuration += recordingLength;
                breakTime += (float)diff / 65536.0f;

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
            static short lx = 0, ly = 0, lz = 0, stuck = 0;
            static char timeString[25];  /* "YYYY-MM-DD hh:mm:ss.000"; */
            static int seconds, packetCount = 0;
            double v;
            static double av = 0.0;

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
            if (output) { fprintf(stdout, "%s,%d,%d,%d\n", timeString, x, y, z); }

            {
                static int lastHour = -1;
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
                else if (packetCount >= 90 && packetCount <= 110) { /* fprintf(stderr, "#"); */ }
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
                if (abs(stopDiff) < 80)
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

        fprintf(stderr, "---\n");
        fprintf(stderr, "Input file: \"%s\"\n", infile);
        fprintf(stderr, "Summary errors: file=%d, event=%d, stuck=%d, range=%d, rate=%d, breaks=%d\n", errorFile, errorEvent, errorStuck, errorRange, errorRate, errorBreaks);
        fprintf(stderr, "Summary info-1: restart=%d, breakTime=%0.1fs, maxAv=%f\n", restarts, breakTime, maxAv);
        fprintf(stderr, "Summary info-2: minInterval=%0.3f, maxInterval=%0.3f, duration=%0.4fh\n", minInterval / 65536.0f, maxInterval / 65536.0f, ((totalDuration >> 16) / 60.0f / 60.0f));
        fprintf(stderr, "Summary info-3: minLight=%d, Bmax=%d%%, Bmin=%d%%, intervalFail=%d\n", minLight, batteryMaxPercent, batteryMinPercent, startStopFail);
        fprintf(stderr, "---\n");

#define HEADER          "filename,      file,      event,      stuck,      range,      rate,      breaks, restarts, breakTime, maxAv, minInterval,            maxInterval,                   duration,                         minLight, batteryMaxPercent, batteryMinPercent,  intervalFail\n"
        fprintf(stdout, "  \"%s\",        %d,         %d,         %d,         %d,        %d,          %d,       %d,     %0.1f, %0.4f,       %0.3f,                  %0.3f,                      %0.4f,                               %d,                %d,                %d,            %d\n",
                           infile, errorFile, errorEvent, errorStuck, errorRange, errorRate, errorBreaks, restarts, breakTime, maxAv, minInterval / 65536.0f, maxInterval / 65536.0f, ((totalDuration >> 16) / 60.0f / 60.0f), minLight, batteryMaxPercent, batteryMinPercent, startStopFail);
    }


    /* Close the files */
    OmReaderClose(reader);
    return 0;
}


/* Main function */
int verify_main(int argc, char *argv[])
{
    fprintf(stderr, "VERIFY: verify a specified binary data file contains sensible data.\n");
    fprintf(stderr, "\n");
    if (argc > 1)
    {
        const char *infile;
        char output = 0;

        if (!strcmp(argv[1], "-headeronly"))
        {
            fprintf(stdout, HEADER);
            return -2;
        }

        infile = argv[1];
        if (argc > 2 && !strcmp(argv[2], "-output")) { output = 1; }
        return verify(infile, output);
    }
    else
    {
        fprintf(stderr, "Usage: verify <binary-input-file | -headeronly> [-output]\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Where: binary-input-file: the name of the binary file to verify.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Example: verify data.cwa\n");
        fprintf(stderr, "\n");
    }

#if defined(_WIN32) && defined(_DEBUG)
    if (IsDebuggerPresent()) { fprintf(stderr, "Press [enter] to exit..."); getc(stdin); }
#endif

    return -1;
}

