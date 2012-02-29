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
// TODO test

	// Switch between hardware versions (0x16 = 1.6, 0x17 = 1.7)
	#define HARDWARE_SELECT 0x31

    // Define firmware version
    #define SOFTWARE_VERSION 0

    // Device-specific profiles
    #ifdef __PIC24FJ256GB106__
        // CWA 1.6 & 1.7 uses a PIC24FJ256GB106
        #if HARDWARE_SELECT == 0x16
	        #include "HardwareProfile-CWA1.6.h"
//#warning "This is a CWA 1.6 build."
	    #elif HARDWARE_SELECT == 0x17
	        #include "HardwareProfile-CWA1.7.h"
	    #elif HARDWARE_SELECT == 0x18
        	//#include "HardwareProfile-CWA1.8.h"
			#warning "This is a CWA 1.8 build."
	    #elif HARDWARE_SELECT == 0x31
        	#include "HardwareProfile-CWA3.1.h"			
        #endif
    #else
        #error "Unknown hardware profile."
    #endif

	// Verify we've selected the correct hardware profile
	#if HARDWARE_VERSION != HARDWARE_SELECT
        #error "Hardware does not match selection."
	#endif


    // Additional defines for all hardware
#if HARDWARE_VERSION == 0x16
    #define DEFAULT_DEBUGGING 3     // 1=delayed activation, 2=on-tap, 3=always
#else
    #define DEFAULT_DEBUGGING 2     // 1=delayed activation, 2=on-tap, 3=always
#endif
	
	

    // --- Required functions for all hardware ---

    // Wait until the battery is out of pre-charge
    void WaitForPrecharge(void);

    // Power save the system
    void SystemPwrSave(unsigned long napSetting);


#if defined(__C30__)
    // Non PSV data address access:
    #define ROM_ADDRESS(_v) (((unsigned long) __builtin_tblpage(_v) << 16) + __builtin_tbloffset(_v))
#endif
    
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
    
    #if 0
    	#warning "This is a no-battery build"
    	#undef USB_BUS_SENSE
    	#define USB_BUS_SENSE 0
    #endif

#endif
