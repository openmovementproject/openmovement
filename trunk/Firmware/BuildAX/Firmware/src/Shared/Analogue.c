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
#include "Analogue.h"
//#include "Peripherals/Analog.h"
//#include "math.h"


// Globals
adc_results_t adcResult;


// Should probably move these to HardwareProfile?
#define AdcSettleLdr() DelayMs(1)
#define AdcSettleTemp() DelayMs(1)



void AdcInit(unsigned char clock_select)
{
	unsigned int adc1con1 = (ADC_MODULE_OFF | 
                    		ADC_IDLE_STOP | 
                    		ADC_FORMAT_INTG | 
                    		ADC_CLK_AUTO |
                    		ADC_AUTO_SAMPLING_OFF);
	unsigned int adc1con2 = (ADC_VREF_EXT_AVSS | /*Used external 2.5v ref*/
		                    ADC_SCAN_ON | 
		                    ADC_INTR_7_CONV| 
		                    ADC_ALT_BUF_OFF |
							ADC_ALT_INPUT_OFF);
	unsigned int adc1con3 = (ADC_CONV_CLK_INTERNAL_RC | 
		                    ADC_SAMPLE_TIME_20 | 
		                    ADC_CONV_CLK_20Tcy);

	PMD1bits.ADC1MD = 0;    // See device errata
	CloseADC10();
	
	if (clock_select == ADC_CLOCK_SYSTEM_PLL)
	{
		 adc1con3 =  /*config3*/
                    (ADC_CONV_CLK_SYSTEM |
                    ADC_SAMPLE_TIME_31 | 
                    ADC_CONV_CLK_2Tcy);	
	}
	else if (clock_select == ADC_CLOCK_SYSTEM)
	{
		 adc1con3 =  /*config3*/
                    (ADC_CONV_CLK_SYSTEM |   
                    ADC_SAMPLE_TIME_31 | 
                    ADC_CONV_CLK_1Tcy);	
	}
	else; // Using rc source - already done

    // Configure ADC for RCOSC
     OpenADC10(adc1con1,adc1con2,adc1con3,
		                    /*configport_L*/
		                    (ADC_SELECT_L),
		                    /*configport_H*/
		                    (ADC_SELECT_H),
		                    /*configscan_L*/
		                    ((~ADC_SELECT_L)&0xfffe), /*We dont want to scan the Vref really - it will always be 1023*/
		                    /*configscan_H*/
		                    (~ADC_SELECT_H));
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
	if (AD1CON1bits.ADON == 0) return (unsigned short*)&adcResult; // Otherwise this code will hang forever
	IFS0bits.AD1IF = 0;		// Clear interrupt flag
	AD1CON1bits.ASAM = 1;	// Begin auto sampling
	while (!IFS0bits.AD1IF);// Wait for the allotted number of conversions set by ADC_INTR_?_CONV in config2
	AD1CON1bits.ASAM = 0;	// Stop auto sampling
	while(!AD1CON1bits.DONE);// Wait for partially complete samples
	
	adcResult.pir = ReadADC10(ADC_INDEX_PIR); 		// PIR
	adcResult.temp = ReadADC10(ADC_INDEX_TEMP); 	// Temp
    adcResult.light = ReadADC10(ADC_INDEX_LIGHT);  	// Light
    adcResult.batt = ReadADC10(ADC_INDEX_BATT);	 	// Battery
    adcResult.mic_ave  = ReadADC10(ADC_INDEX_MIC_AVE);  // Microphone - integrator
    adcResult.mic_peak = ReadADC10(ADC_INDEX_MIC_PEAK); // Microphone - peak detector
	adcResult.mic_raw = ReadADC10(ADC_INDEX_MIC_RAW); 	// Microphone - raw values
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
void ADC1tasks(void)
{
	IFS0bits.AD1IF = 0;			// Clear flag
	AD1CON1bits.ASAM = 0;		// Stop auto sampling
	while(!AD1CON1bits.DONE);	// Wait for partially complete samples- This does not occur unless the int priority is messed up - adc int is highest
}



unsigned int AdcBattToPercent(unsigned int Vbat)
{
    /*
		Fully re-worked for non-rechargeable pair of alkaline cells (Duracells).
		The data is in a spreadsheet in the project documentation.
		This uses a 3 point linear fit with clamping
    */
	#if (BATT_CHARGE_ZERO != 409) 
		#error "Set zero bat charge to 409"
	#endif
	#if (BATT_CHARGE_FULL != 614)
		#error "Set full charge threshold to 614"
	#endif

	#define BATT_FIT_CONST_1	409
	#define BATT_FIT_CONST_2	10
	#define BATT_FIT_CONST_3	450	
	#define BATT_FIT_CONST_4	80
	#define BATT_FIT_CONST_5	573
	#define BATT_FIT_CONST_6	10
	#define BATT_FIT_CONST_7	614	

	unsigned long temp = 0; 
	
	// Early out functions for full and zero charge
	if (Vbat >= BATT_FIT_CONST_7) 
	{
		temp = 100;
	}
	else if (Vbat >= BATT_FIT_CONST_5)
	{
		temp = (BATT_FIT_CONST_6 * (Vbat-BATT_FIT_CONST_5));
		temp /= (BATT_FIT_CONST_7 - BATT_FIT_CONST_5);
		temp += BATT_FIT_CONST_4 + BATT_FIT_CONST_2;
	}
	else if (Vbat >= BATT_FIT_CONST_3)
	{
		temp = (BATT_FIT_CONST_4 * (Vbat-BATT_FIT_CONST_3));
		temp /= (BATT_FIT_CONST_5 - BATT_FIT_CONST_3);
		temp += BATT_FIT_CONST_2;
	}
	else if (Vbat >= BATT_FIT_CONST_1)
	{
		temp = (BATT_FIT_CONST_2 * (Vbat-BATT_FIT_CONST_1));
		temp /= (BATT_FIT_CONST_3 - BATT_FIT_CONST_1);
	}
    else; //if (Vbat < BATT_CHARGE_ZERO) return 0;

    return (unsigned int)temp;
}

// BattConvert - Convert reading to units of 0.001 V
unsigned short AdcToMillivolt(unsigned short value)
{
    // Conversion to millivolts - changed to include any Vref
	unsigned long temp = value;
	// adcresult = (voltage/Vref)*1024
	// Voltage = (adcresult * Vref)/1024
	temp *= VREF_USED_MV;
	temp += 512; // Rounding
	temp >>= 10;
	return (unsigned short)temp;
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
    
    return AdcToMillivolt(value);
}

// TempConvert - Convert temperature as a signed value in 0.1 degrees C
short AdcTempToTenthDegreeC(unsigned short value)
{
// Temp = (voltage-500mv)/10mv, voltage = (result*vref)/1024
// Temp = (((result*vref)/1024)-500)/10
// Temp0.1^C = ((result*vref)/1024)-500
	#if defined(USE_MCP9700)
		unsigned long temp = value;
		temp*=VREF_USED_MV;
		temp+=512; 
		temp>>=10;
		temp -=500;	
		return temp;
	#elif defined(USE_MCP9701)
		#if (VREF_USED_MV!=3000)
			#error "Check conversion!"
		#endif
		value*=3;
		value>>=1;
		value -=205;
		return value;
	#else
		#error "What thermistor version"
	#endif

}


void AdcInitJustMic(void)
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
                    (0b0000010000000000), /*We dont want to scan the Vref really - it will always be 1023*/
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
                    ((0b0000010000000000)), /*We dont want to scan the Vref really - it will always be 1023*/
                    /*configscan_H*/
                    (~ADC_SELECT_H)
                );
    }

	EnableADC1;
}

