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

#include "HardwareProfile-NPRI1.0.h"
//#include "HardwareProfile-DEVBOARDV1.h"
//#include "HardwareProfile-CWA1.6+CHINESE_SCALE.h"

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
