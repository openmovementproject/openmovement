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

// HardwareProfile.h - Hardware Profile switching header for CWA devices
// Karim Ladha, Dan Jackson, 2011-2012

#ifndef HARDWAREPROFILE_CWA_H
#define HARDWAREPROFILE_CWA_H

// These should be chacked properly for this application
//#warning "Assumptions made about interrupt priority levels here."
	// SW WDT
	// #define RTC_SWWDT_TIMEOUT 60
	// Interrupt priority levels - needed for newer device drivers
	#define RTC_INT_PRIORITY				6 
	#define T1_INT_PRIORITY					RTC_INT_PRIORITY 	/*Prevents ms rollover during reads*/
	#define ADC_INT_PRIORITY				5 /*This is very fast*/
	#define FIFO_INTERRUPT_PRIORITY 		5
	#define GLOBAL_I2C_PROCESSOR_PRIORITY	4
	#define DATA_STREAM_INTERRUPT_PRIORITY 	4
	#define ACCEL_INT_PRIORITY				4

// These are now defined in the local HardwareProfile.h
	// Switch between hardware versions (0x16 = 1.6, 0x17 = 1.7)
	//#define HARDWARE_SELECT 0x16
    // Define firmware version
    //#define SOFTWARE_VERSION 25

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
        #endif
    #elif defined (__PIC24FJ1024GB606__)        
        #if HARDWARE_SELECT == 0x64
        	#include "HardwareProfile-AX9v4.h"
        #endif
    #else
        #error "Unknown CWA hardware profile."
    #endif

	// Verify we've selected the correct hardware profile
	#if HARDWARE_VERSION != HARDWARE_SELECT
        #ifndef __DEBUG
            #error "Hardware does not match included."
        #else
            #warning "Check HW version"
        #endif
	#endif


	// Misc software settings
	#define OWN_CDC_BUFFER_SIZES
	#define IN_BUFFER_CAPACITY 128		// Must be 128 for legacy projects
	#define OUT_BUFFER_CAPACITY 256		// Must be 256 for legacy projects


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

    // Performs a self test, returns a bitmap of failures (0x0000 = success)
    unsigned short SelfTest(void);
    

#endif
