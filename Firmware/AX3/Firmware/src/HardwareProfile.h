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

// Added self test result page
#define SELF_TEST_ADDRESS 0x0002A400

	// Switch between hardware versions (0x16 = 1.6, 0x17 = 1.7)
	#define HARDWARE_SELECT 0x17
    
    // Define firmware version
    #define SOFTWARE_VERSION 50

// Compensate for poor charge termination in some devices (%)
#define COMPENSATE_CHARGE_THRESHOLD	80

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
