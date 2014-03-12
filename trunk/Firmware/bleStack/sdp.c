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

// Changes:
// * modified for no dynamic allocation and minimal memory
// * long uuid search support added for BT uuid types

#include <stdlib.h>
#include "l2cap + le.h"
#include "sdp.h"
#include "bt config.h"
#include "debug.h"

// DEBUG
#if DEBUG_SDP
	static const char* file = "sdp.c";
	#define ASSERT(X)			DBG_assert(X, file, __LINE__)
    #define DBG_INFO(X, ...) 	DBG_info(file,X, ##__VA_ARGS__);
    #define DBG_DUMP(X, Y)		DBG_dump(X, Y);
    #define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
#else
    #define DBG_INFO(X, ...)
    #define DBG_DUMP(X, Y)
	#if DBG_ALL_ERRORS			// Leave only errors and assertions on	
		static const char* file = "sdp.c";
    	#define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
		#define ASSERT(X)			DBG_assert(X, file, __LINE__)
	#else
    	#define DBG_ERROR(X, ...)
		#define ASSERT(X)
	#endif
#endif

// Is it a bt sig uuid, expects big endian ptr
unsigned char IsBtSigUuid(const unsigned char* uuid)
{	
	unsigned char i = 0;
	const unsigned char bt_sig_uuid[] = {0,0,0,0,	0,0,	0x10,0x00,	0x80,0x00,	0x00,0x80,0x5f,0x9b,0x34,0xfb};
	for(i=4;i<16;i++)
	{
		if(uuid[i] != bt_sig_uuid[i]) return FALSE_;
	} 
	return TRUE_;
}

#ifdef SDP_SERVICE_RFCOMM_ENABLE
const unsigned char aRFCOMMAttr0_Val[] = {
    /*UUID 32 bit*/
    (SDP_DATA_T_UINT|SDP_DATA_S_32),
    0x00, 0x01, 0x00, 0x01
};
const unsigned char aRFCOMMAttr1_Val[] = {
    /*3 byte element data sequence*/
    SDP_DATA_T_DES|SDP_DATA_S_1B, 0x11,
    /*UUID 16 bits*/
    SDP_DATA_T_UUID|SDP_DATA_S_128,
    0x00, 0x00, 0x11, 0x01,
    0x00, 0x00, 0x10, 0x00,
    0x80, 0x00, 0x00, 0x80,
    0x5F, 0x9B, 0x34, 0xFB
};
const unsigned char aRFCOMMAttr2_Val[] = {
    /*12 byte element data sequence*/
    SDP_DATA_T_DES|SDP_DATA_S_1B, 0x0C,
    /*
     * L2CAP protocol descriptor:
     * UUID = 0x0100
     */
    SDP_DATA_T_DES|SDP_DATA_S_1B, 0x03,
    SDP_DATA_T_UUID|SDP_DATA_S_16,
    0x01, 0x00,
    /*
     * RFCOMM protocol descriptor:
     * UUID = 0x0003
     * CHANNEL = RFCOMM_CH_NUM
     */
    SDP_DATA_T_DES|SDP_DATA_S_1B, 0x05,
    SDP_DATA_T_UUID|SDP_DATA_S_16,
    0x00, 0x03,
    SDP_DATA_T_UINT|SDP_DATA_S_8,
    RFCOMM_CH_DATA
};
const unsigned char aRFCOMMAttr3_Val[] = {
    /*3 byte element data sequence*/
    SDP_DATA_T_DES|SDP_DATA_S_1B, 0x03,
    /*UUID 16 bits (Public Browse Group)*/
    SDP_DATA_T_UUID|SDP_DATA_S_16,
    0x10, 0x02
};
const unsigned char aRFCOMMAttr4_Val[] = {
    /* 9 byte element data sequence */
    SDP_DATA_T_DES|SDP_DATA_S_1B, 0x09,
    /*
     * Natural language ID.
     */
    SDP_DATA_T_UINT|SDP_DATA_S_16,
    'e', 'n',
    /*
     * Language encoding:
     * unsigned short = 0x6A (UTF-8)
     */
    SDP_DATA_T_UINT|SDP_DATA_S_16,
    0x00, 0x6A,
    /*
     * Base attribute ID for the language in the Service Record:
     * unsigned short = 0x0100
     */
    SDP_DATA_T_UINT|SDP_DATA_S_16,
    0x01, 0x00
};
const unsigned char aRFCOMMAttr5_Val[] = {
    /* 11 byte data element sequence */
    SDP_DATA_T_STR|SDP_DATA_S_1B, 0x05,
    /* Service record name string */
    'C', 'O', 'M', '1', 0x00
};

const SDP_SERVICE_ATTRIBUTE aRFCOMMAttrs[] = {
    /*
     * RFCOMM Attribute 0: Handler
     * ID 0x0000
     * Value:
     *     Service handler (UUID32bit)
     */
    { .uID = 0x0000, .uValueLen = 5, .pValue = (unsigned char *)aRFCOMMAttr0_Val },
    /*
     * RFCOMM Attribute 1: Service Class ID List
     * ID 0x0001
     * Value:
     *     Data element sequence of service classes (UUID16bit) that
     *     the service record conforms to.
     */
    { .uID = 0x0001, .uValueLen = 19, .pValue = (unsigned char *)aRFCOMMAttr1_Val },
    /*
     * RFCOMM Attribute 2: Protocol Descriptor List
     * ID 0x0004
     * Value:
     *     Data element sequence of protocol stacks that can be used to
     *     access the service described by the record.
     */
    { .uID = 0x0004, .uValueLen = 14, .pValue = (unsigned char *)aRFCOMMAttr2_Val },
    /*
     * RFCOMM Attribute 3: Service Class ID List
     * ID 0x0005
     * Value:
     *     Data element sequence of browse groups (UUID16bit) the
     *     service record belongs to.
     */
    { .uID = 0x0005, .uValueLen = 5, .pValue = (unsigned char *)aRFCOMMAttr3_Val },
    /*
     * RFCOMM Attribute 4: Language Base ID Attribute List
     * ID 0x0005
     * Value:
     *     A list of language bases. It contains a language identifier,
     *     a character encoding indentifier and a base attribute ID for
     *     the languages used in the service record.
     */
    { .uID = 0x0006, .uValueLen = 11, .pValue = (unsigned char *)aRFCOMMAttr4_Val },
    /*
     * RFCOMM Attribute 5: Service Name
     * ID 0x0000 + BaseAttributeID offset
     * Value:
     *     String containing the name of the service specified in the
     *     service record.
     */
    { .uID = 0x0100, .uValueLen = 7, .pValue = (unsigned char *)aRFCOMMAttr5_Val }
};

#endif /*SDP_SERVICE_RFCOMM_ENABLE*/

#ifdef SDP_SERVICE_RFCOMM_ENABLE
const SDP_SERVICE sServiceRFCOMM = {
    .pcName = "RFCOMM",
    .uNumAttrs = 6,
    .pAttrs = (SDP_SERVICE_ATTRIBUTE *)aRFCOMMAttrs
};
#endif /*SDP_SERVICE_RFCOMM_ENABLE*/

static SDP_CONTROL_BLOCK gSDPCB;
static SDP_CONTROL_BLOCK *gpsSDPCB = NULL;

/*
 * SDP public functions implementation
 */

unsigned char SDP_create()
{
    unsigned short i;

	gpsSDPCB = &gSDPCB;

    /* Initialise the control block structure */
    gpsSDPCB->bInitialised = TRUE_;
    for (i = 0; i < SDP_SERVICE_COUNT; ++i)
    {
        gpsSDPCB->pService[i] = NULL;
    }
    #ifdef SDP_SERVICE_RFCOMM_ENABLE
        gpsSDPCB->pService[0] = (SDP_SERVICE *)&sServiceRFCOMM;
    #else
        #error
    #endif /*SDP_SERVICE_RFCOMM_ENABLE*/

	L2capInstallSdp(SDP_API_putPetition);

    return TRUE_;
}

/*
 * SDP API functions implementation
 */
unsigned char SDP_API_putPetition(const unsigned char * pData, unsigned short uLen)
{
    unsigned char bPDUID;
    unsigned short uTID, uParameterLen;
    unsigned char bRetVal = FALSE_;

    /* SDP CB memory not allocated */
    ASSERT(NULL != gpsSDPCB);

    /* SDP not initialised */
    if (!gpsSDPCB->bInitialised)
    {
        DBG_ERROR("no init\n");
        return FALSE_;
    }

    /* Check the PDU data length */
    if (NULL == pData || uLen < SDP_HDR_LEN)
    {
        DBG_ERROR( "pdu err\n");
        return FALSE_;
    }

    /* Read the PDU header */
    bPDUID = pData[0];
    uTID = BT_READBE16(&pData[1]);
    uParameterLen = BT_READBE16(&pData[3]);

    /* Handle the petition */
    bRetVal = _SDP_handlePetition(bPDUID, uTID, &pData[SDP_HDR_LEN],
            uParameterLen);

    return bRetVal;
}

/*
 * SDP private functions implementation
 */

unsigned char _SDP_handlePetition(unsigned char bPDUID, unsigned short uTID, const unsigned char *pData,
        unsigned short uLen)
{
    unsigned char bRetVal = FALSE_;
    switch(bPDUID)
    {
        case SDP_ERR_PDU:
            DBG_INFO( "err pdu\n");
            bRetVal = FALSE_;
            break;

        case SDP_SS_PDU:

            /* Input data length check. */
            if ((NULL == pData) || (uLen < SDP_SS_REQ_MIN_LEN))
            {
                DBG_ERROR( "req frame err\n");
            }

            DBG_INFO( "SSR.uTID=%02X\n", uTID);
            DBG_DUMP(pData, uLen);

            bRetVal = _SDP_sendSSResp(uTID, pData, uLen);
            break;

        case SDP_SA_PDU:

            /* Input data length check. */
            if ((NULL == pData) || (uLen < SDP_SA_REQ_MIN_LEN))
            {
                DBG_ERROR( "req frame err\n");
            }

            DBG_INFO( "SAR.uTID=%02X\n", uTID);
            DBG_DUMP(pData, uLen);

            bRetVal = _SDP_sendSAResp(uTID, pData, uLen);
            break;

        case SDP_SSA_PDU:

            /* Input data length check. */
            if ((NULL == pData) || (uLen < SDP_SSA_REQ_MIN_LEN))
            {
                DBG_ERROR( "req frame err\n");
            }

            DBG_INFO( "SSAR.uTID=%02X\n", uTID);
            DBG_DUMP(pData,uLen);

            bRetVal = _SDP_sendSSAResp(uTID, pData, uLen);
            break;

        default:
            DBG_ERROR( "pdu id!\n");
            bRetVal = FALSE_;
            break;
    }

    return bRetVal;
}

unsigned char _SDP_sendSSResp(unsigned short uTID, const unsigned char *pData, unsigned short uLen)
{
//    unsigned char pRspData[SDP_MAX_FRAME_SIZE];
	unsigned char* pRspData = L2capGetSendBuffer();
    unsigned char bRetVal = FALSE_;
    unsigned long au32UUID[12], uServiceRecordHandler;
    unsigned short uNumUUID, uRspDataLen, i;
    unsigned short uTServiceRecordCount, uCServiceRecordCount;
    SDP_SERVICE *apsServiceList[SDP_SERVICE_COUNT];

    uTServiceRecordCount = uCServiceRecordCount = 0;

    /*
     * Prepare the ServiceRecordHandleList:
     * Get the UUIDs from the ServiceSearchPattern and match them with
     * the services in the database.
     */

    /* Get the UUIDs from the ServiceSearchPattern. */
    uNumUUID = _SDP_getUUIDs(pData, au32UUID, NULL);
    /*
     * Get the ServiceRecordCount and store the ServiceRecordHandleList
     * in the Response data frame.
     */
    uCServiceRecordCount =
            _SDP_getServiceRecordList(au32UUID, uNumUUID, apsServiceList);
    uTServiceRecordCount = uCServiceRecordCount;
    uRspDataLen = SDP_SS_RSP_MIN_LEN + 4*uCServiceRecordCount;

    /*
     * Fill the Response frame.
     */

    /* Header: PDU ID, Transaction ID and PDU data length. */
    pRspData[0] = SDP_SSR_PDU;
    BT_WRITEBE16(&pRspData[1],uTID);
    BT_WRITEBE16(&pRspData[3],uRspDataLen);
    /* TotalServiceRecordCount and CurrentServiceRecordCount. */
    BT_WRITEBE16(&pRspData[5],uTServiceRecordCount);
    BT_WRITEBE16(&pRspData[7],uCServiceRecordCount);
    /* ServiceRecordHandleList: Already stored. */
    for (i = 0; i < uCServiceRecordCount; ++i)
    {
        if (NULL != apsServiceList[i])
        {
            uServiceRecordHandler = BT_READBE32(&apsServiceList[i]->pAttrs[0].pValue[1]);
			BT_WRITEBE32(&pRspData[9 + (4*i)],uServiceRecordHandler);
        }
    }
    /* ContinuationState (the last byte) */
    pRspData[SDP_HDR_LEN + uRspDataLen - 1] = 0x00;

    /*
     * Send the Response frame.
     */

    DBG_INFO( "SSResp.\n");

    //bRetVal = gpsSDPCB->L2CAPsendData(L2CAP_SDP_PSM,pRspData, SDP_HDR_LEN + uRspDataLen);
	L2capSendPsm (pRspData, SDP_HDR_LEN + uRspDataLen, L2CAP_PSM_SDP);
    return bRetVal;
}

unsigned char _SDP_sendSSAResp(unsigned short uTID, const unsigned char *pData, unsigned short uLen)
{
//    unsigned char pRspData[SDP_MAX_FRAME_SIZE];
	unsigned char* pRspData = L2capGetSendBuffer();
    unsigned char bRetVal;
    unsigned short uRspDataLen, uAttrByteCount, uAttrListsByteCount, uNumUUID,
            uNumServices, i, uReqOffset, uRspOffset;
    unsigned long au32UUID[12];
    SDP_SERVICE *apsServiceList[SDP_SERVICE_COUNT];

    uAttrListsByteCount = uAttrByteCount = uNumUUID = 0;
    uReqOffset = uRspOffset = 0;
    bRetVal = FALSE_;

    /*
     * Prepare the AttributeLists:
     */

    /*
     * Attribute list sequence:
     * Empty sequence
     */
    uAttrListsByteCount = 0x0002;
    pRspData[7] = SDP_DATA_T_DES|SDP_DATA_S_1B;
    pRspData[8] = 0x00;
    /*
     * Get the UUIDs from the ServiceSearchPattern and match them with
     * the services in the database.
     */
    uNumUUID = _SDP_getUUIDs(pData, au32UUID, &uReqOffset);
    uNumServices = _SDP_getServiceRecordList(au32UUID, uNumUUID, apsServiceList);
    /*
     * Fill the Attribute Lists with the attributes of each service (each
     * service is represented with a data element sequence containing its
     * attributes)
     */
    /*Set the offset to the Attribute ID List contained in the request*/
    uReqOffset += 2;

    for (i = 0; i < uNumServices; ++i)
    {
        if (NULL != apsServiceList[i])
        {
            /* Get the IDs and fill the AttributeList. */
            uAttrByteCount = _SDP_getAttrList(apsServiceList[i],
                    &pData[uReqOffset], &pRspData[9]);
        }
        uAttrListsByteCount += uAttrByteCount;
    }
    uRspDataLen = SDP_SSA_RSP_MIN_LEN + uAttrListsByteCount;
    pRspData[8] = uAttrListsByteCount - 2;
    /*
     * Fill the Response frame.
     */

    /* Header: PDU ID, Transaction ID and PDU data length. */
    pRspData[0] = SDP_SSAR_PDU;
    BT_WRITEBE16(&pRspData[1],uTID);
    BT_WRITEBE16(&pRspData[3],uRspDataLen);

    /* AttributeListsByteCount and AttributeList (already stored) */
    BT_WRITEBE16(&pRspData[5],uAttrListsByteCount);
    /* ContinuationSate. */
    pRspData[SDP_HDR_LEN + uRspDataLen - 1] = 0x00;

    /*
     * Send the Response frame.
     */

    DBG_INFO( "SSAResp.\n");
    //bRetVal = gpsSDPCB->L2CAPsendData(L2CAP_SDP_PSM,pRspData, SDP_HDR_LEN + uRspDataLen);
	L2capSendPsm (pRspData, SDP_HDR_LEN + uRspDataLen, L2CAP_PSM_SDP);
    return bRetVal;
}

unsigned char _SDP_sendSAResp(unsigned short uTID, const unsigned char *pData, unsigned short uLen)
{
//    unsigned char pRspData[SDP_MAX_FRAME_SIZE];
	unsigned char* pRspData = L2capGetSendBuffer();
    unsigned char bFound, bRetVal = FALSE_;
    unsigned short uRspDataLen, uAttrListByteCount, i;
    unsigned long uReqSrvHandle, uLocalSrvHandle;
    SDP_SERVICE *pService = NULL;

    /*
     * Prepare the AttributeList:
     * Get the IDs and ID ranges from the AttributeIDList and store the
     * requested attributes into the Response Data frame.
     */

    bFound = FALSE_;
    for (i = 0; (i < SDP_SERVICE_COUNT) && !bFound; ++i)
    {
        uReqSrvHandle = BT_READBE32(&pData[0]);
        uLocalSrvHandle = BT_READBE32(&gpsSDPCB->pService[i]->pAttrs[0].pValue[1]);

        if (uReqSrvHandle == uLocalSrvHandle)
        {
            bFound = TRUE_;
            pService = (SDP_SERVICE *) gpsSDPCB->pService[i];
        }
    }

    /* Get the IDs from the AttributeIDList and create the AttributeList. */
    uAttrListByteCount =  _SDP_getAttrList(pService, &pData[6], &pRspData[7]);
    uRspDataLen = SDP_SA_RSP_MIN_LEN + uAttrListByteCount;

    /*
     * Fill the Response frame.
     */

    /* Header: PDU ID, Transaction ID and PDU data length. */
    pRspData[0] = SDP_SAR_PDU;
    BT_WRITEBE16(&pRspData[1],uTID);
    BT_WRITEBE16(&pRspData[3],uRspDataLen);
    /* AttributeListByteCount and AttributeList (already stored) */
    BT_WRITEBE16(&pRspData[5],uAttrListByteCount);
    /* ContinuationSate. */
    pRspData[SDP_HDR_LEN + uRspDataLen - 1] = 0x00;

    /*
     * Send the Response frame.
     */

    DBG_INFO( "SAResp.\n");
//    bRetVal = gpsSDPCB->L2CAPsendData(L2CAP_SDP_PSM,pRspData,
//            SDP_HDR_LEN + uRspDataLen);
	L2capSendPsm (pRspData, SDP_HDR_LEN + uRspDataLen, L2CAP_PSM_SDP);
    return bRetVal;
}

unsigned short _SDP_getUUIDs(const unsigned char *pServiceSearchPattern,
        unsigned long *pUUID, unsigned short *pReqOffset)
{
    unsigned char bDataDescriptor;
    unsigned short uNumUUID, i, uDataLen, uOffset;

    uNumUUID = uDataLen = i = uOffset = 0;

    /*
     * NOTICE: It is a assumed that pUUID is at least a 12 element array.
     */

    if (NULL == pServiceSearchPattern)
    {
        DBG_ERROR("search err1\n");
        return 0;
    }

    /* Get the pattern length and the offset */
    switch (pServiceSearchPattern[0])
    {
        case SDP_DATA_T_DES|SDP_DATA_S_1B:
            uDataLen = pServiceSearchPattern[1];
            uOffset = 2;
            break;
        case SDP_DATA_T_DES|SDP_DATA_S_2B:
            uDataLen = BT_READBE16(&pServiceSearchPattern[1]);
            uOffset = 3;
            break;
        default:
            DBG_ERROR("search err2\n");
            return 0;
            break;
    }

    while (i < uDataLen)
    {
        bDataDescriptor = pServiceSearchPattern[uOffset + i];
	    switch(bDataDescriptor)
        {
            /* 16 bit UUID */
            case SDP_DATA_T_UUID|SDP_DATA_S_16:
                    pUUID[uNumUUID] = BT_READBE16(&pServiceSearchPattern[uOffset + i + 1]);
                    i+=3;
                    ++uNumUUID;
                    break;
            /* 32 bit UUID */
            case SDP_DATA_T_UUID|SDP_DATA_S_32:
                    pUUID[uNumUUID] = BT_READBE32(&pServiceSearchPattern[uOffset + i + 1]);
                    i+=5;
                    ++uNumUUID;
                    break;

            /* 128 bit UUID */
            case  SDP_DATA_T_UUID|SDP_DATA_S_128:
                    /* Not supported */
					// Check if its a bluetooth uuid
					if(IsBtSigUuid(&pServiceSearchPattern[uOffset + i + 1]))
					{	
						pUUID[uNumUUID] = BT_READBE32(&pServiceSearchPattern[uOffset + i + 1]); // 32 bit uuid from bytes 16 - 12
						i+=17;
						++uNumUUID;
					}
					else // Just skip it
					{
						i+=17;
					}
                    //return uNumUUID; // Old behaviour
                    break;

            default:
                    return uNumUUID;
                    break;
        }
    }
    
    /* Return the Offset to to next parameter in the request */
    if(NULL != pReqOffset)
    {
        *pReqOffset = uOffset + uDataLen;
    }

    return uNumUUID;
}

unsigned short _SDP_getServiceRecordList(const unsigned long *pUUID, unsigned short uLen,
        SDP_SERVICE *ppsServiceList[])
{
    unsigned short i;
    unsigned short uNumServices = 0;
    unsigned char bDiscarded = FALSE_;

    /* Do a sanity check on the input variables */
    if (NULL == pUUID    ||
        NULL == ppsServiceList)
    {
        DBG_ERROR("param err\n");
        return FALSE_;
    }

    #ifdef SDP_SERVICE_RFCOMM_ENABLE
    ppsServiceList[0] = NULL;
    bDiscarded = FALSE_;
    /* Discard the services according to the UUID list */
    for(i = 0; (i < uLen) && !bDiscarded; ++i)
    {
        if (pUUID[i] != 0x00000003 &&
            pUUID[i] != 0x00000100 &&
            pUUID[i] != 0x00001101 &&
            pUUID[i] != 0x00001002 )
        {
            bDiscarded = TRUE_;
        }
    }
    if ((uLen > 0) && !bDiscarded)
    {
        /* Populate the ServiceList with the RFCOMM */
        ppsServiceList[0] = (SDP_SERVICE *) gpsSDPCB->pService[0];
        ++uNumServices;
    }
    #endif

    return uNumServices;
}

unsigned short _SDP_getAttrList(const SDP_SERVICE *pService, const unsigned char *pAttrIDList,
        unsigned char *pAttrList)
{
    unsigned short i, j = 0, k, uAttrListByteCount, uInOffset = 0, uOutOffset, uLen = 0;
    unsigned short uID, uIDRangeHigh, uIDRangeLow;
    unsigned char bDataDescriptor;
    unsigned char bFound = FALSE_;

    /* Do a sanity check on the input variables */
    if (NULL == pAttrIDList ||
        NULL == pAttrList)
    {
        DBG_ERROR( "param err\n");
        return FALSE_;
    }


    /* The first 2 bytes will contain a data element sequence descriptor */
    uOutOffset = 2;
    uAttrListByteCount = 0;

    /* In case of a NULL service, return an empty list */
    if (NULL == pService)
    {
        /* Fill the first 2 bytes (data element sequence descriptor) */
        pAttrList[0] =  SDP_DATA_T_DES|SDP_DATA_S_1B;
        pAttrList[1] =  uAttrListByteCount;

        return uAttrListByteCount + uOutOffset;
    }

    /* Get the list length and the offset. */
    bDataDescriptor = pAttrIDList[0];
    switch (bDataDescriptor)
    {
        case SDP_DATA_T_DES|SDP_DATA_S_1B:
            uLen = pAttrIDList[1];
            uInOffset = 2;
            break;
        case SDP_DATA_T_DES|SDP_DATA_S_2B:
            uLen = BT_READBE16(&pAttrIDList[1]);
            uInOffset = 3;
            break;
        default:
            break;
    }
    i = 0;
    while(i < uLen)
    {
        /*
         * Depending on the descriptor the data will be stored as a single ID
         * or as an ID range.
         */
        bDataDescriptor = pAttrIDList[uInOffset + i];
        switch(bDataDescriptor)
        {
            /* 16 bit ID (single ID) */
            case SDP_DATA_T_UINT|SDP_DATA_S_16:
                uID = BT_READBE16(&pAttrIDList[uInOffset + j + 1]);
                bFound = FALSE_;
                /* Search for that AttrID in the service record */
                for (j = 0; (j < pService->uNumAttrs) && !bFound; ++j)
                {
                    if (pService->pAttrs[j].uID == uID)
                    {
                        /*
                         * Fill the AttrList
                         */

                        /* Store the attribute ID (as an unsigned short) */
                        pAttrList[uOutOffset + uAttrListByteCount] =
                                SDP_DATA_T_UINT|SDP_DATA_S_16;
                        BT_WRITEBE16(&pAttrList[uOutOffset + uAttrListByteCount + 1],pService->pAttrs[j].uID);
                        uAttrListByteCount += 3;
                        /* Store the attribute value */
                        for(k = 0; k < pService->pAttrs[j].uValueLen; ++k)
                        {
                            pAttrList[uOutOffset + uAttrListByteCount + k] =
                                    pService->pAttrs[j].pValue[k];
                        }
                        uAttrListByteCount += k;
                        bFound = TRUE_;
                    }
                }
                i+=3;
                break;
            /* 32 bit ID (range of IDs) */
            case SDP_DATA_T_UINT|SDP_DATA_S_32:
                uIDRangeLow = BT_READBE16(&pAttrIDList[uInOffset + i + 1]);
                uIDRangeHigh = BT_READBE16(&pAttrIDList[uInOffset + i + 1 + 2]);
                /* Search for the AttrID in the service record */
                for (j = 0; j < pService->uNumAttrs; ++j)
                {
                    uID = pService->pAttrs[j].uID;
                    if ((uID >= uIDRangeLow) &&
                        (uID <= uIDRangeHigh))
                    {
                        /*
                         * Fill the AttrList
                         */

                        /* Store the attribute ID (as an unsigned short) */
                        pAttrList[uOutOffset + uAttrListByteCount] =
                                SDP_DATA_T_UINT|SDP_DATA_S_16;
                        BT_WRITEBE16(&pAttrList[uOutOffset + uAttrListByteCount + 1],pService->pAttrs[j].uID);
                        uAttrListByteCount += 3;
                        /* Store the attribute value */
                        for(k = 0; k < pService->pAttrs[j].uValueLen; ++k)
                        {
                            pAttrList[uOutOffset + uAttrListByteCount + k] =
                                    pService->pAttrs[j].pValue[k];
                        }
                        uAttrListByteCount += k;
                    }
                }
                i+=5;
                break;
            /* Error, the element is not an ID */
            default:
                return FALSE_;
                break;
        }
    }

    /* Fill the first 2 bytes (data element sequence descriptor) */
    pAttrList[0] =  SDP_DATA_T_DES|SDP_DATA_S_1B;
    pAttrList[1] =  uAttrListByteCount;

    return uAttrListByteCount + uOutOffset;
}
