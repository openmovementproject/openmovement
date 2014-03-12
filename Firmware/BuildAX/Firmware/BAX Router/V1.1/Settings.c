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

// Configuration settings
// Karim Ladha, 2013-2014

// Includes
#include "HardwareProfile.h"
#include "Settings.h"
#ifdef USE_FAT_FS
#include "FatFs/FatFsIo.h"
#else
#include "MDD File System/FSIO.h"
#endif
#include "USB/USB_CDC_MSD.h"
#include "TCPIP Stack/TelnetLocal.h"
#include "TcpIpTasks.h"
#include "Utils/FSutils.h"
#include "Utils/Util.h"
#include "TimeDelay.h"
#include "Peripherals/ExRtc.h"
#include "Peripherals/Rtc.h"
#include "Data.h"

Settings_t 	settings = {0};
Status_t 	status = {0};

// Reset settings from ROM
void SettingsInitialize(void)
{
	// Load defaults first
	SettingsToDefault(); // Initialise struct

	// Check for SD card settings
	SD_ENABLE(); 		 // Turn on SD card
	if(!SD_CD)
	{
		status.file_state = PRESENT;
		if (FSInit() == TRUE)	// Init FS (calls media init)
		{
			status.file_state = INITIALISED;
			if(SettingsReadFile(SETTINGS_FILE))
			{
				// New settings from file found
				SettingsSave();					// To eeprom
				settings.settings_source = 2; 	// From file
				SettingsAction(ACTION_DELETE_SET);// Erase file
			}
		}
		else
		{
			// FS init fail
			status.file_state = ERROR;
			SD_DISABLE(); 
		}
	}
	else
	{
		// Card not detected
		status.file_state = NOT_PRESENT;
	}

	// If no settings file - check eeprom
	if(settings.settings_source == 0)
	{
		unsigned short check;
		// Read eeprom settings
		ReadNvm(SETTINGS_ADDRESS, (unsigned char*)&settings, sizeof(Settings_t));
		// Enusure the settings are valid
		check = checksum(&settings, (sizeof(Settings_t)-2));	
		if(check != settings.checksum) 	// Mis-match
		{
			SettingsToDefault();			// Clear settings on checksum mismatch
			SettingsSave();					// Save defaults
		}
		else
		{
			settings.settings_source = 1; 	// From eeprom
			settings.session_number++;		// Increment session number
			SettingsSave();					// Save
		}
	}

	// Set start time for this session
	settings.start_time = RtcNow();	
	
	// Open the error log file
	#ifdef ERROR_FILE
	status.error_file = FSfopen(ERROR_FILE, "a");
	#endif

	// Open a log file (will use loaded settings)
	SettingsAction(ACTION_START_FILE);	// Try start new file
	if(status.file_state == ACTIVE) 	// If logging to disk
	{
		// Always set session number and time on open
		status.usb_disk_wp = 1; 
		settings.session_number++;
		status.session_number = settings.session_number;
		status.session_start_time = settings.start_time;
		// Save setting number - so on reset it starts another file
		SettingsSave();	
	}	

	return;
}

void SettingsToDefault(void)
{
	settings.settings_source = 0;	// Defaults
	settings.deviceId = 0;			
	memcpy(settings.device_name, "BAX-ROUTER", 11);			
	settings.start_time = 0UL;	

	// Radio (Si44)
	settings.radioSubnetMask = 0xFF00;
	settings.radioSubnet = 0x4200; //'B' Header
	
	// Logging / forwarding settings
	settings.usb_stream = STATE_OFF;		
	settings.usb_stream_mode = 0;

	// File logging
	settings.file_stream = STATE_OFF;		
	settings.file_stream_mode = 0;	
	settings.session_number = 0;
	settings.save_interval = DEFAULT_FILE_SAVE_INTERVAL;
	settings.max_file_time = DEFAULT_MAX_FILE_TIME;
	settings.max_file_size = DEFAULT_MAX_FILE_SIZE;
	sprintf(settings.output_text_file,"Not created");
	sprintf(settings.output_bin_file,"Not created");
	status.session_start_time = 0;
	status.output_text_file = NULL;
	status.output_bin_file = NULL;

	settings.udp_stream = STATE_OFF;
	settings.udp_stream_mode = 0;

	settings.gsm_stream = STATE_OFF;
	settings.gsm_stream_mode = 0;

	// Telnet
	settings.telnet_stream = STATE_OFF;
	settings.telnet_stream_mode = 0;	
	memcpy(settings.telnet_username,DEFAULT_TELNET_NAME,1+strlen(DEFAULT_TELNET_NAME));
	memcpy(settings.telnet_password,DEFAULT_TELNET_PASS,1+strlen(DEFAULT_TELNET_PASS));

	// Ethernet
	memset(settings.mac,0,7);

	settings.checksum = checksum(&settings, (sizeof(Settings_t)-2));

	// Status
	status.usb_disk_mounted = 1;	// Set in SettingsInitialize()
	status.usb_disk_wp		= 0;	// Set when logging to file
	status.file_state 		= OFF; 	// Set in SettingsInitialize()
	status.session_number	= 0;	// session number of current file
	status.session_start_time=0;	// start time of current session
	status.output_text_file	= NULL;	// Set in SettingsInitialize()
	status.output_bin_file	= NULL;	// Set in SettingsInitialize()
	status.error_file		= NULL;	// Set in SettingsInitialize()
	status.usb_state 		= OFF;	// Set in main()
	status.ethernet_state 	= OFF;	// Set externally
	status.gsm_state 		= OFF;	// Set externally
	status.radio_state 		= OFF;	// Set in main()
	status.mrf_state 		= OFF;	// Set in main()
	status.udp_state		= OFF;	// Set externally
	status.udp_mask			= 0;	// Set externally
	status.telnet_state    	= OFF;	// Set in telnetLocal.c 
	status.telnet_mask		= 0;	// Set in telnetLocal.c
	return;
}

void SettingsSave(void)
{
	// Make sure checksum matches and save to eeprom
	settings.settings_source = 1; // Change source to eeprom
	settings.checksum = checksum(&settings, (sizeof(Settings_t)-2));
	WriteNvm(SETTINGS_ADDRESS, (unsigned char*)&settings, sizeof(Settings_t));
}

// Read settings from config file
char SettingsReadFile(const char *filename)
{
    FSFILE *fp;
    static char lineBuffer[64];
    char *line;

    // Read auto-exec config file
    fp = FSfopen(filename, "r");
    if (fp == NULL) { return 0; }
    for (;;)
    {
        line = (char*)FSfgets(lineBuffer, 64, fp);
        if (line == NULL) { break; }
        SettingsCommand(line, SETTINGS_BATCH, 0);
    }
    FSfclose(fp);

    return 1;
}


char SettingsAction(char flags)
{
    char ret = 0;
	FILE_LED = !FILE_LED; // Flicker LED
    if (flags & ACTION_SETTINGS_WIPE)
	{
		// Factory defaults
		SettingsToDefault();				// Defaults
		SettingsSave();						// Save settings
		ret |= ACTION_SETTINGS_WIPE;
	}
    if (flags & ACTION_UNMOUNT)
    {
        status.usb_disk_mounted = 0;
		status.usb_disk_wp = 1;
		ret |= ACTION_UNMOUNT;
    }
	// Check file system is initialised
	if(status.file_state >= INITIALISED)
	{
		// Ordering is important
	    if (flags & ACTION_DELETE_LOGS)
	    {
			if((settings.output_bin_file) && (!FSremove(settings.output_bin_file)))
				ret |= ACTION_DELETE_LOGS;
			if((settings.output_text_file) && (!FSremove(settings.output_text_file)))
				ret |= ACTION_DELETE_LOGS;
	    }
	    if (flags & ACTION_SAVE)
	    {
			if(status.output_text_file)
			{
	        	FSfclose(status.output_text_file);
				status.output_text_file = FSfopen(settings.output_text_file, "a");
			}
			if(status.output_bin_file)
			{
	        	FSfclose(status.output_bin_file);
				status.output_bin_file = FSfopen(settings.output_bin_file, "a");
			}
			if(status.output_bin_file && status.output_text_file)
				ret |= ACTION_SAVE;
	    }
	    if (flags & ACTION_DELETE_SET)
	    {
	        if(!FSremove(SETTINGS_FILE))ret |= ACTION_DELETE_SET;
	    }
	    if (flags & ACTION_FORMAT)
	    {
	        char volumeBuffer[13] = {0};
	        volumeBuffer[0]  = 'B';
	        volumeBuffer[1]  = 'A';
	        volumeBuffer[2]  = 'X';
	        volumeBuffer[3]  = '0' + ((HARDWARE_VERSION >> 4) & 0x0f);
			volumeBuffer[4]  = '.';
	        volumeBuffer[5]  = '0' + ((HARDWARE_VERSION     ) & 0x0f);
	        volumeBuffer[6]  = '\0';
	 		if (!FSformat(0, settings.deviceId, (char *)volumeBuffer))
			{
				ret |= ACTION_FORMAT;
				status.output_text_file = NULL; // Ensure we invalidate file ptrs
				status.output_bin_file = NULL; 	// Ensure we invalidate file ptrs
			}	
	    }
	    if (flags & ACTION_START_FILE)
	    {
			status.usb_disk_mounted = 0;			// Stop external card access
			status.usb_disk_wp = 1;					// Protect card

			if(status.output_text_file != NULL)		// If we have an open file, close it
				FSfclose(status.output_text_file);
			if(status.output_bin_file != NULL)		// If we have an open file, close it
				FSfclose(status.output_bin_file);

			// Only if we are supposed to be logging
			if (settings.file_stream == STATE_ON)	
			{
				// Generate new names from session
				sprintf(settings.output_text_file,TEXT_FILE_NAME_FMT,settings.session_number);
				sprintf(settings.output_text_file,BIN_FILE_NAME_FMT,settings.session_number);
				// Open new files (status is FSFILE, settings is string)
				status.output_text_file = FSfopen(settings.output_text_file, "a");
				status.output_bin_file = FSfopen(settings.output_bin_file, "a");
				// Add session tag to plain text file
				if(status.output_text_file != NULL) 
				{
					char line[64];
					// Add session info to file (text)
					int len = sprintf(line, "Session: %u\r\nTime: %s\r\n",
						settings.session_number,RtcToString(settings.start_time));
					FSfwrite(line, 1, len, status.output_text_file);
				}
			}
			// Check it went ok - for either file
			if(status.output_text_file || status.output_bin_file)
			{
				status.file_state = ACTIVE;
				ret |= ACTION_START_FILE;
			}
			else // No files open (logging off) 
			{
				if(settings.file_stream == STATE_ON) // Then this is a fail state
					status.file_state = ERROR;
				else
					status.file_state = INITIALISED; // Not logging
				status.usb_disk_wp = 0;
				status.usb_disk_mounted = 1;
			}	
		}
	}
    if (flags & ACTION_MOUNT)
    {
        status.usb_disk_mounted = 1;
		if(status.file_state == ACTIVE)	// Logging to disk
			status.usb_disk_wp = 1;
		else
			status.usb_disk_wp = 0;	
		ret |= ACTION_MOUNT;
    }
    return ret;
}



// Serial commands
char SettingsCommand(const char *line, unsigned char mode, unsigned char source)
{
// Redirect settings
if (strnicmp(line, "setting.", 8) == 0)
{
	// Change to settings
	return SettingsCommandSetting(line, mode, source);
}
else
{
	char resp[128], *ptr; 			// Briefly on the stack
	unsigned short respFlags;		// Set response flags
	switch (mode) {
	case SETTINGS_BATCH	: 	respFlags = 0;break; 
	case SETTINGS_CDC	:	respFlags = TXT_CDC_FLAG;break;
	case SETTINGS_TELNET:	respFlags = TXT_TEL_FLAG;break; 
	case SETTINGS_UDP 	:	respFlags = TXT_UDP_FLAG;break;
	default 			: 	respFlags = 0;break; 
	};

    if (line == NULL || strlen(line) == 0) { return 0; }

	// Begin response
	ptr = resp;

    if (strnicmp(line, "help", 4) == 0)
    {
        ptr+=sprintf(ptr,"HELP: help|status|time|reset|mount|exit|format|save|setting.|\r\n");
    }
    else if (strnicmp(line, "status", 6) == 0)
    {
		// Device type
		ptr+=sprintf(ptr,"BAX, HW:%u.%u, FW:%u.%u",
			(HARDWARE_VERSION/10),(HARDWARE_VERSION%10),
			(FIRMWARE_VERSION/10),(FIRMWARE_VERSION%10));
		
		// Id and name
		ptr+=sprintf(ptr,"\r\nId: %08lX, %s",settings.deviceId, settings.device_name);
		ptr+=sprintf(ptr,"\r\nSession: %u",settings.session_number);

		// Add interim response element (long response)
		AddDataElement(TYPE_TEXT_ELEMENT,source, (ptr-resp), respFlags , resp); 	

		// Begin response 
		ptr = resp;

		// System
		ptr+=sprintf(ptr,"\r\nStart time: %s", RtcToString(settings.start_time));
		ptr+=sprintf(ptr,"\r\nTime:       %s", RtcToString(RtcNow()));
		ptr+=sprintf(ptr,"\r\nSysVolts: %umV", status.sysVolts_mV);
		ptr+=sprintf(ptr,"\r\nVddVolts: %umV", status.vddVolts_mV);

		// Add interim response element (long response)
		AddDataElement(TYPE_TEXT_ELEMENT,source, (ptr-resp), respFlags , resp); 	

		// Begin response
		ptr = resp;

		ptr+=sprintf(ptr,"\r\nSysTemp: %u.%u%cC", (status.sysTemp_x10C/10),(status.sysTemp_x10C%10),0xf8);
		ptr+=sprintf(ptr,"\r\nSettings: ");
		switch(settings.settings_source){
			case (0) : ptr+=sprintf(ptr,"Default");break;
			case (1) : ptr+=sprintf(ptr,"From eeprom");break;
			case (2) : ptr+=sprintf(ptr,"From file");break;
			case (3) : ptr+=sprintf(ptr,"Local");break;
			default:break;
		};

		// Sd card
		ptr+=sprintf(ptr,"\r\nFile: , Mode: %u",settings.file_stream_mode);
		ptr+=sprintf(ptr,"\r\nSetting: ");
		switch(settings.file_stream){
			case (STATE_ON):	ptr+=sprintf(ptr,"Turned on");break;
			default:			ptr+=sprintf(ptr,"Turned off");break;
		};		
		ptr+=sprintf(ptr,"\r\nStatus:  ");
		switch(status.file_state){
			case (NOT_PRESENT):	ptr+=sprintf(ptr,"No card");break;
			case (ERROR):		ptr+=sprintf(ptr,"Error");break;
			case (OFF):			ptr+=sprintf(ptr,"Turned off");break;
			case (STATE_ON):	// Fall through
			case (INITIALISED):	ptr+=sprintf(ptr,"Connected");break;
			case (ACTIVE):		ptr+=sprintf(ptr,"Logging");break;
			default:break;
		};

		// Add interim response element (long response)
		AddDataElement(TYPE_TEXT_ELEMENT,source, (ptr-resp), respFlags , resp); 	

		// Begin response
		ptr = resp;

		// USB/CDC port
		ptr+=sprintf(ptr,"\r\nUSB: , Mode: %u",settings.usb_stream_mode);
		ptr+=sprintf(ptr,"\r\nSetting: ");
		switch(settings.usb_stream){
			case (STATE_ON):	ptr+=sprintf(ptr,"Turned on");break;
			default:			ptr+=sprintf(ptr,"Turned off");break;
		};	
		ptr+=sprintf(ptr,"\r\nStatus:  ");
		switch(status.usb_state){
			case (NOT_PRESENT):	ptr+=sprintf(ptr,"Not detected");break;
			case (ERROR):		ptr+=sprintf(ptr,"Error");break;
			case (OFF):			ptr+=sprintf(ptr,"Turned off");break;
			case (PRESENT):		ptr+=sprintf(ptr,"Power only");break;
			case (INITIALISED):	ptr+=sprintf(ptr,"Connected");break;
			case (ACTIVE):		ptr+=sprintf(ptr,"Streaming");break;
			default:break;
		};

		// UDP
		ptr+=sprintf(ptr,"\r\nUdp: , Mode: %u",settings.udp_stream_mode);
		ptr+=sprintf(ptr,"\r\nSetting: ");
		switch(settings.udp_stream){
			case (STATE_ON):	ptr+=sprintf(ptr,"Turned on");break;
			default:			ptr+=sprintf(ptr,"Turned off");break;
		};	
		ptr+=sprintf(ptr,"\r\nStatus:  ");
		switch(status.udp_state){
			case (NOT_PRESENT):	ptr+=sprintf(ptr,"Not detected");break;
			case (ERROR):		ptr+=sprintf(ptr,"Error");break;
			case (OFF):			ptr+=sprintf(ptr,"Turned off");break;
			case (STATE_ON):	// Fall through
			case (INITIALISED):	ptr+=sprintf(ptr,"Connected");break;
			case (ACTIVE):		ptr+=sprintf(ptr,"Streaming");break;
			default:break;
		};

		// Add interim response element (long response)
		AddDataElement(TYPE_TEXT_ELEMENT,source, (ptr-resp), respFlags , resp); 
	
		// Begin response
		ptr = resp;

		// GSM
		ptr+=sprintf(ptr,"\r\nGSM: , Mode: %u",settings.gsm_stream_mode);
		ptr+=sprintf(ptr,"\r\nSetting: ");
		switch(settings.gsm_stream){
			case (STATE_ON):	ptr+=sprintf(ptr,"Turned on");break;
			default:			ptr+=sprintf(ptr,"Turned off");break;
		};	
		ptr+=sprintf(ptr,"\r\nStatus:  ");
		switch(status.gsm_state){
			case (NOT_PRESENT):	ptr+=sprintf(ptr,"Not detected");break;
			case (ERROR):		ptr+=sprintf(ptr,"Error");break;
			case (OFF):			ptr+=sprintf(ptr,"Turned off");break;
			case (STATE_ON):	// Fall through
			case (INITIALISED):	ptr+=sprintf(ptr,"Connected");break;
			case (ACTIVE):		ptr+=sprintf(ptr,"Streaming");break;
			default:break;
		};

		// Add interim response element (long response)
		AddDataElement(TYPE_TEXT_ELEMENT,source, (ptr-resp), respFlags , resp); 	

		// Begin response
		ptr = resp;

		// Telnet
		ptr+=sprintf(ptr,"\r\nTelnet: , Mode: %u",settings.telnet_stream_mode);
		ptr+=sprintf(ptr,"\r\nSetting: ");
		switch(settings.telnet_stream){
			case (STATE_ON):	ptr+=sprintf(ptr,"Turned on");break;
			default:			ptr+=sprintf(ptr,"Turned off");break;
		};	
		ptr+=sprintf(ptr,"\r\nStatus:  ");
		switch(status.telnet_state){
			case (ERROR):		ptr+=sprintf(ptr,"Error");break;
			case (OFF):			ptr+=sprintf(ptr,"Turned off");break;
			case (STATE_ON):	// Fall through
			case (INITIALISED):	ptr+=sprintf(ptr,"Connected");break;
			case (ACTIVE):		ptr+=sprintf(ptr,"Streaming");break;
			default:break;
		};

		// Radios
		ptr+=sprintf(ptr,"\r\nSi44 Radio:  ");
		switch(status.radio_state){
			case (ERROR):		ptr+=sprintf(ptr,"Error");break;
			case (OFF):			ptr+=sprintf(ptr,"Off");break;
			case (STATE_ON):	// Fall through
			case (INITIALISED):	// Fall through
			case (ACTIVE):		ptr+=sprintf(ptr,"On");break;
			default:break;
		};
		ptr+=sprintf(ptr,"\r\nMRF  Radio:  ");
		switch(status.mrf_state){
			case (ERROR):		ptr+=sprintf(ptr,"Error");break;
			case (OFF):			ptr+=sprintf(ptr,"Off");break;
			case (STATE_ON):	// Fall through
			case (INITIALISED):	// Fall through
			case (ACTIVE):		ptr+=sprintf(ptr,"On");break;
			default:break;
		};

		// Add interim response element (long response)
		AddDataElement(TYPE_TEXT_ELEMENT,source, (ptr-resp), respFlags , resp); 	

		// Begin response
		ptr = resp;

		ptr+=sprintf(ptr,"\r\nETHERNET: ");
		switch(status.ethernet_state){
			case (ERROR):		ptr+=sprintf(ptr,"Error");break;
			case (OFF):			ptr+=sprintf(ptr,"Turned off");break;
			case (STATE_ON):	// Fall through
			case (INITIALISED):	ptr+=sprintf(ptr,"Disconnected");break;
			case (ACTIVE):		ptr+=sprintf(ptr,"Connected");break;
			default:break;
		};
		ptr+=sprintf(ptr,"\r\nMAC:%02X:%02X:%02X:%02X:%02X:%02X",
		settings.mac[0],settings.mac[1],settings.mac[2],settings.mac[3],settings.mac[4],settings.mac[5]);
		ptr+=sprintf(ptr,"\r\nIP: %u.%u.%u.%u",
		AppConfig.MyIPAddr.v[0],AppConfig.MyIPAddr.v[1],AppConfig.MyIPAddr.v[2],AppConfig.MyIPAddr.v[3]);
		ptr+=sprintf(ptr,"\r\nMSK:%u:%u:%u:%u",
		AppConfig.MyMask.v[0],AppConfig.MyMask.v[1],AppConfig.MyMask.v[2],AppConfig.MyMask.v[3]);
		ptr+=sprintf(ptr,"\r\n");

	}
	
    else if (strnicmp(line, "time", 4) == 0)
    {
        unsigned long time = RtcNow();
        if (line[4] != '\0')
        {
            time = RtcFromString(line + 5);
            if (time != 0)
            {
				ExRtcWriteTime(time);
                RtcWrite(time);
            }
            else
            {
                ptr+=sprintf(ptr,"ERROR: Problem parsing time.\r\n");
            }
        }
		ptr+=sprintf(ptr,"Time: %s\r\n", RtcToString(time));
    }
    else if (strnicmp(line, "mount", 5) == 0)
    {
        int value = -1;
        if (line[5] != '\0') { value = (int)my_atoi(line + 6); }
		if(value == 1) 
		{
			SettingsAction(ACTION_SAVE);	// Finalise file before mounting
			SettingsAction(ACTION_MOUNT);	// Mount
		}
		if(value == 0) SettingsAction(ACTION_UNMOUNT);
        ptr+=sprintf(ptr,"Mounted: %u, %u\r\n", status.usb_disk_mounted, status.usb_disk_wp);
    }

    else if (strnicmp(line, "format", 6) == 0)
    {
		char retval = SettingsAction(ACTION_UNMOUNT|ACTION_FORMAT|ACTION_MOUNT);
		if( retval == (ACTION_UNMOUNT|ACTION_FORMAT|ACTION_MOUNT))
			ptr+=sprintf(ptr,"Formatted\r\n");
		else
			ptr+=sprintf(ptr,"Format failed: %02X\r\n",retval);

	}

    else if (strnicmp(line, "save", 4) == 0)
    {
		SettingsAction(ACTION_SAVE);
		ptr+=sprintf(ptr,"Saved file\r\n");
	}

    else if (strnicmp(line, "reset", 5) == 0)
    {
		unsigned short timeout;
		ptr+=sprintf(ptr,"Resetting\r\n");
		// SD card off
		SettingsAction(ACTION_SAVE);
		MDD_ShutdownMedia(); 
		// Respond, flush and wait on cdc
		AddDataElement(TYPE_TEXT_ELEMENT,source, (ptr-resp), respFlags , resp); 
		ReadDataElements(respFlags);
		for(timeout=1000;timeout>0;timeout--)
			{ReadDataElements(respFlags);USBCDCWait();DelayMs(1);}
		// Reset device
		Reset();
	}

    else if (strnicmp(line, "exit", 4) == 0)
    {
		if((respFlags == TXT_TEL_FLAG))
			TelnetDisconnect(1ul<<(source));
		else
			ptr+=sprintf(ptr,"Cannot exit this terminal.\r\n");
	}

    else if (strnicmp(line, "test", 4) == 0)
    {
		ptr+=sprintf(ptr,"Test action...\r\n");
		ptr+=sprintf(ptr,"BaxRx.c: Si44 check failed. %u, %u\r\n",RPINR31bits.SCK4R,RPINR31bits.SDI4R);
	}

    else if (line[0] == '\0')
    {
        ptr+=sprintf(ptr,"\r\n");
    }
    else
    {
        ptr+=sprintf(ptr,"Unknown command: \"%s\"\r\n",line);
    }

	// Send response to command source
	AddDataElement(TYPE_TEXT_ELEMENT,source, (ptr-resp), respFlags , resp); 

    return 1;
}
}

// All commands that modify settings
char SettingsCommandSetting(const char *line, unsigned char mode, unsigned char source)
{
	unsigned char changed = FALSE;
	char resp[128], *ptr; // Briefly on the stack
	unsigned short respFlags;
	switch (mode) {
	case SETTINGS_BATCH	: 	respFlags = 0;break; 
	case SETTINGS_CDC	:	respFlags = TXT_CDC_FLAG;break;
	case SETTINGS_TELNET:	respFlags = TXT_TEL_FLAG;break;
	case SETTINGS_UDP 	:	respFlags = TXT_UDP_FLAG;break;
	default 			: 	respFlags = 0;break; 
	};

	// Begin response
	ptr = resp;

    if (line == NULL || strlen(line) == 0) { return 0; }
	if (strnicmp(line, "setting.", 8) != 0){ return 0; }
	else line+=8; // Skip past the 'setting.' part

    if (strnicmp(line, "id", 2) == 0) // i.e. 'setting.id=123456789\r\n' (32bits) 
    {
        if (line[6] != '\0')
	    { 
		    settings.deviceId = (unsigned int)my_atoi(line + 6); 
			changed = TRUE;
	    }    
        ptr+=sprintf(ptr,"Id: %lu\r\n", settings.deviceId);
    }

    else if (strnicmp(line, "name", 4) == 0)
    {
		if(line[4] != '\0') // field detected
		{
			unsigned char i = sizeof(settings.device_name)-1;
			char *dest = settings.device_name, *ptr = (char*)&line[4];
			while(*ptr<'!' || *ptr==':' || *ptr=='=') ptr++; // Skip spaces etc.
			for(;i>0;i++)
			{
				*dest++ = *ptr++;
				if(*ptr == '\0' || i > 64) break;	// SMF: device_name is allocated a 64 char buffer. Prevent overrun
			}
			*dest = '\0'; // Null terminate
			changed = TRUE;
		}
        ptr+=sprintf(ptr,"Name: %s\r\n", settings.device_name);
    }

    else if (strnicmp(line, "session", 7) == 0)
    {
        if (line[7] != '\0')
        {
            settings.session_number = my_atoi(line + 7); 
			changed = TRUE;
        }
        ptr+=sprintf(ptr,"Session: %u\r\n", settings.session_number);
    }

    else if (strnicmp(line, "usb.", 4) == 0)
    {
		line+=4;
		if (strnicmp(line, "stream", 6) == 0)
		{
			if(line[6]!= '\0')
			settings.usb_stream = my_atoi(line + 6); 
			changed = TRUE;
		}
		else if (strnicmp(line, "mode", 4) == 0)
		{
			if(line[4]!= '\0')
			settings.usb_stream_mode = my_atoi(line + 4); 
			changed = TRUE;
		}
		// USB/CDC port
		ptr+=sprintf(ptr,"\r\nUSB:\r\nMode: %u",settings.usb_stream_mode);
		ptr+=sprintf(ptr,"\r\nSetting: ");
		switch(settings.usb_stream){
			case (STATE_ON):	ptr+=sprintf(ptr,"Turned on");break;
			default:			ptr+=sprintf(ptr,"Turned off");break;
		};
		ptr+=sprintf(ptr,"\r\n");	
    }

    else if (strnicmp(line, "telnet.", 5) == 0)
    {
		line+=7;
		if (strnicmp(line, "stream", 6) == 0)
		{
			if(line[6]!= '\0')
			settings.telnet_stream = my_atoi(line + 6); 
			changed = TRUE;
		}
		else if (strnicmp(line, "mode", 4) == 0)
		{
			if(line[4]!= '\0')
			settings.telnet_stream_mode = my_atoi(line + 4); 
			changed = TRUE;
		}
		else if (strnicmp(line, "name", 4) == 0)
		{
			unsigned char i = sizeof(settings.telnet_username)-1;
			char *dest = settings.telnet_username, *ptr = (char*)&line[4];
			while(*ptr<'!' || *ptr==':' || *ptr=='=')ptr++; // Skip spaces etc.
			for(;i>0;i++)
			{
				*dest++ = *ptr++;
				if(*ptr == '\0') break;	
			}
			*dest = '\0'; // Null terminate
			changed = TRUE;
		}
		else if (strnicmp(line, "password", 8) == 0)
		{
			unsigned char i = sizeof(settings.telnet_password)-1;
			char *dest = settings.telnet_password, *ptr = (char*)&line[8];
			while(*ptr<'!' || *ptr==':' || *ptr=='=')ptr++; // Skip spaces etc.
			for(;i>0;i++)
			{
				*dest++ = *ptr++;
				if(*ptr == '\0') break;	
			}
			*dest = '\0'; // Null terminate
			changed = TRUE;
		}

		ptr+=sprintf(ptr,"\r\nTelnet name:%s\r\nPassword:%s\r\nMode:%u",settings.telnet_username,settings.telnet_password,settings.telnet_stream_mode);
		ptr+=sprintf(ptr,"\r\nSetting: ");
		switch(settings.telnet_stream){
			case (STATE_ON):	ptr+=sprintf(ptr,"Turned on");break;
			default:			ptr+=sprintf(ptr,"Turned off");break;
		};
		ptr+=sprintf(ptr,"\r\n");	
    }

    else if (strnicmp(line, "file.", 5) == 0)
    {
		line+=5;
		if (strnicmp(line, "stream", 6) == 0)
		{
			if(line[6]!= '\0')
			settings.file_stream = my_atoi(line + 6); 
			changed = TRUE;
		}
		else if (strnicmp(line, "mode", 4) == 0)
		{
			if(line[4]!= '\0')
			settings.file_stream_mode = my_atoi(line + 4); 
			changed = TRUE;
		}

		// Sd card
		ptr+=sprintf(ptr,"\r\nFile: \r\nMode: %u",settings.file_stream_mode);
		ptr+=sprintf(ptr,"\r\nSetting: ");
		switch(settings.file_stream){
			case (STATE_ON):	ptr+=sprintf(ptr,"Turned on");
								if(mode != SETTINGS_BATCH)
									SettingsAction(ACTION_START_FILE);	// Start logging
								break;
			default:			ptr+=sprintf(ptr,"Turned off");break;
		};	
		ptr+=sprintf(ptr,"\r\n");	
    }
    else if (strnicmp(line, "udp.", 4) == 0)
    {
		line+=4;
		if (strnicmp(line, "stream", 6) == 0)
		{
			if(line[6]!= '\0')
			settings.udp_stream = my_atoi(line + 6); 
			changed = TRUE;
		}
		else if (strnicmp(line, "mode", 4) == 0)
		{
			if(line[4]!= '\0')
			settings.udp_stream_mode = my_atoi(line + 4); 
			changed = TRUE;
		}
		ptr+=sprintf(ptr,"\r\nUdp:\r\nMode: %u",settings.udp_stream_mode);
		ptr+=sprintf(ptr,"\r\nSetting: ");
		switch(settings.udp_stream){
			case (STATE_ON):	ptr+=sprintf(ptr,"Turned on");break;
			default:			ptr+=sprintf(ptr,"Turned off");break;
		};
		ptr+=sprintf(ptr,"\r\n");	
    }
    else if (strnicmp(line, "gsm.", 4) == 0)
    {
		line+=4;
		if (strnicmp(line, "stream", 6) == 0)
		{
			if(line[6]!= '\0')
			settings.gsm_stream = my_atoi(line + 6); 
			changed = TRUE;
		}
		else if (strnicmp(line, "mode", 4) == 0)
		{
			if(line[4]!= '\0')
			settings.gsm_stream_mode = my_atoi(line + 4); 
			changed = TRUE;
		}

		// GSM
		ptr+=sprintf(ptr,"\r\nGSM:\r\nMode: %u",settings.gsm_stream_mode);
		ptr+=sprintf(ptr,"\r\nSetting: ");
		switch(settings.gsm_stream){
			case (STATE_ON):	ptr+=sprintf(ptr,"Turned on");break;
			default:			ptr+=sprintf(ptr,"Turned off");break;
		};	
		ptr+=sprintf(ptr,"\r\n");	
	
    }

    else if (mode != SETTINGS_BATCH && strnicmp(line, "save", 4) == 0)
    {
		if(settings.settings_source != 1)SettingsSave(); 	// Save to eeprom
		ptr+=sprintf(ptr,"Saved settings\r\n");
	}

    else
    {
        ptr+=sprintf(ptr,"Unknown setting: \"%s\"\r\nTry for example: 'setting.usb.mode=0\\r'\r\n",line);
   	}
	
	if(changed)
	{
		if (mode != SETTINGS_BATCH)	settings.settings_source = 3;/*Set externally*/
		else 						settings.settings_source = 2;/*Batch from file*/
	}

	// Send response to command source
	AddDataElement(TYPE_TEXT_ELEMENT,source, (ptr-resp), respFlags , resp); 
	
	return changed;
}
