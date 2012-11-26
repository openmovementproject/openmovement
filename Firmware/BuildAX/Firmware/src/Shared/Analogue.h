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

// Analogue to digital conversion
// Karim Ladha, Dan Jackson, 2011-2012
// Ported to TEDDI 1.1 hardware - this should not be used for any other hardware

#ifndef ANALOG_H
#define ANALOG_H

// Global result
typedef union
{
    unsigned short values[7]; 
    struct
    {
        unsigned short pir, temp, light, batt, mic_ave, mic_peak, mic_raw;
    };
} adc_results_t;
extern adc_results_t adcResult;

// ADC On/off
void AdcInit(unsigned char clock_select);
void AdcOff(void);

// Valid clock_select values
#define ADC_CLOCK_SYSTEM_PLL	0
#define ADC_CLOCK_SYSTEM		1
#define ADC_CLOCK_RC			2

// ADC Sampling
unsigned short *AdcSampleWait(void);    // Handles LDR/Temp on/off
unsigned short *AdcSampleNow(void);     // LDR/Temp handled externally

// Conversion functions
unsigned int AdcBattToPercent(unsigned int Vbat);
unsigned short AdcToMillivolt(unsigned short value);
unsigned int AdcLdrToLux(unsigned short value);
short AdcTempToTenthDegreeC(unsigned short value);

// This relies on the interrupt clearing the ASAMP bit - Call init first, resead results in ADC1 int, then call adc off
void AdcStartConversion(void);
void ADC1tasks(void);

void AdcInitJustMic(void);

#endif

