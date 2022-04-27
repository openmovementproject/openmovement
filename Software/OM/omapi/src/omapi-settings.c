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
    if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    if (startTime != NULL) { *startTime = OmDateTimeFromString(parts[1]); }

    status = OM_COMMAND(deviceId, "\r\nSTOP\r\n", response, "STOP=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "STOP=2011/11/30,02:50:53"
    if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    if (stopTime != NULL) { *stopTime = OmDateTimeFromString(parts[1]); }

    return OM_OK;
}


int OmSetDelays(int deviceId, OM_DATETIME startTime, OM_DATETIME stopTime)
{
    char command[64];
    int status1 = OM_E_UNEXPECTED, status2 = OM_E_UNEXPECTED;
    char response[OM_MAX_RESPONSE_SIZE], *parts[2] = {0};

    {
        if (startTime < OM_DATETIME_MIN_VALID) { sprintf(command, "\r\nHIBERNATE 0\r\n"); }
        else if (startTime > OM_DATETIME_MAX_VALID) { sprintf(command, "\r\nHIBERNATE -1\r\n"); }
        else sprintf(command, "\r\nHIBERNATE %04u-%02u-%02u %02u:%02u:%02u\r\n", OM_DATETIME_YEAR(startTime), OM_DATETIME_MONTH(startTime), OM_DATETIME_DAY(startTime), OM_DATETIME_HOURS(startTime), OM_DATETIME_MINUTES(startTime), OM_DATETIME_SECONDS(startTime));
        status1 = OM_COMMAND(deviceId, command, response, "HIBERNATE=", OM_DEFAULT_TIMEOUT, parts);
        if (parts[1] == NULL) { status1 = OM_E_UNEXPECTED_RESPONSE; }
    }

    {
        if (stopTime < OM_DATETIME_MIN_VALID) { sprintf(command, "\r\nSTOP 0\r\n"); }
        else if (stopTime > OM_DATETIME_MAX_VALID) { sprintf(command, "\r\nSTOP -1\r\n"); }
        else sprintf(command, "\r\nSTOP %04u-%02u-%02u %02u:%02u:%02u\r\n", OM_DATETIME_YEAR(stopTime), OM_DATETIME_MONTH(stopTime), OM_DATETIME_DAY(stopTime), OM_DATETIME_HOURS(stopTime), OM_DATETIME_MINUTES(stopTime), OM_DATETIME_SECONDS(stopTime));
        status2 = OM_COMMAND(deviceId, command, response, "STOP=", OM_DEFAULT_TIMEOUT, parts);
        if (parts[1] == NULL) { status2 = OM_E_UNEXPECTED_RESPONSE; }
    }

    if (OM_FAILED(status1)) return status1;
    if (OM_FAILED(status2)) return status2;
    return OM_OK;
}


int OmEraseDataAndCommit(int deviceId, OM_ERASE_LEVEL level)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    OM_DOWNLOAD_STATUS downloadStatus;

    // Queries the current download status, fails if a download is in progress
    status = OmQueryDownload(deviceId, &downloadStatus, NULL);
    if (OM_FAILED(status)) return status;
    if (downloadStatus == OM_DOWNLOAD_PROGRESS) return OM_E_NOT_VALID_STATE;

    if (level == OM_ERASE_NONE)
    {
        status = OM_COMMAND(deviceId, "\r\ncommit\r\n", response, "COMMIT", 6000, parts);   // Update file
    }
    else if (level == OM_ERASE_DELETE)
    {
        status = OM_COMMAND(deviceId, "\r\nCLEAR DATA\r\n", response, "COMMIT", 6000, parts);   // Delete file + re-create file
    }
    else if (level == OM_ERASE_QUICKFORMAT)
    {
        status = OM_COMMAND(deviceId, "\r\nFORMAT QC\r\n", response, "COMMIT", 6000, parts);      // Rewrite filesystem + re-create file
    }
    else if (level == OM_ERASE_WIPE)
    {
        status = OM_COMMAND(deviceId, "\r\nFORMAT WC\r\n", response, "COMMIT", 15000, parts);      // Physically erase all NAND pages + re-create file
    }
    else
    {
        status = OM_E_INVALID_ARG;
    }

    if (OM_FAILED(status)) return status;
    // "COMMIT"
    if (parts[0] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    return OM_OK;
}


int OmGetSessionId(int deviceId, unsigned int *sessionId)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    status = OM_COMMAND(deviceId, "\r\nSESSION\r\n", response, "SESSION=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "SESSION=1"
    if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
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
    if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    if ((unsigned int)atoi(parts[1]) != sessionId) { return OM_E_FAIL; }
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
        if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
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
        if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    }
    return OM_OK;
}


int OmGetLastConfigTime(int deviceId, OM_DATETIME *time)
{
    int status = OM_E_UNEXPECTED;
    char response[OM_MAX_RESPONSE_SIZE], *parts[2] = {0};
    status = OM_COMMAND(deviceId, "\r\nLASTCHANGED\r\n", response, "LASTCHANGED=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "LASTCHANGED=2011/11/30,02:39:19"
    if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    if (time != NULL) { *time = OmDateTimeFromString(parts[1]); }
    return OM_OK;
}




// Accelerometer sampling rate codes (current shown as normal / low-power mode)
#define OM_ACCEL_RATE_3200  0x0f
#define OM_ACCEL_RATE_1600  0x0e
#define OM_ACCEL_RATE_800   0x0d
#define OM_ACCEL_RATE_400   0x0c
#define OM_ACCEL_RATE_200   0x0b
#define OM_ACCEL_RATE_100   0x0a
#define OM_ACCEL_RATE_50    0x09
#define OM_ACCEL_RATE_25    0x08
#define OM_ACCEL_RATE_12_5  0x07
#define OM_ACCEL_RATE_6_25  0x06

// Low-power mode
#define OM_ACCEL_RATE_LOW_POWER 0x10   // Low-power mode (for rates from 12.5 Hz - 400 Hz)

// Top two bits of the sampling rate value are used to determine the acceleromter range
#define OM_ACCEL_RANGE_16G  0x00
#define OM_ACCEL_RANGE_8G   0x40
#define OM_ACCEL_RANGE_4G   0x80
#define OM_ACCEL_RANGE_2G   0xC0

// Rate/range calculations
#define OM_ACCEL_RATE_FROM_CONFIG(_f)   (3200 / (1 << (15-((_f) & 0x0f))))
#define OM_ACCEL_RANGE_FROM_CONFIG(_f)  (16 >> ((_f) >> 6))
#define OM_ACCEL_IS_VALID_RATE(_v)      (((_v & 0x3f) >= 0x6 && (_v & 0x3f) <= 0xf) || ((_v & 0x3f) >= 0x17 && (_v & 0x3f) <= 0x1c))


int OmGetAccelConfig(int deviceId, int *rate, int *range)
{
    int status;
    int value;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    status = OM_COMMAND(deviceId, "\r\nRATE\r\n", response, "RATE=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "RATE=74,100"
    if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    value = atoi(parts[1]);
    int currentRate = OM_ACCEL_RATE_FROM_CONFIG(value);
    int currentRange = OM_ACCEL_RANGE_FROM_CONFIG(value);
    int gyroRange = -1;
    if (parts[2] != NULL && atoi(parts[2]) != currentRate) { return OM_E_UNEXPECTED_RESPONSE; }    
    if (parts[3] != NULL)
    {
        gyroRange = atoi(parts[3]);
    }

    // Overloaded rate: negative to indicate low-power mode
    if (value & OM_ACCEL_RATE_LOW_POWER)
    {
        currentRate = -currentRate;
    }

    // Overloaded range: high word indicates synchronous gyro range
    if (gyroRange >= 0)
    {
        currentRange |= gyroRange << 16;
    }

    // Return values
    if (rate != NULL) { *rate = currentRate; }
    if (range != NULL) { *range = currentRange; }

    if (!OM_ACCEL_IS_VALID_RATE(value)) { return OM_E_FAIL; }       // Not a valid configuration
    return OM_OK;
}


int OmSetAccelConfig(int deviceId, int rate, int range)
{
    char command[32];
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    int value = 0;
    int gyroRange = -1; // unspecified

    // Overloaded rate: negative to indicate low-power mode
    if (rate < 0)
    {
        rate = -rate;
        value |= OM_ACCEL_RATE_LOW_POWER;
    }

    // Rate code
    switch (rate)
    {
        case 3200: value |= OM_ACCEL_RATE_3200; break;
        case 1600: value |= OM_ACCEL_RATE_1600; break;
        case  800: value |= OM_ACCEL_RATE_800;  break;
        case  400: value |= OM_ACCEL_RATE_400;  break;
        case  200: value |= OM_ACCEL_RATE_200;  break;
        case  100: value |= OM_ACCEL_RATE_100;  break;
        case   50: value |= OM_ACCEL_RATE_50;   break;
        case   25: value |= OM_ACCEL_RATE_25;   break;
        case   12: value |= OM_ACCEL_RATE_12_5; break;
        case    6: value |= OM_ACCEL_RATE_6_25; break;
        default: return OM_E_INVALID_ARG;
    }

    // Overloaded range code: high word is synchronous gyro enable and range
    if (range & ~0xffff)
    {
        gyroRange = range >> 16;
        switch (gyroRange)
        {
            case 2000:
            case 1000:
            case 500:
            case 250:
            case 125:
                break;
            case 1:
                // 1 is the special case of specifying the gyro is explicitly disabled
                // (the firmware would disable the gyro if not specified on the RATE command anyway)
                gyroRange = 0;
                break;
            default:
                return OM_E_INVALID_ARG;
        }
        range &= 0xffff;
    }

    // Range code
    switch (range)
    {
        case 16:   value |= OM_ACCEL_RANGE_16G; break;
        case  8:   value |= OM_ACCEL_RANGE_8G;  break;
        case  4:   value |= OM_ACCEL_RANGE_4G;  break;
        case  2:   value |= OM_ACCEL_RANGE_2G;  break;
        default: return OM_E_INVALID_ARG;
    }

    if (gyroRange >= 0)
    {
        sprintf(command, "\r\nRATE %u,%u\r\n", value, gyroRange);
    }
    else
    {
        sprintf(command, "\r\nRATE %u\r\n", value);
    }
    status = OM_COMMAND(deviceId, command, response, "RATE=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "RATE=74,100"
    if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    if (atoi(parts[1]) != value) { return OM_E_FAIL; }
    if (parts[2] != NULL && atoi(parts[2]) != rate) { return OM_E_FAIL; }
    if (gyroRange >= 0)
    {
        // "RATE=74,100,<gyroRange>"
        if (parts[3] == NULL || atoi(parts[3]) != gyroRange) { return OM_E_FAIL; }
    }
    return OM_OK;
}

int OmGetMaxSamples(int deviceId, int *value)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    status = OM_COMMAND(deviceId, "\r\nMAXSAMPLES\r\n", response, "MAXSAMPLES=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "MAXSAMPLES=0"
    if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    if (value == NULL) { return OM_E_POINTER; }
    *value = atoi(parts[1]);
    return OM_OK;
}

int OmSetMaxSamples(int deviceId, int value)
{
    char command[32];
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    sprintf(command, "\r\nMAXSAMPLES %u\r\n", value);
    status = OM_COMMAND(deviceId, command, response, "MAXSAMPLES=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "MAXSAMPLES=0"
    if (parts[1] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    if (atoi(parts[1]) != value) { return OM_E_FAIL; }
    return OM_OK;
}

