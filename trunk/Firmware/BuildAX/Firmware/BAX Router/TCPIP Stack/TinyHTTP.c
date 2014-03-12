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

// Tiny HTTP Server
// Samuel Finnigan, Karim Ladha, 2014

#include "TCPIPConfig.h"
#include "TCPIP Stack/TCPIP.h"
#include "TCPIP Stack/TcpIpTasks.h"
#include "TCPIP Stack/TinyHTTP.h"
#include "TCPIP Stack/HTTPHandlePOST.h"
#include "DynamicVars.h"

#define CONNECTION_TIMEOUT 20

//#define HTTP_SERVER_USER_MALLOC	// Enable/disable debug buffer

#define DEBUG_ON
#include "Common/debug.h"

#ifdef USE_FAT_FS
	#include "FatFs/FatFsIo.h"
#else
	#include "MDD File System/FSIO.h"
#endif

#ifdef STACK_USE_TINYHTTP_SERVER
	
	// Maximum number of simultanous connections accepted by the server.
	#ifndef MAX_HTTP_CONNECTIONS
		#define MAX_HTTP_CONNECTIONS (1u)
	#endif

	// Amount of data allocated to the client connections
	#ifndef HTTP_HEAD_BUF_LEN
		#define HTTP_HEAD_BUF_LEN (512ul)
	#endif

#ifndef HTTP_SERVER_USER_MALLOC
	char DebugBuffer[HTTP_HEAD_BUF_LEN];
#endif

	
	#ifndef HTTP_HEAD_BUF_INCREMENT
		#define HTTP_HEAD_BUF_INCREMENT (128u)
	#endif
	
	#ifndef HTTP_REQUEST_PATH_LEN
		#define HTTP_REQUEST_PATH_LEN (128u)
	#endif
	#ifndef HTTP_FILE_EXT_LEN
		#define HTTP_FILE_EXT_LEN (5)
	#endif
	
	#ifndef FSFREAD_LEN
		// TODO: Decrease this for better memory use and check read performance.
		#define FSFREAD_LEN (128u)	// bytes. 
	#endif
	
	#ifndef MDD_ROOT_DIR_PATH
		#define MDD_ROOT_DIR_PATH "/"
	#endif

	#ifndef INDEX_PATH
		#define INDEX_PATH "/index.html"
	#endif


	typedef enum
	{
		T_HTTP_GET			= 0u, 
		T_HTTP_POST			= 1u, 
		T_HTTP_PUT			= 2u, 
		T_HTTP_DELETE		= 3u, 
		T_HTTP_HEAD			= 4u, 
		T_HTTP_OPTIONS		= 5u, 
		T_HTTP_CONNECT		= 6u,
		T_HTTP_VERB_UNKNOWN	= 7u
	} HTTPVerb;	
	
 /****************************************************************************
  Section:
	Headers to search for.. MAX 16 using bit fields
  ***************************************************************************/
// Struct for each record
typedef struct {
	const unsigned short bitFlag;
	const char* header;
}HeaderSearch_t;

// Client header fields (must be single bits)
typedef enum {
	CRLFCRLF_F = 	0x0001,
	CONTENT_LEN_F = 0x0002,
	POST_F = 		0x0004,										
	GET_F = 		0x0008,
	PUT_F =			0x0010,
	DELETE_F = 		0x0020,
	HEAD_F = 		0x0040,
	CONNECT_F =		0x0080
}HeaderSearchFlags_t;

// Ordering unimportant
const HeaderSearch_t HeaderSearchRecord[] =
{
	{CRLFCRLF_F, 			"\r\n"},			/*Actually a CRLFCRLF since these are compares to line start*/										// 0 - 200 (GET/POST succeeded)
	{CONTENT_LEN_F, 		"Content-Length:"},																										// 1 - 302
	{POST_F, 				"POST"},										
	{GET_F, 				"GET"},
	{PUT_F,					"PUT"}, 
	{DELETE_F,				"DELETE"}, 
	{HEAD_F,				"HEAD"}, 
	{CONNECT_F,				"CONNECT"},
	{0,NULL}// List end										
};	
	
 /****************************************************************************
  Section:
	Commands and Data Structures (including code from HTTP2_MDD.c)
  ***************************************************************************/
	// Initial response strings (Corresponding to HTTP_STATUS)
	const char *HTTPResponseHeaders[] =
	{
		"HTTP/1.0 200 OK\r\nConnection: close\r\n\r\n",																							// 0 - 200 (GET/POST succeeded)
		"HTTP/1.0 302 Found\r\nLocation: ",																										// 1 - 302
		"HTTP/1.0 400 Bad Request\r\nConnection: close\r\n\r\n<html><body><h1>400: Bad Request</h1>\r\n",										// 2 - 400
		"HTTP/1.0 403 Forbidden\r\nConnection: close\r\n\r\n<html><body><h1>403: Forbidden</h1>\r\n",											// 3 - 403
		"HTTP/1.0 404 Not found\r\nConnection: close\r\n\r\n<html><body><h1>404: File not found</h1>\r\n",										// 4 - 404
		"HTTP/1.0 414 Request-URI Too Long\r\nConnection: close\r\n\r\n<html><body><h1>414: Request-URI Too Long (Buffer overflow)</h1>\r\n",	// 5 - 414
		"HTTP/1.0 500 Internal Server Error\r\nConnection: close\r\n\r\n<html><body><h1>500: Internal Server Error</h1>\r\n",					// 6 - 500
		"HTTP/1.0 501 Not Implemented\r\nConnection: close\r\n\r\n<html><body><h1>501: Not Implemented</h1>\r\n"								// 7 - 501
	};
	
	//Supported Commands and Server Response Codes
	typedef enum
	{
	    T_HTTP_OK 				= 0u,		// 200 OK
		T_HTTP_REDIRECT 		= 1u,		// 302 Redirect
	    T_HTTP_BAD_REQUEST		= 2u,		// 400 Bad Request
	    T_HTTP_FORBIDDEN		= 3u,		// 403 Forbidden
	    T_HTTP_NOT_FOUND		= 4u,		// 404 Not Found
		T_HTTP_OVERFLOW			= 5u,		// 414 Request-URI Too Long
		T_HTTP_INTERNAL_ERROR 	= 6u,		// 500 Internal Server Error
		T_HTTP_NOT_IMPLEMENTED	= 7u,		// 501 Not Implemented (not a GET or POST command)
		T_HTTP_UNKNOWN			= 8u,		// State of this field before browser header is parsed
	} HTTPRetCode;
	
	/* State of the HTTP/TCP connection */
	typedef enum 
	{
		STATE_WAITING,
		STATE_LISTEN,
		STATE_CONNECTED,
		STATE_PARSE_VERB,
		STATE_FIND_PATH_GET,
		STATE_FIND_PATH_POST,
		STATE_POST_RECEIVE,					// Receive extra data for POST
		STATE_DISCARD,
		STATE_TRANSMIT_RESULT,
		STATE_TRANSMIT_FILE,
		STATE_TRANSMIT_DYN,
        STATE_CLOSE
	} HTTPServerState;


	/* Represent the state of a single client connection */
	typedef struct 
	{
		// State machine
		TCP_SOCKET 		 socket;
		HTTPServerState  state;
		HTTPRetCode 	 retCode;
		HTTPVerb		 verb;
		DWORD			 timer;				// For timeouts	
	
		// Buffer
		char 			*buffer;			// Pointer to clients buffer
		unsigned short	 bufferLen;			// Length of allocation
		unsigned short	 bufferIndex;		// Total bytes in the client buffer
		unsigned short	 bufferLast;		// For Tx, last data index in buffer

		// RX
		unsigned short	 postStart;			// Start in buffer of post message client->buffer[postStart]
		unsigned short	 postLen;			// Set by content length header
		// TX
		unsigned char	 readNewSector;
		FSFILE			*fileHandle;
		unsigned long	 fileSize;
		unsigned long	 sentBytes;	
		char 			 filePath[HTTP_REQUEST_PATH_LEN];
		char			 fileExt[HTTP_FILE_EXT_LEN];
		// TX dynamic
		unsigned short	 dynFieldOffset;
		unsigned short 	 dynFieldLen;
	} HTTPClient;
	
	
	/* Allocate simultaneous client socket connections */
	static HTTPClient clients[MAX_HTTP_CONNECTIONS];
	
	/*
	*	Clear a client connection
	*/
	void HTTPClientClearState(HTTPClient *client)
	{
		client->state = STATE_LISTEN;
		client->retCode = T_HTTP_UNKNOWN;
		client->verb = T_HTTP_VERB_UNKNOWN;
		client->timer = 0xffffffff;
		// Buffer
		client->bufferIndex = 0;	
		client->bufferLast = 0;	
		// Post field
		client->postStart = 0;			
		client->postLen = 0;			
		// Get file
		client->readNewSector = FALSE;
		client->fileHandle = NULL;
		client->fileSize = 0;
		client->sentBytes = 0;	
		client->dynFieldOffset = 0;
		client->dynFieldLen = 0;
		memset(client->filePath,0,HTTP_REQUEST_PATH_LEN);
		memset(client->fileExt,0,HTTP_FILE_EXT_LEN);
	}
	/*
	* Close a client
	*/
	void HTTPClientClose(HTTPClient *client)
	{
		if(client->socket != INVALID_SOCKET)
		{
			TCPClose(client->socket);
			client->socket = INVALID_SOCKET;
		}
		#ifdef HTTP_SERVER_USER_MALLOC
		if(client->buffer)
			free(client->buffer);
		#endif
		client->state = STATE_WAITING;
		client->buffer = NULL;			
		client->bufferLen = 0;	
		HTTPClientClearState(client);	
	}
	
	/* 
	 * Initialize states and sockets 
	 */
	static unsigned short alreadyInited = FALSE;
	void HTTPInit(void) 
	{
		BYTE i;
		if(alreadyInited == FALSE)
		{
			DBG_INFO("TinyHTTP INIT\r\n");
			for(i=0; i<MAX_HTTP_CONNECTIONS; i++)
			{
				clients[i].socket = INVALID_SOCKET;
				clients[i].buffer = NULL;			
				clients[i].bufferLen = 0;		
				HTTPClientClearState(&clients[i]);
			}
			alreadyInited = TRUE;
		}
	
	}
	void HTTPDeInit(void) {
		BYTE i;
		DBG_INFO("TinyHTTP INIT\r\n");
		for(i=0; i<MAX_HTTP_CONNECTIONS; i++)
		{
			HTTPClientClose(&clients[i]);
		}
		alreadyInited = FALSE;
	}

	/*****************************************************************************
	 Function:
	 	signed char PutHTTPError(TCP_SOCKET sock, HTTPRetCode returnCode)
	 
	 Summary:
	 	Write a given HTTP Error code to the TCP TX Buffer
	 
	 Parameters:
	 	TCP_SOCKET sock			- The server socket to write the error over
	 	HTTPRetCode returnCode	- The HTTP return code to write
	 	
	 Returns:
	 	0 if successful
	 	-1 if there was not enough space in the TX buffer
	 *****************************************************************************/
	signed char PutHTTPError(HTTPClient *client, HTTPRetCode returnCode)
	{	// NB: strlen works by iterating through the string until 0x0 encountered.
		// TODO: Replace strlen() call with stored field length
		if(TCPIsPutReady(client->socket) < (strlen(HTTPResponseHeaders[returnCode])) )
			return -1;

		TCPPutString(client->socket, (BYTE*)HTTPResponseHeaders[returnCode]);
				
		TCPPutString(client->socket, (BYTE*)"<pre>Received ");
		
		char str[4];
		snprintf(str, 4, "%u", client->bufferIndex);
		TCPPutString(client->socket, (BYTE*)str);
		
		TCPPutString(client->socket, (BYTE*)" bytes of request:\r\n\r\n");
		
		client->buffer[client->bufferIndex -1] = '\0';
		TCPPutString(client->socket, (BYTE*)client->buffer);
		TCPPutString(client->socket, (BYTE*)"</pre>\r\n</body></html>");
		return 0;
	}
	
	
	/*****************************************************************************
	 Function:
	 	unsigned short FillBuffer(HTTPClient *client)
	 
	 Summary:
	 	Fill the client's local buffer from the TCP RX Buffer.
	 	Note that this does not empty the RX buffer, only copies as much data from
	 	it as we have available space in the local buffer.
	 
	 Parameters:
	 	HTTPClient *client	- The client to service
	 	
	 Returns:
	 	bytes successfuly read from the RX buffer 
	 	0 if read unsuccessful (eg: local buffer full, RX buffer became empty, or client disconnected)
	 *****************************************************************************/
	inline unsigned short FillBuffer(HTTPClient *client)
	{
		// Read as much as possible
		unsigned short space = (HTTP_HEAD_BUF_LEN - client->bufferIndex);
		if(space>0)
		{
			space = TCPGetArray(client->socket, (unsigned char*)&client->buffer[client->bufferIndex], space);
			//DBG_INFO("\r\n<%u BYTES><START>\r\n",space);
			//DBG_WRITE((unsigned char*)&client->buffer[client->bufferIndex],space);
			//DBG_INFO("\r\n<END>\r\n");
			client->bufferIndex += space;
			return space;
		}
		return 0;
	}
	
	/*****************************************************************************
	 Function:
	 	unsigned short parsePath(HTTPClient *client, unsigned char offset, char *path)
	 
	 Summary:
	 	Parse out the requested file path from the first line of a HTTP request
	 
	 Parameters:
	 	HTTPClient *client		- The client requesting the file
	 	unsigned short offset 	- Expected offset of address in the request buffer
	 	char *path				- Memory to store path. Ensure this doesn't overflow!
	 	
	 Returns:
	 	Length of path read before delimeter hit
	 	0xffff if overflowed HTTP_REQUEST_PATH_LEN
	 	0 if error
	 *****************************************************************************/	 
	unsigned short CopyRequestPath(HTTPClient *client, unsigned short offset)
	{
		char delimeter = ' ',copyExt =FALSE;// Space delimits filename in string
		char extension = '.',extOff = 0;	// Start of extension	
		char *filePathPtr;					// Pointer to filename offset in client buffer
		unsigned short j;
		
		filePathPtr = client->buffer + offset;
		for(j=0; j < client->bufferIndex - offset; j++)
		{	
			if( j > HTTP_REQUEST_PATH_LEN )
				return 0xffff;			 		// Buffer overflow error
				
			if(filePathPtr[j] == delimeter)		// Loop exits when hit either end of array or delimeter
			{
				client->filePath[j] = '\0';		// Null-terminate string
				return j;						// return its length
			}
			
			if(copyExt)							// Copy extension + add null
			{
				unsigned char index = j-extOff;
				if(index<HTTP_FILE_EXT_LEN-1)
				{
					client->fileExt[index] = filePathPtr[j];
					client->fileExt[index+1] = '\0';
				}
			}	

			client->filePath[j] = filePathPtr[j];	// Copy character into path buffer

			if(filePathPtr[j] == extension)			// Reset ext on every '.'
			{
				copyExt = TRUE;
				extOff = j+1;
				client->fileExt[0] = '\0';
			}
		}
		
		return 0;								// Error
	}
	
	/*****************************************************************************
	  Function:
		unsigned short FindCRLF(char *buffer, unsigned short end)
	
	  Summary:
		finds a LF, and its CR pair if one exists
	
	  Description:
		Used to find end of line in buffer. Works for both Win and Unix platforms.
	
	  Precondition:
		none.
	
	  Parameters:
		buffer to search in.
		end of buffer as an integer number of bytes
	
	  Returns:
	  	0 - first characters in buffer are \r\n
	  	n - buffer[n] == \r, then \n
	  	0xffff - \r\n not encountered
	  ***************************************************************************/
	unsigned short FindCRLF(char *buffer, unsigned short end)
	{
		unsigned short offset;
		for(offset=0;offset<end;offset++)
		{
			if(buffer[offset] == '\n')
			{
				if(buffer[offset-1] == '\r')
				{
					return(offset-1);
				}
				else
				{
					return offset;	// Tolerate UNIX platforms where no CR is sent
				}
			}		
		}
		return 0xffff;
	}
	
	/*****************************************************************************
	  Function:
		unsigned short ParseHeaders(char* buffer, unsigned short end, unsigned short* headerFlags)
	
	  Summary:
		finds a header using flags field as a map, continues until it hits end of finds one
		returns the offset of the header (first byte of word) and sets flags field to represent 
		what it found. If it hits end, it returns the ammount scanned and flags is cleared.
	
	  Description:
		Used to find parse headers in requests
	
	  Precondition:
		none.
	
	  Parameters:
		buffer to search in, first byte treated as start of a line!
		end last INDEX of buffer as an integer number of bytes
		headerFlags, the bit map of headers its looking for
	
	  Returns: 
		index of first char of the first header it found
	  	0 - first characters in buffer are header
	  	n - buffer[n] == first byte of header or ammount of buffer that can be discarded
	  	0xffff - header not encountered and partial header not encountered end
		
		headerFlags = 0, no header found or partial header
		headerFlags = 0b00000X0000000, X was found 
		
	  ***************************************************************************/
	unsigned short ParseHeaders(char* buffer, unsigned short end, unsigned short* headerFlags)
	{
		unsigned short start,index;
		char* last = &buffer[end]; 	// Last value in range
		for(start=0;start<end;)
		{
			for(index=0;;index++) 	// Search
			{
				if(HeaderSearchRecord[index].header == NULL)break; 	// End of header list 
				if(*headerFlags & HeaderSearchRecord[index].bitFlag)// We are looking for this header
				{
					const char* ptr1 = HeaderSearchRecord[index].header, *ptr2 = &buffer[start];
					for(;;)
					{
						if(! *ptr1 || ! *ptr2)	// If either pointer is null, break
							break;
						if(*ptr1++ != *ptr2++)	// If either pointer not equals the other
							break;	
						if(ptr2 > last)			// Past search limit and not fully matched
						{	
							*headerFlags = 0; 	// Partial match
							return (start);		// Ammount read upto the last line
						}
					}
					if(*ptr1=='\0') // Found!
					{	
						*headerFlags = HeaderSearchRecord[index].bitFlag;
						return start;
					}
				}
			} // End of compare line
			// Get next CRLF
			start += FindCRLF(buffer+start, end-start);
			// Skip over CRLF
			start+=2;
		}
		*headerFlags = 0;	// Not found
		return 0xFFFF; 		// Not found, discard all buffer
	}

	signed char PathIsDirectory(char *path)
	{
		if( FSchdir(path) != -1 )
		{
			FSchdir("\\");
			return 0;
		}
		
		return -1;
	}
	
	unsigned char PathLenCheck(HTTPClient *client, unsigned short filePathLen) 
	{
		if( filePathLen > 0 && filePathLen <= HTTP_REQUEST_PATH_LEN )
		{
			return TRUE;
		}	
		else
		{
			if(filePathLen > HTTP_REQUEST_PATH_LEN)
			{
				DBG_INFO("OVERFLOW\r\n");
				client->retCode = T_HTTP_OVERFLOW;					// Request string too long, didn't find the next delimeter
				return FALSE;
			}
			else
			{
				DBG_INFO("BADREQ\r\n");
				client->retCode = T_HTTP_BAD_REQUEST;				// No correctly positioned path found
				return FALSE;
			}
		}
	}	
	
	inline unsigned long GetFilesize(FSFILE *fileHandle)
	{
		#ifdef USE_FAT_FS
			return (unsigned long)(fileHandle->fsize);
		#else
			return (unsigned long)(fileHandle->size);
		#endif
	}					

	void TryOpenFile(HTTPClient *client)
	{
		client->fileHandle = FSfopen(client->filePath, "r");	// Open file pointer in read mode and store on *client
		
		if(!client->fileHandle)
		{
			DBG_INFO("NOT FOUND\r\n");
			client->retCode = T_HTTP_NOT_FOUND;					// File does not exist (or failed to read)
		
		}
		else
		{
			client->fileSize = FSFileSize(client->fileHandle);	// Get size of file
			client->retCode = T_HTTP_OK;						// Set Return code to 200 OK
			DBG_INFO("name: %s, ",client->filePath);
			DBG_INFO("type: %s, ",client->fileExt);
			DBG_INFO("size: %lu",client->fileSize);
			DBG_INFO("\r\n");
		}	
	}
	
	/*****************************************************************************
	  Function:
		void HTTPServer(void)
	
	  Summary:
		A simple HTTP Server.
	
	  Description:
		Based on Microchip's GenericTCPServer and TelnetLocal examples
	
	  Precondition:
		TCP is initialized.
	
	  Parameters:
		None
	
	  Returns:
	  	None
	  ***************************************************************************/
	void HTTPServer(void)
	{
		/* Iterate through the HTTP clients */
		unsigned char clientIndex;
		for(clientIndex=0; clientIndex<MAX_HTTP_CONNECTIONS;clientIndex++)
		{
			HTTPClient *client = &clients[clientIndex];

			for(;;)
			{
				
				// NO fall throughs, check state each time
				if(client->socket == INVALID_SOCKET)  	// Invalid, waiting
					client->state = STATE_WAITING;		// Continue to get socket
				else if (TCPWasReset(client->socket)) 	// Valid and reset, goto listening
				{
					DBG_INFO("RESET\r\n");
					HTTPClientClearState(client);
				}
				else if((!TCPIsConnected(client->socket)) &&
						(client->state != STATE_LISTEN))	// Was connected, not now
				{
					DBG_INFO("DISCONNECT\r\n");
					TCPDiscard(client->socket);		 		// Discard old data
					HTTPClientClearState(client);
				}
				
				

				// Handle client based on its state
				switch(client->state)
				{
					case STATE_WAITING: // On start-tp and reset
					{	
						// Get a socket
						if(client->socket == INVALID_SOCKET)
							{client->socket = TCPOpen(0, TCP_OPEN_SERVER, HTTP_PORT, TCP_PURPOSE_HTTP_SERVER);}
						if(client->socket == INVALID_SOCKET)					
							break;
						// If client is unconfigured - assign buffer
						if(client->buffer == NULL)
						{
							// Waiting on config
							DBG_INFO("WAITING\r\n");
							// First connection / new socket
							HTTPClientClearState(client);
							client->bufferLen = HTTP_HEAD_BUF_LEN;
						
							#ifdef HTTP_SERVER_USER_MALLOC
							client->buffer = malloc(client->bufferLen);
							if(client->buffer == NULL)
							{
								DBG_INFO("!ASSERT\r\n");
								DBG_ERROR("TinyHTTP.c malloc failure client buffer\r\n");
								break;					// malloc failed.
							}
							#else
							client->buffer = DebugBuffer;
							#endif
						}
									
						// Allocate a socket for this server to listen and accept connections on
						// Abort operation if no TCP socket of type TCP_PURPOSE_HTTP_SERVER is available
						// If this ever happens, you need to go add one to TCPIPConfig.h

						DBG_INFO("LISTENING\r\n");
						client->state = STATE_LISTEN;	
						
					}	// Fall through to STATE_LISTEN
					case STATE_LISTEN:
					{
						if(!TCPIsConnected(client->socket))
							break;
						DBG_INFO("CONNECTED\r\n");
						client->state = STATE_CONNECTED;
						continue;
					}					

					case STATE_CONNECTED:
					{
						unsigned short offsetCR;
						FillBuffer(client); // Get more data from socket

						// Wait for client to send request (greater than zero bytes RX)
						// This may be 0 when RX buffer first read (eg: client has connected 
						//	but we have not yet received the request packet over TCPIP)
						if(client->bufferIndex)
						{
							// We currently only care about the first line of the request, so see if we have all of it:
							// Search len bytes into the array starting at the offset already read
							offsetCR = FindCRLF(client->buffer, client->bufferIndex);
							
							if(offsetCR != 0xffff)				// i.e. found CRLF in this TCP packet
							{
								client->state = STATE_PARSE_VERB;
								continue;
							}
							else if(client->bufferIndex >= HTTP_HEAD_BUF_LEN)
							{
								DBG_INFO("OVERFLOW\r\n");
								client->state = STATE_DISCARD;
								client->retCode = T_HTTP_OVERFLOW;
								break;
							}
						}
						break;
					}	
		
					case STATE_PARSE_VERB:
					{
						DBG_INFO("VERB:");
						FillBuffer(client); // Get more data from socket

						// If we already have a return code due to error, don't parse.
					  	if(client->retCode != T_HTTP_UNKNOWN)
					  	{
							DBG_INFO("->TX\r\n");
						  	client->state = STATE_TRANSMIT_RESULT;
						  	continue;
						}
					
						unsigned short headerFlags = (0xFFFF); // Parse all verbs in table
						ParseHeaders(client->buffer,5,&headerFlags);
						client->timer = TickGet(); // Request time

						if(headerFlags & GET_F)
						{
							DBG_INFO("GET\r\n");
							client->state = STATE_FIND_PATH_GET;
							continue;
						}
						else if(headerFlags & POST_F)
						{
							DBG_INFO("POST\r\n");
							DBG_INFO("POST DATA:\r\n");	
							client->state = STATE_FIND_PATH_POST;		// Find end of post
							client->postStart = 0;						// Start of buffer
							client->postLen = 0;
							continue;
						}
						else if(headerFlags)	// Some other valid verb or not found
						{	
							DBG_INFO("UNSUPPORTED REQ %02X\r\n",headerFlags);
							client->state = STATE_DISCARD;
							client->retCode = T_HTTP_NOT_IMPLEMENTED;	// 501
						}
						else
						{
							DBG_INFO("UNKNOWN REQ\r\n");
							client->state = STATE_DISCARD;
							client->retCode = T_HTTP_BAD_REQUEST;		// 400
						}
						break;		
					}
		
					case STATE_FIND_PATH_GET:
					{
						unsigned char offset = 4;									// GET<space> is 4 characters - offset of URL is 4 chars into buffer.				
						unsigned short filePathLen;									// Length of filename in client buffer
						
						DBG_INFO("GET PATH:");
						FillBuffer(client); // Get more data from socket

						filePathLen = CopyRequestPath(client, offset);				// Copy path into client->filePath
						
						if( PathLenCheck(client, filePathLen) )						// Does the path look valid?
						{
							if(client->filePath[filePathLen-1] == '/')
								client->filePath[filePathLen-1] = '\0'; 			// Trim trailing slash if exists (and not root)
							
							if( PathIsDirectory(client->filePath) != -1 )			// Returns 0 on success
							{
								DBG_INFO("DIR\r\n");
								client->retCode = T_HTTP_REDIRECT;					// Path is a directory. 302 Redirect.
								client->state = STATE_DISCARD;
								continue;
							}
							
							TryOpenFile(client);
						}
						
						client->state = STATE_DISCARD;
						continue;
					}
		
					case STATE_FIND_PATH_POST:
					{
						unsigned char offset = 5;		// POST<space> is 5 characters - offset of URL is 4 chars into buffer.				
						unsigned short filePathLen;		// Length of filename in client buffer
						
						FillBuffer(client); 			// Get more data from socket
						
						// Dump all
						//DBG_WRITE(&client->buffer[client->postStart],(client->bufferIndex - client->postStart));
								
						// Receiving headers
						if(client->bufferIndex)
						{
							// Find path
							if(client->filePath[0] == NULL)						// works because we memset filePath to 0
							{
								filePathLen = CopyRequestPath(client, offset);	// Copy path
								if( ! PathLenCheck(client, filePathLen) )		// overflow or bad req?
								{
									client->state = STATE_DISCARD;
									continue;
								}
								//else path looks like a valid path
							}
							
							for(;;) {
								// What we are looking for in the buffer
								unsigned short flags = (CRLFCRLF_F | CONTENT_LEN_F);
								unsigned short offset = ParseHeaders(&client->buffer[client->postStart], (client->bufferIndex - client->postStart - 1), &flags);
								// See what we found
								if(flags & CONTENT_LEN_F) 	// Found content length header
								{
									unsigned short headerLen = strlen("Content-Length:");
									client->postStart += (offset + headerLen);// Field after content length
									client->postLen = atoi(&client->buffer[client->postStart]);
									//DBG_INFO("CON-LEN:%u->",client->postLen);
									continue;
								}
								else if(flags & CRLFCRLF_F)	// CRLFCRLF first
								{
									unsigned short headerLen = strlen("\r\n");
									client->postStart += (offset + headerLen);// Start of the POST field
									//DBG_INFO("CRLFCRLF->");
									client->state = STATE_POST_RECEIVE;
									break;
								}
								else if(offset == 0xFFFF)	// Can discard this whole buffer
								{
									//DBG_INFO("DISCARD->");
									client->bufferIndex = 0;
									break; // Return when theres new bytes
								}
								else						// Partial header found at end, worst case
								{
									//DBG_INFO("MOVE:%u->",(client->bufferIndex-offset));
									memmove(client->buffer,&client->buffer[offset],(client->bufferIndex-offset));
									client->bufferIndex = (client->bufferIndex-offset);
									client->postStart = 0;
									break; // Return when there's new bytes
								}
							}// Just to allow continue after content len

							if(client->state == STATE_POST_RECEIVE)
								continue;
						}// End if there are bytes
						break;	
					}	
					case STATE_POST_RECEIVE:
					{
						FillBuffer(client);						// Get more data from socket
						
						if(client->bufferIndex)					// If we have received bytes
						{		
							// Do we have the whole post?
							if((client->bufferIndex - client->postStart ) >= client->postLen)
							{
								DBG_INFO("POST RX: Content-Length: %u\r\n",client->postLen);
								DBG_WRITE(&client->buffer[client->postStart],client->postLen);
								DBG_INFO("\r\n");
								
								DBG_INFO("PATH: %s\r\n", client->filePath);
								// We now have all the data in our buffer
								if( HandlePOST(client->buffer, client->postLen, client->filePath) )	// HANDLE POST
								{
									client->retCode = T_HTTP_OK;
									TryOpenFile(client);
								} 
								else 
								{
									client->retCode = T_HTTP_INTERNAL_ERROR;
								}
								
								client->state = STATE_DISCARD;
								continue;
							}
							else if(client->postLen + client->bufferIndex > HTTP_HEAD_BUF_LEN)	
							{	
								// POST body overlaps boundary
								DBG_INFO("MOVING %ub\r\n",(client->bufferIndex - client->postStart));
								memmove(client->buffer, &client->buffer[client->postStart], client->bufferIndex);
								client->bufferIndex = client->bufferIndex - client->postStart;
								client->postStart = 0;
								
								break; // Return when there's new bytes
							}
							else if(client->postLen > HTTP_HEAD_BUF_LEN)	// OVERFLOW CONDITION
							{
								DBG_INFO("POST RX %ub TOO LONG!\r\n", client->postLen);
								
								client->retCode = T_HTTP_INTERNAL_ERROR;	// Not enough memory for POST body
								client->state = STATE_DISCARD;
							}
						}
						break;
					}
	
					case STATE_DISCARD:
					{
						TCPDiscard(client->socket);
						client->state = STATE_TRANSMIT_RESULT;
						continue;
					}
		
					case STATE_TRANSMIT_RESULT:
					{
						DBG_INFO("\r\nRESPONSE:");
						switch(client->retCode) 
						{
							case T_HTTP_OK:
							{
								if(client->sentBytes == 0)
								{
									DBG_INFO((char*)HTTPResponseHeaders[T_HTTP_OK]);
									TCPPutString(client->socket, (BYTE*)HTTPResponseHeaders[T_HTTP_OK]);	// Output HTTP headers on first pass
								}
								else
								{
									// This should have been zero
									DBG_INFO("TX ERR?\r\n");
								}
								client->sentBytes = 0;			// Zero len
								client->bufferIndex = 0; 		// Read the current location
								client->bufferLast = 0;			// End of data (buffer[last] != data)
								client->readNewSector = TRUE;	// Read a new sector
								client->dynFieldLen = 0;		// No dynamic data to send
								client->state = STATE_TRANSMIT_FILE;
								continue;
							}
							case T_HTTP_REDIRECT:
							{
								DBG_INFO("REDIRECT\r\n");
	
								DBG_INFO(HTTPResponseHeaders[T_HTTP_REDIRECT]);
								TCPPutString(client->socket, (BYTE*)HTTPResponseHeaders[T_HTTP_REDIRECT]);
								DBG_INFO(client->filePath);
								TCPPutString(client->socket, (BYTE*)client->filePath);
								
								if(client->filePath[strlen(client->filePath)-1] == '/')
								{
									DBG_INFO((char*)(INDEX_PATH+1));
									TCPPutString(client->socket, (BYTE*)INDEX_PATH+1);
								}
								else
								{
									DBG_INFO((char*)(INDEX_PATH));
									TCPPutString(client->socket, (BYTE*)INDEX_PATH);
								}
									
								DBG_INFO("\r\nConnection: close\r\n\r\n");
								TCPPutString(client->socket, (BYTE*)"\r\nConnection: close\r\n\r\n");
								
								client->state = STATE_CLOSE;
								break;
							}	
							case T_HTTP_UNKNOWN:
							{
								DBG_INFO("UNKNOWN\r\n");
								PutHTTPError(client, T_HTTP_INTERNAL_ERROR);	// 500 Internal Server Error for unknown
								client->state = STATE_CLOSE;
								break;
							}	
							default:
							{
								DBG_INFO("DEFAULT\r\n");
								PutHTTPError(client, client->retCode);			// Else write out correct return msg
								client->state = STATE_CLOSE;
								break;
							}
						}
						
						break;
					}

					case STATE_TRANSMIT_FILE:
					{
						/*	KL. Since we are not listening now and we dont need the last 
							query data, we can use our RX buffer for TX service too.*/
						unsigned short toSend = TCPIsPutReady(client->socket);	// Make certain the socket can be written to
						unsigned long  remaining = client->fileSize - client->sentBytes;
						unsigned short leftInSector = remaining & (MEDIA_SECTOR_SIZE-1);
						
						// Discard extra incoming 
						TCPDiscard(client->socket);	
		
						if(toSend == 0)
							break;	

						// Read file if we need to (always maximum length)
						if(client->readNewSector) 			// Get next data chunk
						{
							client->readNewSector = FALSE;			// Set when current data is sent
							if(client->bufferIndex > 0) 			// Currently in the file, re-read this section
							{
								if (client->bufferIndex == client->bufferLast) // Normal read
								{
									//DBG_INFO("NEXT SECTOR\r\n");
									client->bufferIndex = 0;
								}
								else if(client->bufferIndex > client->bufferLast)// Outside this section, read next one, filptr ok
								{
									DBG_INFO("READ-NEXT\r\n");				
									client->bufferIndex -= client->bufferLast;
									leftInSector = 0; // Dis-allow sector alignment
								}
								else 											// Set back to start of this section, filptr-=last read
								{
									DBG_INFO("RE-READ SECTOR\r\n");
									FSfseek(client->fileHandle,-((signed long)client->bufferLast),SEEK_CUR);
									remaining +=  client->bufferIndex; 			// Adjust remaining count previous send
									leftInSector = 0; // Dis-allow sector alignment									
								}
							}
							else
							{
								DBG_INFO("START OF FILE\r\n");
							}

							// Find out how much to read to our buffer
							if(remaining > (unsigned long)client->bufferLen)	
							{
								client->bufferLast = client->bufferLen;
								if(leftInSector) // Re-allign sector reads where possible
								{
									client->bufferLast = leftInSector;
									if(client->bufferIndex != 0)// Check for fail 
									{
										DBG_ERROR("ASSERT!\r\n");
									}
								}
							}
							else 												
							{		
								client->bufferLast = remaining;
								DBG_INFO("FILE END\r\n");
							}

							if(!FSfread(client->buffer, client->bufferLast, 1, client->fileHandle))
							{
								DBG_INFO("FREAD ERR\r\n");
								client->state = STATE_CLOSE;
								break;
							}
						} // Get new sector

						{
							//----------------------------------------------------------------------------
							// Scan for dynamics if html 
							if(strncmp(client->fileExt,"html",4) == 0)
							{
								DBG_INFO("Scanning for dynamics: %s\r\n",client->filePath);
								char* ptr = client->buffer + client->bufferIndex; 
								unsigned short offset = 0, inBuffer = client->bufferLast - client->bufferIndex;
								for(;offset<inBuffer;offset++)
								{
									if(*ptr++ == '~')
									{
										DBG_INFO("Found offset: %lu\r\n",(unsigned long)offset+client->sentBytes);
										// Set buffer offset: buffer[offset] = '~'
										client->dynFieldOffset = offset + client->bufferIndex; 
										client->dynFieldLen = 0xffff; // Used as flag in this state (i.e. len>0)
										break;
									}		
								}
							}

							// See how much we can send from the buffer
							if((client->bufferLast - client->bufferIndex) < toSend)	// Left in buffer < outbuffer
								toSend = (client->bufferLast - client->bufferIndex);

							if(client->dynFieldLen)									// For dynamic support, send upto field
							{
								if((client->dynFieldOffset - client->bufferIndex) < toSend)
									toSend = (client->dynFieldOffset - client->bufferIndex);
							}

							// Tx chunk
							if(toSend) 
								toSend = TCPPutArray(client->socket, (unsigned char*)(client->buffer+client->bufferIndex), toSend);
							
							#if 0
							DBG_WRITE((char*)(client->buffer+client->bufferIndex),toSend);
							#endif
							
							// Update 
							client->sentBytes += toSend;
							client->bufferIndex += toSend;

							// If sent upto start of a dynamic field - append field
							if(client->dynFieldLen && (client->bufferIndex == client->dynFieldOffset))
							{
								char dynField[MAX_DYN_FIELD_ID_LEN];
								char* fileField = &client->buffer[client->bufferIndex];
								
								// Edge condition, may not be the second '~' in buffer
								signed short leftInBuffer = client->bufferLast - client->bufferIndex;
								unsigned short fieldLen = 0;
								
								if(leftInBuffer >= MAX_DYN_FIELD_ID_LEN)				// Enough in ram buffer to copy out
								{
									DBG_INFO("DYN SCAN RAM\r\n");
									memcpy(dynField,fileField,MAX_DYN_FIELD_ID_LEN);
								}
								else													// Not enough, copy from file
								{	
									DBG_INFO("DYN SCAN FILE\r\n");
									signed short toRead = MAX_DYN_FIELD_ID_LEN-leftInBuffer;			// Required extra bytes to make valid search len	
									
									if(remaining<toRead)
										toRead = remaining;												// If not enough, truncate up to EOF
									
									memset(dynField,0,MAX_DYN_FIELD_ID_LEN);							// Clear buffer to null (supress old data)
									FSfseek(client->fileHandle,-leftInBuffer,SEEK_CUR); 				// Roll back file to the first "~"
									
									if(!FSfread(dynField,(leftInBuffer+toRead), 1, client->fileHandle))	// May be outside the file, dont read past EOF
									{
										DBG_INFO("FREAD ERR DYN\r\n");
										client->state = STATE_CLOSE;
									}
									
									FSfseek(client->fileHandle,(-toRead),SEEK_CUR);		// Restore file pointer location to EOF/buffer end
								}
								
								// Get field length
								fieldLen = DynamicVarLoad(dynField,clientIndex);		// Find length of "~XXXXXX~"
								if(fieldLen) 											// Found id (any pair on non adjacent '~')
								{
									DBG_INFO("DYN FOUND (field len: %u)\r\n", fieldLen);
									client->state = STATE_TRANSMIT_DYN; 				// Send this out first
								}
								
								// Clear the states 
								client->dynFieldOffset = 0;								// Clear dyn offset
								client->dynFieldLen = 0;								// Clear len/flag
								client->bufferIndex+=fieldLen; 							// Resume sending file after second '~'
								client->sentBytes+=fieldLen;							// Should be field len + ~ ~123~ would be +5
								continue;
							}

							if(client->bufferIndex >= client->bufferLast)				// Sent upto or past buffer length
								client->readNewSector = TRUE;							// bufferIndex = bufferLen

							if(client->sentBytes < client->fileSize)					// Sent full file - done
								continue;
						}

						DBG_INFO("FILE DONE\r\n");
						if(client->fileHandle)
							FSfclose(client->fileHandle);
						client->fileHandle = NULL;

						client->state = STATE_CLOSE;
						continue;
					}  	
					case STATE_TRANSMIT_DYN:
					{
						unsigned short toSend = TCPIsPutReady(client->socket);	// Make certain the socket can be written to
						if(!toSend)	break;		// Not enough TX space
						
						if(client->dynFieldOffset >= client->dynFieldLen)	// SMF prevent overflow with >= instead of ==
						{
							// Read as much as possible
							client->dynFieldLen = DynamicVarRead(clientIndex,client->buffer,client->bufferLen);
							client->dynFieldOffset = 0;			// Clear dyn offset (we've written to the buffer)
							
							if(client->dynFieldLen == 0)
							{
								DBG_INFO("DYN DONE\r\n");
								client->readNewSector = TRUE; 		// Indicate buffer is invalid now - was reused
								client->dynFieldLen = 0;			// Clear end index	
								client->state = STATE_TRANSMIT_FILE;// Send rest of file
								/*client->bufferIndex will point to next file bytes after ~XXX~ field */
								continue;
							}
							DBG_INFO("DYN READ: %u\r\n", client->dynFieldLen);
						}

						if((client->dynFieldLen - client->dynFieldOffset) < toSend)	// Unsigned! Beware negative integer underflow!
							toSend = client->dynFieldLen - client->dynFieldOffset;
						
						toSend = TCPPutArray(client->socket, (unsigned char*)(client->buffer+client->dynFieldOffset), toSend);
						DBG_INFO("DYN SENT: %u\r\n",toSend);
						
						client->dynFieldOffset += toSend;
						
						continue;
					}
					case STATE_CLOSE:
					{
						// Flush remaining
						TCPFlush(client->socket);
						// Complete rx
						TCPDiscard(client->socket);	
						// Reset state
						HTTPClientClearState(client);
						// Disconnect
						if(TCPIsConnected(client->socket))
						{
							TCPDisconnect(client->socket);
							if(TCPWasReset(client->socket))
								continue;
							TCPDisconnect(client->socket);
						}
						// Force disconnect
						DBG_INFO("DISCONNECTED\r\n");
						break;
					}	
				}// client->state switch

				// Check timeout
				if ((client->timer != 0xffffffff) &&  // A request has occured
					((TickGet() - client->timer) > (CONNECTION_TIMEOUT*TICK_SECOND)) &&
					(TCPIsConnected(client->socket))) // Still tx/rx busy + blocking
				{
					DBG_INFO("TIMEOUT\r\n");
					TCPDisconnect(client->socket); // Double disconnect == force
					TCPDisconnect(client->socket);
					client->timer = 0xffffffff;
					client->state = STATE_CLOSE;
				}
	
				break; // Inner for loop exit
			}// End of inner for loop
		}// For loop clientIndex
	}

#endif //ifdef STACK_USE_TINYHTTP_SERVER

//EOF

