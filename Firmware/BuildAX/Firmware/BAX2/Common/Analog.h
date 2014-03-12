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

#ifndef _ANALOG_H_
#define _ANALOG_H_

// Types
typedef struct {
	unsigned short temp;
	unsigned short light;
	unsigned short humid;
	unsigned short pir;
} adcResults_t;

typedef struct {
	unsigned short 	humid;
	unsigned short 	temp;
	unsigned short 	light;
	unsigned short 	fvr;
	unsigned short 	humidSat;
	signed short	tempCx10;
	unsigned short 	lightLux;
	unsigned short 	pirCounts;
	unsigned short 	pirSuspCounts;
	unsigned short 	pirEnergy;
	unsigned char 	switchState;
	unsigned short	switchCounts;
	unsigned short 	vddmv;
} sensorVals_t;

// Globals
extern adcResults_t adcResults;
extern sensorVals_t sensorVals;

// Prototypes
extern void InitAnalog(void);
extern void SampleChannel(unsigned char channel);
extern void SampleSensors (void);
extern unsigned char PirTasks(void);
extern unsigned char SwTasks(void);
extern unsigned short AdcSampleTemp(void);
extern signed 	short ConvertTempToCelcius(unsigned short temp);
extern unsigned short SampleLight(void);
extern unsigned short ConvertLightToLux(unsigned short light);
extern unsigned short SampleFvrForVdd(void);
extern unsigned short ConvertFvrToVddMv(unsigned short fvr);
extern unsigned short SampleHumidity(void);
extern unsigned short ConvertHumidityToSat(unsigned short value, signed char tempC);

#endif
// EOF


