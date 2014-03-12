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
#include "Ftl/Ftl.h"
#include "Ftl/FsFtl.h"


// FSIO.c doesn't allocate buffers on non-embedded compiles, we'll allocate them here
#if !defined(__18CXX) && !defined (__C30__) && !defined (__PIC32MX__)
    BYTE gDataBuffer[MEDIA_SECTOR_SIZE];    // The global data sector buffer
    BYTE gFATBuffer[MEDIA_SECTOR_SIZE];     // The global FAT sector buffer

    // ...and it fails to implement these functions...
    BYTE ReadByte(BYTE* pBuffer, WORD index) { return pBuffer[index]; }
    WORD ReadWord(BYTE* pBuffer, WORD index) { return ((WORD)pBuffer[index + 1] << 8) | (WORD)pBuffer[index]; }
    DWORD ReadDWord(BYTE* pBuffer, WORD index) { return ((DWORD)ReadWord(pBuffer, index + 2) << 16) | (DWORD)ReadWord(pBuffer, index); }
#endif

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
        memcpy(buffer, fsftlPrefectchBuffer, FTL_SECTOR_SIZE);
        return 1;
    }
#endif
	FtlReadSector(sector_addr, buffer);
	return 1;		// Always return success
}



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



// Retrieve a line from a file (very inefficiently)
char *FSfgets(char *str, int num, FSFILE *stream)
{
	char *p;
	size_t n;

	if (num <= 0) { return NULL; }
	if (FSfeof(stream)) { str[0] = '\0'; return NULL; }

	for (p = str; !FSfeof(stream); p++, num--)
	{
		// Painfully read one byte at a time
		n = FSfread(p, 1, 1, stream);

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


/************************************************************
	Modification of the Microchip function FSfclose by Dan Jackson.
  Function:
    int FSfflush(FSFILE *fo)
  Summary:
    Update file information
  Conditions:
    File opened
  Input:
    fo -  Pointer to the file to flush
  Return Values:
    0 -   File flushed successfully
    EOF - Error flushing the file
  Side Effects:
    The FSerrno variable will be changed.
  Description:
    A function to flush data to the device without closing the
    file. Created from the FSfclose() function by removing the 
    portion that frees the memory and the line that clears the 
    write flag.
  ************************************************************/

int FSfflush(FSFILE   *fo)
{
    WORD        fHandle;
    int        error = 72;
#ifdef ALLOW_WRITES
    DIRENTRY    dir;
#endif

    FSerrno = CE_GOOD;
    fHandle = fo->entry;

#ifdef ALLOW_WRITES
    if(fo->flags.write)
    {
        if (gNeedDataWrite)
            if (flushData())
            {
                FSerrno = CE_WRITE_ERROR;
                return EOF;
            }

        // Write the current FAT sector to the disk
        WriteFAT (fo->dsk, 0, 0, TRUE);

        // Get the file entry
        dir = LoadDirAttrib(fo, &fHandle);

        if (dir == NULL)
        {
            FSerrno = CE_BADCACHEREAD;
            error = EOF;
            return error;
        }

      // update the time
#ifdef INCREMENTTIMESTAMP
        IncrementTimeStamp(dir);
#elif defined USERDEFINEDCLOCK
        dir->DIR_WrtTime = gTimeWrtTime;
        dir->DIR_WrtDate = gTimeWrtDate;
#elif defined USEREALTIMECLOCK
        CacheTime();
        dir->DIR_WrtTime = gTimeWrtTime;
        dir->DIR_WrtDate = gTimeWrtDate;
#endif

        dir->DIR_FileSize = fo->size;

        dir->DIR_Attr = (BYTE)(fo->attributes);

        // just write the last entry in
        if(Write_File_Entry(fo,&fHandle))
            error = 0;
        else
        {
            FSerrno = CE_WRITE_ERROR;
            error = EOF;
        }
    }
#endif

    // File opened in read mode
    if (error == 72)
        error = 0;

	// Flush the FTL buffers to the device
	FtlFlush(0);

    return(error);
} // FSfflush


/*********************************************************************************
	Modification of the Microchip function FSfwrite by Karim Ladha, auto-flushing and ECC option added by Dan Jackson
  Function:
    size_t FSfwriteSector(const void *ptr, FSFILE *stream, BOOL ecc)
  Summary:
    Write a 512-byte sector to a file
  Conditions:
    File opened in WRITE, APPEND, WRITE+, APPEND+, READ+ mode
  Input:
    ptr -     Pointer to source buffer
    stream -  Pointer to file structure
  Return:
    success or failure
  Side Effects:
    The FSerrno variable will be changed.
  Description:
	FSwriteSector() writes a whole 512-byte sector to the file. 
	If the current file position is not at a sector boundary then the sector will flush out
	any pre-existing sector data with '0xFF' padding. The global gBuffer will be emptied and
	NOT loaded with the new sector data.
  Remarks:
    None.
  *********************************************************************************/
BOOL FSfwriteSector(const void *ptr, FSFILE *stream, BOOL ecc)
{
    //DWORD       count = size * n;
    DISK   *    dsk;                 // pointer to disk structure
    CETYPE      error = CE_GOOD;
    DWORD       l;                     // absolute lba of sector to load
	BYTE ret;
#ifdef FSFTL_SECTOR_FLUSH
	unsigned char flush = 0;
#endif

    // see if the file was opened in a write mode
    if(!(stream->flags.write))
    {
        FSerrno = CE_READONLY;
        error = CE_WRITE_ERROR;
        return 0;
    }

    if (MDD_WriteProtectState())
    {
        FSerrno = CE_WRITE_PROTECTED;
        error = CE_WRITE_PROTECTED;
        return 0;
    }

    gBufferZeroed = FALSE;
    dsk = stream->dsk;

	// Flag for other instructions below
    if( stream->seek == stream->size )
         stream->flags.FileWriteEOF = TRUE;

	
    if (gNeedDataWrite)
    {	// Check if the global data buffer has any previous data in for other sectors to the one being written
		// If it has then flush it and assign ownership to this stream.
    	if (gBufferOwner != stream)// Current buffer is from another file - flush it
		{
            if (flushData())
            {
                FSerrno = CE_WRITE_ERROR;
                return 0;
            }
		}
		// If the last write was to the current sector used the gDataBuffer and was exactly one sector
		if (stream->pos==(dsk->sectorSize))
		{
	    	if (flushData())
	        {
	             FSerrno = CE_WRITE_ERROR;
	             return 0;
	        }
			// gNeedDataWrite = FALSE; from flushData()
		}
		// If the last write to the current sector used the gDataBuffer and was LESS THAN one sector
		else if ((gNeedDataWrite)&&(stream->pos!=(dsk->sectorSize))&&(stream->pos!=0)) 
		{
			unsigned int i;
	
			for (i=stream->pos;i<(dsk->sectorSize);i++)	// Pad out the remaining buffer
			{
				dsk->buffer[i] = 0xFF;
				stream->seek++;
				stream->pos++;
				if(stream->flags.FileWriteEOF)  // If we are writing at the end then increment the EOF
	                stream->size++;
			} 
		
			// Flush out the current sector - will call a sector write on gDataBuffer
	        if (flushData())
	        {
	            FSerrno = CE_WRITE_ERROR;
	            return 0;
	        }
		}
		else 
		{
			// not possible unless gNeedDataWrite was set but no data was written
			// assume we are at the start of an empty sector
			gNeedDataWrite = FALSE;
			goto end;
		}

		// Any data has now been flushed successfully by here
		// Increment the file length to the start of the next sector 
		stream->pos = 0;				// point to start of sector
   		stream->seek+=(dsk->sectorSize);// Seek fwd 1 sector
		stream->sec++;  				// point to the next sector in the file   
		if(stream->flags.FileWriteEOF)  // If we are writing at the end then increment the EOF
                stream->size+=(dsk->sectorSize);

		// get/add a new cluster if necessary
	    if (stream->sec == dsk->SecPerClus)
	    {
#ifdef FSFTL_SECTOR_FLUSH
			flush = (char)(((stream->size / 65536L % FSFTL_SECTOR_FLUSH) == 0) ? (char)1 : (char)0);
#endif
	        stream->sec = 0;
	        if(stream->flags.FileWriteEOF)
	        	{error = FILEallocate_new_cluster(stream, 0);} // add new cluster to the fill
	        else
	        	{error = FILEget_next_cluster( stream, 1);}	// find location in next cluster
	    }
	}

	// Check for any errors so far
	if(error != CE_GOOD)
    {
		return FALSE;
	}

end:
// Now we own the data buffer, and it is flushed, and the file pointer
// points to the start of an empty sector. We get the LBA of this sector first
// Then increment the file sector number and get a new cluster as needed and
// finally write the sector to the file. gNeedDataWrite == FALSE so the following 
// code will get called repetitively as you write sectors.

    l = Cluster2Sector(dsk,stream->ccls);
    l += (WORD)stream->sec;      // add the sector number to it

	// get a new cluster if necessary
    if ((stream->sec + 1) == dsk->SecPerClus)
    {
#ifdef FSFTL_SECTOR_FLUSH
		unsigned short flushEveryNClusters = FSFTL_SECTOR_FLUSH * ((unsigned int)(65536UL / MEDIA_SECTOR_SIZE) / stream->dsk->SecPerClus);		// Flush every FSFTL_SECTOR_FLUSH * 64 kB, (128 sectors per 64kB)
		if (flushEveryNClusters != 0)
		{
			unsigned long cluster = (stream->seek + dsk->sectorSize) / MEDIA_SECTOR_SIZE / stream->dsk->SecPerClus;
			flush = (char)(((cluster % flushEveryNClusters) == 0) ? (char)1 : (char)0);
		}
#endif
        if(stream->flags.FileWriteEOF)
        	{error = FILEallocate_new_cluster(stream, 0);} // add new cluster to the fill
        else
        	{error = FILEget_next_cluster( stream, 1);}	// find location in next cluster

	    // Check for any errors so far
	    if(error != CE_GOOD)
        {
		    return FALSE;
	    }
    }

	// Check for any errors so far
	if(error != CE_GOOD)
    {
		return FALSE;
	}

	stream->sec++;  				// point to the next sector in the file   
	stream->pos = 0;// Set the new position to the end of the sector
   	stream->seek+=(dsk->sectorSize);// Seek fwd 1 sector
	if(stream->flags.FileWriteEOF)  // If we are writing at the end then increment the EOF
            stream->size+=(dsk->sectorSize);

    if (stream->sec == dsk->SecPerClus) stream->sec = 0;

	// Now.....
	//stream->Pos == SectorSize
	//gBufferOwner == stream
    //gBufferZeroed == FALSE
	//gNeedDataWrite == FALSE
	gLastDataSectorRead = 0xFFFFFFFF ; 	// NULL, since this function is NOT read modify write and buffers are flushed 

	// Now ready to write into sector 'l'
	ret = FtlWriteSector(l, (unsigned char *)ptr, ecc);

#ifdef FSFTL_SECTOR_FLUSH
	if (flush)
	{
		FSfflush(stream);
	}
#endif

	// Ignore 'ret', always return true
	return TRUE;
}


// [dgj] Disk free function
unsigned long FSDiskFree(void)
{
    DISK *disk;
    DWORD numEmptyClusters = 0;
    DWORD EndClusterLimit, ClusterFailValue;
    DWORD c;

    disk = &gDiskData;
    switch (disk->type)
    {
#ifdef SUPPORT_FAT32
        case FAT32:
            EndClusterLimit = END_CLUSTER_FAT32;
            ClusterFailValue = CLUSTER_FAIL_FAT32;
            break;
#endif
        case FAT12:
            EndClusterLimit = END_CLUSTER_FAT12;
            ClusterFailValue = CLUSTER_FAIL_FAT16;
            break;
        case FAT16:
        default:
            EndClusterLimit = END_CLUSTER_FAT16;
            ClusterFailValue = CLUSTER_FAIL_FAT16;
            break;
    }

    // Scan through the FAT, counting empty clusters
    for (c = 2; c < disk->maxcls+2; c++)
    {
        DWORD value;
        
        // Read FAT entry
        value = ReadFAT(disk, c);

        // Failed to read cluster or end cluster limit
        if (value == ClusterFailValue || value == EndClusterLimit) { break; }

        // If empty cluster found
        if (value == CLUSTER_EMPTY) { numEmptyClusters++; }
    }

    return numEmptyClusters * disk->SecPerClus * disk->sectorSize;
}



// Get a character
int FSfgetc(FSFILE *fp)
{
    unsigned char c;
    if (FSfread(&c, 1, 1, fp) != 1) return -1;
    return (int)c;
}


// Put a character
int FSfputc(int character, FSFILE *fp)
{
    unsigned char c = character;
    if (FSfwrite(&c, 1, 1, fp) != 1) return -1;
    return (int)c;
}

// Get/put word/dword
void FSfputshort(short v, FSFILE *fp) { FSfputc((char)((v >> 0) & 0xff), fp); FSfputc((char)((v >> 8) & 0xff), fp); }
void FSfputlong(long v, FSFILE *fp) { FSfputc((char)((v >> 0) & 0xff), fp); FSfputc((char)((v >> 8) & 0xff), fp); FSfputc((char)((v >> 16) & 0xff), fp); FSfputc((char)((v >> 24) & 0xff), fp); }
short FSfgetshort(FSFILE *fp) { unsigned short v = 0; v |= (((unsigned short)FSfgetc(fp)) << 0); v |= (((unsigned short)FSfgetc(fp)) << 8); return (short)v; }
long FSfgetlong(FSFILE *fp) { unsigned long v = 0; v |= (((unsigned long)FSfgetc(fp)) << 0); v |= (((unsigned long)FSfgetc(fp)) << 8); v |= (((unsigned long)FSfgetc(fp)) << 16); v |= (((unsigned long)FSfgetc(fp)) << 24); return (long)v; }


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


// [experimental - may cause file system problems] Write volume label after formatting
#ifdef FSFTL_WRITE_VOLUME_LABEL

// Sets the volume label of the drive -- must only be used after formatting, as it will not remove any existing volume label
char FSfsetvolume( const char * fileName)
{
    FILEOBJ    filePtr;
#ifndef FS_DYNAMIC_MEM
    int      fIndex;
#endif
    WORD    fHandle = 0;

#ifdef FS_DYNAMIC_MEM
    filePtr = (FILEOBJ) FS_malloc(sizeof(FSFILE));
#else
    filePtr = NULL;
    // Pick available file structure
    for( fIndex = 0; fIndex < FS_MAX_FILES_OPEN; fIndex++ )
    {
        if( gFileSlotOpen[fIndex] )   //this slot is available
        {
            gFileSlotOpen[fIndex] = FALSE;
            filePtr = &gFileArray[fIndex];
			break;
        }
    }
    if( filePtr == NULL )
    {
        FSerrno = CE_TOO_MANY_FILES_OPEN;
        return 0;      //no file structure slot available
    }
#endif

	#if defined(SUPPORT_LFN)
		#if defined(FS_DYNAMIC_MEM)
			filePtr -> utf16LFNptr = (unsigned short int *)FS_malloc(514);
		#else
			filePtr->utf16LFNptr = &lfnData[fIndex][0];
		#endif
    #endif

    //Format the source string.
    if( !FormatFileName(fileName, filePtr, 0) )
    {
		#ifdef FS_DYNAMIC_MEM
			#if defined(SUPPORT_LFN)
				FS_free((unsigned char *)filePtr->utf16LFNptr);
			#endif
        	FS_free( (unsigned char *)filePtr );
		#else
        	gFileSlotOpen[fIndex] = TRUE;   //put this slot back to the pool
		#endif
		FSerrno = CE_INVALID_FILENAME;
        return 0;   //bad filename
    }

    filePtr->dsk = &gDiskData;
    filePtr->cluster = 0;
    filePtr->ccls    = 0;
    filePtr->entry = 0;
    filePtr->attributes = ATTR_VOLUME;

    // start at the root directory
    filePtr->dirclus = FatRootDirClusterValue;
    filePtr->dirccls = FatRootDirClusterValue;

    // CreateFileEntry ignores the attributes, change the attributes
    if (CreateFileEntry (filePtr, &fHandle, 0, FALSE) == CE_GOOD)
    {
        DIRENTRY dir;

        FSerrno = CE_GOOD;
        fHandle = filePtr->entry;
        filePtr->dirccls = filePtr->dirclus;

        // Get the file entry
        dir = LoadDirAttrib(filePtr, &fHandle);
        if (dir == NULL) { FSerrno = CE_BADCACHEREAD; return 0; }

        // Replace the attributes
        dir->DIR_Attr = ATTR_VOLUME;

        // Rewrite entry
        if(!Write_File_Entry(filePtr, &fHandle)) { return 0; }
    }

    return 1;
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

