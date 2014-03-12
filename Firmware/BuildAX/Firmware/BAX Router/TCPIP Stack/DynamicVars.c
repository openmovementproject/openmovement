/* 
 * Copyright (c) 2014, Newcastle University, UK.
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
// Karim Ladha, Samuel Finnigan, 2014

#include "GenericTypeDefs.h"
#include "TCPIPConfig.h"
#include "DynamicVars.h"

// Time support
#include "Peripherals/Rtc.h"
// File support
#include "HardwareProfile.h"
#ifdef USE_FAT_FS
	#include "FatFs/FatFsIo.h"
#else
	#include "MDD File System/FSIO.h"
#endif

//#define DEBUG_ON
#include "Common/debug.h"

/* // The DynEntry_t struct is defined in DynamicVars.h, but is here for documentation:
 *	typedef struct {
 *		CONST_DYN_LIST unsigned short id;
 *		CONST_DYN_LIST DynamicType_t type;
 *		CONST_DYN_LIST void* ptr;
 * }DynEntry_t;
 */
 
typedef struct {
	DynEntry_t* 	entry;
	FSFILE* 		file;
	unsigned long 	offset;
	unsigned long 	fileSize;
}DynTxClient_t;

DynTxClient_t activeClient[MAX_HTTP_CONNECTIONS];

// Static const dynamic types
DynEntry_t	dynVarMissing		={0xffff,DYN_ERR,""};
DynEntry_t	dynDoubleTild		={0xffff,DYN_TEXT,"~"};
DynEntry_t	dynFieldLenErr		={0xffff,DYN_TEXT,"!MALFORMED FIELD!"};
DynEntry_t	dynFieldFile		={0xffff,DYN_FILE,""};

#define DYN_MISSING			(&dynVarMissing)
#define DYN_DOUBLE			(&dynDoubleTild)
#define DYN_FIELD_ERR		(&dynFieldLenErr)
#define DYN_FILE_VAR		(&dynFieldFile)

// One instance only
#include "DynamicIdList.h"
static void DynamicClientClear(DynTxClient_t* dynClient)
{
	memset(dynClient,0,sizeof(DynTxClient_t));
}

static DynamicType_t DynamicLoadId(unsigned short id, DynTxClient_t* dynClient)
{
	unsigned short i;
	DynamicClientClear(dynClient);
	dynClient->entry = DYN_MISSING;
	// Enumerated number
	// Find match
	for(i=0;i<(sizeof(dynEntries)/sizeof(DynEntry_t));i++)
	{	
		if(dynEntries[i].id == id)
		{
			dynClient->entry = &dynEntries[i];
			return TRUE;
		} 
	}

	return FALSE;
}

static DynamicType_t DynamicLoadFile(char* path, DynTxClient_t* dynClient)
{
	DynamicClientClear(dynClient);
	// Don't check if null - done in reader
	dynClient->entry = DYN_FILE_VAR;
	dynClient->file = FSfopen(path, "r");
	if(dynClient->file != NULL)
	{
		#ifdef USE_FAT_FS
			dynClient->fileSize = (unsigned long)(dynClient->file->fsize);
		#else
			dynClient->fileSize = (unsigned long)(dynClient->file->size);
		#endif
		return TRUE;
	}
	return FALSE;
}

unsigned short DynamicVarLoad(char* sourceFile, unsigned char clientIndex)
{
	unsigned short len = 1, id;
	char* field = &sourceFile[1];
	// Initialise struct here - then assign entry 
	DynTxClient_t* dynClient = &activeClient[clientIndex];
	DynamicClientClear(dynClient);

	if(sourceFile[0]!='~')							// No delimiter "?" = 0
	{
		dynClient->entry = DYN_FIELD_ERR;
		return 0;
	} 
	if(sourceFile[1]=='~') 							// Double delimiter, skip 1	"~~" = 2
	{
		dynClient->entry = DYN_DOUBLE; 
		return 2;
	}

	for(len=2;len <= MAX_DYN_FIELD_ID_LEN-2;len++) 	// Start after "~-N-~"=N+2
	{
		if(sourceFile[len]=='~')break;
		if(len >= MAX_DYN_FIELD_ID_LEN)				// Too long, "~......." 
		{
			dynClient->entry = DYN_FIELD_ERR;
			return len+1;
		}
	}
	
	for(; *field==' '; field++)			// Skip white space
	{
		if(*field == '~')
		{
			dynClient->entry = DYN_FIELD_ERR;
			return len+1;				// All white space error
		}
	} 	

	if(*field == '/')
	{
		// Parse file name
		DBG_INFO("PARSE DYN_FILE\r\n");
		unsigned char i;
		for(i=0;i<MAX_DYN_FIELD_ID_LEN;i++)
		{
			if((field[i] == '~') || (field[i] == ' '))
			{
				field[i] = '\0';	// null-terminate string
				break;
			}
		}
		DynamicLoadFile(field, dynClient);
	}
	else if(*field == '?')
	{
		char negated = 0;
				
		id = atoi((char*)(field + 1)); 		// abusing atoi a bit- runs up to next non-numeric character
		if(id == 0)							// Can fail if negated as offset is wrong
			id = atoi((char*)(field + 2));
		
		DynamicLoadId(id, dynClient);
		
		DBG_INFO("PARSE DYN_COND id=%u, val=%u\r\n", id, *(unsigned char*)dynClient->entry->ptr);
		
		unsigned char i;
		for(i=0;i<MAX_DYN_FIELD_ID_LEN;i++)
		{
			if(field[i] == '!')
			{
				negated = TRUE;
			}
			else if(field[i] == ':')
			{
				if( (!negated && *(unsigned char*)dynClient->entry->ptr)
					|| (negated && ! *(unsigned char*)dynClient->entry->ptr) )
				{
					id = atoi((char*)(field + i + 1));
					DynamicLoadId(id, dynClient);
					break;
				}	
			}
			else if(field[i] == '~')
			{
				break;
			}
		}
		
		if(i == MAX_DYN_FIELD_ID_LEN) {	// Didn't find : and break out of loop
			DBG_INFO("DYN_COND MALFORMED\r\n");
			dynClient->entry = &dynFieldLenErr;
		}	
	}
	else
	{
		// Parse id number
		id = atoi(field);
		DynamicLoadId(id, dynClient);
	}
	
	// Return len
	return len+1;
}

unsigned short DynamicVarRead(unsigned char clientIndex, void* buffer, unsigned short maxLen)
{
	DynTxClient_t* dynClient = &activeClient[clientIndex];
	DynamicType_t type = dynClient->entry->type;
	unsigned short length = 0;
	unsigned char finished = TRUE;

	if(dynClient->entry == NULL) 
		return 0; // Not currently reading or closed

	if(maxLen < MIN_READ_BUFF_LEN) // <Min len file not found> + <dynamic var not found: id>
		type = DYN_LEN_ERR;

	switch(type) {
		case DYN_ERR: { // same as printf("some error on id: %u",id);
			length = sprintf(buffer,"!DYN_ERR:%d!",dynClient->entry->id);
			break; 
		}
		case DYN_TEXT: {
			length = strlen((char*)(dynClient->entry->ptr)) - dynClient->offset;
			if(maxLen < length)
			{
				finished = FALSE;
				length = maxLen;
			}
			else
			{
				finished = TRUE;
			}
			memcpy(buffer, dynClient->entry->ptr+dynClient->offset,length);
			dynClient->offset += length;
			break;
		}	
		case DYN_FILE: {
			
			if(dynClient->file == NULL) // Not found
			{
				length = sprintf(buffer,"!FILE ERR!");
				finished = TRUE;		// Send other tag next
				break;
			}
			else
			{
				unsigned long remaining = dynClient->fileSize - dynClient->offset;
				if(remaining == 0)
				{
					length = 0;
					finished = TRUE;	
					FSfclose(dynClient->file);
					break;
				}
				length = maxLen;
				if(remaining < (unsigned long)length)
					length = remaining;
				if(FSfread(buffer, length, 1, dynClient->file))
				{
					dynClient->offset += length;
					finished = FALSE; // More required
				}
				else
				{
					// Read error! Nothing we can do
					if(dynClient->file!=NULL)
						FSfclose(dynClient->file);
					finished = TRUE;
					length = 0;
				}
			}
			break;
		}
		case DYN_TIME: {
			if(dynClient->entry->ptr == NULL)
				length = sprintf(buffer,"%s",RtcToString(RtcNow()));
			else
				length = sprintf(buffer,"%s",RtcToString(*(DateTime*)dynClient->entry->ptr));
			break;
		}
		case DYN_STATE: {
			switch( *(state_t*)dynClient->entry->ptr )
			{
				case NOT_PRESENT:
					length = sprintf(buffer,"Not Present");
					break;
				case ERROR:
					length = sprintf(buffer,"Error");
					break;
				case STATE_OFF || OFF:
					length = sprintf(buffer,"Off");
					break;
				case STATE_ON || PRESENT:
					length = sprintf(buffer,"On");
					break;
				case INITIALISED:
					length = sprintf(buffer,"Initialised");
					break;
				case ACTIVE:
					length = sprintf(buffer,"Active");
					break;
				default:
					break;
			}	
			break;
		}
		case DYN_BOOL: {
			if( *(unsigned char*)dynClient->entry->ptr == 0 ) 
				length = sprintf(buffer,"No");
			else
				length = sprintf(buffer,"Yes");
			break;
		}
		case DYN_MODE: {
			if( *(unsigned char*)dynClient->entry->ptr == 0 )
				length = sprintf(buffer,"Text");
			else
				length = sprintf(buffer,"Binary");
			break;
		}	
		case DYN_UINT8:{
			length = sprintf(buffer,"%u",*(unsigned char*)dynClient->entry->ptr);
			break;
		}
		case DYN_UINT16:{
			length = sprintf(buffer,"%u",*(unsigned short*)dynClient->entry->ptr);
			break;
		}
		case DYN_UINT32: {
			length = sprintf(buffer,"%u",*(unsigned int*)dynClient->entry->ptr);	// "long" gives a compiler warning
			break;
		}
		case DYN_SINT16:{
			length = sprintf(buffer,"%d",*(signed short*)dynClient->entry->ptr);
			break;
		}
		case DYN_SINT32: {
			length = sprintf(buffer,"%d",*(signed int*)dynClient->entry->ptr);
			break;
		}
		case DYN_LEN_ERR:
		default : {
			if(maxLen > 9)
			{
				length = sprintf(buffer,"!DYN_ERR!"); // 
			}
			break;
		}
	} // Switch

	// Clear entry for this client
	if(length == 0 || finished == TRUE) 
		dynClient->entry = NULL; 

	return length;	
}

