// Device specific definitions for attibutes, descriptors and service classes
// Instantiation of variables and functions (used by hci.c, sdp.c and att.c)
// Karim Ladha, 2015

// 30-01-2014
// Written to separate att handles from the rest of the App 
// to allow more customised configurations. The serial port
// part of the att service is still in the app.

// Application specific includes
#include "Compiler.h"
#include "hci.h"				// Various definitions
#include "sdp.h"				// SDP profile construction
#include "rfcomm.h"
#include "att.h"				// Attribute profile construction
#include "SamplerStreamer.h"	// Mapped attributes
#include "Settings.h"			// Mapped attributes + commands
#include "Analog.h"				// Mapped attributes
#include "bt-serial.h"			// For BLE serial emulation
#include "btconfig.h"			// Settings
#include "btprofile.h"			// Device attributes
#include "bt-nvm.h"

// DEBUG LEVEL
#define DEBUG_LEVEL_LOCAL	DEBUG_APP
#define DBG_FILE			"profile.c"	
#include "debug.h"

// Device definitions 
// Device data used for DID profile
#define DID_VID		0x04D8	// Microchip Inc
#define DID_PID		0x4158	// 'A','X'
#define DID_VER		0x0910	// 9.1.0
#define DID_VID_SRC	0x0002	// USB IF

// Globals

// Remote device rfcomm/att connection handles
extern RfcommConnection_t rfcommConnection;
extern AttConnection_t attConnection;

// Extended inquiry response DID entry (10 bytes)
const uint8_t eir_did_entry[] = {	
		DID_EIR_LEN, 	/* Length */
		DID_EIR_TAG,	/* DID EIR tag */
		BT_INITLE16(DID_VID_SRC),
		BT_INITLE16(DID_VID),
		BT_INITLE16(DID_PID),
		BT_INITLE16(DID_VER)};

// Prototypes
// BLE Att support 
static void AttDataIn 	(const uint8_t * data, uint16_t len, uint16_t handle, uint16_t offset);
static void AttDataOut (uint8_t * data, uint16_t len, uint16_t handle, uint16_t offset);
static uint16_t AttGetHandleLen (uint16_t handle);

void ProfileInit(void)
{
#ifndef DISABLE_LE_MODES
	// Initialise att layer and profile
	#ifdef ATT_USE_BLE_SERIAL	
	memset(&attSerialOut,0,sizeof(attOutHandle_t));// Zero serial out handle
	#endif
	memset(&dataOutHandle,0,sizeof(attOutHandle_t));// Zero data out handle
	memset(&metaDataOut,0,sizeof(attOutHandle_t));	// Zero meta data out handle
	// Install dynamic attribute callbacks
	AttSetWriteCallback(AttDataIn);  
	AttSetReadCallback (AttDataOut); 	
	AttSetGetExtAttLen(AttGetHandleLen); 		
#endif
}

// Low energy protocol settings		
#ifndef DISABLE_LE_MODES
// Notify/Indicate att handles
#ifdef ATT_USE_BLE_SERIAL	
// Serial output handle (global needed for state)
attOutHandle_t attSerialOut = {0};
#endif
// Output data notification variable for streaming
attOutHandle_t dataOutHandle = {0};
attOutHandle_t	metaDataOut = {0};

// Extention of attribute protocol beyond the attribute profile - dynamic attribute write
void AttDataIn 	(const uint8_t * data, uint16_t len, uint16_t handle, uint16_t offset)
{
	DBG_INFO("attwr ha:%04X,ln:%u,of:%u, dt:",handle,len,offset);
	DBG_DUMP(data, len);
	switch(handle){
		#ifdef ATT_USE_BLE_SERIAL
		case ATT_SERIAL_IN_HANDLE : // Set it profile, writes to this append to the usual serial buffer
			// Process the command in a command handler
			SerialDataReceived((void*)data, len);
			break;
		#endif
		case ATT_COMMAND_HANDLE : {
			uint16_t arg = BT_READLE16(data);
			SettingsCommandNumber(arg);
			break;
		}
		default : 
			break;		
	}
	return;
}

// Called BY the stack to READ DYNAMIC external variables
void AttDataOut (uint8_t * data, uint16_t len, uint16_t handle, uint16_t offset)
{
	DBG_INFO("attrd ha:%04X,ln:%u,os:%u\n",handle,len,offset);
	switch(handle) {
		// Add support for any read dynamic variables here
		case ATT_BATT_HANDLE :
			data[0] = AdcBattToPercent(adcResult.batt);
			break;
		case ATT_NAME_HANDLE : {
			char* ptr = GetBTName();
			uint16_t length = strlen(ptr);
			if((length)<(len+offset))len = length - offset;
			memcpy(data,ptr+offset,len);
			break;
		}
		default : {
			break;
		}
	}	
	return;
}
// Called by the stack to READ HANDLE LENGTHS
uint16_t AttGetHandleLen (uint16_t handle)
{
	uint16_t length = 0xffff;
	switch(handle){
		#ifdef ATT_USE_BLE_SERIAL
		case ATT_SERIAL_IN_HANDLE :
			length = 32; // Max len user can write into fifo at once (write more than once)
			break;
		#endif
		case ATT_NAME_HANDLE : 
			length = strlen(GetBTName());
			break;
		default:
			break;
	}
	return length;
}


uint16_t AttSendPending(void)
{
	// Add support for notifying profile handles here.
	// Check each notifier
	uint16_t retVal = 0;

	// Data out notifier
	if((dataOutHandle.dataFlag == TRUE_) && (dataOutHandle.attCfg & ATT_CFG_NOTIFY))
	{
		uint16_t max_len = AttGetMtu(attConnection) - 3;
		if(dataOutHandle.attLen > max_len) dataOutHandle.attLen = max_len;
		AttNotify (attConnection, dataOutHandle.attData,dataOutHandle.attLen,ATT_DATA_OUT_HANDLE);
		dataOutHandle.dataFlag = FALSE_; // Clear flag
		retVal += dataOutHandle.attLen;
	}

	// Meta data out notifier
	if((metaDataOut.dataFlag == TRUE_) && (metaDataOut.attCfg & ATT_CFG_NOTIFY))
	{
		uint16_t max_len = AttGetMtu(attConnection) - 3;
		if(metaDataOut.attLen > max_len) metaDataOut.attLen = max_len;
		AttNotify (attConnection, metaDataOut.attData,metaDataOut.attLen,ATT_META_OUT_HANDLE);
		metaDataOut.dataFlag = FALSE_; // Clear flag
		retVal += metaDataOut.attLen;
	}

	return retVal;
}

uint16_t UserSendIndications(void)
{
	// Add support for indicating profile handles here.
	// Note: Only one at a time can be sent, put in priority order

	// Data out notifier
	if((dataOutHandle.dataFlag == TRUE_) && (!AttGetIndPending(attConnection))  && (dataOutHandle.attCfg & ATT_CFG_INDICATE))
	{
		// Checks
		uint16_t max_len = AttGetMtu(attConnection) - 3;
		if(dataOutHandle.attLen > max_len) dataOutHandle.attLen = max_len;
		AttIndicate (attConnection, dataOutHandle.attData, dataOutHandle.attLen, ATT_DATA_OUT_HANDLE);
		dataOutHandle.dataFlag = FALSE_; // Clear flag
		return dataOutHandle.attLen;
	}

	// Meta data out notifier
	if((metaDataOut.dataFlag == TRUE_) && (!AttGetIndPending(attConnection))  && (metaDataOut.attCfg & ATT_CFG_INDICATE))
	{
		// Checks
		uint16_t max_len = AttGetMtu(attConnection) - 3;
		if(metaDataOut.attLen > max_len) metaDataOut.attLen = max_len;
		AttIndicate (attConnection,metaDataOut.attData,metaDataOut.attLen,ATT_META_OUT_HANDLE);
		metaDataOut.dataFlag = FALSE_; // Clear flag
		return metaDataOut.attLen;
	}

	return 0;
}

// The attribute profile for the device - set ram or rom profile location
//#define attArrayType static __attribute__((aligned(2)))uint8_t 	/*Ram*/
#define attArrayType const __attribute__((aligned(2)))uint8_t		/*Rom*/
// uuid helper for improved readability of profile definition
#define TWELVE_BYTE_NRF_UUID		0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,0xe0,0x93,0xf3,0xa3,0xb5
#define TWELVE_BYTE_UUID			0x99,0x4d,0x36,0x90,0x8c,0xf4,0x11,0xe3,0xba,0xa8,0x08,0x00
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
mappedAttribute_t gatt_changed_client = {	{	0x13,0x00,	_RW_PTR__,	_CLI_CHAR,	_INT16_T_}, NULL}; // Ignore writes to it
attArrayType HW_rev_char[]	=				{	0x14,0x00,	_READABLE,	_CHARACT_,	0x05,0x00,	0x02,0x15,0x00,		_HARDWAR_};
mappedAttribute_t  HW_rev_val	= {			{	0x15,0x00,	_READ_PTR,	_HARDWAR_,	_STRINGT_},	HARDWARE_VERSION};
attArrayType FW_rev_char[]	=				{	0x16,0x00,	_READABLE,	_CHARACT_,	0x05,0x00,	0x02,0x17,0x00,		_FIRMWAR_};
mappedAttribute_t FW_rev_val	= {			{	0x17,0x00,	_READ_PTR,	_FIRMWAR_,	_STRINGT_},	FIRMWARE_VERSION};

// First user service - serial port service
#ifdef ATT_USE_BLE_SERIAL
attArrayType SerialService[] = 				{	0x20,0x00,	_READABLE,	_PRI_SER_,	0x10,0x00,	TWELVE_BYTE_NRF_UUID,0x01,0x00,0x40,0x6e}; // Service uuid
// Serial input - write with no response only
attArrayType serial_in_char[] = 			{	0x21,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x04,0x22,0x00,		TWELVE_BYTE_NRF_UUID,0x02,0x00,0x40,0x6e}; 
attArrayType serial_in_val[] = 				{	0x22,0x00,	0x04,0x84, 	TWELVE_BYTE_NRF_UUID,0x02,0x00,0x40,0x6e,   0x20,0x00}; 
mappedAttribute_t serial_in_desc = {		{	0x23,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Serial input"};
// Serial output - notify or indicate only
attArrayType serial_out_char[] = 			{	0x24,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x30,0x25,0x00,  	TWELVE_BYTE_NRF_UUID,0x03,0x00,0x40,0x6e}; 
attArrayType serial_out_val[] = 			{	0x25,0x00,	0x30,0x84, 	TWELVE_BYTE_NRF_UUID,0x03,0x00,0x40,0x6e,   0x20,0x00}; 
mappedAttribute_t serial_out_cfg = {		{	0x26,0x00,	_RW_PTR__,	_CLI_CHAR,	_INT16_T_}, &attSerialOut.attCfg};	
mappedAttribute_t serial_out_desc ={		{	0x27,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Serial output"};
#endif

// User service - sensor data + command
attArrayType SensorCmdService[] = 			{	0x30,0x00,	_READABLE,	_PRI_SER_,	0x10,0x00,	TWELVE_BYTE_UUID,0x00,0x00,0x00,0x00}; // Service uuid
// Battery service 
attArrayType batt_char[] =					{	0x31,0x00,	_READABLE,	_CHARACT_,	0x05,0x00,	0x02,0x32,0x00,		_BAT_LEV_};
attArrayType batt_val[] =					{	0x32,0x00,	_R_USER__,	_BAT_LEV_,	_INT8_T__}; 
// Direct simple command input (no arguments)
attArrayType command_char[] = 				{	0x40,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0C,0x41,0x00,		TWELVE_BYTE_UUID,0x01,0x00,0x00,0x00};
attArrayType command_val[] = 				{	0x41,0x00,	0x0C,0x84,	TWELVE_BYTE_UUID,0x1,0x00,0x00,0x00,		_INT16_T_};
mappedAttribute_t command_desc = {			{	0x42,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Command: SAVE=0|STREAM=1|RESET=2|LED=3|CLEAR=4|STOP=5|PAUSE=6|PLAY=7"};
// Data output - read, notify or indicate only
attArrayType stream_out_char[] = 			{	0x50,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x32,0x51,0x00,		TWELVE_BYTE_UUID,0x02,0x00,0x00,0x00}; 
mappedAttribute_t stream_out_val = { 		{	0x51,0x00,	0x32,0x82, 	TWELVE_BYTE_UUID,0x02,0x00,0x00,0x00,   (sizeof(sampleData_t)-4),0x00}, &currentSample}; 
mappedAttribute_t stream_out_cfg = {		{	0x52,0x00,	_RW_PTR__,	_CLI_CHAR,	_INT16_T_}, &dataOutHandle.attCfg};
mappedAttribute_t stream_out_desc = {		{	0x53,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Data output"};
// Meta data output - read and notify or indicate
attArrayType meta_char[] = 					{	0x70,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x32,0x71,0x00,		TWELVE_BYTE_UUID,0x04,0x00,0x00,0x00};
mappedAttribute_t meta_val = {				{	0x71,0x00,	0x32,0x82,	TWELVE_BYTE_UUID,0x04,0x00,0x00,0x00,	0x08,0x00}, &status.pressure};
mappedAttribute_t meta_cfg = {				{	0x72,0x00,	_RW_PTR__,	_CLI_CHAR,	_INT16_T_}, &metaDataOut.attCfg};	
mappedAttribute_t meta_desc = {				{	0x73,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Pressure Pa (uint32), Temp x10C (sint16), Battery mV (uint16)"};

// Setting characteristics - directly mapped
// User service - settings
attArrayType SensorSettingsService[] = 		{	0x80,0x00,	_READABLE,	_PRI_SER_,	0x10,0x00,	TWELVE_BYTE_UUID,0x05,0x00,0x00,0x00}; // Service uuid
// Streaming on/off
attArrayType streaming_char[] = 			{	0x81,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x02,0x82,0x00,		TWELVE_BYTE_UUID,0x06,0x00,0x00,0x00};
mappedAttribute_t streaming_val = {			{	0x82,0x00,	0x02,0x82,	TWELVE_BYTE_UUID,0x06,0x00,0x00,0x00,	_INT8_T__}, .valPtr = &status.streaming};
mappedAttribute_t streaming_desc = {		{	0x83,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Streaming on/off"};
// LED
attArrayType ledSet_char[] = 				{	0x90,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x91,0x00,		TWELVE_BYTE_UUID,0x07,0x00,0x00,0x00};
mappedAttribute_t ledSet_val = {			{	0x91,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x07,0x00,0x00,0x00,	_INT8_T__}, .valPtr = &status.ledOverride};
mappedAttribute_t ledSet_desc = {			{	0x92,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"LED setting"};
// Sleepmode
attArrayType sleepMode_char[] = 			{	0xa0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xa1,0x00,		TWELVE_BYTE_UUID,0x08,0x00,0x00,0x00};
mappedAttribute_t sleepMode_val = {			{	0xa1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x08,0x00,0x00,0x00,	_INT8_T__}, .valPtr = &settings.sleepMode};
mappedAttribute_t sleepMode_desc = {		{	0xa2,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Sleep mode setting"};
// Inactivity timer
attArrayType inact_char[] = 				{	0xb0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xb1,0x00,		TWELVE_BYTE_UUID,0x09,0x00,0x00,0x00};
mappedAttribute_t inact_val = {				{	0xb1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x09,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.inactivityThreshold};
mappedAttribute_t inact_desc = {			{	0xb2,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Inactivity timeout"};
// Sample rate
attArrayType sampRate_char[] = 				{	0xc0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xc1,0x00,		TWELVE_BYTE_UUID,0x0a,0x00,0x00,0x00};
mappedAttribute_t sampRate_val = {			{	0xc1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0a,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.sampleRate};
mappedAttribute_t sampRate_desc = {			{	0xc2,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Sample rate Hz"};
// Accelerometer - on
attArrayType accelOn_char[] = 				{	0xd0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xd1,0x00,		TWELVE_BYTE_UUID,0x0b,0x00,0x00,0x00};
mappedAttribute_t accelOn_val = {			{	0xd1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0b,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.accelOn};
mappedAttribute_t accelOn_desc = {			{	0xd2,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Accel. on/off"};
// Accelerometer - rate
attArrayType accelRate_char[] = 			{	0xe0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xe1,0x00,		TWELVE_BYTE_UUID,0x0c,0x00,0x00,0x00};
mappedAttribute_t accelRate_val = {			{	0xe1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0c,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.accelRate};
mappedAttribute_t accelRate_desc = {		{	0xe2,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Accel. rate Hz"};
// Accelerometer - range
attArrayType accelRange_char[] = 			{	0xf0,0x00,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0xf1,0x00,		TWELVE_BYTE_UUID,0x0d,0x00,0x00,0x00};
mappedAttribute_t accelRange_val = {		{	0xf1,0x00,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0d,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.accelRange};
mappedAttribute_t accelRange_desc = {		{	0xf2,0x00,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Accel. range g"};
// Gyroscope - on
attArrayType gyroOn_char[] = 				{	0x00,0x01,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x01,0x01,		TWELVE_BYTE_UUID,0x0e,0x00,0x00,0x00};
mappedAttribute_t gyroOn_val = {			{	0x01,0x01,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0e,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.gyroOn};
mappedAttribute_t gyroOn_desc = {			{	0x02,0x01,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Gyro. on/off"};
// Gyroscope - rate
attArrayType gyroRate_char[] = 				{	0x10,0x01,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x11,0x01,		TWELVE_BYTE_UUID,0x0f,0x00,0x00,0x00};
mappedAttribute_t gyroRate_val = {			{	0x11,0x01,	0x0E,0x82,	TWELVE_BYTE_UUID,0x0f,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.gyroRate};
mappedAttribute_t gyroRate_desc = {			{	0x12,0x01,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Gyro. rate Hz"};
// Gyroscope - range
attArrayType gyroRange_char[] = 			{	0x20,0x01,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x21,0x01,		TWELVE_BYTE_UUID,0x10,0x00,0x00,0x00};
mappedAttribute_t gyroRange_val = {			{	0x21,0x01,	0x0E,0x82,	TWELVE_BYTE_UUID,0x10,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.gyroRange};
mappedAttribute_t gyroRange_desc = {		{	0x22,0x01,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Gyro. range dps"};
// Magnetometer - on
attArrayType magOn_char[] = 				{	0x30,0x01,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x31,0x01,		TWELVE_BYTE_UUID,0x11,0x00,0x00,0x00};
mappedAttribute_t magOn_val = {				{	0x31,0x01,	0x0E,0x82,	TWELVE_BYTE_UUID,0x11,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.magOn};
mappedAttribute_t magOn_desc = {			{	0x32,0x01,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Mag. on/off"};
// Magnetometer - rate
attArrayType magRate_char[] = 				{	0x40,0x01,	_READABLE,	_CHARACT_,	0x13,0x00,	0x0E,0x41,0x01,		TWELVE_BYTE_UUID,0x12,0x00,0x00,0x00};
mappedAttribute_t magRate_val = {			{	0x41,0x01,	0x0E,0x82,	TWELVE_BYTE_UUID,0x12,0x00,0x00,0x00,	_INT16_T_}, .valPtr = &settings.magRate};
mappedAttribute_t magRate_desc = {			{	0x42,0x01,	_READ_PTR,	_USER_DES,	_STRINGT_},	"Mag. rate Hz"};

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
	&gatt_changed_client,			// Writable client characteristic
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
const uint16_t att_profile_entries = (sizeof(att_profile) / sizeof(void*));
#endif

		
// Service discovery protocol database

// Serial port profile
const uint8_t sdp_spp_att_handle[] = {
	(SDP_UINT|SDP_32BIT),BT_INITBE32(MIN_SERVICE_HANDLE)};
	
const uint8_t sdp_spp_att_service_class_list[] = {
	(SDP_DES|SDP_SIZE_N8),1+UUID16_LEN,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(SERVICE_UUID16_SSP)};
	
const uint8_t sdp_spp_att_protocol_desc_list[] = {
	(SDP_DES|SDP_SIZE_N8),12,
	(SDP_DES|SDP_SIZE_N8),3,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(PROTOCOL_DESC_L2CAP),
	(SDP_DES|SDP_SIZE_N8),5,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(PROTOCOL_DESC_RFCOMM),	
	(SDP_UINT|SDP_8BIT),RFCOMM_SERIAL_CHANNEL};	
	
const uint8_t sdp_spp_att_browse_group_list[] = {
	(SDP_DES|SDP_SIZE_N8),1+UUID16_LEN,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(PUBLIC_BROWSE_GROUP)};
	
const uint8_t sdp_spp_att_language_base[] = {
	(SDP_DES|SDP_SIZE_N8),9,
	(SDP_UINT|SDP_16BIT),BT_INITBE16(LANGUAGE_ID_ENGLISH),
	(SDP_UINT|SDP_16BIT),BT_INITBE16(LANGUAGE_ENC_UTF8),
	(SDP_UINT|SDP_16BIT),BT_INITBE16(SDP_PRI_LANG)};	
	
const uint8_t sdp_spp_att_service_name[] = {
	(SDP_STR|SDP_SIZE_N8),4,
	'S','P','S','\0'};

const SdpServiceRecord_t spp_service = {
	.atts = 6,
	.att = {
		{.id = SDP_ATT_HANDLE, 		.len = 5, 		.val = sdp_spp_att_handle},
		{.id = SDP_ATT_S_CLASS, 	.len = 5, 		.val = sdp_spp_att_service_class_list},
		{.id = SDP_ATT_PROT_DESC, 	.len = 14, 		.val = sdp_spp_att_protocol_desc_list},
		{.id = SDP_ATT_BROWSE, 		.len = 5, 		.val = sdp_spp_att_browse_group_list},
		{.id = SDP_ATT_LANG_BASE, 	.len = 11, 		.val = sdp_spp_att_language_base},
		{.id = SDP_PRI_LANG+SDP_NAME_OS, .len = 6, 	.val = sdp_spp_att_service_name}}		
}; // End of SPP record	

// Device identification profile v1.3
const uint8_t sdp_did_att_handle[] = {
	(SDP_UINT|SDP_32BIT),BT_INITBE32(MIN_SERVICE_HANDLE+1)};
	
const uint8_t sdp_did_att_service_class_list[] = {
	(SDP_DES|SDP_SIZE_N8),1+UUID16_LEN,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(SERVICE_UUID16_DID)};
	
const uint8_t sdp_did_att_browse_group_list[] = {
	(SDP_DES|SDP_SIZE_N8),1+UUID16_LEN,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(PUBLIC_BROWSE_GROUP)};
	
const uint8_t sdp_did_att_language_base[] = {
	(SDP_DES|SDP_SIZE_N8),9,
	(SDP_UINT|SDP_16BIT),BT_INITBE16(LANGUAGE_ID_ENGLISH),
	(SDP_UINT|SDP_16BIT),BT_INITBE16(LANGUAGE_ENC_UTF8),
	(SDP_UINT|SDP_16BIT),BT_INITBE16(SDP_PRI_LANG)};	
	
const uint8_t sdp_did_att_service_name[] = {
	(SDP_STR|SDP_SIZE_N8),4,
	'D','I','D','\0'};

const uint8_t sdp_did_att_did_version[] = {
	(SDP_UINT|SDP_16BIT),BT_INITBE16(DID_SERVICE_VERSION)};

const uint8_t sdp_did_att_vendor_id[] = {
	(SDP_UINT|SDP_16BIT),BT_INITBE16(DID_VID)};

const uint8_t sdp_did_att_product_id[] = {
	(SDP_UINT|SDP_16BIT),BT_INITBE16(DID_PID)};

const uint8_t sdp_did_att_version[] = {
	(SDP_UINT|SDP_16BIT),BT_INITBE16(DID_VER)};

const uint8_t sdp_did_att_pri_record[] = {
	(SDP_BOOL),0x01};

const uint8_t sdp_did_att_vid_source[] = {
	(SDP_UINT|SDP_16BIT),BT_INITBE16(DID_VID_SRC)};

const SdpServiceRecord_t did_service = {
	.atts = 11,
	.att = {
		{.id = SDP_ATT_HANDLE, 		.len = 5, 		.val = sdp_did_att_handle},
		{.id = SDP_ATT_S_CLASS, 	.len = 5, 		.val = sdp_did_att_service_class_list},
		{.id = SDP_ATT_BROWSE, 		.len = 5, 		.val = sdp_did_att_browse_group_list},
		{.id = SDP_ATT_LANG_BASE, 	.len = 11, 		.val = sdp_did_att_language_base},
		{.id = SDP_PRI_LANG+SDP_NAME_OS, .len = 6, 	.val = sdp_did_att_service_name},
		{.id = SDP_ATT_DID_SPEC, 	.len = 3, 		.val = sdp_did_att_did_version},
		{.id = SDP_ATT_VENDORID, 	.len = 3, 		.val = sdp_did_att_vendor_id},
		{.id = SDP_ATT_PRODUCTID, 	.len = 3, 		.val = sdp_did_att_product_id},
		{.id = SDP_ATT_VERSION, 	.len = 3, 		.val = sdp_did_att_version},
		{.id = SDP_ATT_PRI_RECORD, 	.len = 2, 		.val = sdp_did_att_pri_record},
		{.id = SDP_ATT_VID_SOURCE, 	.len = 3, 		.val = sdp_did_att_vid_source}}
}; // End of DID record		

// The service database used by the sdp protocol	
const SdpServiceRecordDatabase_t sdp_service_record_database = {
	2, {	(SdpServiceRecord_t*)&spp_service,	(SdpServiceRecord_t*)&did_service	}}; // Service database
	

// Found online describing SDP entry for gatt server
#if 0
erin@sundays:~/project/bluez$ sdptool browse local
Browsing FF:FF:FF:00:00:00 ...
Service Name: Generic Attribute Profile
Service Provider: BlueZ
Service RecHandle: 0x10000
Service Class ID List:
  "Generic Attribute" (0x1801)
Protocol Descriptor List:
  "L2CAP" (0x0100)
    PSM: 31
  "ATT" (0x1801)
    uint16: 0x1
    uint16: 0xffff
Profile Descriptor List:
  "Generic Attribute" (0x1801)
    Version: 0x0100
#endif	
	
#define GAP_START_HANDLE	0x0001
#define GAP_LAST_HANDLE		0x0005
#define GATT_START_HANDLE	0x0010
#define GATT_LAST_HANDLE	0x0142

// GAP profile for BR/EDR/LE supporting hosts. BT4 spec p1748
const uint8_t sdp_gap_handle[] = {
	(SDP_UINT|SDP_32BIT),BT_INITBE32(MIN_SERVICE_HANDLE+2)};
	
const uint8_t sdp_gap_service_class_list[] = {
	(SDP_DES|SDP_SIZE_N8),1+UUID16_LEN,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(SERVICE_UUID16_GAP)};
	
const uint8_t sdp_gap_protocol_desc_list[] = {
	(SDP_DES|SDP_SIZE_N8),19,
	(SDP_DES|SDP_SIZE_N8),6,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(PROTOCOL_DESC_L2CAP),
	(SDP_UINT|SDP_16BIT),BT_INITBE16(L2CAP_DESC_PSM_ATT),
	(SDP_DES|SDP_SIZE_N8),9,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(PROTOCOL_DESC_GAP),	
	(SDP_UINT|SDP_16BIT),BT_INITBE16(GAP_START_HANDLE),		
	(SDP_UINT|SDP_16BIT),BT_INITBE16(GAP_LAST_HANDLE)};	
	
const uint8_t sdp_gap_browse_group_list[] = {
	(SDP_DES|SDP_SIZE_N8),1+UUID16_LEN,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(PUBLIC_BROWSE_GROUP)};

const uint8_t sdp_gap_language_base[] = {
	(SDP_DES|SDP_SIZE_N8),9,
	(SDP_UINT|SDP_16BIT),BT_INITBE16(LANGUAGE_ID_ENGLISH),
	(SDP_UINT|SDP_16BIT),BT_INITBE16(LANGUAGE_ENC_UTF8),
	(SDP_UINT|SDP_16BIT),BT_INITBE16(SDP_PRI_LANG)};	
	
const uint8_t sdp_gap_service_name[] = {
	(SDP_STR|SDP_SIZE_N8),4,
	'G','A','P','\0'};
	
const SdpServiceRecord_t gap_service = {
	.atts = 4,
	.att = {
		{.id = SDP_ATT_HANDLE, 		.len = 5, 		.val = sdp_gap_handle},
		{.id = SDP_ATT_S_CLASS, 	.len = 5, 		.val = sdp_gap_service_class_list},
		{.id = SDP_ATT_PROT_DESC, 	.len = 21, 		.val = sdp_gap_protocol_desc_list},		
		{.id = SDP_ATT_BROWSE, 		.len = 5, 		.val = sdp_gap_browse_group_list},
		{.id = SDP_ATT_LANG_BASE, 	.len = 11, 		.val = sdp_gap_language_base},
		{.id = SDP_PRI_LANG+SDP_NAME_OS, .len = 6, 	.val = sdp_gap_service_name}		
}}; // End of GAP record		
	
// GATT profile for BR/EDR/LE supporting hosts. Assumed from BT4 spec p1748
const uint8_t sdp_gatt_handle[] = {
	(SDP_UINT|SDP_32BIT),BT_INITBE32(MIN_SERVICE_HANDLE+3)};
	
const uint8_t sdp_gatt_service_class_list[] = {
	(SDP_DES|SDP_SIZE_N8),1+UUID16_LEN,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(SERVICE_UUID16_GATT)};
	
const uint8_t sdp_gatt_protocol_desc_list[] = {
	(SDP_DES|SDP_SIZE_N8),19,
	(SDP_DES|SDP_SIZE_N8),6,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(PROTOCOL_DESC_L2CAP),
	(SDP_UINT|SDP_16BIT),BT_INITBE16(L2CAP_DESC_PSM_ATT),
	(SDP_DES|SDP_SIZE_N8),9,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(PROTOCOL_DESC_GATT),	
	(SDP_UINT|SDP_16BIT),BT_INITBE16(GATT_START_HANDLE),		
	(SDP_UINT|SDP_16BIT),BT_INITBE16(GATT_LAST_HANDLE)};	
	
const uint8_t sdp_gatt_browse_group_list[] = {
	(SDP_DES|SDP_SIZE_N8),1+UUID16_LEN,
	(SDP_UUID|SDP_16BIT),BT_INITBE16(PUBLIC_BROWSE_GROUP)};

const uint8_t sdp_gatt_language_base[] = {
	(SDP_DES|SDP_SIZE_N8),9,
	(SDP_UINT|SDP_16BIT),BT_INITBE16(LANGUAGE_ID_ENGLISH),
	(SDP_UINT|SDP_16BIT),BT_INITBE16(LANGUAGE_ENC_UTF8),
	(SDP_UINT|SDP_16BIT),BT_INITBE16(SDP_PRI_LANG)};	
	
const uint8_t sdp_gatt_service_name[] = {
	(SDP_STR|SDP_SIZE_N8),5,
	'G','A','T','T','\0'};
	
const SdpServiceRecord_t gatt_service = {
	.atts = 4,
	.att = {
		{.id = SDP_ATT_HANDLE, 		.len = 5, 		.val = sdp_gatt_handle},
		{.id = SDP_ATT_S_CLASS, 	.len = 5, 		.val = sdp_gatt_service_class_list},
		{.id = SDP_ATT_PROT_DESC, 	.len = 21, 		.val = sdp_gatt_protocol_desc_list},		
		{.id = SDP_ATT_BROWSE, 		.len = 5, 		.val = sdp_gatt_browse_group_list},
		{.id = SDP_ATT_LANG_BASE, 	.len = 11, 		.val = sdp_gatt_language_base},
		{.id = SDP_PRI_LANG+SDP_NAME_OS, .len = 7, 	.val = sdp_gatt_service_name}	
}}; // End of GATT record			
	
	
	
	
//EOF
