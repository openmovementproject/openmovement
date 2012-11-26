/* 
 * Copyright (c) 2012, Newcastle University, UK.
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

// [dgj] Fake RAM NVM stubs

#include "HardwareProfile.h"
#include "SystemProfile.h"
#include "ConfigApp.h"
#include "WirelessProtocols/MCHP_API.h"
//#include "WirelessProtocols/NVM.h"


// Structure
typedef struct 
{
    unsigned short MyPANID;
    unsigned char CurrentChannel;
    unsigned char ConnMode;
    unsigned char ConnectionTable[CONNECTION_SIZE * sizeof(CONNECTION_ENTRY)];
    unsigned long OutFrameCounter;
    
    #if defined(PROTOCOL_MIWI) || defined(PROTOCOL_MIWI_PRO)
        unsigned short MyShortAddress;
        unsigned char MyParent;
        
    	#if defined(NWK_ROLE_COORDINATOR)
            
            #ifdef PROTOCOL_MIWI
                unsigned char RoutingTable[8];
        		unsigned char KnownCoordinators;
            #endif
            
            #ifdef PROTOCOL_MIWI_PRO
                unsigned char RoutingTable[(NUM_COORDINATOR/8)];
        		unsigned char NeighborRoutingTable[((unsigned short)NUM_COORDINATOR/8) * ((unsigned short)NUM_COORDINATOR)];
        		unsigned char FamilyTree[NUM_COORDINATOR];
            #endif
    		
            unsigned char Role;
    	#endif
    #endif
    
} nvm_t;    


// Instance of the structure
nvm_t nvm = 
{
    0xFFFF,             // MyPANID;
    0xFF,               // CurrentChannel;
    ENABLE_ALL_CONN,    // ConnMode;
    {0},                // ConnectionTable[CONNECTION_SIZE * sizeof(CONNECTION_ENTRY)];
    0,                  // OutFrameCounter;
    #if defined(PROTOCOL_MIWI) || defined(PROTOCOL_MIWI_PRO)
        0xFFFF,         // MyShortAddress;
        0xFF,           // MyParent;
    	#if defined(NWK_ROLE_COORDINATOR)
            #ifdef PROTOCOL_MIWI
                {0},    // RoutingTable[8];
        		0,      // KnownCoordinators;
            #endif
            #ifdef PROTOCOL_MIWI_PRO
                {0},    // RoutingTable[(NUM_COORDINATOR/8)];
        		{0},    // NeighborRoutingTable[(((unsigned short)NUM_COORDINATOR/8) * ((unsigned short)NUM_COORDINATOR)];
        		{0},    // FamilyTree[NUM_COORDINATOR];
            #endif
            0,
    	#endif
    #endif
};


BOOL NVMInit(void)
{
    return FALSE;
}

void nvmGetMyPANID(unsigned short *v) { *v = nvm.MyPANID; }
void nvmPutMyPANID(unsigned short *v) { nvm.MyPANID = *v; }

void nvmGetCurrentChannel(unsigned char *v) { *v = nvm.CurrentChannel; }
void nvmPutCurrentChannel(unsigned char *v) { nvm.CurrentChannel = *v; }

void nvmGetConnMode(unsigned char *v) { *v = nvm.ConnMode; }
void nvmPutConnMode(unsigned char *v) { nvm.ConnMode = *v; }

void nvmGetConnectionTable(void *v) { memcpy(v, nvm.ConnectionTable, (CONNECTION_SIZE * sizeof(CONNECTION_ENTRY))); }
void nvmPutConnectionTable(void *v) { memcpy(nvm.ConnectionTable, v, (CONNECTION_SIZE * sizeof(CONNECTION_ENTRY))); }
void nvmPutConnectionTableIndex(void *v, unsigned short index) { memcpy(nvm.ConnectionTable + (index * sizeof(CONNECTION_ENTRY)), v, sizeof(CONNECTION_ENTRY)); }

void nvmGetOutFrameCounter(unsigned long *v) { *v = nvm.OutFrameCounter; }
void nvmPutOutFrameCounter(unsigned long *v) { nvm.OutFrameCounter = *v; }

#if defined(PROTOCOL_MIWI) || defined(PROTOCOL_MIWI_PRO)

	void nvmGetMyShortAddress(unsigned short *v) { *v = nvm.MyShortAddress; }
	void nvmPutMyShortAddress(unsigned short *v) { nvm.MyShortAddress = *v; }

	void nvmGetMyParent(unsigned char *v) { *v = nvm.MyParent; }
	void nvmPutMyParent(unsigned char *v) { nvm.MyParent = *v; }
	
	#ifdef NWK_ROLE_COORDINATOR
        #ifdef PROTOCOL_MIWI

    		void nvmGetRoutingTable(void *v) { memcpy(v, nvm.RoutingTable, 8); }
    		void nvmPutRoutingTable(void *v) { memcpy(nvm.RoutingTable, v, 8); }
		
    		void nvmGetKnownCoordinators(unsigned char *v) { *v = nvm.KnownCoordinators; }
    		void nvmPutKnownCoordinators(unsigned char *v) { nvm.KnownCoordinators = *v; }
        #endif
    		
        #ifdef PROTOCOL_MIWI_PRO

    		void nvmGetRoutingTable(void *v) { memcpy(v, nvm.RoutingTable, (NUM_COORDINATOR/8)); }
    		void nvmPutRoutingTable(void *v) { memcpy(nvm.RoutingTable, v, (NUM_COORDINATOR/8)); }
    		
    		void nvmGetNeighborRoutingTable(void *v) { memcpy(v, nvm.NeighborRoutingTable, (((unsigned short)NUM_COORDINATOR/8) * ((unsigned short)NUM_COORDINATOR))); }
    		void nvmPutNeighborRoutingTable(void *v) { memcpy(nvm.NeighborRoutingTable, v, (((unsigned short)NUM_COORDINATOR/8) * ((unsigned short)NUM_COORDINATOR))); }
    
    		void nvmGetFamilyTree(void *v) { memcpy(v, nvm.FamilyTree, NUM_COORDINATOR); }
    		void nvmPutFamilyTree(void *v) { memcpy(nvm.FamilyTree, v, NUM_COORDINATOR); }
        #endif

		void nvmGetRole(unsigned char *v) { *v = nvm.Role; }
		void nvmPutRole(unsigned char *v) { nvm.Role = *v; }
	#endif
#endif


