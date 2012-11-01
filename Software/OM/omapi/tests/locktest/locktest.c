
/**
 *  @file locktest.c
 *  @brief     Open Movement API Example: Test locking/unlocking all connected devices.
 *  @author    Dan Jackson
 *  @date      2012
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2012, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  A command-line tool to test locking/unlocking a device.
 *  Only the first device found will be used.
 *
 *  @remarks Makes use of \ref api_init, \ref device_status, \ref return_codes
 */


/* Headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* API header */
#include "omapi.h"


#define LOCK_CODE 1234

/* Set the lock code for a device (zero to remove the lock) */
int setlock(int deviceId, unsigned short code)
{
    int result;
    
    result = OmSetLock(deviceId, code);
    if (OM_FAILED(result))
    {
        printf("WARNING: OmSetLock() %s\n", OmErrorString(result)); 
        return 0;
    }
    
    printf("INFO #%d: OmSetLock() to code: %d %s\n", deviceId, code, ((code == 0) ? "(removed)" : "(set)"));
    return 1;
}


/* Unlock a device */
int unlock(int deviceId, unsigned short code)
{
    int result;
    
    result = OmUnlock(deviceId, code);
    if (OM_FAILED(result))
    {
        printf("WARNING: OmUnlock() %s\n", OmErrorString(result)); 
        return 0;
    }
    
    if (result)
    {
        printf("WARNING: OmUnlock() incorrect code: %d\n", code);
        return 0;
    }
    
    printf("INFO #%d: OmUnlock() device unlocked (used code: %d)\n", deviceId, code);
    return 1;
}


/* Check if a device is locked */
int islock(int deviceId)
{
    int result;
    int locked = 0, hasLockCode = 0;

    /* Read lock status */ 
    result = OmIsLocked(deviceId, &hasLockCode);
    if (OM_FAILED(result))
    {
        printf("WARNING: OmIsLocked() %s\n", OmErrorString(result));
        return 0;
    }
    
    locked = result;
    printf("INFO #%d: IsLocked=%d, HasLockCode=%d\n", deviceId, locked, hasLockCode);
    return 1;
}

    
    
/* Do some test reads */
int doread(int deviceId)
{
    int result;

    /* Read hardware and firmware versions */
    {
        int firmwareVersion = 0, hardwareVersion = 0;
        result = OmGetVersion(deviceId, &firmwareVersion, &hardwareVersion);
        if (OM_FAILED(result)) { printf("WARNING: OmGetVersion() %s\n", OmErrorString(result)); }
        else
        {
            printf("INFO #%d: Firmware %d, Hardware %d\n", deviceId, firmwareVersion, hardwareVersion);
        }
    }
    
    /* Read battery level */
    {
        result = OmGetBatteryLevel(deviceId);
        if (OM_FAILED(result)) { printf("WARNING: OmGetBatteryLevel() %s\n", OmErrorString(result)); }
        else
        {
            printf("INFO #%d: Battery at %d%%\n", deviceId, result);
        }
    }
    
    /* Read session id */
    {
        unsigned int sessionId = 0;
        result = OmGetSessionId(deviceId, &sessionId);
        if (OM_FAILED(result)) { printf("WARNING: OmGetSessionId() %s\n", OmErrorString(result)); }
        else
        {
            printf("INFO #%d: Session id = %u\n", deviceId, sessionId);
        }
    }
    
    /* Read metadata */
    {
        char metadata[OM_METADATA_SIZE + 1];
        result = OmGetMetadata(deviceId, metadata);
        if (OM_FAILED(result)) { printf("WARNING: OmGetMetadata() %s\n", OmErrorString(result)); }
        else
        {
            printf("INFO #%d: Metadata = %s\n", deviceId, metadata);
        }
    }
        
    return 0;
}


/* Do a test write */
int dowrite(int deviceId)
{
    int result;
    int sessionId;

    /* Generate a dummy session ID based on the time */
    sessionId = time(NULL) & 0x7fffffff;

    /* Set session id */
    {
        result = OmSetSessionId(deviceId, sessionId);
        if (OM_FAILED(result)) { printf("WARNING: OmSetSessionId() %s\n", OmErrorString(result)); }
        else
        {
            printf("INFO #%d: Session id written as: %u\n", deviceId, sessionId);
        }
    }
    
    return 0;
}


/* Main function */
int main(int argc, char *argv[])
{
    int numDevices;
    int deviceId = -1;
    int result;
    int i;
    
    printf("LOCKTEST: test locking.\n");
    printf("\n");
    
    /* Check that we have parameters */
    if (argc <= 1)
    {
        printf("Usage:  locktest <setlock|clearlock|unlock|islock|read|write>...\n");
        printf("\n");
        return -1;
    }

    /* Start the API */
    result = OmStartup(OM_VERSION);
    if (OM_FAILED(result)) { printf("ERROR: OmStartup() %s\n", OmErrorString(result)); return -1; }

    /* Query the current number of devices attached */
    result = OmGetDeviceIds(NULL, 0);
    if (OM_FAILED(result)) { printf("ERROR: OmGetDeviceIds() %s\n", OmErrorString(result)); return -1; }
    numDevices = result;

    /* Get the first found devices id */
    result = OmGetDeviceIds(&deviceId, 1);
    if (OM_FAILED(result)) { printf("ERROR: OmGetDeviceIds() %s\n", OmErrorString(result)); return -1; }
    
    /* Operate with this device */
    printf("NOTE: Operating on first device found: #%d\n", deviceId);
    
    /* Process each command */
    for (i = 1; i < argc; i++)
    {
        printf("--- %s [%d] ---\n", argv[i], deviceId);
        if (!strcmp(argv[i], "setlock"))
        {
            setlock(deviceId, LOCK_CODE);
        }
        else if (!strcmp(argv[i], "clearlock"))
        {
            setlock(deviceId, 0);
        }
        else if (!strcmp(argv[i], "unlock"))
        {
            unlock(deviceId, LOCK_CODE);
        }
        else if (!strcmp(argv[i], "islock"))
        {
            islock(deviceId);
        }
        else if (!strcmp(argv[i], "read"))
        {
            doread(deviceId);
        }
        else if (!strcmp(argv[i], "write"))
        {
            dowrite(deviceId);
        }
        else
        {
            printf("WARNING: Unrecognized command '%s' (case-sensitive).\n", argv[i]);
        }
    }
    printf("---\n");

    /* Shutdown the API */
    result = OmShutdown();
    if (OM_FAILED(result)) { printf("ERROR: OmShutdown() %s\n", OmErrorString(result)); return -1; }
    
    return 0;
}

