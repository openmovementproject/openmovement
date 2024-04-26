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

#include "Compiler.h" 
#ifndef FSFTL_IN_FSCONFIG
#include "FSconfig.h"
#ifndef FS_CONFIG_FTL_AWARE
#warning "Local FSconfig.h does not seem to be aware of the new FSutils.c/FsFtl.c split, see #defines from CWA/FSconfig.h for example."
#endif
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


/*
// Define one or more of these in the local FSconfig.h
//#define FSFTL_READ_PREFETCH     	// [not tested] Support next-sector pre-fetching
//#define FSFTL_OWN_FORMAT			// [don't use] Support own format command
//#define FSFTL_WRITE_VOLUME_LABEL    // [experimental - may cause file system problems] Write volume label after formatting
*/

// Prefetch (experimental)
#ifdef FSFTL_READ_PREFETCH
void FsFtlPrefetch(void);
#endif

// Drive reported as mounted when connected to USB
extern char fsftlUsbDiskMounted;


// Formats the media, optionally initializes the NAND memory
char FsFtlFormat(char wipe, long serial, const char *volumeLabel);


#endif
