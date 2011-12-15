/* 
 * Copyright (c) 2009-2011, Newcastle University, UK.
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

// Open Movement API - Status Functions

#include "omapi-internal.h"


int OmGetVersion(int deviceId, int *firmwareVersion, int *hardwareVersion)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    status = OM_COMMAND(deviceId, "\r\nID\r\n", response, "ID=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    //"ID=CWA,hardwareId,firmwareId,deviceId,sessionId"
    if (parts[5] == NULL) { return OM_E_FAIL; }
    if (strcmp(parts[1], "CWA") != 0) { return OM_E_FAIL; }
    if (firmwareVersion != NULL) *firmwareVersion = atoi(parts[3]);
    if (hardwareVersion != NULL) *hardwareVersion = atoi(parts[2]);
    return OM_OK;
}


int OmGetBatteryLevel(int deviceId)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    status = OM_COMMAND(deviceId, "\r\nSAMPLE 1\r\n", response, "$BATT=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "$BATT=697,4083,mV,100,1"
    if (parts[5] == NULL) { return OM_E_FAIL; }
    status = atoi(parts[4]);                                    // Percentage charge complete
    if (status > 95 && atoi(parts[5]) != 0) { status = 100; }   // Charge complete
    else if (status = 100) { status = 99; }                     // Charge not quite complete
    return status;
}


int OmSelfTest(int deviceId)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    status = OM_COMMAND(deviceId, "\r\nSTATUS 1\r\n", response, "TEST=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "TEST=xxxx"
    if (parts[1] == NULL) { return OM_E_FAIL; }
    status = strtol(parts[1], NULL, 16);
    return status;
} 


int OmGetMemoryHealth(int deviceId)
{
    int status;
    int reserved, worstPlane, p, planes;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    status = OM_COMMAND(deviceId, "\r\nSTATUS 3\r\n", response, "FTL=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "NAND=1,0,106,2,8,0"
    if (parts[4] == NULL) { return OM_E_FAIL; }
    reserved = atoi(parts[3]);
    worstPlane = 0;
    planes = atoi(parts[4]);
    if (planes <= 0 || planes > 8) { return OM_E_FAIL; }
    for (p = 0; p < planes; p++)
    {
        int value;
        if (parts[5 + p] == NULL) { return OM_E_FAIL; }
        value = atoi(parts[5 + p]);
        if (value > worstPlane) { worstPlane = value; }
    }
    status = reserved - worstPlane;
    status -= 6;    // Subtract a bit more to give some margin for error (log blocks, next block, bam block)
    if (status < 0) { status = 0; }
    return status;
}


int OmGetBatteryHealth(int deviceId)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    status = OM_COMMAND(deviceId, "\r\nSTATUS 2\r\n", response, "BATTHEALTH=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "BATTHEALTH=n"
    if (parts[1] == NULL) { return OM_E_FAIL; }
    status = atoi(parts[1]);
    return status;
}


int OmSetLed(int deviceId, OM_LED_STATE ledState)
{
    char command[32];
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    sprintf(command, "\r\nLED %d\r\n", (int)ledState);
    status = OM_COMMAND(deviceId, command, response, "LED=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    //printf("LED=n");
    if (parts[1] == NULL || atoi(parts[1]) != ledState) { return OM_E_FAIL; }
    return OM_OK;
}


int OmGetAccelerometer(int deviceId, int *x, int *y, int *z)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[OM_MAX_PARSE_PARTS] = {0};
    status = OM_COMMAND(deviceId, "\r\nSAMPLE 5\r\n", response, "$ACCEL=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "$ACCEL=128,-138,178"
    if (parts[3] == NULL) { return OM_E_FAIL; }
    if (x != NULL) *x = atoi(parts[1]);
    if (y != NULL) *y = atoi(parts[2]);
    if (z != NULL) *z = atoi(parts[3]);
    return OM_OK;
}


int OmGetTime(int deviceId, OM_DATETIME *time)
{
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[2] = {0};
    status = OM_COMMAND(deviceId, "\r\nTIME\r\n", response, "$TIME=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "$TIME=2011/11/30,02:50:53"
    if (parts[1] == NULL) { return OM_E_FAIL; }
    if (time == NULL) { return OM_E_POINTER; }
    *time = OmDateTimeFromString(parts[1]);
    return OM_OK;
}


int OmSetTime(int deviceId, OM_DATETIME time)
{
    char command[64];
    int status;
    char response[OM_MAX_RESPONSE_SIZE], *parts[2] = {0};
    sprintf(command, "\r\nTIME %04u-%02u-%02u %02u:%02u:%02u\r\n", OM_DATETIME_YEAR(time), OM_DATETIME_MONTH(time), OM_DATETIME_DAY(time), OM_DATETIME_HOURS(time), OM_DATETIME_MINUTES(time), OM_DATETIME_SECONDS(time));
    status = OM_COMMAND(deviceId, command, response, "$TIME=", OM_DEFAULT_TIMEOUT, parts);
    if (OM_FAILED(status)) return status;
    // "$TIME=2011/11/30,02:50:53"
    if (parts[1] == NULL) { return OM_E_FAIL; }
    return OM_OK;
}


