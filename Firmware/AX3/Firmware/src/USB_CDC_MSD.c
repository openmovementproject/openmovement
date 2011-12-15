/* 
 * Copyright (c) 2009-2011, Newcastle University, UK.
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

// USB CDC and MSD I/O
// Kim Ladha, Dan Jackson, 2011

#include "USB/USB.h"
#include "HardwareProfile.h"
#include "FSconfig.h"
#include "MDD File System\internal flash.h"
#include "USB_CDC_MSD.h"
#include "./USB/usb_function_msd.h"
#include "./USB/usb_function_cdc.h"
#include "Settings.h"
#include <TimeDelay.h>

// Globals
// USB circular buffer head/tail pointers
unsigned char inHead = 0, inTail = 0;
unsigned char outHead = 0, outTail = 0;

#define IN_BUFFER_CAPACITY 128
#define OUT_BUFFER_CAPACITY 256
static unsigned char inBuffer[IN_BUFFER_CAPACITY];
static unsigned char outBuffer[OUT_BUFFER_CAPACITY];

// Allows manipulation of endpoint and buffer
extern USB_HANDLE CDCDataOutHandle;
extern USB_HANDLE CDCDataInHandle;
extern volatile FAR unsigned char cdc_data_rx[CDC_DATA_OUT_EP_SIZE];
extern volatile FAR unsigned char cdc_data_tx[CDC_DATA_IN_EP_SIZE];


#if USB_NUM_STRING_DESCRIPTORS > 3
// USB Product serial number -- serial number is overwritten and is initially ',' 
// characters as that triggers the "invalid serial number" behaviour on Windows
struct { BYTE bLength; BYTE bDscType; WORD string[11]; } sd003 =
{
    sizeof(sd003), USB_DESCRIPTOR_STRING,		// sizeof(sd003) = 24, USB_DESCRIPTOR_STRING = 0x03
    {'C','W','A','0' + (HARDWARE_VERSION >> 4),'0' + (HARDWARE_VERSION & 0x0f),'_',',',',',',',',',','}
}; //                                                                            14  16  18  20  22
#endif


// Standard Response to INQUIRY command stored in ROM
const ROM InquiryResponse inq_resp =
{
    0x00,   // peripheral device is connected, 0 = direct access block device
    0x80,   // high bit = removable
    0x04,   // version, 0 = does not conform to any standard, 4 = SPC-2, 5 = SPC-3
    0x02,   // response is in format specified by SPC-2
    0x20,   // n-4 = 36-4=32= 0x20
    0x00,   // sccs etc.
    0x00,   // bque=1 and cmdque=0,indicates simple queueing 00 is obsolete, but as in case of other device, we are just using 00
    0x00,   // 00 obsolete, 0x80 for basic task queueing
    {'C','W','A',' ',' ',' ',' ',' '},                                  // T10 assigned Vendor identification
    {'C','W','A',' ','M','a','s','s',' ','S','t','o','r','a','g','e'},  // Product identification
    {'0' + (HARDWARE_VERSION >> 12),'0' + (HARDWARE_VERSION >> 8),'0' + (HARDWARE_VERSION >> 4),'0' + (HARDWARE_VERSION & 0x0f)} // Product revision level
};


#if defined(__C30__) || defined(__C32__)
// The LUN variable definition for the MSD driver.  
LUN_FUNCTIONS LUN[MAX_LUN + 1] = 
{
    {
        &MDD_MediaInitialize,
        &MDD_ReadCapacity,
        &MDD_ReadSectorSize,
        &MDD_FTL_USB_MediaDetect,
        &MDD_SectorRead,
        &MDD_WriteProtectState,
        &MDD_SectorWrite
    }
};
#endif

#pragma code

// Repurposed - sort out you own oscillator stuff prior to this
void USBInitializeSystem(void)
{
    // Clear USB CDC I/O buffers
    inHead = 0; inTail = 0;
    outHead = 0; outTail = 0;

#if USB_NUM_STRING_DESCRIPTORS > 3
    // If we have a valid serial number, patch the USB response with the serial number
    if (settings.deviceId != 0xffff)
	{
        unsigned short *number = (unsigned short *)((char *)&sd003 + 14);
        *number++ = (signed short)'0' + ((settings.deviceId / 10000) % 10);
        *number++ = (signed short)'0' + ((settings.deviceId /  1000) % 10);
        *number++ = (signed short)'0' + ((settings.deviceId /   100) % 10);
        *number++ = (signed short)'0' + ((settings.deviceId /    10) % 10);
        *number++ = (signed short)'0' + ((settings.deviceId        ) % 10);
    }
#endif

	// If the module is already running (e.g. after bootloader), then hard detatch
	if (U1CONbits.USBEN)
	{
		U1CONbits.USBEN = 0;
	    DelayMs(2 * 4); // Wait >2ms before re-attach (*4 as PLL is on)
	}

    USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
    					//variables to known states.
}


// USB-specific put char
void usb_putchar(unsigned char v)
{
    // If the queue is not full...
    if (((outTail + 1) % OUT_BUFFER_CAPACITY) != outHead)
    {
        // Add the character at the tail, and increment the tail position
        outBuffer[outTail] = v;
        outTail = ((outTail + 1) % OUT_BUFFER_CAPACITY);
    }
}


// USB-specific get char
int usb_getchar(void)
{
    int v = -1;
    // If the buffer is not empty...
    if (inHead != inTail)
    {
        // Remove the head item and increment the head
        v = inBuffer[inHead];
        inHead = ((inHead + 1) % IN_BUFFER_CAPACITY);
    }
    return v;
}

// Waits processing IO until buffer is empty
void USBCDCWait(void)
{
	// While configured and not suspended and buffer not empty, spin calling process I/O
    while ((USBDeviceState >= CONFIGURED_STATE) && (USBSuspendControl != 1) && outHead != outTail)
    {
		#ifndef USB_INTERRUPT
        USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
		#endif
		USBProcessIO();
    }
}



// Perform USB Serial I/O tasks -- must be called periodically from ProcessIO() to empty the out-buffer and fill the in- buffers
void USBSerialIO(void)
{
	unsigned char *buffer;
    unsigned char maxLength;
	unsigned char numBytes;

	// Copy recieved bytes to inBuffer
	GetCDCBytesToCircularBuffer();
    
    // Transmit USB CDC data
    buffer = outBuffer + outHead;
    maxLength = (outTail >= outHead) ? (outTail - outHead) : (OUT_BUFFER_CAPACITY - outHead);
    
    if (maxLength > 0)
    {
        if (USBUSARTIsTxTrfReady())
        {
            putUSBUSART((char *)buffer, maxLength);
            numBytes = maxLength;   // We have to assume that they were all written
            if (numBytes > 0)
            {
                outHead = (outHead + numBytes) % OUT_BUFFER_CAPACITY;
            }
        }
    }
    
}


// GetCDCBytesToCircularBuffer() replaces getsUSBUSART()
unsigned char GetCDCBytesToCircularBuffer(void)
{
    unsigned char maxLength;
    unsigned char cdc_rx_len = 0;

	/* Find space in buffer */
    //maxLength = (inTail >= inHead) ? (IN_BUFFER_CAPACITY - inTail - (inHead == 0 ? 1 : 0)) : (inHead - 1 - inTail);
	if (inTail >= inHead)
		{maxLength = IN_BUFFER_CAPACITY + inHead - inTail - 1;}   
	else
		{maxLength = inHead - inTail - 1;}

	if ((maxLength > 0)&&(!USBHandleBusy(CDCDataOutHandle))) 	// Is there any room in the buffer and is there any in-data
    {
		unsigned char len = USBHandleGetLength(CDCDataOutHandle);
		if (maxLength>len)	// Is there sufficient room for the new data
		{
			// Copy data from dual-ram buffer to user's circular buffer
        	for(cdc_rx_len = 0; cdc_rx_len < len; cdc_rx_len++)
		    {
				inBuffer[inTail] = cdc_data_rx[cdc_rx_len];
				inTail = (inTail+1) % IN_BUFFER_CAPACITY;
			}   
	        // Prepare dual-ram buffer for next out transaction
	        CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(BYTE*)&cdc_data_rx,sizeof(cdc_data_rx));
		}
	}
	/*Return written bytes*/
	return cdc_rx_len;

}


// USBProcessIO()
void USBProcessIO(void)
{   
	if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;

	if (1)
	{
	    USBSerialIO();
	    CDCTxService();
	}
    MSDTasks();    
}



#if defined(USB_CDC_SET_LINE_CODING_HANDLER)
// mySetLineCodingHandler() - SetLineCoding command sent to change the baud rate.
void mySetLineCodingHandler(void)
{
    if (cdc_notice.GetLineCoding.dwDTERate.Val > 115200)
    {
        // (Optionally) stall to signal an unsupported baud rate (only some applications handle this).
        //USBStallEndpoint(0,1);
    }
    else
    {
        // Update the CDC driver's baud rate
        CDCSetBaudRate(cdc_notice.GetLineCoding.dwDTERate.Val);
    }
}
#endif


// USBCBSuspend(void) -- Suspend USB
void USBCBSuspend(void)
{
    ; // Do nothing
}


// USBCBWakeFromSuspend -- Wakeup from USB suspend
void USBCBWakeFromSuspend(void)
{
	; // (Undo anything done in USBCBSuspend() -- Do nothing
}


// USBCB_SOF_Handler() -- SOF packet sent every 1 ms.
void USBCB_SOF_Handler(void)
{
    ; // (Caller handles everything)
}


// USBCBErrorHandler() -- USB error handler for debugging
void USBCBErrorHandler(void)
{
    ; // (Caller handles everything)
}


// USBCBCheckOtherReq() -- Handle class-specific requests.
void USBCBCheckOtherReq(void)
{
    USBCheckMSDRequest();
    USBCheckCDCRequest();
}


// USBCBStdSetDscHandler() -- Called on a setup SET_DESCRIPTOR request.
void USBCBStdSetDscHandler(void)
{
    // Claim session ownership if this request is supported
}


// USBCBInitEP() - Initialize the endpoints (host sent a SET_CONFIGURATION).
void USBCBInitEP(void)
{
    #if (MSD_DATA_IN_EP == MSD_DATA_OUT_EP)
        USBEnableEndpoint(MSD_DATA_IN_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    #else
        USBEnableEndpoint(MSD_DATA_IN_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
        USBEnableEndpoint(MSD_DATA_OUT_EP,USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    #endif
    USBMSDInit();
    CDCInitEP();
}


// USBCBSendResume -- Callback to send USB signal to wake host (signal resume for 1-13 msec).
void USBCBSendResume(void)
{
    unsigned short delay;
    USBResumeControl = 1;
    for (delay = 1800; delay != 0; delay--) { Nop(); }
    USBResumeControl = 0;
}


// USER_USB_CALLBACK_EVENT_HANDLER() -- Dispatches USB events (in an interrupt when USB_INTERRUPT defined) 
BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
    switch(event)
    {
        case EVENT_CONFIGURED:      USBCBInitEP();              break;
        case EVENT_SET_DESCRIPTOR:  USBCBStdSetDscHandler();    break;
        case EVENT_EP0_REQUEST:     USBCBCheckOtherReq();       break;
        case EVENT_SOF:             USBCB_SOF_Handler();        break;
        case EVENT_SUSPEND:         USBCBSuspend();             break;
        case EVENT_RESUME:          USBCBWakeFromSuspend();     break;
        case EVENT_BUS_ERROR:       USBCBErrorHandler();        break;
        case EVENT_TRANSFER:        Nop();                      break;
        default:                                                break;
    }      
    return TRUE; 
}

