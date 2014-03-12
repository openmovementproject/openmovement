/* 
 * Copyright (c) 2013-2014, Newcastle University, UK.
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

// ADC functions
// Karim Ladha, 2013-2014

#include <adc.h>
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Settings.h"
#include "Analog.h"

void AdcInit(void)
{
	AD1CON1 = 	(ADC_MODULE_OFF &
				ADC_IDLE_CONTINUE &
				ADC_AD12B_10BIT &
				ADC_FORMAT_INTG &
				ADC_SSRC_AUTO &
				ADC_MULTIPLE  &
				ADC_AUTO_SAMPLING_OFF);
	
	AD1CON2 = 	(ADC_VREF_AVDD_AVSS &
				ADC_SCAN_ON &
				ADC_SELECT_CHAN_0 &
				ADC_SETS_OF_SAMPLES_PER_INT_3 &
				ADC_ALT_BUF_OFF &
				ADC_ALT_INPUT_OFF);
	
	AD1CON3 = 	(ADC_SAMPLE_TIME_25 &
				ADC_CONV_CLK_INTERNAL_RC &
				ADC_CONV_CLK_50Tcy) /*Tad > 118ns, N/A for RC source*/ ;
	
	AD1CON4 = 	(ADC_DMA_DIS);
	
	AD1CHS123	= 0;
	AD1CHS0 	= 0;
	
	AD1CSSH = 	(~ADC_SELECT_H);
	AD1CSSL =	(~ADC_SELECT_L);

	// Now turn module on
	AD1CON1bits.ADON = 1;
}


void AdcOff(void)
{
	// Turn module off
	AD1CON1bits.ADON = 0;
}

void AdcSampleNow(void)
{
	if (AD1CON1bits.ADON == 0) return; // Otherwise this code will hang forever
	IFS0bits.AD1IF = 0;		// Clear interrupt flag
	AD1CON1bits.ASAM = 1;	// Begin auto sampling
	while (!IFS0bits.AD1IF);// Wait for the allotted number of conversions set by ADC_INTR_?_CONV in config2
	AD1CON1bits.ASAM = 0;	// Stop auto sampling
	while(!AD1CON1bits.DONE);// Wait for partially complete samples

	status.vddVolts_mV 		= AdcVrefToVdd(ADC1BUF0);
	status.sysTemp_x10C 	= AdcTempToTenthDegreeC(ADC1BUF1);
	status.sysVolts_mV		= 2*AdcToMillivolt(ADC1BUF2);

    return;
}

unsigned short AdcVrefToVdd(unsigned short value)
{
	// Fixed on first Hardware version (3.445v nominal)
	return 3445UL;
}

// BattConvert - Convert reading to units of 0.001 V
unsigned short AdcToMillivolt(unsigned short value)
{
    // Conversion to millivolts
    // Assumes Vdd is Vref
	unsigned long temp = value;
	temp *= status.vddVolts_mV;
	temp >>= 10;
	return temp;
}



// TempConvert - Convert temperature as a signed value in 0.1 degrees C
short AdcTempToTenthDegreeC(unsigned short value)
{
    // Assumes Vdd is Vref
	signed long temp = value;
	temp *= status.vddVolts_mV;
	temp >>= 10;
	temp -= 500;
	return temp;
}


