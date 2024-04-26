// Karim Ladha, 28-06-2015
// Interface between fatfs and ftl
// Honours fatfs's diskio.h
// Also adds fatfs's hardware functions 
// Presents logical drive sector interface
// To be used as common point of attachment for all disk functions

// Includes
#include <stdlib.h>
#include "FatFs/ff.h"
#include "FatFs/diskio.h"
#include "Ftl/xFtl.h"
#include "config.h"

#if defined(FILE_SYS_USE_RTC_TIME)
	#include "Peripherals/SysTime.h"
#endif

// Debug setting
#define DEBUG_LEVEL_LOCAL	DEBUG_FILESYS
#define DBG_FILE			"diskio.c"
#include "utils/debug.h"	

// Globals
/* 	Disk status:
	User variable to control disk seen by FatFs and FTL
	FatFs flags are all in the lower nibble
	STA_NOINIT, STA_NODISK, STA_PROTECT
	Other apps are using the upper nibble
	STA_MSD_PROTECT	0x20	MSD write protect flag 
	STA_ECC_OFF 	0x40 	Do not write ecc info to disk (faster)
*/	
unsigned char gDiskIOStatus = 0xFF; /* Uninitialized, not present and not writeable... */


// Definitions
#ifndef STA_ECC_OFF
#define STA_ECC_OFF 	0x10 	/* Do not write ecc info to disk (faster)*/
#endif
#ifndef STA_MSD_NO_DISK
#define STA_MSD_NO_DISK 0x20 	/* MSD media detect flag */
#endif
#ifndef STA_MSD_PROTECT
#define STA_MSD_PROTECT	0x40	/* MSD write protect flag */
#endif


// Prototypes

// Source

// API for FatFs module (multiple functions) 
DSTATUS disk_initialize(BYTE drive)
{
	if(drive != 0)
	{
		// Error - Only one drive implemented
		return (unsigned char)STA_NOINIT;
	}
	// Call startup once only
	if(gDiskIOStatus & STA_NOINIT)
	{
		// If not initialised, try to init FTL
		if(!FtlStartup())
			gDiskIOStatus = (0xFF & (~STA_NOINIT)); // Flag as inited, mark as un-useable
		else
			gDiskIOStatus = 0;						// Initialised, present, writeable, ready
	}
    return disk_status(drive);
}

DSTATUS disk_status(BYTE drive)
{
	return gDiskIOStatus;
}

DRESULT disk_read(BYTE drive, BYTE *buffer, DWORD sector, BYTE count)
{
	unsigned char diskStatus;
    if (drive != 0) { return RES_PARERR; }
    if (buffer == 0) { return RES_PARERR; }
	diskStatus = disk_status(drive);
	if(diskStatus & STA_NOINIT) { return RES_NOTRDY; }
	if(diskStatus & STA_NODISK) { return RES_NOTRDY; }
	for(;count > 0; count--, sector++, buffer += FTL_SECTOR_SIZE)
	{
		if(!FtlReadSector((unsigned long)sector, (unsigned char*)buffer))
			return RES_ERROR;
	}
    return RES_OK;
}

DRESULT disk_write(BYTE drive, const BYTE *buffer, DWORD sector, BYTE count)
{
	unsigned char diskStatus;
	char eccOn;
    if (drive != 0) { return RES_PARERR; }
    if (buffer == 0) { return RES_PARERR; }
	diskStatus = disk_status(drive);	
	if(diskStatus & STA_NOINIT) { return RES_NOTRDY; }
	if(diskStatus & STA_NODISK) { return RES_NOTRDY; }
	if(diskStatus & STA_PROTECT) { return RES_WRPRT; }	
	// Error correction may be turned off by user with flag
	eccOn = (diskStatus & STA_ECC_OFF) ? 0 : 1;
	for(;count > 0; count--, sector++, buffer += FTL_SECTOR_SIZE)
	{
		if(!FtlWriteSector((unsigned long)sector, (const unsigned char*)buffer, eccOn))
			return RES_ERROR;
	}
    return RES_OK;
}

DRESULT disk_ioctl(BYTE drive, BYTE command, void *buffer)
{
	unsigned char diskStatus;
    if (drive != 0) { return RES_PARERR; }
	diskStatus = disk_status(drive);	
	if(diskStatus & STA_NOINIT) { return RES_NOTRDY; }
	if(diskStatus & STA_NODISK) { return RES_NOTRDY; }	
	
    switch (command)
    {
        case CTRL_SYNC:
            if(!FtlFlush(0))return RES_ERROR;
			else	        return RES_OK;

        case GET_SECTOR_COUNT: {
			unsigned long logicalSectors;	
			if(!FtlDiskParameters(&logicalSectors, NULL, NULL))
				return RES_NOTRDY;
            *((unsigned long *)buffer) = logicalSectors;
            return RES_OK;
		}
        case GET_SECTOR_SIZE: {
			/*
			unsigned short sectorSize;	
			if(!FtlDiskParameters(NULL, &sectorSize, NULL))
				return RES_NOTRDY;
			*((unsigned short *)buffer) = sectorSize;
			*/
			*((unsigned short *)buffer) = (unsigned short)FTL_SECTOR_SIZE;
            return RES_OK;
		}
        case GET_BLOCK_SIZE: {
            // FatFs aligns the disk structures to erase block boundaries on format
			unsigned short blockSectors;	
			if(!FtlDiskParameters(NULL, NULL, &blockSectors))
				return RES_NOTRDY;
			*((unsigned long *)buffer) = blockSectors;
            return RES_OK;
		}
        case CTRL_ERASE_SECTOR:
			#if _USE_ERASE
				#warning "Erase not honored to preserve flash memory life." 
			#endif
            return RES_OK;
    }

    return RES_PARERR;
}

// Other close coupled fatfs required functions

/* OEM-Unicode bidirectional conversion */
WCHAR ff_convert (WCHAR chr, UINT dir)
{
	return (WCHAR)chr;
}

/* Unicode upper-case conversion */
WCHAR ff_wtoupper (WCHAR chr)
{
	if (chr >= 'a' && chr <= 'z') { chr = 'A' + chr - 'a'; }
	return (WCHAR)chr;
}

/* Allocate memory block */
void* ff_memalloc (UINT msize)	
{
	return DBG_MALLOC((size_t)msize);
}

/* Free memory block */		
void ff_memfree (void* mblock)
{
	DBG_FREE(mblock);
}		

/* Get file time stamp */
unsigned long get_fattime(void)
{
#if defined(FILE_SYS_USE_RTC_TIME)
	DateTime_t now;
	SysTimeRead(&now);
	return SysTimeToFat(&now);
#elif defined(FILE_SYS_USE_INC_TIME)
	static unsigned long time;
	return time++;
#else
	#error "Invalid selection"
#endif
}

// This disk driver expects the following function to be available in the FTL api
#ifdef FTL_LEGACY_MODE

#warning "This should probably not be used."

#undef FTL_LOGICAL_BLOCKS
#undef FTL_PAGES_PER_BLOCK
#undef FTL_SECTORS_PER_PAGE
#undef FTL_PHYSICAL_BLOCKS
#undef FTL_PLANES
#undef FTL_SECTOR_TO_PAGE_SHIFT
#undef FTL_SECTOR_TO_BLOCK_SHIFT
#undef FTL_SPARE_OFFSET
#undef FTL_SPARE_BYTES_PER_PAGE

#define _USING_FTL_CONFIG_H_
#include "FtlConfig.h"

/* Retrieve FTL disk parameters */
char FtlDiskParameters(unsigned long* logicalSectors, unsigned short* sectorSize, unsigned short* eraseBlockSectors)
{
	// Set outputs
	if(logicalSectors != NULL)*logicalSectors = 1UL * FTL_LOGICAL_BLOCKS * FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE;
	if(sectorSize != NULL)*sectorSize = FTL_SECTOR_SIZE;
	if(eraseBlockSectors != NULL)*eraseBlockSectors = FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE;

	return 1;
}

#endif

//EOF
