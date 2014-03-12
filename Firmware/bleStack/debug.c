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

// Debugging outputs
// Karim Ladha, 2013-2014

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "bt config.h"
#include "debug.h"

#include "Utils\util.h"

#if (DBG_ENABLE)

// Allows remapping of default debug channel (e.g. to LCD)
void DBG_printf(const char* file,const char* fmt,...)
{
    va_list myargs;
    va_start(myargs, fmt);
	if(file)fprintf(stderr,"%s: ",file);
    vfprintf(stderr, fmt, myargs); // Divert to stderr
    va_end(myargs);
}

// Very fast call for small const packets
void DBG_fast(const char* data,unsigned char len)
{
	write(2,(void*)data,len);// 2 = stderr	
}

inline void DBG_assert(unsigned char condition, const char* file, unsigned short line)
{
	if(!condition)
	{
		DBG_printf(NULL,"ASSERT!: %s, %u\r\n",file,line);
	}
}

void DBG_dump(const unsigned char* data, unsigned short len)
{
	unsigned short i;
	unsigned char trunc = FALSE_;
	if(len>24)
	{
		len = 24; // Clamp dump load
		trunc = TRUE_;
	}
	for(i=0;i<len;i++)
	{
		DBG_printf(NULL,"%02X ",(unsigned char)data[i]);
	}
	if(trunc == TRUE_)
		DBG_printf(NULL,"..\r\n");
	else
		DBG_printf(NULL,"\r\n");
}

#endif
