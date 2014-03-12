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

// Configuration file and hardware profile selection
// Karim Ladha, 2013-2014

#ifndef _HARDWARE_PROFILE_H_
#define _HARDWARE_PROFILE_H_

// Hardware profile selection
#include "HardwareProfile-BAX2.0.h"
//#include "HardwareProfile-BAX1.0.h"

// Protoptypes
void 	DelayMs(unsigned short ms);
void 	Delay100us(void);
char* 	Byte2Hex(unsigned char val); 

// Debug setting - global! 
//#define DEBUG_ON
//#define SW_LED_DEBUG		/*Use to set led debug flash green = detect, red = not detect*/

// Settings
// Encryption
#define ENCRYPT						/* Turn on encryption*/
#define AES_KEY_PKT_TYPE	0		/* Packet type for encryption packets*/
#define AES_KEY_SIZE 		16		/* For AES128 this is 16 (bytes)*/
#define AES_BLOCK_SIZE		AES_KEY_SIZE
// Setting uart line input
#define MAX_LINE_LEN		64 		/* Line buffer for settings input*/
// Address on eeprom
#define SETTINGS_ADDRESS	0		/* Offset in eeprom*/
// WDT timer values
#define WDT_SETTING			32		/* x4mS, 32-> 128ms*/
#define WDT_PERIOD_MS 		(WDT_SETTING*4ul)
#define WDT_COUNT_5S		((5000ul+WDT_PERIOD_MS)/WDT_PERIOD_MS)
#define WDT_COUNT_1S		((1000ul+WDT_PERIOD_MS)/WDT_PERIOD_MS)

// Default settings / factor resets
// Battery voltage limit to stop transmitting
#define VBATT_MIN			2200UL	/*2.2v*/
// PIR settings
//#define PIR_OFF										/*Use to skip the pir state in the state machine*/
#define PIR_TASKS_PRESCALER				1			/*Use if WDT period < pir tasks period (divides by)*/
#define PIR_THRESHOLD 					20			/*Static threshold*/
#define PIR_THRESHOLD_SUSPECT_VAL 		7			/*Timebased, more sensitive threshold*/
#define PIR_THRESHOLD_SUSPECT_COUNT		4			/*Max number of suspicious detections to trigger*/
#define PIR_THRESHOLD_SUSPECT_RELEASE	8			/*Persist time of suspicious events (in samples)*/
#define PIR_DISARM_TIME		(1*WDT_COUNT_1S)		/*Max time to disarm the sensor for in the presense of activity (add led + settle time)*/	
#define PIR_LED_TIME 		(1*WDT_COUNT_1S)		/*LED on time*/
#define SW_LED_TIME			(1*WDT_COUNT_1S)		/*As above but for switch events*/
#define PIR_SETTLE_TIME		(1*WDT_COUNT_1S)		/*Wait time from LED off to sampling PIR again (LED affects result)*/
// Sample rate settings
#define CREDIT_INTERVAL		(1000ul/WDT_PERIOD_MS)	/*Credit interval*/	
#define MAX_SWITCH_CREDITS	10						/*Max allowed PIR credit limit*/
#define MAX_PIR_CREDITS		10						/*Max allowed PIR credit limit*/
#define SAMPLE_INTERVAL 	(30000ul/WDT_PERIOD_MS) /*Interval between sensor samples (temp, humidity and light)*/
#define SAMPLE_RETRANSMITS	1						/*Number of times we transmit the sensor data*/
// Radio settings
#define RADIO_TX_PWR_DBM	20						/*Added to packet and used to setup radio*/
#define PACKET_TYPE 		1						/*Added to packet*/ 
#define HEADER_BYTE_MSB		'B'						/*Added to packet header[3]*/ 
#endif

