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

// Self-test functionality
// KL 16-04-2014

// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Peripherals/Nand.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Rtc.h"
#include "MDD File System/FSIO.h"
#include "Ftl/FsFtl.h"
#include "Utils/Util.h"
#include "Peripherals/Analog.h"
#include "Settings.h"
#include "Logger.h"

extern write_handler_t writeHandler;
extern char FsFtlFormat(char wipe, long serial, const char *volumeLabel);

// Settings for self test
#define MIN_BATT_FOR_LOG_TEST 	648 // 3.8v / ~70%
#define SELF_TEST_SIZE 			512
#define SELF_TEST_RAM_BUFF 		scratchBuffer
// External buffer to use...
extern unsigned char scratchBuffer[640];

static unsigned short selfTestOffset = 0;

static void NvmSelfTestWrite (const void *buffer, unsigned int len)
{
	unsigned char* dest = SELF_TEST_RAM_BUFF + selfTestOffset;
	unsigned short remaining = SELF_TEST_SIZE - selfTestOffset;
	if(remaining < len) return;
	memcpy(dest,(void*)buffer,len);
	selfTestOffset += len;
	return;
}
static unsigned char NvmSelfTestWriteStart(void)
{
	unsigned char read[1];
	unsigned char retval = 0;
	ReadProgram(SELF_TEST_ADDRESS, read, 1);
	if(read[0] == '\f')return retval;
	else if (read[0] == 'W') retval = 2; // Wipe disk
	else retval = 1;

	memset(SELF_TEST_RAM_BUFF,0,SELF_TEST_SIZE+1);	// For null
	stdout->_flag |= _IOBINARY;						// Set binary flag
	writeHandler = NvmSelfTestWrite; 				// Divert printf
	selfTestOffset = 0;
	return retval;
}
static void NvmSelfTestWriteEnd(void)
{
	unsigned long nvmAddress = SELF_TEST_ADDRESS;
	WriteProgramPage(nvmAddress, SELF_TEST_RAM_BUFF, SELF_TEST_SIZE);
	writeHandler = NULL; 		// Reinit
}

void RunTestSequence(void)
{
	int i,j;
	unsigned char battOk = FALSE;
	unsigned char doFormat = FALSE;
	unsigned char doWriteFile = FALSE;
	unsigned char doRunLogTest = FALSE;
	unsigned char testResult = FALSE;

	// Check test entry
	testResult = NvmSelfTestWriteStart();
	switch(testResult) {
		case 0 : return; // New self test not required
		case 2 : doFormat = TRUE; // User requested wipe + test
		case 1 : break;	// Run test
		default : break;
	}	

	// Indicate test running
	LED_SET(LED_GREEN);

	// Self test format version
	printf("\fSELF TEST FORMAT: 1.0\r\n");
	// FW and HW revision
	printf("HW: %01X.%01X, FW: %u\r\n",
	HARDWARE_VERSION>>4, HARDWARE_VERSION&0xf ,SOFTWARE_VERSION);

	// Sensors
	AdcInit();AdcSampleWait();AdcOff();
	if(adcResult.batt >= MIN_BATT_FOR_LOG_TEST)battOk = TRUE;
	printf("BATT = %u mV\r\n", 	AdcBattToMillivolt(adcResult.batt));
	printf("LIGHT = %u\r\n", 	adcResult.ldr);
	printf("TEMPERATURE = %d.%u %cC\r\n",(AdcTempToTenthDegreeC(adcResult.temp)/10),((unsigned short)AdcTempToTenthDegreeC(adcResult.temp)%10),0xF8);

	// RTC / Secondary osc
	unsigned long mips = (OSCCONbits.COSC==0b001)?16000000ul:4000000ul;
	unsigned short start, elapsed, seconds;
	T2CON = 0x0008;T3CON = 0x0000;
	TMR2 = TMR3 = 0;
	PR2 = mips & 0xFFFF;
	PR3 = mips >> 16;
	IFS0bits.T3IF = 0;
	seconds = rtcTicksSeconds;
	start = TMR1;
	T2CONbits.TON = 1;
	while(!IFS0bits.T3IF);
	elapsed = TMR1;
	seconds = rtcTicksSeconds - seconds;
	elapsed -= start;
	// Catch overflow
	elapsed += (32768ul*seconds);
	T2CON = T3CON = 0x0000;
	printf("RTC FREQ = %uHz\r\n", elapsed);

	// LED
	unsigned short deltaAcc[3] = {0};
	signed short delta;
	for(j=0;j<3;j++)
	{
		deltaAcc[j] = 0;
		for(i=0;i<8;i++)
		{
			unsigned char led = ~(1<<j);
			LED_SET(LED_WHITE);
			AdcInit();AdcSampleWait();AdcOff();
			delta = adcResult.ldr;
			LED_SET(led);
			AdcInit();AdcSampleWait();AdcOff();
			delta -= adcResult.ldr;
			if(delta > 0) 	{deltaAcc[j] += delta;}
			else 			{deltaAcc[j] = 0;}
		}
	}
	LED_SET(LED_GREEN);
	printf("RED LED SCORE = %u\r\n",deltaAcc[2]);
	printf("GREEN LED SCORE = %u\r\n",deltaAcc[1]);
	printf("BLUE LED SCORE = %u\r\n",deltaAcc[0]);

	// Nand and filesys
	unsigned char nandId[6] = {0};
	NandInitialize();
	NandVerifyDeviceId();
	NandReadDeviceId(nandId);
	if(!nandPresent)
	{
		printf("ERROR: NO NAND\r\n");
	}
	else
	{
		printf("NAND ID: %02X:%02X:%02X:%02X:%02X\r\n",nandId[0],nandId[1],nandId[2],nandId[3],nandId[4]);
		// FTL & NAND startup
		FtlStartup();                       
		// Check for file sys
		if((FSInit()!=TRUE) || (doFormat == TRUE))
		{
				// Format disk on fail
	        static char volumeBuffer[13] = {0};
			if (settings.deviceId > 99999ul)
			{
				sprintf	(volumeBuffer,"AX3_%07lu",
						settings.deviceId % 10000000);
			}
			else
			{
				sprintf	(volumeBuffer,"AX3%c%c_%05lu",
						('0' + ((HARDWARE_VERSION >> 4) & 0x0f)),
						('0' + ((HARDWARE_VERSION     ) & 0x0f)),
						settings.deviceId);
			}

	        if(FsFtlFormat(TRUE, settings.deviceId, volumeBuffer))
			{
				printf("FORMAT FAILED\r\n\r\n");
			}
			else 
			{	
				printf("FORMAT OK\r\n");
				doWriteFile = TRUE;
				if(battOk)
				{
					printf("LOG TEST SCHEDULED\r\n");
					doRunLogTest = TRUE; // Only if disk formatted
				}
				else
				{
					printf("LOW BATT TEST NOT SCHEDULED\r\n");
				}
			}
		}
		else
		{
			// Indicator to output results to file
			printf("FILE SYSTEM FOUND OK\r\n");
			doWriteFile = TRUE;
		}
	}

	// FTL health
	// FTL=warn#,erro#,spare,2(planes),bad-0,bad-1
	printf("FTL = %s\r\n", FtlHealth());

	// Accel
	AccelVerifyDeviceId();
	AccelStartup((ACCEL_RATE_100 | ACCEL_RANGE_8G));
	if(!accelPresent) printf("ERROR: NO ACCEL\r\n");
	else
	{
		// Get initial multi sensor point
		__attribute__((aligned(2)))accel_t stats[6]; // Current, last, max, min, mean, deviation
		signed long accumulators[3] = {0,0,0}; // for mean
		memset(stats,0,sizeof(accel_t)*6);
	
		// Initialise our mins and maxs
		// Max [2] 
		stats[2].x = 0x8000;
		stats[2].y = 0x8000;
		stats[2].z = 0x8000;
		// Min [3]
		stats[3].x = 0x7FFF;
		stats[3].y = 0x7FFF;
		stats[3].z = 0x7FFF;
	
		for(i=0;i<64;i++) // Dummy 64 samples
		{
			DelayMs(10);
			AccelSingleSample(&stats[1]); // [1] = last sample
		}
	
		for(i=0;i<64;i++) // 64 samples
		{
			DelayMs(10);
			AccelSingleSample(&stats[0]);
	
			for(j=0;j<3;j++) // For x,y,z
			{
				// Max [2] 
				if(stats[0].values[j] > stats[2].values[j])
					stats[2].values[j] = stats[0].values[j];
				// Min [3] 
				if(stats[0].values[j] < stats[3].values[j])
					stats[3].values[j] = stats[0].values[j];
	
				// Sum noise sum to [5]
				if(stats[0].values[j] > stats[1].values[j])
					stats[5].values[j] += (stats[0].values[j] - stats[1].values[j]);
				else 
					stats[5].values[j] += (stats[1].values[j] - stats[0].values[j]);
	
				// Accumulators separate
				accumulators[j] += stats[0].values[j]; 
			}
			// Store this sample to find next deltas
			memcpy(&stats[1],&stats[0],sizeof(accel_t));
		}// for 64
	
		// Device off
		AccelStandby();
	
		// Find mean, (2^6) samples in accumulators, use shift to get mean in [4]
		stats[4].x = accumulators[0] >> 6;
		stats[4].y = accumulators[1] >> 6;
		stats[4].z = accumulators[2] >> 6;
	
		// Print results
		printf("ACCEL LAST = %d, %d, %d\r\n",	stats[1].x, stats[1].y, stats[1].z);
		printf("ACCEL MAX = %d, %d, %d\r\n",		stats[2].x, stats[2].y, stats[2].z);
		printf("ACCEL MIN = %d, %d, %d\r\n",		stats[3].x, stats[3].y, stats[3].z);
		printf("ACCEL MEAN = %d, %d, %d\r\n",	stats[4].x, stats[4].y, stats[4].z);
		printf("ACCEL DEV = %u, %u, %u\r\n",		stats[5].x, stats[5].y, stats[5].z);
	}

	// File output
	if(doWriteFile)
	{
		FSFILE *file = FSfopen("test.txt","w");
		if(file)
		{
			FSfwrite(SELF_TEST_RAM_BUFF, 1, selfTestOffset + 1, file);
			FSfclose(file);
		}
	}

	// Done
	NvmSelfTestWriteEnd();

	// Test log?
	if(doRunLogTest)
	{
		// Set system time
		RtcWrite(RtcFromString("2000/1/1,00:00:00"));
		SettingsInitialize();			// Init
		settings.loggingStartTime = 0; 	// Always log
		settings.loggingEndTime = -1;	// Always log
		settings.maximumSamples = 0;	// Always log
		settings.debuggingInfo = 3;		// Flash logging led
		LoggerWriteMetadata(DEFAULT_FILE, 1); // Commit (with debugging information)
	}

	return;
}
// End of self test

