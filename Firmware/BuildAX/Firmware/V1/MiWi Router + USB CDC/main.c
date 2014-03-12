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
 
// Dan Jackson, 2012
 
 // Main 
#include "HardwareProfile.h"
#include "WirelessProtocols/Console.h"
//#include "Transceivers/Transceivers.h"
#include "WirelessProtocols/MCHP_API.h"
#include "WirelessProtocols/SymbolTime.h"
#include "Settings.h"
#include "Network.h"
#include "ConfigApp.h"
#include "WirelessProtocols/NVM.h"
#include "Peripherals/Rtc.h"
#include "Utils/Util.h"
#include "Peripherals/myi2c.h"

#include <USB\usb.h>
#include <USB\usb_device.h>
#include <USB\usb_function_cdc.h>
#include "USB/USB_CDC_MSD.h"

#undef ConsoleIsGetReady
#define ConsoleIsGetReady() (USBProcessIO(), usb_haschar())


extern BYTE RoutingTable[NUM_COORDINATOR/8];



#ifndef ROUTER
#error "ROUTER not defined"
#endif
#ifdef SENSOR
#error "SENSOR is defined"
#endif


void __attribute__((interrupt,auto_psv)) _RTCCInterrupt(void)
{
	RtcTasks(); // Keeps time up to date
}
void __attribute__((interrupt,auto_psv)) _T1Interrupt(void)
{
	RtcTimerTasks();// Needed to keep RTC upto date
}
void __attribute__((interrupt, shadow, auto_psv)) _ADC1Interrupt(void)
{
	ADC1tasks(); 	// Must be first in ISR
}
void __attribute__((interrupt,auto_psv)) _CNInterrupt(void)  // Change notification interrupts
{
	IFS1bits.CNIF = 0; // Clear flag
}
void __attribute__((interrupt,auto_psv)) _DefaultInterrupt(void)
{
	Nop();
	Nop();
	Nop();
	Nop();
	Reset();
} 


// Status packet
typedef struct
{
    unsigned char  reportType;          // @ 0  [1] USER_REPORT_TYPE (0x12)
    unsigned char  reportId;            // @ 1  [1] Report identifier (0x53, ASCII 'S')
    unsigned short deviceId;            // @ 2  [2] Device identifier (16-bit)
    unsigned char  version;             // @ 4  [1] Low nibble = packet version (0x3), high nibble = config (0x0)
    unsigned char  power;               // @ 5  [1] Power (top-bit indicates USB, if low-15 are 0x7ffff, unknown)
	unsigned short sequence;			// @ 6  [2] Packet sequence number
    unsigned short shortAddress;		// @ 8  [2] Short address
    unsigned char  lastLQI;				// @ 10 [1] LQI of last received keep-alive broadcast
    unsigned char  lastRSSI;			// @ 11 [1] RSSI of last received keep-alive broadcast
    unsigned short parentAddress;		// @ 12 [2] Parent address
    unsigned short parentAltAddress;	// @ 14 [2] Parent alt. address
    unsigned char  neighbours[NUM_COORDINATOR/8];	// @ 16 [8] Neighbouring routers bitmap
} __attribute__ ((packed, aligned(1))) statusPacket_t;

statusPacket_t statusPacket = {0};

unsigned short packetSequence = 0;
unsigned char lastLQI = 0xff, lastRSSI = 0xff;

// For parity with sensor -- Settings.c can access this struct
reading_t reading = {0};


char TransmitStatusPacket(void)
{
    int i;
    BYTE destinationAddress[8] = {0,0,0,0,0,0,0,0};
    size_t len;
    BYTE result;

    // Set the status packet
    statusPacket.reportType = 0x12;
    statusPacket.reportId = 0x53;
    statusPacket.deviceId = settings.deviceId;
    statusPacket.version = 1;	// 0 = broken neighbour table, 1 = correct neighbour table
    
	// Power status
    statusPacket.power = 0x7f;
    if (USB_BUS_SENSE) statusPacket.power |= 0x80;
    
    // Packet sequence
	statusPacket.sequence = packetSequence++;			    

    // Short address
    statusPacket.shortAddress = *(unsigned short *)myShortAddress.v;
    
	// RSS of last broadcast received
    statusPacket.lastLQI = lastLQI;	
    statusPacket.lastRSSI = lastRSSI;

	// Own parent entry in family tree
    statusPacket.parentAddress = (unsigned short)ConnectionTable[myParent].Address[0] | ((unsigned short)ConnectionTable[myParent].Address[1] << 8);
    statusPacket.parentAltAddress = *(unsigned short *)ConnectionTable[myParent].AltAddress.v;

	// 64/8 = 8 byte bitmap of neighbour table
	for (i = 0; i < NUM_COORDINATOR/8; i++)
	{
		statusPacket.neighbours[i] = RoutingTable[i];
	}
    
    // Destination
    destinationAddress[0] = (unsigned char)settings.destination; //myLongAddress[0];
    destinationAddress[1] = (unsigned char)(settings.destination >> 8); //myLongAddress[1];
    destinationAddress[2] = myLongAddress[2];
    destinationAddress[3] = myLongAddress[3];
    destinationAddress[4] = myLongAddress[4];
    destinationAddress[5] = myLongAddress[5];
    destinationAddress[6] = myLongAddress[6];
    destinationAddress[7] = myLongAddress[7];
    
    //len = sizeof(sendingDataPacket);
    len = sizeof(statusPacket);
    
    // Write packet
    MiApp_FlushTx();
    for (i = 0; i < len; i++)
    {
	    MiApp_WriteData(*((unsigned char *)&statusPacket + i));
    }    

	//DumpPacket(&statusPacket);
   
    // Transmit packet
    ConsolePutROMString("Unicast... ");
    #if 0
        result = MiApp_UnicastAddress(destinationAddress, TRUE, FALSE);     // long
    #else
        result = MiApp_UnicastAddress(destinationAddress, FALSE, FALSE);    // short
    #endif
    ConsolePutROMString(result ? "OK\r\n" : "FAIL\r\n");
    
    return result;
}    

#ifdef COORD
unsigned char seen[NUM_COORDINATOR] = {0};
#define SEEN_COUNTER_RESET 12		// 120 seconds 'seen' history for a router
#endif

int main(void)
{   
    BYTE i, j;
	static unsigned long lastUpdate;
	static unsigned long lastKeepalive;
	static unsigned long lastReceived;
	static unsigned long tnow;
    BOOL bConnected;
    
    //unsigned short failCount = 0;
    // TODO: PAN-Coord to send keep-alive [Multicast to all coordinators (0xFFFD), Multicast to all FFDs (0xFFFE), Broadcast to all devices (0xFFFF)].

    // Initialize the system + clock
    InitIO();
    CLOCK_SOSCEN();
    CLOCK_INTOSC();
	RtcStartup();
    RtcInterruptOn(0);  // RTC interrupt only -- needed for low power flash

    CheckLowPowerCondition();
    
    SettingsInitialize();
    
    #if (defined (USE_USB_CDC) || defined(RADIO_PLL))
        CLOCK_PLL();
        DelayMs(5);
    #endif
    
	InitI2C();
	
	// Init the RTC to ensure the time is OK
	RtcInterruptOn(0);

	// This connects to the USB if present
	ConsoleInit(); 
   
    NetworkInit();

    // Attempt to init with freezer unless button is pressed
    bConnected = TRUE;
    if ((PUSH_BUTTON_1 == 0) || ( MiApp_ProtocolInit(TRUE) == FALSE))
    {
        MiApp_ProtocolInit(FALSE);  // Init without freezer
        bConnected = FALSE;
    }
    else
    {
        // Show network freezer settings used
        DumpConnection(0xFF);
    }
    
    Printf("\r\nDevice id = "); PrintChar(settings.deviceId >> 8); PrintChar(settings.deviceId & 0xff);
    /*
    {
        BYTE role;
        nvmGetRole(&role);
        if (role == ROLE_FFD_END_DEVICE) { Printf("I am an FFD end-device\r\n"); }
        else if (role == ROLE_COORDINATOR) { Printf("I am a coordinator\r\n"); }
        else if (role == ROLE_PAN_COORDINATOR) { Printf("I am a pan coordinator\r\n"); }
        else { Printf("I am an unknown device\r\n"); }        
    }
    */
    
    tnow = RtcTicks();
    lastUpdate = 0;					// Need to update now
    lastKeepalive = tnow;	// Don't trigger keep-alive timeout yet
    lastUpdate = tnow;
    //lastReceived = tnow;
    lastReceived = 0;				// Not received
    
    // Main router loop
    while (USB_BUS_SENSE == status.usb)
    {
tnow = RtcTicks();
        if (!bConnected)
        {
            if (!USB_BUS_SENSE || !status.noconnect)
            {
LED_SET(LED_YELLOW);
                bConnected = Reconnect();
                //LCDDisplay((char *)"Start Connection on Channel %d", currentChannel, TRUE);
                if (bConnected)
                {
                    lastKeepalive = tnow;
                    packetSequence = 0;
                }
LED_SET(LED_OFF);
            }
        }
        else
        {
#ifdef COORD
            // Coordinator's update is a keep-alive
            if ((tnow - lastUpdate) > 10 * 0x10000ul)
            {
                MiApp_FlushTx();
        	    MiApp_WriteData(0x12);
                MiApp_WriteData('B');
                MiApp_WriteData((unsigned char)settings.deviceId);
                MiApp_WriteData((unsigned char)(settings.deviceId >> 8));
                
                // Bitmap of 'seen-within-the-last-two-minutes' coordinators (so they know they are visible)
				for (i = 0; i < NUM_COORDINATOR/8; i++)
				{
					unsigned char c = 0x00, b;
					for (b = 0; b < 8; b++)
					{
						if (seen[8 * i + b])
						{
							seen[8 * i + b]--;
							c |= (1 << b);
						}
					}
	                MiApp_WriteData(c);
				}
				
                MiApp_BroadcastPacket(FALSE);
                lastKeepalive = lastUpdate = tnow;
            }
#else    
            // Disconnected if failed to receive keep-alive
            if ((tnow - lastKeepalive) > 130 * 0x10000ul)
            {
                bConnected = FALSE;
                continue;
            }    
            
            // Router's update to coordinator
static unsigned long diff;
diff = (tnow - lastUpdate);
static unsigned long compare = 30 * 0x10000ul;
if (diff > compare)
            {
				TransmitStatusPacket();	            
                lastUpdate = tnow;
            }
#endif
            
// Status colour
if ((tnow - lastReceived) <= 5000)
{
    LED_SET(LED_GREEN);
}    
else
{
    if ((tnow - lastKeepalive) <= 10000)
    {
        LED_SET(LED_WHITE);
    }    
    else if ((tnow - lastKeepalive) <= 10 * 0x10000ul)
    {
        LED_SET(LED_BLUE);
    }    
    else
    {
        LED_SET(LED_RED);
    }    
}    
            
            
            if (MiApp_MessageAvailable())
            {
                LED_SET(LED_OFF);
                
                #ifdef COORD
                if (rxMessage.PayloadSize >= 10 && rxMessage.Payload[0] == 0x12 && rxMessage.Payload[1] == 'S')
                {
	                unsigned short shortAddress = rxMessage.Payload[8] | ((unsigned short)rxMessage.Payload[9] << 8);
	                int index = (shortAddress >> 8);
					if (index < NUM_COORDINATOR)
					{
						seen[index] = SEEN_COUNTER_RESET;
					}
	            }
                #endif 
                
                //rxMessage.flags.bits.secEn, rxMessage.flags.bits.broadcast, rxMessage.PacketLQI, rxMessage.PacketRSSI, rxMessage.flags.bits.srcPrsnt, rxMessage.flags.bits.altSrcAddr, rxMessage.SourceAddress[], rxMessage.PayloadSize, rxMessage.Payload[]
                if (rxMessage.PayloadSize >= 2 && rxMessage.Payload[0] == 0x12 && rxMessage.Payload[1] == 'B')
                {
	                // Received a keep-alive
                    #ifndef COORD
						lastLQI = rxMessage.PacketLQI;
						lastRSSI = rxMessage.PacketRSSI;
						{
							unsigned short shortAddress = *(unsigned short *)myShortAddress.v;
							int offset = 4 + ((shortAddress >> 8) / 8);
							// If we've got our short address's bit set, or the broadcast doesn't include visiblity
							if (rxMessage.PayloadSize < offset || (rxMessage.Payload[offset] & (1 << ((shortAddress >> 8) % 8))))
							{
		                        lastKeepalive = tnow;
							}
						}
                    #endif
                    //NetworkDumpMessage();
                }
#ifdef SLIP
                else
                {
                    lastReceived = tnow;
#ifdef COORD
            	    // Send as SLIP (RFC 1055) encoded packet
            	    usb_putchar(SLIP_END);
            	    usb_slip_encoded(rxMessage.Payload, rxMessage.PayloadSize);
            	    usb_putchar(SLIP_END);
#endif
//NetworkDumpMessage();
                }	    
#else
                else if (rxMessage.PayloadSize >= 5 && rxMessage.Payload[0] == 0x12 && rxMessage.Payload[1] == DATA_TEDDI_REPORT_ID)
                {
	                // Received a report
                    lastReceived = tnow;
#ifdef COORD
                    DumpPacket((dataPacket_t *)rxMessage.Payload);
#endif
                }    
                else
                {
	                // Received another message
                    lastReceived = tnow;
#ifdef COORD
                    NetworkDumpMessage();
#endif
                }    
#endif
                
                MiApp_DiscardMessage();
            }
        }    
        
        USBProcessIO();
        {
            const char *line = _user_gets();
            if (line != NULL) { SettingsCommand(line); }	
        }
            
    }
    
	#ifdef USE_USB_CDC
	    USBDeviceDetach();
    #endif
    
    // Shut down all, then reset
    //RadioOff();
    SystemPowerOff(0);

    
}

