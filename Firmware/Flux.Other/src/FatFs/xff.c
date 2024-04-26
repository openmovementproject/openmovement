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


// Includes
#include <stdlib.h>

#include "config.h"
#include "FatFs/xff.h"

// Debug setting
#define DEBUG_LEVEL_LOCAL	DEBUG_FILESYS
#define DBG_FILE			"xff.c"
#include "utils/debug.h"	

// Globals

/*	The drive and file system:
	This is the drive mount state of the FTL disc object and it address (constant pointer)
*/
FATFS gFS;

/*	The open files:
	Not used if dynamic file allocation is enabled
	The total number is a setting in config.h
*/
#ifdef FS_MAX_FILES_OPEN
	FIL gFile[FS_MAX_FILES_OPEN];
#endif

// Initialize and mounts the disk. Returns TRUE if successful.
int FSInit(void)
{
	FRESULT res;
#ifdef FS_MAX_FILES_OPEN
	// Clear all files-in-use
	memset(gFile, 0, sizeof(gFile));
#endif
	// Initialize and mount the disk
	res = f_mount(0, &gFS);
	return (res == FR_OK);
}

// C lib. fopen version
FIL* f_fopen(const char *path, const char *mode)
{	
	FIL *file;
    FRESULT result;
	unsigned char append, flags;
#ifdef FS_MAX_FILES_OPEN
	uint16_t index;
#endif

	// Get a free object
#ifdef FS_MAX_FILES_OPEN
	file = NULL;
	for(index = 0; index < FS_MAX_FILES_OPEN; index++)
	{
		if(gFile[index].fs != NULL)
		{
			file = &gFile[index];
			break;
		}
	}
#else
	file = DBG_MALLOC(sizeof(FIL));
#endif

	// Check for object
	if(file == NULL)
	{
		DBG_INFO("fopen fail, no resources");
		return NULL;
	}

    // Opening flags and append
    flags = 0;
    append = 0;
    if (mode[0] == 'r')
    { 
        flags = FA_READ | FA_OPEN_EXISTING;
        if (mode[1] == '+') { flags |= FA_WRITE; }
    }
    else if (mode[0] == 'w')
    { 
        flags = FA_WRITE | FA_CREATE_ALWAYS;
        if (mode[1] == '+') { flags |= FA_READ; }
    }
    else if (mode[0] == 'a')
    {
        flags = FA_WRITE | FA_OPEN_ALWAYS;
        if (mode[1] == '+') { flags |= FA_READ; }
        append = 1;
    }

	// Open file
    result = f_open(file, path, flags);
	if(result == FR_OK)
	{
        if (append)
        {
            f_lseek(file, f_size(file));
        }
		return file;
	}

	// Free if failed
#ifdef FS_MAX_FILES_OPEN
	file->fs = NULL;
#else
	DBG_FREE(file);
#endif
    return NULL;
}

// C lib. fclose version
int f_fclose(FIL *file)
{
    FRESULT res;
    
	// Checks
	if(!file) return -1;

    // Close the file
    res = f_close((FIL *)file);

	// Free it
#ifdef FS_MAX_FILES_OPEN
	file->fs = NULL;
#else
	DBG_FREE(file);
#endif

	// Return
    return (res == FR_OK) ? 0 : -1;
}


// C stdio fseek() compatible replacement
int f_fseek(FIL *file, long offset, int whence)
{
	switch (whence)
	{
		case 1:		// SEEK_CUR
			offset += f_tell(file);
			break;
		case 2:		// SEEK_END
			offset += f_size(file);
			break;
		default:	// SEEK_SET
			break;
	}
	
	if (f_lseek(file, offset) != FR_OK)
	{
		return -1;
	}
	return 0;
}

// C stdio fread() compatible replacement
size_t f_fread(void *buffer, size_t size, size_t count, FIL *file)
{
	size_t result = 0;
	f_read(file, buffer, size * count, &result);
	if (size > 1)
	{
		result /= size;
	}
	return result;
}

// C stdio fwrite() compatible replacement
size_t f_fwrite(void *buffer, size_t size, size_t count, FIL *file)
{
	size_t result = 0;
	f_write(file, buffer, size * count, &result);
	if (size > 1)
	{
		result /= size;
	}
	return result;
}

// C stdio fgetc() compatible replacement
int f_fgetc(FIL *file)
{
	char c = 0;
	unsigned int result = 0;
	FRESULT res = f_read(file, &c, 1, &result);
	if (res != FR_OK || result < 1)
	{
		return -1;
	}
	return (int)c;
}

// C stdio fgets() compatible replacement (FF can optionally have 'f_gets()')
#if !defined(_USE_STRFUNC) || _USE_STRFUNC < 1
char *f_fgets(char *str, int num, FIL *stream)
{
	char *p;
	unsigned int n;

	if (num <= 0) { return NULL; }
	if (f_eof(stream)) { str[0] = '\0'; return NULL; }

	for (p = str; !f_eof(stream); p++, num--)
	{
		// Painfully read one byte at a time
		if (f_read(stream, p, 1, &n) != FR_OK)
        {
            if (p == str) { return NULL; }
            n = 0;
        }

		// If no more buffer space, or end of file or CR or LF...
		if (num <= 1 || n == 0 || *p == '\r' || *p == '\n')
		{
			// NULL-terminate and exit
			*p = '\0';
			break;
		}
	}
	return str;
}
#endif


// Available free space in bytes on the drive (returns 0 if disk full)
unsigned long f_diskfree(void)
{
	DWORD freeClusters = 0;
	FATFS *fs;
	f_getfree("0:", &freeClusters, &fs);
	unsigned long freeSectors = freeClusters * fs->csize;
#if _MAX_SS != 512
	return freeSectors * fs->ssize;
#else
	return freeSectors * 512;
#endif
}

// EOF
