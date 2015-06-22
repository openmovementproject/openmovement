// Device specific definitions for attibutes, descriptors and service classes
// Karim Ladha, 2015
#ifndef _PROFILE_H_
#define _PROFILE_H_

// Headers
#include <stdint.h>
#include "att.h" // For handle types

// HCI layer device attributes
// Device properties
#define BT_DEFAULT_NAME				"AX9-"
#define BT_DEFAULT_PIN 				"0000"
#define BT_DEFAULT_COD 				0x080704ul	//0x040424	

// EIR other entries (little endian) - DID profile entry
#define EIR_ADDITIONAL_ENTRIES_LEN	10
#define EIR_ADDITIONAL_ENTRIES	eir_did_entry
extern const uint8_t eir_did_entry[];

// EIR service uuid lists
#define BT_EIR_SERVICE16_LIST_LEN	1	// SPS 0x1101, Serial port service uuid
#define BT_EIR_SERVICE16_LIST		"\x01\x11"  
#define BT_EIR_SERVICE32_LIST_LEN	0	// No 32bit uuids
#define BT_EIR_SERVICE32_LIST
#define BT_EIR_SERVICE128_LIST_LEN	1	// iAP service uuid 00000000-DECA-FADE-DECA-DEAFDECACAFF
#define BT_EIR_SERVICE128_LIST		"\xFF\xCA\xCA\xDE\xAF\xDE\xCA\xDE\xDE\xFA\xCA\xDE\x00\x00\x00\x00"

#if 0
// Service Record Database - created in profile.c... Not at the moment, using legacy spp
typedef struct SdpServiceRecord_tag* ServiceRecordDatabase_t;
extern const ServiceRecordDatabase_t gServiceRecordDatabase[];
#endif

// Attribute layer device profile
// The profile used by att.c
extern const void* att_profile[];
extern const uint16_t att_profile_entries;
// Attribute dynamic handles - att.c will use callbacks for access
#define ATT_SERIAL_IN_HANDLE	0x0022 	
#define ATT_SERIAL_OUT_HANDLE	0x0025	
#define ATT_NAME_HANDLE			0x0003	
#define ATT_BATT_HANDLE			0x0032	
#define ATT_COMMAND_HANDLE		0x0041	
#define ATT_DATA_OUT_HANDLE		0x0051	
#define ATT_META_OUT_HANDLE		0x0071	

// BLE serial emulation setings
#define ATT_USE_BLE_SERIAL			// Creates serial port
#define LE_ADD_CR_TO_CMDS			// Each packet to serial port input handle is CR teminated

// Dynamic handles
extern attOutHandle_t attSerialOut;		// BLE serial port data out
extern attOutHandle_t dataOutHandle; 	// Data out for streamer
extern attOutHandle_t metaDataOut; 		// Meta-data out for streamer

// Prototypes
void ProfileInit(void);

#endif

