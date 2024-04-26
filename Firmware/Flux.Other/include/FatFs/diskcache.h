/*	Karim Ladha, 23-09-2015
	-------------	Disk cache driver for embedded systems	-----------------------
	Simple sector-wise disk caching functionality to allow asynchronous disk access
	Implements a sequential pre-emptive read ahead strategy to offer read speed up
	Disk writes use a simple queue system to gain faster writes with lower latency
	Allows asynchronous disk access from an application to operate in an interrupt
	Disk access occurs in a task function typically called from the main scope
	Links to physical media through diskio.c/h api the same as the FatFs library 
	Offers compile time and real time re-configuration and dynamic memory usage 
	Initialisation with start-up(). To re-initialise, call shut-down() then start-up()
	-------------------------------------------------------------------------------
	
	User definitions and behaviour or configuration options to place in "config.h"

	Error handling for disk io errors:
	If the app can handle disk errors, it must check these flags in returned sectors
	For write errors, optionally specify the number of write tries with DISK_CACHE_WRITE_TRIES
		- default is one try only i.e. the diskio driver must perform best effort
		- To detect write completion, the data valid flag is set on write success 
		- If the app detects the the write done AND valid flags both clear, the write failed

	To limit disk access errors define DISK_CACHE_IO_ERROR_LIMIT
	Once error count reaches the limit, disk access stops pending re-initialisation
		- The default is off, unlimited
		- After the limit, the fault call back is called if set

	To detect lock-up errors due to incorrect driver usage, set the DISK_CACHE_BLOCKING_LIMIT
	The count increases each time the tasks function fails to stop the driver blocking
		- The default is off, unlimited
		- After the limit, the fault call back is called if set
	
	The maximum number of sectors to use: 
		- Minimum is one for read OR write. Default is 4 if not set
		- Setting values more than one enables read ahead performance boost
		- The start up function allocation setting is limited to this definition
		- The default number is also the maximum, define DISK_CACHE_MAX_SECTOR_COUNT
		- Defining DISK_CACHE_NO_MALLOC will make the allocation static (requires sector size definition)
		
	The sector size, default is 512 bytes if driver query fails:
		- Optionally define DISK_CACHE_SECTOR_SIZE to fix the size
	
	The limit for the number of disk sector accesses per tasks call:
		- Optionally define DISK_CACHE_MAX_DISK_TASKS. Default is 1.

	Required definitions for separate app and tasks scope:
	The internal thread flags are typically only suitable for multi-thread usage
	ISR blocking is needed to allow app to run in an interrupt (i.e. msd app)
	The user must provide these macros to block app calls:
		DISK_CACHE_BLOCK_APP()
		DISK_CACHE_UNBLOCK_APP()
	
	Example minimalist config.h setup, PIC24:
	
	#define DISK_CACHE_MAX_SECTOR_COUNT	6
	#define DISK_CACHE_MAX_DISK_TASKS	2
	#define DISK_CACHE_WRITE_TRIES		2
	#define DISK_CACHE_BLOCK_APP()		{IEC0bits.IC2IE = 0;Nop();Nop();Nop();}
	#define DISK_CACHE_UNBLOCK_APP()	{IEC0bits.IC2IE = 1;}
	
	History:
	Based on the AsynchronousDisk application written in 2014
	Links to an adapted version of the Microchip USB MSD multi-sector device driver
	
	Revisions:
	23-09-2015:	First prototype written	
	12-10-2015:	First release tested and working.

	Results of revisions:
	12-10-2015: Results for USB MSD application in size optimised debug build and ECC checking off	
					- Cache *miss* on reading 100MB file was 1 in 1000 
					- Read performance was ~750 kB/s 
					- Write performance was ~450 kB/s
*/ 

#ifndef _DISK_CACHE_H_
#define _DISK_CACHE_H_

// Includes
#include <stdint.h>

// Definitions
// Argument for start up and shut down for typical usage/behaviour
#define DISK_CACHE_DEFAULT		0	

// Types
typedef enum { // Sector state enumerated type with flags
	S_IDLE 			= 0x00,	// Sector unused 
	S_READ_REQUEST	= 0x01,	// Application requested read 
	S_WRITE_OPEN	= 0x02, // Application currently writing	
	S_WRITE_DONE	= 0x04,	// Application has finished writing
	S_CACHE_FRONT	= 0x08,	// Driver flagged for pre-emptive read
	S_DATA_VALID 	= 0x10,	// Driver flagged sector data valid 
	S_TRANSFERRING	= 0x20, // Driver flagged reading or writing
	S_ACCESS_ERROR	= 0x40,	// Driver flagged transfer errors
	S_WRITE_COLLIDE	= 0x80,	// Driver flagged transfer collision 	
	// Masks
	S_IS_BUSY_MASK	= 0xE7,	// Mask to check if currently busy
	// Debugging help (common named states)
	S_NEW_READ_REQ	= 0x09,	// App has requested new read position
	S_READ_OPEN		= 0x11,	// App is reading data from sector
	S_CACHE_END		= 0x18,	// End of cache queue, no more free sectors
	S_BUSY_READING	= 0x21,	// Transferring requested data from disk
	S_BUSY_PRE_READ	= 0x28,	// Pre-emptively reading data from disk
	S_BUSY_WRITING	= 0x24,	// Writing finished sector to disk
	S_WR_B4_READ_Q	= 0x2D,	// Writing sector to disk, app requesting read, queue commencing
	S_WR_B4_READ	= 0x25	// Writing sector to disk, app requesting read	
} SectorState_t;

typedef struct { // The sector structure 
	uint32_t lba;			// Sector address (align to dword)
	SectorState_t state;	// Sector state enumerated type
	uint8_t* buffer;		// Sector data buffer
} DiskCacheSector_t;		// Sector structure type

typedef enum { // Disk state enumerated type with flags
	DC_STOPPED		= 0x00, // Start up state	
	// Cache driver general status flags
	DC_OPERATIONAL	= 0x01, // Driver initialised, memory allocated, working		
	DC_ALLOCATED	= 0x02, // Memory is assigned to one or more of the sectors		
	DC_DISK_FAULT	= 0x04, // Disk read-write access is not functioning			
	// Operating flags
	DC_CACHE_FULL	= 0x08, // All the sectors are assigned or busy
	DC_TASKS_BLOCK 	= 0x10, // Not implemented yet (for future OS threading support)
	DC_APP_ENABLE	= 0x20, // Application functions are available	
	// Application flags
	DC_READING 		= 0x40,	// An application read operation is active
	DC_WRITING 		= 0x80, // An application write operation is active
	// Debugging help (common named states)
	DC_READY_IDLE	= 0x23,	// The normal idle state
	DC_READY_READ	= 0x63,	// The normal reading state
	DC_READY_WRITE	= 0xA3,	// The normal writing state	
	DC_READY_RDnWR	= 0xE3	// The normal read+writing state	
} DiskState_t; 

typedef struct { // The disk cache state struct
	DiskState_t state;		// Disk cache status enumerated type with flags
	uint8_t numSectors;		// Number of sectors used for disk cache driver
	uint8_t diskErrors; 	// Disk failed access error counter, from disk driver
	uint8_t driverErrors;	// Driver errors from unexpected states, should be none
	uint8_t taskEvents; 	// Used by application to detect if tasks has been called 
	uint8_t appEvents; 		// Used to detect any application activity during tasks
	uint8_t	appBlocked;		// Counter of sequential application blocks
	uint8_t appWarnings;	// Warning of driver misuse risking undefined behaviour
	uint8_t appErrors;		// Driver misuse errors that risked disk corruption
#if defined(__DEBUG) && (DEBUG_FILESYS >= 3)
	uint32_t cacheHitCount;	// Debug statistics of driver efficiency
	uint32_t cacheMissCount;// Used to gauge the usefulness of the cache
#endif
	DiskCacheSector_t * sector;	// The cached sector array
} DiskCacheState_t;

// Globals
extern volatile DiskCacheState_t gDiskCache;
extern volatile DiskCacheSector_t gCacheSectors[];

// Prototypes
// Tasks API, performs disk access asynchronously to application
// Initialise structures and states, allocate up to sectorCount (zero sets max sector count setting)
uint8_t DiskCacheStartup(uint8_t sectorCount);
// Tasks function, called in other scope, handles disk i/o
void DiskCacheTasks(void);
// Fault callback installer, called if lock up detected
void DiskCacheSetFaultCallBack(void(*CallBack)(void));
// Clear states, free memory and stops disk access (to reset driver, de-allocation may not be desired)
uint8_t DiskCacheShutdown(uint8_t keepAllocation);

// Application sector API
// Check if the read function will return immediately
uint8_t DiskCacheReadReady(uint32_t lba);
// Read sector, returns NULL or ptr to start of sector, sets ownership
uint8_t* DiskCacheReadOpen(uint32_t lba, DiskCacheSector_t** sector);
// Close sector, frees read sector
void DiskCacheReadClose(DiskCacheSector_t* sector);

// Check if the write function will return immediately
uint8_t DiskCacheWriteReady(uint32_t lba);
// Start writing sector, returns pointer to start of a sector or NULL
uint8_t* DiskCacheWriteOpen(uint32_t lba, DiskCacheSector_t** sector);
// Finish sector, commits written sector to disk
void DiskCacheWriteClose(DiskCacheSector_t* sector);
// Close sector, frees reads and aborts writes
void DiskCacheWriteAbort(DiskCacheSector_t* sector);

#endif

