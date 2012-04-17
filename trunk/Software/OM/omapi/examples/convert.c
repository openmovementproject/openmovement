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
 *  @file convert.c
 *  @brief     Open Movement API Example: Convert a binary data file to a CSV text file.
 *  @author    Dan Jackson
 *  @date      2011-2012
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2012, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  A command-line tool to convert a specified binary data file to a CSV text file.
 *
 *  @remarks Makes use of \ref reader
 */



/* Headers */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* API header */
#include "omapi.h"


/* Conversion function */
int convert(const char *infile, const char *outfile, char tee)
{
    FILE *ofp = NULL;
    OmReaderHandle reader;
    int numSamples;

    /* Open the binary file reader on the input file */
    reader = OmReaderOpen(infile);
    if (reader == NULL)
    {
        printf("ERROR: Problem opening file: %s\n", infile);
        return -1;
    }

    /* Open the output file (or stdout if none) */
    if (outfile != NULL)
    {
        ofp = fopen(outfile, "wt");
    }
    if (ofp == NULL)
    {
        ofp = stdout;
    }

    /* Iterate over all of the blocks of the file */
    while ((numSamples = OmReaderNextBlock(reader)) >= 0)
    {
        short *buffer;
        int i;

        buffer = OmReaderBuffer(reader);
        for (i = 0; i < numSamples; i++)
        {
            OM_DATETIME dateTime;
            unsigned short fractional;
            short x, y, z;
            static char timeString[25];  /* "YYYY-MM-DD hh:mm:ss.000"; */

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

            /* Output the data */
            fprintf(ofp, "%s,%d,%d,%d\n", timeString, x, y, z);

            /* 'Tee' the data to stderr */
            if (tee) { fprintf(stderr, "%s,%d,%d,%d\n", timeString, x, y, z); }
        }
    }

    /* Close the files */
    OmReaderClose(reader);
    if (ofp != NULL && ofp != stdout) { fclose(ofp); }
    return 0;
}


/* Main function */
int convert_main(int argc, char *argv[])
{
    printf("CONVERT: convert a specified binary data file to a CSV text file.\n");
    printf("\n");
    if (argc > 1)
    {
        const char *infile = argv[1];
        const char *outfile = NULL;
        char tee = 0;
        if (argc > 2) { outfile = argv[2]; }
        if (argc > 3 && !strcmp(argv[3], "-tee")) { tee = 1; }
        return convert(infile, outfile, tee);
    }
    else
    {
        printf("Usage: convert <binary-input-file> [text-output-file [-tee]]\n");
        printf("\n");
        printf("Where: binary-input-file: the name of the binary file to convert.\n");
        printf("       text-output-file: the name of the comma-separated-value text file to create, stdout if none.\n");
        printf("\n");
        printf("Example: convert data.cwa data.csv\n");
        printf("\n");
    }
    return -1;
}

