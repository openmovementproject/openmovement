/*
* Copyright (c) 2009-2015, Newcastle University, UK.
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

// General extensions to FatFs file system including global variables for the logical disk.
// Karim Ladha 2015
// additions by Dan Jackson

#ifndef _XFF_H_
#define _XFF_H_

// Includes
#include "FatFs/ff.h"

// Initialize and mounts the disk. Returns TRUE if successful.
int FSInit(void);


// C stdio-compatible functions

// C lib. fopen version
FIL* f_fopen(const char *path, const char *mode);

// C lib. fclose version
int f_fclose(FIL *file);

// C stdio fseek() compatible replacement
int f_fseek(FIL *file, long offset, int whence);

// C stdio fread() compatible replacement
size_t f_fread(void *buffer, size_t size, size_t count, FIL *file);

// C stdio fwrite() compatible replacement
size_t f_fwrite(void *buffer, size_t size, size_t count, FIL *file);

// C stdio fgetc() compatible replacement
int f_fgetc(FIL *f);


// C stdio fgets() compatible replacement (FF can optionally have 'f_gets()')
#if !defined(_USE_STRFUNC) || _USE_STRFUNC < 1
char *f_fgets(char *str, int num, FIL *stream);
#endif

// Available free space in bytes on the drive (returns 0 if disk full)
unsigned long f_diskfree(void);

#endif
