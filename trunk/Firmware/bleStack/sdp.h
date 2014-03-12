// Based on Smalltooth SDP code (see below).
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

#ifndef __SDP_H__
#define __SDP_H__

#define SDP_DATA_T_NIL 0x00
#define SDP_DATA_T_UINT 0x08
#define SDP_DATA_T_SINT 0x10
#define SDP_DATA_T_UUID 0x18
#define SDP_DATA_T_STR 0x20
#define SDP_DATA_T_BOOL 0x28
#define SDP_DATA_T_DES 0x30
#define SDP_DATA_T_DEA 0x38
#define SDP_DATA_T_URL 0x40

#define SDP_DATA_S_8 0x0
#define SDP_DATA_S_16 0x1
#define SDP_DATA_S_32 0x2
#define SDP_DATA_S_64 0x3
#define SDP_DATA_S_128 0x4
#define SDP_DATA_S_1B 0x5
#define SDP_DATA_S_2B 0x6
#define SDP_DATA_S_4B 0x7

/* PDU identifiers */
#define SDP_ERR_PDU 0x01
#define SDP_SS_PDU 0x02
#define SDP_SSR_PDU 0x03
#define SDP_SA_PDU 0x04
#define SDP_SAR_PDU 0x05
#define SDP_SSA_PDU 0x06
#define SDP_SSAR_PDU 0x07

/* Response lengths and sizes */
#define SDP_HDR_LEN 5
/*
 * ServiceSearchReq minimum Length: 7byte
 * ServiceSearchPattern (with at least one UUID16) = 4byte
 * MaxServiceRecordCount = 2byte
 * ContinuationSate = 1byte
 */
#define SDP_SS_REQ_MIN_LEN 7
/*
 * ServiceSearchRsp minimum Length: 5byte
 * TotalServiceRecordCount = 2byte
 * CurrentServiceRecordCount = 2byte
 * ServiceRecordHandleList = 0byte
 * ContinuationSate = 1byte
 */
#define SDP_SS_RSP_MIN_LEN 5
/*
 * ServiceAttributeReq minimum Length: 11byte
 * ServiceRecordHandle = 4byte
 * MaxAttributeByteCount = 2byte
 * AttributeIDList (with at least one ID) = 4byte
 * ContinuationSate = 1byte
 */
#define SDP_SA_REQ_MIN_LEN 11
/*
 * ServiceSearchAttributeRsp minimum Length: 3byte
 * AttributeListsByteCount = 2byte
 * AttributeList (with 0 attributes) = 0byte
 * ContinuationSate = 1byte
 */
#define SDP_SA_RSP_MIN_LEN 3
/*
 * ServiceSearchAttributeReq minimum Length: 11byte
 * ServiceSearchPattern (with at least one UUID16) = 4byte
 * MaxAttributeByteCount = 2byte
 * AttributeIDList (with at least one ID) = 4byte
 * ContinuationSate = 1byte
 */
#define SDP_SSA_REQ_MIN_LEN 11
/*
 * ServiceAttributeRsp minimum Length: 3byte
 * AttributeListsByteCount = 2byte
 * AttributeList (with 0 attributes) = 0byte
 * ContinuationSate = 1byte
 */
#define SDP_SSA_RSP_MIN_LEN 3

#define SDP_MAX_FRAME_SIZE 128

#define SDP_SERVICE_RFCOMM_ENABLE
#define SDP_SERVICE_COUNT 1

typedef struct _SDP_SERIVCE_ATTRIBUTE
{
    /* Attribute ID */
    unsigned short uID; 
    /* Attribute Value */
    unsigned short uValueLen;
    unsigned char *pValue;
} SDP_SERVICE_ATTRIBUTE;

typedef struct _SDP_SERVICE
{
    char *pcName;
    unsigned short uNumAttrs;
    SDP_SERVICE_ATTRIBUTE *pAttrs;
} SDP_SERVICE;

typedef struct _SDP_CONTROL_BLOCK
{
    unsigned char bInitialised;
    SDP_SERVICE *pService[SDP_SERVICE_COUNT];
//    unsigned char (*L2CAPsendData)(unsigned short, const unsigned char*, unsigned short);
} SDP_CONTROL_BLOCK;


// Only one user function - initialisation
unsigned char SDP_create(void);

/*
 * SDP layer private function prototypes
 */

unsigned char SDP_API_putPetition(const unsigned char * pData, unsigned short uLen);
//unsigned char SDP_API_putPetition(const unsigned char *pData, unsigned short uLen);

unsigned char _SDP_handlePetition(unsigned char bPDUID, unsigned short uTID, const unsigned char *pData, unsigned short uLen);
unsigned char _SDP_sendSSResp(unsigned short uTID, const unsigned char *pData, unsigned short uLen);
unsigned char _SDP_sendSAResp(unsigned short uTID, const unsigned char *pData, unsigned short uLen);
unsigned char _SDP_sendSSAResp(unsigned short uTID, const unsigned char *pData, unsigned short uLen);

unsigned short _SDP_getUUIDs(const unsigned char *pServiceSearchPattern, unsigned long *pUUID,
        unsigned short *pReqOffset);
unsigned short _SDP_getServiceRecordList(const unsigned long *pUUID, unsigned short uLen,
        SDP_SERVICE *ppsServiceList[]);
unsigned short _SDP_getAttrList(const SDP_SERVICE *pService,
        const unsigned char *pAttrIDList,
        unsigned char *pAttrList);

#endif /*__SDP_H__*/
