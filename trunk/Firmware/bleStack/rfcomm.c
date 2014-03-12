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

#include <stdlib.h>
#include "l2cap + le.h"
#include "rfcomm.h"
#include "rfcomm_fcs.h"
#include "bt config.h"
#include "debug.h"

/*
 * RFCOMM definitions
 */

// DEBUG
#if DEBUG_RFCOMM
	static const char* file = "rfcomm";
	#define ASSERT(X)			DBG_assert(X, file, __LINE__)
    #define DBG_INFO(X, ...) 	DBG_info(file,X, ##__VA_ARGS__);
    #define DBG_DUMP(X, Y)		DBG_dump(X, Y);
    #define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
#else
    #define DBG_INFO(X, ...)
    #define DBG_DUMP(X, Y)
	#if DBG_ALL_ERRORS			// Leave only errors and assertions on
		static const char* file = "rfcomm";
    	#define DBG_ERROR(X, ...)	DBG_error(file,X, ##__VA_ARGS__);
		#define ASSERT(X)			DBG_assert(X, file, __LINE__)
	#else
    	#define DBG_ERROR(X, ...)
		#define ASSERT(X)
	#endif
#endif

/*
 * RFCOMM variables
 */

static RFCOMM_CONTROL_BLOCK	gRFCOMMCB;
static RFCOMM_CONTROL_BLOCK *gpsRFCOMMCB = NULL;
unsigned char(*RfCommDataCb)(const unsigned char *pData, unsigned short uLen) = NULL;

/*
 * RFCOMM public functions implementation
 */

unsigned char RFCOMM_create()
{
    unsigned short i;

	// KL: Created one instance only
    gpsRFCOMMCB = &gRFCOMMCB;

    /* Initialise the channels */
    for (i = 0; i < RFCOMM_NUM_CHANNELS; ++i)
    {
        gpsRFCOMMCB->asChannel[i].bDLC = i;
        gpsRFCOMMCB->asChannel[i].bEstablished = FALSE_;
        gpsRFCOMMCB->asChannel[i].bLocalCr = 0;
        gpsRFCOMMCB->asChannel[i].bRemoteCr = 0;
    }

    /* Set the role as responder */
    gpsRFCOMMCB->bRole = RFCOMM_ROLE_RESPONDER;
    gpsRFCOMMCB->isInitialised = TRUE_;

	// Sends rfcomm data from controller via l2cap
	L2capInstallRfcomm(RfcommPutData); 

    return TRUE_;
}

/*
 * RFCOMM API functions implementation
 */
void RfCommInstallDataCB (unsigned char(*CallBack)(const unsigned char *pData, unsigned short uLen))
{
	RfCommDataCb = CallBack;
}

unsigned char RfcommPutData(const unsigned char *pData, unsigned short uLen)
{
    unsigned char bCtrl, bFCS, bMsgType, bMsgLen;
    unsigned char uOffset, uMsgOffset, bChNumber;
    unsigned short uFrameHdrLen, uFrameInfLen;
    unsigned char bRetVal= FALSE_, bHasCrField = FALSE_;
    RFCOMM_CHANNEL *psChannel = NULL;

    DBG_INFO ("->");
    DBG_DUMP(pData, uLen);

    /*
     * Parse the header
     */
    ASSERT(uLen >= RFCOMM_HDR_LEN_1B);

    /* Get the Channel number and Control field */
    bChNumber = pData[0] >> 3;
    bCtrl = pData[1];
    /* Check the channel */
    psChannel = _RFCOMM_getChannel(bChNumber);
    if(NULL == psChannel)
    {
        return FALSE_;
    }

    /* Check if the len field has 1 or 2 octets */
    if ((pData[2] & RFCOMM_MASK_LI_1B))
    {
        uFrameHdrLen = RFCOMM_HDR_LEN_1B;
        uFrameInfLen = pData[2] >> 1;
    }
    else
    {
        uFrameHdrLen = RFCOMM_HDR_LEN_2B;
        uFrameInfLen = BT_READBE16(&pData[2]) >> 1;
    }
    uOffset = uFrameHdrLen;

    /*
     * Parse the payload according to the frame type and the information
     */
    ASSERT(uLen >= (uFrameHdrLen + uFrameInfLen + 1));

    /* Information frame */
    if (bCtrl == RFCOMM_UIH_FRAME ||
        bCtrl == (RFCOMM_UIH_FRAME|RFCOMM_PF_BIT))
    {
        /* Handle the credit field (if any) */
        if (bCtrl & RFCOMM_PF_BIT)
        {
            bHasCrField = TRUE_;
            bFCS = pData[uFrameHdrLen + uFrameInfLen + 1];
        }
        else
        {
            bHasCrField = FALSE_;
            bFCS = pData[uFrameHdrLen + uFrameInfLen];
        }

        /* FCS check */
        if (!RFCOMM_FCS_CheckCRC(pData, 2, bFCS))
        {
            DBG_ERROR("crc fail\n");
            return FALSE_;
        }

        /* If the channel is in DM it cannot handle any information */
        if (!psChannel->bEstablished)
        {
            DBG_ERROR("close ch\n");
            return FALSE_;
        }

        /*
         * Handle the information field depending on the channel:
         * Multiplexor (DLCI = 0): Message.
         * Data channel (DLCI >= 1): User data.
         */
        if (bChNumber == RFCOMM_CH_MUX)
        {
            bMsgType = pData[uOffset + 0];
            bMsgLen = (pData[uOffset + 1] >> 1);
            uMsgOffset = uOffset + 2;
            /* Switch according to the message type */
            switch (bMsgType)
            {
                case RFCOMM_PN_CMD:
                    bRetVal = _RFCOMM_handlePN(&pData[uMsgOffset], bMsgLen);
                    break;

                case RFCOMM_TEST_CMD:
                    bRetVal = _RFCOMM_handleTEST(&pData[uMsgOffset], bMsgLen);
                    break;

                case RFCOMM_MSC_CMD:
                    bRetVal = _RFCOMM_handleMSC(&pData[uMsgOffset], bMsgLen);
                    break;

                case RFCOMM_RPN_CMD:
                    bRetVal = _RFCOMM_handleRPN(&pData[uMsgOffset], bMsgLen);
                    break;

                case RFCOMM_RLS_CMD:
                    bRetVal = _RFCOMM_handleRLS(&pData[uMsgOffset], bMsgLen);
                    break;

                case RFCOMM_FCON_CMD:
                case RFCOMM_FCOFF_CMD:
                default:
                    DBG_INFO("%x unsupp\n", bMsgType);
                    bRetVal = FALSE_;
                    break;
            }
        }
        else
        {
            /* Handle the credit field (if any) */
            if(bHasCrField)
            {
                /* Add the credits on the remote end */
                psChannel->bRemoteCr += (unsigned char) pData[uOffset + 0];
                ++uOffset;
            }
            /* Decrease the credit counter if the frame has user data */
            if (uFrameInfLen > 0)
            {
                psChannel->bLocalCr--;

				// User installed call back
				if(RfCommDataCb)
                	RfCommDataCb(&pData[uOffset], uFrameInfLen);

                /* In case of running low of credits, allocate some more */
                if (psChannel->bLocalCr < 0x08)
                {
                    bRetVal = _RFCOMM_sendUIHCr(bChNumber, 0x10);
                    if (bRetVal)
                    {
                        psChannel->bLocalCr += 0x010;
                    }
                }
            }
        }
        return bRetVal;
    }
    else
    {
        /* FCS check */
        bFCS = pData[uFrameHdrLen + uFrameInfLen];
        if (!RFCOMM_FCS_CheckCRC(pData, uFrameHdrLen, bFCS))
        {
            DBG_ERROR("crc fail\n");
            return FALSE_;
        }
        /* Handle the frame according to the control field */
        switch (bCtrl)
        {
            case RFCOMM_SABM_FRAME|RFCOMM_PF_BIT:
                /* Check if the channel is established before sending the UA */
                if (psChannel->bEstablished)
                {
                    DBG_ERROR("re-open?!\n");
                    return FALSE_;
                }
                /* Send the UA and establish the channel */
                bRetVal = _RFCOMM_sendUA(bChNumber);
                if (bRetVal)
                {
					DBG_INFO("open:%u\n",bChNumber);
                    psChannel->bEstablished = TRUE_;
					psChannel->mtu = 0; // Will cach it on call to get mtu (KL)
//KL: Hook
BTEvent(BT_EVENT_COMM_OPEN);
                }
                break;

            case RFCOMM_UA_FRAME|RFCOMM_PF_BIT:
                break;

            case RFCOMM_DM_FRAME:
            case RFCOMM_DM_FRAME|RFCOMM_PF_BIT:
                break;

            case RFCOMM_DISC_FRAME|RFCOMM_PF_BIT:
                /* Send a UA frame and disconnect the channel */
                bRetVal = _RFCOMM_sendUA(bChNumber);
                if (bRetVal)
                {
                    psChannel->bEstablished = FALSE_;
					psChannel->mtu = 0;
                    psChannel->bLocalCr = 0;
                    psChannel->bRemoteCr = 0;
//KL: Hook
BTEvent(BT_EVENT_COMM_CLOSED);
                }
                break;

            default:
                /*
                 * Frames to be discarded:
                 * SABM, UA and DISC without the P/F bit.
                 */
                break;
        }
    }

	return TRUE_;
}

// Get max payload len - cached on connect
unsigned short RfcommMaxPayload(void)
{
	// Get channel
	RFCOMM_CHANNEL *psChannel = _RFCOMM_getChannel(RFCOMM_CH_DATA);
	if(psChannel == NULL) return 0;
	if(psChannel->bEstablished != TRUE_) return 0;
	// Cach the value for this channel
	if(psChannel->mtu == 0) 
	{
		// Max payload is the l2cap max minus the rfcomm headers 
		psChannel->mtu = (L2capGetMtuForPsm(L2CAP_PSM_RFCOMM)-RFCOMM_HDR_LEN_2B-1);
	}
	return (psChannel->mtu);
}

unsigned char RfCommSendPacket(const unsigned char *pData, unsigned short uLen)
{
    unsigned char bRetVal = FALSE_;
    unsigned char bChNum = RFCOMM_CH_DATA;
    RFCOMM_CHANNEL *psChannel = NULL;

    /* Check if the DATA CHANNEL is active */
    psChannel = _RFCOMM_getChannel(bChNum);
    if(NULL == psChannel ||
       psChannel->bRemoteCr == 0)
    {
        return FALSE_;
    }

    /* Send a UIH frame containing the data */
    bRetVal = _RFCOMM_sendUIH(RFCOMM_CH_DATA, pData, uLen);
    if (bRetVal)
    {
        psChannel->bRemoteCr--;
    }

    return bRetVal;
}

unsigned char RfCommDisconnect(unsigned char bChannel)
{
    //unsigned char aData[RFCOMM_DISC_LEN];
	unsigned char* aData;
    unsigned char bRetVal = FALSE_;
    RFCOMM_CHANNEL *psChannel = NULL;

    psChannel = _RFCOMM_getChannel(bChannel);
    if(NULL == psChannel)
    {
        return FALSE_;
    }
    /* Prepare the frame */
	aData = L2capGetSendBuffer();
    /* Address */
    aData[0] = _RFCOMM_getAddress(bChannel, RFCOMM_CMD);
    /* Control */
    aData[1] = RFCOMM_DISC_FRAME|RFCOMM_PF_BIT;
    /* InfoLength */
    aData[2] = (0x00 << 1) | 0x01;
    /* FCS */
    aData[3] = RFCOMM_FCS_CalcCRC(aData, RFCOMM_HDR_LEN_1B);

    /* Send the frame */
    //bRetVal = gpsRFCOMMCB->L2CAPsendData(L2CAP_RFCOMM_PSM,
    //        aData, RFCOMM_DISC_LEN);
	bRetVal = L2capSendPsm(aData, RFCOMM_DISC_LEN, L2CAP_PSM_RFCOMM);

    return bRetVal;
}

/*
 * RFCOMM private functions implementation
 */

RFCOMM_CHANNEL* _RFCOMM_getChannel(unsigned char uChNumber)
{
    RFCOMM_CHANNEL *pRetChannel = NULL;
    ASSERT(uChNumber < RFCOMM_NUM_CHANNELS);
    /* Get a handle to the channel */
    pRetChannel = (RFCOMM_CHANNEL *) &(gpsRFCOMMCB->asChannel[uChNumber]);

    return pRetChannel;
}

unsigned char _RFCOMM_getAddress(unsigned char bChNumber, unsigned char bType)
{
    unsigned char bRole, bCR = 0x00;

    ASSERT(NULL != gpsRFCOMMCB);
    bRole = gpsRFCOMMCB->bRole;

    /* Set the Command/Response bit according to the role */
    if (((bType == RFCOMM_DATA) && (bRole == RFCOMM_ROLE_INITIATIOR))||
        ((bType == RFCOMM_CMD) && (bRole == RFCOMM_ROLE_INITIATIOR)) ||
        ((bType == RFCOMM_RSP) && (bRole == RFCOMM_ROLE_RESPONDER)))
    {
        bCR = 0x01;
    }
    return (bChNumber << 3) + (bRole << 2) + ((bCR << 1) | 0x01);
}

unsigned char _RFCOMM_sendUA(unsigned char bChNum)
{
    //unsigned char aData[RFCOMM_UA_LEN];
	unsigned char* aData;
    unsigned char bRetVal = FALSE_;
    RFCOMM_CHANNEL *psChannel = NULL;

    psChannel = _RFCOMM_getChannel(bChNum);
    if(NULL == psChannel)
    {
        return FALSE_;
    }
    /* Prepare the frame */
	aData = L2capGetSendBuffer();
    /* Address */
    aData[0] = _RFCOMM_getAddress(bChNum, RFCOMM_RSP);
    /* Control */
    aData[1] = RFCOMM_UA_FRAME|RFCOMM_PF_BIT;
    /* InfoLength */
    aData[2] = (0x00 << 1) | 0x01;
    /* FCS */
    aData[3] = RFCOMM_FCS_CalcCRC(aData, RFCOMM_HDR_LEN_1B);

    /* Send the frame */
    //bRetVal = gpsRFCOMMCB->L2CAPsendData(L2CAP_RFCOMM_PSM,
    //        aData, RFCOMM_UA_LEN);
	bRetVal = L2capSendPsm(aData, RFCOMM_UA_LEN, L2CAP_PSM_RFCOMM);

    return bRetVal;
}

unsigned char _RFCOMM_sendUIH(unsigned char bChNum, const unsigned char *pData, unsigned short uLen)
{
    //unsigned char aFrame[RFCOMM_UIH_LEN + uLen + 1];
	unsigned char* aFrame;
    unsigned short i;
    unsigned char uOffset = 0;
    unsigned char bRetVal;
    RFCOMM_CHANNEL *psChannel = NULL;

    /* Check the channel */
    psChannel = _RFCOMM_getChannel(bChNum);
    if(NULL == psChannel)
    {
        return FALSE_;
    }

    /* Fill the header */
	aFrame = L2capGetSendBuffer();
    aFrame[0] = _RFCOMM_getAddress(bChNum, RFCOMM_DATA);
    aFrame[1] = RFCOMM_UIH_FRAME;
    /* The lenght field will be one or two octet long depending on uLen */
    if (uLen > 127)
    {
        aFrame[2] = (uLen << 1) & 0x00FE;
        aFrame[3] = (uLen >> 7) & 0x007F;
        uOffset = 4;
    }
    else
    {
        aFrame[2] = (uLen << 1) | 0x01;
        uOffset = 3;
    }

    /* Copy the payload */
    for (i = 0; i < uLen; ++i)
    {
        aFrame[uOffset + i] = pData[i];
    }
    
    /* Calculate the FCS */
    aFrame[uOffset + i] = RFCOMM_FCS_CalcCRC(aFrame, 2);

    /* Send the frame */
    //bRetVal = gpsRFCOMMCB->L2CAPsendData(L2CAP_RFCOMM_PSM,
    //        aFrame, RFCOMM_UIH_LEN + uLen);
	uLen += RFCOMM_UIH_LEN + ((uOffset==4)?1:0);
	bRetVal = L2capSendPsm(aFrame, uLen, L2CAP_PSM_RFCOMM);


    return bRetVal;
}

unsigned char _RFCOMM_sendUIHCr(unsigned char bChNum, unsigned char uNumCr)
{
    //unsigned char aFrame[RFCOMM_UIH_CR_LEN];
	unsigned char* aFrame = L2capGetSendBuffer();
    unsigned char bRetVal = FALSE_;

    /* Send a UIH frame with credit field and no user data */
    aFrame[0] = _RFCOMM_getAddress(bChNum, RFCOMM_DATA);
    aFrame[1] = RFCOMM_UIH_FRAME | RFCOMM_PF_BIT;
    aFrame[2] = (0x00 << 1) | 0x01;
    aFrame[3] = uNumCr;
    aFrame[4] = RFCOMM_FCS_CalcCRC(aFrame, 2);

//    bRetVal = gpsRFCOMMCB->L2CAPsendData(L2CAP_RFCOMM_PSM,
//            aFrame, RFCOMM_UIH_CR_LEN);
	bRetVal = L2capSendPsm(aFrame, RFCOMM_UIH_CR_LEN, L2CAP_PSM_RFCOMM);

    return bRetVal;
}

unsigned char _RFCOMM_handlePN(const unsigned char *pMsgData, unsigned char uMsgLen)
{
    unsigned char uChNum;
    unsigned short uMTU, ourMTU;
    unsigned char aPNRsp[RFCOMM_MSGHDR_LEN + RFCOMM_PNMSG_LEN];
    unsigned char bRetVal = FALSE_;
    RFCOMM_CHANNEL *psChannel = NULL;

    /* Check the len */
    ASSERT(uMsgLen == RFCOMM_PNMSG_LEN);

    /* Get the Channel Number to configure */
    uChNum = pMsgData[0] >> 1;
    uMTU = BT_READLE16(&pMsgData[4]);
    /* Check if we have that channel */
    psChannel = _RFCOMM_getChannel(uChNum);
    if(NULL == psChannel)
    {
        return FALSE_;
    }
    /*
     * Fill the response PN frame header
     * MsgType = PN Response
     * MsgLength = RFCOMM_PNMSG_LEN (+E/A bit)
     */
    aPNRsp[0] = RFCOMM_PN_RSP;   
    aPNRsp[1] = (RFCOMM_PNMSG_LEN << 1) | 0x01;
    /*
     * Fill the default values
     * DLCI = DLCI to configure
     * I_CL = 0xE0 (Enable credit flow control) and 0x00 (Use UIH frames)
     * P = 0x00 (No priority, the lowest one)
     * T = 0x00 (T1 not negotiable in RFCOMM)
     * N = RFCOMM_MTU (Maximum frame size)
     * NA = 0x00 (N2 is always 0 in RFCOMM)
     * K = 0x07 (Starting number of credits)
     */
    aPNRsp[2] = (uChNum << 1) | gpsRFCOMMCB->bRole;
    aPNRsp[3] = (0x0E << 4) | 0x00;
    aPNRsp[4] = 0x00;
    aPNRsp[5] = 0x00;
	// This will return the packet size restriction (smaller of their mtu OR our buffer size)
	ourMTU = (L2capGetMtuForPsm(L2CAP_PSM_RFCOMM)-RFCOMM_HDR_LEN_2B-1); // Extra byte for >128 length
    BT_WRITELE16(&aPNRsp[6],ourMTU);
    aPNRsp[8] = 0x00;
    aPNRsp[9] = 0x07;
    /* Check the CL and the I bits */
    if (pMsgData[1] != ((0x0F << 4)|(0x00)))
    {
        DBG_ERROR("req no cr flow!\n");
        return FALSE_;
    }

    /* Set both the Remote and the Local Credits */
    psChannel->bLocalCr = 0x07;
    psChannel->bRemoteCr = pMsgData[7];
    
    /* 
     * Send the PN response
     */

    /* Same priority */
    aPNRsp[4] = pMsgData[2];
    /* Configure the minimum MTU */
    if (uMTU < ourMTU)
    {
        BT_WRITELE16(&aPNRsp[6],uMTU);
    }

	DBG_INFO("rmtu:%u, lmtu:%u\n",uMTU,ourMTU);    

    bRetVal = _RFCOMM_sendUIH(0x00, aPNRsp,
            RFCOMM_MSGHDR_LEN + RFCOMM_PNMSG_LEN);

    return bRetVal;
}

unsigned char _RFCOMM_handleMSC(const unsigned char *pMsgData, unsigned char uMsgLen)
{
    unsigned char bRetVal = FALSE_;
    unsigned short i;
    unsigned char aResponse[RFCOMM_MSGHDR_LEN + RFCOMM_MSCMSG_LEN];
    unsigned char aRequest[RFCOMM_MSGHDR_LEN + RFCOMM_MSCMSG_LEN];

    ASSERT(NULL != gpsRFCOMMCB);
    /* Check the message length */
    ASSERT(uMsgLen <= RFCOMM_MSCMSG_LEN);

    /* Fill the header */
    aResponse[0] = RFCOMM_MSC_RSP;
    aResponse[1] = (uMsgLen << 1) | 0x01;
    /* Get a copy of the received values */
    for(i = 0; i < uMsgLen; ++i)
    {
        aResponse[2 + i] = pMsgData[i];
    }
    /* Send the frame */
    bRetVal = _RFCOMM_sendUIH(0x00, aResponse, RFCOMM_MSGHDR_LEN + uMsgLen);
    if (!bRetVal)
    {
        return bRetVal;
    }

    /* Send a request after responding the command */
    /* Fill the header */
    aRequest[0] = RFCOMM_MSC_CMD;
    aRequest[1] = (RFCOMM_MSCMSG_LEN << 1) | 0x01;
    /* Fill the command */
    aRequest[2] = pMsgData[0];  /* Set the same DLCi as the initial command */
    aRequest[3] = 0x8D;         /* Set the DV, RTR, RTC and EA bits */
    aRequest[4] = 0x00;         /* No break signal */
    /* Send the frame */
    bRetVal = _RFCOMM_sendUIH(0x00, aRequest,
            RFCOMM_MSGHDR_LEN + RFCOMM_MSCMSG_LEN);
    return bRetVal;
}

unsigned char _RFCOMM_handleRPN(const unsigned char *pMsgData, unsigned char uMsgLen)
{
    unsigned char bRetVal = FALSE_;
    unsigned char uDLCI;
    unsigned char aResponse[RFCOMM_MSGHDR_LEN + RFCOMM_RPNMSG_LEN];

    /* Check the message length */
    ASSERT(uMsgLen == RFCOMM_RPNMSG_LEN);

    /* Get the DLC to configure */
    uDLCI = pMsgData[0] >> 2;

    /* Fill the header */
    aResponse[0] = RFCOMM_RPN_RSP;
    aResponse[1] = (RFCOMM_RPNMSG_LEN << 1) | 0x01;
    /*
     * Fill the default values
     * DLCI = DLCI to configure
     * B = 0x03 (9600bps)
     * D_S_P_PT = PT=0 (odd parity),
     *            P=0 (No parity),
     *            S=0 (1 stop) and D=0x3 (8 bits)
     * FLC = 0x00 (No flow control)
     * XON = 0x00 (XON character)
     * XOFF = 0x00 (XOFF character)
     * PM = 0xFFFF (All new parameters accepted)
     */
    /* DLCI */
    aResponse[2] = (uDLCI << 2) | (0x01 << 1) | 0x01;
    /* Baudrate (0x03) = 9600 */
    aResponse[3] = 0x03;
    /* Data (0x03), stop (0), parity (0) and parity type (0) bits */
    aResponse[4] = (0 << 4) | (0 << 3) | (0 << 2) | 0x03;
    /* Flow control (0x00) = No FLC */
    aResponse[5] = 0x00;
    /* XON (0x00) and XOFF (0x00) characters */
    aResponse[6] = 0x00;
    aResponse[7] = 0x00;
    /* Parameter mask (0xFFFF) = Accept all the changes */
    aResponse[8] = 0xFF;
    aResponse[9] = 0xFF;

    bRetVal = _RFCOMM_sendUIH(0x00, aResponse,
            RFCOMM_MSGHDR_LEN + RFCOMM_RPNMSG_LEN);

    return bRetVal;
}

unsigned char _RFCOMM_handleRLS(const unsigned char *pMsgData, unsigned char uMsgLen)
{
    unsigned char uDLCI;
    unsigned char bErrFlags, aRLSRsp[RFCOMM_MSGHDR_LEN + RFCOMM_RLSMSG_LEN];
    unsigned char bRetVal = FALSE_;

    /* Check the message length */
    ASSERT(uMsgLen == RFCOMM_RLSMSG_LEN);
    
    uDLCI = pMsgData[0];

    /* Fill the header */
    aRLSRsp[0] = RFCOMM_RLS_RSP;
    aRLSRsp[1] = (RFCOMM_RLSMSG_LEN << 1) | 0x01;
    /* DLC */
    aRLSRsp[2] = uDLCI;
    /* Line status */
    aRLSRsp[3] = pMsgData[1];
    /* If an error has occurred */
    if (pMsgData[1] & RFCOMM_MASK_RLS_ERROR)
    {
        bErrFlags = pMsgData[1] >> 1;
        if (bErrFlags & RFCOMM_MASK_RLS_OVERRUN)
        {
            /* TODO: Error handling */
        }
        if (bErrFlags & RFCOMM_MASK_RLS_PARITY)
        {
            /* TODO: Error handling */
        }
        if (bErrFlags & RFCOMM_MASK_RLS_FRAMING)
        {
            /* TODO: Error handling */
        }
    }

    bRetVal = _RFCOMM_sendUIH(RFCOMM_CH_MUX,
            aRLSRsp, RFCOMM_MSGHDR_LEN + RFCOMM_RLSMSG_LEN);

    return bRetVal;
}

unsigned char _RFCOMM_handleTEST(const unsigned char *pMsgData, unsigned char uMsgLen)
{
    unsigned char aTESTRsp[RFCOMM_MSGHDR_LEN + uMsgLen];
    unsigned short i;
    unsigned char bRetVal = FALSE_;

    /* Fill the header */
    aTESTRsp[0] = RFCOMM_TEST_RSP;
    aTESTRsp[1] = (uMsgLen << 1) | 0x01;
    /* Fill the payload */
    for (i = 0; i < uMsgLen; ++i)
    {
        aTESTRsp[2 + i] = pMsgData[i];
    }

    bRetVal = _RFCOMM_sendUIH(RFCOMM_CH_MUX,
            aTESTRsp, RFCOMM_MSGHDR_LEN + uMsgLen);

    return bRetVal;
}

