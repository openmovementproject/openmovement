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

#include <adc.h>
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Analog.h"
//#include "math.h"

// Globals
adc_results_t adcResult;

// Should probably move these to HardwareProfile?
#define AdcSettleLdr() DelayMs(1)
#define AdcSettleTemp() DelayMs(1)



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
				ADC_SETS_OF_SAMPLES_PER_INT_1 &
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


// No waiting since this HW has no light or temp sensors
unsigned short *AdcSampleWait(void)
{
	AdcSampleNow();
 	return adcResult.values;
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

	adcResult.batt = ADC1BUF0;

	// These sensors are not present
	adcResult.light = 0;
	adcResult.prox = 0;

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
void __attribute__((interrupt, shadow, auto_psv)) _AD1Interrupt(void)
{
	IFS0bits.AD1IF = 0;			// Clear flag
	AD1CON1bits.ASAM = 0;		// Stop auto sampling
	while(!AD1CON1bits.DONE);	// Wait for partially complete samples
}



unsigned int AdcBattToPercent(unsigned int Vbat)
{
    /*
		Modified Batt fuel code to fit dual slope fit with floor and ceiling - see excel sheet

		Ceiling	642		
		Const 1	605	Sub	
		Const 2	180	Mult	
		Const 3	73	Add	
		Const 4	8	ShiftR	
		Const 5	559	Sub	
		Const 6	373	Mult	
		Const 7	6	Add	
		Const 8	8	ShiftR	
		Floor	559		
					
		Percentage = (((adc-sub)*mult)/2^shift)+add	
		
	#define BATT_CEILING	642
	#define BATT_FLOOR	559
	#define BATT_C1		605		Range 1 605 - 642
	#define BATT_C2		180	
	#define BATT_C3		73
	#define BATT_C4		8
	#define BATT_C5		559		Range 1 559 - 605
	#define BATT_C6		373
	#define BATT_C7		6
	#define BATT_C8		8
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
unsigned short AdcBattToMillivolt(unsigned short value)
{
    // Conversion to millivolts
    // Vref = 3.3V, Vbat = 6.6V * value / 1024  =>  value = Vbat * 1024 / 6.6;
	unsigned long temp = value;
	temp *= 6600;
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
		//#error "What thermistor version"
		return 0;
	#endif

}


