/* 
 * Copyright (c) 2009-2014, Newcastle University, UK.
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
// Karim Ladha, Dan Jackson, 2011-2014

// Needed for SPI to work
#define USE_AND_OR

#include <adc.h>
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Analog.h"
//#include "math.h"


// Globals
unsigned short battRaw = 0;

// Precharge function - light weight for bootloader
unsigned short GetBatt(void)
{
//	PMD1bits.ADC1MD = 0;    // See device errata

	AD1CON1 = 0x40E5;
	AD1CON2 = 0x8400;
	AD1CON3 = 0xD413;
	AD1CSSL = 0x0800;
	AD1PCFG = 0xf7ff;
	AD1CON1bits.ADON = 1;

	IFS0bits.AD1IF = 0;		// Clear interrupt flag
	AD1CON1bits.ASAM = 1;	// Begin auto sampling
	while (!IFS0bits.AD1IF);// Wait for the allotted number of conversions set by ADC_INTR_?_CONV in config2
	AD1CON1bits.ASAM = 0;	// Stop auto sampling
	while(!AD1CON1bits.DONE);// Wait for partially complete samples

#ifdef BATT_SMOOTHED
	if(battRaw==0)battRaw=ADC1BUF0;
	if(ADC1BUF0>battRaw)battRaw++;
	else battRaw--; 
#else
	battRaw=ADC1BUF0;
#endif
	
	AD1CON1bits.ADON = 0;
//	PMD1bits.ADC1MD = 1;    // See device errata

	return battRaw;
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
    #define BATT_CHARGE_ZERO		614		
	#define BATT_CHARGE_FULL		708		

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


