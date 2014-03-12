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

// HTTP POST handler
// Samuel Finnigan, 2013-2014

#include "HTTPHandlePOST.h"

#include "TCPIPConfig.h"
#include "Settings.h"
#include "Utils/Util.h"

#define DEBUG_ON
#include "Common/debug.h"


POSTHandler_t postHandlers[] = { 
	{ POSTsettings, 	"/settings.html"},	// Post to /settings.html
	{ POSTlogin,		"/index.html"},		// Post to /index.html
	{ POSTerror,		""}					// Post to emptystring
};
#define POST_HANDLER_COUNT ( sizeof(postHandlers) / sizeof(postHandlers[0] )

/* Select function to execute */
unsigned char HandlePOST(char *buffer, unsigned short len, char *filePath)
{
	// Select correct handler based on file path
	unsigned short i;
	for(i=0; i < POST_HANDLER_COUNT ); i++)
	{
		if(strcmp(filePath, postHandlers[i].name) == 0)				// Found
		{
			DBG_INFO("POST MATCH %s, ptr: 0x0%X\r\n", postHandlers[i].name, postHandlers[i].functionPtr);
			
			if(postHandlers[i].functionPtr == NULL)
				return FALSE;	/// Error!
			
			return postHandlers[i].functionPtr(buffer, len);	// Call function ptr and return its status
		}
	}
	
	// Could call POSTerror here. Can't post to this address
	return FALSE;
}


/**
 * HANDLER FOR settings.html
 * 
 * Tokenize parameters and pass to function (pointer?)
 * Modifies Buffer!
 * 
 * Precondition: len < buffer length -1
 */
unsigned char POSTsettings(char *buffer, unsigned short len)
{
	char *keyPtr = buffer;				// Points to beginning of key
	
	DBG_INFO("POST SETTINGS\r\n");
	
	buffer[len+1] = NULL;						// Ensure end of buffer is NULL for strtok
	keyPtr = strtok(buffer, POST_DELIMETER);	// strtok replaces next & with NULL
	
	while (keyPtr != NULL)						// Next key
	{
		DBG_INFO("%s\r\n",keyPtr);
				
		// Handle setting
		if (strnicmp(keyPtr, "setting.", 8) == 0)	// This is a setting
		{ 
			if(! SettingsCommandSetting(keyPtr, SETTINGS_BATCH, 0) )	// DEBUG: SETTINGS_CDC will print over CDC port, SETTINGS_BATCH is no-reply
			{
				return FALSE;			// If error, return FALSE
			}
		}
		else
		{
			DBG_INFO("POST CMD UNRECOGNISED %s\r\n", keyPtr);
			return FALSE;				// Return FALSE
		}
		
		keyPtr = strtok(NULL, POST_DELIMETER);	// Next token (strtok stores internal state)
	}
	
	return TRUE;
}

/* Handle user logins */
unsigned char POSTlogin(char *buffer, unsigned short len)
{
	DBG_INFO("POST LOGIN\r\n");
	return 0;
}

/* Handle errors */
unsigned char POSTerror(char *buffer, unsigned short len)
{
	DBG_INFO("POST ERR\r\n");
	return 1;
}

