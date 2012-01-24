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

// Open Movement API - Settings Functions

#include "omapi-internal.h"


int OmGetDelays(int deviceId, OM_DATETIME *startTime, OM_DATETIME *stopTime)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[2] = {0};

    status = OM_COMMAND(deviceId, "\r\nHIBERNATE\r\n", response, "HIBERNATE=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "HIBERNATE=2011/11/30,02:50:53"
    if (parts[1] == NULL) { return OM_E_FAIL; }
    if (startTime != NULL) { *startTime = OmDateTimeFromString(parts[1]); }

    status = OM_COMMAND(deviceId, "\r\nSTOP\r\n", response, "STOP=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "STOP=2011/11/30,02:50:53"
    if (parts[1] == NULL) { return OM_E_FAIL; }
    if (stopTime != NULL) { *stopTime = OmDateTimeFromString(parts[1]); }

    return OM_OK;
}


int OmSetDelays(int deviceId, OM_DATETIME startTime, OM_DATETIME stopTime)
{
    char command[64];
    int status1 = OM_E_FAIL, status2 = OM_E_FAIL;
    char response[OM_MAX_RESPONSE_SIZE], *parts[2] = {0};

    {
        if (startTime < OM_DATETIME_MIN_VALID) { sprintf(command, "\r\nHIBERNATE 0\r\n"); }
        else if (startTime > OM_DATETIME_MAX_VALID) { sprintf(command, "\r\nHIBERNATE -1\r\n"); }
        else sprintf(command, "\r\nHIBERNATE %04u-%02u-%02u %02u:%02u:%02u\r\n", OM_DATETIME_YEAR(startTime), OM_DATETIME_MONTH(startTime), OM_DATETIME_DAY(startTime), OM_DATETIME_HOURS(startTime), OM_DATETIME_MINUTES(startTime), OM_DATETIME_SECONDS(startTime));
        status1 = OM_COMMAND(deviceId, command, response, "HIBERNATE=", OM_DEFAULT_TIMEOUT, parts);
        if (parts[1] == NULL) { status1 = OM_E_FAIL; }
    }

    {
        if (stopTime < OM_DATETIME_MIN_VALID) { sprintf(command, "\r\nSTOP 0\r\n"); }
        else if (stopTime > OM_DATETIME_MAX_VALID) { sprintf(command, "\r\nSTOP -1\r\n"); }
        else sprintf(command, "\r\nSTOP %04u-%02u-%02u %02u:%02u:%02u\r\n", OM_DATETIME_YEAR(stopTime), OM_DATETIME_MONTH(stopTime), OM_DATETIME_DAY(stopTime), OM_DATETIME_HOURS(stopTime), OM_DATETIME_MINUTES(stopTime), OM_DATETIME_SECONDS(stopTime));
        status2 = OM_COMMAND(deviceId, command, response, "STOP=", OM_DEFAULT_TIMEOUT, parts);
        if (parts[1] == NULL) { status2 = OM_E_FAIL; }
    }

    if (OM_FAILED(status1)) return status1;
    if (OM_FAILED(status2)) return status2;
    return OM_OK;
}


int OmClearDataAndCommit(int deviceId)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    OM_DOWNLOAD_STATUS downloadStatus;

    // Queries the current download status, fails if a download is in progress
    status = OmQueryDownload(deviceId, &downloadStatus, NULL);
    if (OM_FAILED(status)) return status;
    if (downloadStatus == OM_DOWNLOAD_PROGRESS) return OM_E_NOT_VALID_STATE;

    //status = OM_COMMAND(deviceId, "\r\nCLEAR DATA\r\n", response, "COMMIT", 4000, parts);   // Delete file + re-create file
    status = OM_COMMAND(deviceId, "\r\nFORMAT QC\r\n", response, "COMMIT", 6000, parts);      // Make all data inaccessible through FTL + re-create file
    //status = OM_COMMAND(deviceId, "\r\nFORMAT WC\r\n", response, "COMMIT", 15000, parts);      // Physically erase all NAND pages + re-create file
    if (OM_FAILED(status)) return status;
    // "COMMIT"
    if (parts[0] == NULL) { return OM_E_FAIL; }
    return OM_OK;
}


int OmCommit(int deviceId)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    OM_DOWNLOAD_STATUS downloadStatus;

    // Queries the current download status, fails if a download is in progress
    status = OmQueryDownload(deviceId, &downloadStatus, NULL);
    if (OM_FAILED(status)) return status;
    if (downloadStatus == OM_DOWNLOAD_PROGRESS) return OM_E_NOT_VALID_STATE;

    status = OM_COMMAND(deviceId, "\r\nCOMMIT\r\n", response, "COMMIT", 6000, parts);      // Update file
    if (OM_FAILED(status)) return status;
    // "COMMIT"
    if (parts[0] == NULL) { return OM_E_FAIL; }
    return OM_OK;
}


int OmGetSessionId(int deviceId, unsigned int *sessionId)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    status = OM_COMMAND(deviceId, "\r\nSESSION\r\n", response, "SESSION=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "SESSION=1"
    if (parts[1] == NULL) { return OM_E_FAIL; }
    if (sessionId == NULL) { return OM_E_POINTER; }
    *sessionId = atoi(parts[1]);
    return OM_OK;
}


int OmSetSessionId(int deviceId, unsigned int sessionId)
{
    char command[32];
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    sprintf(command, "\r\nSESSION %u\r\n", sessionId);
    status = OM_COMMAND(deviceId, command, response, "SESSION=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "SESSION=1"
    if (parts[1] == NULL) { return OM_E_FAIL; }
    if (atoi(parts[1]) != sessionId) { return OM_E_FAIL; }
    return OM_OK;
}


int OmGetMetadata(int deviceId, char *metadata)
{
    int chunk;
    int lastNonSpace = -1;

    // Clear the buffer (+ NULL-termination)
    if (metadata == NULL) { return OM_E_POINTER; }
    memset(metadata, 0x00, (OM_METADATA_SIZE + 1));

    // Read each chunk
    for (chunk = 0; chunk < 14; chunk++)
    {
        char command[32], expected[32];
        int status, o;
        char *p;
        char response[OM_MAX_RESPONSE_SIZE], *parts[2] = {0};
        sprintf(command, "\r\nANNOTATE%02d\r\n", chunk);
        sprintf(expected, "ANNOTATE%02d=", chunk);
        status = OM_COMMAND(deviceId, command, response, expected, OM_DEFAULT_TIMEOUT, parts);
        if (OM_FAILED(status)) return status;
        // "ANNOTATE00=0123456789abcdef0123456789abcdef"
        if (parts[1] == NULL) { return OM_E_FAIL; }
        p = parts[1];
        for (o = 0; o < 32; o++)
        {
            char c;
            c = *p;
            if (c == '\0') { c = ' '; } else { p++; }
            if (c < 32) { c = '|'; }
            metadata[32 * chunk + o] = c;
            if (c != ' ')  lastNonSpace = 32 * chunk + o;
        }
    }
    metadata[lastNonSpace + 1] = '\0';  // trim
    return OM_OK;
}


int OmSetMetadata(int deviceId, const char *metadata, int size)
{
    int chunk;

    if (metadata == NULL && size != 0) { return OM_E_POINTER; }

    // Write each chunk
    for (chunk = 0; chunk < 14; chunk++)
    {
        char command[64], expected[32];
        int status, o;
        char *p;
        char response[OM_MAX_RESPONSE_SIZE], *parts[2] = {0};
        sprintf(command, "\r\nANNOTATE%02d=", chunk);
        sprintf(expected, "ANNOTATE%02d=", chunk);

        p = command + strlen(command);
        for (o = 0; o < 32; o++)
        {
            char c;
            if (size != 0 && metadata != NULL && 32 * chunk + o < size) { c = metadata[32 * chunk + o]; }
            else { c = ' '; }
            if (c < 32) { c = '|'; }
            *p++ = c;
        }
        *p++ = '\r'; *p++ = '\n'; *p++ = '\0';

        status = OM_COMMAND(deviceId, command, response, expected, OM_DEFAULT_TIMEOUT, parts);
        if (OM_FAILED(status)) return status;
        if (parts[1] == NULL) { return OM_E_FAIL; }
    }
    return OM_OK;
}


int OmGetLastConfigTime(int deviceId, OM_DATETIME *time)
{
    int status = OM_E_FAIL;
    char response[OM_MAX_RESPONSE_SIZE], *parts[2] = {0};
    status = OM_COMMAND(deviceId, "\r\nLASTCHANGED\r\n", response, "LASTCHANGED=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "LASTCHANGED=2011/11/30,02:39:19"
    if (parts[1] == NULL) { return OM_E_FAIL; }
    if (time != NULL) { *time = OmDateTimeFromString(parts[1]); }
    return OM_OK;
}

