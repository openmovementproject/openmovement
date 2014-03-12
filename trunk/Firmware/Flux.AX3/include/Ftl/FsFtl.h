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

// File System I/O (FSIO) to Flash Translation Layer (FTL) bridge
// Dan Jackson and Karim Ladha, 2011

#ifndef FSFTL_H
#define FSFTL_H


#define FSFTL_SECTOR_FLUSH 8		// (8 = 512kB)  Flush every N * 64kB written with FSfwriteSector
//#define FSFTL_READ_PREFETCH     	// [never tested] Support next-sector pre-fetching
//#define FSFTL_OWN_FORMAT			// [don't use] Support own format command
//#define FSFTL_WRITE_VOLUME_LABEL    // [experimental - may cause file system problems] Write volume label after formatting

#include "Compiler.h" 
#ifndef FSFTL_IN_FSCONFIG
#include "FSconfig.h"
#endif
#include "MDD File System/FSDefs.h" 
#include "MDD File System/FSIO.h"
#include "Ftl/Ftl.h"

#if FTL_SECTOR_SIZE != MEDIA_SECTOR_SIZE
	#error "Sector size mismatch between filesystem and storage."
#endif

#if ((FTL_LOGICAL_BLOCKS << FTL_SECTOR_TO_BLOCK_SHIFT) > 0x3FFD5F)
#error "Too many logical sectors for FSformat"
#endif

BYTE MDD_FTL_FSIO_MediaDetect(void);
BYTE MDD_FTL_USB_MediaDetect(void);

MEDIA_INFORMATION * MDD_FTL_MediaInitialize(void);

BYTE MDD_FTL_FSIO_SectorRead(DWORD sector_addr, BYTE* buffer);
BYTE MDD_FTL_USB_SectorRead(DWORD sector_addr, BYTE* buffer);

BYTE MDD_FTL_USB_SectorWrite(DWORD sector_addr, BYTE* buffer, BYTE allowWriteToZero);
BYTE MDD_FTL_FSIO_SectorWrite(DWORD sector_addr, BYTE* buffer, BYTE allowWriteToZero);

WORD MDD_FTL_ReadSectorSize(void);
DWORD MDD_FTL_ReadCapacity(void);
BYTE MDD_FTL_WriteProtectState(void);
BYTE MDD_FTL_InitIO(void);


#ifdef FSFTL_READ_PREFETCH
void FsFtlPrefetch(void);
#endif

// FSIO.c doesn't allocate buffers on non-embedded compiles
#if !defined(__18CXX) && !defined (__C30__) && !defined (__PIC32MX__)
    extern BYTE gDataBuffer[MEDIA_SECTOR_SIZE];    // The global data sector buffer
    extern BYTE gFATBuffer[MEDIA_SECTOR_SIZE];     // The global FAT sector buffer

	// ...and it fails to prototype these functions...
    BYTE ReadByte( BYTE* pBuffer, WORD index );
    WORD ReadWord( BYTE* pBuffer, WORD index );
    DWORD ReadDWord( BYTE* pBuffer, WORD index );
#endif


// Drive reported as mounted when connected to USB
extern char fsftlUsbDiskMounted;


// Don't define functions that require FSFILE
#ifndef FSFTL_IN_FSCONFIG

    // Get a character
    int FSfgetc(FSFILE *fp);

    // Put a character
    int FSfputc(int character, FSFILE *fp);

    // Get/put word/dword
    void FSfputshort(short v, FSFILE *fp);
    void FSfputlong(long v, FSFILE *fp);
    short FSfgetshort(FSFILE *fp);
    long FSfgetlong(FSFILE *fp);
    
    // Flushes the file system stream and FTL buffers
    int FSfflush(FSFILE *stream);

    // Retrieve a line from a file
    extern char *FSfgets(char *str, int num, FSFILE *stream);

	// Formats the media, optionally initializes the NAND memory
	char FsFtlFormat(char wipe, long serial, const char *volumeLabel);

    // Writes an aligned sector with optional ECC
    BOOL FSfwriteSector(const void *ptr, FSFILE *stream, BOOL ecc);

#endif

// Calculates the free space remaining on the drive
unsigned long FSDiskFree(void);

#endif
