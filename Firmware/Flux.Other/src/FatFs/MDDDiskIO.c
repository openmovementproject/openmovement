/* 
 * Copyright (c) 2013, Newcastle University, UK.
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

// MDD Disk/File System - FatFs Bridge
// Dan Jackson, 2013

#include "FatFs/ff.h"
#include "FatFs/diskio.h"
#include "FsConfig.h"


// MDD function implementations
/*
// MEDIA_INFORMATION
typedef struct 
{
    unsigned char errorCode;
    union 
    {
        unsigned char value;
        struct 
        {
            unsigned char sectorSize : 1;
            unsigned char maxLUN : 1;
        } bits;
    } validityFlags;
    unsigned short sectorSize;
    unsigned char maxLUN;
} MEDIA_INFORMATION;

// MEDIA_ERRORS
#define MEDIA_NO_ERROR           0
#define MEDIA_DEVICE_NOT_PRESENT 1
#define MEDIA_CANNOT_INITIALIZE  2

unsigned char MDD_MediaDetect(void);
MEDIA_INFORMATION *MDD_MediaInitialize(void);
unsigned char MDD_SectorRead(unsigned long sector_addr, unsigned char* buffer);
unsigned char MDD_SectorWrite(unsigned long sector_addr, unsigned char* buffer, unsigned char allowWriteToZero);
unsigned short MDD_ReadSectorSize(void);
unsigned long MDD_ReadCapacity(void);
unsigned char MDD_WriteProtectState(void);
unsigned char MDD_InitIO(void);
*/

#ifndef FS_USE_MULTI_SECTOR
BYTE MDD_MultipleSectorWrite(DWORD sector_addr, BYTE* buffer, int numSectors, BYTE allowWriteToZero)
{
	int i;
	for (i = 0; i < numSectors; i++)
	{
		if (!MDD_SectorWrite(sector_addr + i, buffer + i * MEDIA_SECTOR_SIZE, allowWriteToZero))
		{
			return FALSE;
		}
	}
	return TRUE;
}
#endif

#ifndef FS_USE_MULTI_SECTOR_READ
BYTE MDD_MultipleSectorRead(DWORD sector_addr, BYTE* buffer, int numSectors)
{
	int i;
	for (i = 0; i < numSectors; i++)
	{
		if (!MDD_SectorRead(sector_addr + i, buffer + i * MEDIA_SECTOR_SIZE))
		{
			return FALSE;
		}
	}
	return TRUE;
}
#endif


static MEDIA_INFORMATION *mediaInformation = NULL;

DSTATUS disk_initialize(BYTE drive)
{
    MDD_InitIO();
    mediaInformation = MDD_MediaInitialize();
    return disk_status(drive);
}

DSTATUS disk_status(BYTE drive)
{
    unsigned char flags = 0;
    if (drive != 0 || mediaInformation == NULL || mediaInformation->errorCode != MEDIA_NO_ERROR) { flags |= STA_NOINIT; }
    if (!MDD_MediaDetect()) { flags |= STA_NODISK; }
    if (MDD_WriteProtectState()) { flags |= STA_PROTECT; }
    return flags;
}

DRESULT disk_read(BYTE drive, BYTE *buffer, DWORD sector, BYTE count)
{
    if (drive != 0) { return RES_PARERR; }
    if (buffer == 0) { return RES_PARERR; }
    if (mediaInformation == NULL || mediaInformation->errorCode != MEDIA_NO_ERROR) { return RES_NOTRDY; }

    if (!MDD_MultipleSectorRead(sector, buffer, count))
    {
        return RES_ERROR;
    }    
    return RES_OK;
}

DRESULT disk_write(BYTE drive, const BYTE *buffer, DWORD sector, BYTE count)
{
    if (drive != 0) { return RES_PARERR; }
    if (buffer == 0) { return RES_PARERR; }
    if (mediaInformation == NULL || mediaInformation->errorCode != MEDIA_NO_ERROR) { return RES_NOTRDY; }

    if (!MDD_MultipleSectorWrite(sector, (BYTE*)buffer, count, 1))
    {
        return RES_ERROR;
    }
    return RES_OK;
}

DRESULT disk_ioctl(BYTE drive, BYTE command, void *buffer)
{
    if (drive != 0) { return RES_PARERR; }
    if (mediaInformation == NULL || mediaInformation->errorCode != MEDIA_NO_ERROR) { return RES_NOTRDY; }

    switch (command)
    {
        case CTRL_SYNC:
            // Ignore (no writes)
            return RES_OK;

        case GET_SECTOR_COUNT:
            *((unsigned long *)buffer) = MDD_ReadCapacity();
            return RES_OK;

        case GET_SECTOR_SIZE:
            *((unsigned short *)buffer) = MDD_ReadSectorSize();
            return RES_OK;

        case GET_BLOCK_SIZE:
            // Assume block size is 1 sector
            *((unsigned long *)buffer) = 1 * MDD_ReadSectorSize();
            return RES_OK;

        case CTRL_ERASE_SECTOR:
            // Ignore (no writes)
            return RES_OK;
    }

    return RES_PARERR;
}

/*
unsigned long get_fattime(void)
{
    return 0;
}
*/

