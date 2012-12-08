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

	// Switch between hardware versions (0x16 = 1.6, 0x17 = 1.7)
	#define HARDWARE_SELECT 0x17
    
    // Define firmware version
    #define SOFTWARE_VERSION 35

	// No legacy defines for this code
	#define NO_LEGACY

    // Continue on USB if NAND or Accelerometer not found -- WARNING: Not recommended (filesystem/accelerometer code could hang)
    //#define IGNORE_UNRECOGNIZED_PERIPHERALS
    #ifdef IGNORE_UNRECOGNIZED_PERIPHERALS
    	#warning "IGNORE_UNRECOGNIZED_PERIPHERALS could be dangerous."
    #endif
    
    // Include CWA hardware profile
    #include "Hardware/HardwareProfile-CWA.h"
    
    // Additional defines for all hardware
#if HARDWARE_VERSION == 0x16
    #define DEFAULT_DEBUGGING 0     // 1=delayed activation, 2=on-tap, 3=always
#else
    #define DEFAULT_DEBUGGING 0     // 1=delayed activation, 2=on-tap, 3=always
#endif
    
    #if 0
    	#warning "This is a no-battery build"
    	#undef USB_BUS_SENSE
    	#define USB_BUS_SENSE 0
    #endif

#endif
