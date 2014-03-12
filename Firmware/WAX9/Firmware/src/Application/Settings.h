/* 
 * Copyright (c) 2013-2014, Newcastle University, UK.
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

// WAX9 Settings
// Karim Ladha, 2013-2014

// This is not the conventional settings struct, it is made to be more applicable to the transmitter HW

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

// Device settings structure used in eeprom and ram
typedef struct
{
	// Device
    unsigned short deviceId;            // Device identifier 

	// Streaming settings
	unsigned char sleepMode;			// 0. No sleep + run on USB power, 1. Low power discoverable, 2. Wake on movement, 3. Wake on rotate, 4. Hibernate until USB connect
	unsigned short inactivityThreshold;	// Uses movement to detect inactivity
    unsigned short sampleRate;          // Sample rate code
    unsigned char dataMode;             // Data output mode 0. ascii, 1. binary
	unsigned char highPerformance;		// Use PLL oscillator esp for ascii mode + high rate

	// Sensor settings
	unsigned short accelOn;
	unsigned short accelRate;
	unsigned short accelRange;
	unsigned short gyroOn;
	unsigned short gyroRate;
	unsigned short gyroRange;
	unsigned short magOn;
	unsigned short magRate;

	// Set by selected sleep mode 
	unsigned short accelInts;

	// Checksum
	unsigned short checksum;
}settings_t;

typedef struct {
	// State
	unsigned char streaming;

	// Sensor data	
	unsigned char accelIntPending;
	unsigned long pressure;
	signed short temperature;
	unsigned short battmv;
	unsigned short inactivity;
	unsigned char newXdata;

	// LED
	signed char ledOverride;
	unsigned char usbAtStart;
}status_t;

extern settings_t settings;
extern status_t status;


char SettingsCommand(const char *line);
char SettingsCommandNumber(unsigned short num);
void SettingsInitialize(unsigned char wipe);
void SettingsSave(void);

// Helper functions
unsigned short FindChecksum (void* ptr, unsigned short len);
signed long ReadStringInteger(char* ptr,char ** end);

#endif
