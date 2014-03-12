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

#include "hci + le.h" 	// For mtu size only
#include "l2cap + le.h"	// Install, send data
#include "ble att.h"	// Definitions
#include "bt config.h"	
#include "debug.h"

#ifdef ENABLE_LE_MODE

// PROFILE
#define ATT_CREATE_PROFILE 	// Causes below header to make a copy of the profile
#include "Profile.h"		// Attribute profile header (and profile variable)

// DEBUG
#if DEBUG_BLE
	static const char* file = "ble att.c";
	#define ASSERT(X)			DBG_assert(X, file, __LINE__)
    #define DBG_INFO(X, ...) 	DBG_info(file,X, ##__VA_ARGS__);
    #define DBG_DUMP(X, Y)		DBG_dump(X, Y);
    #define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
#else
    #define DBG_INFO(X, ...)
    #define DBG_DUMP(X, Y)
	#if DBG_ALL_ERRORS			// Leave only errors and assertions on
		static const char* file = "ble att.c";
    	#define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
		#define ASSERT(X)			DBG_assert(X, file, __LINE__)
	#else
    	#define DBG_ERROR(X, ...)
		#define ASSERT(X)
	#endif
#endif

// Definitions
#define ATT_MTU_LIMIT	512			// As per BT4.0 spec

// GLOBALS
static void(*AttWriteHandler)(const unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset) = NULL;
static void(*AttReadHandler)(unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset) = NULL;
static unsigned short (*ExtAttGetLen)(unsigned short handle);
static void (*BleAttHandleValueConfirmCallBack)(void);

// PRIVATE VARIABLES
unsigned short 	att_mtu = 23; 		// Current MTU for connection (23 is default for att)
unsigned short 	att_security = 0;	// Current security flags for connection, 0 is none
unsigned char 	isConnected = FALSE_;// Connection status
// Internal response composition
static unsigned char* respPtr = NULL;
static unsigned short respLen = 0;
static unsigned short indicationTimeout = 0;

// CODE
unsigned char BleAttInit(void)
{
	att_mtu = 23;
	isConnected = FALSE_;
	respPtr = NULL;
	respLen = 0;
	L2capInstallAtt(BleAttPacketHandler);	
	return TRUE_;
}
void BleSetWriteCallback(void(*CallBack)	(const unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset))	
{
	AttWriteHandler = CallBack;
}
void BleSetReadCallback(void(*CallBack)	(unsigned char * data, unsigned short len, unsigned short handle, unsigned short offset))	
{
	AttReadHandler = CallBack;
}
void BleSetGetExtAttLen(unsigned short (*CallBack)	(unsigned short handle))	
{
	ExtAttGetLen = CallBack;
}
void BleSetValSentCallBack(void (*CallBack)	(void))	
{
	BleAttHandleValueConfirmCallBack = CallBack;
}

unsigned short BleAttGetMtu(void) {return att_mtu;}

unsigned char BleAttPacketHandler (const unsigned char * data, unsigned short len)
{
	// Debugging
	DBG_INFO("<-:");
	DBG_DUMP(data,len);

	switch (data[0])	{
		case	ERROR_RESPONSE:		
			DBG_ERROR("ERR:OP:%02X,ER:%02X,HA:%04X\n",data[1],data[3],BT_READLE16(&data[1]));			
			break;			

		case	EXCHANGE_MTU_REQUEST:
			DBG_INFO("mtu req:%u\n",BT_READLE16(&data[1]));	
			BleAttExchangeMtuResponse(data,len);
			break;		
		
		case	EXCHANGE_MTU_RESPONSE:	
			att_mtu = BT_READLE16(&data[1]); // Accept the new mtu
			DBG_INFO("mtu resp:%u\n",att_mtu);	
			break;			

		case	FIND_INFORMATION_REQUEST:
			DBG_INFO("inf req\n");
			BleAttInfoRequest(data,len);					
			break;

		case	FIND_INFORMATION_RESPONSE:
			DBG_ERROR("inf resp!\n");
			break;
		case	FIND_BY_TYPE_VALUE_REQUEST:
			DBG_INFO("fbt req\n");
			BleAttFindByTypeValue(data,len);				
			break;

		case	FIND_BY_TYPE_VALUE_RESPONSE:
			DBG_ERROR("fbt resp!\n");
			break;

		case	READ_BY_TYPE_REQUEST:
			DBG_INFO("rbt req\n");
			BleAttReadByType(data,len);
			break;

		case	READ_BY_TYPE_RESPONSE:
			DBG_ERROR("rbt resp!\n");
			break;

		case	READ_REQUEST:
			DBG_INFO("r req\n");
			BleAttRead(data,len);
			break;
		case	READ_RESPONSE:
			DBG_ERROR("r resp!\n");
			break;

		case	READ_BLOB_REQUEST:
			DBG_INFO("rb req\n");
			BleAttReadBlob(data,len);
			break;

		case	READ_BLOB_RESPONSE:
			DBG_ERROR("rb resp!\n");
			break;

		case	READ_MULTIPLE_REQUEST:
			DBG_INFO("rm req\n");
			BleAttReadMultiple(data,len);
			break;

		case	READ_MULTIPLE_RESPONSE:
			DBG_ERROR("rm resp!\n");
			break;

		case	READ_BY_GROUP_TYPE_REQUEST:
			DBG_INFO("rbg req\n");
			BleAttReadByGroup(data,len);
			break;

		case	READ_BY_GROUP_TYPE_RESPONSE:
			DBG_ERROR("rbg resp!\n");
			break;

		case	WRITE_REQUEST:
			DBG_INFO("w req\n");
			BleAttWriteRequest(data,len);
			break; 
		case	WRITE_RESPONSE:
			DBG_ERROR("w resp!\n");
			break;
		case	WRITE_COMMAND:
			DBG_INFO("wnr req\n");
			BleAttWriteCommand(data,len);
			break; 

		case	PREPARE_WRITE_REQUEST:
			DBG_INFO("pw req\n");
			BleAttPrepareWriteRequest(data,len);
			break;
		case	PREPARE_WRITE_RESPONSE:
			DBG_ERROR("pw resp!\n");
			break;

		case	EXECUTE_WRITE_REQUEST:
			DBG_INFO("ew req\n");
			BleAttExecuteWriteRequest(data,len);
			break;
		case	EXECUTE_WRITE_RESPONSE:
			DBG_ERROR("ew resp!\n");
			break;

		case	HANDLE_VALUE_NOTIFICATION:
			DBG_ERROR("not!\n");
			break;
		case	HANDLE_VALUE_INDICATION:
			DBG_ERROR("ind!\n");
			break;
		case	HANDLE_VALUE_CONFIRMATION:
			DBG_INFO("conf.\n");
			// We must inform the high levels when data send was successful
			BleAttHandleValueConfirmCallBack();
			break;
		case	SIGNED_WRITE_COMMAND:
			DBG_INFO("sw cmd\n");
			BleAttSignedWriteCommand(data,len);
			break;
		default :	break;
		}
	
	return 0;
}						

void BleAttErrorResponse (unsigned char opcode, unsigned short handle, unsigned char reason)
{
	// If we don't already own a buffer, get one
	if(respPtr == NULL) 
		{respPtr = L2capGetSendBuffer();}
	// Make error response
	respPtr[0] = ERROR_RESPONSE;		
	respPtr[1] = opcode;
	BT_WRITELE16(&respPtr[2],handle);
	respPtr[4] = reason;
	// Debug
	DBG_INFO("BLE err:");
	DBG_DUMP(respPtr, 5);
	// Send
	if(!L2capSendCid(respPtr,5, L2CAP_LE_ATT_CID))
		{DBG_ERROR("send fail!\n");}
	// Clear ptr on send
	respPtr = NULL;
	return;	
}

void BleAttExchangeMtuResponse (const unsigned char* data, unsigned short len)
{
	unsigned short mtu = BT_READLE16(&data[1]);
	// Must used smallest value of the clients rx mtu and my maximum rx mtu
	if(mtu < hci_connection.controllerLeBufferSize) att_mtu = mtu;
	else att_mtu = hci_connection.controllerLeBufferSize;
	// The max mtu for att is 512 according to the spec
	if (att_mtu > ATT_MTU_LIMIT)
		att_mtu = ATT_MTU_LIMIT;
	// Make response
	BleAttResponseStart(EXCHANGE_MTU_RESPONSE);
	BT_WRITELE16(&respPtr[1],att_mtu);
	// Debug
	DBG_INFO("mtu: %u\n",att_mtu);
	// Send
	if(!L2capSendCid(respPtr,3, L2CAP_LE_ATT_CID))
		{DBG_ERROR("send fail!\n");}
	// Clear ptr on send
	respPtr = NULL;
	return;
}

unsigned short BleAttResponseStart(unsigned char opCode)
{
	// Init pointer, write op code
	respPtr = L2capGetSendBuffer();
	respPtr[0] = opCode;
	respLen = 1;
	// Return remaining
	return att_mtu - respLen; 
}
unsigned short BleAttResponseAddData(const unsigned char * data, unsigned short len)
{
	// Check there is room for the new entry
	if ((att_mtu - respLen) < len) 
		return 0; 
	// Copy in the entry
	unsigned char* dest = respPtr + respLen;
	for(;len>0;len--)
	{
		// If null source - add zeros
		if(data != NULL)
			*dest++ = *data++;
		else
			*dest++ = 0;
		respLen++;
	}
	// Return remaining length
	return att_mtu - respLen; 			
}
unsigned short BleAttResponseAddInt16LE(const unsigned short val)
{
	// Check there is room for the new entry
	if ((att_mtu - respLen) < sizeof(unsigned short)) 
		return 0; 
	// Copy in the entry
	BT_WRITELE16((respPtr+respLen),val);
	respLen+=sizeof(unsigned short);
	// Return remaining length
	return att_mtu - respLen; 			
}
void BleAttResponseSend(void)
{
	// Debug
	DBG_INFO("->:");
	DBG_DUMP(respPtr, respLen);
	// Send data
	if(!L2capSendCid(respPtr, respLen, L2CAP_LE_ATT_CID))
		{DBG_ERROR("send fail!\n");}
	// Clear ptr on send
	respPtr = NULL;
	return;
}
void BleAttResponseCancel(void)
{
	// Send zero len data
	L2capSendCid(respPtr, 0, L2CAP_LE_ATT_CID);
	// Clear ptr on send
	respPtr = NULL;
	return;	
}

// Return pointer to next handle
attribute16_t* BleAttGetNextHandle(unsigned short startHandle, unsigned short endHandle)
{
	unsigned short i;
	attribute16_t* ret = NULL;
	// Checks, have we a valid search range?
	if(startHandle <= endHandle)
	{
		// Find last index in array
		unsigned short lastHandleIndex = (sizeof(att_profile) / sizeof(void*))-(1+1);// For first and last last null val and zero reffed array
		// Search array for a matching handle - return ptr
		for(i=1;i<=lastHandleIndex;i++)
		{
			ret = (attribute16_t*)att_profile[i];
			if(ret->handle >= startHandle) break;
		}
		// Make sure we still only search within specified range
		if (ret->handle > endHandle) 	return NULL;
		if (ret->handle < startHandle) 	return NULL;
	}
	return ret;
}

void BleAttInfoRequest(const unsigned char* data, unsigned short len)
{
	unsigned short handle, start, end, num, remaining;
	unsigned char format = 0;
	attribute16_t* attribute;

	// Find range
	start = BT_READLE16(&data[1]);
	end = 	BT_READLE16(&data[3]);

	// Check valid
	if((start > end)||(start==0)){BleAttErrorResponse (FIND_INFORMATION_REQUEST, start, INVALID_HANDLE);return;}

	// Init response vars
	num = 0;
	format = 0;
	remaining = BleAttResponseStart(FIND_INFORMATION_RESPONSE);
	handle = start;

	// Look for handles
	for(handle=start;handle<=end;handle++)
	{
		// Get handle in range
		attribute = BleAttGetNextHandle(handle,end);
		// Check it found one
		if(attribute == NULL) break; // No more handles
		// Update handle so next search starts after it
		handle = attribute->handle;
		// Is it the first one
		if(format == 0)
		{
			// Set the format
			if(attribute->flags & ATT_UFLAGS_LONG_UUID) format = 2;
			else 										format = 1;
			// Write format part of command
			remaining = BleAttResponseAddData(&format, 1); 
		}
		else
		{
			// Is it the right length? Can we add it the the list?
			if((attribute->flags & ATT_UFLAGS_LONG_UUID) && (format != 2)) 	 continue; // No
			if((!(attribute->flags & ATT_UFLAGS_LONG_UUID)) && (format != 1))continue; // No
		}

		// Try to add it to the response
		if(format == 1)	
		{
			// Check we have enough room for the entry
			if(remaining < 4) break;
			// Copy to response
			num++;
			remaining = BleAttResponseAddInt16LE(handle);
			remaining = BleAttResponseAddInt16LE(attribute->type);
		}
		else if(format == 2)	
		{
			// Check we have enough room for the entry
			if(remaining < 18) break;
			// Copy to response
			num++;
			remaining = BleAttResponseAddInt16LE(handle);
			remaining = BleAttResponseAddData(((attribute128_t*)attribute)->type , 16);
		}
		else; // 
		handle = attribute->handle;
	}
	// If there is a response to send - send it
	if(num > 0)	BleAttResponseSend();
	else BleAttErrorResponse ( FIND_INFORMATION_REQUEST, start, ATTRIBUTE_NOT_FOUND);
	// Done
	return;
}

void BleAttFindByTypeValue(const unsigned char* data, unsigned short len)
{
	unsigned short handle, start, end, type, num, remaining, currentGroup, groupHandle , attLen, i;
	const unsigned char *value;
	unsigned char *attValue;
	attribute16_t* attribute;

	// Find qualifiers
	start = BT_READLE16(&data[1]);
	end = 	BT_READLE16(&data[3]);
	type = 	BT_READLE16(&data[5]);
	value = &data[7];

	// Check valid
	if((start > end)||(start==0)){BleAttErrorResponse ( FIND_BY_TYPE_VALUE_REQUEST, start, INVALID_HANDLE);return;}

	num = 0;
	remaining = BleAttResponseStart(FIND_BY_TYPE_VALUE_RESPONSE);
	handle = start;
	currentGroup = 0;
	groupHandle = 0;

	// Look for handles - no read priveledges are needed according to spec
	for(handle=start;handle<=end;handle++)
	{
		// Get handle in range
		attribute = BleAttGetNextHandle(handle,end);
		// Check it found one
		if(attribute == NULL) break; // No more handles
		// Update handle so next search starts after it
		handle = attribute->handle;
		// See if we need to compare a 16 or 128 bit types
		// TODO: 128 bit types not supported - technically it could be
		if(attribute->flags & ATT_UFLAGS_LONG_UUID) continue;
		// If we have an open response
		if(currentGroup)
		{
			// Higher or equal grouping level found, close this response - end of current group
			unsigned short attributeGroupType = attribute->type;
			if(attributeGroupType <= currentGroup) 					// Order of types goes primary<secondary<include<characteristic etc.
			{
				// Close this group
				remaining = BleAttResponseAddInt16LE(handle - 1); 	// Close current group in response
				currentGroup = 0;									// Clear group value
				num++;												// Indicate value added
			}
			else
			{
				// Add this handle to the group
				continue; // We know type must be > search type, continue searching group
			}
		}

		// Compare type match
		if(attribute->type != type) continue;

		attValue  = GET_ATT_VAL(attribute);
		attLen = GetAttributeLen(attribute);		

		// Now compare values
		for(i=0;(((attValue[i] == value[i]) )&&(i<attLen));i++){;}
		if(i!=attLen) continue; // Mismatch

		// Have we room for it - 4 bytes per pair
		if (remaining < 4) break;

		// Now add to response
		remaining = BleAttResponseAddInt16LE(handle);
		currentGroup = attribute->type;
		groupHandle = handle;
	}

	// If we have a response open, close it
	if(currentGroup)
	{
		// Close this group assuming last handle is the last in current group
		remaining = BleAttResponseAddInt16LE(handle);
		num++;
	}

	// If there is a response to send - send it
	if(num > 0)	BleAttResponseSend();
	else BleAttErrorResponse ( FIND_BY_TYPE_VALUE_REQUEST, start, ATTRIBUTE_NOT_FOUND);

	// Done
	return;
}	

unsigned char BleAttCanRead(unsigned char doError, unsigned char opcode, unsigned short errHandle, attribute16_t* attribute)
{
	// Check permissions / security 
	if(	(attribute->flags & ATT_UFLAGS_AUTHORISATION_NEEDED)	&& 
		!(att_security & ATT_UFLAGS_AUTHORISATION_NEEDED))
	{
		if(doError)
		{
			BleAttErrorResponse ( opcode , errHandle, INSUFFICIENT_AUTHORIZATION);
		}
		return FALSE_;
	}
	if((attribute->flags & ATT_UFLAGS_AUTHENTICATION_NEEDED)	&& 
		!(att_security & ATT_UFLAGS_AUTHENTICATION_NEEDED	))
	{
		if(doError)
		{
			BleAttErrorResponse ( opcode , errHandle, INSUFFICIENT_AUTHENTICATION);
		}
		return FALSE_;
	}
	if((attribute->flags & ATT_UFLAGS_ENCRYPTION_NEEDED)		&& 
		!(att_security & ATT_UFLAGS_ENCRYPTION_NEEDED))
	{
		if(doError)
		{	
			BleAttErrorResponse ( opcode , errHandle, INSUFFICIENT_ENCRYPTION);
		}
		return FALSE_;
	}
	if(!(attribute->flags & ATT_FLAGS_READ))
	{
		if(doError)
		{
			BleAttErrorResponse ( opcode , errHandle, READ_NOT_PERMITTED);
		}
		return FALSE_;
	}
	return TRUE_;
}

unsigned short GetAttributeLen(attribute16_t* attribute)
{
	unsigned short len;

	// Support for long or short types
	if(attribute->flags & ATT_UFLAGS_LONG_UUID) 
	{
		len = ((attribute128_t*)attribute)->vlen;
	}
	else
	{
		len = attribute->vlen;
	}

	// Add support for variable length strings - characterised as len == 0xffff
	if((attribute->flags & ATT_UFLAGS_POINTER_VALUE) && (len == 0xffff))
	{
		void** valPtr = (void**)GET_MAPPED_ATT_VAL(attribute); 	// Get pointer to the pointer
		if(*valPtr == NULL) len = 0;
		else len = strlen(*valPtr);	
	}

	// Support for external user handles with variable lengths
	else if(attribute->flags & ATT_UFLAGS_DYNAMIC)  
	{
		// Calls user function to get the current length (uses default if not set)
		if(ExtAttGetLen)
		{
			unsigned short dynamicLen = ExtAttGetLen(attribute->handle);
			if (dynamicLen != 0xffff) len = dynamicLen;
		}
	}

	return len;
}

unsigned short BleAttDoRead(unsigned char* data, unsigned short len, unsigned short offset, attribute16_t* attribute)
{
	unsigned short actuallyRead;
	// Is it a user variable
	if(attribute->flags & ATT_UFLAGS_DYNAMIC)
	{
		if (AttReadHandler)
		{
			actuallyRead = len;
			AttReadHandler(data,len,attribute->handle,offset);
		}
		else
		{
			actuallyRead = len;
			memset(data,0,len);
		}
	}
	else if(attribute->flags & ATT_UFLAGS_POINTER_VALUE)
	{
		void** valPtr = (void**)GET_MAPPED_ATT_VAL(attribute); 	// Get pointer to the pointer
		if(*valPtr == NULL) actuallyRead = 0;
		else
		{
			actuallyRead = len;
			memcpy(data,*valPtr+offset,len);				// Read this location in ram
		}
	}
	// Else, read the ROM value associated with it
	else
	{	
		unsigned char* attVal = GET_ATT_VAL(attribute) + offset;
		actuallyRead = len;
		memcpy(data,attVal,len);
	}	
	return actuallyRead;	
}

void BleAttReadByType(const unsigned char* data, unsigned short len)
{
	// RESP: 0x09,len(1),Nx(handle,value)
	unsigned short handle, start, end, type, num, remaining;
	unsigned char valueLen;
	attribute16_t* attribute;

	// Find qualifiers
	start = BT_READLE16(&data[1]);
	end = 	BT_READLE16(&data[3]);
	type = 	BT_READLE16(&data[5]);

	// Check valid
	if((start > end)||(start==0)){BleAttErrorResponse ( READ_BY_TYPE_REQUEST, start, INVALID_HANDLE);return;}

	num = 0;
	remaining = BleAttResponseStart(READ_BY_TYPE_RESPONSE);
	handle = start;
	valueLen = 0;

	// Look for handles
	for(;handle<=end;handle++)
	{
		// Get handle in range
		attribute = BleAttGetNextHandle(handle,end);
		// Check it found one
		if(attribute == NULL) break; // No more handles
		// Update handle so next search starts after it
		handle = attribute->handle;
		// TODO: enable RBT requests on long uuids, for now skip
		if (attribute->flags & ATT_UFLAGS_LONG_UUID) continue; 
		// See if it matches the type we want
		if(attribute->type != type) continue;
		// If its the first value - set the value length
		if(valueLen == 0)
		{
			// See if we can read it - do error and return if we cant
			if(BleAttCanRead(TRUE_, READ_BY_TYPE_REQUEST, start, attribute) == FALSE_)
				return; // Note: Only error if first value can't be read

			// Set the length parameter
			valueLen = GetAttributeLen(attribute);
			unsigned char handleValuePairLen = valueLen + 2;

			// Truncate value length if required on first value only
			if(remaining < (1+handleValuePairLen)) 
			{
				valueLen = (remaining-3);
				handleValuePairLen = valueLen + 2;
				// Spec is unclear if the len field should reflect truncation - it is here
			}

			// Add len field to response
			remaining = BleAttResponseAddData(&handleValuePairLen,1);
		}
		else
		{
			// Make sure its the same length
			if(GetAttributeLen(attribute) != valueLen)	continue;

			// For subsequent handles, just skip unreadable ones
			if(BleAttCanRead(FALSE_, READ_BY_TYPE_REQUEST, start, attribute) == FALSE_)
				continue; // Skip this one
		}

		// Copy handle value pair to response if theres room
		if(remaining < (valueLen + 2)) 				break;

		remaining = BleAttResponseAddInt16LE(handle);
		BleAttDoRead((unsigned char*)(respPtr+respLen), valueLen, 0, attribute);
		remaining -= valueLen;
		respLen += valueLen;
		num++;
		
		// Is there enough room for another handle value pair?
		if(remaining < (valueLen + 2))	break;		
	}

	// If there is a response to send - send it
	if(num > 0)	BleAttResponseSend();
	else BleAttErrorResponse ( READ_BY_TYPE_REQUEST, start, ATTRIBUTE_NOT_FOUND);

	// Done
	return;
}	

void BleAttRead(const unsigned char* data, unsigned short len)
{
	unsigned short remaining, handle;
	unsigned char valueLen;
	attribute16_t* attribute;

	// Find qualifiers
	handle = BT_READLE16(&data[1]);

	// Begin response
	remaining = BleAttResponseStart(READ_RESPONSE);

	// Get handle in range
	attribute = BleAttGetNextHandle(handle,handle);
	// Check it found one
	if(attribute == NULL) 
		{BleAttErrorResponse (  READ_REQUEST,handle,INVALID_HANDLE);return;}

	// See if we can read it - do error and return if we cant
	if(BleAttCanRead(TRUE_, READ_REQUEST, handle, attribute) == FALSE_)
		return; 

	// Get the length parameter
	valueLen = GetAttributeLen(attribute);
	// Copy data
	if(remaining < (valueLen)) valueLen = remaining; // Do truncate
	BleAttDoRead((respPtr+respLen), valueLen, 0, attribute);
	remaining -= valueLen;
	respLen += valueLen;

	// If there is a response to send - send it
	BleAttResponseSend();
	// Done
	return;
}	

void BleAttReadBlob(const unsigned char* data, unsigned short len)
{
	unsigned short remaining, handle, offset;
	unsigned char valueLen;
	attribute16_t* attribute;

	// Find qualifiers
	handle = BT_READLE16(&data[1]);
	offset = BT_READLE16(&data[3]);

	// Begin response
	remaining = BleAttResponseStart(READ_BLOB_RESPONSE );

	// Get handle in range
	attribute = BleAttGetNextHandle(handle,handle);
	// Check it found one
	if(attribute == NULL) 
		{BleAttErrorResponse (  READ_BLOB_REQUEST,handle,INVALID_HANDLE);return;}

	// See if we can read it - do error and return if we cant
	if(BleAttCanRead(TRUE_, READ_REQUEST, handle, attribute) == FALSE_)
		return;

	// Get the length parameter
	valueLen = GetAttributeLen(attribute);
	if(valueLen < (att_mtu-3))
		{BleAttErrorResponse ( READ_BLOB_REQUEST,handle,ATTRIBUTE_NOT_LONG);return;}
	
	// Ensure value offset is valid
	if(valueLen <= offset)
		{BleAttErrorResponse ( READ_BLOB_REQUEST,handle,INVALID_OFFSET);return;}
	
	// Copy data
	valueLen -= offset; 								// Set valueLen to remaining len
	if(remaining < (valueLen)) valueLen = remaining; 	// Do truncate
	BleAttDoRead((respPtr+respLen), valueLen, offset, attribute);
	remaining -= valueLen;
	respLen += valueLen;

	// If there is a response to send - send it
	BleAttResponseSend();
	// Done
	return;
}

void BleAttReadMultiple(const unsigned char* data, unsigned short len)
{
	unsigned short handle, num, remaining, i;
	unsigned char valueLen;
	attribute16_t* attribute;

	num = 0;
	remaining = BleAttResponseStart(READ_MULTIPLE_RESPONSE);
	valueLen = 0;

	// Look for handles (first one is at data[1] so i is 1)
	for(i=1;i<len;i+=2)
	{
		// Get first handle
		handle = BT_READLE16(&data[i]);
		// Get handle in range
		attribute = BleAttGetNextHandle(handle,handle);
		// Check it found one
		if(attribute == NULL)
			{BleAttErrorResponse ( READ_MULTIPLE_REQUEST,handle,INVALID_HANDLE);return;}
		// See if we can read each one - do error and return if we cant
		if(BleAttCanRead(TRUE_, READ_MULTIPLE_REQUEST, handle, attribute) == FALSE_)
			return; 
		// Set the length parameter
		valueLen = GetAttributeLen(attribute);

		// Copy data
		if(remaining < (valueLen)) valueLen = remaining; // Do truncate last value
		BleAttDoRead((respPtr+respLen), valueLen, 0, attribute);
		remaining -= valueLen;
		respLen += valueLen;

		num++;
		if(remaining == 0) break;
	}
	// If there is a response to send - send it
	BleAttResponseSend();
	// Done
	return;
}		

void BleAttReadByGroup(const unsigned char* data, unsigned short len)
{
	unsigned short first, last, start, end, type, num, remaining;
	unsigned char handleGroupValuePairLen, valueLen;
	unsigned char* uuidPtr;
	attribute16_t* attribute;

	// Find qualifiers
	start = BT_READLE16(&data[1]);
	end = 	BT_READLE16(&data[3]);
	last = start; // Inited later
	uuidPtr =  (unsigned char*)&data[5];

	// TODO: Add 128 bit type compare as well
	type = BT_READLE16(&data[5]); 
	ASSERT(len == 7); // If this fails then a 128bit type read has been encountered!

	// Check valid
	if((start > end)||(start==0))
		{BleAttErrorResponse ( READ_BY_GROUP_TYPE_REQUEST, start,INVALID_HANDLE);return;}

	// Init vars
	valueLen = 0;
	num = 0;

	// Begin response
	remaining = BleAttResponseStart(READ_BY_GROUP_TYPE_RESPONSE);

	// First search starts at start, subsequent ones at end of last range
	for(first=start; ;first = last+1) 
	{
		// Find first handle that matches type
		for(; first<=end; first++)
		{
			// KL TODO: Add support for 128bit search too
			attribute = BleAttGetNextHandle(first,end);	
			// Make sure it found one
			if (attribute == NULL)break;		// No more entries
			// Update handle so next search starts after it
			first = attribute->handle;
			if (attribute->flags & ATT_UFLAGS_LONG_UUID) continue; // TODO: Add 128 bit type compare as well
			else if (attribute->type == type)break;	// Match found
		}

		// Early out if not found, if non-found returns att not found error below
		if (attribute == NULL)break;// No more to check
		if (first>end)break;		// Search range finished

		// For first entry match
		if(valueLen == 0)
		{
			// On the first match set length
			valueLen = GetAttributeLen(attribute);
			handleGroupValuePairLen = 4 + valueLen;

			// Add length to response
			remaining = BleAttResponseAddData(&handleGroupValuePairLen,1);

			// See if read is permitted WITH error out
			if(BleAttCanRead(TRUE_, READ_BY_GROUP_TYPE_REQUEST, start, attribute) == FALSE_)
				return; 
		}
		else
		{
			// See if read is permitted WITHOUT error out - skip if can't read				
			if(BleAttCanRead(FALSE_, READ_BY_GROUP_TYPE_REQUEST, start, attribute) == FALSE_)
				continue; 
		}	
		
		// Now find the LAST handle in this group
		for(last=(first+1);last<=end; last++)
		{
			attribute16_t* temp = BleAttGetNextHandle(last,end);	
			if (temp == NULL)
			{
				last--;
				break;		// End of group not found - set end handle
			}

			// Update handle so next search starts after it
			last = temp->handle;

			// Skip long types
			if (temp->flags & ATT_UFLAGS_LONG_UUID)
			{
				continue; 	// TODO: Add compare for long uuid types
			}
			else if(temp->type <= attribute->type) 
			{
				last--;		// New starting group found - set end handle
				break;
			}
		}

		// Only copy in the handles and values if SAME LENGTH and theres room
		if(remaining < handleGroupValuePairLen) break;		// No more room
		if(valueLen != GetAttributeLen(attribute))  continue;	// Different size, skip it

		// Add to handles response
		remaining = BleAttResponseAddInt16LE(first);
		remaining = BleAttResponseAddInt16LE(last);
		// Add value
		BleAttDoRead((respPtr+respLen), valueLen, 0, attribute);
		remaining -= valueLen;
		respLen += valueLen;
		// Indicate response has data
		num ++;
		// Make sure theres any point checking for more handle ranges
		if(remaining < handleGroupValuePairLen) break;		// No more room
	}

	// If there is a response to send - send it
	if(num > 0)	BleAttResponseSend();
	else BleAttErrorResponse ( READ_BY_GROUP_TYPE_REQUEST, start, ATTRIBUTE_NOT_FOUND);

	// Done
	return;
}

unsigned char BleAttCanWrite(unsigned char doError, unsigned char opcode, unsigned short errHandle, attribute16_t* attribute, unsigned short length)
{
	// Permission checks
	if(	(attribute->flags & ATT_UFLAGS_AUTHORISATION_NEEDED)	&& 
		!(att_security & ATT_UFLAGS_AUTHORISATION_NEEDED))
	{
		if(doError)
		{
			BleAttErrorResponse (opcode, errHandle,INSUFFICIENT_AUTHORIZATION);
		}
		return FALSE_;
	}
	if((attribute->flags & ATT_UFLAGS_AUTHENTICATION_NEEDED)	&& 
		!(att_security & ATT_UFLAGS_AUTHENTICATION_NEEDED))
	{
		if(doError)
		{
			BleAttErrorResponse (opcode, errHandle, INSUFFICIENT_AUTHENTICATION);
		}
		return FALSE_;
	}
	if((attribute->flags & ATT_UFLAGS_ENCRYPTION_NEEDED)	&& 
		!(att_security & ATT_UFLAGS_ENCRYPTION_NEEDED))
	{
		if(doError)
		{
			BleAttErrorResponse (opcode, errHandle,INSUFFICIENT_ENCRYPTION);
		}
		return FALSE_;
	}
	if(!(attribute->flags & ATT_FLAGS_WRITE || attribute->flags & ATT_FLAGS_WRITE_NR))
	{
		if(doError)
		{
			BleAttErrorResponse (opcode, errHandle,WRITE_NOT_PERMITTED);
		}
		return FALSE_;
	}
	if(length > GetAttributeLen(attribute))
	{
		if(doError)
		{
			BleAttErrorResponse (opcode, errHandle,INVALID_ATTRIBUTE_VALUE_LENGTH);
		}
		return FALSE_;
	}
	return TRUE_;
}

void BleAttDoWrite(const unsigned char* data, unsigned short len, unsigned short offset, attribute16_t* attribute)
{
	// Create connection event on write
	BTEvent(BT_EVENT_COMM_OPEN);

	// Is this a user variable
	if(attribute->flags & ATT_UFLAGS_DYNAMIC)
	{
		// Write value using call back
		if(AttWriteHandler)
		{
			AttWriteHandler(data, len, attribute->handle, offset);
		}
	}
	if(attribute->flags & ATT_UFLAGS_POINTER_VALUE)
	{
		void** valPtr = (void**)GET_MAPPED_ATT_VAL(attribute); 	// Get pointer to the pointer
		if(*valPtr != NULL)										// Check pointer
			memcpy(*valPtr+offset,data,len);					// Write it
	}
	else
	{
		// There are no default writable values we need to support - do not enable writable rom values
	}
}

void BleAttWriteRequest(const unsigned char* data, unsigned short len)
{
	unsigned short handle;
	attribute16_t* attribute;

	// Find qualifiers
	handle = BT_READLE16(&data[1]);
	attribute = BleAttGetNextHandle(handle,handle);	

	// Checks
	if(attribute == NULL)	
		{BleAttErrorResponse( WRITE_REQUEST,handle,INVALID_HANDLE);return;}

	len -= 3; // Set len to value len
	data += 3;// Set ptr to value

	// See if we can write
	if(BleAttCanWrite(TRUE_, WRITE_REQUEST,handle, attribute,len))
	{
		// Do write
		BleAttDoWrite(data, len, 0, attribute);
		// Respond
		BleAttResponseStart(WRITE_RESPONSE);
		BleAttResponseSend();
	}
	return;
}

void BleAttWriteCommand(const unsigned char* data, unsigned short len)
{
	unsigned short handle;
	attribute16_t* attribute;

	// Find qualifiers
	handle = BT_READLE16(&data[1]);
	attribute = BleAttGetNextHandle(handle,handle);

	// Checks
	if(attribute == NULL)	
		{BleAttErrorResponse( WRITE_COMMAND, handle, INVALID_HANDLE);return;}
	
	len -= 3; // Set len to value len
	data += 3;// Set ptr to value

	// No response
	// See if we can write
	if(BleAttCanWrite(TRUE_, WRITE_COMMAND,handle, attribute,len))
	{
		// Do write
		BleAttDoWrite(data, len, 0, attribute);
	}

	return;
}

void BleAttSignedWriteCommand(const unsigned char* data, unsigned short len)
{
	unsigned short handle;
	attribute16_t* attribute;

	// Find qualifiers
	handle = BT_READLE16(&data[1]);
	attribute = BleAttGetNextHandle(handle,handle);

	// Checks
	if(attribute == NULL)	
		{BleAttErrorResponse( SIGNED_WRITE_COMMAND, handle, INVALID_HANDLE);return;}
	
	// TODO: Check signature is ok

	len -= 3; // Set len to value len
	len -= 12;// Remove signature
	data += 3;// Set ptr to value

	// No response
	// See if we can write
	if(BleAttCanWrite(TRUE_, SIGNED_WRITE_COMMAND, handle, attribute,len))
	{
		// Do write
		BleAttDoWrite(data, len, 0, attribute);
	}

	return;
}

void BleAttPrepareWriteRequest(const unsigned char* data, unsigned short len)
{
	unsigned short handle, offset;
	attribute16_t* attribute;

	// Find qualifiers
	handle = BT_READLE16(&data[1]);
	offset = BT_READLE16(&data[3]);
	attribute = BleAttGetNextHandle(handle,handle);

	// Checks
	if(attribute == NULL)	
		{BleAttErrorResponse( PREPARE_WRITE_REQUEST, handle,INVALID_HANDLE);return;}
	
	len -= 5; // Set len to value len
	data += 5;// Set ptr to value

	// See if we can write
	if(BleAttCanWrite(TRUE_, PREPARE_WRITE_REQUEST, handle, attribute,(offset+len)))
	{
		// Set partial write flag
		attribute->flags |= ATT_UFLAGS_QUEUED_WRITE_PENDING; 
		// Do write
		BleAttDoWrite(data, len, offset, attribute);
		// Respond
		BleAttResponseStart(WRITE_RESPONSE);
		BleAttResponseAddInt16LE(handle);
		BleAttResponseAddInt16LE(offset);	
		BleAttResponseAddData(data,len);
		BleAttResponseSend();
	}

	return;
}

void BleAttExecuteWriteRequest(const unsigned char* data, unsigned short len)
{
	unsigned short handle;
	attribute16_t* attribute;

	for(handle = 1;handle < 0xffff; handle++)
	{
		attribute = BleAttGetNextHandle(handle,0xffff);
		// Update handle so next search starts after it
		handle = attribute->handle;
		if(attribute == NULL) break; // No more handles
		else 
		{
			attribute->flags &= ~(ATT_UFLAGS_QUEUED_WRITE_PENDING); // Clear pending flags
		}
	}
	// Respond
	BleAttResponseStart(EXECUTE_WRITE_RESPONSE);
	BleAttResponseSend();
	return;
}

unsigned char BleAttNotify (const unsigned char * data, unsigned short len, unsigned short handle)	
{
	unsigned short remaining;
	// Make response
	BleAttResponseStart(HANDLE_VALUE_NOTIFICATION);
	remaining = BleAttResponseAddInt16LE(handle);
	if(remaining >= len)
	{
		BleAttResponseAddData(data,len);
		BleAttResponseSend();
		return TRUE_;
	}
	BleAttResponseCancel();
	return FALSE_;
}

unsigned char BleAttIndicate (const unsigned char * data, unsigned short len, unsigned short handle)	
{
	unsigned short remaining;
	// Make response
	BleAttResponseStart(HANDLE_VALUE_INDICATION);
	remaining = BleAttResponseAddInt16LE(handle);
	if(remaining >= len)
	{
		BleAttResponseAddData(data,len);
		BleAttResponseSend();
		indicationTimeout = ATT_IND_TIMEOUT;
		return TRUE_;
	}
	BleAttResponseCancel();
	return FALSE_;
}

void BleTimeoutHandler(void)
{
	if (indicationTimeout && --indicationTimeout == 0)
	{
		// Indication timeout - clear flag anyway
		DBG_INFO("ind timeout!\n");
		BleAttHandleValueConfirmCallBack();
	}
	return;
}

#endif

