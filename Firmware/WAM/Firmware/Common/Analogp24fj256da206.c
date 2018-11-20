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


// Should probably move these to HardwareProfile?
#define AdcSettleLdr() DelayMs(1)
#define AdcSettleTemp() DelayMs(1)



void AdcInit(void)
{
	PMD1bits.ADC1MD = 0;    // See device errata
	AD1CON1 = 0;
//	CloseADC10();
	
    // See if we're using the internal RC oscillator
    if (OSCCONbits.COSC == 0b101)
    {
		AD1CON1 = 0b0000000011100000;
		AD1CON2 = 0b0000010000001000; // Interrupt after 3rd conversion
		AD1CON3 = 0b1001010000010100; // RC clock, 20Tad samp, clk/20
		AD1CHS  = 0b0001111100011111;
		ANCFG   = 0b0000000000000000;
		AD1CSSL = ~ADC_SELECT_L;
		AD1CSSH = 0b0000000000000000;
//        // Configure ADC for RCOSC
//        OpenADC10(	/*config1*/
//                    (ADC_MODULE_OFF | 
//                    ADC_IDLE_STOP | 
//                    ADC_FORMAT_INTG | 
//                    ADC_CLK_AUTO |
//                    ADC_AUTO_SAMPLING_OFF), 
//                    /*config2*/
//                    (ADC_VREF_AVDD_AVSS | 
//                    ADC_SCAN_ON | 
//                    ADC_INTR_3_CONV| 
//                    ADC_ALT_BUF_OFF |
//                    ADC_ALT_INPUT_OFF), 
//                    /*config3*/
//                    (ADC_CONV_CLK_INTERNAL_RC |     /* INTERNAL RC */
//                    ADC_SAMPLE_TIME_20 | 
//                    ADC_CONV_CLK_20Tcy),
//                    /*configbg*/
//                    (0),
//                    /*configscan_L*/
//                    (~ADC_SELECT_L),
//                    /*configscan_H*/
//                    (~ADC_SELECT_H)
//                );
    }
    else
    {
		AD1CON1 = 0b0000000011100000;
		AD1CON2 = 0b0000010000001000; // Interrupt after 3rd conversion
		AD1CON3 = 0b0001010000010100; // System clock, 20Tad samp, clk/20
		AD1CHS  = 0b0001111100011111;
		ANCFG   = 0b0000000000000000;
		AD1CSSL = ~ADC_SELECT_L;
		AD1CSSH = 0b0000000000000000;

//        // Configure ADC for normal oscillator
//        OpenADC10(	/*config1*/
//                    (ADC_MODULE_OFF | 
//                    ADC_IDLE_STOP | 
//                    ADC_FORMAT_INTG | 
//                    ADC_CLK_AUTO |
//                    ADC_AUTO_SAMPLING_OFF), 
//                    /*config2*/
//                    (ADC_VREF_AVDD_AVSS | 
//                    ADC_SCAN_ON | 
//                    ADC_INTR_3_CONV| 
//                    ADC_ALT_BUF_OFF |
//                    ADC_ALT_INPUT_OFF), 
//                    /*config3*/
//                    (ADC_CONV_CLK_SYSTEM |      /* SYSTEM CLOCK */
//                    ADC_SAMPLE_TIME_20 | 
//                    ADC_CONV_CLK_20Tcy),
//                    /*configbg*/
//                    (0),
//                    /*configscan_L*/
//                    (~ADC_SELECT_L),
//                    /*configscan_H*/
//                    (~ADC_SELECT_H)
//                );
    }

//	EnableADC1;
	AD1CON1bits.ADON = 1;
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
	if (AD1CON1bits.ADON == 0) return adcResult.values; // Otherwise this code will hang forever

	IFS0bits.AD1IF = 0;		// Clear interrupt flag
	AD1CON1bits.ASAM = 1;	// Begin auto sampling
	while (!IFS0bits.AD1IF);// Wait for the allotted number of conversions set by ADC_INTR_?_CONV in config2
	AD1CON1bits.ASAM = 0;	// Stop auto sampling
	while(!AD1CON1bits.DONE);// Wait for partially complete samples

	adcResult.light = ReadADC10(ADC_INDEX_LDR);
	adcResult.batt = ReadADC10(ADC_INDEX_BATT);
	adcResult.temp = ReadADC10(ADC_INDEX_TEMP);

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
		#error "What thermistor version"
	#endif

}
