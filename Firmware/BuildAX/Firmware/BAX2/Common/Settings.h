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

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

// A packet
typedef union {
	struct {
		unsigned char 	pktType;
		unsigned char 	pktId;
		signed char 	xmitPwrdBm;
		unsigned short 	battmv;
		unsigned short	humidSat;
		signed short 	tempCx10;
		unsigned short 	lightLux;
		unsigned short	pirCounts;
		unsigned short	pirEnergy;
		unsigned short	swCountStat;
	};
	unsigned char b[AES_BLOCK_SIZE+1]; // +1 for type (17 bytes)
} packet_t;

// The global settings
typedef struct {
	union {
	unsigned char 	val8[4];
	unsigned long 	val32; 
	}deviceId;
	unsigned char 	key[AES_KEY_SIZE];
	// Other settings
	unsigned short 	batt_min;
	unsigned char	tx_power;
	unsigned char 	pir_threshold;
	unsigned char 	pir_suspect_val;
	unsigned char 	pir_suspect_count;
	unsigned char 	pir_suspect_release;
	unsigned short 	pir_disarm_time;
	unsigned short	pir_led_time;
	unsigned short 	sw_led_time;
	unsigned short 	sensor_settle_time;
	unsigned short 	credit_interval;
	unsigned char 	max_pir_credits;
	unsigned char 	max_switch_credits;
	unsigned short 	sample_interval;
	unsigned char 	num_tx_repeats;
	// Checksum
	unsigned short 	checksum;
} settings_t;

// Globals
extern settings_t settings;

// String constants for settings
typedef struct {
	const char* name; 
	void* address; 
	unsigned char len;
} setting_vals_t;

#ifdef MAKE_SETTINGS_TABLE
const setting_vals_t setting_val[] = {
	{"packet_header",		&settings.deviceId.val8[3],		sizeof(unsigned char)},
	{"device_group",		&settings.deviceId.val8[2],		sizeof(unsigned char)},
	{"device_id",			&settings.deviceId.val8[0],		sizeof(unsigned short)},
	{"batt_min",			&settings.batt_min,				sizeof(unsigned short)},
	{"tx_power",			&settings.tx_power,				sizeof(unsigned char)},
	{"pir_threshold",		&settings.pir_threshold,		sizeof(unsigned char)},
	{"pir_suspect_val",		&settings.pir_suspect_val,		sizeof(unsigned char)},
	{"pir_suspect_count",	&settings.pir_suspect_count,	sizeof(unsigned char)},
	{"pir_suspect_release",	&settings.pir_suspect_release,	sizeof(unsigned char)},
	{"pir_disarm_time",		&settings.pir_disarm_time,		sizeof(unsigned short)},
	{"pir_led_time",		&settings.pir_led_time,			sizeof(unsigned short)},
	{"sw_led_time",			&settings.sw_led_time,			sizeof(unsigned short)},
	{"sensor_settle_time",	&settings.sensor_settle_time,	sizeof(unsigned short)},
	{"credit_interval",		&settings.credit_interval,		sizeof(unsigned short)},
	{"max_pir_credits",		&settings.max_pir_credits,		sizeof(unsigned char)},
	{"max_switch_credits",	&settings.max_switch_credits,	sizeof(unsigned char)},
	{"sensor_interval",		&settings.sample_interval,		sizeof(unsigned short)},
	{"num_tx_repeats",		&settings.num_tx_repeats,		sizeof(unsigned char)},
};
const unsigned short num_of_settings = (sizeof(setting_val)/sizeof(setting_vals_t));
#endif

// Prototypes
unsigned char SetttingsAction(unsigned char mode);
unsigned short SettingsChecksum(void);
void SettingsSave(void);
void SettingCommandMode(void);

// Definitions
// Arguments for SettingAction(...)
#define SETTINGS_LOAD		0x00
#define SETTINGS_CLEAR		0x01
#define SETTINGS_NEW_ID		0x02
#define SETTINGS_NEW_ENCRYP	0x04	
#define SETTINGS_RESET_ALL	0x07


#endif