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

#include "FSConfig.h" // Moved here to allow file system selection- not all functions supported between two fs modes 
#ifdef USE_FAT_FS
	#include "FatFs/FatFsIo.h"
#else
	#include "MDD File System/FSIO.h"
#endif
#include "Utils/FSutils.h"

// FSIO.c doesn't allocate buffers on non-embedded compiles, we'll allocate them here
#if !defined(__18CXX) && !defined (__C30__) && !defined (__PIC32MX__)
    unsigned char gDataBuffer[MEDIA_SECTOR_SIZE];    // The global data sector buffer
    unsigned char gFATBuffer[MEDIA_SECTOR_SIZE];     // The global FAT sector buffer

    // ...and it fails to implement these functions...
    unsigned char ReadByte(unsigned char* pBuffer, unsigned short index) { return pBuffer[index]; }
    unsigned short ReadWord(unsigned char* pBuffer, unsigned short index) { return ((unsigned short)pBuffer[index + 1] << 8) | (unsigned short)pBuffer[index]; }
    unsigned int ReadDWord(unsigned char* pBuffer, unsigned short index) { return ((unsigned int)ReadWord(pBuffer, index + 2) << 16) | (unsigned int)ReadWord(pBuffer, index); }
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
#ifndef USE_FAT_FS
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

	// Flush the underlying device
#ifdef FS_FLUSH
    FS_FLUSH();
#endif

    return(error);
} // FSfflush
#endif

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
#ifndef USE_FAT_FS
BOOL FSfwriteSector(const void *ptr, FSFILE *stream, BOOL ecc)
{
    //DWORD       count = size * n;
    DISK   *    dsk;                 // pointer to disk structure
    CETYPE      error = CE_GOOD;
    DWORD       l;                     // absolute lba of sector to load
	BYTE ret;
#ifdef FS_SECTOR_FLUSH
	unsigned char flush = 0;
#endif
#ifdef _WIN32
	printf("--- %d\n", stream->sec);
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
#ifdef _WIN32
			printf("NEW CLUSTER (a) %d\n", stream->ccls);
#endif
#ifdef FS_SECTOR_FLUSH
			flush = (char)(((stream->size / 65536L % FS_SECTOR_FLUSH) == 0) ? (char)1 : (char)0);
#endif
	        stream->sec = 0;
	        if(stream->flags.FileWriteEOF)
	        	{error = (CETYPE)FILEallocate_new_cluster(stream, 0);} // add new cluster to the fill
	        else
	        	{error = (CETYPE)FILEget_next_cluster( stream, 1);}	// find location in next cluster
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
#ifdef _WIN32
		printf("NEW CLUSTER (b) %d\n", stream->ccls);
#endif
#ifdef FS_SECTOR_FLUSH
		unsigned short flushEveryNClusters = FS_SECTOR_FLUSH * ((unsigned int)(65536UL / MEDIA_SECTOR_SIZE) / stream->dsk->SecPerClus);		// Flush every FS_SECTOR_FLUSH * 64 kB, (128 sectors per 64kB)
		if (flushEveryNClusters != 0)
		{
			unsigned long cluster = (stream->seek + dsk->sectorSize) / MEDIA_SECTOR_SIZE / stream->dsk->SecPerClus;
			flush = (char)(((cluster % flushEveryNClusters) == 0) ? (char)1 : (char)0);
		}
#endif
        if(stream->flags.FileWriteEOF)
        	{error = (CETYPE)FILEallocate_new_cluster(stream, 0);} // add new cluster to the fill
        else
        	{error = (CETYPE)FILEget_next_cluster( stream, 1);}	// find location in next cluster

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
    #ifdef FS_WRITE_SECTOR_ECC
        ret = FS_WRITE_SECTOR_ECC(l, (unsigned char *)ptr, ecc);
    #else
        ret = MDD_SectorWrite(l, (unsigned char *)ptr, 0);
    #endif
    
#ifdef FS_SECTOR_FLUSH
	if (flush)
	{
#ifdef _WIN32
		printf("+++ FLUSH!\n");
#endif
		FSfflush(stream);
	}
#endif

	// Ignore 'ret', always return true
	return TRUE;
}
#endif

// [dgj] Disk free function
unsigned long FSDiskFree(void)
{
    DISK *disk;
    DWORD numEmptyClusters = 0;
    DWORD EndClusterLimit, ClusterFailValue;
    DWORD c;

	if (!gDiskData.mount) return 0;

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

    return ((numEmptyClusters) * (disk->SecPerClus) * (disk->sectorSize));
}

// [dgj] Disk free function
unsigned char FSDiskFull(void)
{
    DISK *disk;
    DWORD EndClusterLimit, ClusterFailValue;
    DWORD c;
	
	if (!gDiskData.mount) return TRUE;

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
        if (value == CLUSTER_EMPTY) { return FALSE; }
    }

    return TRUE;
}

unsigned char FSDiskSectorsPerCluster(void)
{
    int sectorsPerCluster = gDiskData.SecPerClus;
    return (unsigned char)sectorsPerCluster;
}

unsigned long FSDiskDataSector(void)
{
    unsigned long diskDataSector = gDiskData.data;
    return diskDataSector;
}

#ifndef USE_FAT_FS
unsigned long FSFileOffsetToLogicalSector(const char *filename, unsigned long offset)
{
    FSFILE *fp;
	unsigned long sector;

    // Start at the first cluster of the file
    fp = FSfopen(filename, "r");
    if (fp == NULL) { return 0xFFFFFFFFul; }

	// Seek file pointer
    if (FSfseek(fp, offset, SEEK_SET) != 0) { return 0xFFFFFFFFul; }

    // The logical sector is the (cluster - 2) * sectors_per_cluster + sector_within_cluster + disk_data_offset
    sector = (((unsigned long)fp->ccls - 2) * fp->dsk->SecPerClus) + fp->sec + fp->dsk->data;

    FSfclose(fp);

    return sector;
}
#endif

#ifndef USE_FAT_FS
char FSFollowClusterChain(const char *filename, void *reference, void (*callback)(void *, unsigned short, unsigned long))
{
    DWORD cluster, lastCluster, failCluster;
    char ret = TRUE;
    unsigned short index;
    FSFILE *fp;
    
    // Start at the first cluster of the file
    fp = FSfopen(filename, "r");
    if (fp == NULL) { return 0; }
    cluster = fp->cluster;

    // Use the file system's special values
    switch (fp->dsk->type)
    {
#ifdef SUPPORT_FAT32
        case FAT32: failCluster = CLUSTER_FAIL_FAT32; lastCluster = LAST_CLUSTER_FAT32; break;
#endif
        case FAT12: failCluster = CLUSTER_FAIL_FAT16; lastCluster = LAST_CLUSTER_FAT12; break;
        case FAT16: default: failCluster = CLUSTER_FAIL_FAT16; lastCluster = LAST_CLUSTER_FAT16; break;
    }

    // Follow each cluster in the chain
    for (index = 0; cluster < lastCluster && index < 0xffff; index++)
    {
        if (cluster <= 1) { ret = 0; break; }
        if (cluster > fp->dsk->maxcls+2) { ret = 0; break; }
        
        // User callback
        if (callback != NULL)
        {
            callback(reference, index, cluster);
        }
        
        cluster = ReadFAT(fp->dsk, cluster);
        if (cluster == failCluster) { ret = 0; break; }
    }

    FSfclose(fp);

    return ret;
}
#endif

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



// [experimental - may cause file system problems] Write volume label after formatting
// Sets the volume label of the drive -- must only be used ONCE after formatting, as it will not remove any existing volume label
#ifndef USE_FAT_FS
char FSfsetvolume(const char *fileName)
{
    FILEOBJ    filePtr;
#ifndef FS_DYNAMIC_MEM
    int      fIndex;
#endif
    WORD    fHandle = 0;

    // TODO: Find and remove any existing volume label    
    
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



/*****************************************************************************
  Function:
    BYTE MDD_SDSPI_MultipleSectorWrite (DWORD sector_addr, BYTE * buffer, int numSectors, BYTE allowWriteToZero)
  Summary:
    Writes a sector of data to an SD card.
  Conditions:
    The MDD_SectorWrite function pointer must be pointing to this function.
  Input:
    sector_addr -      The address of the sector on the card.
    buffer -           The buffer with the data to write.
    allowWriteToZero -
                     - TRUE -  Writes to the 0 sector (MBR) are allowed
                     - FALSE - Any write to the 0 sector will fail.
  Return Values:
    TRUE -  The sector was written successfully.
    FALSE - The sector could not be written.
  Side Effects:
    None.
  Description:
    The MDD_SDSPI_SectorWrite function writes one sector of data (512 bytes) 
    of data from the location pointed to by 'buffer' to the specified sector of 
    the SD card.
  Remarks:
    The card expects the address field in the command packet to be a byte address.
    The sector_addr value is ocnverted to a byte address by shifting it left nine
    times (multiplying by 512).
  ***************************************************************************************/

BYTE MDD_FAKE_MultipleSectorWrite(DWORD sector_addr, BYTE* buffer, int numSectors, BYTE allowWriteToZero)
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


#ifdef USE_SD_INTERFACE_WITH_SPI
//unsigned long debugWriteBuffer = 0;
BYTE MDD_SDSPI_MultipleSectorWrite(DWORD sector_addr, BYTE FSUTILS_EDS * buffer, int numSectors, BYTE allowWriteToZero)
{
    static ASYNC_IO info;
    BYTE status;
	int numWritten = 0;

//debugWriteBuffer = (unsigned long)buffer;

    if (allowWriteToZero == FALSE && sector_addr == 0x00000000) { return FALSE; }    
    
	if (numSectors == 0) { return TRUE; }

	// Write all sectors
    //Initialize structure so we write multiple sectors worth of data.
    info.wNumBytes = MEDIA_SECTOR_SIZE;							// 512 byte chunks
    info.dwBytesRemaining = MEDIA_SECTOR_SIZE * numSectors;
    info.pBuffer = buffer;
    info.dwAddress = sector_addr;
    info.bStateVariable = ASYNC_WRITE_QUEUED;

status = MDD_SDSPI_AsyncWriteTasks(&info);
if(status == ASYNC_WRITE_COMPLETE) return TRUE;	// Should not happen!
if(status == ASYNC_WRITE_ERROR) return FALSE;

    //Repeatedly call the write handler until the operation is complete (or a failure/timeout occurred).
    for (;;)
    {
        status = MDD_SDSPI_AsyncWriteTasks(&info);
		if (status == ASYNC_WRITE_SEND_PACKET)
		{
numWritten++;
info.pBuffer += MEDIA_SECTOR_SIZE;
//			info.pBuffer = buffer + (MEDIA_SECTOR_SIZE * numSectors) - info.dwBytesRemaining;	
//			info.pBuffer = buffer + (info.dwAddress - sector_addr) * MEDIA_SECTOR_SIZE;
if (numWritten >= numSectors)
{
//	return FALSE;
}
		}
        else if(status == ASYNC_WRITE_COMPLETE)
        {
            break;
        }    
        else if(status == ASYNC_WRITE_ERROR)
        {
            return FALSE;
        }
    }    
    return TRUE;
}    
#endif




#ifdef FS_USE_MULTI_SECTOR
	#ifdef USE_SD_INTERFACE_WITH_SPI
		#define MDD_MultipleSectorWrite MDD_SDSPI_MultipleSectorWrite
	#else
		#define MDD_MultipleSectorWrite MDD_FAKE_MultipleSectorWrite
		#warning "No efficient multiple-sector write available."
	#endif
#else
	#define MDD_MultipleSectorWrite MDD_FAKE_MultipleSectorWrite
	#ifdef USE_SD_INTERFACE_WITH_SPI
		#warning "Not using the efficient SD-SPI multiple-sector write - define FS_USE_MULTI_SECTOR in FSconfig.h"
	#endif
#endif



#ifndef USE_FAT_FS

#ifdef ENABLE_SCRAMBLE
char gFSScramble = 0;
#endif

// Write multiple sectors
BOOL FSfwriteMultipleSectors(const void FSUTILS_EDS *ptr, FSFILE *stream, int count, BOOL ecc)
{
    DISK   *dsk;                 // pointer to disk structure
    CETYPE error = CE_GOOD;
    DWORD l;                     // absolute lba of sector to load
	BYTE ret;
    int written = 0;
	unsigned char FSUTILS_EDS *writePointer;

    if(!(stream->flags.write)) { FSerrno = CE_READONLY; error = CE_WRITE_ERROR; return 0; }
    if (MDD_WriteProtectState()) { FSerrno = CE_WRITE_PROTECTED; error = CE_WRITE_PROTECTED; return 0; }

    gBufferZeroed = FALSE;
    dsk = stream->dsk;

	// Flag for other instructions below
    if( stream->seek == stream->size ) { stream->flags.FileWriteEOF = TRUE; }
	
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
	        stream->sec = 0;
	        if(stream->flags.FileWriteEOF)
	        	{error = FILEallocate_new_cluster(stream, 0);} // add new cluster to the fill
	        else
	        	{error = FILEget_next_cluster( stream, 1);}	// find location in next cluster
	    }
	}

	// Check for any errors so far
	if(error != CE_GOOD) { return FALSE; }

end:
// Now we own the data buffer, and it is flushed, and the file pointer
// points to the start of an empty sector. We get the LBA of this sector first
// Then increment the file sector number and get a new cluster as needed and
// finally write the sector to the file. gNeedDataWrite == FALSE so the following 
// code will get called repetitively as you write sectors.

	while (count > 0)
	{
		int contiguous;

	    l = Cluster2Sector(dsk,stream->ccls);
	    l += (WORD)stream->sec;      // add the sector number to it
	
		// Check for any errors so far
		if(error != CE_GOOD) { return FALSE; }
	
		// Calculate maximum number of contiguous entries that can be written to this cluster
		contiguous = dsk->SecPerClus - stream->sec;
		
		// At most, the number of remaining entries
		if (contiguous > count) { contiguous = count; }
		
		// Sanity check, this should not be the case...
		if (contiguous <= 0) { break; }
	
		// get a new cluster if necessary
	    if ((stream->sec + contiguous) >= dsk->SecPerClus)
	    {
	        if(stream->flags.FileWriteEOF)
	        	{error = FILEallocate_new_cluster(stream, 0);} // add new cluster to the fill
	        else
	        	{error = FILEget_next_cluster( stream, 1);}	// find location in next cluster

	        // Check for cluster allocation fail
	        if(error != CE_GOOD) { return FALSE; }
	    }
	
		// Update entries
		stream->sec += contiguous; 	// point to the next sector in the file   
		stream->pos = 0;// Set the new position to the end of the sector
	   	stream->seek+=(dsk->sectorSize * contiguous);// Seek fwd N sectors
		if(stream->flags.FileWriteEOF)  // If we are writing at the end then increment the EOF
	            stream->size+=(dsk->sectorSize * contiguous);
		if (stream->sec >= dsk->SecPerClus) stream->sec = 0;	// (must have allocated a new cluster already above)
		gLastDataSectorRead = 0xFFFFFFFF ; 	// NULL, since this function is NOT read modify write and buffers are flushed 
	
		// Now ready to write into sectors starting at 'l'
		writePointer = (unsigned char FSUTILS_EDS *)ptr + (written * MEDIA_SECTOR_SIZE);
#ifdef ENABLE_SCRAMBLE
gSDScramble = (gFSScramble == 1 || gFSScramble == 2) ? gFSScramble : 0;
#endif
		ret = MDD_MultipleSectorWrite (l, writePointer, contiguous, 0);
#ifdef ENABLE_SCRAMBLE
gSDScramble = 0;
#endif
	
		// We've written this number now
		count -= contiguous;
        written += contiguous;
	}	

	// Ignore 'ret', always return true
	return TRUE;
}

#else

// Write multiple sectors
BOOL FSfwriteMultipleSectors(const void *ptr, FSFILE *stream, int count, BOOL ecc)
{
	const unsigned char *p = (const unsigned char *)ptr;
	int i;
	BOOL ret = TRUE;

#warning "Using fake version of FSfwriteMultipleSectors()"

	for (i = 0; i < count; ++i)
	{
//LED_G = !LED_G;
		ret &= FSfwriteSector(p, stream, ecc);
		p += MEDIA_SECTOR_SIZE;
		if (ret == FALSE) break; // There was an error
	}

	return ret;
}

#endif



//EOF
