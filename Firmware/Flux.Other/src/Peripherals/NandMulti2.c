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

// NAND2-Specific Flash Control
// Dan Jackson, 2012

#include "HardwareProfile.h"
#if defined(FLASH_CE) || defined(FLASH_CE_PIN)
#error "FLASH_CE must not be defined when using multi-nand"
#endif

// --- NAND 2 ---
#define FLASH_CE_PIN        NAND_DEVICE2_CE_PIN
#define FLASH_CE            NAND_DEVICE2_CE

// Remap the names to be device-specific
#define NAND_NO_GLOBALS     // Allow globals only for NAND 1
#define nandPresent         nand2Present
#define NandInitialize      Nand2Initialize
#define NandShutdown        Nand2Shutdown
#define NandReadDeviceId    Nand2ReadDeviceId
#define NandVerifyDeviceId  Nand2VerifyDeviceId
#define NandEraseBlock      Nand2EraseBlock
#define NandCopyPage        Nand2CopyPage
#define NandLoadPageRead    Nand2LoadPageRead
#define NandReadBuffer      Nand2ReadBuffer
#define NandLoadPageWrite   Nand2LoadPageWrite
#define NandWriteBuffer     Nand2WriteBuffer
#define NandStorePage       Nand2StorePage
#define NandStorePageRepeat Nand2StorePageRepeat
#define NandReadParameters  Nand2ReadParameters
#define NandReadBuffer512WordSummed	Nand2ReadBuffer512WordSummed
#define NandWritePage		Nand2WritePage
#define _rbwait_timeout_count _rbwait_timeout_count2

// Use the actual implementation in Nand.c
#include "Nand.c"

// --------------
