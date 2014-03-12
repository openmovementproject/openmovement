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

// Settings
// Karim Ladha, 2013-2014

// Include
#include <stdlib.h>
#include <string.h>
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#define MAKE_SETTINGS_TABLE
#include "Settings.h"
#include "Utils.h"
#include "Analog.h"

// Prototypes
int strncasecmp(const char *f1, const char *f2, size_t n );
void PrintSensorValues(void);
void RunStreamer(void);

unsigned short SettingsChecksum(void)
{
	unsigned char i, *ptr = (void*)&settings;
	short checksum = 0;
	// Find checksum, not includeing checksum itself
	for(i=0;i<((sizeof(settings_t))-sizeof(void*));i++) 
	{
		checksum += *ptr++;
	}
	return checksum;
}

void SettingsSave(void)
{
	settings.checksum = SettingsChecksum();
	EepromWrite((unsigned char*)&settings, sizeof(settings_t), SETTINGS_ADDRESS);
}

unsigned char SetttingsAction(unsigned char mode)
{
	unsigned char i,save = FALSE;
/* mode:
#define SETTINGS_LOAD		0x00
#define SETTINGS_CLEAR		0x01
#define SETTINGS_NEW_ID		0x02
#define SETTINGS_NEW_ENCRYP	0x04	
#define SETTINGS_RESET_ALL	0x07
*/
	// Read settings from eeprom
	EepromRead((unsigned char*)&settings, sizeof(settings_t), SETTINGS_ADDRESS);

	// Check settings are valid first (or changing id will reset all)
	if(settings.checksum != SettingsChecksum())
	{
		mode |= SETTINGS_CLEAR; // Need to clear them
		save = TRUE;
	}

	// Check id is valid
	if(	(settings.deviceId.val32&0x00fffffful) == 0x00ffffffUL || 
		(settings.deviceId.val32&0x00fffffful) == 0x00000000UL ||
		(mode & SETTINGS_NEW_ID) )
	{
		// Make a random id
		settings.deviceId.val8[3] = HEADER_BYTE_MSB; 
		WriteRandom(settings.deviceId.val8, 3);
		save = TRUE;
	}

	// Check for invalid encryption 
	for(i=0;i<AES_KEY_SIZE;i++)
	{
		if(settings.key[i] != 0xff && settings.key[i] != 0x00) 
			break;
	}
	if((mode & SETTINGS_NEW_ENCRYP) || (i == AES_KEY_SIZE))
	{
		// Need to generate encryption mask
		WriteRandom(&settings.key, AES_KEY_SIZE);
		// Save encryption packet	
		save = TRUE;
	}

	// If setting to defaults
	if(mode & SETTINGS_CLEAR)
	{
		settings.batt_min = VBATT_MIN;
		settings.tx_power = RADIO_TX_PWR_DBM;
		settings.pir_threshold = PIR_THRESHOLD;
		settings.pir_suspect_val = PIR_THRESHOLD_SUSPECT_VAL;
		settings.pir_suspect_count = PIR_THRESHOLD_SUSPECT_COUNT;
		settings.pir_suspect_release = PIR_THRESHOLD_SUSPECT_RELEASE;
		settings.pir_disarm_time = PIR_DISARM_TIME;
		settings.pir_led_time = PIR_LED_TIME;
		settings.sensor_settle_time = PIR_SETTLE_TIME;
		settings.sw_led_time = SW_LED_TIME;
		settings.credit_interval = CREDIT_INTERVAL;
		settings.max_pir_credits = MAX_PIR_CREDITS;
		settings.max_switch_credits = MAX_SWITCH_CREDITS;
		settings.sample_interval = SAMPLE_INTERVAL;
		settings.num_tx_repeats = SAMPLE_RETRANSMITS;
		save = TRUE;
	}
	
	// Do we need to save
	if(save)
	{
		SettingsSave();
	}
	return save;
}

void SettingCommandMode(void)
{
	unsigned char run = TRUE;
	unsigned char print_all = TRUE;
	char* line;

	while(run)
	{
		// Print all settings
		if(print_all == TRUE)
		{
			unsigned short val;
			unsigned char i;
			print_all = FALSE;
			// Print all settings
			PrintConst("\r\nSettings:\r\n");
			PrintConst("Device:");
			PrintConst(Byte2Hex(settings.deviceId.val8[3]));
			PrintConst(Byte2Hex(settings.deviceId.val8[2]));
			PrintConst(Byte2Hex(settings.deviceId.val8[1]));
			PrintConst(Byte2Hex(settings.deviceId.val8[0]));
			PrintConst("\r\n");
			for(i=0;i<(sizeof(setting_val)/sizeof(setting_vals_t));i++)
			{
				PrintConst(setting_val[i].name);
				val = 0;
				if (setting_val[i].len == 0)		{PrintConst("\r\n");continue;}
				else if (setting_val[i].len == 1)	{val = ((unsigned char*)setting_val[i].address)[0];}
				else if(setting_val[i].len == 2)	{val = (((unsigned short)((unsigned char*)setting_val[i].address)[1])<<8)+((unsigned char*)setting_val[i].address)[0];}
				PrintConst(" = ");
				PrintUint(val);
				PrintConst("\r\n");
			}
			PrintConst("Commands:save | defaults | renew | reset | sample | stream | exit (esc)\r\n");
		}		

		// Get a uart line
		LED_SET(LED_RED);
		COMM_CLEAR_ERRS(); // Check uart errors
		line = UartGetLine();

		// Inspect line
		if (line == NULL) 
			run = FALSE; // Exit
		else
		{
			unsigned char i = 0;
			LED_SET(LED_OFF);
			// Check commands first
			if(line[0] == '\0')
			{
				print_all = TRUE;
				continue;
			}
			else if (strncasecmp(line,"save",4) == 0)
			{
				SettingsSave();
				print_all = TRUE;
				PrintConst("\r\nSaved.\r\n");
				continue;
			}
			else if (strncasecmp(line,"defaults",8) == 0)
			{
				SetttingsAction(SETTINGS_CLEAR);
				print_all = TRUE;
				PrintConst("\r\nDone.\r\n");
				continue;
			}
			else if (strncasecmp(line,"renew",5) == 0)
			{
				SetttingsAction(SETTINGS_RESET_ALL);
				print_all = TRUE;
				PrintConst("\r\nDone.\r\n");
				continue;
			}
			else if (strncasecmp(line,"exit",4) == 0)
			{
				PrintConst("\r\nOk.\r\n");
				run = 0; 
				continue;
			}
			else if (strncasecmp(line,"sample",6) == 0)
			{
				SampleSensors();
				SampleChannel(PIR_CHANNEL);	
				PrintSensorValues();
				continue;
			}
			else if (strncasecmp(line,"stream",6) == 0)
			{
				RunStreamer();
				PrintConst("\f");
				print_all = TRUE;
				continue;
			}
			else if (strncasecmp(line,"reset",5) == 0)
			{
				PrintConst("\r\nOk.\r\n");
				LED_SET(LED_OFF);
				Reset();
			}
			// Now compare to every named variable in settings
			else
			{
				for(i=0;i<num_of_settings;i++)
				{
					unsigned char len = strlen(setting_val[i].name);
					if (strncasecmp(line,setting_val[i].name,len) == 0)
					{
						unsigned long temp = my_atoi(line + len);
						if(temp != 0xffffffff)
						{
							unsigned char val[2];
							val[0] = temp;
							val[1] = temp>>8;
							if(setting_val[i].len == 2)			{((unsigned char*)setting_val[i].address)[1] = val[1]; 
																((unsigned char*)setting_val[i].address)[0] = val[0];}
							else if (setting_val[i].len == 1)	{((unsigned char*)setting_val[i].address)[0] = val[0];}
							else ;
						}
						i = 0; // Indicate it was processed using index var
						print_all = TRUE;
						break;
					}
				}// for
			}

			// See if it was processed
			if(i!=0)	
			{			
				PrintConst("\r\nNot recognised:<");
				PrintConst(line);
				PrintConst(">\r\n");
			}
		} // if line != null
	}// while(run)
	LED_SET(LED_OFF);
}

void PrintSensorValues(void)
{
	signed short tempAbs = sensorVals.tempCx10;
	PrintConst("\r\nSample:");
	PrintConst("\r\nTemp:");
	if(tempAbs < 0)
		{tempAbs=-tempAbs;PrintConst("-");}
	PrintUint(tempAbs / 10);
	PrintConst(".");
	PrintUint(tempAbs % 10);

	PrintConst("\r\nHumidity:");
	PrintUint((sensorVals.humidSat>>8));
	PrintConst(".");
	PrintUint((40*(sensorVals.humidSat&0xff))>>10);
	PrintConst("\r\nLight:");
	PrintUint(sensorVals.lightLux);
	PrintConst("\r\nBatt:");
	PrintUint(sensorVals.vddmv);	
	PrintConst("\r\nPIR:");
	PrintUint(adcResults.pir);	
	PrintConst("\r\nSW:");
	(sensorVals.switchState)?PrintConst("1"):PrintConst("0");
}

void RunStreamer(void)
{
	while((!BUTTON)&&(!COMM_HASCHAR())&&(RX_LEVEL))
	{
		SampleSensors();
		SampleChannel(PIR_CHANNEL);
		sensorVals.switchState = SWITCH;		PrintConst("\f<Button/key exit>");
		PrintSensorValues();
		DelayMs(50);
	}
}

int strncasecmp(const char *f1, const char *f2, size_t n )
{
	char c1,c2;
	for(;;)
	{
		if(n-- == 0) return 0; 	// Identical for field len
		c1=*f1++; c2=*f2++;		// De-reference
		if((c1 == '\0')||(c2=='\0')) // Check null
			return (-1);		// String end before n
		if(c1>='a' && c1<='z')	// To upper c1
			c1 -= ('a' - 'A'); 	// Subtract 32
		if(c2>='a' && c2<='z')	// To upper c2
			c2 -= ('a' - 'A'); 	// Subtract 32
		if(c1 != c2) 			// Compare
			return (c1-c2);		// Fail
		else ; 					// Next char
	}

}

// EOF
