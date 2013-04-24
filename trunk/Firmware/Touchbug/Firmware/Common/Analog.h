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

#ifndef ANALOG_H
#define ANALOG_H


// Global result
typedef union
{
    unsigned short values[4];  
    struct
    {
        unsigned short ldr, ir1 , batt, ir2;
    };
} adc_results_t;
extern adc_results_t adcResult;


// ADC On/off
void AdcInit(void);
void AdcOff(void);

// ADC Sampling
unsigned short *AdcSampleWait(void);    // Handles LDR/Temp on/off
unsigned short *AdcSampleNow(void);     // LDR/Temp handled externally

// Conversion functions
unsigned int AdcBattToPercent(unsigned int Vbat);
unsigned short AdcBattToMillivolt(unsigned short value);
unsigned int AdcLdrToLux(unsigned short value);
short AdcTempToTenthDegreeC(unsigned short value);

// This relies on the interrupt clearing the ASAMP bit
void AdcStartConversion(void);


// Legacy code compatibility
#ifndef NO_LEGACY

    // Global array (new code should use structure)
//    #define ADCresult (adcResult.values)
//
//    // Array indices (new code should use structure elements)
//    #define ADC_BATTERY 0
//    #define ADC_LDR 1
//    #define ADC_TEMP 2

    // Functions
//    #define InitADCOn() AdcInit()
//    #define InitADCOnRCOSC() AdcInit()  // auto-detects
//    #define InitADCOff() AdcOff()
//    #define SampleADC(_v) AdcSampleWait(_v)
//    #define SampleADC_noLDR(_v) AdcSampleNow(_v)
//    #define ConvertBattToPercentage(_v) AdcBattToPercent(_v)
//    #define BattConvert_mV(_v) AdcBattToMillivolt(_v)
//    #define LDRConvert_Lux(_v) AdcLdrToLux(_v)
//    #define TempConvert_TenthDegreeC(_v) AdcTempToTenthDegreeC(_v)

#endif


#endif

