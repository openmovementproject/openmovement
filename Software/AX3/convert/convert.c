// ??? A CWA/OMX split function between two times -- support crop / cut ----[=====]----  /  ==]---------[===
// ??? Unify with OMAPI to make a single exporter for OMX and old CWA files (What to do with additional per-block CWA data like batt/temp/light?)

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

// Stream Converter
// Dan Jackson, 2012-2013

/* Headers */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* API header */
#include "StreamParser.h"

#define tfprintf(tee, stream, ...) { if ((stream) != NULL) { fprintf(stream, __VA_ARGS__); } if (tee) { printf(__VA_ARGS__); } }


typedef enum { TIME_TIMESTAMP, TIME_SERIAL, TIME_SECONDS, TIME_EXCEL, TIME_MATLAB } TimeMode;


double parseTime(const char *value, double firstTimestamp, double relativeTimestamp)
{
    const char *p = value;
    double t = -1.0;
    if (p != NULL)
    {
        if (*p == '#')
        {
            t = (unsigned long)(firstTimestamp / (24 * 60 * 60)) * (24 * 60 * 60);  // Start of first day
            p++;
        }
        else if (value[0] == '+')
        {
            t = firstTimestamp;     // First timestamp
            p++;
        }
        else if (value[0] == '-' && relativeTimestamp >= 0) // Relative timestamp
        {
            t = relativeTimestamp;
            p++;
        }
        t += atof(p);
    }
    return t;
}


/* Conversion function */
int convert(const char *infile, const char **outfiles, char tee, TimeMode timeMode, const char *startString, const char *stopString, const char *sampleStepString)
{
    FILE *ofp[MAX_STREAMS] = {0};
    int sampleIndex[MAX_STREAMS] = {0};
    double lastOutput[MAX_STREAMS] = {0};
    double startTime = -1.0f;
    double stopTime = -1.0f;
    ReaderHandle reader;
    int numSamples;
    int sampleStep, timeStep;
    double firstTime = 0;
    char openFail;
    int i;

    /* Open the binary file reader on the input file */
    reader = ReaderOpen(infile);
    if (reader == NULL)
    {
        printf("ERROR: Problem opening input file: %s\n", infile);
        return -1;
    }

    /* Open the output file (or stdout if none) */
    openFail = 0;
    for (i = 0; i < MAX_STREAMS; i++)
    {
        if (outfiles[i] == NULL) { continue; }
        if (outfiles[i][0] == '\0') { ofp[i] = stdout; continue; }
        ofp[i] = fopen(outfiles[i], "wt");
        if (ofp[i] == NULL)
        { 
            printf("ERROR: Problem opening output file for stream '%c': %s\n", i, outfiles[i]);
            openFail = 1; 
            break; 
        }
    }

    if (!openFail)
    {
        // Read metadata
        unsigned long long firstTimestamp, lastTimestamp;
        const filestream_fileheader_t *metadata;

        metadata = ReaderMetadata(reader, &firstTimestamp, &lastTimestamp);

        // Step
        sampleStep = 1; timeStep = 0;
        if (sampleStepString != NULL)
        {
            if (sampleStepString[0] != '\0' && sampleStepString[strlen(sampleStepString) - 1] == 's')
            {
                timeStep = atoi(sampleStepString);
            }
            else
            {
                sampleStep = atoi(sampleStepString);
                if (sampleStep < 1) { sampleStep = 1; } 
            }
        }

        // Start times
        startTime = parseTime(startString, firstTimestamp / 65536.0, firstTimestamp / 65536.0);

        // Stop time
        stopTime = parseTime(stopString, firstTimestamp / 65536.0, startTime);


        // Output metadata
        if (ofp['!'] != NULL)
        {
            tfprintf(tee, ofp['!'], "filename,%s\n", infile);
            if (metadata != NULL)
            {
                tfprintf(tee, ofp['!'], "deviceId,%d\n", metadata->deviceId);
                tfprintf(tee, ofp['!'], "deviceType,%d\n", metadata->deviceType);
                tfprintf(tee, ofp['!'], "deviceVersion,%d\n", metadata->deviceVersion);
                tfprintf(tee, ofp['!'], "firmwareVer,%d\n", metadata->firmwareVer);
                tfprintf(tee, ofp['!'], "comments,%.128s\n", metadata->comment);
                tfprintf(tee, ofp['!'], "metadata,%.192s\n", metadata->metadata);
            }
            tfprintf(tee, ofp['!'], "firstTimestamp,%f\n", firstTimestamp / 65536.0);
            tfprintf(tee, ofp['!'], "lastTimestamp,%f\n", lastTimestamp / 65536.0);
        }

        /* Iterate over all of the blocks of the file */
        while ((numSamples = ReaderNextBlock(reader)) >= 0)
        {
            float *floatBuffer = NULL;
            int j;
            unsigned char type = 0;
            unsigned char thisStream = 0;
            unsigned char channels = 0;

            // Read the information
            ReaderDataType(reader, &type, &thisStream, &channels);

            // Filter streams
            if (thisStream < 0 || thisStream > MAX_STREAMS || ofp[thisStream] == NULL) { continue; }
            if (type == 's') { numSamples = 1; }

            for (i = 0; i < numSamples; i++)
            {
                static char timeString[64];  /* "YYYY-MM-DD hh:mm:ss.000"; */
				double t;

                // Skip sample count
                if (type == 'd' && (sampleIndex[thisStream] + i) % sampleStep != 0) { continue; }

                // Read time
				t = ReaderTimeSerial(reader, i);

                // Skip out-of-range samples
                if (startTime != -1.0 || stopTime != -1.0)
                {
                    if (startTime > 0.0 && t < startTime) { continue; }
                    if (stopTime > 0.0 && t > stopTime) { continue; }
                }

                // Time step
                if (firstTime == 0) { firstTime = t; }
                if (timeStep > 0)
                {
                    if (lastOutput[thisStream] == 0 || t >= lastOutput[thisStream] + timeStep || t < lastOutput[thisStream])
                    {
                        lastOutput[thisStream] = t;
                    }
                    else
                    {
                        continue;
                    }
                }

                if (startTime > 0 && t < startTime) { continue; }
                if (stopTime > 0 && t >= stopTime) { continue; }

#if 0
                tfprintf(tee, ofp[thisStream], "%c,", stream);
#endif

			    if (timeMode != TIME_TIMESTAMP)
			    {
				    /* Get epoch-based time value for this sample */
                    if (timeMode == TIME_SECONDS)
                    {
                        sprintf(timeString, "%0.4f", t - firstTime); 
                    }
                    else if (timeMode == TIME_EXCEL)  { sprintf(timeString, "%0.12f", t / 86400.0 + 25569.0); }
                    else if (timeMode == TIME_MATLAB) { sprintf(timeString, "%0.12f", t / 86400.0 + 25569.0 + 693960.0); }
                    else { sprintf(timeString, "%0.4f", t); } // TIME_SERIAL                
			    }
			    else
			    {
				    /* Get the date/time value for this sample, and the 1/65536th fractions of a second */
				    unsigned long dateTime;
				    unsigned short fractional;
				    dateTime = ReaderTimestamp(reader, i, &fractional);
				    sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 
					    DATETIME_YEAR(dateTime), DATETIME_MONTH(dateTime), DATETIME_DAY(dateTime), 
					    DATETIME_HOURS(dateTime), DATETIME_MINUTES(dateTime), DATETIME_SECONDS(dateTime), 
					    (int)fractional * 1000 / 0xffff);
			    }

                /* Output the data */
                tfprintf(tee, ofp[thisStream], "%s", timeString);

                if (type == 'd')
                {
                    if (floatBuffer == NULL)
                    {
                        // Read the samples
                        floatBuffer = ReaderBufferFloats(reader);
                    }
                    for (j = 0; j < channels; j++)
                    {
                        tfprintf(tee, ofp[thisStream], ",%f", floatBuffer[i * channels + j]);
                    }
                }
                else if (type == 's')
                {
                    const char *s = (const char *)ReaderRawDataPacket(reader) + 24;
                    tfprintf(tee, ofp[thisStream], ",\"%s\"", s);
                }
				else if (type == 'b')
				{
					int blockOuterLen = *((const unsigned char *)ReaderRawDataPacket(reader) + 21); // state->channelPacking
					const unsigned char *block = (const unsigned char *)ReaderRawDataPacket(reader) + 24 + (i * blockOuterLen);
					// block[0..3] = timestamp
					// block[4..5] = fractional
					unsigned char type = block[6];
					unsigned char subType = block[7];
					const unsigned char *report = block + 8;
					int blockLen = blockOuterLen - 8;

					char subTypeString[16];
					sprintf(subTypeString, "0x%02x", subType);

					char payload[480 * 8];
					payload[0] = '\0';

					if (type == 'b')
					{
						// Improve 'payload' output for this type

					}

					tfprintf(tee, ofp[thisStream], ",%c,%s,%s", type, subTypeString, payload);
				}

                tfprintf(tee, ofp[thisStream], "\n");

            }

            // Update stream sample index
            sampleIndex[thisStream] += numSamples;

        }
    }

    /* Close the input file */
    ReaderClose(reader);

    /* Close the output files */
    for (i = 0; i < MAX_STREAMS; i++)
    {
        if (ofp[i] != NULL && ofp[i] != stdout) { fclose(ofp[i]); }
    }

    return openFail;
}


/* Main function */
int main(int argc, char *argv[])
{
    int ret = -1, i, arg;
    char help = 0;
    const char *infile = NULL;
    const char *outfiles[MAX_STREAMS] = {0};
    char tee = 0;
    int stream = -1;
    const char *sampleStart = NULL;
    const char *sampleStop = NULL;
    const char *sampleStep = NULL;
	TimeMode timeMode = TIME_TIMESTAMP;

    fprintf(stderr, "CONVERT: convert a specified binary data file to a CSV text file.\n");
    fprintf(stderr, "\n");

    arg = 0;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (!strcmp(argv[i], "-tee")) { tee = 1; }
			else if (!strcmp(argv[i], "-stream"))
            { 
                // TODO: Eventually, allow multiple streams to be output to a single file
                const char *sc = argv[++i];
                stream = '\0';
                while (*sc != '\0')
                {
                    stream = *sc++;
                    if (stream < 0 || stream >= MAX_STREAMS) { fprintf(stderr, "ERROR: Specified stream invalid.\n"); help = 1; }
                    else if (outfiles[stream] != NULL) { fprintf(stderr, "ERROR: Stream '%c' already specified.\n", stream); help = 1; }
                    else
                    {
                        outfiles[stream] = "";   // will default to stdout
                    }
                }
            }
            else if (!strcmp(argv[i], "-out")) 
            { 
                if (stream < 0 || stream >= MAX_STREAMS) { fprintf(stderr, "ERROR: Stream must now be specified before output filename, for example:  -stream a -out %s\n", argv[++i]); help = 1; }
                else if (outfiles[stream] != NULL && outfiles[stream][0] != '\0') { fprintf(stderr, "ERROR: Stream '%c' output already specified.\n", stream); help = 1; }
                else
                {
                    outfiles[stream] = argv[++i]; 
                }
            }
			else if (!strcmp(argv[i], "-t:timestamp")) { timeMode = TIME_TIMESTAMP; }
			else if (!strcmp(argv[i], "-t:serial"))    { timeMode = TIME_SERIAL; }
			else if (!strcmp(argv[i], "-t:secs"))      { timeMode = TIME_SECONDS; }
			else if (!strcmp(argv[i], "-t:excel"))     { timeMode = TIME_EXCEL; }
			else if (!strcmp(argv[i], "-t:matlab"))    { timeMode = TIME_MATLAB; }
            else if (!strcmp(argv[i], "-start"))       { sampleStart = argv[++i]; }
            else if (!strcmp(argv[i], "-stop"))        { sampleStop = argv[++i]; }
            else if (!strcmp(argv[i], "-step"))        { sampleStep = argv[++i]; }
            else
            {
                fprintf(stderr, "ERROR: Unrecognized parameter: %s\n", argv[i]);
                help = 1;
            }
        }
        else
        {
            if (arg == 0) { infile = argv[i]; }
            else if (arg == 1) { fprintf(stderr, "ERROR: '-out' flag no longer optional, use:  -out %s\n", argv[i]); }
            else
            {
                fprintf(stderr, "ERROR: Unrecognized positional parameter #%d: %s\n", arg + 1, argv[i]);
                help = 1;
            }
            arg++;
        }
    }

    if (infile == NULL) { help = 1; }

    if (help)
    {
        fprintf(stderr, "Usage: convert <input.omx>\n");
        fprintf(stderr, "       [-stream <a|g|m|p|t|!|*|...> -out <output.csv>]...\n");
        fprintf(stderr, "       [-start <start>] [-stop <stop>] [-step 1]\n");
        fprintf(stderr, "       [-t:<timestamp|serial|secs|excel|matlab>]\n");
        fprintf(stderr, "       [-tee]\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Where: stream types are a=accel., g=gyro., m=mag., p=pres., t=time, !=meta., *=annotation.\n");
        fprintf(stderr, "       start and stop times are prefixed with '+' for relative seconds since data start, '#' for seconds from start of calendar day, and '-' for stop times relative to start time.\n");
        fprintf(stderr, "\n");
    }
    else
    {
        ret = convert(infile, outfiles, tee, timeMode, sampleStart, sampleStop, sampleStep);
    }

#if defined(_WIN32) && defined(_DEBUG)
    _getch();
#endif
    return ret;
}

