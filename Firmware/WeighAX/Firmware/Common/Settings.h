/* 
 * Copyright (c) 2012, Newcastle University, UK.
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
 
// Configuration settings, status and command handling
// Dan Jackson, 2012
#ifndef SETTINGS_H
#define SETTINGS_H

// Device settings
typedef struct
{
    unsigned short deviceId;          	// Device identifier (stored in ROM)
    unsigned short config;				// Configuration
    unsigned short destination;			// Destination (for sensors data and status updates from routers)

char* gsmDefaultNumber;
} Settings;

extern Settings settings;

// Device status
typedef struct
{
    unsigned char usb;
    char ledOverride;
	unsigned char batteryFull;
} Status;

extern Status status;


void SettingsInitialize(void);
char SettingsCommand(const char *line);





#define LOG_SIZE 32                 // "*TTTT##########################0"
#define LOG_COUNT 15
/*
// Device history/log
typedef struct
{
    unsigned short restartCount;        // Restart count
    unsigned short reserved[15];        // 30 bytes of reserved space

    char log[LOG_COUNT][LOG_SIZE];      // 15x 32-byte log chunks
} Log;
*/
#define LOG_VALUE_RESTART 0

#define LOG_CATEGORY_USER 0x0100
#define LOG_CATEGORY_STOP 0x0200

unsigned short SettingsGetLogValue(unsigned int index);
unsigned short SettingsIncrementLogValue(unsigned int index);
void SettingsAddLogEntry(unsigned short status, unsigned long timestamp, const char *message);
const char *SettingsGetLogEntry(int index, unsigned short *status, unsigned long *timestamp);

char* SettingsGetDefaultNumber(void);
void SettingsSetDefaultNumber(char* newNumber);

unsigned short SettingsGetDeviceId(void);
void SettingsSetDeviceId(unsigned short newId);
#endif
