// Karim Ladha 2014 
// Standard hardware specofu  nvm driver api

#ifndef _NVM_H_
#define _NVM_H_

#include <stdint.h>
/* 
Include a config.h in the project
	Required definitions
		define MIN_ALLOWED_WRITE_ADD	// Write protect start of code (bootloader)
		define MAX_ALLOWED_WRITE_ADD	// Write protect end of code (config/settings)
		define WRITE_PAGE_SIZE			// Size of nvm page
		define ERASE_BLOCK_SIZE			// Size of nvm block
	Optional definitions
		define SCRATCH_BLOCK_ADD		// Use intermediary nvm block, stack size = page size
		define EEPROM_START_ADDRESS		// Map eeprom to memory here with defined length 
		define EEPROM_SIZE_BYTES		// Uses I2C_eeprom_read/write(add,buff,len) from Eeprom.h
*/

// Read from non-volatile memory
uint8_t ReadNvm(uint32_t address, void *buffer, uint16_t length);

// Write non-volatile memory as if it were eeprom
uint8_t WriteNvm(uint32_t pageAddress, void *buffer, uint16_t length);

// Extended, non-user api, no context saving
uint8_t EraseNvmBlock(uint32_t address);
uint8_t ReadNvmPage(uint32_t address, void* buffer);
uint8_t WriteNvmPage(uint32_t address, void* buffer);

#endif
