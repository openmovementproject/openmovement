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

// HardwareProfile.h - Hardware Profile switching header
// Karim Ladha, Dan Jackson, 2011

#ifndef HARDWAREPROFILE_H
#define HARDWAREPROFILE_H

#include "HardwareProfile-Sociometer1.2p33ep256mu806.h"


#define SOFTWARE_VERSION 10
//#define DEBUG_LEDS								// Red in sleep, always flash
#define USE_POLLED /*Codec interrupts not used during the DCI setup of the codec*/

#define OFFSET_LOGGING	// Log sound file name and offset into OMX stream (to keep data in sync.)

#define CHECK_AUDIO		// Check for non-zero audio after initialization, and re-initialize the codec

// Audio file lengths
#define AUDIO_TIME_MASK 0xfffff000      // Every hour   (57.6 MB @8kHz)
//#define AUDIO_TIME_MASK 0xffffffc0      // Every minute (0.96 MB @8kHz)
//#define AUDIO_TIME_MASK 0xffffff00      // Every 4 minutes (~4 MB @8kHz)
//#define AUDIO_TIME_MASK 0xfffe0000      // Every day    (1382.4 MB @8kHz)

// Scramble flags
#define SCRAMBLE_NONE			0x00		// No scrambling
#define SCRAMBLE_REVERSE		0x01		// Sector scramble
#define SCRAMBLE_EXTENSION		0x02		// Non-default extension
#define SCRAMBLE_HIDDEN			0x04		// System+hidden files
#define SCRAMBLE_HEADER 		0x08		// Non-default header
#define SCRAMBLE_ALL			0xff		// All scrambling options


// Default settings for this build (can all be overridden in settings.ini)
#define DEFAULT_SCRAMBLE SCRAMBLE_NONE // SCRAMBLE_ALL			// scrambling by default
#define DEFAULT_RECORD_INTERVAL_ALWAYS          // Device will record when disconnected, unless told otherwise by settings file
#define DEFAULT_RECORD_TYPE RECORD_NONE //RECORD_BOTH         // RECORD_NONE, RECORD_SOUND, RECORD_ACCEL, RECORD_BOTH 
#define DEFAULT_RECORD_HOURS 0x00FFFFFFul		// 0-23
//#define DEFAULT_LOCK 6502 // 0x0000				// 0x0000 = off,
#define DEFAULT_DEBUG 3							// 3 = always, 2 = on-press, 1 = only at startup, 0 = never
#define DEFAULT_INACTIVITY 0    				// 0 = off


// Compile-in feature switches (default enabled/disable chosen above)
#define ENABLE_NONSTANDARD_DISK					// Allow intentionally corrupt first sectors -- repair them on-the-fly while reading through the device (a direct card read will still return the invalid data)
#define ENABLE_SCRAMBLE							// enable scrambling code
#define ENABLE_ENCRYPT             				// enable RC4 encryption code (when a key is set)
//#define ENCRYPT_DEFAULT_KEY "sociometer"		// set a key in the firmware
#ifdef ENABLE_SCRAMBLE
extern char gFSScramble;						// set by user code before calling FSfwriteMultipleSectors
extern char gSDScramble;						// set by fs code
#endif

#define DEFAULT_AUDIO_RATE 0					// 1 = 16kHz, otherwise 8kHz

// App location - don't put a UL on the end of the assembler becomes invalid, FYI, the goto instr doen;t need it
#define USER_APP_RESET_ADDRESS 0x01B000

#define MODE_OFF	0
#define MODE_USE	1
#define MODE_LOG	2

#define MODE_ACCEL		MODE_LOG
#define MODE_GYRO		MODE_OFF
#define MODE_ADC		MODE_LOG
#define MODE_PROX       MODE_USE 



//#define ACCEL_8BIT_MODE
#define ACCEL_RATE			ACCEL_RATE_100
#define ACCEL_RANGE			ACCEL_RANGE_8G
#define ACCEL_DEFAULT_RATE  (ACCEL_RATE | ACCEL_RANGE)



    // Read from program memory (faster if even address and length)
    char ReadProgram(unsigned long address, void *buffer, unsigned short length);

    // Write to program memory (must be a page-aligned address; if length is odd, one extra byte of junk will be written)
    char WriteProgramPage(unsigned long pageAddress, void *buffer, unsigned short length);

    // Read from program memory (all three bytes of each program 'word', even addresses only)
    char ReadProgramFull(unsigned long address, void *buffer, unsigned short length);

    // Write to program memory (all three bytes of each program 'word', must be a page-aligned address)
    char WriteProgramFullPage(unsigned long pageAddress, void *buffer, unsigned short length);

    // Performs a self test, returns a bitmap of failures (0x0000 = success)
    unsigned short SelfTest(void);

#endif
