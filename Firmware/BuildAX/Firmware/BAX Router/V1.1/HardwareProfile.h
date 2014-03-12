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

// Hardware-specific code
// Karim Ladha, 2013-2014

#ifndef HARDWAREPROFILE_H
#define HARDWAREPROFILE_H

#include "HardwareProfile-BaxRouter1.0.h"

// FAT_FS
#define USE_FAT_FS

// Firmware version
#define FIRMWARE_VERSION 10

// Controls rate of T1 ISR and LED tasks
#define TIMED_TASKS_RATE 10

// File settings
#define SETTINGS_FILE 			"SETTINGS.INI"
#define ERROR_FILE 				"ERRORS.TXT"
#define TEXT_FILE_NAME_FMT		"LOG%05u.txt"
#define BIN_FILE_NAME_FMT		"DAT%05u.bin"
#define DEFAULT_MAX_FILE_TIME	(1ul*60ul*60ul) 	// Set to max file duration (sec)
#define DEFAULT_MAX_FILE_SIZE	(1ul*1024ul*1024ul) // Set to max file size (bytes)
#define DEFAULT_FILE_CHECK_INTERVAL	(1ul*60ul)		// Set to required file check interval (sec)
#define DEFAULT_FILE_SAVE_INTERVAL 	(10ul*60ul)		// Set to required file save interval (sec)

// Calls to write nvm above eeprom address write eeprom not program memory and do not need erases
#define EEPROM_START_ADDRESS 0x00100000 // Access above 1MB uses eeprom
#define SETTINGS_ADDRESS (EEPROM_START_ADDRESS+0)

char ReadNvm	(unsigned long address, void *buffer, unsigned short length);
char WriteNvm	(unsigned long pageAddress, void *buffer, unsigned short length);
char EraseNvm	(unsigned long address);

// BAX settings
#define MAX_LOCAL_BAX_KEYS	10
#define AES_BLOCK_SIZE		16
#define ROUTE_ALL_SUBNET	/*Route decoded packets too*/

// MiWi Settings
//#define ENABLE_CONSOLE // Debug only
// PAN Coordinator
#define ROUTER
#define COORD
#define MIWI_KEEP_ALIVE_SEC 10ul
#define SEEN_COUNTER_RESET 12		// 120 seconds 'seen' history for a router
//#define CHANNEL_MASK_DEFAULT 0b0100001000010000		// Ch 15, 20, 25
#define CHANNEL_MASK_DEFAULT 0b0000001000000000		// Ch 20
#define CHANNEL_MASK (((unsigned long)CHANNEL_MASK_DEFAULT)<<11)
#define CLOCK_FREQ 96000000ul

// Missing prototypes
extern void MrfInterruptIsr(void);

// Helpers
#define LE_READ_16(ptr)		((((unsigned short)(*(unsigned char*)(ptr+1)))<<8) + *(unsigned char*)ptr)


#endif
