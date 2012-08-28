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

// main-receiver.c - Main module for receiver
// Dan Jackson, Cas Ladha, Karim Ladha, 2010-2011.

#include "HardwareProfile.h"

#if (DEVICE_TYPE == 0)				// 0 = FFD Attached receiver


// Includes
#include "ConfigApp.h"
#include <stdio.h>
#include <delays.h>
#include <reset.h>
#include "usb_cdc.h"
#include "util.h"
//#include "accel.h"
#include "data.h"
//#include "data-teddi.h"
#include "WirelessProtocols\MCHP_API.h"
//#include "WirelessProtocols\SymbolTime.h"

// Variables
#pragma idata


// MiWi
#if ADDITIONAL_NODE_ID_SIZE > 0
    BYTE AdditionalNodeID[ADDITIONAL_NODE_ID_SIZE] = {0};
#endif

volatile unsigned int accelDoubleTaps = 0;
volatile unsigned int accelSingleTaps = 0;


#pragma udata


#pragma code

// Private prototypes
extern void CriticalError(void);
extern void DisableIO(void);
extern void ShippingMode(void);
extern void WaitForPrecharge(void);
extern void DeviceId(void);
void RunStartup(void);
char RunAttached(void);
void RunDetatched(void);
void ProcessConsoleCommand(void);


// HighISR - defined in MRF24J40 tranceiver code and chains to UserInterruptHandler()
// UserInterruptHandler - Non-RF high-priority interrupt handler
void UserInterruptHandler()
{
	// USB bus sense - our "reset switch"
	if (USB_BUS_SENSE_IF)
	{
		Reset();
		USB_BUS_SENSE_IF = 0;		// never called
	}

	#if !defined(DISABLE_USB) && defined(USB_INTERRUPT) && !defined(USB_INTERRUPT_LOW)
		// Check bus status and service USB interrupts (call once every ~100us, in most cases executes in ~50 instruction cycles)
		USBDeviceTasks(); 
	#endif

    // return to HighISR()
}


// LowISR - Low-priority interrupt handler
#pragma interruptlow LowISR
void LowISR()
{
    // Check which interrupt flag caused the interrupt, service the interrupt, clear the interrupt flag.
	if (ACCEL_INT1_IF)              // ADXL interrupt
	{
		CriticalError();			// Shouldn't be enabled
	}

	#if !defined(DISABLE_USB) && defined(USB_INTERRUPT) && defined(USB_INTERRUPT_LOW)
		// Check bus status and service USB interrupts (call once every ~100us, in most cases executes in ~50 instruction cycles)
		USBDeviceTasks(); 
	#endif
    // return from #pragma interruptlow section
}


// Declarations
#pragma code


// main - main function
void main(void)
{
    // Start-up code: remappable pins, battery pre-charge, initialize 8MHz INTOSC, read configuration from flash, init UART is used
    RunStartup();

    // If USB detected then run USB mode until disconnected (then reset); otherwise...
    if (RunAttached())            
    {
		// USB detatched... switch CPU to 8MHz and disable USB module
	    UCONbits.USBEN = 0;		// USBEN can only be change when module clocked
		CLOCK_INTOSC();
	    UCONbits.SUSPND = 1;    // Suspend USB
		Reset();				// Reset back to non-attatched mode
	}

	// Run detatched
    RunDetatched();

    // Reset when charging / USB detected
    Reset();
}



// Startup code
void RunStartup(void)
{
#ifdef USE_WDT
    // Enable WDT (WDTEN hardware config must be OFF for software control)
	ClrWdt();
    WDTCONbits.SWDTEN = 1;
#endif

    // WaitForPrecharge - includes remappable pin setup
	WaitForPrecharge();

    // Switch to INTOSC
    CLOCK_INTOSC();
	ClrWdt();

    // Load configuration from ROM (patch device ID)
    DataConfigLoad();

	// Set standard output to user output function
	stdout = _H_USER;

	// NOTE: Remapping and configuration of pins already done in WaitForPrecharge()

	// Enable interrupts
    INTCONbits.GIE = 1;     // Enable interrupts

	// Start debug USART if enabled
#if defined(DEBUG_USART2_TX) || defined(UART2_DATA_OUTPUT)
	#warning "Not expected to have debug USART enabled -- ignore if you really wanted it."
    USARTStartup();
#endif

    // We use bus sense interrupt as our reset switch
#ifdef USE_USB_BUS_SENSE_IO
    USB_BUS_SENSE_IE = 1;   // USB bus sense interrupt
#endif

    // Startup finished
    return;
}


// RunAttached - Run system in USB mode (recharge, communication, 48 MHz) until a disconnect occurs
char RunAttached(void)
{
	// Return 0 if the USB is not connected
	if (!USB_BUS_SENSE) return 0;

	// LED on
	LED = 1;

    // Start MiWi
    RF_INIT_PINS();

	// Initialize the protocol stack defined in ConfigApp.h. (false = don't restore previous network configuration)
    MiApp_ProtocolInit(FALSE);

    // Set default channel
    MiApp_SetChannel(dataConfig.channel); // (== FALSE) -- Channel not supported in current condition

    // Select the connection mode (from: ENABLE_ALL_CONN, ENABLE_PREV_CONN, ENABLE_ACTIVE_SCAN_RSP, DISABLE_ALL_CONN)
    MiApp_ConnectionMode(ENABLE_ALL_CONN);

	// Try to establish a new connection with peer device (index 0xff = any peer, mode = CONN_MODE_DIRECT)
    //#ifdef ENABLE_HAND_SHAKE
        //i = MiApp_EstablishConnection(0xFF, );
    //#endif
    //MiApp_StartConnection(START_CONN_DIRECT, 10, 0);
    //DumpConnection(0xFF);  // Debug dump connection entry (0xff = all)

	// Start USB (initializes PLL)
#ifndef DISABLE_USB
    USBInitializeSystem();
#else
	CLOCK_USB_XTAL();
	USARTStartup1MBaud(); // Higher speed needed in receive mode = 1MBaud
#endif

#ifndef DISABLE_USB
    // Now USB running at 48MHz -- CPU running at 8 MHz
    #if defined(USB_INTERRUPT)
		#if !defined(USB_INTERRUPT_LOW)
			USBDeviceAttach();
		#else
			USBDeviceAttachLow();
		#endif
    #endif
#endif

	// Enable interrupts
    INTCONbits.GIE = 1;     

#ifdef MOTOR_DEVICE
	MOTOR_INIT();
#endif

    // Main loop
    while (USB_BUS_SENSE)
    {

        // Check whether a packet has been received by the transceiver. 
        if (MiApp_MessageAvailable())
        {
			do
			{
// LED blink off (also in MRF24J40 on interrupt)
LED = 0;

	            // Examine message stored in the rxFrame structure of RECEIVED_MESSAGE.
	            #ifdef ENABLE_SECURITY
	                //if (rxMessage.flags.bits.secEn) ...		// Check whether secured
	            #endif
	            #ifndef TARGET_SMALL
	                //if (rxMessage.flags.bits.broadcast) ...	// Check whether broadcast or unicast
	                //rxMessage.PacketRSSI
	                //if (rxMessage.flags.bits.srcPrsnt)		// Check whether has a source
	                //{
	                //    if (rxMessage.flags.bits.altSrcAddr) { rxMessage.SourceAddress[1], rxMessage.SourceAddress[0] }
	                //    else { for (i = 0; i < MY_ADDRESS_LENGTH; i++) { rxMessage.SourceAddress[MY_ADDRESS_LENGTH-1-i]; } }
	                //}
	            #endif
	
				// Check report type
	            if (rxMessage.Payload[0] == USER_REPORT_TYPE)			// 0x12
	            {
	                // Check report ID
		            if (rxMessage.Payload[1] == DATA_ACCEL_REPORT_ID)	// 0x78 (ASCII 'x')
		            {
						DataDumpPacket((DataPacket *)&(rxMessage.Payload[0]), rxMessage.PayloadSize);
					}
					/*
					else if (rxMessage.Payload[1] == DATA_TEDDI_REPORT_ID)	// 0x54 (ASCII 'T')
					{
						TeddiDataDumpPacket((TeddiDataPacket *)&(rxMessage.Payload[0]), rxMessage.PayloadSize);
					}
					*/
				}
	            
	            // Release the current received packet so that the stack can start to process the next received frame.
	            MiApp_DiscardMessage();
// LED back on
LED = 1;

	  		} while (MiApp_MessageAvailable());

        }
        else
        {
            // If no packet received, check if we want to send any
            //MiApp_FlushTx();                                            // Reset the transmit buffer
            //MiApp_WriteData(USER_REPORT_TYPE);
            //MiApp_WriteData('X');
            ////MiApp_UnicastAddress(destinationAddress, TRUE, FALSE);    // Send to specific address (BYTE *address, true=permanent, false=unsecured)
			////MiApp_UnicastConnection(id, FALSE) 						// Send to known connection ID (id, false=unsecure)
			//MiApp_BroadcastPacket(FALSE);								// Broadcast packet (false=unsecure)
        }

#ifndef DISABLE_USB
        #if defined(USB_POLLING)
            // Check bus status and service USB interrupts (call once every ~100us, in most cases executes in ~50 instruction cycles)
            USBDeviceTasks(); 
        #endif

        // Handle serial console commands
        ProcessConsoleCommand();
#endif

        // USB process I/O : KL: Changed to use define DISABLE_USB
        USBProcessIO();

        // Check we're still attached
		#ifndef USE_USB_BUS_SENSE_IO
		#error Must be bus-sense enabled (otherwise must clear watchdog timer)
		#endif
        if (USB_BUS_SENSE) { ClrWdt(); }   // Clear WDT while the USB detect line is high
    }

#ifdef MOTOR_DEVICE
	MOTOR_OFF();
#endif

	// Detatching, LED off
	LED = 0;

    #if defined(USB_INTERRUPT)
        USBDeviceDetach();    // Wait >100ms before re-attach
    #endif

    return 1;                 // Attach mode was entered, now completed (detached)
}


// Append variable number of bytes to packet
int VariableAppend(const char *str)
{
	const char *p = str;
	int value = -1;
	int count = 0;
	
	do
	{
		if (*p >= '0' && *p <= '9')
		{
			if (value < 0) { value = 0; }
			value = 10 * value + (*p - '0');
		}
		else if (value >= 0)
		{
			MiApp_WriteData((unsigned char)value);
			count++;
			value = -1;
		}
	} while (*p++ != '\0');
	
	return count;
}


int ExtractNum(char *str, unsigned int *outValue)
{
	unsigned int value = 0xffff;
	int o = 0;
	
	do
	{
		if (str[o] >= '0' && str[o] <= '9')
		{
			if (value == 0xffff) { value = 0; }
			value = 10 * value + (str[o] - '0');
		}
		else if (value != 0xffff)
		{
			*outValue = value;
			if (str[o] != '\0') { str[o] = '\0'; o++; }
			return o;
		}
	} while (str[o++] != '\0');
	
	return 0;
}


// RunDetatched - Run system in detatched mode
void RunDetatched()
{
	// As this is a receiver, low power mode until connected
	// TODO: Shut down until wake on USB
	CLOCK_INTRC();
	while (!USB_BUS_SENSE) 
	{
		LED = 1;
		Delay10TCYx(5);
		LED = 0;
		Delay1KTCYx(8); 	// At 32kHz: ~1 sec
		if (!USB_BUS_SENSE) { ClrWdt(); }
	}
	return;
}


// ProcessConsoleCommand
void ProcessConsoleCommand(void)
{
    const char *line;
    line = _user_gets();
    if (line != NULL && strlen(line) > 0)
    {
        //printf((const rom far char *)"%s\r\n", line);
        if (stricmp_rom(line, (const rom far char *)"help") == 0)
        {
            printf((const rom far char *)"HELP: help|echo|status|id|reset|device|channel|mode\r\n");
        }
        else if (strnicmp_rom(line, (const rom far char *)"echo", 4) == 0)
        {
            int value = -1;
            if (line[5] != '\0') { value = (int)my_atoi(line + 5); }
            if (value == 0 || value == 1)
            {
                commEcho = value;
            }
            printf((const rom far char *)"ECHO=%d\r\n", commEcho);
        }
        else if (stricmp_rom(line, (const rom far char *)"status") == 0)
        {
      		printf((const rom far char *)"$STATUS1=INT0E:%d,INT0F:%d,INT1E:%d,INT1F:%d,INT2E:%d,INT2F:%d,GIE:%d\r\n", INTCONbits.INT0IE, INTCONbits.INT0IF, INTCON3bits.INT1IE, INTCON3bits.INT1IF, INTCON3bits.INT2IE, INTCON3bits.INT2IF, INTCONbits.GIE);
        }
        else if (stricmp_rom(line, "id") == 0)
        {
#ifdef MOTOR_DEVICE
            printf("ID=WAX-r(m),%x,%x,%u\r\n", HARDWARE_VERSION, SOFTWARE_VERSION, dataStatus.deviceId);
#else
            printf("ID=WAX-r,%x,%x,%u\r\n", HARDWARE_VERSION, SOFTWARE_VERSION, dataStatus.deviceId);
#endif
        }
        else if (stricmp_rom(line, (const rom far char *)"reset") == 0)
        {
      		printf((const rom far char *)"RESET\r\n");
      		printf((const rom far char *)"OK\r\n");
            Delay10KTCYx(200);        // Briefly wait (allows client to close port before device dies)
            #if defined(USB_INTERRUPT)
                USBDeviceDetach();    // Wait >100ms before re-attach
            #endif
            Reset();
        }
        else if (stricmp_rom(line, (const rom far char *)"shipping") == 0)
        {
      		printf((const rom far char *)"SHIPPING\r\n");
      		printf((const rom far char *)"OK\r\n");
            Delay10KTCYx(200);        // Briefly wait (allows client to close port before device dies)
            #if defined(USB_INTERRUPT)
                USBDeviceDetach();    // Wait >100ms before re-attach
            #endif
            ShippingMode();
        }
        else if (strnicmp_rom(line, "device", 6) == 0)
        {
            unsigned short id = 0xffff;
            if (line[6] != '\0') { id = (unsigned short)my_atoi(line + 7); }
            if (id != 0xffff)
            {
				DataConfigSetId(id);
				DataConfigSave();
            }
            printf("DEVICE=%u\r\n", dataStatus.deviceId);
        }
        else if (strnicmp_rom(line, "channel", 7) == 0)
        {
            unsigned char channel = 0xff;
            if (line[7] != '\0') { channel = (unsigned char)my_atoi(line + 8); }
            if (channel != 0xff)
            {
				dataConfig.channel = channel;
				DataConfigSave();
            }
            printf("CHANNEL=%u\r\n", dataConfig.channel);
        }
        else if (strnicmp_rom(line, "startmode", 9) == 0)
        {
            unsigned char startmode = 0xff;
            if (line[9] != '\0') { startmode = (unsigned char)my_atoi(line + 10); }
            if (startmode != 0xff)
            {
				dataConfig.startRxMode = startmode;
				DataConfigSave();
            }
            printf("STARTMODE=%u\r\n", dataConfig.startRxMode);
        }
        else if (strnicmp_rom(line, "mode", 4) == 0)
        {
            unsigned char mode = 0xff;
            if (line[4] != '\0') { mode = (unsigned char)my_atoi(line + 5); }
            if (mode != 0xff)
            {
				dataStatus.rxMode = mode;
            }
            printf("MODE=%u\r\n", dataStatus.rxMode);
        }
        else if (strnicmp_rom(line, "sendm", 5) == 0 || strnicmp_rom(line, "sendb", 5) == 0)
        {
	        char *p = line + 5;
			char success;
			int count;
			unsigned int firstDeviceId = 0;
			
			p += ExtractNum(p, &firstDeviceId);

			if (firstDeviceId == 0)
			{
	            printf("SEND=UNSPECIFIED\r\n");
	  		}          
	  		else
	  		{
				// Reset the transmit buffer
				MiApp_FlushTx();
			
			    // IMPORTANT: This code has to mirror the DataPacket struct
				MiApp_WriteData(USER_REPORT_TYPE);                      // reportType;		    // [1] = 0x12 (USER_REPORT_TYPE)
				MiApp_WriteData('M');                  					// reportId;            // [1] = (ASCII 'M')
				MiApp_WriteData(0);                  					// version;             // [1] = Packet version
				MiApp_WriteData(0);                  					// count;               // [1] = Number of devices (calculate from packet length if 0)
				MiApp_WriteData((unsigned char)firstDeviceId);          // firstDeviceId;       // [2] = First device identifier (16-bit)
				MiApp_WriteData((unsigned char)(firstDeviceId >> 8));   // firstDeviceId;       // [2] = First device identifier (16-bit)

				count = VariableAppend(p);
				
				if (count % 2 != 0 || count <= 0)
				{
		            printf("SEND=INVALID\r\n");
				}
				else
				{
					if (line[4] == 'b' || line[4] == 'B')
					{
						success = MiApp_BroadcastPacket(FALSE);							// Broadcast packet (false=unsecure)
					}
					else
					{
						static BYTE targetLongAddress[8];
						targetLongAddress[0] = (unsigned char)firstDeviceId;
						targetLongAddress[1] = (unsigned char)(firstDeviceId >> 8);
						targetLongAddress[2] = EUI_2;
						targetLongAddress[3] = EUI_3;
						targetLongAddress[4] = EUI_4;
						targetLongAddress[5] = EUI_5;
						targetLongAddress[6] = EUI_6;
						targetLongAddress[7] = EUI_7;
						success = MiApp_UnicastAddress(targetLongAddress, TRUE, FALSE);     // Send to specific address (BYTE *address, true=permanent, false=unsecured)
					}
					
					if (!success)
					{
			            printf("SEND=FAILED\r\n");
					}
				}
			}	
        }
#ifdef MOTOR_DEVICE
        else if (strnicmp_rom(line, "motor", 5) == 0)
        {
            int duty = -1;
            if (line[5] != '\0') { duty = (unsigned char)my_atoi(line + 6); }
            if (duty >= 0x00 && duty <= 0xff)
            {
				MOTOR_SET_DUTY((unsigned char)duty);
            }
            else if (duty > 0xff)
            {
				MOTOR_SET_DUTY(0xff);
	            MOTOR_ON();
            }
            printf("MOTOR=%d\r\n", MOTOR_GET_DUTY());
        }
#endif
        else if (stricmp_rom(line, (const rom far char *)"exit") == 0)
        {
      		printf((const rom far char *)"NOTE: You'll have to close the terminal window yourself!\r\n");
        }
        else if (stricmp_rom(line, (const rom far char *)"AT") == 0)
        {
            ;
        }
        else if (line[0] == '\0')
        {
            printf((const rom far char *)"\r\n");
        }
        else
        {
            printf((const rom far char *)"ERROR: Unknown command: %s\r\n", line);
        }
    }
}

#else
static char dummy;	// C18 doesn't like empty object files
#endif
