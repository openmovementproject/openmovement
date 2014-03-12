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

// NAND1-Specific Flash Control
// Dan Jackson, 2012

#include "HardwareProfile.h"
#if defined(FLASH_CE) || defined(FLASH_CE_PIN)
#error "FLASH_CE must not be defined when using multi-nand"
#endif

// --- NAND 1 ---
#define FLASH_CE_PIN        FLASH_CE1_PIN
#define FLASH_CE            FLASH_CE1

// Remap the names to be device-specific
//#define NAND_NO_GLOBALS     // Allow globals only for NAND 1
#define nandPresent         nand1Present
#define NandInitialize      Nand1Initialize
#define NandShutdown        Nand1Shutdown
#define NandReadDeviceId    Nand1ReadDeviceId
#define NandVerifyDeviceId  Nand1VerifyDeviceId
#define NandEraseBlock      Nand1EraseBlock
#define NandCopyPage        Nand1CopyPage
#define NandLoadPageRead    Nand1LoadPageRead
#define NandReadBuffer      Nand1ReadBuffer
#define NandLoadPageWrite   Nand1LoadPageWrite
#define NandWriteBuffer     Nand1WriteBuffer
#define NandStorePage       Nand1StorePage
#define NandStorePageRepeat Nand1StorePageRepeat
#define NandReadParameters  Nand1ReadParameters
#define _rbwait_timeout_count _rbwait_timeout_count1

// Use the actual implementation in Nand.c
#include "Nand.c"

// --------------
