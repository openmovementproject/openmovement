/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
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

// File System Utilities (depends on FSIO.c)
// Karim Ladha and Dan Jackson, 2011-2012

#ifdef USE_EDS
#define FSUTILS_EDS __eds__
#else
#define FSUTILS_EDS
#endif

#ifndef FSUTILS_IN_FSCONFIG
#include "FSconfig.h"
#endif

// FSIO.c doesn't allocate buffers on non-embedded compiles, this makes FSIO.c work
#if !defined(__18CXX) && !defined (__C30__) && !defined (__PIC32MX__) && !defined(FSUTILS_H)
    extern unsigned char gDataBuffer[MEDIA_SECTOR_SIZE];    // The global data sector buffer
    extern unsigned char gFATBuffer[MEDIA_SECTOR_SIZE];     // The global FAT sector buffer

	// ...and it fails to prototype these functions...
    unsigned char ReadByte( unsigned char* pBuffer, unsigned short index );
    unsigned short ReadWord( unsigned char* pBuffer, unsigned short index );
    unsigned int ReadDWord( unsigned char* pBuffer, unsigned short index );
#endif


// Only define functions if not included from a local FSconfig.h
#ifndef FSUTILS_IN_FSCONFIG

#ifndef FSUTILS_H
#define FSUTILS_H

//#include "MDD File System/FSIO.h"
struct FSFILE;

// Define these in the local FSconfig.h
//#define FS_FLUSH() FtlFlush(0)
//#define FS_WRITE_SECTOR_ECC(sector, buffer, ecc) FtlWriteSector(sector, buffer, ecc);
//#define FS_SECTOR_FLUSH 8		// (8 = 512kB)  Flush every N * 64kB written with FSfwriteSector

// Write a (sector-aligned) sector of data to the file (512 bytes) 
BOOL FSfwriteSector(const void *ptr, FSFILE *stream, BOOL ecc);
BOOL FSfwriteMultipleSectors(const void FSUTILS_EDS *ptr, FSFILE *stream, int count, BOOL ecc);
// Flush writes to a file
int FSfflush(FSFILE *fo);

// Free space on the drive
unsigned long FSDiskFree(void);
// TRUE if disk is full
unsigned char FSDiskFull(void);

// Read the number of sectors per cluster on the disk
unsigned char FSDiskSectorsPerCluster(void);

// The sector location of the first file cluster
unsigned long FSDiskDataSector(void);

// Returns the on-disk sector for the specified file offset
unsigned long FSFileOffsetToLogicalSector(const char *filename, unsigned long offset);

// Follow the cluster chain for a file, calling a user callback function
char FSFollowClusterChain(const char *filename, void *reference, void (*callback)(void *, unsigned short, unsigned long));

// Read a line from a file
char *FSfgets(char *str, int num, FSFILE *stream);

// Read/write a character
int FSfgetc(FSFILE *fp);
int FSfputc(int character, FSFILE *fp);

// Read/write a WORD
void FSfputshort(short v, FSFILE *fp);
short FSfgetshort(FSFILE *fp); 

// Read/write a DWORD
void FSfputlong(long v, FSFILE *fp); 
long FSfgetlong(FSFILE *fp);

// Creates a volume label (can only be used once after formatting, no existing volume entry removed)
char FSfsetvolume(const char *fileName);

#endif

#endif

