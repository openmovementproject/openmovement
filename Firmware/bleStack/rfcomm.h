// Based on Smalltooth RFCOMM code (see below).
// Changes Copyright Newcastle university (see below).

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

/*
   Copyright 2012 Guillem Vinals Gangolells <guillem@guillem.co.uk>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */

// Bluetooth RFCOMM layer
// Guillem Vinals Gangolells <guillem@guillem.co.uk>
// Changes: Karim Ladha, 2013-2014

// Changes:
// * modified for no dynamic allocation and minimal memory
// * Fixed packet length bug for 2 byte length packets
// * Added call backs on comm open and close
// * Added proper MTU support integration to L2CAP

#ifndef __RFCOMM_H__
#define __RFCOMM_H__

#include "bt config.h"
/*
 * RFCOMM definitions
 */

/* Control field values */
#define RFCOMM_SABM_FRAME 0x2F
#define RFCOMM_UA_FRAME 0x63
#define RFCOMM_DM_FRAME 0x0F
#define RFCOMM_DISC_FRAME 0x43
#define RFCOMM_UIH_FRAME 0xEF
#define RFCOMM_PF_BIT 0x10

/* Length of the frame header */
#define RFCOMM_HDR_LEN_1B 3
#define RFCOMM_HDR_LEN_2B 4

/* Length of a frame */
#define RFCOMM_SABM_LEN 4
#define RFCOMM_UA_LEN 4
#define RFCOMM_UIH_LEN 4
#define RFCOMM_UIH_CR_LEN 5
#define RFCOMM_DM_LEN 4
#define RFCOMM_DISC_LEN 4

/* Length of a multiplexer message */
#define RFCOMM_MSGHDR_LEN 2
/* Header not included */
#define RFCOMM_PNMSG_LEN 8
#define RFCOMM_MSCMSG_LEN 3
#define RFCOMM_RLSMSG_LEN 2
#define RFCOMM_RPNMSG_LEN 8
#define RFCOMM_NCMSG_LEN 1

/* Multiplexer message types */
#define RFCOMM_PN_CMD 0x83
#define RFCOMM_PN_RSP 0x81
#define RFCOMM_TEST_CMD 0x23
#define RFCOMM_TEST_RSP 0x21
#define RFCOMM_FCON_CMD 0xA3
#define RFCOMM_FCON_RSP 0xA1
#define RFCOMM_FCOFF_CMD 0x63
#define RFCOMM_FCOFF_RSP 0x61
#define RFCOMM_MSC_CMD 0xE3
#define RFCOMM_MSC_RSP 0xE1
#define RFCOMM_RPN_CMD 0x93
#define RFCOMM_RPN_RSP 0x91
#define RFCOMM_RLS_CMD 0x53
#define RFCOMM_RLS_RSP 0x51
#define RFCOMM_NSC_RSP 0x11

/* Masks */
#define RFCOMM_MASK_LI_1B 0x01
#define RFCOMM_MASK_RLS_ERROR 0x01
#define RFCOMM_MASK_RLS_OVERRUN 0x04
#define RFCOMM_MASK_RLS_PARITY 0x02
#define RFCOMM_MASK_RLS_FRAMING 0x01

/* Role configuration */
#define RFCOMM_CMD 0x01
#define RFCOMM_RSP 0x02
#define RFCOMM_DATA 0x03
#define RFCOMM_ROLE_RESPONDER 0x00
#define RFCOMM_ROLE_INITIATIOR 0x01

//#define RFCOMM_MTU 242

/*
 * RFCOMM structure definition
 */

typedef struct _RFCOMM_CHANNEL
{
    /* After receiving the SABM the channel will be established */
    unsigned char bEstablished;
	/* Maximum send size of the channel*/
	unsigned short mtu;
    /* Only relevant to the data channel, after doing the PN and the MSC */
    unsigned char bDataEnabled;
    unsigned char bDLC;
    unsigned char bLocalCr;
    unsigned char bRemoteCr;
} RFCOMM_CHANNEL;

typedef struct _RFCOMM_CONTROL_BLOCK
{
    unsigned char isInitialised;

    /* RFCOMM_NUM_CHANNELS, multiplexer control channel and data channels */
    RFCOMM_CHANNEL asChannel[RFCOMM_NUM_CHANNELS];

    /* The role can be either initiator (0x01) or responder (0x00) */
    unsigned char bRole;

} RFCOMM_CONTROL_BLOCK;

// User functions
unsigned char RFCOMM_create(void);
unsigned short RfcommMaxPayload(void);
unsigned char RfCommSendPacket(const unsigned char *pData, unsigned short uLen);
unsigned char RfCommDisconnect(unsigned char bChannel);
void RfCommInstallDataCB (	unsigned char(*RfCommDataCb)(const unsigned char *pData, unsigned short uLen));

unsigned char RfcommPutData(const unsigned char *pData, unsigned short uLen);

/*
 * RFCOMM layer private function prototypes
 */

unsigned char _RFCOMM_getAddress(unsigned char bChNumber, unsigned char bType);
RFCOMM_CHANNEL* _RFCOMM_getChannel(unsigned char uChNumber);

unsigned char _RFCOMM_sendUA(unsigned char bChNum);
unsigned char _RFCOMM_sendUIH(unsigned char bChNum, const unsigned char *pData, unsigned short uLen);
unsigned char _RFCOMM_sendUIHCr(unsigned char bChNum, unsigned char uNumCr);

unsigned char _RFCOMM_handlePN(const unsigned char *pMsgData, unsigned char uMsgLen);
unsigned char _RFCOMM_handleRPN(const unsigned char *pMsgData, unsigned char uMsgLen);
unsigned char _RFCOMM_handleRLS(const unsigned char *pMsgData, unsigned char uMsgLen);
unsigned char _RFCOMM_handleMSC(const unsigned char *pMsgData, unsigned char uMsgLen);
unsigned char _RFCOMM_handleTEST(const unsigned char *pMsgData, unsigned char uMsgLen);

#endif /*__RFCOMM_H__*/
