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

// HardwareProfile.h - Hardware Profile header.
// Based on Microchip Technology file 'HardwareProfile.h' from "USB Device - Composite - MSD + CDC" example.

#ifndef HARDWAREPROFILE_H
#define HARDWAREPROFILE_H

    #define ROUTER

    // PAN Coordinator
//    #define COORD

	#define USE_USB_CDC

#define SLIP

    #ifdef COORD
        // PAN coordinator uses PLL
       	#define RADIO_PLL
    #else
        // Other routers use PLL
       	#define RADIO_PLL
       	
       	// For testing, initial device ID to use if none is set (do not use 0)
       	#define INITIAL_DEVICE_ID (0x01 << 8)
       	
       	#ifdef INITIAL_DEVICE_ID
       	    #if (INITIAL_DEVICE_ID == 0 || INITIAL_DEVICE_ID == 0xffff || (INITIAL_DEVICE_ID & 0xff) != 0x00)
       	        #error "Invalid router device id"
       	    #endif
       	#endif
    #endif

    #include "ConfigApp.h"

    #if defined(__C30__) && defined(__PIC24FJ256GB106__)
    	#include "HardwareProfile - TEDDIv1.1.h"
    #else
    	#error "Unexpected compiler or device."
    #endif

#endif
