/* 
 * Copyright (c) 2011-2014, Newcastle University, UK.
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
// Karim Ladha, Dan Jackson, 2011-2014

#ifndef HARDWAREPROFILE_H
#define HARDWAREPROFILE_H


	#include "..\Common\HardwareProfile-WAX9.1.h"

    // Define firmware version
	#define HARDWARE_NAME	"WAX9"
    #define HARDWARE_VERSION "1.0"
    #define FIRMWARE_VERSION "3.2"
	#define BT_CHIPSET_TARGET 	2564ul	// Set to detect mismatch in errors
	//#define PRODUCTION_BUILD			// Set to load production settings to .hex


	// USB serial + line buffer settings 
	#define LOCK_PRINTF_USBCALLS_IN_INTERRUPTS_ON_WRITE
	#define SERIAL_BUFFER_SIZE		64 
	#define IN_BUFFER_CAPACITY 		128	
	#define OUT_BUFFER_CAPACITY 	512

	// Bluetooth serial + line fifo buffers
	#define BT_LINE_BUFFER_SIZE		64
	#define BT_IN_BUFFER_SIZE 		128
	#define BT_OUT_BUFFER_SIZE		128

	// Main data out fifo
	#define STREAM_OUT_BUFFER_SIZE	2048	
    
	#define MINIMUM_T1_RATE			2
	#define LOW_FREQ_TASKS_TICKS 	(65536UL/MINIMUM_T1_RATE)

	// SW WDT
	#define RTC_SWWDT_TIMEOUT 		60 

	#define EEPROM_START_ADDRESS 	0x0100000UL // To aggree with mapping in linker so we can use C variable names
	#define SETTINGS_ADDRESS 		(EEPROM_START_ADDRESS + 0)	
	#define SELF_TEST_RESULTS 		(EEPROM_START_ADDRESS + 0x400)
	#define SELF_TEST_SIZE			1024ul

    // Power save the system
    void SystemPwrSave(unsigned long napSetting);

    // Read from program memory (faster if even address and length)
    char ReadNvm(unsigned long address, void *buffer, unsigned short length);

	// This actually writes a page (128 bytes), address must be page aligned
	char WriteNvm(unsigned long pageAddress, void *buffer, unsigned short length);

	// Erase block of program memory - must be block (1024) aligned
	char EraseNvm(unsigned long address);

#endif
