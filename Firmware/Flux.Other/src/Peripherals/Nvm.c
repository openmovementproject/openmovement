// Karim Ladha 2014 
// Standard nvm driver for all flash devices
#include <string.h>
#include <stdint.h>
#include "Compiler.h"
#include "Config.h"
#if defined(EEPROM_START_ADDRESS) && defined(EEPROM_SIZE_BYTES)
	#include "Peripherals/Eeprom.h"
#endif

// Checks
#if !defined(ERASE_BLOCK_SIZE) || !defined(WRITE_PAGE_SIZE)
	#error "Specify flash parameters for chip used"
#endif
#if ((ERASE_BLOCK_SIZE&(ERASE_BLOCK_SIZE-1))!=0) || ((WRITE_PAGE_SIZE&(WRITE_PAGE_SIZE-1))!=0)
	#error "Flash parameters must be powers of two"
#endif

// Warnings
#ifndef SCRATCH_BLOCK_ADD
	#warning "Without a scratch block this driver will stack a full erase block during writes."
#endif

// Definitions
#define NVM_ERASE_BLOCK_MASK	(~((uint32_t)ERASE_BLOCK_SIZE-1))
#define NVM_WRITE_PAGE_MASK		(~((uint32_t)WRITE_PAGE_SIZE-1))

// Globals
// Make a variable to prevent linking instruction code into scratch block (see below)!!!

// Cleanup code and compiler specific stuff
#if defined(__C30__)
	#define NVM_STARTUP()	\
							uint16_t oldTBLPAG = TBLPAG;\
							uint16_t IPL_save = SRbits.IPL;\
							SRbits.IPL = 7;
	#define NVM_CLEANUP()	\
							TBLPAG = oldTBLPAG;\
							SRbits.IPL = IPL_save;
	#ifdef SCRATCH_BLOCK_ADD
		const uint8_t __prog__  __attribute__((space(prog),address(SCRATCH_BLOCK_ADD))) scratchBlock[ERASE_BLOCK_SIZE];
	#endif
#elif defined(__arm__)
	// NVM of the nRF51 series
	#define NVM_STARTUP()
	#define NVM_CLEANUP()
	#ifdef SCRATCH_BLOCK_ADD
		const uint8_t scratchBlock[ERASE_BLOCK_SIZE] __attribute__((section(".scratchblock"),aligned(ERASE_BLOCK_SIZE), used)) ;
		#undef SCRATCH_BLOCK_ADD
		#define SCRATCH_BLOCK_ADD	((uint32_t)scratchBlock)
	#endif
#endif

// Internal API - Required per device...
uint8_t EraseNvmBlock(uint32_t address);
uint8_t ReadNvmPage(uint32_t address, void* buffer);
uint8_t WriteNvmPage(uint32_t address, void* buffer);

// Source
uint8_t ReadNvm(uint32_t address, void *buffer, uint16_t length)
{
	uint8_t retval = 0;
	// Using externally mapped eeprom
	#if defined(EEPROM_START_ADDRESS) && defined(EEPROM_SIZE_BYTES)
	if (address >= EEPROM_START_ADDRESS)
	{
		address -= EEPROM_START_ADDRESS;
		if((address + length) <= EEPROM_SIZE_BYTES)
			retval = I2C_eeprom_read(address, buffer, length);
	}
	else
	#endif	
	{
		retval = 1;
		NVM_STARTUP();	
		// Using nvm api, flash memory
		while(length > 0)
		{
			// Calculate parameters
			uint32_t pageAddress = address&NVM_WRITE_PAGE_MASK;
			uint16_t pageOffset = address&(~NVM_WRITE_PAGE_MASK);
			uint16_t readLen = ((length+pageOffset) > WRITE_PAGE_SIZE)?(WRITE_PAGE_SIZE-pageOffset):length;
			// Read page to ram
			__attribute__((aligned(8)))uint8_t pageBuffer[WRITE_PAGE_SIZE];
			ReadNvmPage(pageAddress, pageBuffer);
			// Copy out from offset
			memcpy(buffer,&pageBuffer[pageOffset],readLen);
			// Adjust address and length
			buffer += readLen;
			address += readLen;
			length -= readLen;
		}
		NVM_CLEANUP();
	}
	return retval;
}


// If we have a scratch block, use it, if not then use stack
uint8_t WriteNvm(uint32_t address, void *buffer, uint16_t length)
{
	uint32_t readAdd, writeAdd;
	#ifdef SCRATCH_BLOCK_ADD	
	__attribute__((aligned(8)))uint8_t pageBuffer[WRITE_PAGE_SIZE];
	#else
	__attribute__((aligned(8)))uint8_t blockBuffer[ERASE_BLOCK_SIZE];	// Very large stacked var
	uint8_t *blockPtr;
	#endif
	uint16_t offset, maxLen, blockRemainder, dataRemainder;
	uint8_t *pagePtr, *source = (uint8_t*)buffer;
	uint8_t retval = 0;
	
	// Set data remaining counter
	dataRemainder = length;
		
	// Early outs
	if(length == 0)
		retval = 1;
	else if(buffer == NULL)
		retval = 0; 
	// Externally mapped eeprom
	#if defined(EEPROM_START_ADDRESS) && defined(EEPROM_SIZE_BYTES)
	else if (address >= EEPROM_START_ADDRESS)
	{
		address -= EEPROM_START_ADDRESS;
		if((address + length) <= EEPROM_SIZE_BYTES)
		{
			retval = I2C_eeprom_write(address, buffer, length);
			I2C_eeprom_wait();
		}
		// Diverted
		break;
	}
	#endif
	else if ((address < MIN_ALLOWED_WRITE_ADD) || ((address+length) > MAX_ALLOWED_WRITE_ADD)) // Checks on nvm address
		retval = 0;
	else
	{
		// Begin cleanup
		NVM_STARTUP();	
		
		// Check if writing aligned full block for faster write
		if(((address & (ERASE_BLOCK_SIZE-1)) == 0) && ((length & (ERASE_BLOCK_SIZE-1)) == 0))
		{
			uint16_t i;
			// Fast aligned write, integer blocks at aligned address
			while(length > ERASE_BLOCK_SIZE)
			{
				EraseNvmBlock(address);
				for(i=0;i<ERASE_BLOCK_SIZE;i+=WRITE_PAGE_SIZE,address+=WRITE_PAGE_SIZE,buffer+=WRITE_PAGE_SIZE,length-=WRITE_PAGE_SIZE)
					WriteNvmPage(address, buffer);
			}
			NVM_CLEANUP();
			return 1;
		}

		// Begin
		for(;;)
		{
			// Address of the block being written into, now reading
			readAdd = address & NVM_ERASE_BLOCK_MASK;		
			
			// Copy the original block out to ram or rom
			#ifdef SCRATCH_BLOCK_ADD
				writeAdd = SCRATCH_BLOCK_ADD;
				EraseNvmBlock(writeAdd);
				for(blockRemainder=ERASE_BLOCK_SIZE;blockRemainder>0;blockRemainder-=WRITE_PAGE_SIZE)
				{
					ReadNvmPage(readAdd,pageBuffer);
					WriteNvmPage(writeAdd,pageBuffer);
					readAdd += WRITE_PAGE_SIZE;
					writeAdd += WRITE_PAGE_SIZE;
				}
				readAdd = SCRATCH_BLOCK_ADD;
			#else
				blockPtr = blockBuffer;
				for(blockRemainder=ERASE_BLOCK_SIZE;blockRemainder>0;blockRemainder-=WRITE_PAGE_SIZE)
				{
					ReadNvmPage(readAdd,blockPtr);
					readAdd += WRITE_PAGE_SIZE;
					blockPtr += WRITE_PAGE_SIZE;
				}
				blockPtr = blockBuffer;
			#endif
			
			// Set addresses for copy back
			writeAdd = address & NVM_ERASE_BLOCK_MASK;	
			// Erase the block being written into
			EraseNvmBlock(writeAdd);
			// Set block remainder to full block
			blockRemainder = ERASE_BLOCK_SIZE;
			// Find start of data region offset within the block copy
			offset = address & (ERASE_BLOCK_SIZE-1);	
		
			// Write in data
			while(blockRemainder > 0)
			{
				// Find max data length within this page
				maxLen = 0;
				if(offset < WRITE_PAGE_SIZE)
					maxLen = WRITE_PAGE_SIZE - offset;
				// Truncate if past end of data
				if(maxLen > dataRemainder) 
					maxLen = dataRemainder;
				#ifdef SCRATCH_BLOCK_ADD
					// Pointer to start of page in page buffer
					pagePtr = pageBuffer;
					// Copy page to ram
					ReadNvmPage(readAdd, pagePtr);
					// Adjust scratch read pos
					readAdd += WRITE_PAGE_SIZE;		
				#else
					// Pointer to start of page in block buffer
					pagePtr = blockPtr;
					// Adjust block buff read pos
					blockPtr += WRITE_PAGE_SIZE;		
				#endif
				// Copy source into page ram if within current page
				if(maxLen > 0)
				{
					memcpy(pagePtr+offset,source,maxLen);	
					// Adjust source pointer, data position and remainder
					source += maxLen;
					dataRemainder -= maxLen;
				}
				// Write page to destination
				WriteNvmPage(writeAdd,pagePtr);
				// Reduce offset to data
				if(offset >= WRITE_PAGE_SIZE)
					offset -= WRITE_PAGE_SIZE;
				else
					offset = 0;
				// Adjust dest pointers
				writeAdd += WRITE_PAGE_SIZE;
				blockRemainder -= WRITE_PAGE_SIZE;
			}
			// Check if done, exit for loop
			if(dataRemainder == 0)
				break;
			// Set address to next block, if spanning a block
			address = (address & NVM_ERASE_BLOCK_MASK) + ERASE_BLOCK_SIZE;
		} // for(;;)
		// Done, clean up for nvm case
		NVM_CLEANUP();
	}// else
	
	return 1;
}

#if defined(__C30__)
// Internal erase block function
uint8_t EraseNvmBlock(uint32_t address)
{
	NVMCON = 0x4042;					// Page erase on next write
	TBLPAG = address >> 16;				// Top 8 bits of address (bottom bits not needed)
	__builtin_tblwtl((uint16_t)address, 0xffff); // A dummy write to load address of erase page
	asm("DISI #16");					// Disable interrupts for unlock sequence
	__builtin_write_NVM();				// Erase scratch page
	return 1;
}
// Internal read page function
uint8_t ReadNvmPage(uint32_t address, void* buffer)
{
	uint16_t i;
	TBLPAG = (uint16_t)(address >> 16);		// Top 8 bits of address
	for(i = 0; i < (WRITE_PAGE_SIZE/2); i++)
	{
		*(uint16_t *)buffer = __builtin_tblrdl((uint16_t)address);	// Read the bottom two bytes of the instruction
		buffer += sizeof(uint16_t);
		address += 2;
	}
	return 1;
}
// Internal write page function
uint8_t WriteNvmPage(uint32_t address, void* buffer)
{
	uint16_t i;
	TBLPAG = (uint16_t)(address >> 16);		// Set page register
	NVMCON = 0x4001;					// Page write		 
	for(i = 0; i < (WRITE_PAGE_SIZE/2); i++)
	{
		__builtin_tblwtl(address, *(uint16_t*)buffer); 
		__builtin_tblwth(address, 0xff);
		buffer+=sizeof(uint16_t);
		address+=2;
	}
	asm("DISI #16");					// Disable interrupts for unlock sequence
	__builtin_write_NVM();				// Write the page
	return 1;
}
#elif defined(__arm__)
// NVM of the nRF51 series
// Internal erase block function
uint8_t EraseNvmBlock(uint32_t address)
{
	// Check alignment
	if(address & (~NVM_ERASE_BLOCK_MASK)) return 0;
	// Erase block
	NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Een;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
	NRF_NVMC->ERASEPAGE = address;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
	NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
	return 1;
}
// Internal read page function
uint8_t ReadNvmPage(uint32_t address, void* buffer)
{
	memcpy((uint8_t*)buffer,(uint8_t*)address,WRITE_PAGE_SIZE);
	return 1;
}
// Internal write page function
uint8_t WriteNvmPage(uint32_t address, void* buffer)
{
	uint32_t aligned; /* Align to 32 bit word */
	memcpy((void*)&aligned,buffer,sizeof(uint32_t)); 
	NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
	*(uint32_t*)address = aligned;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
	NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
	return 1;
}
#endif
