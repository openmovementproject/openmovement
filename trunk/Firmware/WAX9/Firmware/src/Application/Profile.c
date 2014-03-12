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

// 30-01-2014
// Written to separate BLE handles from the rest of the App 
// to allow more customised configurations. The serial port
// part of the BLE service is still in the app.

// Application specific includes
#include "Compiler.h"
#include "Settings.h"
#include "Analog.h"
#include "ble att.h"
#include "bt app.h"
#include "bt nvm.h"
#include "Profile.h"
#include "bt config.h"

#ifdef ENABLE_LE_MODE

// Output data notification variable
attOutHandle_t dataOutHandle = {0};
attOutHandle_t	metaDataOut = {
	.dataFlag = 0,
	.attCfg = 0,
	.attLen = 8,
	.attData = (void*)&status.pressure
};

void UserProfileInit(void)
{
	// Initialise profile
	memset(&dataOutHandle,0,sizeof(attOutHandle_t));// Zero data out handle
	memset(&metaDataOut,0,sizeof(attOutHandle_t));	// Zero meta data out handle
	metaDataOut.attLen = 8;							// Set length
	metaDataOut.attData = (void*)&status.pressure;	// Set pointer
}

void UserAttributeWrite(const unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset)
{
	switch(handle) {
		// Add support for any written dynamic variables here
		case ATT_COMMAND_HANDLE : {
			unsigned short arg = BT_READLE16(data);
			SettingsCommandNumber(arg);
			break;
		}
		default : 
			break;
	}
	return;
}

void UserAttributeRead(unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset)
{
	switch(handle) {
		// Add support for any read dynamic variables here
		case ATT_BATT_HANDLE :
			data[0] = AdcBattToPercent(battRaw);
			break;
		case ATT_NAME_HANDLE : {
			char* ptr = GetBTName();
			unsigned short length = strlen(ptr);
			if((length)<(len+offset))len = length - offset;
			memcpy(data,ptr+offset,len);
			break;
		}
		default : 
			break;
	}
}

// Readable/writeable dynamic variables may have a variable length 
unsigned short UserAttributeReadLen(unsigned short handle)
{
	switch(handle){
		// Add support for any variable length dynamic fields here
		case ATT_NAME_HANDLE : {
			unsigned short length = strlen(GetBTName());
			return length;
		}
		default : 
			return 0xffff; // Return invalid length (defaults to hard coded one)
			break;
	}
}	

unsigned short UserSendNotifications(void)
{
	// Add support for notifying profile handles here.
	// Check each notifier
	unsigned short retVal = 0;

	// Data out notifier
	if((dataOutHandle.dataFlag == TRUE_) && (dataOutHandle.attCfg & ATT_CFG_NOTIFY))
	{
		unsigned short max_len = BleAttGetMtu() - 3;
		if(dataOutHandle.attLen > max_len) dataOutHandle.attLen = max_len;
		BleAttNotify (dataOutHandle.attData,dataOutHandle.attLen,ATT_DATA_OUT_HANDLE);
		dataOutHandle.dataFlag = FALSE_; // Clear flag
		retVal += dataOutHandle.attLen;
	}

	// Meta data out notifier
	if((metaDataOut.dataFlag == TRUE_) && (metaDataOut.attCfg & ATT_CFG_NOTIFY))
	{
		unsigned short max_len = BleAttGetMtu() - 3;
		if(metaDataOut.attLen > max_len) metaDataOut.attLen = max_len;
		BleAttNotify (metaDataOut.attData,metaDataOut.attLen,ATT_META_OUT_HANDLE);
		metaDataOut.dataFlag = FALSE_; // Clear flag
		retVal += metaDataOut.attLen;
	}

	return retVal;
}

unsigned short UserSendIndications(void)
{
	// Add support for indicating profile handles here.
	// Note: Only one at a time can be sent, put in priority order

	// Data out notifier
	if((dataOutHandle.dataFlag == TRUE_) && (bleIndPending == FALSE_)  && (dataOutHandle.attCfg & ATT_CFG_INDICATE))
	{
		// Checks
		unsigned short max_len = BleAttGetMtu() - 3;
		if(dataOutHandle.attLen > max_len) dataOutHandle.attLen = max_len;
		BleAttIndicate (dataOutHandle.attData,dataOutHandle.attLen,ATT_DATA_OUT_HANDLE);
		bleIndPending = TRUE_;
		dataOutHandle.dataFlag = FALSE_; // Clear flag
		return dataOutHandle.attLen;
	}

	// Meta data out notifier
	if((metaDataOut.dataFlag == TRUE_) && (bleIndPending == FALSE_)  && (metaDataOut.attCfg & ATT_CFG_INDICATE))
	{
		// Checks
		unsigned short max_len = BleAttGetMtu() - 3;
		if(metaDataOut.attLen > max_len) metaDataOut.attLen = max_len;
		BleAttIndicate (metaDataOut.attData,metaDataOut.attLen,ATT_META_OUT_HANDLE);
		bleIndPending = TRUE_;
		metaDataOut.dataFlag = FALSE_; // Clear flag
		return metaDataOut.attLen;
	}

	return 0;
}

#endif
//EOF
