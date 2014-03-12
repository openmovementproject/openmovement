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

// FatFs compatibility layer to replicate the Microchip Memory Disk Drive File System API
// Dan Jackson, 2013


// Includes
#include "stdlib.h"
#include "FatFs/FatFsIo.h"

// State tracking structure
typedef struct
{
    FATFS fatfs;
#ifndef FS_DYNAMIC_MEM
    char filesInUse[FS_MAX_FILES_OPEN];
    FIL files[FS_MAX_FILES_OPEN];
#endif
    unsigned long time;
} fatfsio_t;

static fatfsio_t fatfsio = {{0}};



/* Unicode support functions */
#if _USE_LFN							/* Unicode - OEM code conversion */
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

#if _USE_LFN == 3						/* Memory functions */
/* Allocate memory block */
void* ff_memalloc (UINT msize)
{ 
	return malloc((size_t)msize);
}
/* Free memory block */
void ff_memfree (void* mblock)
{
	return free(mblock);
}
#endif
#endif


// Initialize and mounts the disk. Returns TRUE if successful.
int FSInit(void)
{
    FRESULT res;
    int i;
    
    // Clear all files-in-use
#ifndef FS_DYNAMIC_MEM
    for (i = 0; i < FS_MAX_FILES_OPEN; i++)
    {
        fatfsio.filesInUse[i] = 0;
    } 
#endif
    // Initialize and mount the disk
    res = f_mount(0, &fatfsio.fatfs);
    
    return (res == FR_OK);
}

// Open a file and, if successful, returns a pointer to a newly allocated FSFILE object.
// The file is created if we are writing (if it already exists, it is removed first).
// The file pointer is set to the end of the file in append mode.
FSFILE *FSfopen(const char *filename, const char *mode)
{
	FIL *fil = NULL;
    FSFILE *file = NULL;
    //const char *c;
    unsigned char flags;
    char append;
    int i;
    
    // Opening flags
    flags = 0;
    append = 0;
    if (mode[0] == 'r')
    { 
        flags = FA_READ | FA_OPEN_EXISTING;
        if (mode[1] == '+') { flags |= FA_WRITE; }
    }
#ifdef ALLOW_WRITES
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
#endif
    
    // Open using the next available entry
#ifndef FS_DYNAMIC_MEM
    for (i = 0; i <= FS_MAX_FILES_OPEN; i++)
    {
		if (fatfsio.filesInUse[i])continue;
        fil = &fatfsio.files[i];
		break;
	}
#else
	fil = (FIL*)malloc(sizeof(FIL));
#endif
	// Check we have a valid memory location
	if (!fil) return NULL;

    FRESULT res;
    res = f_open(fil, filename, flags);
    if (res == FR_OK)
    {
        if (append)
        {
            f_lseek(fil, f_size(fil));
        }
#ifndef FS_DYNAMIC_MEM
        fatfsio.filesInUse[i] = 1;
#endif
        file = (FSFILE *)fil;
    }
    return file;
}

// Close the file and release the allocated FSFILE object.
// Returns 0 if successful, -1 otherwise.
int FSfclose(FSFILE *file)
{
    FRESULT res;
	int i;
    
    // Close the file
    res = f_close((FIL *)file);

#ifndef FS_DYNAMIC_MEM       
    // Find the entry to remove
    for (i = 0; i < FS_MAX_FILES_OPEN; i++)
    {
        if (file == (FSFILE *)&fatfsio.files[i])
        {
            fatfsio.filesInUse[i] = 0;
        }
    }
#else
	if(file)free(file);
#endif
    
    return (res == FR_OK) ? 0 : -1;
}

// Sets the file position to the start of the file.
void FSrewind(FSFILE *file)
{
    f_lseek((FIL *)file, 0);
}

// Read from the file
size_t FSfread(void *buffer, size_t size, size_t count, FSFILE *file)
{
    FRESULT res;
    unsigned int length;
    unsigned int result = 0;
    length = size * count;      // This limits the read to 64kB
    if (length == 0) { return 0; }
    res = f_read((FIL *)file, buffer, length, &result);
    if (size > 1) { result /= size; }
    return result;
}

// Seek within the file. 
int FSfseek(FSFILE *file, long offset, int whence)
{
    FRESULT res;
    if (whence == SEEK_CUR)
    {
        offset += f_tell((FIL *)file);
    }
    else if (whence == SEEK_END)
    {
        offset = f_size((FIL *)file) - offset;
        //if (offset < 0) { offset = 0; }
    }
    res = f_lseek((FIL *)file, offset);
    return (res == FR_OK) ? 0 : -1;
}

// Gets the current location in a file.
long FSftell(FSFILE *file)
{
    return f_tell((FIL *)file);
}

// Checks whether the current file position is at the end.
int FSfeof(FSFILE *file)
{
    return f_eof((FIL *)file);
}


#ifdef ALLOW_FORMATS

#if !(_USE_MKFS > 0)
#error "_USE_MKFS not defined."
#endif

// Format a drive. Mode 0 erases the FAT and drive root, mode 1 creates a new boot sector. 
// This will use the FORMAT_SECTORS_PER_CLUSTER macro.
// Returns 0 if successful, -1 otherwise.
int FSformat(char mode, long int serialNumber, char *volumeId)
{
    FRESULT res;
    unsigned int allocationUnit = 0;
    #ifdef FORMAT_SECTORS_PER_CLUSTER
    allocationUnit = (unsigned int)FORMAT_SECTORS_PER_CLUSTER * (unsigned int)MEDIA_SECTOR_SIZE;
    #endif
    res = f_mkfs(0, 0, allocationUnit);
    return (res == FR_OK) ? 0 : -1;
}

#endif


#ifdef ALLOW_WRITES

/*
// Set the file attributes. Returns 0 if successful, -1 otherwise.
int FSattrib(FSFILE *file, unsigned char attributes)
{
    FRESULT res;
    const unsigned char *filename;
    unsigned char attr;
    
    filename = ???;
    
    attr = 0;    
    if (attributes & ATTR_READ_ONLY) { attr |= AM_RDO; }
    if (attributes & ATTR_HIDDEN)    { attr |= AM_HID; }
    if (attributes & ATTR_SYSTEM)    { attr |= AM_SYS; }
    if (attributes & ATTR_ARCHIVE)   { attr |= AM_ARC; }
    
    res = f_chmod(filename, attr, 0xff);
    return (res == FR_OK) ? 0 : -1;
}
*/

// Rename a file. Returns 0 if successful, -1 otherwise.
/*
int FSrename(const char *filename, FSFILE *file)
{
    const char *oldName = NULL;
    oldName = ???;
    return (f_rename(oldName, filename) == FR_OK) ? 0 : -1;
}
*/

// Delete a file. Returns 0 if successful, -1 otherwise.
int FSremove(const char *filename)
{
    return (f_unlink(filename) == FR_OK) ? 0 : -1;
}

// Write data to a file
size_t FSfwrite(const void *buffer, size_t size, size_t count, FSFILE *file)
{
    FRESULT res;
    unsigned int length;
    unsigned int result = 0;
    length = size * count;      // This limits the read to 64kB
    if (length == 0) { return 0; }
    res = f_write((FIL *)file, buffer, length, &result);
    if (size > 1) { result /= size; }
    return result;
}

#endif



#ifdef ALLOW_DIRS

// Change the current working directory. Returns 0 if successful, -1 otherwise.
int FSchdir(char *path)
{
    return (f_chdir(path) == FR_OK) ? 0 : -1;
}

// Gets the current working directory. Returns 0 if successful, -1 otherwise.
char *FSgetcwd(char *path, int maxPath)
{
    FRESULT res;
    res = f_getcwd(path, maxPath);
    return (res == FR_OK) ? path : NULL;
}

#ifdef ALLOW_WRITES

// Create a directory. Returns 0 if successful, -1 otherwise.
int FSmkdir(char *path)
{
    return (f_mkdir(path) == FR_OK) ? 0 : -1;
}

// Remove a directory. Returns 0 if successful, -1 otherwise.
int FSrmdir(char * path, unsigned char rmsubdirs)
{
    return (f_unlink(path) == FR_OK) ? 0 : -1;
}

#endif

#endif


#ifdef USEREALTIMECLOCK
// TODO: Make not depend on this...
#include "Peripherals/Rtc.h"
#endif


unsigned long get_fattime(void)
{
#ifdef INCREMENTTIMESTAMP
    // Completely different to Microchip implementation (that increments the individual file's timestamp)
    fatfsio.time++;
#endif

#ifdef USEREALTIMECLOCK
	{
		DateTime now = RtcNow();
		unsigned long offset = ((unsigned long)2000-1980)<<(26);
		now = (unsigned long)now>>1;
		now+=offset; // Now it is in FAT format
		fatfsio.time = now;
	}
#endif

    return fatfsio.time;
}

#ifdef USERDEFINEDCLOCK
// Manually set the clock variables
int SetClockVars(unsigned int year, unsigned char month, unsigned char day, unsigned char hour, unsigned char minute, unsigned char second)
{
    if (!((year >= 1980 && year <= 2106) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31) && (hour <= 23) && (minute <= 59) && (second <= 59)))
    {
        fatfsio.time = 0;
        return -1;
    }    
    fatfsio.time = (unsigned int)((year - 1980) << 9) | ((unsigned int)month << 5) | day;
    fatfsio.time <<= 16;
    fatfsio.time |= ((unsigned int)hour << 11) | ((unsigned int)minute << 5) | (second/2);
    return 0;
}
#endif




#ifdef ALLOW_FILESEARCH

// Perform initial search for a file. Returns 0 if a file was found, or -1 if no matching files were found.
int FindFirst(const char *filename, unsigned int attributes, SearchRec *search)
{
	const char *c;
	const char *filenamePart;

    search->attributes = attributes;

#if _USE_LFN
	search->fno.lfname = search->lfn;
	search->fno.lfsize = sizeof(search->lfn);  //_MAX_LFN + 1;
#endif

	// Find the path part
	search->path[0] = '\0';
	filenamePart = filename;
	for (c = filename; *c != '\0'; c++) 
	{ 
		if (*c == '/' || *c == '\\') 
		{  
			int len = (int)(c - filename);
			if (len > sizeof(search->path) - 1) { len = sizeof(search->path) - 1; }
			memcpy(search->path, filename, len);
			search->path[len] = '\0';
			filenamePart = c + 1;
		}
	}

	// Find the filename part
	{
		// Check for overflow and NULL-terminate
		int len = sizeof(filenamePart);
		if (len > sizeof(search->filespec) - 1) { len = sizeof(search->filespec) - 1; }
		memcpy(search->filespec, filenamePart, len);
		search->filespec[len] = '\0';
	}

	search->res = f_opendir(&search->dir, search->path);
	if (search->res != FR_OK) 
	{
		return -1;
	}

	return FindNext(search);
}

// Continue search after FindFirst. Returns 0 if another file was found, or -1 if no further matching files were found.
int FindNext(SearchRec *search)
{
    int ret = -1;

	for (;;)
	{
        search->res = f_readdir(&search->dir, &search->fno);                   // Read a directory item
        if (search->res != FR_OK || search->fno.fname[0] == 0) { ret = -1; break; }  // Break on error or end of dir
        if (search->fno.fname[0] == '.') continue;             // Ignore dot entry 
#if _USE_LFN
        search->filename = *search->fno.lfname ? search->fno.lfname : search->fno.fname;
#else
        search->filename = search->fno.fname;
#endif

		// Compare directories
		// TODO: Compare attributes properly
		if ( (search->attributes & ATTR_DIRECTORY) && !(search->fno.fattrib & AM_DIR)) { continue; }
		if (!(search->attributes & ATTR_DIRECTORY) &&  (search->fno.fattrib & AM_DIR)) { continue; }

		ret = 0;
		break;
	}
    //f_closedir(&search->dir);
	return ret;
}
#endif


// Returns the error code for the last file function. See FSIO.h FSerror defintion for full details.
/*
int FSerror(void)
{
    return ???;
}
*/

// Create a master boot record, with the specified first sector of the partion (non-zero), and number of sectors.
int FSCreateMBR(unsigned long firstSector, unsigned long numSectors)
{
#ifdef _MULTI_PARTITION
    //f_fdisk(0, sizes, work);
    return 0;
#else
    // FatFs cannot separtely create an MBR from formatting, must just format instead (f_mkfs)
    return 0;
#endif
}


#ifdef ALLOW_GET_DISK_PROPERTIES

// Retrieve properties about the disk.
/*
void FSGetDiskProperties(FS_DISK_PROPERTIES *properties)
{
    ???
}
*/

#endif


// ---- The following are implementations of extension functions found in FsUtils.c ----

// Write a (sector-aligned) sector of data to the file (512 bytes)
BOOL FSfwriteSector(const void *ptr, FSFILE *stream, BOOL ecc)
{
    return FSfwriteMultipleSectors(ptr, stream, 1, ecc);
}

BOOL FSfwriteMultipleSectors(const void *ptr, FSFILE *stream, int count, BOOL ecc)
{
    FRESULT res;
    unsigned int length = count * MEDIA_SECTOR_SIZE;
    unsigned int result = 0;
    
    #ifdef FS_WRITE_SECTOR_ECC
        #warning "ECC state passing through FatFs to the FS_WRITE_SECTOR_ECC not yet implemented."
    #endif
    
#ifdef FS_SECTOR_FLUSH
    #warning "FS_SECTOR_FLUSH not yet honoured".
    /*
		unsigned short flushEveryNClusters = FS_SECTOR_FLUSH * ((unsigned int)(65536UL / MEDIA_SECTOR_SIZE) / stream->dsk->SecPerClus);		// Flush every FS_SECTOR_FLUSH * 64 kB, (128 sectors per 64kB)
		if (flushEveryNClusters != 0)
		{
			unsigned long cluster = (stream->seek + dsk->sectorSize) / MEDIA_SECTOR_SIZE / stream->dsk->SecPerClus;
			flush = (char)(((cluster % flushEveryNClusters) == 0) ? (char)1 : (char)0);
		}
        if (flush)
        {
            FSfflush(stream);
        }
    */
#endif

    res = f_write((FIL *)stream, ptr, length, &result);
    return (res == FR_OK && result == length);
}


// Flush writes to a file
int FSfflush(FSFILE *fo)
{
    f_sync((FIL *)fo);
#ifdef FS_FLUSH
    FS_FLUSH();
#endif
	return 0;
}

// Free space on the drive
unsigned long FSDiskFree(void)
{
	FATFS *fs = &(fatfsio.fatfs);
    unsigned long free;
	int res;

    // Get the number of free clusters
    res = f_getfree(NULL, &free, &fs);
    if (res != FR_OK) return 0;

    // Total sectors = (fs->n_fatent - 2) * fs->csize;
    
    // Get the number of free sectors
    free *= fs->csize;

    // Return the number of free bytes
    return free * MEDIA_SECTOR_SIZE;
}

// TRUE if disk is full
unsigned char FSDiskFull(void)
{
    return (FSDiskFree() == 0);
}

// Read the number of sectors per cluster on the disk
unsigned char FSDiskSectorsPerCluster(void)
{
    return fatfsio.fatfs.csize;
}

// The sector location of the first file cluster
unsigned long FSDiskDataSector(void)
{
    return fatfsio.fatfs.database;
}

// Follow the cluster chain for a file, calling a user callback function
/*
char FSFollowClusterChain(const char *filename, void *reference, void (*callback)(void *, unsigned short, unsigned long))
{
    ???
}
*/

// Read a line from a file
char *FSfgets(char *str, int num, FSFILE *stream)
{
#if (_USE_STRFUNC >= 1)
    return f_gets(str, num, (FIL *)stream);
#else
	char *p;
	unsigned int n;

	if (num <= 0) { return NULL; }
	if (f_eof(stream)) { str[0] = '\0'; return NULL; }

	for (p = str; !f_eof(stream); p++, num--)
	{
		// Painfully read one byte at a time
		f_read(stream, p, 1, &n);

		// If no more buffer space, or end of file or CR or LF...
		if (num <= 1 || n == 0 || *p == '\r' || *p == '\n')
		{
			// NULL-terminate and exit
			*p = '\0';
			break;
		}
	}
	return str;
#endif
}

// Read/write a character
int FSfgetc(FSFILE *fp)
{
    FRESULT res;
    char c = 0;
    unsigned int result = 0;
    res = f_read((FIL *)fp, &c, 1, &result);
    if (res == FR_OK && result == 1)
    {
        return (int)c;
    }
    return -1;
}

int FSfputc(int character, FSFILE *fp)
{
#if (_USE_STRFUNC >= 1)
    return f_putc(character, (FIL *)fp);
#else
	char c = (char)character;
	unsigned int bw = 0;
	return (f_write((FIL*)fp, &c, 1, &bw) == FR_OK && bw == 1) ? 1 : 0;
#endif
}

// Get/put word/dword
void FSfputshort(short v, FSFILE *fp) { FSfputc((char)((v >> 0) & 0xff), fp); FSfputc((char)((v >> 8) & 0xff), fp); }
void FSfputlong(long v, FSFILE *fp) { FSfputc((char)((v >> 0) & 0xff), fp); FSfputc((char)((v >> 8) & 0xff), fp); FSfputc((char)((v >> 16) & 0xff), fp); FSfputc((char)((v >> 24) & 0xff), fp); }
short FSfgetshort(FSFILE *fp) { unsigned short v = 0; v |= (((unsigned short)FSfgetc(fp)) << 0); v |= (((unsigned short)FSfgetc(fp)) << 8); return (short)v; }
long FSfgetlong(FSFILE *fp) { unsigned long v = 0; v |= (((unsigned long)FSfgetc(fp)) << 0); v |= (((unsigned long)FSfgetc(fp)) << 8); v |= (((unsigned long)FSfgetc(fp)) << 16); v |= (((unsigned long)FSfgetc(fp)) << 24); return (long)v; }

// Creates a volume label (can only be used once after formatting, no existing volume entry removed)
char FSfsetvolume(const char *fileName)
{
    FRESULT res;
#if !(_USE_LABEL > 0)
//#error "_USE_LABEL not defined."
	res = FR_INT_ERR;
#else
    res = f_setlabel(fileName);
#endif
    return (res == FR_OK);
}

