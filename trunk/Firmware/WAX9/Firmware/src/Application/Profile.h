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

// GATT profile for LE stack
// Karim Ladha, 2013-2014

#ifndef _GATT_PROFILE_H_
#define _GATT_PROFILE_H_

//#include "HardwareProfile.h"
#include "ble att.h"	// Definitiond and handle types
#include "bt app.h"		// For outgoing serial output handle
#include "Settings.h"	// For mapped variables, commands
#include "SamplerStreamer.h" // For current sample value

// Serial io handles, must match the profile below
#define ATT_SERIAL_IN_HANDLE	0x0022 	// Used by BT App, write only
#define ATT_SERIAL_OUT_HANDLE	0x0025	// Used by BT App, notify/indicate only

// Readable handles to match the profile below
#define ATT_NAME_HANDLE			0x0003	// Used by profile.c 
#define ATT_BATT_HANDLE			0x0032	// Used by profile.c 
#define ATT_COMMAND_HANDLE		0x0041	// Used by profile.c 
#define ATT_DATA_OUT_HANDLE		0x0051	// Used by profile.c 
#define ATT_META_OUT_HANDLE		0x0071	// Used by profile.c 

// uuid helper for readability
#define TWELVE_BYTE_NRF_UUID		0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,0xe0,0x93,0xf3,0xa3,0xb5
#define TWELVE_BYTE_UUID			0x99,0x4d,0x36,0x90,0x8c,0xf4,0x11,0xe3,0xba,0xa8,0x08,0x00

// Globals
extern attOutHandle_t dataOutHandle; 	// User may set this to send data using a notification
extern attOutHandle_t metaDataOut; 		// User may set this to send data using a notification

// Non-user prototypes - called by BTApp
void UserProfileInit(void);
void UserAttributeWrite(const unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset);
void UserAttributeRead(unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset);
unsigned short UserAttributeReadLen(unsigned short handle);
unsigned short UserSendNotifications(void);
unsigned short UserSendIndications(void);

// Definition to prevent multiple copies of the variable below
#ifdef ATT_CREATE_PROFILE

// Set ram or rom profile location
//#define attArrayType static __attribute__((aligned(2)))unsigned char 	/*Ram*/
#define attArrayType const __attribute__((aligned(2)))unsigned char		/*Rom*/

typedef const struct {
	attArrayType att[22]; // Max size is handle+flags+uuid128+len = 22
	__attribute__((packed)) const void* valPtr; // Don't let compiler pack it
} mappedAttribute_t;

/* Rules: Attribute handles must be sequencial, missing handles are ok, specify order in att_profile[], uuids are LE so backwards (Char vals go: <8bits flags (for the value)>, <16bits handle>, <uuid>)
							 					Handle16, 	flags16, 	type(uuid),	length16, 	valueN*/
// GAP service instance
attArrayType gap[] = 						{	0x01,0x00,	_READABLE,	_PRI_SER_,	0x02,0x00,	_GAP_SER_};
attArrayType gap_name_char[] =				{	0x02,0x00,	_READABLE,	_CHARACT_,	0x05,0x00,	0x02,0x03,0x00,		_DEV_NAME};
attArrayType gap_name[] = 					{	0x03,0x00,	0x02,0x04,	_DEV_NAME,	0x00,0x00}; // Callback read
attArrayType gap_appearance_char[] =		{	0x04,0x00,	_READABLE,	_CHARACT_,	0x05,0x00,	0x02,0x05,0x00,		_APPEARAN};
attArrayType gap_appearance[] =				{	0x05,0x00,	_READABLE,	_APPEARAN,	0x02,0x00,	0x00,0x00}; 

// GATT service to inform remote host of changes
attArrayType gatt[]	=						{	0x10,0x00,	_READABLE,	_PRI_SER_,	0x02,0x00,	_GAT_SER_};
attArrayType gatt_changed_char[] =			{	0x11,0x00,	_READABLE,	_CHARACT_,	0x05,0x00,	0x26,0x12,0x00,		_SER_CHA_};
attArrayType gatt_changed[]	=				{	0x12,0x00,	0x26,0x00,	_SER_CHA_,	0x04,0x00,	0x20,0x00,0xff,0xff}; // All user handles have changed! 20 - 65535
attArrayType HW_rev_char[]	=				{	0x13,0x00,	_READABLE,	_CHARACT_,	0x05,0x00,	0x02,0x14,0x00,		_HARDWAR_};
mappedAttribute_t  HW_rev_val	= {			{	0x14,0x00,	_READ_DYN,	_HARDWAR_,	_STRINGT_},	HARDWARE_VERSION};
attArrayType FW_rev_char[]	=				{	0x15,0x00,	_READABLE,	_CHARACT_,	0x05,0x00,	0x02,0x16,0x00,		_FIRMWAR_};
mappedAttribute_t FW_rev_val	= {			{	0x16,0x00,	_READ_DYN,	_FIRMWAR_,	_STRINGT_},	FIRMWARE_VERSION};

// First user service - serial port service
#ifdef ATT_USE_BLE_SERIAL
attArrayType SerialService[] = 				{	0x20,0x00,	_READABLE,	_PRI_SER_,	0x10,0x00,	TWELVE_BYTE_NRF_UUID,0x01,0x00,0x40,0x6e}; // Service uuid
// Serial input - write with no response only
attArrayType serial_in_char[] = 			{	0x21,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x04,0x22,0x00,		TWELVE_BYTE_NRF_UUID,0x02,0x00,0x40,0x6e}; 
attArrayType serial_in_val[] = 				{	0x22,0x00,	0x04,0x84, 	TWELVE_BYTE_NRF_UUID,0x02,0x00,0x40,0x6e,   0x20,0x00}; 
mappedAttribute_t serial_in_desc = {		{	0x23,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Serial input"};
// Serial output - notify or indicate only
attArrayType serial_out_char[] = 			{	0x24,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x30,0x25,0x00,  	TWELVE_BYTE_NRF_UUID,0x03,0x00,0x40,0x6e}; 
attArrayType serial_out_val[] = 			{	0x25,0x00,	0x30,0x84, 	TWELVE_BYTE_NRF_UUID,0x03,0x00,0x40,0x6e,   0x20,0x00}; 
mappedAttribute_t serial_out_cfg = {		{	0x26,0x00,	_RW_DYNA_,	_CLI_CHAR,	_INT16_T_}, &bleSerialOut.attCfg};	
mappedAttribute_t serial_out_desc ={		{	0x27,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Serial output"};
#endif

// User service - sensor data + command
attArrayType SensorCmdService[] = 			{	0x30,0x00,	_READABLE,	_PRI_SER_,	0x10,0x00,	TWELVE_BYTE_UUID,0x00,0x00,0x00,0x00}; // Service uuid
// Battery service 
attArrayType batt_char[] =					{	0x31,0x00,	_READABLE,	_CHARACT_,	0x05,0x00,	0x02,0x32,0x00,		_BAT_LEV_};
attArrayType batt_val[] =					{	0x32,0x00,	0x02,0x04,	_BAT_LEV_,	_INT8_T__}; 
// Direct simple command input (no arguments)
attArrayType command_char[] = 				{	0x40,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0C,0x41,0x00,		TWELVE_BYTE_UUID,0x01,0x00,0x00,0x00};
attArrayType command_val[] = 				{	0x41,0x00,	0x0C,0x84,	TWELVE_BYTE_UUID,0x1,0x00,0x00,0x00,		_INT16_T_};
mappedAttribute_t command_desc = {			{	0x42,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Command: SAVE=0|STREAM=1|RESET=2|LED=3|CLEAR=4|STOP=5|PAUSE=6|PLAY=7"};
// Data output - read, notify or indicate only
attArrayType stream_out_char[] = 			{	0x50,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x32,0x51,0x00,		TWELVE_BYTE_UUID,0x02,0x00,0x00,0x00}; 
mappedAttribute_t stream_out_val = { 		{	0x51,0x00,	0x32,0x82, 	TWELVE_BYTE_UUID,0x02,0x00,0x00,0x00,   (sizeof(sampleData_t)-4),0x00}, &currentSample}; 
mappedAttribute_t stream_out_cfg = {		{	0x52,0x00,	_RW_DYNA_,	_CLI_CHAR,	_INT16_T_}, &dataOutHandle.attCfg};
mappedAttribute_t stream_out_desc = {		{	0x53,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Data output"};
// Meta data output - read and notify or indicate
attArrayType meta_char[] = 					{	0x70,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x32,0x71,0x00,		TWELVE_BYTE_UUID,0x04,0x00,0x00,0x00};
mappedAttribute_t meta_val = {				{	0x71,0x00,	0x32,0x82,	TWELVE_BYTE_UUID,0x04,0x00,0x00,0x00,	0x08,0x00}, &status.pressure};
mappedAttribute_t meta_cfg = {				{	0x72,0x00,	_RW_DYNA_,	_CLI_CHAR,	_INT16_T_}, &metaDataOut.attCfg};	
mappedAttribute_t meta_desc = {				{	0x73,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Pressure Pa (uint32), Temp x10C (sint16), Battery mV (uint16)"};


// Setting characteristics - directly mapped
// User service - settings
attArrayType SensorSettingsService[] = 		{	0x80,0x00,	_READABLE,	_PRI_SER_,	0x10,0x00,	TWELVE_BYTE_UUID,0x05,0x00,0x00,0x00}; // Service uuid
// Streaming on/off
attArrayType streaming_char[] = 			{	0x81,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x02,0x82,0x00,		TWELVE_BYTE_UUID,0x06,0x00,0x00,0x00};
mappedAttribute_t streaming_val = {			{	0x82,0x00,	0x02,0x82,	TWELVE_BYTE_UUID,0x06,0x00,0x00,0x00,	_INT8_T__}, .valPtr = &status.streaming};
mappedAttribute_t streaming_desc = {		{	0x83,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Streaming on/off"};
// LED
attArrayType ledSet_char[] = 				{	0x90,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x91,0x00,		TWELVE_BYTE_UUID,0x07,0x00,0x00,0x00};
mappedAttribute_t ledSet_val = {			{	0x91,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x07,0x00,0x00,0x00,	_INT8_T__}, .valPtr = &status.ledOverride};
mappedAttribute_t ledSet_desc = {			{	0x92,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"LED setting"};
// Sleepmode
attArrayType sleepMode_char[] = 			{	0xa0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xa1,0x00,		TWELVE_BYTE_UUID,0x08,0x00,0x00,0x00};
mappedAttribute_t sleepMode_val = {			{	0xa1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x08,0x00,0x00,0x00,	_INT8_T__}, .valPtr = &settings.sleepMode};
mappedAttribute_t sleepMode_desc = {		{	0xa2,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Sleep mode setting"};
// Inactivity timer
attArrayType inact_char[] = 				{	0xb0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xb1,0x00,		TWELVE_BYTE_UUID,0x09,0x00,0x00,0x00};
mappedAttribute_t inact_val = {				{	0xb1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x09,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.inactivityThreshold};
mappedAttribute_t inact_desc = {			{	0xb2,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Inactivity timeout"};
// Sample rate
attArrayType sampRate_char[] = 				{	0xc0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xc1,0x00,		TWELVE_BYTE_UUID,0x0a,0x00,0x00,0x00};
mappedAttribute_t sampRate_val = {			{	0xc1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0a,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.sampleRate};
mappedAttribute_t sampRate_desc = {			{	0xc2,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Sample rate Hz"};
// Accelerometer - on
attArrayType accelOn_char[] = 				{	0xd0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xd1,0x00,		TWELVE_BYTE_UUID,0x0b,0x00,0x00,0x00};
mappedAttribute_t accelOn_val = {			{	0xd1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0b,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.accelOn};
mappedAttribute_t accelOn_desc = {			{	0xd2,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Accel. on/off"};
// Accelerometer - rate
attArrayType accelRate_char[] = 			{	0xe0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xe1,0x00,		TWELVE_BYTE_UUID,0x0c,0x00,0x00,0x00};
mappedAttribute_t accelRate_val = {			{	0xe1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0c,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.accelRate};
mappedAttribute_t accelRate_desc = {		{	0xe2,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Accel. rate Hz"};
// Accelerometer - range
attArrayType accelRange_char[] = 			{	0xf0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xf1,0x00,		TWELVE_BYTE_UUID,0x0d,0x00,0x00,0x00};
mappedAttribute_t accelRange_val = {		{	0xf1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0d,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.accelRange};
mappedAttribute_t accelRange_desc = {		{	0xf2,0x00,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Accel. range g"};
// Gyroscope - on
attArrayType gyroOn_char[] = 				{	0x00,0x01,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x01,0x01,		TWELVE_BYTE_UUID,0x0e,0x00,0x00,0x00};
mappedAttribute_t gyroOn_val = {			{	0x01,0x01,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0e,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.gyroOn};
mappedAttribute_t gyroOn_desc = {			{	0x02,0x01,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Gyro. on/off"};
// Gyroscope - rate
attArrayType gyroRate_char[] = 				{	0x10,0x01,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x11,0x01,		TWELVE_BYTE_UUID,0x0f,0x00,0x00,0x00};
mappedAttribute_t gyroRate_val = {			{	0x11,0x01,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0f,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.gyroRate};
mappedAttribute_t gyroRate_desc = {			{	0x12,0x01,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Gyro. rate Hz"};
// Gyroscope - range
attArrayType gyroRange_char[] = 			{	0x20,0x01,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x21,0x01,		TWELVE_BYTE_UUID,0x10,0x00,0x00,0x00};
mappedAttribute_t gyroRange_val = {			{	0x21,0x01,	0x0E,0x82,	TWELVE_BYTE_UUID,0x10,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.gyroRange};
mappedAttribute_t gyroRange_desc = {		{	0x22,0x01,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Gyro. range dps"};
// Magnetometer - on
attArrayType magOn_char[] = 				{	0x30,0x01,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x31,0x01,		TWELVE_BYTE_UUID,0x11,0x00,0x00,0x00};
mappedAttribute_t magOn_val = {				{	0x31,0x01,	0x0E,0x82,	TWELVE_BYTE_UUID,0x11,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.magOn};
mappedAttribute_t magOn_desc = {			{	0x32,0x01,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Mag. on/off"};
// Magnetometer - rate
attArrayType magRate_char[] = 				{	0x40,0x01,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x41,0x01,		TWELVE_BYTE_UUID,0x12,0x00,0x00,0x00};
mappedAttribute_t magRate_val = {			{	0x41,0x01,	0x0E,0x82,	TWELVE_BYTE_UUID,0x12,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.magRate};
mappedAttribute_t magRate_desc = {			{	0x42,0x01,	_READ_DYN,	_USER_DES,	_STRINGT_},	"Mag. rate Hz"};

/* Global constant of attribute pointers
   Rules: First and last ptrs must be null.
   Handles in service ranges must be in numeric order.
*/
const __attribute__((space(psv)))void* att_profile[] = 
{
	NULL,
	// GAP service attributes
	gap,							// Mandatory, one instance only
	gap_name_char,					// Mandatory, device name
	gap_name,						// Value place holder
	gap_appearance_char,			// Mandatory, device physical appearance
	gap_appearance,					// Value place holder
	
	// GATT service to inform remote host of changes
	gatt,							// Primary service type
	gatt_changed_char,				// Optional service to inform remote device of changes (mandatory if changes are expected)
	gatt_changed,					// Value place holder
	HW_rev_char,					// Hardware revision characteristic
	&HW_rev_val,					// Hardware revision string
	FW_rev_char,					// Firmware revision characteristic
	&FW_rev_val,					// Firmware revision string

#ifdef ATT_USE_BLE_SERIAL	
	// First user service - serial port service
	SerialService,
	serial_in_char,					// Ascii string input for commands (write no response)
	serial_in_val,					// Value location - mapped in BTApp
	&serial_in_desc,				// Value user description
	serial_out_char,				// Ascii response out (indicate/notify)
	serial_out_val,					// Value location - mapped in BTApp
	&serial_out_cfg,				// Notify/Indicate config
	&serial_out_desc,				// Value user description
#endif

	// User service - sensor data + command
	SensorCmdService,				// Primary service type
	// Battery level service
	batt_char,						// Battery
	batt_val,						// Standard service in %
	// Numerical command input
	command_char,					// Numerical command input characteristic
	command_val,					// Numerical command input value					
	&command_desc,					// Numerical command input description
	// Streaming data output
	stream_out_char,				// Binary stream out
	&stream_out_val,				// Value place holder
	&stream_out_cfg,				// Notify/Indicate config
	&stream_out_desc,				// Value user description
	// Meta data output
	meta_char,						// Other sensor data
	&meta_val,						// Value
	&meta_cfg,						// Configuration
	&meta_desc,						// Description

	// User service - settings
	SensorSettingsService,
	// Streaming setting
	streaming_char,					// Characteristic				
	&streaming_val,					// Value
	&streaming_desc,				// Description
	// Led override setting	
	ledSet_char,					// Characteristic					
	&ledSet_val,					// Value
	&ledSet_desc,					// Description
	// Sleepmode configuration	
	sleepMode_char,					// Characteristic				
	&sleepMode_val,					// Value
	&sleepMode_desc,				// Description
	// Inactivity timeout	
	inact_char,						// Characteristic					
	&inact_val,						// Value
	&inact_desc,					// Description
	// Sample rate value	
	sampRate_char,					// Characteristic					
	&sampRate_val,					// Value
	&sampRate_desc,					// Description
	// Accelerometer settings	
	accelOn_char,					// Characteristic					
	&accelOn_val,					// Value
	&accelOn_desc,					// Description
	accelRate_char,					// Characteristic
	&accelRate_val,					// Value
	&accelRate_desc,				// Description
	accelRange_char,					// Characteristic
	&accelRange_val,				// Value
	&accelRange_desc,				// Description
	// Gyroscope settings
	gyroOn_char,					// Characteristic					
	&gyroOn_val,					// Value
	&gyroOn_desc,					// Description
	gyroRate_char,					// Characteristic
	&gyroRate_val,					// Value
	&gyroRate_desc,					// Description
	gyroRange_char,					// Characteristic
	&gyroRange_val,					// Value
	&gyroRange_desc,				// Description
	// Magnetometer settings
	magOn_char,						// Characteristic					
	&magOn_val,						// Value
	&magOn_desc,					// Description
	magRate_char,					// Characteristic
	&magRate_val,					// Value
	&magRate_desc,					// Description			

	// End of list
	NULL							
};

#endif //ATT_CREATE_PROFILE

#endif

