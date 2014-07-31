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


/* Note: Changes required to Microchip's FSIO.c

In fsio.c, FILEget_next_cluster(), change:
    if ( c >= disk->maxcls)
...to:
    if ( c >= disk->maxcls + 2)
...to match the limit in FATfindEmptyCluster().

In fsio.c, FSformat(), after the while loop:
    while (DataClusters > 0xFFED)
...add:
    #ifdef FORMAT_SECTORS_PER_CLUSTER
        disk->SecPerClus = FORMAT_SECTORS_PER_CLUSTER;
        DataClusters = secCount / disk->SecPerClus;
    #endif
...to allow user-defined cluster sizes in FAT16 partitions.

In fsio.c, FILECreateHeadCluster(), comment out:
    error = EraseCluster(disk,*cluster);
...I don't think this is required (at least for writing -- check whether reading copes)
*/

#ifdef _WIN32
#define NO_WARNING_DISABLE
#endif

#include "Compiler.h"
#include "HardwareProfile.h"

#if defined(_WIN32) && !defined(FTL)
    // Ignore source file
#else

#include "Ftl/Ftl.h"
#include "Ftl/FsFtl.h"



static MEDIA_INFORMATION mediaInformation; 		// Mchips MDD variable
char fsftlUsbDiskMounted = 1;


BYTE MDD_FTL_FSIO_MediaDetect(void)
{
	return TRUE;
}

BYTE MDD_FTL_USB_MediaDetect(void)
{
	return fsftlUsbDiskMounted ? TRUE : FALSE;
}


WORD MDD_FTL_ReadSectorSize(void)
{
	return MEDIA_SECTOR_SIZE;
}

DWORD MDD_FTL_ReadCapacity(void)
{
	return (1UL * FTL_LOGICAL_BLOCKS * FTL_SECTORS_PER_PAGE * FTL_PAGES_PER_BLOCK - FTL_FIRST_SECTOR);	// The last -FTL_FIRST_SECTOR is to offset for the first sector on the disk (after MBR)
}

BYTE MDD_FTL_WriteProtectState(void)
{
	return FALSE;
}


MEDIA_INFORMATION * MDD_FTL_MediaInitialize(void)
{
	// Bug fix, the USB stack may re-call this function after writing some data thus loosing it all as segment remap is lost
	MDD_FTL_InitIO();

	mediaInformation.errorCode = MEDIA_NO_ERROR;
	mediaInformation.sectorSize = MEDIA_SECTOR_SIZE;	
	mediaInformation.validityFlags.bits.sectorSize = 1; // Indicates the media has set its sector size

	return &mediaInformation;
}


BYTE MDD_FTL_InitIO(void)
{
    // NOTE: Media initialize does *not* call FtlStartup() -- this is to cope with multiple inits, etc (+ redoing the formatting before an initialize)
    //FtlStartup();
	return TRUE;
}


BYTE MDD_FTL_FSIO_SectorWrite(DWORD sector_addr, BYTE* buffer, BYTE allowWriteToZero)
{
	if (sector_addr == 0 && !allowWriteToZero) { return 0; }
	return FtlWriteSector(sector_addr, buffer, 1);
}

BYTE MDD_FTL_USB_SectorWrite(DWORD sector_addr, BYTE* buffer, BYTE allowWriteToZero)
{
	if (sector_addr == 0 && !allowWriteToZero) { return 0; }
#ifdef FSFTL_READ_PREFETCH
    fsftlPrefetchSector = (DWORD)-1;
#endif
	return FtlWriteSector(sector_addr, buffer, 1);
	//return 1;		// Always return success
}

BYTE MDD_FTL_FSIO_SectorRead(DWORD sector_addr, BYTE* buffer)
{
	return FtlReadSector(sector_addr, buffer);
}

BYTE MDD_FTL_USB_SectorRead(DWORD sector_addr, BYTE* buffer)
{
#ifdef FSFTL_READ_PREFETCH
    fsftlLastSector = sector_addr;
    if (sector_addr == fsftlPrefetchSector)
    {
        memcpy(buffer, fsftlPrefetchBuffer, FTL_SECTOR_SIZE);
        return 1;
    }
#endif
	FtlReadSector(sector_addr, buffer);
	return 1;		// Always return success
}


/*
// Internal types from FSIO.c
typedef FSFILE   * FILEOBJ;         // Pointer to an FSFILE object
typedef struct
{
    char      DIR_Name[DIR_NAMESIZE];           // File name
    char      DIR_Extension[DIR_EXTENSION];     // File extension
    BYTE      DIR_Attr;                         // File attributes
    BYTE      DIR_NTRes;                        // Reserved byte
    BYTE      DIR_CrtTimeTenth;                 // Create time (millisecond field)
    WORD      DIR_CrtTime;                      // Create time (second, minute, hour field)
    WORD      DIR_CrtDate;                      // Create date
    WORD      DIR_LstAccDate;                   // Last access date
    WORD      DIR_FstClusHI;                    // High word of the entry's first cluster number
    WORD      DIR_WrtTime;                      // Last update time
    WORD      DIR_WrtDate;                      // Last update date
    WORD      DIR_FstClusLO;                    // Low word of the entry's first cluster number
    DWORD     DIR_FileSize;                     // The 32-bit file size
}_DIRENTRY;

typedef _DIRENTRY * DIRENTRY;                   // A pointer to a directory entry structure

// External FSIO Prototypes needed by the functions below
extern DWORD Cluster2Sector (DISK * disk, DWORD cluster);
extern BYTE flushData (void);
extern BYTE FILEallocate_new_cluster (FILEOBJ fo, BYTE mode);
extern BYTE FILEget_next_cluster (FILEOBJ fo, DWORD n);
extern BYTE Write_File_Entry( FILEOBJ fo, WORD * curEntry);
extern void IncrementTimeStamp(DIRENTRY dir);
extern DWORD WriteFAT (DISK *dsk, DWORD ccls, DWORD value, BYTE forceWrite);
extern DIRENTRY LoadDirAttrib(FILEOBJ fo, WORD *fHandle);
extern CETYPE CreateFileEntry(FILEOBJ fo, WORD *fHandle, BYTE mode, BOOL createFirstCluster);
extern BYTE FormatFileName( const char* fileName, FILEOBJ fptr, BYTE mode);
extern DWORD ReadFAT (DISK *dsk, DWORD ccls);

// External FSIO Globals
extern BYTE    FSerrno;                    	// Global error variable.  Set to one of many error codes after each function call.
extern BYTE    gBufferZeroed;			   	// Global variable indicating that the data buffer contains all zeros
extern FSFILE  *   gBufferOwner;           	// Global variable indicating which file is using the data buffer
extern DWORD   gLastDataSectorRead;   		// Global variable indicating which data sector was read last
extern BYTE    gNeedDataWrite;             	// Global variable indicating that there is information that needs to be written to the data section
#if defined(USEREALTIMECLOCK) || defined(USERDEFINEDCLOCK)
// Timing variables
extern BYTE    gTimeCrtMS;     // Global time variable (for timestamps) used to indicate create time (milliseconds)
extern WORD    gTimeCrtTime;   // Global time variable (for timestamps) used to indicate create time
extern WORD    gTimeCrtDate;   // Global time variable (for timestamps) used to indicate create date
extern WORD    gTimeAccDate;   // Global time variable (for timestamps) used to indicate last access date
extern WORD    gTimeWrtTime;   // Global time variable (for timestamps) used to indicate last update time
extern WORD    gTimeWrtDate;   // Global time variable (for timestamps) used to indicate last update date
extern void CacheTime(void);
#endif

extern DISK gDiskData;
extern DWORD FatRootDirClusterValue;

#ifndef FS_DYNAMIC_MEM
    extern FSFILE gFileArray[FS_MAX_FILES_OPEN];
    extern BYTE   gFileSlotOpen[FS_MAX_FILES_OPEN];
	#ifdef SUPPORT_LFN
		extern unsigned short int lfnData[FS_MAX_FILES_OPEN][257];
	#endif
#endif
*/



// Support for pre-fetching
#ifdef FSFTL_READ_PREFETCH
static DWORD fsftlLastSector = (DWORD)-1;
static DWORD fsftlPrefetchSector = (DWORD)-1;
// TODO: Share another buffer (FSIO?) to save RAM, but must add software switch for prefetch only on USB MSD
static BYTE fsftlPrefectchBuffer[FTL_SECTOR_SIZE];

void FsFtlPrefetch(void)
{
    if (fsftlPrefetchSector != fsftlLastSector + 1)
    {
        fsftlPrefetchSector = fsftlLastSector + 1;
    	FtlReadSector(fsftlPrefetchSector, fsftlPrefectchBuffer);
    }
}
#endif





// The size (in number of sectors) of the desired data portion drive (must fit the specified file on disk with filesystem overhead)
#define VIRTUAL_DISK_DRIVE_CAPACITY (1UL * FTL_LOGICAL_BLOCKS * FTL_SECTORS_PER_PAGE * FTL_PAGES_PER_BLOCK - FTL_FIRST_SECTOR)      // FAT16 if >= 4096 clusters (if >= 0x40000 sectors)
																																	// The last -FTL_FIRST_SECTOR is to offset for the first sector on the disk (after MBR)

// The nominal size of the file equal to the size of the flash memory (will be patched-over with the actual data size)
#if ((FLASH_CAPACITY_SECTORS * MEDIA_SECTOR_SIZE) <= VIRTUAL_DISK_MAX_FILE_SIZE)
#define VIRTUAL_DISK_FILE_SIZE (FLASH_CAPACITY_SECTORS * MEDIA_SECTOR_SIZE)
#else
#define VIRTUAL_DISK_FILE_SIZE VIRTUAL_DISK_MAX_FILE_SIZE
#endif

// Calculate the number of clusters and the number of FAT sectors required to point to the clusters
#define VIRTUAL_DISK_SECTORS_PER_CLUSTER 0x40UL   // 0x40L -- 64 * 512 = 32Kb clusters
#define VIRTUAL_DISK_NUM_CLUSTERS ((VIRTUAL_DISK_DRIVE_CAPACITY / VIRTUAL_DISK_SECTORS_PER_CLUSTER) + 1)
#define VIRTUAL_DISK_NUM_FAT_SECTORS (VIRTUAL_DISK_NUM_CLUSTERS * 2L / MEDIA_SECTOR_SIZE + 1)       // FAT16 = 2 bytes per cluster

// Calculate 
#define VIRTUAL_DISK_NUM_RESERVED_SECTORS 1
#define VIRTUAL_DISK_NUM_ROOT_DIRECTORY_SECTORS 1
#define VIRTUAL_DISK_OVERHEAD_SECTORS (VIRTUAL_DISK_NUM_RESERVED_SECTORS + VIRTUAL_DISK_NUM_ROOT_DIRECTORY_SECTORS + VIRTUAL_DISK_NUM_FAT_SECTORS)
#define VIRTUAL_DISK_TOTAL_DISK_SIZE (VIRTUAL_DISK_OVERHEAD_SECTORS + VIRTUAL_DISK_DRIVE_CAPACITY)

// Sector addresses
#define SECTOR_MBR 0	// MasterBootRecord
#define SECTOR_BOOT (0 + 1) // BootSector
#define SECTOR_FAT0 (SECTOR_BOOT + 1)	// FAT0
#define SECTOR_FATN (SECTOR_FAT0 + VIRTUAL_DISK_NUM_FAT_SECTORS - 1)		// FATN
#define SECTOR_ROOTDIR (SECTOR_FAT0 + VIRTUAL_DISK_NUM_FAT_SECTORS)		// RootDirectory0
#define SECTOR_FILECONTENTS (SECTOR_ROOTDIR + 1)	// slack0




#ifdef FSFTL_OWN_FORMAT
// Based on Microchip Technology file 'Files.c' from "USB Device - Composite - MSD + CDC" example.

// Logical Block Addressing (LBA)
// Cylinder Head Sector (CHS) Addressing
// Defaults for LBA = 0x3f sectors/track, 0xff heads [and usually 512 bytes per sector]
// (little-endian storage) hhhhhhhh ccssssss cccccccc  [63 sectors/track, 255 heads]

// C 0-1023 (10-bit, base 0, 1024 values)
// H 0-254 (8-bit, base 0, 255 values)
// S 1-63 (6-bit, base 1, 63 values)

// Range from CHS(0,0,1) to CHS(1023,254,63)

// Heads on disk: Nh = 255 (0-254)

// Sectors per track: Ns = 63 (1-63)

// LBA(c, h, s) = (c * Nh * Ns) + (h * Ns) + (s - 1)
//              = (c * 255 * 63) + (h * 63) + s - 1

// C(a) = a / (Nh * Ns)     = a / (255 * 63)
// H(a) = (a / Ns) % Nh     = (a / 63) % 255
// S(a) = (a % Ns) + 1      = (a % 63) + 1

//   C = 0x3e6 (998)  11 1110 0110
//   H = 0x07  (7)             111
//   S = 0x3f  (63)        11 1111

// 512 MB = 512 * 1024 * 1024 bytes = 1048576 blocks
// Total CHS = (65, 69, 5)
// (little-endian storage) hhhhhhhh ccssssss cccccccc  -> 69, 5, 65

const BYTE MasterBootRecord[MEDIA_SECTOR_SIZE] =
{
    // Code Area
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	//0x0000
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0010
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0020
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0030
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0040
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0050
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0060
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0070
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0080
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0090
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x00A0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x00B0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x00C0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x00D0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x00E0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x00F0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0100
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0110
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0120
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0130
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0140
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0150
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0160
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0170
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                     //0x0180
    
    // IBM 9 byte/entry x 4 entries primary partition table
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                                             //0x018A
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x0190
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             //0x01A0
    
    // ???
    0x00, 0x00,                                                                                     //0x01AE
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                                 //0x01B0
    
    // Disk signature
    0xF5, 0x8B, 0x16, 0xEA,                                                                         //0x01B8
    
    // ??? - usually 0x0000
    0x00, 0x00,                                                                                     //0x01BC
    
    // Table of Primary Partitions (16 bytes/entry x 4 entries)
    // Entry 1                                                                                       //0x01BE
    0x80,                   // Status - 0x80 (bootable), 0x00 (not bootable), other (error)
    0x01, 0x01, 0x00,       // Cylinder-head-sector address of first sector in partition
    0x06,                   // Partition type - 0x01; 0x06 = FAT16 32MB+
    0x07, 0xFF, 0xE6,       // Cylinder-head-sector address of last sector in partition
    0x01, 0x00, 0x00, 0x00, // Logical block address of first sector in partition
    VIRTUAL_DISK_TOTAL_DISK_SIZE & 0xff, (VIRTUAL_DISK_TOTAL_DISK_SIZE >> 8) & 0xff, (VIRTUAL_DISK_TOTAL_DISK_SIZE >> 16) & 0xff, (VIRTUAL_DISK_TOTAL_DISK_SIZE >> 24) & 0xff,   // Length of partition in sectors (512MB 0x100000 sectors)
    
    // Entry 2                                                                                       
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x01CE
    // Entry 3
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x01DE
    // Entry 4
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x01EE
    
    // MBR signature
    0x55, 0xAA                                                                                      //0x01FE
};


// Physical Sector - 1, Logical Sector - 0
const BYTE BootSector[MEDIA_SECTOR_SIZE]  =
{
    0xEB, 0x3C, 0x90,			                        // Jump instruction
    'M', 'S', 'D', 'O', 'S', '5', '.', '0',             // OEM Name "MSDOS5.0"
    (MEDIA_SECTOR_SIZE&0xFF), (MEDIA_SECTOR_SIZE>>8),	// Bytes per sector - MEDIA_SECTOR_SIZE
    VIRTUAL_DISK_SECTORS_PER_CLUSTER, //0x40,	    	// Sectors per cluster
    VIRTUAL_DISK_NUM_RESERVED_SECTORS, 0x00,			// Reserved sector count
    0x01,			                                    // Number of FATs
    0x10, 0x00,                                         // Max number of root directory entries - 16 files allowed
    0x00, 0x00,			                                // Total sectors (0 = use 4-byte number later)
    0xF8,			                                    // Media Descriptor
    (VIRTUAL_DISK_NUM_FAT_SECTORS & 0xff), ((VIRTUAL_DISK_NUM_FAT_SECTORS >> 8) & 0xff),        // Sectors per FAT
    0x3F, 0x00,	                                        // Sectors per track
    0xFF, 0x00,                                         // Number of heads
    0x01, 0x00, 0x00, 0x00,		                        // Hidden sectors
    VIRTUAL_DISK_TOTAL_DISK_SIZE & 0xff, (VIRTUAL_DISK_TOTAL_DISK_SIZE >> 8) & 0xff, (VIRTUAL_DISK_TOTAL_DISK_SIZE >> 16) & 0xff, (VIRTUAL_DISK_TOTAL_DISK_SIZE >> 24) & 0xff,   // Total sectors
    0x00,			                                    // Physical drive number (0x80 = fixed disk, 0x00 = removable)
    0x00,			                                    // Reserved ("current head")
    0x29,			                                    // Signature
    0x01, 0x00, 0x00, 0x00,		                        // [39] ID (serial number)
    'F', 'T', 'L', ' ', 'D', 'R', 'I', 'V', 'E', ' ', ' ',	// [43] Volume Label - "FTL DRIVE"     // 'N', 'O', ' ', 'N', 'A', 'M', 'E', ' ', ' ', ' ', ' ',	// [43] Volume Label - "NO NAME    "
    'F', 'A', 'T', '1', '6', ' ', ' ', ' ',	            // FAT system "FAT16   "
    // Operating system boot code
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x55, 0xAA			//End of sector (0x55AA)
};


// FAT0
const BYTE FAT0[MEDIA_SECTOR_SIZE] =
{
    0xF8, 0xFF,	// FAT16 entry 0: Copy of the media descriptor 0xF8, remaining 8-bits 0xff
    0xFF, 0xFF,	// FAT16 entry 1: End of cluster chain marker (bit 15 = last shutdown was clean, bit 14 = no disk I/O errors were detected)
    0xFF, 0xFF,	// FAT16 entry 2: End of cluster chain marker (first file is one cluster long) (0x03, 0x00 == points to next entry in the cluster chain)
    //0xF7, 0xFF,	// FAT16 entry 3: Cluster marked as bad (OS won't try to use it)
};


// FATN
const BYTE FATN[MEDIA_SECTOR_SIZE] =
{
	0
};


// We only use a single sector for the root directory (maximum 16 files in the root directory)
const BYTE RootDirectory0[MEDIA_SECTOR_SIZE] =
{
    // + 0x00
    'F','T','L',' ','D','R','I','V','E',' ',' ', // Drive Name (11 characters, padded with spaces)
    0x08, 								// Specify this entry as a volume label
    0x00, 								// Reserved (case information)
    0x00,  					            // Create time fine resolution (10ms unit, 0-199)
    0x00, 0x00, 					    // Create time (00:00) [15-11=H, 10-5=M, 4-0=S/2]
    0x21, 0x3c, 						// Create date (01/01/10) [15-9=Y, 8-5=M1, 4-0=D1]
    0x21, 0x3c, 						// Last access date (01/01/10) [15-9=Y, 8-5=M1, 4-0=D1]
    0x00, 0x00, 						// EA-index
    0x00, 0x00,		 					// Last modified time (00:00) [15-11=H, 10-5=M, 4-0=S/2]
    0x21, 0x3c, 						// Last modified date (01/01/10) [15-9=Y, 8-5=M1, 4-0=D1]
    0x00, 0x00,              			// First FAT16 cluster (0 for volume labels)
    0x00, 0x00, 0x00, 0x00,             // File Size (0 for volume labels)
    
	/*
    // + 0x20
    'T','E','S','T',' ',' ',' ',' ',    // File name (exactly 8 characters)
    'T','X','T',                        // File extension (exactly 3 characters)
    0x21, 								// Specify this entry as a +A +R file
    0x00, 								// Reserved (case information)
    0x00,  					            // Create time fine resolution (10ms unit, 0-199)
    0x00, 0x00, 					    // Create time (00:00) [15-11=H, 10-5=M, 4-0=S/2]
    0x21, 0x3c, 						// Create date (01/01/10) [15-9=Y, 8-5=M1, 4-0=D1]
    0x21, 0x3c, 						// Last access date (01/01/10) [15-9=Y, 8-5=M1, 4-0=D1]
    0x00, 0x00, 						// EA-index
    0x00, 0x00,		 					// Last modified time (00:00) [15-11=H, 10-5=M, 4-0=S/2]
    0x21, 0x3c, 						// Last modified date (01/01/10) [15-9=Y, 8-5=M1, 4-0=D1]
    0x02, 0x00, 						// First FAT16 cluster (0 and 1 are reserved)
    12, 0, 0, 0, 						// File Size
	*/

    // + 0x40
};



char FsFtlOwnFormat(void)
{
	unsigned long i;
	char success = 1;

	success &= MDD_FTL_SectorWrite(SECTOR_MBR, (BYTE*)MasterBootRecord, 1);
	success &= MDD_FTL_SectorWrite(SECTOR_BOOT, (BYTE*)BootSector, 1);
	success &= MDD_FTL_SectorWrite(SECTOR_FAT0, (BYTE*)FAT0, 1);
	for (i = SECTOR_FAT0 + 1; i <= SECTOR_FATN; i++)
	{
		success &= MDD_FTL_SectorWrite(i, (BYTE*)FATN, 1);
	}
	success &= MDD_FTL_SectorWrite(SECTOR_ROOTDIR, (BYTE*)RootDirectory0, 1);
	//success &= MDD_FTL_SectorWrite(SECTOR_FILECONTENTS, (BYTE*)slack0, 1);

	return success;
}


#endif



// Format the device
char FsFtlFormat(char wipe, long serial, const char *volumeLabel)
{
	char error = 0;

	// Wipe the NAND device
	if (wipe != 0) { FtlDestroy(wipe - 1); }

#ifdef FSFTL_OWN_FORMAT
	FsFtlOwnFormat();
#else
	// Move first sector from 1 to FTL_FIRST_SECTOR to align disk clusters with NAND pages
	if (FSCreateMBR(FTL_FIRST_SECTOR, 1UL * FTL_LOGICAL_BLOCKS * FTL_PAGES_PER_BLOCK * FTL_SECTORS_PER_PAGE))
	{
		error |= 0x01;
	}
	if (FSformat(1, serial, (char *)volumeLabel))
	{
		error |= 0x02;
	}	
#endif

    if (volumeLabel != NULL)
    {
#ifdef FSFTL_WRITE_VOLUME_LABEL
        if (!FSfsetvolume(volumeLabel)) { error |= 0x04; }
#endif
    }
	return error;
}

#endif
