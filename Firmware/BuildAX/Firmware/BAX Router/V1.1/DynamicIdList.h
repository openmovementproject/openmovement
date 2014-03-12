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

// Dynamic variables
// Karim Ladha, Samuel Finnigan, 2013-2014

#ifndef DYN_LIST_H
#define DYN_LIST_H

#include "Settings.h"
#include "DynamicVars.h"

DynEntry_t dynEntries[] =
{
	// Leave index 0 as this - atoi fails to 0
	{0, DYN_TEXT,	"Var id 0, atoi fail?"},				// id zero invalid
	{1, DYN_TEXT,	"This text was dynamically inserted!"},	// const char array insertion
	{2, DYN_TEXT,	"checked"},								// html forms default value (used with conditional)
	{6, DYN_TIME,	NULL},									// read the current time
	
	{11, DYN_TEXT,	&settings.device_name},					// ram strings too, id not sequencial OK
	{12, DYN_TIME,	&settings.start_time},					// read a specific time
	
	// Logging / forwarding settings
	{13, DYN_STATE,	&settings.usb_stream},					// usb cdc stream setting
	{14, DYN_MODE,	&settings.usb_stream_mode},				// mode of usb cdc stream
	
	{15, DYN_STATE,	&settings.file_stream},					// file stream setting
	{16, DYN_MODE,	&settings.file_stream_mode},			// file stream data mode
	{17, DYN_STATE,	&settings.telnet_stream},				// telnet stream setting
	{18, DYN_MODE,	&settings.telnet_stream_mode},			// telnet stream data mode
	
	{19, DYN_STATE, &settings.udp_stream},					// udp stream setting
	{20, DYN_MODE,	&settings.udp_stream_mode},				// udp stream mode
	
	{21, DYN_STATE,	&settings.gsm_stream},					// gsm stream setting
	{22, DYN_MODE,	&settings.gsm_stream_mode},				// gsm stream mode
	
	// Status indicators
	{81, DYN_SINT16, &status.sysTemp_x10C},					// system temperature
	{82, DYN_UINT16, &status.sysVolts_mV},					// system voltage on power connector
	{83, DYN_BOOL,   &status.usb_disk_mounted},				// sd card usb msd mounted state
	{84, DYN_STATE,  &status.usb_state},					// usb cdc stream status
	{85, DYN_STATE,  &status.radio_state},					// 433 MHz radio status
	{86, DYN_STATE,  &status.mrf_state},					// 2.4 GHz radio status 
	{87, DYN_BOOL,   &status.telnet_state},					// Telnet conections
	
	{88, DYN_TEXT,	&settings.output_text_file},			// file name of plaintext log file
	{89, DYN_TEXT,	&settings.output_bin_file},				// file name of binary log file
};

#else
	#error "Included more than once"
#endif
