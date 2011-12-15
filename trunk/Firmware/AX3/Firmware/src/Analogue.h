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

/* This code was written for the ADC on the PIC 24 */
/* KL 28-03-11 */
#ifndef ANALOGUE_H
#define ANALOGUE_H


#include <adc.h>

// Globals
extern unsigned int ADCresult[3];// [0]-batt, [1]-LDR, [2]-Temp

// Following can be set to zero if LDR is not disabled between reads
#define DelayLDRsettle() DelayMs(1)

// Following can be set to zero if Temp sensor is not disabled between reads
#define DelayTempsettle() DelayMs(1)

void InitADCOn(void);
void InitADCOnRCOSC(void);
void InitADCOff(void);

// This one returns a pointer to a unsigned int buffer[2], the first one is the battery, then LDR
unsigned int *SampleADC(void);
unsigned int *SampleADC_noLDR(void);
unsigned int ConvertBattToPercentage(unsigned int);

unsigned short BattConvert_mV(unsigned short value);
unsigned int LDRConvert_Lux(unsigned short value);
short TempConvert_TenthDegreeC(unsigned short value);

#define ADC_BATTERY 0
#define ADC_LDR 1
#define ADC_TEMP 2

#endif

