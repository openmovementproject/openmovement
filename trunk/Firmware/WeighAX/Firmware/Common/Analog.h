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

#include "HardwareProfile.h"

// Global result
typedef union
{
    unsigned short values[4];  // [0]-batt, [1]-LDR, [2]-Temp
    struct
    {
        unsigned short batt, light, temp;
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
unsigned short AdcToMillivolt(unsigned short value);
unsigned int AdcLdrToLux(unsigned short value);
short AdcTempToTenthDegreeC(unsigned short value);

// Useful macro
#define UpdateAdc()	{AdcInit();\
					AdcSampleNow();\
					AdcOff(); }

// This relies on the interrupt clearing the ASAMP bit
void AdcStartConversion(void);

#ifdef BATT_6V_ALKALINE_3V3REF
	#define BATT_CEILING		930
	#define BATT_C1		837
	#define BATT_C2		55
	#define BATT_C3		80
	#define BATT_C4		8
	#define BATT_C5		620
	#define BATT_C6		94
	#define BATT_C7		0
	#define BATT_C8		8
	#define BATT_FLOOR		620
#elif defined(BATT_6V_ALKALINE_3VREF)
	#define BATT_CEILING		1024
	#define BATT_C1		921
	#define BATT_C2		50
	#define BATT_C3		80
	#define BATT_C4		8
	#define BATT_C5		682
	#define BATT_C6		86
	#define BATT_C7		0
	#define BATT_C8		8
	#define BATT_FLOOR		682
#elif defined(BATT_6V_LITHIUM_3V3REF)
	#define BATT_CEILING	642
	#define BATT_C1	605
	#define BATT_C2	180
	#define BATT_C3	73
	#define BATT_C4	8
	#define BATT_C5	559
	#define BATT_C6	373
	#define BATT_C7	6
	#define BATT_C8	8
	#define BATT_FLOOR	559
#elif defined(BATT_6V_LITHIUM_3VREF)
	#define BATT_CEILING	707
	#define BATT_C1		666		
	#define BATT_C2		162	
	#define BATT_C3		73
	#define BATT_C4		8
	#define BATT_C5		614	
	#define BATT_C6		330
	#define BATT_C7		6
	#define BATT_C8		8
	#define BATT_FLOOR	614
#else
	#error "Set battery curve constants for configuration"
#endif

#endif

