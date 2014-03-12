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

// WAX9 Settings
// Karim Ladha, 2013-2014

// This is not the conventional settings struct, it is made to be more applicable to the transmitter HW

#include <string.h>
#include <TimeDelay.h>
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "Settings.h"
#include "Utils/Util.h"
#include "bt app.h"
#include "bt serial.h"
#include "bt config.h"
#include "bt nvm.h"
#include "SamplerStreamer.h"
#include "Analog.h"
#include "Peripherals/MultiSensor.h"
#include "Peripherals/bmp085.h"

// RAM var
settings_t settings;
status_t status;

// Settings in hex file - factory defaults
__prog__ settings_t __attribute__((space(prog),address((SETTINGS_ADDRESS)))) EepromSettings = 
{
		// Factory defaults - DO NOT CHANGE WITHOUT CHANGING CHECKSUM
		.deviceId 				= 0xffff,// DeviceId - will use mac address
#ifdef PRODUCTION_BUILD
		.sleepMode				= 5, 	// Wake->check peripherals->save mac->shipping mode 
#else
		.sleepMode				= 1, 	// Wake->normal discoverable
#endif
		.inactivityThreshold	= 300,	// 5 Minutes	
		.sampleRate      		= 50,	// 50 Hz
		.dataMode     			= 0,	// Ascii
		.highPerformance		= 0,	// Off

		.accelOn				= 1,	// Enabled
		.accelRate				= 200,	// 200 Hz
		.accelRange				= 8,	// +/- 8g
		.gyroOn					= 1,	// Enabled
		.gyroRate				= 200,	// 200 Hz	
		.gyroRange				= 2000, // +/- 2000dps
		.magOn					= 1,	// Enabled
		.magRate				= 10,	// 10 Hz
		.accelInts				= 0,	// Off, set by sleep mode
#ifdef PRODUCTION_BUILD
		.checksum				= 0x04DE // For the factory defaults
#else
		.checksum				= 0x04DA // For the factory defaults
#endif
};

void SettingsInitialize(unsigned char wipe)
{
	unsigned short checksum;
	// Read out of eeprom
	ReadNvm((SETTINGS_ADDRESS), &settings, sizeof(settings_t));
	checksum = FindChecksum (&settings, sizeof(settings_t) - 2);

	if ((wipe) || (checksum != settings.checksum))
	{
		// Defaults
		settings.deviceId 				= 0xffff; 	// Use MAC
		settings.sleepMode				= 1; 		// Default on, low power
		settings.inactivityThreshold	= 300;	
		settings.sampleRate      		= 50;
		settings.dataMode     			= 0;
		settings.highPerformance		= 0;

		settings.accelOn				= 1;
		settings.accelRate				= 200;
		settings.accelRange				= 8;
		settings.gyroOn					= 1;
		settings.gyroRate				= 200;
		settings.gyroRange				= 2000;
		settings.magOn					= 1;
		settings.magRate				= 10;
		settings.accelInts				= 0;

		SettingsSave(); // Regenerates checksum
	}

	// Set accel ints to match sleep mode
	switch(settings.sleepMode){
		case(2):settings.accelInts = 1;break; // Int on movement
		case(3):settings.accelInts = 2;break; // Int on orientation change
		default:settings.accelInts = 0;break; // No accel ints
	};
	
	// Check if we need high performance
	if 	((settings.sampleRate > 100)&&
		((settings.dataMode&0x7f) == 0)) 	
			settings.highPerformance = TRUE;
	else if (settings.sampleRate > 400)
			settings.highPerformance = TRUE;
	else 	settings.highPerformance = FALSE;

	// Status

	// Streaming?
	status.streaming = FALSE;

	// Status sensor data
	status.pressure = 0;
	status.temperature = 0;
	status.battmv = 0;
	status.accelIntPending = 0;
	status.inactivity = 0;
	status.newXdata = FALSE;

	// LED
	status.ledOverride = -1;

	// USB - cant init this since it could cause mismatch whilst changing settings
	// status.usbAtStart = 0; 

	return;
}

void SettingsSave(void)
{
	// Check if we need high performance
	if 	((settings.sampleRate > 100)&&
		((settings.dataMode&0x7f) == 0)) 	// Text mode
	{		
			if(settings.sampleRate > 200)
				settings.sampleRate = 200;	// Clamp
			settings.highPerformance = TRUE;// Faster clock required
	}
	else if (settings.sampleRate > 400)		// Binary slip
	{
			settings.sampleRate = 400;		// Clamp
			settings.highPerformance = TRUE;// Faster clock required
	}
	else 	settings.highPerformance = FALSE;// Faster clock not required

	// Check sleep timeout - clamp
	status.inactivity = 0; // Clear count
	if(settings.inactivityThreshold < 3)settings.inactivityThreshold = 3;
	// Check sleep mode - default if oor
	if(settings.sleepMode > 5)			settings.sleepMode = 1;
	// Check for performance issue
	if ((settings.sampleRate > 100)) 	settings.highPerformance = TRUE;
	else 								settings.highPerformance = FALSE;
	// Check if we need to have accelerometer ints enabled
	switch(settings.sleepMode){
		case(2):settings.accelInts = 1;break; // Wake on movement
		case(3):settings.accelInts = 2;break; // Wake on orientation change
		default:settings.accelInts = 0;break; // No accel ints
	};
	// Save with new checksum
	settings.checksum = FindChecksum (&settings, sizeof(settings_t) - 2);
	WriteNvm(SETTINGS_ADDRESS, &settings,  sizeof(settings_t));
}

char SettingsCommandNumber(unsigned short num)
{
	char retval = TRUE;
	unsigned char settingsChanged = FALSE, doStream = FALSE;

	// Stop streaming
	status.streaming = FALSE; // Can not change settings while streaming

	// "SAVE=0|STREAM|RESET|LED|CLEAR|STOP|PAUSE|PLAY"
	// Only small subset of commands are supported (non-argument types)
	switch (num) {
		case 0 : {
			retval = TRUE;
			settingsChanged = TRUE; // Latch new settings
			break;
		}
		case 1 : {
			retval = TRUE;
			settingsChanged = TRUE; // Save current settings
			doStream = TRUE;		// Stream
			break;
		}
		case 2 : {
			retval = TRUE;
			BTDeinit();
			Reset();
			break;
		}
		case 3 : {
			retval = TRUE;
			status.ledOverride = -1;// LED overide off 
			break;
		}
		case 4 : {
			retval = TRUE;
			SettingsInitialize(1); 	// Load defaults
			settingsChanged=TRUE; 	// Latch new settings
			break;
		}
		case 5 : {
			retval = TRUE;
			settingsChanged = TRUE; // Stop and latch new settings
			break;
		}
		case 6 : {
			retval = TRUE;			// Pause
			break;
		}
		case 7 : {					// Play
			retval = TRUE;
			status.streaming = TRUE;
			break;
		}
		default :  {
			retval = FALSE;
			break;
		}
	}
	// Save new settings if they have changed
	if(settingsChanged==TRUE)
	{
		// Save settings if changed
		SettingsSave();
		if (doStream == TRUE)		
		{
			if (settings.highPerformance && OSCCONbits.COSC !=0b001)
			{
				// Need clock switch first
				HciPhyPause();
				CLOCK_PLL();
				HciPhyAdaptClock(); 
				HciPhyPlay();
			}
			SamplerInitOn();	// Start sampler interrupt and setup sensors
			status.streaming = TRUE;
		}
		else
		{
			if (OSCCONbits.COSC ==0b001) // Need to switch down clock
			{
				// Wait until safe
				HciPhyPause();
				CLOCK_INTOSC();
				HciPhyAdaptClock(); 
				HciPhyPlay();
			}
			// If accel ints is changed - this will adapt accel to use new setting
			MultiStandby(); 
		}
	}
	return retval;
}

// Commands
// Valid strings
const char* cmdstring[] ={
	"HELP", 
	"ECHO",
	"RESET", 
	"SAMPLE",
	"STREAM",
	"LED", 
	"DEVICE",
	"CLEAR",
	"SETTINGS", 
	"RATE",
	"DATAMODE",
	"SLEEPMODE",
	"INACTIVE",
	"NAME",
	"PIN",
	"TEST_RESULTS"	    
}; // Match to ENUM and NUM_OF_CMDS

// Enumerated type (for code readability only)
typedef enum {
	HELP,
	ECHO,
	RESET,
	SAMPLE,
	STREAM,
	LEDSET,
	DEVICE,
	CLEARALL,
	SETTINGS,
	RATE,
	DATAMODE,
	SLEEPMODE,
	INACTIVE,
	NAME,
	PIN,
	TEST_RESULTS,
	INVALID
}cmd_t;

#define NUM_OF_CMDS (sizeof(cmdstring)/sizeof(const char*))

// Serial commands
char SettingsCommand(const char *line)
{
	signed long val;
	unsigned char  settingsChanged = FALSE;
	unsigned char  settingsShow = FALSE;
	unsigned char cmd, strlength;
	char* next = (char*) line; // Next is the next character to read

	do { // Enable chained commands

	    if (next == NULL || strlen(next) == 0) { return 0; }

		// Clear any spaces, commas etc that user may separate cmds by
		while(	(*next == '\r')||
				(*next == '\n')||
				(*next == ',')||
				(*next == ' ')||
				(*next == ':')||
				(*next == '|')||
				(*next == '&'))	next++;

		// Find out which string
		for(cmd=0;cmd<NUM_OF_CMDS;cmd++)
		{
			strlength = strlen(cmdstring[cmd]);
			if (strnicmp(next,cmdstring[cmd], strlength) == 0)
				break;
		}
		
		// Set pointer to next char
		next += strlength; // Pointer to next char after string 
	
		// Process string command
		switch ((cmd_t)cmd) {
			case (HELP):
			{
				printf("COMMANDS:\r\n");
				for(cmd=0;cmd<NUM_OF_CMDS-1;cmd++) // Skip test results command
				{
					printf("%s\r\n",cmdstring[cmd]);
				}
				*next = '\0'; 
				break;
			}
			case (ECHO):
			{
				val = ReadStringInteger(next, &next);
				if (val == 1)commEchoBt = TRUE;
				if (val == 0)commEchoBt = FALSE;
				printf("ECHO: %u\r\n",commEchoBt);
				*next = '\0'; // Don't process further commands
				break;
			}
			case (RESET):
			{
				printf("RESET:\r\n");
				BluetoothSerialWait();
				DelayMs(100); // Wait for resp to send
				BTDeinit();
				Reset();
				*next = '\0'; // Don't process further commands
				break;
			}
			case (SAMPLE):
			{
				unsigned short clkMult = GetSystemClockMIPS() / 4;
				void* string;
	 			// Sample triaxial sensors
				sampleData_t sample;
				sample.sampleCount = 0;
				sample.sampleTicks = 0;
				MultiStartup(&settings);
				DelayMs(10*clkMult);
				MultiSingleSample(&sample.sensor);
				DelayMs(10*clkMult);
				MultiSingleSample(&sample.sensor);
				DelayMs(10*clkMult);
				MultiSingleSample(&sample.sensor); // 3rd sample should be stable
				MultiStandby();
				// Sample pressure sensor and temperature
				DelayMs(30*clkMult);
				BMP085_Initiate_up_conversion();
				DelayMs(30*clkMult);
				if (BMP085_read_up())status.pressure = BMP085_calc_pressure();	
				BMP085_Initiate_ut_conversion();	
				DelayMs(30*clkMult);
				if(BMP085_read_ut())status.temperature = BMP085_calc_temperature();
				// Get battery reading in mv
				GetBatt();
				status.battmv = AdcBattToMillivolt(battRaw);
				// Make ascii data packet		
				MakeDataPacket(&sample, 128, &string); // 128 is ascii, all data
				// Queue for transmission
				printf("DATA: N,Ax,Ay,Az,Gx,Gy,Gz,Mx,My,-Mz,Batmv,Temp0.1C,PresPa,Ia\r\n");
				BluetoothSerialWait();
				printf("%s",(char*)string);
				*next = '\0'; // Don't process further commands
				break;
			}
			case (STREAM):
			{
				if (status.streaming == FALSE)
				{
					settingsChanged = TRUE;	// Adapt settings + streaming
					status.streaming = TRUE;
				}
				*next = '\0'; // Exit this loop now
				break;
			}
			case (LEDSET):
			{
				val = ReadStringInteger(next, &next);
				status.ledOverride = (unsigned char)val;
				printf("LED: %d\r\n",status.ledOverride);
				break;
			}
			case (DEVICE):
			{
				val = ReadStringInteger(next, &next);
				if (val != -1ul) { // If a value was found
					settings.deviceId = val;
					settingsChanged=TRUE;
				}
				else settingsShow=TRUE;
				break;
			}
			case (CLEARALL):
			{
				SettingsInitialize(1); 	// Load defaults
				settingsChanged=TRUE;	// Trigger printout
				break; 
			}
			case (RATE): // All rate cmds, upto 4 args
			{
				unsigned char i;
				static char argc;		// rate char setting eg rate 'x' 1 50 8
				short arg[4]; 	// upto 3 shorts parsed, last one is arg count

				// Early out for non alphabetical first arg or missing space
				if ((next[0] == ' ') && ((next[1]>='A'&& next[1]<='Z') || (next[1]>='a'&& next[1]<='z')))
				{
					// Alphabetic asignor a,g,m,x
					argc = *(next+1);
					
					// ptr incremented past ' ' and letter 
					next += 2;

					// Get numbers, remember how many,
					for(i=0;i<3;i++)
					{
						// Read values from the string
						val = ReadStringInteger(next, &next);
						if (val != -1ul) 	arg[i] = (short)val;
						else 				break;
					}
					arg[3] = i; // Number of arguments
	
					// Set values
					switch(argc){
						case('a') :
						case('A') : { 	if(arg[3] >= 1) settings.accelOn = (arg[0])?1:0;
										if(arg[3] >= 2) settings.accelRate = arg[1];
										if(arg[3] >= 3) settings.accelRange = arg[2];
										break;}
						case('g') : 
						case('G') : { 	if(arg[3] >= 1) settings.gyroOn = (arg[0])?1:0;
										if(arg[3] >= 2) settings.gyroRate = arg[1];
										if(arg[3] >= 3) settings.gyroRange = arg[2];
										break;}
						case('m') :
						case('M') : { 	if(arg[3] >= 1) settings.magOn = (arg[0])?1:0;
										if(arg[3] >= 2) settings.magRate = arg[1];
										break;}
						case('x') : 
						case('X') :	{ 	if(arg[3] > 0)	settings.sampleRate = (arg[(arg[3]-1)]); // Last value
										break;}
						default:	break;
					}
					settingsChanged=TRUE; 
				} // if no space and letter - same as settings
				settingsShow=TRUE;
				break;  
			}
			case (SETTINGS):
			{
				settingsShow=TRUE;	
				break;
			}
			case (DATAMODE):
			{
				val = ReadStringInteger(next, &next);
				if (val != -1ul) {
					settings.dataMode = val;
					settingsChanged=TRUE;
				}
				else settingsShow=TRUE;
				break;
			}
			case (SLEEPMODE):
			{
				val = ReadStringInteger(next, &next);
				if (val != -1ul) {
					settings.sleepMode = val;
					settingsChanged=TRUE;
				}
				else settingsShow=TRUE;
				break;
			}
			case (INACTIVE):
			{
				val = ReadStringInteger(next, &next);
				if (val != -1ul) {
					settings.inactivityThreshold = val;
					if(settings.inactivityThreshold < 3) // Arbitrary min value
						settings.inactivityThreshold = 3;
					settingsChanged=TRUE;
				}
				else settingsShow=TRUE;
				break;
			}
			case (NAME):
			{
				if(*next==' ' || *next=='=')
				{
					do{next++;}while(*next==' ' || *next=='=');
					if(*next!='\0' && *next>='!' && *next<='~' && strlen(next)!=0)
					{
						unsigned short len = strlen(next) + 1; // +1 for null
						if (len>MAX_BT_NAME_LEN)len = MAX_BT_NAME_LEN;
						next[len-1] = '\0';
						memcpy(GetBTName(),next,len);
						WriteNvm((DEVICE_NAME_NVM), next, len);
						printf("NAME: %s\r\n",next);
					}
				}
				else
					printf("NAME: %s\r\n",GetBTName());
				*next = '\0'; // Force break by ending string
				break;
			}
			case (PIN):
			{
				if(*next==' ' || *next=='=')
				{
					do{next++;}while(*next==' ' || *next=='=' || *next=='\0');
					if(*next!='\0' && *next!='\r' && *next!='\n' && (strlen(next)==4))
					{
						next[4] = '\0';
						memcpy(GetBTPin(),next,5);
						WriteNvm((DEVICE_PAIRING_CODE), next, 5);
						printf("PIN: %s\r\n",next);
					}
				}
				else
					printf("PIN: %s\r\n",GetBTPin());
				*next = '\0'; // Force break by ending string
				break;
			}
			case (TEST_RESULTS):
			{
				// Reads eeprom and sends out test data using RFCOMM
				if(STREAM_OUT_BUFFER_SIZE < SELF_TEST_SIZE)break;
				ReadNvm(SELF_TEST_RESULTS,streamerOutBuffer,SELF_TEST_SIZE);
				unsigned short timeout = 10000ul, toSend = 0, sent = 0, remaining = strlen((char*)streamerOutBuffer);
				for(;;)
				{
					while(--timeout && RfcommTxBusy())
						{Delay10us(10);}
					if(timeout == 0) 	
						break;
					toSend = RfcommMaxPayload();
					if (toSend > remaining) 
						toSend = remaining;
					RfcommSendData(streamerOutBuffer+sent,toSend);
					BTTasks(); // Trigger stack to do something
					if(toSend>=remaining)break;
						remaining -= toSend;
					sent += toSend;
				}
				*next = '\0'; // Force break by ending string
				break;
			}
			case (INVALID): // Do not remove, this is the last case if interrator searches whole list above.
			default:
			{
				printf("ERROR: Unknown command\r\n");
				*next = '\0'; //Force break on and unknow cmd
				break;
			}
		}; // End switch

		// Wait until response is delivered
		BluetoothSerialWait();

	// If there are more cmds, process them too
	}while (*next && next);

	// Save new settings if they have changed
	if(settingsChanged == TRUE)
	{
		// Check for performance issue
		if ((settings.sampleRate > 100)&&(settings.dataMode == 0)) 	settings.highPerformance = TRUE;
		else 														settings.highPerformance = FALSE;
		// Check if we need to have accelerometer ints enabled
		switch(settings.sleepMode){
			case(2):settings.accelInts = 1;break; // Wake on movement
			case(3):settings.accelInts = 2;break; // Wake on orientation change
			default:settings.accelInts = 0;break; // No accel ints
		};
		// Save settings if changed
		SettingsSave();
		if(status.streaming)
		{
			// Set new clock if needed
			if (settings.highPerformance && OSCCONbits.COSC !=0b001)
			{
				// Need clock switch first
				HciPhyPause();
				CLOCK_PLL();
				HciPhyAdaptClock(); 
				HciPhyPlay();
			}
			SamplerInitOn();			// Start sampler interrupt and setup sensors
			settingsChanged = FALSE; 	// No response
		}
		else
		{
			if (OSCCONbits.COSC ==0b001) // If we need to switch down clock
			{
				// Wait until safe
				HciPhyPause();
				CLOCK_INTOSC();
				HciPhyAdaptClock(); 
				HciPhyPlay();
			}
			MultiStandby(); 
		}
	}

	// Print out settings if triggered or changed
	if((settingsShow==TRUE) || (settingsChanged==TRUE))
	{
		const unsigned char* mac = (const unsigned char*)GetBTMAC();

		printf(HARDWARE_NAME", HW: %s, FW: %s, CS: CC%u\r\nID: %u\r\n",HARDWARE_VERSION,FIRMWARE_VERSION,chipsetId,settings.deviceId);
		printf("NAME: %s, PIN: %s\r\n",GetBTName(),GetBTPin());
		printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",mac[5],mac[4],mac[3],mac[2],mac[1],mac[0]);
		printf("ACCEL: %u, %u, %u\r\n",settings.accelOn,settings.accelRate,settings.accelRange);
		printf("GYRO: %u, %u, %u\r\n",settings.gyroOn,settings.gyroRate,settings.gyroRange);
		printf("MAG: %u, %u\r\n",settings.magOn,settings.magRate);
		printf("RATEX: %u\r\n",settings.sampleRate);
		printf("DATA MODE: %u%s\r\n",settings.dataMode,((settings.highPerformance)?"!":""));
		printf("SLEEP MODE:%u\r\n",settings.sleepMode);	
		printf("INACTIVE:%usec, %usec\r\n",settings.inactivityThreshold, status.inactivity);		
	}

	return 1;
}

// Adds up chars, returns truncated short sum
unsigned short FindChecksum (void* ptr, unsigned short len)
{
	unsigned short retval = 0;
	for (;len>0;len--) {retval += *(unsigned char*)ptr++;}
	return retval;
}

// Read string for a number and atoi it, if atoi works set ptr to after number
// If atoi fails, set ptr to after number but return -1ul
// If letter encountered, set pointer to it and return -1ul
// Skip over space, comma, colon, equal
signed long ReadStringInteger(char* ptr,char ** end)
{
	signed long val = -1;
	*end = ptr;

	// Void pointer drop out
	if((ptr == NULL)||(*ptr == '\0')||(*end == NULL)){return -1ul;} 					

	// Skip over spaces, commas, colons, equals
	while((*ptr==' ')||(*ptr==',')||(*ptr==':')||(*ptr=='='))ptr++;

	// Convert if numeric, else fail
	if(*ptr == '-' || (*ptr >= '0' && *ptr <= '9')){val = my_atoi(ptr);}
	
	// Skip over number if there was one
	if (val != -1ul)
		{while(*ptr == '-' || (*ptr >= '0' && *ptr <= '9'))ptr++;}
		
	// Skip over trailing spaces, commas and colons
	while((*ptr==' ')||(*ptr==',')||(*ptr==':'))ptr++;

	// Set ptr to the next symbol after the number
	*end = ptr;													
	return (short)val;
}
