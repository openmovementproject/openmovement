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

// IMPORTANT: These are not yet all implemented in FatFsIo.c
#ifndef FATFSIO_H
#define FATFSIO_H


// FatFs
#include "FatFs/ff.h"
struct FIL;


// User's local configuration
#include "FSconfig.h"



#if 1   //------------------------------------------------------

// TODO: Remove this cheat
#include "MDD File System/FSDefs.h"

#else

// Error codes
#define CE_GOOD                    0
#define CE_ERASE_FAIL              1
#define CE_NOT_PRESENT             2
#define CE_NOT_FORMATTED           3
#define CE_BAD_PARTITION           4
#define CE_UNSUPPORTED_FS          5
#define CE_INIT_ERROR              6
#define CE_NOT_INIT                7
#define CE_BAD_SECTOR_READ         8
#define CE_WRITE_ERROR             9
#define CE_INVALID_CLUSTER         10
#define CE_FILE_NOT_FOUND          11
#define CE_DIR_NOT_FOUND           12
#define CE_BAD_FILE                13
#define CE_DONE                    14
#define CE_COULD_NOT_GET_CLUSTER   15
#define CE_FILENAME_2_LONG         16
#define CE_FILENAME_EXISTS         17
#define CE_INVALID_FILENAME        18
#define CE_DELETE_DIR              19
#define CE_DIR_FULL                20
#define CE_DISK_FULL               21
#define CE_DIR_NOT_EMPTY           22
#define CE_NONSUPPORTED_SIZE       23
#define CE_WRITE_PROTECTED         24
#define CE_FILENOTOPENED           25
#define CE_SEEK_ERROR              26
#define CE_BADCACHEREAD            27
#define CE_CARDFAT32               28
#define CE_READONLY                29
#define CE_WRITEONLY               30
#define CE_INVALID_ARGUMENT        31
#define CE_TOO_MANY_FILES_OPEN     32
#define CE_UNSUPPORTED_SECTOR_SIZE 33

// File attributes
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME    0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20
#define ATTR_MASK      0x3f // For search function

// End-of-file
#ifndef EOF
    #define EOF (-1)
#endif

// KL: Used by some of microchips libraries
#define FILE_NAME_SIZE_8P3           11
#define FILE_NAME_SIZE               FILE_NAME_SIZE_8P3

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


#endif   //------------------------------------------------------



// File handle
#define FSFILE FIL

// Seek macros
#ifndef SEEK_SET
    #define SEEK_SET 0
#endif
#ifndef SEEK_CUR
    #define SEEK_CUR 1
#endif
#ifndef SEEK_END
    #define SEEK_END 2
#endif

// FSfopen macros
#define FS_APPEND     "a"   // Append to existing
#define FS_WRITE      "w"   // Create new or replace existing
#define FS_READ       "r"   // Read from existing
#define FS_APPENDPLUS "a+"  // Append to existing, can also read
#define FS_WRITEPLUS  "w+"  // Create new or replace existing, can also read
#define FS_READPLUS   "r+"  // Read from existing, can also write


// Search records
typedef struct
{
    char *filename;
    unsigned char attributes;
    unsigned long filesize;
    unsigned long timestamp;
    // Additional tracking for private use
    FRESULT res;
    FILINFO fno;
    DIR dir;
#if _USE_LFN
    char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
#endif
	char path[64];
	char filespec[32];
} SearchRec;



// Initialize and mounts the disk. Returns TRUE if successful.
int FSInit(void);

// Open a file and, if successful, returns a pointer to a newly allocated FSFILE object.
// The file is created if we are writing (if it already exists, it is removed first).
// The file pointer is set to the end of the file in append mode.
FSFILE *FSfopen(const char *filename, const char *mode);

// Close the file and release the allocated FSFILE object.
// Returns 0 if successful, -1 otherwise.
int FSfclose(FSFILE *file);

// Sets the file position to the start of the file.
void FSrewind(FSFILE *file);

// Read from the file
size_t FSfread(void *buffer, size_t size, size_t count, FSFILE *file);

// Seek within the file. 
int FSfseek(FSFILE *file, long offset, int whence);

// Gets the current location in a file.
long FSftell(FSFILE *file);

// Checks whether the current file position is at the end.
int FSfeof(FSFILE *file);


#ifdef ALLOW_FORMATS

// Format a drive. 
// This will use the FORMAT_SECTORS_PER_CLUSTER macro.
// Returns 0 if successful, -1 otherwise.
int FSformat(char wipe, long int serialNumber, char *volumeId);

#endif


#ifdef ALLOW_WRITES

// Set the file attributes. Returns 0 if successful, -1 otherwise.
int FSattrib(FSFILE *file, unsigned char attributes);

// Rename a file. Returns 0 if successful, -1 otherwise.
int FSrename(const char *filename, FSFILE *file);

// Delete a file. Returns 0 if successful, -1 otherwise.
int FSremove(const char *filename);

// Write data to a file
size_t FSfwrite(const void *buffer, size_t size, size_t count, FSFILE *file);

#endif



#ifdef ALLOW_DIRS

// Change the current working directory. Returns 0 if successful, -1 otherwise.
int FSchdir(char *path);

// Gets the current working directory. Returns 0 if successful, -1 otherwise.
char *FSgetcwd(char *path, int maxPath);

#ifdef ALLOW_WRITES

// Create a directory. Returns 0 if successful, -1 otherwise.
int FSmkdir(char *path);

// Remove a directory. Returns 0 if successful, -1 otherwise.
int FSrmdir(char * path, unsigned char rmsubdirs);

#endif

#endif


#ifdef USERDEFINEDCLOCK
// Manually set the clock variables
int SetClockVars(unsigned int year, unsigned char month, unsigned char day, unsigned char hour, unsigned char minute, unsigned char second);
#endif


#ifdef ALLOW_FILESEARCH
// Perform initial search for a file. Returns 0 if a file was found, or -1 if no matching files were found.
int FindFirst(const char *filename, unsigned int attributes, SearchRec *search);

// Continue search after FindFirst. Returns 0 if another file was found, or -1 if no further matching files were found.
int FindNext(SearchRec *search); 
#endif


#ifdef ALLOW_FSFPRINTF
// Formatted printing to a file.
//int FSfprintf(FSFILE *file, const char *format, ...);
extern int f_printf(FIL *file, const char *format, ...);
#define FSfprintf(file, format, ...) f_printf((FIL *)(file), (format), __VA_ARGS__)
#endif


// Returns the error code for the last file function. See FSIO.h FSerror defintion for full details.
int FSerror(void);

// Create a master boot record, with the specified first sector of the partion (non-zero), and number of sectors.
int FSCreateMBR(unsigned long firstSector, unsigned long numSectors);


#ifdef ALLOW_GET_DISK_PROPERTIES

// FS_DISK_ERRORS
#define FS_GET_PROPERTIES_NO_ERRORS        0
#define FS_GET_PROPERTIES_DISK_NOT_MOUNTED 1
#define FS_GET_PROPERTIES_CLUSTER_FAILURE  2
#define FS_GET_PROPERTIES_STILL_WORKING    0xff

// Disk format
#define FAT12 1
#define FAT16 2
#define FAT32 3

typedef struct
{
    char new_request;
    int properties_status;  // FS_DISK_ERRORS
    struct
    {
        unsigned char disk_format;
        unsigned short sector_size;
        unsigned char sectors_per_cluster;
        unsigned long total_clusters;
        unsigned long free_clusters;
    } results;
} FS_DISK_PROPERTIES;

// Retrieve properties about the disk.
void FSGetDiskProperties(FS_DISK_PROPERTIES *properties);

#endif


// ---- The following are implementations of extension functions found in FsUtils.c ----

// Write a (sector-aligned) sector of data to the file (512 bytes)
BOOL FSfwriteSector(const void *ptr, FSFILE *stream, BOOL ecc);
BOOL FSfwriteMultipleSectors(const void *ptr, FSFILE *stream, int count, BOOL ecc);

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




