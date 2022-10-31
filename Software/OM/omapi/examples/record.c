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
 *  @file record.c
 *  @brief     Open Movement API Example: Batch clear and set devices to record.
 *  @author    Dan Jackson
 *  @date      2012
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2012, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  A command-line tool to batch setup all connected devices to record, time synchronization.
 *  Session identifiers is set to the device id.
 *  The pairing of session identifiers and their assigned device identifiers is written to an output file (if specified).
 *  The time is synchronized with the PC at setup time.
 *
 *  @remarks Makes use of \ref api_init, \ref device_status, \ref settings, \ref return_codes, \ref datetime
 */


/* Headers - need a sleep function to wait */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(millis) sleep((millis) / 1000)
#endif
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


/* API header */
#include "omapi.h"

static FILE *ofp = NULL;
typedef enum { DEVICE_DISCONNECTED = 0, DEVICE_CONNECTED, DEVICE_ERROR, DEVICE_DONE } devicestatus_t;

struct deviceInfo_tag;
typedef struct deviceInfo_tag
{
	int deviceId;
	devicestatus_t status;
	struct deviceInfo_tag *next;
} deviceInfo_t;
deviceInfo_t *deviceInfo = NULL;

static devicestatus_t *getDeviceStatus(int deviceId)
{
	// Find existing
	for (deviceInfo_t *di = deviceInfo; di != NULL; di = di->next)
	{
		if (di->deviceId == deviceId)
		{
			return &di->status;
		}
	}
	// Insert
	deviceInfo_t *newDeviceInfo = (deviceInfo_t *)malloc(sizeof(deviceInfo_t));
	memset(newDeviceInfo, 0, sizeof(deviceInfo_t));
	newDeviceInfo->deviceId = deviceId;
	newDeviceInfo->status = DEVICE_DISCONNECTED;
	newDeviceInfo->next = deviceInfo;	// chain to existing
	deviceInfo = newDeviceInfo;	// new head of chain
	return &newDeviceInfo->status;
}


// Start time
int startDays = 1, startHour = 0;
int durationDays = 8, endHour = 0;
int minBatt = 85;
int debugMode = 0;


// Experimental: Check NAND id
#define ID_NAND

#ifdef ID_NAND
// "NANDID=%02x:%02x:%02x:%02x:%02x:%02x,%d\r\n", id[0], id[1], id[2], id[3], id[4], id[5], nandPresent
static const char NAND_DEVICE_DONT_CARE[6]      = {0x00};
static const char NAND_DEVICE_HY27UF084G2x[6]   = { 0xAD, 0xDC, 0x00 };                     // "NAND_DEVICE_ALT"
//??                                             0xad,0xdc,0x84,0x25,0xad,0x00,
static const char NAND_DEVICE_HY27UF084G2B[6]   = { 0xAD, 0xDC, 0x10, 0x95, 0x54, 0x00 };	// 1 "NAND_DEVICE"
static const char NAND_DEVICE_HY27UF084G2M[6]   = { 0xAD, 0xDC, 0x80, 0x95, 0xAD, 0x00 };	// 2 (one matched by "NAND_DEVICE_ALT", the on-board check terminates after the first two bytes as it matches against NAND_DEVICE_HY27UF084G2x)
static const char NAND_DEVICE_MT29F8G08AAA[6]   = { 0x2C, 0xD3, 0x90, 0x2E, 0x64, 0x00 };	// 3
static const char NAND_DEVICE_S34ML04G1[6]      = { 0x01, 0xDC, 0x90, 0x95, 0x54, 0x00 };	// 4 "NAND_DEVICE_ALT2"
static const char NAND_DEVICE_MT29F8G08ABADA[6] = { 0x2C, 0xDC, 0x90, 0x95, 0x56, 0x00 };	// 5 "NAND_DEVICE_ALT3" (actually 4G NAND_DEVICE_MT29F4G..., the on-board check terminates after the first two bytes)
static const char NAND_DEVICE_AX6[6]            = { 0xC2, 0xDC, 0x90, 0x95, 0x56, 0x00 };	// 6 AX6


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
		if (memcmp(NAND_DEVICE_HY27UF084G2B, (char *)localId, 6) == 0)        { *identified = 1; }
		else if (memcmp(NAND_DEVICE_HY27UF084G2M, (char *)localId, 6) == 0)   { *identified = 2; }
		else if (memcmp(NAND_DEVICE_MT29F8G08AAA, (char *)localId, 6) == 0)   { *identified = 3; }
		else if (memcmp(NAND_DEVICE_S34ML04G1, (char *)localId, 6) == 0)      { *identified = 4; }
		else if (memcmp(NAND_DEVICE_MT29F8G08ABADA, (char *)localId, 6) == 0) { *identified = 5; }
		else if (memcmp(NAND_DEVICE_AX6, (char *)localId, 6) == 0)            { *identified = 6; }
	}
    if (parts[2] == NULL) { return OM_E_UNEXPECTED_RESPONSE; }
    if (present != NULL) 
    {
        *present = atoi(parts[2]);
    }
    return OM_OK;
}
#endif

static int onlySingleId = -1;
static int onlySingleIdReturn = -1;

static bool hasGyro(int deviceId)
{
	char serialBuffer[OM_MAX_PATH];
	if (OM_FAILED(OmGetDeviceSerial(deviceId, serialBuffer))) return false;
	return memcmp(serialBuffer, "AX6", 3) == 0;
}

int record_setup(int deviceId)
{
    int result;
    char timeString[32];
    time_t now;
    struct tm *tm;
    OM_DATETIME nowTime;
    int firmwareVersion, hardwareVersion;
    int battery;

    /* Check battery level */
    battery = OmGetBatteryLevel(deviceId);
    if (OM_FAILED(battery)) { printf("ERROR: OmGetBatteryLevel() %s\n", OmErrorString(battery)); return 0; }
    else
    {
        printf("RECORD #%d: Battery at %d%% (%s)\n", deviceId, battery, (battery >= 100) ? "charged" : "charging");
    }

    /* Check hardware and firmware versions */
    result = OmGetVersion(deviceId, &firmwareVersion, &hardwareVersion);
    if (OM_FAILED(result)) { printf("ERROR: OmGetVersion() %s\n", OmErrorString(result)); return 0; }
    printf("RECORD #%d: Firmware %d, Hardware %d\n", deviceId, firmwareVersion, hardwareVersion);

#ifdef ID_NAND
    /* NAND ID */
    {
		unsigned char nandId[6] = { 0 };
        int nandType = -1;
        memset(nandId, 0, sizeof(nandId));
        result = OmGetNandId(deviceId, nandId, NULL, &nandType);
        if (result == OM_E_NOT_IMPLEMENTED) { fprintf(stderr, "NOTE: This firmware doesn't support NANDID command\n"); }
        else if (result == OM_E_UNEXPECTED_RESPONSE) { fprintf(stderr, "NOTE: Unexpected NANDID response (firmware probably doesn't support NANDID command)\n"); }
        else if (OM_FAILED(result)) { fprintf(stderr, "ERROR: Problem running OmGetNandId() %s\n", OmErrorString(result)); return 0; }
        else 
        {
			const char *nandDescription;
			switch (nandType)
			{
				case 1: nandDescription = "STANDARD (HY27UF084G2B)"; break;    // NAND_DEVICE_HY27UF084G2B
				case 2: nandDescription = "ALTERNATE (HY27UF084G2M)"; break;   // NAND_DEVICE_HY27UF084G2M
				case 3: nandDescription = "MICRON (MT29F8G08AAA)"; break;      // NAND_DEVICE_MT29F8G08AAA
				case 4: nandDescription = "SPANSION (S34ML04G1)"; break;       // NAND_DEVICE_S34ML04G1
				case 5: nandDescription = "MICRON (MT29F4G08ABADA)"; break;    // id "NAND_DEVICE_MT29F8G08ABADA", but actually 4 Gb model
				case 6: nandDescription = "AX6"; break;                        // AX6
				default: nandDescription = "UNKNOWN!"; break;
			}

            fprintf(stderr, "RECORD #%d: NAND type =%d %s [%02x:%02x:%02x:%02x:%02x:%02x]\n", deviceId, nandType, nandDescription, nandId[0], nandId[1], nandId[2], nandId[3], nandId[4], nandId[5]);
            if (nandType <= 0) { printf("ERROR: OmGetNandId() not known type (try again to be sure) [%02x:%02x:%02x:%02x:%02x:%02x].\n", nandId[0], nandId[1], nandId[2], nandId[3], nandId[4], nandId[5]); return 0; }     // ERROR: Not known type
        }
    }
#endif

    /* Get NAND information */
    {
        int memoryHealth = 0;
        memoryHealth = OmGetMemoryHealth(deviceId);
        if (OM_FAILED(memoryHealth)) { fprintf(stderr, "ERROR: OmGetMemoryHealth() %s\n", OmErrorString(result)); return 0; }
        // Spare blocks
        fprintf(stderr, "RECORD #%d: Memory health =%d\n", deviceId, memoryHealth);
		if (memoryHealth == 0) { printf("ERROR: OmGetMemoryHealth() no spare planes.\n"); return 0; }
		else if (memoryHealth < OM_MEMORY_HEALTH_ERROR) { printf("ERROR: OmGetMemoryHealth() in error region.\n"); return 0; }
        else if (memoryHealth < OM_MEMORY_HEALTH_WARNING) { printf("ERROR: OmGetMemoryHealth() in warning region.\n"); return 0; }
    }

    /* Synchronize the time */
    now = time(NULL);
    tm = localtime(&now);
    nowTime = OM_DATETIME_FROM_YMDHMS(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    fprintf(stderr, "RECORD #%d: Synchronizing the time...\n", deviceId);
    result = OmSetTime(deviceId, nowTime);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmSetTime() %s\n", OmErrorString(result)); return 0; }

    /* Set the accelerometer (+gyro) configuration */
	int range = OM_ACCEL_DEFAULT_RANGE;
	bool gyro = hasGyro(deviceId);
	fprintf(stderr, "RECORD #%d: Device has gyro: %s\n", deviceId, gyro ? "true" : "false");
	if (gyro)
	{
		range |= 2000 << 16;
	}
    result = OmSetAccelConfig(deviceId, OM_ACCEL_DEFAULT_RATE, range);
    fprintf(stderr, "RECORD #%d: Setting accelerometer (+gyro) configuration...\n", deviceId);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmSetAccelConfig() %s\n", OmErrorString(result)); return 0; }

    /* Set the session id (use the deviceId) */
    result = OmSetSessionId(deviceId, deviceId);
    fprintf(stderr, "RECORD #%d: Setting session id: %u\n", deviceId, deviceId);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmSetSessionId() %s\n", OmErrorString(result)); return 0; }

	/* Clear the max samples setting */
	OmSetMaxSamples(deviceId, 0);
	
	/* Clear the metadata */
	OmSetMetadata(deviceId, NULL, 0);
	
	/* Set the debug setting*/
	char debugCommand[128];
	char debugResponse[128];
	sprintf(debugCommand, "\r\nDEBUG %d\r\n", debugMode);
	OmCommand(deviceId, debugCommand, debugResponse, sizeof(debugResponse), "DEBUG=", 2000, NULL, 0);
	
#if 1
    {
        time_t now;
        struct tm *tm;
        OM_DATETIME startTime, stopTime;

        /* Get the current time */
        now = time(NULL);

        printf("RECORD #%d: Setting delayed start/stop times (start in +%d day(s) at %02d:00:00, stop after %d day(s) at %02d:00:00)\n", deviceId, startDays, startHour, durationDays, endHour);

        /* Start recording on the day 1 day from now, at midnight on that day */
        now += startDays * 24 * 60 * 60;    /* 1 day in seconds */
        tm = localtime(&now);
        tm->tm_hour = startHour;
        tm->tm_min = 0;
        tm->tm_sec = 0;
        startTime = OM_DATETIME_FROM_YMDHMS(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
		printf("RECORD #%d: START %04d-%02d-%02d %02d:%02d:%02d\n", deviceId, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

        /* Stop recording 8 days from that day, at midnight */
        now += durationDays * 24 * 60 * 60;    /* 8 days in seconds */
        tm = localtime(&now);
        tm->tm_hour = endHour;
        tm->tm_min = 0;
        tm->tm_sec = 0;
        stopTime = OM_DATETIME_FROM_YMDHMS(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
		printf("RECORD #%d: STOP  %04d-%02d-%02d %02d:%02d:%02d\n", deviceId, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

        result = OmSetDelays(deviceId, startTime, stopTime);
        if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmSetDelays() %s\n", OmErrorString(result)); return 0; }
    }

#else
    /* Set the delayed start/stop times */
    fprintf(stderr, "RECORD #%d: Setting start/stop: ZERO/INFINITY\n", deviceId);
    result = OmSetDelays(deviceId, OM_DATETIME_ZERO, OM_DATETIME_INFINITE);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmSetDelays() %s\n", OmErrorString(result)); return 0; }
#endif

    /* Commit the new settings */
    fprintf(stderr, "RECORD #%d: Committing new settings...\n", deviceId);
    result = OmEraseDataAndCommit(deviceId, OM_ERASE_WIPE);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmEraseDataAndCommit() %s\n", OmErrorString(result)); return 0; }

    /* The device is ready for recording */
    fprintf(stderr, "RECORD,%u,%s,%d,%d\n", deviceId, timeString, battery, firmwareVersion);
    printf("RECORD,%u,%s,%d,%d\n", deviceId, timeString, battery, firmwareVersion);
    if (ofp != NULL)
    {
        fprintf(ofp, "RECORD,%u,%s,%d,%d\n", deviceId, timeString, battery, firmwareVersion);
        fflush(ofp);
    }

    return 1;
}


/* Device updated */
static void record_DeviceCallback(void *reference, int deviceId, OM_DEVICE_STATUS status)
{
    if (status == OM_DEVICE_CONNECTED)
    {
        fprintf(stderr, "RECORD #%d: Device CONNECTED\n", deviceId);
		*getDeviceStatus(deviceId) = DEVICE_CONNECTED;
        if (onlySingleId <= 0) OmSetLed(deviceId, OM_LED_YELLOW);
    }
    else if (status == OM_DEVICE_REMOVED)
    {
        fprintf(stderr, "RECORD #%d: Device REMOVED\n", deviceId);
		*getDeviceStatus(deviceId) = DEVICE_DISCONNECTED;
    }
    else
    {
        fprintf(stderr, "RECORD #%d: Error, unexpected status %d\n", deviceId, status);
    }
    return;
}


/* Record function */
int record(const char *outfile)
{
    int result;

    /* Open the input and output files */
    if (outfile != NULL)
    {
        ofp = fopen(outfile, "at");
    }

    /* Set device callback before API startup to get initially-connected devices through the callback */
    OmSetDeviceCallback(record_DeviceCallback, NULL);

    /* Start the API */
    result = OmStartup(OM_VERSION);
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmStartup() %s\n", OmErrorString(result)); return -1; }

    for (;;)
    {
        static int maxDevices = 0;
        static int *deviceIds = NULL;
        int numDevices;
        int numCharging = 0;
        int i;

        /* Get devices */
        numDevices = OmGetDeviceIds(NULL, 0);
        if (numDevices > maxDevices) { maxDevices = numDevices; deviceIds = (int *)realloc(deviceIds, sizeof(int) * maxDevices); }
        numDevices = OmGetDeviceIds(deviceIds, maxDevices);
        if (numDevices > maxDevices) { numDevices = maxDevices; }
        fprintf(stderr, "%d... ", numDevices);
        for (i = 0; i < numDevices; i++)
        {
            int deviceId = deviceIds[i];

            if (onlySingleId > 0 && deviceId != onlySingleId) { continue; }

            if (*getDeviceStatus(deviceId) == DEVICE_CONNECTED)
            {
                // Check battery...
                int battery = OmGetBatteryLevel(deviceId);
                if (OM_FAILED(battery)) { printf("ERROR: OmGetBatteryLevel(%d) %s\n", deviceId, OmErrorString(battery)); *getDeviceStatus(deviceId) = DEVICE_ERROR; }
                else
                {
                    fprintf(stderr, "[%d@%d%%];", deviceId, battery);

                    // If sufficient battery (or setting a specific device), set-up to record
                    if (onlySingleId > 0 || battery >= minBatt)
                    {
                        int ret;
                        fprintf(stderr, "\n");
                        ret = record_setup(deviceId);
                        if (ret == 1)
                        {
							*getDeviceStatus(deviceId) = DEVICE_DONE;
                            if (onlySingleId <= 0) OmSetLed(deviceId, OM_LED_MAGENTA);
onlySingleIdReturn = 0;
                        }
                        else
                        {
							*getDeviceStatus(deviceId) = DEVICE_ERROR;
                            if (onlySingleId <= 0) OmSetLed(deviceId, OM_LED_RED);
onlySingleIdReturn = 1;
                        }
                    } else { numCharging++; } 
                }
            }
        }
        fprintf(stderr, "\n");

        if (onlySingleId > 0) { break; }

        /* Wait 5 seconds */
        Sleep(5000);
    }

    /* Shutdown the API */
    result = OmShutdown();
    if (OM_FAILED(result)) { fprintf(stderr, "ERROR: OmShutdown() %s\n", OmErrorString(result)); return -1; }

    /* Close the input and output files */
    if (ofp != NULL) { fclose(ofp); }

    if (onlySingleId > 0) { return onlySingleIdReturn; }
    return 0;
}


/* Main function */
int record_main(int argc, char *argv[])
{
    printf("RECORD: batch sets clear and record fully-charged devices.\n");
    printf("\n");
	if (argc > 1)
	{
		int i;
		const char *outfile = NULL;

		for (i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-id") == 0)
			{
				onlySingleId = (int)strtoul(argv[++i], NULL, 10);
				printf("*** SINGLE ID MODE FOR %d ***\n", onlySingleId);
			}
			else if (strcmp(argv[i], "-startdays") == 0) { startDays = atoi(argv[++i]); printf("PARAM: startDays=%d\n", startDays); }
			else if (strcmp(argv[i], "-starthour") == 0) { startHour = atoi(argv[++i]); printf("PARAM: startHour=%d\n", startHour); }
			else if (strcmp(argv[i], "-durationdays") == 0) { durationDays = atoi(argv[++i]); printf("PARAM: durationDays=%d\n", durationDays); }
			else if (strcmp(argv[i], "-endhour") == 0) { endHour = atoi(argv[++i]); printf("PARAM: endHour=%d\n", endHour); }
			else if (strcmp(argv[i], "-minbatt") == 0) { minBatt = atoi(argv[++i]); printf("PARAM: minBatt=%d\n", minBatt); }
			else if (strcmp(argv[i], "-debugmode") == 0) { debugMode = atoi(argv[++i]); printf("PARAM: debugmode=%d\n", debugMode); }
			else if (argv[i][0] == '-') { printf("WARNING: Ignoring parameter: %s\n", argv[i]); }
			else
			{
				outfile = argv[i];
			}
		}

        return record(outfile);
    }
    else
    {
        printf("Usage: record <output-file>\n");
        printf("\n");
        printf("Where: output-file: will receive the device ids\n");
        printf("\n");
        printf("Example: deploy output.csv\n");
        printf("\n");
    }
    return -1;
}

