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

/*
 * TinyHTTP.h
 * 
 * With regards to:
 * http://www.w3.org/Library/src/HTTPServ.c
 *
 */

#ifndef __TINYHTTP_H
#define __TINYHTTP_H

#ifdef STACK_USE_TINYHTTP_SERVER
	
	void HTTPServer(void);
	void HTTPInit(void);
		
	/* Not implemented yet...
	void startup(void);
	void serve_file(TCP_SOCKET socket, const char *filename);
	
	// Status Handlers
	void status_ok(TCP_SOCKET socket);				// 200
	void status_bad_request(TCP_SOCKET socket);		// 400
	void status_not_found(TCP_SOCKET socket);		// 404
	void status_not_implemented(TCP_SOCKET socket);	// 501
	*/

#endif	// defined STACK_USE_TINYHTTP_SERVER
#endif	// ifndef __TINYHTTP_H

//EOF


