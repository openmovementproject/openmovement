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

#include "HardwareProfile.h"
#include "Peripherals/Analog.h"
#include "TimeDelay.h"
//#include "math.h"


// Globals
adc_results_t adcResult;


#ifndef ADC_INDEX_LIGHT
#define ADC_INDEX_LIGHT ADC_INDEX_LDR		// Compatibility with older projects
#endif

// Should probably move these to HardwareProfile?
#define AdcSettleLdr() DelayMs(1)
#define AdcSettleTemp() DelayMs(1)



void AdcInit(void) // Just performs a conversion and shuts off the module
{
	// Basic settings for single conversion
	AD1CON1 = 0x0070;       // Integer format, 10bit
	AD1CON2 = 0x0408;       // Avdd refs, scan on, int every third sample
	AD1CON3 = 0x9400;       // RC, 20Tsamp, Trc=Tad
    AD1CON5 = 0x0000;       // Auto comparators off
    AD1CHS = BATT_ANALOG_PIN;           // Channel select, unused
    AD1CSSL =   ADC_SCAN_SELECT_L;      // Scan inputs, batt, light, temp 
    AD1CSSH =  0;                       // Scan inputs none

//	AD1CON1bits.ADON = 1; 	// Turn on 
//	IFS0bits.AD1IF = 0;		// Clear interrupt flag
//	AD1CON1bits.ASAM = 1;	// Begin auto sampling
//    
//	while (!IFS0bits.AD1IF);// Wait for the allotted number of conversions set by ADC_INTR_?_CONV in config2
//	AD1CON1bits.ASAM = 0;	// Stop auto sampling
//	while(!AD1CON1bits.DONE);// Wait for partially complete samples
//
//	adcResult.values[1] = ADC1BUF0;
//	adcResult.values[2] = ADC1BUF1;
//	adcResult.values[0] = ADC1BUF2;
    
	AD1CON1bits.ADON = 0;	// Adc off

	return;
}


void AdcOff(void)
{
	AD1CON1bits.ADON = 0;	// Adc off
}


// Handles LDR/Temp on/off
unsigned short *AdcSampleWait(void)
{
    unsigned short *ret;
    
    LDR_ENABLE();
    TEMP_ENABLE();
    AdcSettleLdr();
    //AdcSettleTemp(); // They will both settle at the same time

    ret = AdcSampleNow();
 
    LDR_DISABLE();
    TEMP_DISABLE();

    return ret;
}


// LDR/Temp handled externally
unsigned short *AdcSampleNow(void)
{

	AD1CON1bits.ADON = 1; 	// Turn on 
    
	IFS0bits.AD1IF = 0;		// Clear interrupt flag
	AD1CON1bits.ASAM = 1;	// Begin auto sampling
    
	while (!IFS0bits.AD1IF);// Wait for the allotted number of conversions set by ADC_INTR_?_CONV in config2
	AD1CON1bits.ASAM = 0;	// Stop auto sampling
	while(!AD1CON1bits.DONE);// Wait for partially complete samples

	adcResult.values[1] = ADC1BUF0;
	adcResult.values[2] = ADC1BUF1;
	adcResult.values[0] = ADC1BUF2;
    
	AD1CON1bits.ADON = 0;	// Adc off
    
    return adcResult.values;
}

unsigned int AdcBattToPercent(unsigned int Vbat)
{
    /*
		This is the new code written by KL 2012 which compensates for the non linearity of
		the battery and the internal resistance of the cell whilst charging. It does not compensate
		the voltage drop caused by current draw from the motor, LED etc so, for consistent values, 
		call with the same power draw each time. At 0% left the battery will have ~6% actual storage
		this can be used to keep the RTC going etc.
    */
	#if (BATT_CHARGE_ZERO != 614) 
		#error "Set zero bat charge to 614"
	#endif
	#if (BATT_CHARGE_FULL != 708)
		#error "Set full charge threshold to 708"
	#endif

	#define BATT_FIT_CONST_1	666LU
	#define BATT_FIT_CONST_2	150LU
	#define BATT_FIT_CONST_3	538LU	
	#define BATT_FIT_CONST_4	8
	#define BATT_FIT_CONST_5	614LU
	#define BATT_FIT_CONST_6	375LU
	#define BATT_FIT_CONST_7	614LU	
	#define BATT_FIT_CONST_8	8

	unsigned long temp; 
	
	// Compensate for charging current and internal resistance
	if (USB_BUS_SENSE)
	{
		if (Vbat>BATT_FIT_CONST_1) 		// Const voltage charging, variable current
		{
			Vbat -= (((BATT_CHARGE_FULL-BATT_FIT_CONST_1)-(Vbat-BATT_FIT_CONST_1))>>2);
		}
	 	else if (Vbat>12) Vbat -= 12; 	// Const current charging 200mA 
	}
	
	// Early out functions for full and zero charge
	if (Vbat > BATT_CHARGE_FULL) return 100;
    if (Vbat < BATT_CHARGE_ZERO) return 0;

	// Calculations for curve fit
	if (Vbat>BATT_FIT_CONST_1)
	{
		temp = (BATT_FIT_CONST_2 * (Vbat - BATT_FIT_CONST_3))>>BATT_FIT_CONST_4;
	}
	else if (Vbat>BATT_FIT_CONST_5)
	{
		temp = (BATT_FIT_CONST_6 * (Vbat - BATT_FIT_CONST_7))>>BATT_FIT_CONST_8;
	}
	else 
	{
		temp = 0;
	}

    return (unsigned int)temp;
}

// BattConvert - Convert battery reading to units of 0.001 V
unsigned short AdcBattToMillivolt(unsigned short value)
{
    // Conversion to millivolts
    // Vref = 3V, Vbat = 6V * value / 1024  =>  value = Vbat * 1024 / 6;  3.2V = 546
    //return ((60 * value + 512) >> 10) * 100;
	unsigned long temp = value;
	temp *= 6000;
	temp >>= 10;
	return temp;
}


// AdcLdrToLux - convert ADC reading to log10(lux) * 10^3  -->  lux = pow(10.0, log10LuxTimes10Power3 / 1000.0);
unsigned int AdcLdrToLux(unsigned short value)
{
    // 'APDS-9007' Ambient Light Photo Sensor with Logarithmic Current Output 
    //  Summary: The measured ADC voltage is log10(lux) --> lux = 10 ^ V
    //
    // adc = (V / Vref) * (2^10)  -->  V = (adc * Vref) / (2^10)
	//unsigned short voltageMV = (unsigned short)((((unsigned long)value * VREF_USED_MV) + 512) >> 10);
    //
    // I = V / R  -->  I = <voltage-mV> / 1000 / <loadResistance> * 1000000000; where loadResistance = 100kOhm = 100000 Ohm
    //unsigned short currentNA = voltageMV * 10;
    //
    // 10uA per log10(lux)  -->  10000 nA per log10(lux)  -->  log10(lux) * 10^3 = <current-nA> / 10
    //unsigned short log10LuxTimes10Power3 = currentNA / 10;
    //
    // Calculate lux value (1 to 986279)
    //double lux = pow(10.0, log10LuxTimes10Power3 / 1000.0);
    
    return AdcBattToMillivolt(value);
}


// TempConvert - Convert temperature as a signed value in 0.1 degrees C
short AdcTempToTenthDegreeC(unsigned short value)
{
	#if defined(USE_MCP9700)
		unsigned long temp = value;
		temp*=375;
		temp>>=7;
		temp -=500;	
		return temp;
	#elif defined(USE_MCP9701)
		value*=3;
		value>>=1;
		value -=205;
		return value;
	#else
		#error "What thermistor version"
	#endif

}
