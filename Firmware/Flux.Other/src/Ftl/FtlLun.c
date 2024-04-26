// Karim Ladha 03-07-2015
// Uses logical disk from diskio.c/h to make generic LUN interface
// LUN interface is used by the USB mass storage drivers
// Allows FatFs and USB stack to have a common interface other than FTL


// Headers
#include "FatFs/diskio.h"			/* Attaches to FTL just like FatFs  */
#include "config.h"					/* Configuration and definitions	*/

// Debug setting
#define DEBUG_LEVEL_LOCAL	DEBUG_FILESYS
#define DBG_FILE			"ftllun.c"
#include "utils/debug.h"

// Types
// Compatible with MEDIA_INFORMATION type in legacy "FSDefs.h"
typedef struct {
	uint8_t errorCode;
    union {
		uint8_t value;
		struct {
	    	uint8_t sectorSize  : 1;
			uint8_t maxLUN      : 1;
			uint8_t sectors		: 1;
		}bits;
    } validityFlags;
    uint16_t sectorSize;
    uint8_t maxLUN;
	// Extended	
	uint32_t logicalSectors;
} MediaInfo_t;

// Globals
MediaInfo_t asyncLunInfo = {0};

// The LUN driver source

// Function pointer to the MediaInitialize() function of the physical media being used. 
MediaInfo_t* LUNMediaInitialize(void)
{
	// Set default values
	asyncLunInfo.validityFlags.value = 0;
	asyncLunInfo.logicalSectors = 0;
	asyncLunInfo.sectorSize = 0;

	// Try to initialise the drive if needed
	if(disk_status(0) & STA_NOINIT) /* Disk 0 default */
		disk_initialize(0); 

	// If no disk, return early
	if((disk_status(0) & STA_NODISK) != 0)
		return NULL;

	// Read drive parameters into local structure
	if(disk_ioctl(0, GET_SECTOR_COUNT, &asyncLunInfo.logicalSectors) == RES_OK)	
		asyncLunInfo.validityFlags.bits.sectors = 1;
	if(disk_ioctl(0, GET_SECTOR_SIZE, &asyncLunInfo.sectorSize) == RES_OK)	
		asyncLunInfo.validityFlags.bits.sectorSize = 1;

	// Set max LUN variable
	asyncLunInfo.validityFlags.bits.maxLUN = 1;
	asyncLunInfo.maxLUN = 0; /* Max drive index */

	// Return drive information
	return &asyncLunInfo;
}

// Function pointer to the ReadCapacity() function of the physical media being used.
DWORD LUNReadCapacity(void)
{
	if(!asyncLunInfo.validityFlags.bits.sectors)
		LUNMediaInitialize();
	/* Its actually asking for the last LBA on disk */
//TODO: Find out why we need to reduce the disk capacity
	return asyncLunInfo.logicalSectors - 256;	
}

// Function pointer to the ReadSectorSize() function of the physical media being used.
WORD LUNReadSectorSize(void)
{
	if(!asyncLunInfo.validityFlags.bits.sectorSize)
		LUNMediaInitialize();
	return asyncLunInfo.sectorSize;
}

// Function pointer to the MediaDetect() function of the physical media being used.
BYTE LUNMediaDetect(void)
{
	if(disk_status(0) & STA_NOINIT) /* Disk 0 default */
		disk_initialize(0); 
	return (disk_status(0) & STA_MSD_NO_DISK) ? 0 : 1;
}

// Function pointer to the WriteProtectState() function of the physical media being used.
BYTE LUNWriteProtectState(void)
{
	if(disk_status(0) & STA_NOINIT) /* Disk 0 default */
		disk_initialize(0); 
	return (disk_status(0) & STA_MSD_PROTECT) ? 1 : 0; /* Disk 0 default */
}

// Function pointer to the SectorRead() function of the physical media being used.
BYTE LUNSectorRead(DWORD sector_addr, BYTE* buffer)
{
	if(RES_OK == disk_read(0, buffer, sector_addr, 1))
		return 1; 
	return 0;	
}

// Function pointer to the SectorWrite() function of the physical media being used.
BYTE LUNSectorWrite(DWORD sector_addr, BYTE* buffer, BYTE allowWriteToZero)
{
	if(RES_OK == disk_write(0, buffer, sector_addr, 1))
		return 1; 
	return 0;
}

// Normally handled externally if file system is used
BYTE LUNInitIO(void)
{
	// If the LUN is uninitialised
	if(!asyncLunInfo.validityFlags.value)
		LUNMediaInitialize();
	return 1;
}

// The disk can't be shut down in case used for file system
BYTE LUNShutdownMedia(void)
{
	// Flush pending writes if initialised
	if(!(disk_status(0) & STA_NOINIT))
		if(disk_ioctl(0, CTRL_SYNC, NULL) != RES_OK)
			return 0;
	// Flushed ok, volatile states saved			
	return 1;
}



#ifdef FTLLUN_TABLE

#include "MDD File System/FSDefs.h" /* For MEDIA_INFORMATION structure */
#include "USB/USB.h"
#include "usb_config.h"
#include "USB/usb_function_msd.h"

typedef MEDIA_INFORMATION*(*MediaInitialize_t)();

// The full LUN function table implementation:
// The LUN variable definition is in usb_function_msd.h
#define MAX_LUN 0 // Zero indexed, 0 -> One logical disk
LUN_FUNCTIONS LUN[MAX_LUN + 1] = {
{
	(MediaInitialize_t)&LUNMediaInitialize,
	&LUNReadCapacity,
	&LUNReadSectorSize,
	&LUNMediaDetect,
	&LUNSectorRead,
	&LUNWriteProtectState,
	&LUNSectorWrite
}};

#endif


//EOF
