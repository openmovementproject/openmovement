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

// Simple interface to Si44 BAX receiver
// Karim Ladha, 2013-2014

// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Settings.h"
#include "Peripherals/Si443x.h"
#include "BaxRx.h"
#include "Data.h"
#include "aes.h"

// Debug setting
//#define DEBUG_ON
#include "Debug.h"

// Globals
BaxKey_t baxKeys[MAX_LOCAL_BAX_KEYS];
static void(*EncryptionPacketCb)(BaxPacket_t* pkt) = NULL;

void BaxRxInit(void)
{
	// Wipe key memory
	memset(baxKeys,0,sizeof(baxKeys));
	// Clear callback
	EncryptionPacketCb = NULL;
	// Init radio
	Si44RadioInit();
	if(Si44RadioState != SI44_INITIALISED)
	{
		status.radio_state = ERROR;
		return;
	}
	status.radio_state = INITIALISED;

	// Set header bytes
	Si44RadioSetHeader((unsigned char*)"\0\0\0B",1);// Only check MSB for 'B'
	// Start receiving
	Si44RadioRx(0);
	status.radio_state = ACTIVE;
}

void BaxRxTasks(void)
{
	// On Hz upto every 60 sec
	if(status.radio_state == OFF || status.radio_state == ERROR) return; // Call init first

	// Check radio ok
	if(!Si44RadioCheckOk())
	{
		DBG_ERROR("BaxRx.c: Si44 check failed. %u, %u\r\n",RPINR31bits.SCK4R,RPINR31bits.SDI4R);
		BaxRxInit();
		if(status.radio_state != ACTIVE)
			status.radio_state = ERROR;
	}
			
	// Check interrupt pin, if low, set flag
	if(!Si44_nIRQ) Si44_IRQ_FLAG = 1;

}

unsigned short BaxAddKeys(BaxKey_t* keys, unsigned short num)
{
	unsigned short i, added;
	
	for(;num>0;num--, keys++)
	{
		for(i=0;i<MAX_LOCAL_BAX_KEYS;i++)
		{
			// Erase all duplicate entries for address
			if(baxKeys[i].address == keys->address)
			{
				DBG_INFO("\r\nOLD KEY DEL.");
				memset(&baxKeys[i],0,sizeof(BaxKey_t));
			}
		}
		for(i=0;i<MAX_LOCAL_BAX_KEYS;i++)
		{
			// Add new entries in first empty slot
			if(baxKeys[i].address == 0ul)
			{	
				memcpy(&baxKeys[i],keys,sizeof(BaxKey_t));
				DBG_INFO("\r\nNEW KEY ADD.");
				added++;
				break;
			}
		}
		if(i >= MAX_LOCAL_BAX_KEYS)
		{
			// Dump newest in list if no space
			DBG_INFO("New key replaces old, no spce.");
			memmove(&baxKeys[0],&baxKeys[1],sizeof(BaxKey_t));
			memcpy(&baxKeys[MAX_LOCAL_BAX_KEYS-1],keys,sizeof(BaxKey_t));
		}
	}
	return added;
}

unsigned char BaxDecodePkt(BaxPacket_t* pkt)
{
	unsigned short i;
	for(i=0;i<MAX_LOCAL_BAX_KEYS;i++)
	{
		if(baxKeys[i].address == pkt->address)
		{
			unsigned char temp[16];
			aes_decrypt_128(&pkt->b[6],&pkt->b[6],&baxKeys[i].b[4],temp);
			return TRUE;
		}
	}
	return FALSE;
}

void BaxReadPacket(void)
{
	if(Si44RadioState == SI44_RXED)
	{
		BaxPacket_t pkt;															// Temp var
		unsigned char len = 0;														// Length
		len = Si44RadioReadPkt(&pkt.b[0], &pkt.b[5], sizeof(BaxPacket_t)-5,&pkt.b[4]);// Read
		Si44RadioRx(0);																// Re-enable reciever
		// Check mask before accepting
		unsigned short temp = LE_READ_16(&pkt.b[2]); // Read top word & verify mask
		if((settings.radioSubnetMask&temp)==(settings.radioSubnetMask&settings.radioSubnet))
		{	
			// We received a packet in our sub network
			SI44_LED = !SI44_LED;	// Flash led
			// Forward encryption packets to external handler
			if(pkt.pktType == AES_KEY_PKT_TYPE)
			{
				DBG_INFO("\r\nBAX LINK PKT.");
				if(EncryptionPacketCb)EncryptionPacketCb(&pkt);
			}
			else if (BaxDecodePkt(&pkt))
			{
				// Add element
				DBG_INFO("\r\nBAX PKT OK.");
				AddDataElement(TYPE_BAX_PKT,DATA_DEST_ALL, (5+len), GetOpenStreams(), pkt.b);
			}
			else
			{
				#ifdef ROUTE_ALL_SUBNET
				DBG_INFO("\r\nBAX PKT UNKNOWN.");
				pkt.pktType = RAW_PKT_TYPE;
				AddDataElement(TYPE_BAX_PKT,DATA_DEST_ALL, (5+len), GetOpenStreams(), pkt.b);
				#endif
			}
		}																			
	}
}

void BaxSetDiscoveryCB(void(*CallBack)(BaxPacket_t* pkt))
{
	DBG_INFO("\r\nBax discover: %s",(CallBack)?"ON":"OFF");
	EncryptionPacketCb = CallBack;
}
//EOF
