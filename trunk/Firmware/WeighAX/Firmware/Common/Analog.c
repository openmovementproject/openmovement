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

// Needed for SPI to work
#define USE_AND_OR

#include <adc.h>
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Analog.h"
//#include "math.h"


// Globals
adc_results_t adcResult;

void AdcInit(void)
{
	PMD1bits.ADC1MD = 0;    // See device errata
	CloseADC10();
	
    // See if we're using the internal RC oscillator
    if (OSCCONbits.COSC == 0b101)
    {
        // Configure ADC for RCOSC
        OpenADC10(	/*config1*/
                    (ADC_MODULE_OFF | 
                    ADC_IDLE_STOP | 
                    ADC_FORMAT_INTG | 
                    ADC_CLK_AUTO |
                    ADC_AUTO_SAMPLING_OFF), 
                    /*config2*/
                    (ADC_VREF_AVDD_AVSS | 
                    ADC_SCAN_ON | 
                    ADC_INTR_EACH_CONV| 
                    ADC_ALT_BUF_OFF |
                    ADC_ALT_INPUT_OFF), 
                    /*config3*/
                    (ADC_CONV_CLK_INTERNAL_RC |     /* INTERNAL RC */
                    ADC_SAMPLE_TIME_20 | 
                    ADC_CONV_CLK_20Tcy),
                    /*configport_L*/
                    (ADC_SELECT_L),
                    /*configport_H*/
                    (ADC_SELECT_H),
                    /*configscan_L*/
                    (~ADC_SELECT_L),
                    /*configscan_H*/
                    (~ADC_SELECT_H)
                );
    }
    else
    {
        // Configure ADC for normal oscillator
        OpenADC10(	/*config1*/
                    (ADC_MODULE_OFF | 
                    ADC_IDLE_STOP | 
                    ADC_FORMAT_INTG | 
                    ADC_CLK_AUTO |
                    ADC_AUTO_SAMPLING_OFF), 
                    /*config2*/
                    (ADC_VREF_AVDD_AVSS | 
                    ADC_SCAN_ON | 
                    ADC_INTR_EACH_CONV| 
                    ADC_ALT_BUF_OFF |
                    ADC_ALT_INPUT_OFF), 
                    /*config3*/
                    (ADC_CONV_CLK_SYSTEM |      /* SYSTEM CLOCK */
                    ADC_SAMPLE_TIME_20 | 
                    ADC_CONV_CLK_20Tcy),
                    /*configport_L*/
                    (ADC_SELECT_L),
                    /*configport_H*/
                    (ADC_SELECT_H),
                    /*configscan_L*/
                    (~ADC_SELECT_L),
                    /*configscan_H*/
                    (~ADC_SELECT_H)
                );
    }

	EnableADC1;
}


void AdcOff(void)
{
	AD1CON1bits.ADON = 0;
	PMD1bits.ADC1MD = 1; // See device errata
}


// Handles LDR/Temp on/off
unsigned short *AdcSampleWait(void)
{
    unsigned short *ret;
    ret = AdcSampleNow();
    return ret;
}


// LDR/Temp handled externally
unsigned short *AdcSampleNow(void)
{

	if (AD1CON1bits.ADON == 0) return (unsigned short*)&adcResult; // Otherwise this code will hang forever
	IFS0bits.AD1IF = 0;		// Clear interrupt flag
	AD1CON1bits.ASAM = 1;	// Begin auto sampling
	while (!IFS0bits.AD1IF);// Wait for the allotted number of conversions set by ADC_INTR_?_CONV in config2
	AD1CON1bits.ASAM = 0;	// Stop auto sampling
	while(!AD1CON1bits.DONE);// Wait for partially complete samples
	
	adcResult.batt = ReadADC10(ADC_INDEX_BATT);

    return adcResult.values;
}

void AdcStartConversion(void)
{
	if (AD1CON1bits.ADON == 0) return; // Otherwise this code will hang forever
	IPC3bits.AD1IP = ADC_INT_PRIORITY;
	IFS0bits.AD1IF = 0;		// Clear interrupt flag
	IEC0bits.AD1IE = 1;		// Enable interrupts
	AD1CON1bits.ASAM = 1;	// Begin auto sampling
}

// ADC interrupt will clear the ASAMP bit after sample completes
void __attribute__((interrupt, shadow, auto_psv)) _ADC1Interrupt(void)
{
	IFS0bits.AD1IF = 0;			// Clear flag
	AD1CON1bits.ASAM = 0;		// Stop auto sampling
	while(!AD1CON1bits.DONE);	// Wait for partially complete samples
}


unsigned int AdcBattToPercent(unsigned int Vbat)
{
    /*
		Modified Batt fuel code to fit dual slope fit with floor and ceiling - see excel sheet

		Ceiling		100%	
		Const 1		Sub	
		Const 2		Mult	
		Const 3		Add	
		Const 4		ShiftR	
		Const 5		Sub	
		Const 6		Mult	
		Const 7		Add	
		Const 8		ShiftR	
		Floor		0%
					
		Percentage = (((adc-sub)*mult)/2^shift)+add	
   */		

	unsigned long temp; 
	
	// Compensate for charging current and internal resistance
//	if (USB_BUS_SENSE)
//	{
//		if (Vbat>BATT_FIT_CONST_1) 		// Const voltage charging, variable current
//		{
//			Vbat -= (((BATT_CHARGE_FULL-BATT_FIT_CONST_1)-(Vbat-BATT_FIT_CONST_1))>>2);
//		}
//	 	else if (Vbat>12) Vbat -= 12; 	// Const current charging 200mA 
//	}
	
	// Early out functions for full and zero charge
	if (Vbat > BATT_CEILING) return 100;
    if (Vbat < BATT_FLOOR) return 0;

	// Calculations for curve fit
	if (Vbat>=BATT_C1)
	{
		temp = ((BATT_C2 * (Vbat - BATT_C1))>>BATT_C4)+BATT_C3;
	}
	else if (Vbat>=BATT_C5)
	{
		temp = ((BATT_C6 * (Vbat - BATT_C5))>>BATT_C8)+BATT_C7;
	}
	else 
	{
		temp = 0;
	}

    return (unsigned int)temp;
}


// BattConvert - Convert battery reading to units of 0.001 V
unsigned short AdcToMillivolt(unsigned short value)
{
    // Conversion to millivolts
    // Vref = 3V, Vbat = 6V * value / 1024  =>  value = Vbat * 1024 / 6;  3.2V = 546
    //return ((60 * value + 512) >> 10) * 100;
	unsigned long temp = value;
#ifdef 			VREF_3v3
	temp *= 3300;
#elif defined 	(VREF_3v0)
	temp *= 3000;
#else
	#warning "Vref must be known for mV conversion"
#endif
	temp >>= 10;
	return temp;
}


// LDRConvert - Convert LDR reading to units of Lux
unsigned int AdcLdrToLux(unsigned short value)
{
	return value;
	// example of how to convert the numbers
    //#include "math.h"
	// N*(3/1024) = Log10(Lux)
//	unsigned long temp = value;
//	temp *= 3000;
//	temp >>= 10;
//	temp = (unsigned long)pow(10,3+((float)temp/1000)); 
//   return temp; // Temp is in milli-Lux
}



