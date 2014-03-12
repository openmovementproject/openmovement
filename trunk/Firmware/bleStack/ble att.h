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

// BLE ATT
// Karim Ladha, 2013-2014

#ifndef BLE_ATT_H
#define BLE_ATT_H

// TYPES
// Master type - used for searching therefore common to all sub types
typedef struct {			// Generic attribute master type, service and value types
	unsigned short handle;	// Index of the attribute
	unsigned short flags;   // Flags
	unsigned short type;	// Type 16/128bit uuid
	unsigned short vlen;	// Value length
	unsigned char  value[];	// Value
}attribute16_t;

// Long uuid attribute entry type
typedef struct {			// Generic attribute large uuid master type, service and value types
	unsigned short handle;	// Index of the attribute
	unsigned short flags;   // Flags
	unsigned char  type[16];// Type 128bit uuid
	unsigned short vlen;	// Value length
	unsigned char  value[];	// Value
}attribute128_t;

// A simple data output type, useful for creating profiles with output variables
typedef struct {
	unsigned char dataFlag; 	// Attribute update flag, inform of new data
	unsigned short attCfg;		// Map to variable as _CLI_CHAR type + readable/writable
	unsigned short attLen;		// Set data length
	unsigned char* attData;		// Set data pointer
}attOutHandle_t;
#define ATT_CFG_NOTIFY 		0x0001
#define ATT_CFG_INDICATE 	0x0002

// GLOBALS

// Definitions


// PROTOTYPES
unsigned char BleAttInit							(void);
unsigned short BleAttGetMtu							(void);
unsigned char BleAttNotify							(const unsigned char * data, unsigned short len, unsigned short handle);		// Send data to remote device without confirmation
unsigned char BleAttIndicate						(const unsigned char * data, unsigned short len, unsigned short handle);		// Send data to remote device with confirmation
void          BleTimeoutHandler (void);
void 		  BleSetWriteCallback(void(*CallBack)	(const unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset));// Will be called when incoming data arrives
void 		  BleSetReadCallback (void(*CallBack)	(unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset));		// Called to read non-static declared handles
void 		  BleSetGetExtAttLen(unsigned short (*CallBack)	(unsigned short handle));
void 		  BleSetValSentCallBack(void (*CallBack)	(void));

// OP CODES
#define ERROR_RESPONSE    			0x01 //Request Opcode in Error, Attribute Handle In Error, Error Code
#define EXCHANGE_MTU_REQUEST  		0x02 //Client Rx MTU
#define EXCHANGE_MTU_RESPONSE  		0x03 //Server Rx MTU
#define FIND_INFORMATION_REQUEST 	0x04 //Starting Handle, Ending Handle, UUID
#define FIND_INFORMATION_RESPONSE 	0x05 //Format, Information Data 
#define FIND_BY_TYPE_VALUE_REQUEST 	0x06 //Starting Handle, Ending Handle, Attribute Type, Attribute Value
#define FIND_BY_TYPE_VALUE_RESPONSE 0x07 //Handles Information List 
#define READ_BY_TYPE_REQUEST  		0x08 //Starting Handle, Ending Handle, UUID
#define READ_BY_TYPE_RESPONSE  		0x09 //Length, Attribute Data List 
#define READ_REQUEST    			0x0A //Attribute Handle 
#define READ_RESPONSE    			0x0B //Attribute Value
#define READ_BLOB_REQUEST   		0x0C //Attribute Handle, Value Offset 
#define READ_BLOB_RESPONSE   		0x0D //Part Attribute Value 
#define READ_MULTIPLE_REQUEST  		0x0E //Handle Set 
#define READ_MULTIPLE_RESPONSE  	0x0F //Value Set 
#define READ_BY_GROUP_TYPE_REQUEST 	0x10 //Start Handle, Ending Handle, UUID
#define READ_BY_GROUP_TYPE_RESPONSE 0x11 //Length, Attribute Data List 
#define WRITE_REQUEST    			0x12 //Attribute Handle, Attribute Value 
#define WRITE_RESPONSE    			0x13 //Ack 
#define WRITE_COMMAND    			0x52 //Attribute Handle, Attribute Value 
#define PREPARE_WRITE_REQUEST  		0x16 //Attribute Handle, Value_Offset, Part Attribute Value
#define PREPARE_WRITE_RESPONSE  	0x17 //Attribute Handle, Value Offset, Part Attribute Value
#define EXECUTE_WRITE_REQUEST  		0x18 //Flags 
#define EXECUTE_WRITE_RESPONSE  	0x19 //Ack 
#define HANDLE_VALUE_NOTIFICATION 	0x1B //Attribute Handle, Attribute Value 
#define HANDLE_VALUE_INDICATION  	0x1D //Attribute Handle, Attribute Value 
#define HANDLE_VALUE_CONFIRMATION 	0x1E //Ack 
#define SIGNED_WRITE_COMMAND  		0xD2 //Attribute Handle, Attribute Value, Authentication Signature

// ERROR CODES
#define INVALID_HANDLE						0x01
#define READ_NOT_PERMITTED					0x02
#define WRITE_NOT_PERMITTED					0x03
#define INVALID_PDU							0x04
#define INSUFFICIENT_AUTHENTICATION			0x05
#define REQUEST_NOT_SUPPORTED				0x06
#define INVALID_OFFSET						0x07
#define INSUFFICIENT_AUTHORIZATION			0x08
#define PREPARE_QUEUE_FULL					0x09
#define ATTRIBUTE_NOT_FOUND					0x0A
#define ATTRIBUTE_NOT_LONG					0x0B
#define INSUFFICIENT_ENCRYPTION_KEY_SIZE	0x0C
#define INVALID_ATTRIBUTE_VALUE_LENGTH		0x0D
#define UNLIKELY_ERROR						0x0E
#define INSUFFICIENT_ENCRYPTION				0x0F
#define UNSUPPORTED_GROUP_TYPE				0x10
#define INSUFFICIENT_RESOURCES				0x11
#define APPLICATION_ERROR					0x80

// GATT Declarations - type
#define GATT_DECLARATION_PRIMARY_SERVICE                0x2800
#define GATT_DECLARATION_SECONDARY_SERVICE              0x2801
#define GATT_DECLARATION_INCLUDE                        0x2802
#define GATT_DECLARATION_CHARACTERISTIC                 0x2803
 
// GATT Characteristic Descriptors
#define GATT_DESCRIPTOR_EXTENDED_PROPERTIES             0x2900
#define GATT_DESCRIPTOR_USER_DESCRIPTION                0x2901
#define GATT_DESCRIPTOR_CONFIGURATION_CLIENT            0x2902
#define GATT_DESCRIPTOR_CONFIGURATION_SERVER            0x2903
#define GATT_DESCRIPTOR_PRESENTATION_FORMAT             0x2904
#define GATT_DESCRIPTOR_AGGREGATE_FORMAT                0x2905
#define GATT_DESCRIPTOR_VALID_RANGE                     0x2906
#define GATT_DESCRIPTOR_EXTERNAL_REPORT_REFERENCE       0x2907
#define GATT_DESCRIPTOR_REPORT_REFERENCE                0x2908
 
// GATT Characteristics
#define GATT_CHARACTERISTIC_DEVICE_NAME                 0x2A00  // Device name (UTF-8)
#define GATT_CHARACTERISTIC_APPEARANCE                  0x2A01  // Device physical appearance
#define GATT_CHARACTERISTIC_SERVICE_CHANGED				0x2A05	// Used to inform remote device of changes
#define GATT_CHARACTERISTIC_DATE_TIME                   0x2A08  // uint16 year; uint8 month, day, hours, minutes, seconds
#define GATT_CHARACTERISTIC_BATTERY_LEVEL               0x2A19  // Battery percentage (0-100)
#define GATT_CHARACTERISTIC_TEMPERATURE_MEASUREMENT     0x2A1C  // Temperature Measurement (see spec.)
#define GATT_CHARACTERISTIC_MODEL_NUMBER_STRING         0x2A24  // Model number string (UTF-8)
#define GATT_CHARACTERISTIC_SERIAL_NUMBER_STRING        0x2A25  // Serial number string (UTF-8)
#define GATT_CHARACTERISTIC_FIRMWARE_REVISION_STRING    0x2A26  // Firmware revision string (UTF-8)
#define GATT_CHARACTERISTIC_HARDWARE_REVISION_STRING    0x2A27  // Hardware revision string (UTF-8)
#define GATT_CHARACTERISTIC_SOFTWARE_REVISION_STRING    0x2A28  // Software revision string (UTF-8)
#define GATT_CHARACTERISTIC_MANUFACTURER_NAME_STRING    0x2A29  // Manufacturer name string (UTF-8)
#define GATT_CHARACTERISTIC_BODY_SENSOR_LOCATION        0x2A38  // 0=Other, 1=Chest, 2=Wrist, 3=Finger, 4=Hand, 5=Ear Lobe, 6=Foot (8-bit)
 
// GATT Units
#define GATT_UNIT_UNITLESS                              0x2700  // unitless
#define GATT_UNIT_SECOND                                0x2703  // time (second)
#define GATT_UNIT_HERTZ                                 0x2722  // frequency (hertz)
#define GATT_UNIT_PERCENTAGE                            0x27AD  // percentage
#define GATT_UNIT_VOLT                                  0x2728  // electric potential difference (volt)
#define GATT_UNIT_METRES_PER_SECOND_SQUARED             0x2713  // acceleration (metres per second squared)
#define GATT_UNIT_RADIAN_PER_SECOND                     0x2743  // angular velocity (radian per second)
#define GATT_UNIT_TESLA                                 0x272D  // magnetic flex density (tesla)
#define GATT_UNIT_KELVIN                                0x2705  // thermodynamic temperature (kelvin)
#define GATT_UNIT_PASCAL                                0x2724  // pressure (pascal)
#define GATT_UNIT_LUX                                   0x2731  // illuminance (lux)
 
// GATT Services
#define GATT_SERVICE_GENERIC_ACCESS                     0x1800
#define GATT_SERVICE_GENERIC_ATTRIBUTE                  0x1801
#define GATT_SERVICE_CURRENT_TIME                       0x1805
#define GATT_SERVICE_DEVICE_INFORMATION                 0x180A
#define GATT_SERVICE_BATTERY                            0x180F

// Properties for the flags field - from bt spec4
#define ATT_FLAGS_BROADCAST		0x01
#define ATT_FLAGS_READ			0x02
#define ATT_FLAGS_WRITE_NR		0x04
#define ATT_FLAGS_WRITE			0x08
#define ATT_FLAGS_NOTIFY		0x10
#define ATT_FLAGS_INDICATE		0x20
#define ATT_FLAGS_WRITE_SGND	0x40
#define ATT_FLAGS_EXTENDED		0x80
#define ATT_FLAGS_RW			0x0A

// User properties used to identify attribute parameters
#define ATT_UFLAGS_LONG_UUID				0x8000
#define ATT_UFLAGS_AUTHORISATION_NEEDED		0x4000
#define ATT_UFLAGS_AUTHENTICATION_NEEDED	0x2000
#define ATT_UFLAGS_ENCRYPTION_NEEDED		0x1000
#define ATT_UFLAGS_QUEUED_WRITE_PENDING		0x0800
#define ATT_UFLAGS_DYNAMIC					0x0400 // Causes Att.c to call BleDataOut(...)
#define ATT_UFLAGS_POINTER_VALUE			0x0200 // Length field important, uses value as byte*	

// Helper macros to enable co-existance of different uuids
#define GET_ATT_VAL(attptr)	((unsigned char*)((attptr->flags & ATT_UFLAGS_LONG_UUID)?(((attribute128_t*)attptr)->value): (attptr->value)))
#define GET_MAPPED_ATT_VAL(attptr)	((unsigned char*)(((attribute128_t*)attptr)->value)) // Mapped attributes are fixed length 24 bytes, pointer at end

// Need a new uuid? This one is recommended by the bluetooth sig
// http://www.itu.int/ITU-T/asn1/cgi-bin/uuid_generate
// Bluetooth base uuid: 00000000-0000-1000-8000-00805F9B34FB

// Private prototypes
unsigned char BleAttPacketHandler (const unsigned char * data, unsigned short len); // Handle and ACL len removed, call in L2CAP on CID 0x4
// Assemble complex response
unsigned short BleAttResponseStart(unsigned char opCode);
unsigned short BleAttResponseAddData(const unsigned char * data, unsigned short len);
unsigned short BleAttResponseAddInt16LE(const unsigned short val);
void 		   BleAttResponseSend(void);
void 		   BleAttResponseCancel(void);
// Simple responses
void BleAttErrorResponse (unsigned char opcode, unsigned short handle, unsigned char reason);
void BleAttExchangeMtuResponse (const unsigned char* data, unsigned short len);
// Read info handlers
void BleAttInfoRequest(const unsigned char* data, unsigned short len);
void BleAttFindByTypeValue(const unsigned char* data, unsigned short len);
// Read command handlers
unsigned short GetAttributeLen(attribute16_t* attribute);
unsigned char BleAttCanRead(unsigned char doError, unsigned char opcode, unsigned short errHandle, attribute16_t* attribute);
unsigned short BleAttDoRead(unsigned char* data, unsigned short len, unsigned short offset, attribute16_t* attribute);
void BleAttReadByType(const unsigned char* data, unsigned short len);
void BleAttRead(const unsigned char* data, unsigned short len);
void BleAttReadBlob(const unsigned char* data, unsigned short len);
void BleAttReadMultiple(const unsigned char* data, unsigned short len);
void BleAttReadByGroup(const unsigned char* data, unsigned short len);
// Write command handlers
unsigned char BleAttCanWrite(unsigned char doError, unsigned char opcode, unsigned short errHandle, attribute16_t* attribute, unsigned short length);
void BleAttDoWrite(const unsigned char* data, unsigned short len, unsigned short offset, attribute16_t* attribute);
void BleAttWriteRequest(const unsigned char* data, unsigned short len);
void BleAttWriteCommand(const unsigned char* data, unsigned short len);
void BleAttPrepareWriteRequest(const unsigned char* data, unsigned short len);
void BleAttExecuteWriteRequest(const unsigned char* data, unsigned short len);
void BleAttSignedWriteCommand(const unsigned char* data, unsigned short len);

// Helpful definitions to make profiles user readable (width help align in table)
//		0x00,0x00

// Flags
#define _READABLE			0x02,0x00
#define _READ_DYN			0x02,0x02
#define _RW_DYNA_			0x0A,0x02 

// UUID16 types
#define _PRI_SER_			0x00,0x28
#define _SEC_SER_        	0x01,0x28
#define _ATT_INC_			0x02,0x28
#define _CHARACT_			0x03,0x28
#define _GAP_SER_			0x00,0x18
#define _GAT_SER_			0x01,0x18
#define _INFO_SER			0x0A,0x18
#define _DEV_NAME           0x00,0x2A  
#define _APPEARAN           0x01,0x2A
#define _PRIV_SET			0x02,0x2A
#define _REC_ADD_			0x03,0x2A
#define _PREF_CON			0x04,0x2A 
#define _SER_CHA_			0x05,0x2A
#define _BAT_LEV_           0x19,0x2A 
#define _SYS_ID__         	0x23,0x2A  
#define _MOD_NUM_         	0x24,0x2A  
#define _SER_NUM_        	0x25,0x2A  
#define _FIRMWAR_    		0x26,0x2A 
#define _HARDWAR_    		0x27,0x2A  
#define _SOFTWARE			0x28,0x2A
#define _MAN_NAME    		0x29,0x2A  
#define _IEEE_REG			0x2A,0x2A
#define _PnP_ID__			0x50,0x2A
#define _CHAR_FMT			0x04,0x29
#define _USER_DES           0x01,0x29
#define _CLI_CHAR			0x02,0x29

// Mapped variable length fields. Use for len field for mapped attributes only
#define _STRINGT_			0xff,0xff 
#define _INT32_T_			0x04,0x00
#define _INT24_T_			0x03,0x00
#define _INT16_T_			0x02,0x00
#define _INT8_T__			0x01,0x00
#endif
