/* 
 * Copyright (c) 2009-2011, Newcastle University, UK.
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

// ADC - Analogue to digital conversion
// Karim Ladha, 2011


// Needed for SPI to work
#define USE_AND_OR

#include <adc.h>
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Analogue.h"
//#include "math.h"

// Globals
unsigned int ADCresult[3];

void InitADCOn(void)
{
	CloseADC10();
	PMD1bits.ADC1MD = 0; // See device errata
	AD1CHS = 12; /*Channel select also performed by scan function*/
	/*Configure adc*/
	OpenADC10(	/*config1*/
				(ADC_MODULE_OFF | 
				ADC_IDLE_STOP | 
				ADC_FORMAT_INTG | 
				ADC_CLK_AUTO |
				ADC_AUTO_SAMPLING_ON), 
				/*config2*/
				(ADC_VREF_AVDD_AVSS | 
				ADC_SCAN_ON | 
				ADC_INTR_3_CONV| 
				ADC_ALT_BUF_OFF |
				ADC_ALT_INPUT_OFF), 
				/*config3*/
				(ADC_CONV_CLK_SYSTEM | 
				ADC_SAMPLE_TIME_20 | 
				ADC_CONV_CLK_20Tcy),
				/*configport_L*/
				(ANALOGUE_SELECT_L),
				/*configport_H*/
				(ANALOGUE_SELECT_H),
				/*configscan_L*/
				(~ANALOGUE_SELECT_L),
				/*configscan_H*/
				(~ANALOGUE_SELECT_H)
			);

	EnableADC1;
}

void InitADCOnRCOSC(void)
{
	CloseADC10();
	PMD1bits.ADC1MD = 0; // See device errata
	AD1CHS = 12; /*Channel select also performed by scan function*/
	/*Configure adc*/
	OpenADC10(	/*config1*/
				(ADC_MODULE_OFF | 
				ADC_IDLE_STOP | 
				ADC_FORMAT_INTG | 
				ADC_CLK_AUTO |
				ADC_AUTO_SAMPLING_ON), 
				/*config2*/
				(ADC_VREF_AVDD_AVSS | 
				ADC_SCAN_ON | 
				ADC_INTR_3_CONV| 
				ADC_ALT_BUF_OFF |
				ADC_ALT_INPUT_OFF), 
				/*config3*/
				(ADC_CONV_CLK_INTERNAL_RC | 
				ADC_SAMPLE_TIME_20 | 
				ADC_CONV_CLK_20Tcy),
				/*configport_L*/
				(ANALOGUE_SELECT_L),
				/*configport_H*/
				(ANALOGUE_SELECT_H),
				/*configscan_L*/
				(~ANALOGUE_SELECT_L),
				/*configscan_H*/
				(~ANALOGUE_SELECT_H)
			);

	EnableADC1;
}

void InitADCOff(void)
{
	AD1CON1bits.ADON = 0;
	PMD1bits.ADC1MD = 1; // See device errata
}


unsigned int* SampleADC(void) // Handle LDR on/off separately
{
LDR_ENABLE();
TEMP_ENABLE();
DelayLDRsettle();
//DelayTempsettle(); // They will both settle at the same time

ConvertADC10(); // starts conversion
while(!AD1CON1bits.DONE); /*wait till conversion complete*/

LDR_DISABLE();
TEMP_DISABLE();

#ifdef CWA1_7
ADCresult[1] = ReadADC10(0); // LDR
ADCresult[2] = ReadADC10(1); // Temp
ADCresult[0] = ReadADC10(2); // Battery
#else 
ADCresult[0] = ReadADC10(0); // Battery
ADCresult[1] = ReadADC10(1); // LDR
ADCresult[2] = ReadADC10(2); // Temp
#endif	

return ADCresult;
}


unsigned int* SampleADC_noLDR(void)
{
ConvertADC10(); // starts conversion

while(!AD1CON1bits.DONE); /*wait till conversion complete*/

#ifdef CWA1_7
ADCresult[1] = ReadADC10(0); // LDR
ADCresult[2] = ReadADC10(1); // Temp
ADCresult[0] = ReadADC10(2); // Battery
#else 
ADCresult[0] = ReadADC10(0); // Battery
ADCresult[1] = ReadADC10(1); // LDR
ADCresult[2] = ReadADC10(2); // Temp
#endif	

return ADCresult;
}

unsigned int ConvertBattToPercentage(unsigned int Vbat)
{
/*
This function makes an approximation only. The batt adc value is:
N = 1024 * Vbat/6
100% ~= 4.0v--> 682
0% ~=3v		--> 512
682 - 512 = 170
I do a linear approximation.
Note, the bat goes to 4.2v but if I use this then it never gets to 100%

I have had to hack this since the ADC reads too low when USB is connected,
probably because the USB system clock is a bit higher so sample time is lower
*/
//#define batt_zero_charge	512
//#define batt_full_charge	682
//#define batt_full_charge_with_USB	670
#define batt_delta (batt_full_charge - batt_zero_charge)
#define batt_delta_with_USB (batt_full_charge_with_USB - batt_zero_charge)

unsigned int percent = Vbat;
if (percent<batt_zero_charge) 			return 0;
if (percent>batt_full_charge) 			return 100;
if (percent>batt_full_charge_with_USB) 	return 100;

percent -= batt_zero_charge;
percent *= 100;

if(USB_BUS_SENSE)percent /= batt_delta_with_USB; 
else percent /= batt_delta;

return percent;
}


// BattConvert - Convert battery reading to units of 0.001 V
unsigned short BattConvert_mV(unsigned short value)
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
unsigned int LDRConvert_Lux(unsigned short value)
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
short TempConvert_TenthDegreeC(unsigned short value)
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
