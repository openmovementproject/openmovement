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

// USB CDC and MSD I/O
// Karim Ladha, Dan Jackson, 2011-2012

#include "HardwareProfile.h"
//#include "Delays.h"
#include "USB/USB.h"
#include "Usb/USB_CDC_MSD.h"
#include "usb_config.h"
#ifdef USB_USE_MSD
	#include "FSconfig.h"
	#include "./USB/usb_function_msd.h"
	#ifdef USE_INTERNAL_FLASH
		#include "MDD File System/internal flash.h"
	#endif
	#ifdef USE_SD_INTERFACE_WITH_SPI
		#include "MDD File System/SD-SPI.h"
	#endif
#endif
#ifdef USB_USE_CDC
	#include "./USB/usb_function_cdc.h"
#endif
#include <TimeDelay.h>

// Globals
#ifdef USB_USE_CDC
	// USB circular buffer head/tail pointers
	volatile unsigned short inHead = 0, inTail = 0;
	volatile unsigned short outHead = 0, outTail = 0;
	
#ifndef OWN_CDC_BUFFER_SIZES
#if !defined(IN_BUFFER_CAPACITY) || !defined(OUT_BUFFER_CAPACITY)
	#warning "Using default buffer sizes"
	#define IN_BUFFER_CAPACITY 128		// Must be 128 for legacy projects
	#define OUT_BUFFER_CAPACITY 256		// Must be 256 for legacy projects
#endif
#endif

	#if __DEBUG
	// These could be static - in debug it helps to see them (retrieve things not sent out on cdc)
	volatile unsigned char inBuffer[IN_BUFFER_CAPACITY];
	volatile unsigned char outBuffer[OUT_BUFFER_CAPACITY];
	#else
	static volatile unsigned char inBuffer[IN_BUFFER_CAPACITY];
	static volatile unsigned char outBuffer[OUT_BUFFER_CAPACITY];
	#endif
	
	// Allows manipulation of endpoint and buffer
	extern USB_HANDLE CDCDataOutHandle;
	extern USB_HANDLE CDCDataInHandle;
	extern volatile FAR unsigned char cdc_data_rx[CDC_DATA_OUT_EP_SIZE];
	extern volatile FAR unsigned char cdc_data_tx[CDC_DATA_IN_EP_SIZE];
#endif

#ifdef USB_USE_MSD
	#if defined(__C30__) || defined(__C32__)
	// The LUN variable definition for the MSD driver.  
	LUN_FUNCTIONS LUN[MAX_LUN + 1] = 
	{
	    {
	        &MDD_USB_MediaInitialize,
	        &MDD_USB_ReadCapacity,
	        &MDD_USB_ReadSectorSize,
	        &MDD_USB_MediaDetect,
	        &MDD_USB_SectorRead,
	        &MDD_USB_WriteProtectState,
	        &MDD_USB_SectorWrite
	    }
	};
	#endif
	
	#ifndef HAVE_OWN_ENQUIRY_RESPONCE
	/* Standard Response to INQUIRY command stored in ROM 	*/
	const ROM InquiryResponse inq_resp = {
	0x00,		// peripheral device is connected, direct access block device
	0x80,           // removable
	0x04,	 	// version = 00=> does not conform to any standard, 4=> SPC-2
	0x02,		// response is in format specified by SPC-2
	0x20,		// n-4 = 36-4=32= 0x20
	0x00,		// sccs etc.
	0x00,		// bque=1 and cmdque=0,indicates simple queueing 00 is obsolete,
			// but as in case of other device, we are just using 00
	0x00,		// 00 obsolete, 0x80 for basic task queueing
	{'M','i','c','r','o','c','h','p'
    },
	// this is the T10 assigned Vendor ID
	{'M','a','s','s',' ','S','t','o','r','a','g','e',' ',' ',' ',' '
    },
	{'0','0','0','1'
    }
	};
	#endif
#endif

#pragma code

// Repurposed - sort out you own oscillator stuff prior to this
void USBInitializeSystem(void)
{
#ifdef USB_USE_CDC
    // Clear USB CDC I/O buffers
    inHead = 0; inTail = 0;
    outHead = 0; outTail = 0;
#endif

	UsbInitDescriptors(); // Does nothing if descriptors are static

	// If the module is already running (e.g. after bootloader), then hard detatch
	if (U1CONbits.USBEN)
	{
		U1CONbits.USBEN = 0;
	    DelayMs(4 * 4); // Wait >2ms before re-attach (*4 as PLL is on)
	}

    USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
    					//variables to known states.
}

#ifdef USB_USE_CDC
	
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

	// USB-specific char ready
	char usb_haschar(void)
	{
	    return (inHead != inTail);
	}
	
	// write() redirect for USB
	void usb_write(const void *buffer, unsigned int len)
	{

		// LATER: Make this perform a faster than one byte at at time
		unsigned int i;
        const char *p = (const char *)buffer;
	    for (i = len; i; --i)
	    {
//KL: To prevent failures when printing from interrupts use this define
#ifdef LOCK_PRINTF_USBCALLS_IN_INTERRUPTS_ON_WRITE
if(SRbits.IPL ==0){
#endif
            // If full
            int deadlock = 0;
            while (((outTail + 1) % OUT_BUFFER_CAPACITY) == outHead)
            {
                // While configured and not suspended and buffer not empty, spin calling process I/O
                if (!USB_BUS_SENSE || (USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1)) break;
                #ifndef USB_INTERRUPT
                USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
                #endif
                USBProcessIO();

                // Is it potentially dead-locked?
                if (USBHandleBusy(CDCDataOutHandle) && cdc_trf_state != CDC_TX_READY)
                {
                    if (deadlock++ > 10) { break; }
                    if (deadlock > 1) Delay10us(50);
                }
            }
#ifdef LOCK_PRINTF_USBCALLS_IN_INTERRUPTS_ON_WRITE
}
#endif
            // Write a byte
	        outBuffer[outTail] = *p++;
	        outTail = ((outTail + 1) % OUT_BUFFER_CAPACITY);

	    }
	}
	
	// Identifies if output buffer has data
	char USBCDCBusy(void)
	{
	    return (USB_BUS_SENSE && (USBDeviceState >= CONFIGURED_STATE) && (USBSuspendControl != 1) && outHead != outTail);
    }

    /*
	// Waits processing IO until buffer is not-full
	void USBCDCWaitNotFull(void)
	{
//int timeout = 1000;
		// While configured and not suspended and buffer not empty, spin calling process I/O
        while (USB_BUS_SENSE && (USBDeviceState >= CONFIGURED_STATE) && (USBSuspendControl != 1) && (((outTail + 1) % OUT_BUFFER_CAPACITY) == outHead))	//     // Prevent lockup?  && inHead == inTail
        {
            #ifndef USB_INTERRUPT
            USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
            #endif
            USBProcessIO();
//if (!timeout--) { break; }
//DelayMs(1);
        }
	}
    */
	
	// Waits processing IO until buffer is empty
	void USBCDCWait(void)
	{
        int deadlock = 0;
		// While configured and not suspended and buffer not empty, spin calling process I/O
        while (USB_BUS_SENSE && (USBDeviceState >= CONFIGURED_STATE) && (USBSuspendControl != 1) && outHead != outTail)	//     /* Prevent lockup? */ && inHead == inTail
        {
            // Is it potentially dead-locked?
            if (USBHandleBusy(CDCDataOutHandle) && cdc_trf_state != CDC_TX_READY)
            {
                if (deadlock++ > 10) 
				{ 
					Nop();
					break; 
				}
                if (deadlock > 1) Delay10us(50);
            }

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
	    unsigned short maxOutLength;
		unsigned short numBytes;
	
		// Copy recieved bytes to inBuffer
		GetCDCBytesToCircularBuffer();
	    
	    // Transmit USB CDC data
	    buffer = (unsigned char *)outBuffer + outHead;
	    maxOutLength = (outTail >= outHead) ? (outTail - outHead) : (OUT_BUFFER_CAPACITY - outHead);
	    
	    if (maxOutLength > 0)
	    {
	        if (USBUSARTIsTxTrfReady())
	        {
if (maxOutLength > 64) { maxOutLength = 64; }       // needed to fix data loss
	            putUSBUSART((char *)buffer, maxOutLength);
	            numBytes = maxOutLength;   // We have to assume that they were all written
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
	    unsigned short maxInLength;
	    unsigned short cdc_rx_len = 0;
	
		/* Find space in buffer */
	    //maxInLength = (inTail >= inHead) ? (IN_BUFFER_CAPACITY - inTail - (inHead == 0 ? 1 : 0)) : (inHead - 1 - inTail);
		if (inTail >= inHead)
			{maxInLength = IN_BUFFER_CAPACITY + inHead - inTail - 1;}   
		else
			{maxInLength = inHead - inTail - 1;}
	
//if (maxInLength > 0)	// Allow overflow of incoming buffer if not processed in time	
		if ((!USBHandleBusy(CDCDataOutHandle))) 	// Is there any room in the buffer and is there any in-data
	    {
			unsigned short len = USBHandleGetLength(CDCDataOutHandle);
			
			// Truncate at the maximum buffer capacity (circular buffer implementation has a capacity one less than the buffer size)
			if (len >= IN_BUFFER_CAPACITY - 1)
			{
				len = IN_BUFFER_CAPACITY - 1;
			}

// Truncate
if (len > maxInLength)
{
	len = maxInLength;
}
			
			// If there is sufficient room for the new data
			if (len <= maxInLength)	
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
#endif


// USBProcessIO()
void USBProcessIO(void)
{   
	if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;
	#ifdef USB_USE_CDC
	    USBSerialIO();
	    CDCTxService();
	#endif

	#ifdef USB_USE_MSD
    	MSDTasks();  
	#endif 
}

#ifdef USB_USE_CDC
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
	#ifdef USB_USE_MSD
    	USBCheckMSDRequest();
	#endif
	#ifdef USB_USE_CDC
    	USBCheckCDCRequest();
	#endif
}


// USBCBStdSetDscHandler() -- Called on a setup SET_DESCRIPTOR request.
void USBCBStdSetDscHandler(void)
{
    // Claim session ownership if this request is supported
}


// USBCBInitEP() - Initialize the endpoints (host sent a SET_CONFIGURATION).
void USBCBInitEP(void)
{
	#ifdef USB_USE_MSD
	    #if (MSD_DATA_IN_EP == MSD_DATA_OUT_EP)
	        USBEnableEndpoint(MSD_DATA_IN_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
	    #else
	        USBEnableEndpoint(MSD_DATA_IN_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
	        USBEnableEndpoint(MSD_DATA_OUT_EP,USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
	    #endif
	    USBMSDInit();
	#endif
	#ifdef USB_USE_CDC
    	CDCInitEP();
	#endif
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

//EOF
