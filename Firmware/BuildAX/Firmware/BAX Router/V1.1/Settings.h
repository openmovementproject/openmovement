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

// Setting and status veriables
// Karim Ladha, 2013-2014

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "HardwareProfile.h"
#include "TCPIPConfig.h"
#include "Peripherals/Rtc.h"
#ifdef USE_FAT_FS
#include "FatFs/FatFsIo.h"
#else
#include "MDD File System/FSIO.h"
#endif

// Types
// Generic state type
typedef enum {
	NOT_PRESENT = -2,
	ERROR = -1,
	OFF = 0,		
	STATE_OFF = 0,	/*Settings - OFF*/
	PRESENT = 1,
	STATE_ON = 1,	/*Settings - ON*/
	INITIALISED = 2,
	ACTIVE = 3
} state_t;

typedef struct
{
	// General settings
	unsigned long deviceId;			// 32 bit identifier
	char device_name[64];			// user friendly name
	unsigned char settings_source; 	// source of current settings 0 = default, 1 = eeprom, 2 = file, 3 = remote
	DateTime start_time;			// start time

	// Radio
	unsigned short radioSubnet;
	unsigned short radioSubnetMask;
	
	// Usb stream settings
	state_t usb_stream;				// usb cdc stream settings
	unsigned char usb_stream_mode;	// mode of usb cdc stream

	// File stream settings
	state_t file_stream;			// file stream settings
	unsigned char file_stream_mode;	// file stream data mode
	unsigned short session_number;	// session number of current (inited from eeprom)
	long save_interval;				// interval to save log file
	long max_file_time;				// second wise max file duration
	long max_file_size;				// byte wise max file size
	char output_bin_file[20];		// file name of binary log file
	char output_text_file[20];		// file name of text log file

	// udp modes
	state_t udp_stream;
	unsigned char udp_stream_mode;

	// gsm settings
	state_t gsm_stream;
	unsigned char gsm_stream_mode;
	
	// Telnet
	state_t telnet_stream;
	unsigned char telnet_stream_mode;
	char telnet_username[TELNET_PAS_NAME_SIZE+1];
	char telnet_password[TELNET_PAS_NAME_SIZE+1];

	// MAC address - read from eeprom on startup
	unsigned char mac[7];

	// Checksum
	unsigned short checksum;
} Settings_t;

typedef struct
{
	signed short sysTemp_x10C;		// system temperature
	unsigned short sysVolts_mV;		// system voltage on power connector
	unsigned short vddVolts_mV;		// Vdd level post regulator
	unsigned char usb_disk_mounted;	// sd card usb msd mounted state
	unsigned char usb_disk_wp;		// sd card usb msd write protect state
	state_t file_state;				// file stream status
	unsigned short session_number;	// session number of current file
	DateTime session_start_time;	// start time of current session
	FSFILE* output_text_file;		// output file pointer text
	FSFILE* output_bin_file;		// output file pointer bin
	FSFILE* error_file;				// desination for error log
	state_t usb_state;				// usb cdc stream status
	state_t ethernet_state;			// ethernet stream status
	state_t gsm_state;				// GSM radio stream staus
	state_t radio_state;			// 433 MHz radio status
	state_t mrf_state;				// 2.4 GHz radio status 
	state_t udp_state;				// Status of udp connection
	unsigned short udp_mask;		// Mask of open udp listeners
	state_t telnet_state;			// Telnet conections
	unsigned short telnet_mask;		// Mask of open telnet connections	
} Status_t;

// Globals
extern Settings_t settings;
extern Status_t status;

// Prototypes
void SettingsToDefault(void);
void SettingsInitialize(void);
void SettingsSave(void);
char SettingsReadFile(const char *filename);
char SettingsAction(char flags);
char SettingsCommand(const char *line, unsigned char mode, unsigned char source);
char SettingsCommandSetting(const char *line, unsigned char mode, unsigned char source);

// Definitions
// For commands- specify stream source / mode so the response mode is correct
#define SETTINGS_BATCH		0
#define SETTINGS_CDC		1
#define SETTINGS_TELNET		2
#define SETTINGS_UDP		3

// For controlling the sd card files / drive
#define ACTION_FORMAT		0x01
#define ACTION_DELETE_LOGS  0x02
#define ACTION_DELETE_SET   0x04
#define ACTION_SAVE         0x08
#define ACTION_MOUNT      	0x10
#define ACTION_UNMOUNT      0x20
#define ACTION_START_FILE	0x40
#define ACTION_SETTINGS_WIPE 0x80

#endif


