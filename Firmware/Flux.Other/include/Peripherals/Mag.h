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

// Header for drivers for the magnetometer devices
// Karim Ladha 08-12-2012

#ifndef MAG_H
#define MAG_H

extern char 	magPresent;

// Data types
typedef union
{
    struct { short x, y, z; };
    struct { unsigned char xl, xh, yl, yh,  zl, zh; };
    short values[3];
} mag_t;

// Read data rate in Hz
unsigned char 	MagRate(void);		

// Read device ID
unsigned char MagVerifyDeviceId(void);

// MagStartup - startup with 0 == polled
void MagStartup(unsigned char samplingRate);

// Shutdown the Magnetometer
void MagStandby(void);

// Sample the device. Note: The samples are 2's compliment and 16bit
void MagSingleSample(mag_t *MagBuffer);

// Enable interrupts
void MagEnableInterrupts(void);

// Write offsets, these correct the mag o/p
void MagWriteOffsets(mag_t *magBuffer);

// Read at most 'maxEntries' 3-axis samples - Note: device may have no fifo
unsigned char MagReadFIFO(mag_t *MagBuffer, unsigned char maxEntries);

// Read interrupt source - Note: device may have only one int source
unsigned char MagReadIntSource(void);

// Debug dump registers
void MagDebugDumpRegisters(void);

// 16 bit data will not be packed and this function will not be implemented
void MagPackData(short *input, unsigned char *output);


// ------ Uniform -ValidSettings() & -StartupSettings() functions ------

// Flags for settings
#define MAG_FLAG_FIFO_INTERRUPTS 0x0001			// Has an interrupt (but not a FIFO)

// Returns whether given settings are valid
char MagValidSettings(unsigned short rateHz, unsigned short sensitivity, unsigned long flags);

// Starts the device with the given settings
void MagStartupSettings(unsigned short rateHz, unsigned short sensitivity, unsigned long flags);

// ------

#endif

