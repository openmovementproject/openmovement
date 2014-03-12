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

// NVM variable location
// Karim Ladha, 2013-2014

// Series of defines controlling where variables are located on the NVM

#ifndef _bt_nvm_h_
#define _bt_nvm_h_

// NVM API
// Types
#define BD_ADD_SIZE 6
#define BD_LINK_KEY_SIZE 16
typedef struct {
	unsigned char bd_add[BD_ADD_SIZE];
	unsigned char link_key[BD_LINK_KEY_SIZE];
}linkKey_t;

// Globals
extern unsigned long device_cod_local;						// Device cod to be set on controller

// Prototypes
extern void InitNvm(void);									// Call before BtInit to read nvm etc
extern char ReadNvm	(unsigned long address, void *buffer, unsigned short length); 		// NVM API
extern char WriteNvm(unsigned long pageAddress, void *buffer, unsigned short length); 	// NVM API
extern unsigned char FindLinkKeyForBD(linkKey_t* pktBuff); 	// Returns true and copies link key to pktBuff if found
extern unsigned char StoreLinkKey(linkKey_t* newKey);		// Stores key to nvm
extern const unsigned long cc256x_init_script_size;			// Init script size
extern char* GetBTName(void);								// Called to get name pointer to ram
extern char* GetBTPin(void);								// Called to get char pointer to pin code
extern char* GetBTMAC(void);								// Called to get ram pointer to mac
extern unsigned long GetBTCOD(void);						// Called to read cod
extern void SaveBTMAC(void);								// Called by stack when mac of our controller is read
#endif
