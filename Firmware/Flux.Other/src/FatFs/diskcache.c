/*	Karim Ladha, 23-09-2015
	-------------	Disk cache driver for embedded systems	-----------------------
	Simple sector-wise disk caching functionality to allow asynchronous disk access
	Implements a sequential pre-emptive read ahead strategy to offer read speed up
	Disk writes use a simple queue system to gain faster writes with lower latency
	Allows asynchronous disk access from an application to operate in an interrupt
	Disk access occurs in a task function typically called from the main scope
	Links to physical media through diskio.c/h api the same as the FatFs library 
	Offer good compile time and real time reconfigurability and dynamic memory usage 
	------------------------------------------------------------------------------- */
#include <string.h>
#include <stdint.h>
#include "FatFs/diskio.h"
#include "FatFs/DiskCache.h"
#include "config.h"

// Debug setting
// Will use global setting if set. 
#define DEBUG_LEVEL_LOCAL	DEBUG_FILESYS
#define DBG_FILE			"cache.c"
#include "utils/debug.h"

// Debug error catcher
#if(DEBUG_FILESYS > 0)
inline void DiskCacheErrorTrap(void)
{
	Nop(); Nop(); // Breakpoint here
	Nop(); Nop();
}
#else
	#define DiskCacheErrorTrap(_x)	{;}
#endif

// Definitions
// Check user has provided a mutex function
#ifndef DISK_CACHE_ISR_OFF
	#error "Mutex method required"
#endif
// Set total sector count
#ifndef DISK_CACHE_MAX_SECTOR_COUNT
	#warning "Default sector count used."
	#define DISK_CACHE_MAX_SECTOR_COUNT 	4	
#endif
// Set write error retry limit
#ifndef DISK_CACHE_WRITE_TRIES
	#define DISK_CACHE_WRITE_TRIES	1
#endif
#if (DISK_CACHE_WRITE_TRIES == 0)
	#error "No trys means read only, invalid selection"
#endif
// Set disk access limit per task call to reduce blocking (max number of sector read/write per task call)
#ifndef DISK_CACHE_MAX_DISK_TASKS
	#define DISK_CACHE_MAX_DISK_TASKS 		1	
#endif
// Loops of the task function before giving up on flush 
#ifndef DISK_CACHE_MAX_FLUSH_TASK_LOOPS
	#define DISK_CACHE_MAX_FLUSH_TASK_LOOPS  (10 + (DISK_CACHE_MAX_SECTOR_COUNT / DISK_CACHE_MAX_DISK_TASKS))
#endif
// Default disk access via diskio.c/h, same api as FatFs
#ifndef DISK_CACHE_SECTOR_READ
	#define DISK_CACHE_SECTOR_READ(_lba, _buff)	(disk_read(0, (BYTE*)_buff, (DWORD) _lba, 1) == RES_OK)	
#endif
#ifndef DISK_CACHE_SECTOR_WRITE
	#define DISK_CACHE_SECTOR_WRITE(_lba, _buff)	(disk_write(0, (BYTE*)_buff, (DWORD) _lba, 1) == RES_OK)	
#endif
// Disk access error limit before cache driver gives up
#ifndef DISK_CACHE_IO_ERROR_LIMIT
	// #define DISK_CACHE_IO_ERROR_LIMIT // No limit set by default
#endif
// Driver blocking limit before driver is considered locked up
#ifndef DISK_CACHE_BLOCKING_LIMIT
	// #define DISK_CACHE_BLOCKING_LIMIT // No limit set by default	
#endif

// General private definitions
#define INVALID_LBA					0xFFFFFFFFul
#define INVALID_SECTOR_INDEX 		(0xFF)
#define DISK_CACHE_TASK_LOOP_COUNT	gDiskCache.numSectors

// Globals
// Sector buffers if malloc is *not* used 
#ifdef DISK_CACHE_NO_MALLOC	
static uint8_t sectorBuffers[DISK_CACHE_MAX_SECTOR_COUNT][DISK_CACHE_SECTOR_SIZE];
#endif
// The cached sector structs
volatile DiskCacheSector_t gCacheSectors[DISK_CACHE_MAX_SECTOR_COUNT] = {{0}};
// The global cached disk state
volatile DiskCacheState_t gDiskCache = {
	.state			= DC_STOPPED,		
	.numSectors		= 0,
	.taskEvents		= 0,
	.appEvents		= 0,		
	.diskErrors		= 0,		
	.driverErrors	= 0,	
	.appWarnings	= 0,	
	.appErrors		= 0,
	.sector			= (DiskCacheSector_t*)gCacheSectors };
// Fault callback function pointer
void(*diskCacheFaultCB)(void) = NULL;

// Private prototypes - allocate and free disk sectors
static void DiskCacheIdleSector(DiskCacheSector_t* sector);
static void DiskCacheAllocate(void);
static void DiskCacheFree(void);
static void DiskCacheFlush(void);
static uint8_t DiskCacheGetSector(uint32_t lba, DiskCacheSector_t** matchedSector, DiskCacheSector_t** readySector);
static uint8_t DiskCacheQueueNext(uint8_t index);

// Source

// Initialise structures and states
uint8_t DiskCacheStartup(uint8_t sectorCount)
{
	// Early out on re-initialise
	if(gDiskCache.state & DC_OPERATIONAL)	
	{
		// Already initialised
		DiskCacheErrorTrap();
		DBG_INFO("tried re-init!");
		gDiskCache.appWarnings++;
		return 1;
	}
	// Reset disk fault state flag
	gDiskCache.state &= ~DC_DISK_FAULT;
	// Clear the driver state
	memset((void*)&gDiskCache, 0, sizeof(gDiskCache));
	memset((void*)gCacheSectors, 0, sizeof(gCacheSectors));
	gDiskCache.sector = (DiskCacheSector_t*)gCacheSectors;
	// Initialise the sector pool up to size specified
	gDiskCache.numSectors = DISK_CACHE_MAX_SECTOR_COUNT;
	if((sectorCount != 0) && (sectorCount <= DISK_CACHE_MAX_SECTOR_COUNT))
		gDiskCache.numSectors = sectorCount;
	// Try to assign buffers up to sector count
	DiskCacheAllocate();
	// Clear all counters
	gDiskCache.taskEvents = 0;
	gDiskCache.appEvents = 0;
	gDiskCache.diskErrors = 0;
	gDiskCache.driverErrors = 0;
	gDiskCache.appBlocked = 0;
	gDiskCache.appWarnings = 0;
	gDiskCache.appErrors = 0;	
	// Check and set state
	if(gDiskCache.numSectors == 0)	// No sector buffers... start up error
	{
		// Disk could not start (zero sectors), block application
		gDiskCache.state = DC_STOPPED;
		// Free any memory allocated (should be zero)
		DiskCacheFree();
		// Debug outputs
		DiskCacheErrorTrap();
		DBG_INFO("failed to init");
		gDiskCache.driverErrors++;
		return 0;
	}
	// Debug stats to zero
#if defined(__DEBUG) && (DEBUG_FILESYS >= 3)
	gDiskCache.cacheHitCount = 0;
	gDiskCache.cacheMissCount = 0;
#endif
	// Ready for access. Set flags to enable driver
	gDiskCache.state |= (DC_OPERATIONAL | DC_APP_ENABLE);
	DBG_INFO("inited with %u sectors", gDiskCache.numSectors);
	return 1;
}

// Turn off the disk with orderly shut down (called at task scope)
uint8_t DiskCacheShutdown(uint8_t keepAllocation)
{
	// Clear start up failed condition
	if(gDiskCache.state == DC_STOPPED)
	{
		// Improper driver usage 
		DBG_INFO("already off");	
		gDiskCache.appWarnings++;
		return 1;
	}	
	// Block application access
	gDiskCache.state &= ~DC_APP_ENABLE;
	// Flush if required
	if(gDiskCache.state & (DC_READING | DC_WRITING))	
	{
		// Flush transactions (reads and writes) - flags cleared
		DiskCacheFlush();
	}
	// Free memory if required
	if(!keepAllocation)
	{
		// Free memory allocation - flag cleared
		DiskCacheFree();
	}
	// Clear all but allocation flag
	gDiskCache.state &= (DC_ALLOCATED); 
	// Debug out
	DBG_INFO("disk shutdown");	
	// Disk is now off..
	return 1;
}

// Fault callback installer
void DiskCacheSetFaultCallBack(void(*CallBack)(void))
{
	if(CallBack != NULL)
		diskCacheFaultCB = CallBack;
	return;
}

// Private force sector to idle method
static void DiskCacheIdleSector(DiskCacheSector_t* sector)
{
	sector->lba = INVALID_LBA;
	sector->state = S_IDLE;
}

// Initialise the sector pool
static void DiskCacheAllocate(void)
{
	uint16_t index, sectorSize;

	// Get sector size from driver, if not fixed
	#if !defined(DISK_CACHE_SECTOR_SIZE)
	// Query disk for sector size
	if(disk_ioctl(0, GET_SECTOR_SIZE, &sectorSize) != RES_OK)
	{
		DiskCacheErrorTrap();
		DBG_INFO("diskio err");
		gDiskCache.diskErrors++;
		// Fix the sector size to default expected size
		sectorSize = 512ul;
	}
	#else
	// Use defined sector size
	sectorSize = DISK_CACHE_SECTOR_SIZE;
	#endif
	
	// Allocate the sector buffers up to the set value in the disk state
	for(index = 0; index < gDiskCache.numSectors; index++)
	{
		// Index is the count of allocated sectors
		DiskCacheSector_t* indexSector = &gDiskCache.sector[index];
		// Initialise disk sector state
		DiskCacheIdleSector(indexSector);
	#if !defined(DISK_CACHE_NO_MALLOC) 
		// Check is not already allocated, then allocate 
		if(indexSector->buffer == NULL)
			{indexSector->buffer = DBG_MALLOC(sectorSize);}
		// Check it worked and add to count
		if(indexSector->buffer != NULL)
		{
			// Set allocation flag 
			gDiskCache.state |= DC_ALLOCATED;
			continue;
		}
		// Failed to allocate a sector
		DiskCacheErrorTrap();
		gDiskCache.driverErrors++;
		DBG_INFO("malloc sector failed");
		break;
	#else
		// Using static buffers
		indexSector->buffer = &sectorBuffers[index][0];
	#endif
	} // For each sector
	
	// Set actual number allocated
	gDiskCache.numSectors = index;
	return;
}

// Free the sector pool
static void DiskCacheFree(void)
{
	uint16_t index;
	for(index = 0; index < DISK_CACHE_MAX_SECTOR_COUNT; index++)
	{
		DiskCacheSector_t* indexSector = &gDiskCache.sector[index];
	#if !defined(DISK_CACHE_NO_MALLOC) 
		// Check not null, then free
		if(indexSector->buffer != NULL)
			{DBG_FREE(indexSector->buffer);}
		indexSector->buffer = NULL;
	#endif
		DiskCacheIdleSector(indexSector);
	}
	// Clear allocation flag
	gDiskCache.state &= ~DC_ALLOCATED;
	// Clear sector count
	gDiskCache.numSectors = 0;
}

// Complete pending actions
static void DiskCacheFlush(void)
{
	int16_t index, taskLoops = DISK_CACHE_MAX_FLUSH_TASK_LOOPS;	
	// Early out if not initialised
	if(!(gDiskCache.state & DC_OPERATIONAL))
	{
		DiskCacheErrorTrap();
		DBG_INFO("invalid flush");	
		gDiskCache.appWarnings++;
		return;
	}	
	// Block application access
	gDiskCache.state &= ~DC_APP_ENABLE;
	// Loop while any sectors are active
	while(gDiskCache.state & (DC_READING | DC_WRITING))
	{
		// Run tasks function
		DiskCacheTasks();
		// Check for timeout
		if(!taskLoops--)
		{
			// Only indicate error if writes fail
			if(gDiskCache.state & DC_WRITING)
			{
				DiskCacheErrorTrap();
				DBG_INFO("flush write failed");
				gDiskCache.driverErrors++;
			}
			break;
		}
	}
	// Clear states of sectors
	for(index = 0; index < DISK_CACHE_MAX_SECTOR_COUNT; index++)
	{
		DiskCacheSector_t* indexSector = &gDiskCache.sector[index];
		DiskCacheIdleSector(indexSector);
	}	
	// Set application state to idle
	gDiskCache.state &= ~(DC_READING | DC_WRITING);
}

// Internal method to scan the available sector buffers for idle sectors and those matching an lba
// Sets the matched pointer to the last matched lba* and the ready sector is set to the last not busy sector
// If there are no matches, returns zero. If all sectors busy then readySector is null.
// *Will try to return non-transferring match if multiple found. 
static uint8_t DiskCacheGetSector(uint32_t lba, DiskCacheSector_t** matchedSector, DiskCacheSector_t** readySector)
{
	uint8_t index, matchCount, readyCount;
	DiskCacheSector_t *indexSector;
	
	// Initialise variables
	*matchedSector = NULL;
	*readySector = NULL;
	matchCount = 0;
	readyCount = 0;
	
	// Find idle sector buffer and count idle ones, check for lba match
	for(index = 0; index < gDiskCache.numSectors; index++)
	{
		indexSector = &gDiskCache.sector[index];

		// Check if sector is not busy and can accept read request
		if(!(indexSector->state & S_IS_BUSY_MASK))
		{
			readyCount++;
			// Idle sector, set return sector to first ready one 
			if(*readySector == NULL)
				*readySector = indexSector;
		}
		
		// Check for lba in sector list
		if(indexSector->lba == lba)
		{
			matchCount++;
			// Matched sector, use only if not transferring
			if(!(indexSector->state & S_TRANSFERRING))
				*matchedSector = indexSector;
		}
	}	
	
	// Check number of non-busy sectors
	if(readyCount == 0) 
	{
		#ifdef DISK_CACHE_BLOCKING_LIMIT
		static uint8_t lastBlocked;
		// If the driver tasks has been called since the last block occurred
		if(lastBlocked != gDiskCache.taskEvents)
		{
			// Log the task event of this block to detect next blocking event
			lastBlocked = gDiskCache.taskEvents;
			// Add to blocking counter. Blocking condition persisted despite tasks call.
			gDiskCache.appBlocked++;
		}
		#endif
		// The driver sectors are all busy
		gDiskCache.state |= DC_CACHE_FULL;		
	}
	else
	{
		// Clear the sectors all busy flag
		gDiskCache.state &= ~(DC_CACHE_FULL);
		#ifdef DISK_CACHE_BLOCKING_LIMIT
		// Clear the lock-up detect blocking counter
		gDiskCache.appBlocked = 0;
		#endif
	}
	
	// Return number of matched sectors
	return matchCount;
}

// Check if the read function will return immediately
uint8_t DiskCacheReadReady(uint32_t lba)
{
	// Scan sectors for lba, return true if cached ready
	DiskCacheSector_t *matchedSector, *readySector;
	if(!(gDiskCache.state & DC_APP_ENABLE))return 0;
	DiskCacheGetSector(lba, &matchedSector, &readySector);	
	if((matchedSector != NULL) && (matchedSector->state & S_DATA_VALID) && (!(matchedSector->state & S_ACCESS_ERROR))) 
		return 1;
	return 0;
}

// Read sector, returns NULL or ptr to start of sector, sets ownership
uint8_t* DiskCacheReadOpen(uint32_t lba, DiskCacheSector_t** sector)
{
	DiskCacheSector_t *matchedSector, *readySector;
	uint8_t matched;
	
	// Event indication counter
	gDiskCache.appEvents++;
	
	// Make sure the module is accessible
	if(!(gDiskCache.state & DC_APP_ENABLE))
	{
		// Early exit if blocked
		DiskCacheErrorTrap();
		DBG_INFO("not ready");
		gDiskCache.appWarnings++;
		*sector = NULL;
		return NULL;		
	}
	
	// Set reading flag
	gDiskCache.state |= DC_READING;

	// Search for sector lba 
	matched = DiskCacheGetSector(lba, &matchedSector, &readySector);

	// Check for lba in sector list
	if(matched > 0)
	{
		// Check that only one matched
		if(matched > 1)
		{
			// This indicates a driver usage fault
			DiskCacheErrorTrap();
			DBG_INFO("collision read");
			gDiskCache.driverErrors++;		
		}

		// Check if sector has been read in to buffer		
		if(matchedSector->state & S_DATA_VALID)
		{
			// It has been read to buffer (or pre-emptively read)
			DBG_INFO("read done lba: %s",DBG_ultoa(lba)); 
			// Only set the requested flag, don't disrupt the working queue
			matchedSector->state |= S_READ_REQUEST;
			// Stats - cache hit
			#if defined(__DEBUG) && (DEBUG_FILESYS >= 3)
				gDiskCache.cacheHitCount++;
			#endif
			// Set sector pointer and return data
			*sector = matchedSector;
			return matchedSector->buffer;
		}
		
		// Not already read in, set flags for requested and start of queue
		matchedSector->state |= S_READ_REQUEST | S_CACHE_FRONT;
		
		// Setup pending read request
		DBG_INFO("app read req: %s",DBG_ultoa(lba));	
		// Block read until safely written to disk
		*sector = NULL;
		return NULL;
	}
	
	// No address match, check if ready sector returned
	if (readySector != NULL)
	{
		// Setup read request
		DBG_INFO("app new read: %s",DBG_ultoa(lba));
		// Set requested bit to own sector
		// Set queue flag to re-evaluate pre-emptive queue
		readySector->state = S_READ_REQUEST | S_CACHE_FRONT;
		// Set sector address
		readySector->lba = lba;
		// Stats - miss
		#if defined(__DEBUG) && (DEBUG_FILESYS >= 3)
			gDiskCache.cacheMissCount++;
		#endif
		// Return blocking, waiting for read
		*sector = NULL;
		return NULL;			
	}
	
	// Otherwise the disk is busy....(limited resources, not error)
	DBG_INFO("read blocked");
	*sector = NULL;
	return NULL;	
}

// Close currently open sector, clears ownership
void DiskCacheReadClose(DiskCacheSector_t* sector)
{
	// Event indication counter
	gDiskCache.appEvents++;
	
	// Make sure the disk is ready for access
	if(!(gDiskCache.state & DC_APP_ENABLE))
	{
		// Early exit if not initialised 
		DiskCacheErrorTrap();
		DBG_INFO("not ready");
		gDiskCache.appWarnings++;
		return;		
	}	
	// Reading sector - close
	if(sector->state & S_READ_REQUEST)
	{
		// Debug
		DBG_INFO("read close: %s",DBG_ultoa(sector->lba));
	}
	else
	{
		// Not in reading state
		DiskCacheErrorTrap();
		DBG_INFO("invalid read close: %s",DBG_ultoa(sector->lba));
		gDiskCache.appErrors++;	
		// This indicates incorrect driver usage, try to recover sector
		DiskCacheIdleSector(sector);			
	}
	// Clear ownership and error flags, associated data left ready in buffer
	sector->state &= ~(S_READ_REQUEST | S_CACHE_FRONT | S_ACCESS_ERROR);
	return;
}

// Check if the write function will return a sector immediately
uint8_t DiskCacheWriteReady(uint32_t lba)
{
	// Scan sectors for lba
	DiskCacheSector_t *matchedSector, *readySector;
	if(!(gDiskCache.state & DC_APP_ENABLE))return 0;	
	DiskCacheGetSector(lba, &matchedSector, &readySector);	
	if(readySector != NULL) 
		return 1;
	if((matchedSector != NULL) && (!(matchedSector->state & S_TRANSFERRING)))
		return 1;
	return 0;
}

// Start writing sector, returns pointer to start of a sector or NULL
uint8_t* DiskCacheWriteOpen(uint32_t lba, DiskCacheSector_t** sector)
{
	DiskCacheSector_t *matchedSector, *readySector;
	uint8_t matched;
	
	// Event indication counter
	gDiskCache.appEvents++;
	
	// Make sure the module is accessible
	if(!(gDiskCache.state & DC_APP_ENABLE))
	{
		// Early exit if blocked
		DiskCacheErrorTrap();
		DBG_INFO("not ready");
		gDiskCache.appWarnings++;
		*sector = NULL;
		return NULL;		
	}
	
	// Set writing flag
	gDiskCache.state |= DC_WRITING;

	// Search for sector lba 
	matched = DiskCacheGetSector(lba, &matchedSector, &readySector);

	// Check for lba in sector list
	if(matched > 0)
	{
		// Check less than 2 matched (one may be transferring)
		if(matched > 2)
		{
			// This indicates a driver usage fault
			DiskCacheErrorTrap();
			DBG_INFO("collision write");
			gDiskCache.driverErrors++;		
		}

		// Check if matched sector is actively reading or writing to disk
		if(matchedSector->state & S_TRANSFERRING)
		{
			// If we have another sector to return instead
			if(readySector != NULL)
			{
				// Transfer read request flags across
				if(matchedSector->state & S_READ_REQUEST)
					readySector->state |= S_READ_REQUEST;
				
				// If actively writing this lba 
				if(matchedSector->state & S_WRITE_DONE)
				{
					// Leave this one to finish writing to disk
					DBG_INFO("re-writing safely");
					// Clear reading, data-valid and queue flags
					matchedSector->state &= ~(S_READ_REQUEST | S_DATA_VALID | S_CACHE_FRONT);					
				}
				// If it is a read transfer, usage error
				else if(matchedSector->state & S_READ_REQUEST)
				{
					// This is a usage violation 
					DiskCacheErrorTrap();
					DBG_INFO("write to reading"); 
					gDiskCache.appWarnings++;	
					// Set collide flag, read will cancel
					matchedSector->state = S_WRITE_COLLIDE;
					// Move read request flag to new writing sector
					readySector->state |= S_READ_REQUEST;					
				}
				// If pre-reading, just cancel it
				else //if(matchedSector->state & S_CACHE_FRONT)
				{
					// Cancel pre-emptive read
					DBG_INFO("pre-read stopped");	
					// Set collide flag, queue cancelled
					matchedSector->state = S_WRITE_COLLIDE;					
				}
				// Setup ready sector for this write
				readySector->lba = lba;
				readySector->state = S_WRITE_OPEN;
				// Return ready sector for the write
				*sector = readySector;	
				return readySector->buffer;
			} // Have ready sector
			else // Don't have ready sector
			{
				// If writing sector to disk
				if(matchedSector->state & S_WRITE_DONE)
				{
					// This is a usage error, write with no ready sectors
					DiskCacheErrorTrap();
					DBG_INFO("write write collided"); 
					gDiskCache.appErrors++;	
					// Just set collision flag, write will re-open
				}
				// If read or pre-read transfer, usage error, possible corruption
				else if(matchedSector->state & S_READ_REQUEST)
				{
					// This is a usage violation, write to a read requested sector
					DiskCacheErrorTrap();
					DBG_INFO("write read collided"); 
					gDiskCache.appErrors++;	
					// Load may be active during write, possibly corrupting read or write
				}
				// If pre-reading, just cancel pre-read
				else //if(matchedSector->state & S_CACHE_FRONT)
				{
					// Cancel and reassign for write
					DBG_INFO("pre-read cancel");
					matchedSector->state = S_IDLE;
				}
				// Set flags for collided write on sector
				matchedSector->state |= S_WRITE_OPEN | S_WRITE_COLLIDE;
				// Clear any access errors and data-valid flag
				matchedSector->state &= ~(S_DATA_VALID | S_ACCESS_ERROR);				
				// Return matched sector
				*sector = matchedSector;
				return matchedSector->buffer;		
			}
		}// Transferring...
		
		// Check if matched sector is open for reading or has been read previously
		// If already read, use in place, will stall read if not already active		
		if(matchedSector->state & S_READ_REQUEST)
		{
			// Clear data valid flag. Will return to valid after write completes
			matchedSector->state &= ~S_DATA_VALID ;
			// This is a usage violation. 
			DiskCacheErrorTrap();
			DBG_INFO("write read override"); 
			gDiskCache.appWarnings++;					
			// The app may already be reading data which could corrupt the read
		}
		
		// Check for already writing case
		if(matchedSector->state & (S_WRITE_OPEN | S_WRITE_DONE))
		{		
			// Re-write the sector in place
			DBG_INFO("Re-write writing");
		}

		// Check for pre-emptive read queued, debug only
		if(matchedSector->state & S_CACHE_FRONT)
		{		
			// Override queueing sector
			DBG_INFO("Write to queued");
		}		
		
		// Normal writing procedure but clear errors, data-valid and write done flags
		matchedSector->state |= S_WRITE_OPEN;
		matchedSector->state &= ~(S_WRITE_DONE | S_DATA_VALID | S_ACCESS_ERROR);
		*sector = matchedSector;
		return matchedSector->buffer;
	}
	// No match, use ready sector
	if(readySector != NULL)
	{
		DBG_INFO("writing");
		readySector->state = S_WRITE_OPEN;
		readySector->lba = lba;
		*sector = readySector;	
		return readySector->buffer;
	}
	// This indicates a driver usage fault, no ready sector and no match
	DiskCacheErrorTrap();
	DBG_INFO("write fail full");
	gDiskCache.appWarnings++;	
	*sector = NULL;	
	return NULL;
}


// Finish sector, causes tasks to write sector to disk
void DiskCacheWriteClose(DiskCacheSector_t* sector)
{
	// Event indication counter
	gDiskCache.appEvents++;
	
	// Make sure the module is ready for reads
	if(!(gDiskCache.state & DC_APP_ENABLE))
	{
		// Error! Early exit if not initialised
		DiskCacheErrorTrap();
		DBG_INFO("blocked finalise");	
		gDiskCache.appWarnings++;
		return;		
	}	

	// Check for collision states - write done, read done and transferring
	if(sector->state & (S_WRITE_DONE | S_DATA_VALID | S_TRANSFERRING))
	{
		DiskCacheErrorTrap();
		DBG_INFO("write close state fault");
		gDiskCache.appWarnings++;			
	}
	
	// If sector is open, set as finalised + clear errors
	if(sector->state & S_WRITE_OPEN)
	{
		// Debug out
		DBG_INFO("app written: %s",DBG_ultoa(sector->lba));
		// Clear and set appropriate flags - write open -> write close
		sector->state &= ~(S_WRITE_OPEN | S_ACCESS_ERROR);
		sector->state |= S_WRITE_DONE;
	}
	else 
	{
		// Not in writing state 
		DiskCacheErrorTrap();
		DBG_INFO("invalid finalise: %s",DBG_ultoa(sector->lba));
		gDiskCache.appErrors++;
		// This indicates incorrect driver usage, try to recover sector
		DiskCacheIdleSector(sector);		
	}	
	return;
}

// Close currently open sector, clears ownership
void DiskCacheWriteAbort(DiskCacheSector_t* sector)
{
	// Event indication counter
	gDiskCache.appEvents++;
	
	// Make sure the disk is ready for access
	if(!(gDiskCache.state & DC_APP_ENABLE))
	{
		// Early exit if not initialised 
		DiskCacheErrorTrap();
		DBG_INFO("blocked close");
		gDiskCache.appWarnings++;
		return;		
	}	
	
	// Aborting sector write, check flags are valid
	if(sector->state & S_WRITE_OPEN)
	{
		// Debug
		DBG_INFO("write abort: %s",DBG_ultoa(sector->lba));
		// If we are supposed to read this sector
		if(sector->state & S_READ_REQUEST)
		{
			// Leave requested and queue flags alone
			sector->state &= ~(S_READ_REQUEST | S_CACHE_FRONT);		
			return;
		}		
	}
	else
	{
		// Not in writing state
		DiskCacheErrorTrap();
		DBG_INFO("invalid write abort: %s",DBG_ultoa(sector->lba));
		gDiskCache.appErrors++;	
		// This indicates incorrect driver usage
	}
	// Set sector to idle, unknown data
	DiskCacheIdleSector(sector);
	return;
}

// Move queue point forward (N.b. application must be safely blocked)
static uint8_t DiskCacheQueueNext(uint8_t index)
{
	DiskCacheSector_t *queuedSector, *currentSector;	
	// Checks, there must be at least 2 sectors to read pre-emptively
	if(gDiskCache.numSectors < 2)
		return INVALID_SECTOR_INDEX;
	// Get current sector
	currentSector = &gDiskCache.sector[index];
	// Get next index
	if(++index >= gDiskCache.numSectors)
		index = 0;
	// Get next sector
	queuedSector = &gDiskCache.sector[index];
	// Set queue point if not busy
	if((queuedSector->state & S_IS_BUSY_MASK) == 0)
	{
		// Set queued state
		queuedSector->lba = currentSector->lba + 1;
		queuedSector->state = S_CACHE_FRONT;
		// Clear previous queue flag
		currentSector->state &= ~S_CACHE_FRONT;
		// Debug
		DBG_INFO("queue moved fwd");
	}
	else
	{
		// Too busy, don't move queue position
		DBG_INFO("queueing failed");
		return INVALID_SECTOR_INDEX;
	}
	// Return the pre-emptive read point
	return index;
}

// Tasks function, is called in other scope, handles disk i/o
// Tasks deliberately returns after fixed number of read/writes to 
// reduce blocking time when multi-tasking.
void DiskCacheTasks(void)
{
	// Static index state
	static uint8_t index = 0, writeTries = 0;
	// Other local variables
	DiskCacheSector_t *sector; 
	//AsyncSectorState_t cachedState;
	uint16_t eventCheck;
	uint8_t loopTasks, diskTasks, queueStart, success;
	uint8_t numReading, numWriting;
	
	// Make sure the module is ready to run tasks
	if(!(gDiskCache.state & DC_OPERATIONAL))
		return;		

	// Save the application event count, increment the task events
	gDiskCache.taskEvents++;
	eventCheck = gDiskCache.appEvents;

	
	// Set task loop state and counters
	loopTasks = DISK_CACHE_TASK_LOOP_COUNT;	
	diskTasks = DISK_CACHE_MAX_DISK_TASKS;
	queueStart = INVALID_SECTOR_INDEX;
	numReading = numWriting = 0; 
	
	// Check the sectors for required actions
	while((diskTasks)&&(loopTasks))
	{
		// Decrement the loop counter
		loopTasks--;

		// Write retries will keep focus on writing sector
		if(writeTries != 0)
			index--;
		
		// Set next index and sector pointer
		if(++index >= gDiskCache.numSectors)index = 0;
		sector = &gDiskCache.sector[index];
		
		// Block interrupts to protect state, perform required actions
		DISK_CACHE_ISR_OFF();
		// Application blocked . . . .
		
		// Skip and count idle sectors
		if(sector->state == S_IDLE)
		{
			// Clear retries count in this case, not relevant
			writeTries = 0;			
			// Unblock application
			DISK_CACHE_ISR_ON();
			// Check next sector
			continue;
		}// Idle case...

		// Check for error conditions (unexpected states)
		if(sector->state & (S_TRANSFERRING | S_WRITE_COLLIDE))
		{
			// Clear retries count in this case, not relevant
			writeTries = 0;	
			// Register driver error
			DiskCacheErrorTrap();
			DBG_INFO("invalid state");
			gDiskCache.driverErrors++;
			// Clear error - disk may be corrupted
			DiskCacheIdleSector(sector);
			// Unblock application
			DISK_CACHE_ISR_ON();
			// Check next sector
			continue;
		}// Idle case...
		
		// Save pointer to the queue start if not already set
		if((sector->state & S_CACHE_FRONT) && (queueStart == INVALID_SECTOR_INDEX))
		{
			// New reads create protected rolling pre-emptive read point
			queueStart = index;
		}	
	
		// Handle writes first (application currently blocked)
		if(sector->state & (S_WRITE_DONE | S_WRITE_OPEN))
		{
			// Count writing and written sectors
			numWriting++;			
			// Sectors being written, wait till done then write to disk
			if(!(sector->state & S_WRITE_DONE))
			{
				// Clear write retries count as not relevant
				writeTries = 0;				
				// Unblock application
				DISK_CACHE_ISR_ON();
				// Check next sector			
				continue;
			}			
			// Set transferring flag
			sector->state |= S_TRANSFERRING;
			// Un-block interrupts for disk access
			DISK_CACHE_ISR_ON();	
			// Perform write, check for errors
			success = DISK_CACHE_SECTOR_WRITE(sector->lba, sector->buffer);
			// Decrement disk access count
			diskTasks--;	
			
			// Block interrupts, clear transferring flag and check state
			DISK_CACHE_ISR_OFF();
			// Clear transferring flag and sector write done flag				
			sector->state &= ~S_TRANSFERRING;
			
			// If two writes have collided
			if(sector->state & S_WRITE_COLLIDE)
			{
				// Clear write retries count if the retry had a write collision
				writeTries = 0;
				// Sector still open for writing, only clear write done and collision flags
				sector->state &= ~(S_WRITE_COLLIDE | S_WRITE_DONE);
				// Unblock application
				DISK_CACHE_ISR_ON();
				continue;
			}
			// If there was a write error
			if((!success) && (writeTries < DISK_CACHE_WRITE_TRIES))
			{
				// A disk access error occurred
				DiskCacheErrorTrap();
				DBG_INFO("write lba error: %s",DBG_ultoa(sector->lba));
				gDiskCache.diskErrors++;	
				// Set error flag
				sector->state |= S_ACCESS_ERROR;
				// Set retry state count. It prevents loop checking next sector
				writeTries++;
			}
			// If written ok or have reached the write try limit
			else 
			{
				// Clear the write done and error flags
				sector->state &= ~(S_WRITE_DONE | S_ACCESS_ERROR);
				
				// Check the retry limit for a write fault
				if(writeTries && (writeTries >= DISK_CACHE_WRITE_TRIES))
				{
					// A disk write error was not recoverable after try count limit
					DiskCacheErrorTrap();
					DBG_INFO("write fail limit: %s",DBG_ultoa(sector->lba));
					
					//////////////////////////////////////////////////////////	
					// There is not much that can be done for such errors.	//
					// The disk is not allowing writes to this sector.		//
					//////////////////////////////////////////////////////////
					
					// Reset try count to allow loop poll continuation loosing data
					writeTries = 0;					
				}
				else
				{
					// Buffer contains valid sector data now, it is readable 
					sector->state |= S_DATA_VALID;
					// Count if reading now
					if(sector->state & S_READ_REQUEST)
					{
						// Add to count, data can now be read
						numReading++;
					}
					// If this is the start of the pre-emptive read queue, advance queue
					if(sector->state & S_CACHE_FRONT)
					{
						// Move pre-emptive read point forward 
						queueStart = DiskCacheQueueNext(index);
					}
				}
			}		
			// Unblock application
			DISK_CACHE_ISR_ON();
			// Check next sector
			continue;
		}// Writes...	

		// Handle reads after writes (application blocked)
		if(sector->state & S_READ_REQUEST)
		{
			// Count requested sectors
			numReading++;
			// Skip if it is already read or it is in a writing (blocking) state
			if(sector->state & (S_DATA_VALID | S_WRITE_OPEN | S_WRITE_DONE))
			{
				// Unblock interrupts
				DISK_CACHE_ISR_ON();	
				continue;	
			}
			// Set transferring flag to flag as reading active
			sector->state |= S_TRANSFERRING;
			// Unblock interrupts to minimise blocking
			DISK_CACHE_ISR_ON();	
			// Perform read, keep read result
			success = DISK_CACHE_SECTOR_READ(sector->lba, sector->buffer);
			// Decrement disk access count
			diskTasks--;				

			// Block interrupts, clear transferring flag and check state
			DISK_CACHE_ISR_OFF();
			// Clear transferring flag					
			sector->state &= ~S_TRANSFERRING;			

			// If write collided, make idle unless share access violation occurred
			if(sector->state & S_WRITE_COLLIDE)
			{
				if(sector->state & S_READ_REQUEST)
				{
					// A usage error created a data violation collision
					sector->state &= ~S_WRITE_COLLIDE;
					// Sector state changed to writing, add to count
					numWriting++;
				}
				else
				{
					// Read flag was moved to other sector, clear this sector
					DiskCacheIdleSector(sector);
				}
				// Unblock application
				DISK_CACHE_ISR_ON();
				continue;
			}
			// Set the read complete flag
			sector->state |= S_DATA_VALID;		
			// Check for read errors
			if(!success)
			{
				// Requested read error
				DiskCacheErrorTrap();
				DBG_INFO("read lba error: %s",DBG_ultoa(sector->lba));
				gDiskCache.diskErrors++;			
				sector->state |= S_ACCESS_ERROR;
			}
			// If read was successful
			else
			{
				// Check if queue flag is set, move queue point
				if(sector->state & S_CACHE_FRONT)
					queueStart = DiskCacheQueueNext(index);
				// Requested read success
				DBG_INFO("read lba: %s",DBG_ultoa(sector->lba));
			}
			// Unblock application
			DISK_CACHE_ISR_ON();
			// Check next sector			
			continue;
		}// Reading case...

		// Unblock application
		DISK_CACHE_ISR_ON();
		// Check next sector			
		continue;
	}// End of task while loop

	// Pre-emptive read loop
	// Reset loop counter
	loopTasks = DISK_CACHE_TASK_LOOP_COUNT;	
	// Use up any available disk tasks as pre-emptive reads (if actively reading)
	while((queueStart != INVALID_SECTOR_INDEX) && (numReading) && (diskTasks) && (loopTasks))
	{
		DiskCacheSector_t *queuedSector;
		// Get queued sector
		queuedSector = &gDiskCache.sector[queueStart];
		// Decrement the loop counter
		loopTasks--;
		// Block interrupts to protect state
		DISK_CACHE_ISR_OFF(); 
		// Early exit if already read or state prohibits safe read
		if(queuedSector->state & S_IS_BUSY_MASK)
		{
			// Unblock application
			DISK_CACHE_ISR_ON();
			// Exit loop early
			break;
		}	
		// Try to move the queue point if it is already read and then re-check sector
		if(queuedSector->state & S_DATA_VALID)
		{
			queueStart = DiskCacheQueueNext(queueStart);
			// Unblock application
			DISK_CACHE_ISR_ON();			
			// Re-check the sector state flags
			continue;
		}
			
		// Flag as transferring
		queuedSector->state |= S_TRANSFERRING;

		// Unblock application
		DISK_CACHE_ISR_ON();

		// Perform pre-emptive reading
		success = DISK_CACHE_SECTOR_READ(queuedSector->lba, queuedSector->buffer);		
		// Decrement disk access count
		diskTasks--;				
		// Clear transferring flag					
		queuedSector->state &= ~S_TRANSFERRING;

		// The request and queued flags are volatile, block ISR
		DISK_CACHE_ISR_OFF();	

		// If write collided, discard this read, make idle
		if(queuedSector->state & S_WRITE_COLLIDE)
		{
			// Clear collision flag
			queuedSector->state &= ~S_WRITE_COLLIDE;
			// If idle, queue read was cancelled
			if(queuedSector->state == S_IDLE)
			{
				// Clear sector state
				DiskCacheIdleSector(queuedSector);
				continue;
			}
			// The sector was reassigned (now writing)
			continue;			
		}
		// If disk read failed
		if(!success)
		{
			// Requested read error
			DiskCacheErrorTrap();
			DBG_INFO("queue read error: %s",DBG_ultoa(queuedSector->lba));
			gDiskCache.diskErrors++;				
			// Read flag moved to other sector
			DiskCacheIdleSector(queuedSector);
			// Unblock application
			DISK_CACHE_ISR_ON();
			continue;
		}
		
		// Read completed ok, set the read complete flag
		queuedSector->state |= S_DATA_VALID;		
		// Requested read success
		DBG_INFO("read lba: %s",DBG_ultoa(queuedSector->lba));
		// Try to move the cache start to next sector, clears queue flag
		{
			uint8_t newQueueStart = DiskCacheQueueNext(queueStart);
			if(newQueueStart != queueStart)
			{
				// Another cache read is available, set it
				queueStart = newQueueStart;
				// Unblock application
				DISK_CACHE_ISR_ON();
				// Allow loop to continue reading
				continue;		
			}
		}
		// The cache point couldn't advance, so no more free sectors
		// Exit read caching loop early...
		break;
	}// Queued reading loop...
	
	// Block ISR to read/modify volatile states
	DISK_CACHE_ISR_OFF();
	
	// If no new events occurred during the tasks
	if(eventCheck == gDiskCache.appEvents)
	{
		// Check if cache driver is idle now
		uint8_t wasBusy = gDiskCache.state & (DC_READING | DC_WRITING);
		if(numReading == 0)
		{
			// No reads active, clear flag
			gDiskCache.state &= ~DC_READING;
		}
		if(numWriting == 0)
		{
			// No writes active or complete, clear flag
			gDiskCache.state &= ~DC_WRITING;
		}
		// Debug only, detect idle state transition busy -> idle
		if(wasBusy && (!(gDiskCache.state & (DC_READING | DC_WRITING))))
		{
			// Active to idle transition detected
			DBG_INFO("cache idle");
		}
	}
	
	#ifdef DISK_CACHE_IO_ERROR_LIMIT					
	// Check error count is within limit if enabled
	if(gDiskCache.diskErrors >= DISK_CACHE_IO_ERROR_LIMIT)
	{
		// Set io access error flag
		DiskCacheErrorTrap();
		DBG_INFO("excess io errs!");
		// If fault call back set, call it
		if(diskCacheFaultCB)
			diskCacheFaultCB();
		// Shut down driver, leave allocation
		DiskCacheShutdown(1);
		// Set disk fault flag
		gDiskCache.state |= DC_DISK_FAULT;
	}	
	#endif	
	
	#ifdef DISK_CACHE_BLOCKING_LIMIT
	// Check if the driver has become locked up and blocking
	if(gDiskCache.appBlocked > DISK_CACHE_BLOCKING_LIMIT)
	{
		// Debug output
		DiskCacheErrorTrap();
		DBG_INFO("driver locked up limit!");		
		// If fault call back set, call it
		if(diskCacheFaultCB)
			diskCacheFaultCB();
		// Shut down driver, leave allocation
		DiskCacheShutdown(1);
	}
	#endif
	
	// Unblock the ISR before returning
	DISK_CACHE_ISR_ON();
	
	// Done..
	return;
}

//EOF
