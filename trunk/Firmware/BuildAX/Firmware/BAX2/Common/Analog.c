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

// Simple analog drive for the 26k20 PIC MCU customised to the BAXx.x hardware
// Karim Ladha, 2013-2014

// Include
#include "Compiler.h"
#include "HardwareProfile.h"
#include "Settings.h"
#include "Analog.h"

// Debug setting
#ifndef DEBUG_ON
//	#define DEBUG_ON
#endif
#include "Debug.h"

// Globals
adcResults_t adcResults;
sensorVals_t sensorVals;

// Code
void InitAnalog(void)
{
	ADCON0 = 0b00000000; // adc off
	ADCON1 = 0b00010000; // vref+ used
	ADCON2 = 0b10011011; // Right Justified, 6Tad sample time, IntRC
}

void SampleChannel(unsigned char channel)
{
	ADCON0 = 0b00000001; 	// adc on
	ADCON0 |= (channel<<2); // adc channel
	ADCON0bits.GO = 1;
	while (ADCON0bits.GO){;}
	switch (channel) {
		case TEMP_CHANNEL 		: adcResults.temp 	= ((unsigned short)ADRESH<<8) + ADRESL;break;
		case LIGHT_CHANNEL 		: adcResults.light 	= ((unsigned short)ADRESH<<8) + ADRESL;break;
		case HUMID_CHANNEL 		: adcResults.humid 	= ((unsigned short)ADRESH<<8) + ADRESL;break;
		case PIR_CHANNEL 		: adcResults.pir 	= ((unsigned short)ADRESH<<8) + ADRESL;break;
		default: break;
	}
	ADCON0 = 0b00000000; 				// adc off
	return;
}

void SampleSensors(void)
{
	// Turn on sensors
	SENSORS_ENABLE = 1;
	// Settle time
	DelayMs(10);

	{
		sensorVals.temp = AdcSampleTemp();
		sensorVals.tempCx10 = ConvertTempToCelcius(sensorVals.temp);
		DBG_INFO("\r\nTemp:");
		DBG_PRINT_INT(sensorVals.tempCx10);
	}
	{
		// Using lookup and linear interpolate and temperature from above, see excel sheet
		unsigned char temp8 = (sensorVals.tempCx10 + 5)/10; // Truncate to 1C resolution
		sensorVals.humid = SampleHumidity();				// Sample
		sensorVals.humidSat = ConvertHumidityToSat(sensorVals.humid,temp8); 
		DBG_INFO("\r\nHumidity:");
		DBG_PRINT_INT((sensorVals.humidSat>>8));
		DBG_INFO(".");
		DBG_PRINT_INT((40*(sensorVals.humidSat&0xff))>>10);
	}
	{
		sensorVals.light = SampleLight();
		sensorVals.lightLux = ConvertLightToLux(sensorVals.light); 
		DBG_INFO("\r\nLight:");
		DBG_PRINT_INT(sensorVals.lightLux);
	}
	{
		sensorVals.fvr = SampleFvrForVdd();
		sensorVals.vddmv = ConvertFvrToVddMv(sensorVals.fvr);
		DBG_INFO("\r\nBatt:");
		DBG_PRINT_INT(sensorVals.vddmv);
	}
	{
		// We dont want to do this or the state change could be missed
		//sensorVals.switchState = SWITCH;
	}

	// Turn off sensors
	SENSORS_ENABLE = 0;
	// Done...
}

unsigned char PirTasks(void)
{
	// PIR tasks state
	static unsigned short lastVal = 0;
	static unsigned short IIRsum = 0;
	static unsigned char  counter = 0, suspect = 0, detected = 1, disarmed = 1; 

	signed short delta;
	unsigned char absDelta;

	// Update the ADC value
	SampleChannel(PIR_CHANNEL);
	// Differentiate
	delta = adcResults.pir - lastVal;
	// Rectify and clamp to 6 bits (for 8bit IIR)
	if(delta > 63 || delta < -63) absDelta = 63; 	
	else if (delta < 0) absDelta = -delta;			
	else absDelta = delta;
	// Update energy accumulator
	sensorVals.pirEnergy += absDelta;							
	// IIR low pass results (+2 bit precision)
	IIRsum = (IIRsum*3) + (absDelta<<2);		
	IIRsum = (IIRsum + 4) >> 2;	
	// Save current sample
	lastVal = adcResults.pir;

	// Time and threshold based detection
	if((absDelta >= settings.pir_suspect_val) && 
		(suspect < settings.pir_suspect_count)){counter = settings.pir_suspect_release; suspect++;}
	else if (counter) 							{counter--;}
	else if (suspect) 							{suspect--;counter = settings.pir_suspect_release;}
	else ;

	// Disarm detector until settled or for timeout
	if(detected == 1)
	{
		if(disarmed == 0) // True after initial detection				
		{	
			disarmed = settings.pir_disarm_time; // Disarm detector for a time
		}
		if(	 IIRsum < settings.pir_threshold 	// If we are not triggered
			|| (--disarmed==0)	)				// or the disarm timer is up
		{
			disarmed = 0;	// Re-arm
			suspect = 0;	// Clear time based detection
			IIRsum = 0;		// Clear filter
			detected = 0;	// Clear flag, will be armed on next sample
		}
	}

	// Suspicion raised above threshold, for distant movement
	else if (suspect >= settings.pir_suspect_count) 
	{
		detected = 1;
		sensorVals.pirSuspCounts++;
	}	

	// Threshold only based trigger, for close proximity
	else if(IIRsum >= settings.pir_threshold)
	{
		// Return detected
		detected = 1;
		sensorVals.pirCounts++; 
	} 

	// Debugging
	DBG_INFO("\r\nPIR,");
	DBG_PRINT_INT(adcResults.pir);
	DBG_INFO(",");
	DBG_PRINT_INT(absDelta);
	DBG_INFO(",");
	DBG_PRINT_INT(IIRsum);
	DBG_INFO(",");
	DBG_PRINT_INT(suspect);
	DBG_INFO(",");
	DBG_PRINT_INT(detected);

	// Return
	return (detected & (disarmed==0));
}

// Test external switch for state
unsigned char SwTasks(void)
{
	// Change in state
	unsigned char swState = (SWITCH)?1:0;
	if(sensorVals.switchState != swState)
	{
		sensorVals.switchState = swState;
		sensorVals.switchCounts++;
		return 1;
	}
	return 0;
}


unsigned short AdcSampleTemp(void)
{
	unsigned short result = 0;
	unsigned char i;
	InitAnalog(); 			// adc to default
	ADCON0 = 0b00000001; 	// adc on
	ADCON0 |= (TEMP_CHANNEL<<2);// adc channel

	// Sample multiple times
	for(i=0;i<64;i++)
	{
		// Sample sensor - ~30us * 64
		ADCON0bits.GO = 1;
		while (ADCON0bits.GO){;}
		result += ((unsigned short)ADRESH<<8) + ADRESL;
	}

	InitAnalog(); 			// Put everything back (Off)
	return result;
}

signed short ConvertTempToCelcius(unsigned short temp)
{
	// Using linear fit - see excel sheet
	unsigned long temp32 = ((unsigned long)sensorVals.temp * 973UL - 15237495UL)>>16;
	signed short result = temp32 & 0xffff;
	return result;
}

unsigned short SampleLight(void)
{
	// Assembles 16bit ADC value using oversampling
	// Specially delayed to get ~10ms average for 
	// fluorescent lamp compensation @ 50Hz
	unsigned char i;
	unsigned long sum = 0;
	InitAnalog(); 					// adc to default
	ADCON0 = 0b00000001; 			// adc on
	ADCON0 |= (LIGHT_CHANNEL<<2); 	// adc channel
	ADCON0bits.GO = 1;
	// Sample multiple times
	for(i=0;i<64;i++)
	{
		// Sample sensor - ~30us
		ADCON0bits.GO = 1;
		while (ADCON0bits.GO){;}
		sum += ((unsigned short)ADRESH<<8) + ADRESL;
		// Sample sensor - ~30us
		ADCON0bits.GO = 1;
		while (ADCON0bits.GO){;}
		sum += ((unsigned short)ADRESH<<8) + ADRESL;
		// Additional delay - 100us
		Delay100us();
		// Total delay = 64 * 160us = 10.2ms
	}
	// Discard 1 bit (10bits * 64 sample = 16bits)
	sum >>= 1;	
	// Turn off ADC
	ADCON0 = 0b00000000; 	// adc off
	// Return result
	return (sum & 0xffff);
}

unsigned short ConvertLightToLux(unsigned short light)
{
	// Convert light value (based on 400uA = 1000lux and 1.8v vref and 2200 Ohm resistor)
	unsigned long temp32 = ((unsigned long)sensorVals.light*2045);
	unsigned short result = temp32>>16; 
	return result;
}

unsigned short SampleFvrForVdd(void)
{
	unsigned short result = 0;
	unsigned char i;
	CVRCON2bits.FVREN = 1;	// FVR on
	Delay100us();			// Wait for stability
	Delay100us();
	InitAnalog(); 			// adc to default
	ADCON1 = 0b00000000; 	// vdd used for reference
	ADCON2 = 0b10101011; 	// Right Justified, 12Tad sample time, IntRC, 20us Taq
	ADCON0 = 0b00000001; 	// adc on
	ADCON0 |= (FVR_CHANNEL<<2);	// adc channel

	// Sample multiple times
	for(i=0;i<64;i++)
	{
		// Sample sensor - ~30us * 64
		ADCON0bits.GO = 1;
		while (ADCON0bits.GO){;}
		result += ((unsigned short)ADRESH<<8) + ADRESL;
	}

	CVRCON2bits.FVREN = 0; 	// FVR off
	InitAnalog(); 			// Put everything back (Off)
	return result;
}

unsigned short ConvertFvrToVddMv(unsigned short fvr)
{
	// Find Vdd in mv (using reference voltage on board, 1200mV)
	unsigned long temp32 = (1200UL<<16)/fvr;
	unsigned short result = temp32 & 0xffff;
	return result;
}

unsigned short SampleHumidity(void)
{
/*
	KL 2013: The sensor used is a resisitive element type. This uses a thin
	and fragile metal oxide layer on a ceramic substrate. The layer is easilly 
	and permanently damages by DC current. To compensate this a high pass or 
	dc blocking capacitor is used. One side of the capacitor is energised to 
	a reference voltage briefly. The capacitor charges through the sensors
	resistance and a second series sense resistance. The induced voltage is
	amplified and measured by the ADC. The amplifier is a programmable gain
	amplifier (PGA) with gains of x1,x10 and x50. Measurements must be taken 
	close to the start of the charge transient otherwise the capacitor voltage
	will have risen too high and the circuit will produce unreliable results.
	A 100us delay has been added to allow the amplifier gain to settle.
	Multiple samples are added to reduce noise and increase resolution.
*/
	#define HUMIDITY_LIMIT (1022ul * 8) /*Could be 1023 * 8 but we must catch OOR errors*/

	unsigned char i;
	unsigned long sumHumid;

	// Sample humidity sensor during the transients
	ADCON0 = 0b00000001; 	// adc on
	ADCON0 |= (HUMID_CHANNEL<<2); // adc channel

	HUMIDITY_GAIN_SEL_50();
	Delay100us();
	sumHumid = 0;
	for(i=0;i<8;i++)
	{
		// Pulse sensor output
		HUMIDITY_PULSE = 1;
		// Sample sensor - ~30us
		ADCON0bits.GO = 1;
		while (ADCON0bits.GO){;}
		// Clear pulse
		HUMIDITY_PULSE = 0;	
		// Get values
		sumHumid += ((unsigned short)ADRESH<<8) + ADRESL;
		// Wait for re-settling
		DelayMs(1);
	}
	if(sumHumid < HUMIDITY_LIMIT)
	{
		// Done...
	}
	else // Out of sensor dynamic range, lower gain
	{
		HUMIDITY_GAIN_SEL_10();
		Delay100us();
		sumHumid = 0;
		for(i=0;i<8;i++)
		{
			// Pulse sensor output
			HUMIDITY_PULSE = 1;
			// Sample sensor - ~30us
			ADCON0bits.GO = 1;
			while (ADCON0bits.GO){;}
			// Clear pulse
			HUMIDITY_PULSE = 0;	
			// Get values
			sumHumid += ((unsigned short)ADRESH<<8) + ADRESL;
			// Wait for re-settling
			DelayMs(1);
		}
		if(sumHumid < HUMIDITY_LIMIT)
		{
			// Correct for additional PGA gain
			sumHumid*=5;	// Note: Max value of 40,960
		}
		else // Still out of range
		{
			// Sample at x10 amplification first with settle time for PGA
			HUMIDITY_GAIN_SEL_1();
			Delay100us();
			sumHumid = 0;
			for(i=0;i<8;i++)
			{
				// Pulse sensor output
				HUMIDITY_PULSE = 1;
				// Sample sensor - ~30us
				ADCON0bits.GO = 1;
				while (ADCON0bits.GO){;}
				// Clear pulse
				HUMIDITY_PULSE = 0;	
				// Get values
				sumHumid += ((unsigned short)ADRESH<<8) + ADRESL;
				// Wait for re-settling
				DelayMs(1);
			}
			// In this mode it can't be out of range but the multiply will break the short
			sumHumid*=50;
		}	
	}
	// Adc and PGA off
	HUMIDITY_GAIN_SEL_10(); // Lowest power
	ADCON0 = 0b00000000; 	// Adc off
	return (sumHumid+4)>>3;	// Set back to 10 bit range
}
																	
// Lookup [temp][humidity] = adcVal																		
const unsigned short lookup[12][15] = {																	
{	0,	11,		23,		54,		115,	234,	447,	852,	1498,	2607,	4470,	7411,	11983,	18168,	25600	}, // 5^C
{	8,	15,		33,		75,		160,	320,	586,	1102,	1903,	3294,	5313,	8533,	13098,	19092,	25600	}, // 10^C
{	10,	21,		47,		102,	216,	430,	792,	1370,	2337,	3858,	6189,	9627,	14080,	19761,	26195	}, // 15^C
{	14,	29,		63,		137,	280,	557,	1004,	1702,	2873,	4655,	6953,	10626,	15222,	20480,	26819	}, // 20^C
{	18,	38,		80,		178,	361,	694,	1263,	2200,	3393,	5313,	8046,	11612,	16091,	21662,	26819	}, // 25^C
{	23,	49,		102, 	229,	465,	922,	1478,	2492,	3994,	6189,	9084,	12656,	17067,	22086,	27473	}, // 30^C
{	28,	63,		129, 	287,	586,	1102,	1782,	2949,	4655,	6953,	9968,	13571,	17879,	22528,	28160	}, // 35^C
{	34,	75,		158, 	350,	694,	1277,	2117,	3498,	5313,	7932,	10831,	14441,	18773,	23467,	28160	}, // 40^C
{	40,	86,		187, 	414,	852,	1498,	2438,	3994,	5867,	8533,	11612,	15222,	19421,	23966,	28882	}, // 45^C
{	45,	102, 	220, 	485,	1004,	1728,	2802,	4470,	6549,	9233,	12378,	15865,	20114,	24487,	28882	}, // 50^C
{	51,	112, 	249, 	557,	1102,	1969,	3112,	4855,	6953,	9881,	12947,	16565,	20480,	24487,	28882	}, // 55^C
{	54,	122, 	273, 	618,	1209,	2158,	3498,	5313,	7411,	10430,	13571,	17067,	20859,	25031,	29642	}};// 60^C
// Lookup temp to index in table												
const signed char temp2index[] = 		{	5,	10,	15,	20,	25,	30,	35,	40,	45,	50,	55,	60	}; 				// x12
// Convert index to humidity																	
const unsigned char index2humidity[] = 	{ 	20,	25,	30,	35,	40,	45,	50,	55,	60,	65,	70,	75,	80,	85,	90}; 	// x15

static const unsigned char num_table_temps = (sizeof(temp2index)/sizeof(unsigned char));
static const unsigned char num_table_humids = (sizeof(index2humidity)/sizeof(unsigned char));

typedef	struct {			// This struct describes a point in the table
	signed char temp;		// Temperature in celcius
	unsigned char index; 	// First lookup index, from temperature
	unsigned short value;	// An analogue value in the table
	unsigned char humidity;	// The humidity associated with the value
} tablePoint_t;

unsigned short ConvertHumidityToSat(unsigned short value, signed char tempC)
{
	unsigned char i,j;
	unsigned short retval = 0;

	tablePoint_t rangeHumidPairs[4];// These are the 4 points to interpolate

	// Find two temp ranges to use
	if(tempC <= temp2index[0])  
	{		
		// Out of range, use first column only	(Tmin)
		rangeHumidPairs[0].temp = temp2index[0];
		rangeHumidPairs[0].index = 0;	
		rangeHumidPairs[1].temp = temp2index[0];
		rangeHumidPairs[1].index = 0;
		rangeHumidPairs[2].temp = temp2index[0];
		rangeHumidPairs[2].index = 0;
		rangeHumidPairs[3].temp = temp2index[0];
		rangeHumidPairs[3].index = 0;
	}
	else if(tempC >= temp2index[num_table_temps-1])
	{ 	
		// Out of range, use last column only (Tmax)	
		rangeHumidPairs[0].temp = temp2index[num_table_temps-1];
		rangeHumidPairs[0].index = num_table_temps-1;	
		rangeHumidPairs[1].temp = temp2index[num_table_temps-1];
		rangeHumidPairs[1].index = num_table_temps-1;
		rangeHumidPairs[2].temp = temp2index[num_table_temps-1];
		rangeHumidPairs[2].index = num_table_temps-1;
		rangeHumidPairs[3].temp = temp2index[num_table_temps-1];
		rangeHumidPairs[3].index = num_table_temps-1;
	}
	else
	{
		for (i=1;i<num_table_temps;i++)
		{
			// If the index is less than the read temp
			if(tempC <= temp2index[i]) 
			{
				rangeHumidPairs[0].temp = temp2index[i-1];
				rangeHumidPairs[0].index = i-1;	
				rangeHumidPairs[1].temp = temp2index[i-1];
				rangeHumidPairs[1].index = i-1;
				rangeHumidPairs[2].temp = temp2index[i];
				rangeHumidPairs[2].index = i;
				rangeHumidPairs[3].temp = temp2index[i];
				rangeHumidPairs[3].index = i;
				break;
			}
		}
	}

	// Now find the range/humidity pairs for each temp index [0] and index [2]
	for(j=0;j<4;j+=2) // Twice, each temperature
	{
		unsigned char tableTempIndex = rangeHumidPairs[j].index;
		if(value <= lookup[tableTempIndex][0])
		{
			// Out of range, use first lookup value only	
			rangeHumidPairs[j].value = lookup[tableTempIndex][0];
			rangeHumidPairs[j].humidity = index2humidity[0];
			rangeHumidPairs[j+1].value = lookup[tableTempIndex][0];
			rangeHumidPairs[j+1].humidity = index2humidity[0];
			continue;
		}
		if(value >= lookup[tableTempIndex][num_table_humids-1])
		{
			// Out of range, use last lookup value only	
			rangeHumidPairs[j].value = lookup[tableTempIndex][num_table_humids-1];
			rangeHumidPairs[j].humidity = index2humidity[num_table_humids-1];
			rangeHumidPairs[j+1].value = lookup[tableTempIndex][num_table_humids-1];
			rangeHumidPairs[j+1].humidity = index2humidity[num_table_humids-1];
			continue;
		}
		for (i=1;i<num_table_humids;i++) 	// Lower range
		{
			// If the index is less than the read temp
			if(value <= lookup[tableTempIndex][i]) 
			{
				rangeHumidPairs[j].value = lookup[tableTempIndex][i-1];
				rangeHumidPairs[j].humidity = index2humidity[i-1];
				rangeHumidPairs[j+1].value = lookup[tableTempIndex][i];
				rangeHumidPairs[j+1].humidity = index2humidity[i];
				break;
			}
		}
	}

	// Now linear interpolate each pair - careful of precision loss and overflows
	{
		signed long humidity[2]; // Extra long values needed for precision
		signed short deltaR, deltaV; // Range delta and value delta
		for(j=0;j<4;j+=2) // Twice
		{
			// Linear interpolation humidity ranges first
			deltaV = rangeHumidPairs[j+1].value - rangeHumidPairs[j].value;		// 16 bits max
			if(deltaV != 0)	// If the values are different (i.e. not out of range limited)
			{
				deltaR = rangeHumidPairs[j+1].humidity - rangeHumidPairs[j].humidity; 			// Small value
				humidity[j>>1] = (signed long)(value - rangeHumidPairs[j].value) * deltaR * 256UL; // Added precision for divide
				humidity[j>>1] = (humidity[j>>1]/deltaV) + ((unsigned long)rangeHumidPairs[j].humidity << 8);
			}
			else
			{
				// Same value (out of range)
				humidity[j>>1] = ((unsigned long)rangeHumidPairs[j].humidity << 8);
			}
		}

		// Linear interpolation accross both temperatures
		// Find difference in temperatures
		deltaV = temp2index[(rangeHumidPairs[2].index)] - temp2index[(rangeHumidPairs[0].index)]; 
		if(deltaV != 0) // If not the same temperature
		{
			signed long temp32 = tempC;
			deltaR = humidity[1] - humidity[0]; // Difference in humidities
			temp32 -= temp2index[(rangeHumidPairs[0].index)]; 
			temp32 *= deltaR;
			retval = (temp32/deltaV) + humidity[0];
		}
		else
		{
			// Same temperature (cause by out of range)
			retval = humidity[0];
		}
	}
	return retval;
}
// EOF


