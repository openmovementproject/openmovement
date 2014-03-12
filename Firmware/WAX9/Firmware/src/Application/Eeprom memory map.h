/* 
 * Copyright (c) 2013-2014, Newcastle University, UK.
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

// EEPROM variable location
// Karim Ladha, 2013-2014

// Series of defines controlling where variables are located on the eeprom

#ifndef _eeprom_mem_map_h_
#define _eeprom_mem_map_h_

#include "HardwareProfile.h" // Contains eeprom base address

#define SETTINGS_ADDRESS 		(EEPROM_START_ADDRESS + 0)
#define DEVICE_NAME_NVM			(EEPROM_START_ADDRESS + 0x100)
#define DEVICE_PAIRING_CODE		(EEPROM_START_ADDRESS + 0x180)
#define DEVICE_MAC_ADDRESS		(EEPROM_START_ADDRESS + 0x1C0)
#define LINK_KEY_ADDRESS 		(EEPROM_START_ADDRESS + 0x200)
#define INIT_SCRIPT_BASE_ADD 	(EEPROM_START_ADDRESS + 0x6FC)

extern const unsigned long cc256x_init_script_size;

#endif
