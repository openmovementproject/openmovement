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

// HardwareProfile.h - Hardware Profile switching header
// Karim Ladha, Dan Jackson, 2011-2012

#ifndef HARDWAREPROFILE_H
#define HARDWAREPROFILE_H
    
    // Include processor specific headers
    #include "XC.h"
    // Include this to patch peripheral source/headers from flux
    #include "GenericTypeDefs.h"

// AX6 address changes - UPDATE IN LINKER IF CHANGED
#define SETTINGS_ADDRESS    0xA9800ul
#define LOG_ADDRESS         0xAA000ul

#define DEVICE_ID_ADDRESS   0xAA800ul
#define SELF_TEST_ADDRESS   0xAB000ul

#if 0

// These addresses must match the reserved sections of program memory in the linker script
// FROM AX3 DO NOT USE!
// AX3 BL2.0 Doesn't erase past 0x29FFF
       
#define DEVICE_ID_ADDRESS 0x2A000ul
//ROM BYTE __attribute__ ((address(DEVICE_ID_ADDRESS),space(prog),aligned(ERASE_BLOCK_SIZE),section(".device_id"),noload)) DeviceIdData[ERASE_BLOCK_SIZE];

#define LOG_ADDRESS  0x29C00ul
//ROM BYTE __attribute__ ((address(LOG_ADDRESS),space(prog),aligned(ERASE_BLOCK_SIZE),section(".log" ),noload)) LogData[ERASE_BLOCK_SIZE];

#define SETTINGS_ADDRESS  0x29800ul
//ROM BYTE __attribute__ ((address(SETTINGS_ADDRESS),space(prog),aligned(ERASE_BLOCK_SIZE),section(".settings" ),noload)) SettingsData[ERASE_BLOCK_SIZE];

// Added self test result page
#define SELF_TEST_ADDRESS 0x2A400ul
        
#endif

// Matched to __APP_START_OF_CODE in bootloader
#define     BOOT_RESET_VECTOR	0x0000
#define     APP_RESET_VECTOR	0x3000 

	// Switch between hardware versions (0x16 = 1.6, 0x17 = 1.7)
	#define HARDWARE_SELECT 0x64
#define     HARDWARE_NAME   "AX6" // BootCheck
#define     HARDWARE_VERSION_STRING  "6.4"
    // Define firmware version
    #define SOFTWARE_VERSION 52

// Bootloader version...
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define FIRMWARE_VERSION TOSTRING(SOFTWARE_VERSION)    // BootCheck

// High-speed capable
//#define HIGH_SPEED_USES_PLL	// Not fully tested (doesn't appear to be required anyway)

	#define ACCEL_HIGH_SPEED_WATERMARK 8	// Appears to work for short samples at 3200Hz (not tested enough to see if it always works)

	// No legacy defines for this code
	#define NO_LEGACY

    // Continue on USB if NAND or Accelerometer not found -- WARNING: Not recommended (filesystem/accelerometer code could hang)
    //#define IGNORE_UNRECOGNIZED_PERIPHERALS
    #ifdef IGNORE_UNRECOGNIZED_PERIPHERALS
    	#warning "IGNORE_UNRECOGNIZED_PERIPHERALS could be dangerous."
    #endif
    
	// User-management of bad blocks
	#define NAND_BLOCK_MARK

    // Multi NAND
    #define NAND_MULTI_PLANEWISE

    // Include CWA hardware profile
    #include "Hardware/HardwareProfile-CWA.h"
    
    // Additional defines for all hardware
#if HARDWARE_VERSION == 0x16
    #define DEFAULT_DEBUGGING 0     // 1=delayed activation, 2=on-tap, 3=always
#else
    #define DEFAULT_DEBUGGING 0     // 1=delayed activation, 2=on-tap, 3=always
#endif
    
    // USB CDC Buffer
	#define OWN_CDC_BUFFER_SIZES
	#define IN_BUFFER_CAPACITY 128
	#define OUT_BUFFER_CAPACITY 256
    
    #if 0
    	#warning "This is a no-battery build"
    	#undef USB_BUS_SENSE
    	#define USB_BUS_SENSE 0
    #endif

    // Project properties
    #define ENABLE_printhexdump     // Enable hex dump function
    #define RTC_SWWDT               // Enable software WDT
	#define RTC_SWWDT_TIMEOUT 60

	// Shortcut hack to miss out ECC on reads that are correctly check-summed data sectors
	// Unfortunately, this has to touch Nand.c (for the low-level sum to be no additional cost), and Ftl.c FtlReadSector() (for the ECC skipping logic).
	// IMPORTANT: With this enabled, all SectorRead() calls *must* be in to word-aligned buffers.
	//#define NAND_READ_SECTOR_WORD_SUMMED

#endif
