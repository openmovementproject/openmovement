// usb_cdc.c - USB CDC device code.
// Portions derived from 'main.c' (CDC Basic Demo) Copyright Microchip Technology (see below).
// Changes Copyright Newcastle University (see below).

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

/*
 FileName:      main.c
 Dependencies:  See INCLUDES section
 Processor:		PIC18, PIC24, and PIC32 USB Microcontrollers
 Hardware:		This demo is natively intended to be used on Microchip USB demo
 				boards supported by the MCHPFSUSB stack.  See release notes for
 				support matrix.  This demo can be modified for use on other hardware
 				platforms.
 Complier:  	Microchip C18 (for PIC18), C30 (for PIC24), C32 (for PIC32)
 Company:		Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the “Company”) for its PIC® Microcontroller is intended and
 supplied to you, the Company’s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*/

// Includes
#include "USB/USB.h"
#include "HardwareProfile.h"
#include <stdio.h>
#include <delays.h>
#include "USB/usb_function_cdc.h"
//#include "util.h"
#include "usb_cdc.h"


// Variables
#pragma idata

static char usbDebugStatus = 0;

// USB circular buffer head/tail pointers
unsigned char inHead = 0, inTail = 0;
unsigned char outHead = 0, outTail = 0;


#pragma udata

// USB circular buffer sizes
#if (DEVICE_TYPE == 0)				// 0 = FFD Attached receiver
    #define IN_BUFFER_CAPACITY_POWER  8     // 2^8 = 256
    #define OUT_BUFFER_CAPACITY_POWER 8     // 2^8 = 256
#else                               //     RFD Transmitter
    #define IN_BUFFER_CAPACITY_POWER  6     // 2^6 = 64
    #define OUT_BUFFER_CAPACITY_POWER 6     // 2^6 = 64
#endif


// USB circular buffers
#define IN_BUFFER_CAPACITY (((int)1)<<IN_BUFFER_CAPACITY_POWER)
#define OUT_BUFFER_CAPACITY (((int)1)<<OUT_BUFFER_CAPACITY_POWER)
#define OUT_MASK(_i) ((_i) & (unsigned char)(OUT_BUFFER_CAPACITY - 1))
#define IN_MASK(_i)  ((_i) & (unsigned char)(IN_BUFFER_CAPACITY - 1))
static unsigned char inBuffer[IN_BUFFER_CAPACITY];
static unsigned char outBuffer[OUT_BUFFER_CAPACITY];


// Allows manipulation of endpoint and buffer
extern USB_HANDLE CDCDataOutHandle;
extern USB_HANDLE CDCDataInHandle;
extern volatile FAR unsigned char cdc_data_rx[CDC_DATA_OUT_EP_SIZE];
extern volatile FAR unsigned char cdc_data_tx[CDC_DATA_IN_EP_SIZE];


// Private prototypes
//void USBDeviceTasks(void);
void USBDebugStatus(void);


// Declarations
#pragma code

/*
// Circular buffer operations (b = buffer, h = head, t = tail, i = index / number of elements, c = capacity)
static unsigned char _buffer_temp;
#define BUFFER_CLEAR(_h, _t) { _h = 0; _t = 0; }
#define BUFFER_INCREMENT(_i, _c) (((_i) + 1) % (_c))
#define BUFFER_ADD(_b, _h, _t, _c, _v) { if (BUFFER_INCREMENT(_t, _c) != (_h)) { (_b)[_t] = (_v); _t = BUFFER_INCREMENT(_t, _c); } }
#define BUFFER_REMOVE(_b, _h, _t, _c) (((_h) == (_t)) ? 0 : (_buffer_temp = (_b)[_h], _h = BUFFER_INCREMENT(_h, _c), _buffer_temp))
#define BUFFER_LENGTH(_h, _t, _c) (((_t) >= (_h)) ? ((_t) - (_h)) : ((_c) - (_h) + (_t)))
#define BUFFER_FREE(_h, _t, _c) (((_t) >= (_h)) ? ((_c) - (_t) + (_h) - 1) : ((_h) - (_t) - 1))
#define BUFFER_CONTIGUOUS_ENTRIES(_h, _t, _c) (((_t) >= (_h)) ? ((_t) - (_h)) : ((_c) - (_h)))
#define BUFFER_ENTRY_POINTER(_b, _h) ((_b) + (_h))
#define BUFFER_CONTIGUOUS_FREE(_h, _t, _c) (((_t) >= (_h)) ? ((_c) - (_t) - ((_h) == 0 ? 1 : 0)) : ((_h) - (_t) - 1))
#define BUFFER_FREE_POINTER(_b, _t) ((_b) + (_t))
#define BUFFER_EXTERNALLY_ADDED(_t, _c, _i) ((_t) = (((_t) + (_i)) % (_c)))
#define BUFFER_EXTERNALLY_REMOVED(_h, _c, _i) ((_h) = (((_h) + (_i)) % (_c)))
*/

// InitializeSystem - Centralized initialization routine.
void USBInitializeSystem(void)
{
    CLOCK_USB_XTAL();            // Initialize HS crystal and PLL for 48 MHz USB module to run at full-speed

#ifdef USE_USB_BUS_SENSE_IO
    USB_BUS_SENSE_TRIS = 1;
#endif
    
    // Clear USB CDC I/O buffers
	stdout = _H_USER;
    inHead = 0; inTail = 0;
    outHead = 0; outTail = 0;

    USBDeviceInit();    //usb_device.c.  Initializes USB module SFRs and firmware variables to known states.
}


// USB-specific put char
void usb_putchar(unsigned char v)
{
    static unsigned char newOutTail;
    // Find new tail position
    newOutTail = OUT_MASK(outTail + 1);

    // If the queue is not full...
    if (newOutTail != outHead)
    {
        // Add the character at the tail, and increment the tail position
        outBuffer[outTail] = v;
        outTail = newOutTail;
    }
}


// USB-specific put string
void usb_puts(const char *v)
{
    static unsigned char newOutTail;
	while (*v != '\0')
	{
	    newOutTail = OUT_MASK(outTail + 1);
	    if (newOutTail == outHead) { break; }
	    outBuffer[outTail] = *v++;
	    outTail = newOutTail;
	}
}


// USB-specific put string
void usb_putrs(const rom far char *v)
{
    static unsigned char newOutTail;
	while (*v != '\0')
	{
	    newOutTail = OUT_MASK(outTail + 1);
	    if (newOutTail == outHead) { break; }
	    outBuffer[outTail] = *v++;
	    outTail = newOutTail;
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
        inHead = IN_MASK(inHead + 1);
    }
    return v;
}


// USB write
unsigned char usb_write(void *buffer, unsigned char length)
{
    unsigned char *sp;                          // source pointer
    unsigned char *dp;                          // destination pointer
    int contiguous;                             // number of contiguous entries that could be written
    unsigned char remaining;                    // number of bytes left to write

    sp = (unsigned char *)buffer;               // source pointer at start of buffer
    remaining = length;                         // all buffer bytes still to write
    while (remaining > 0)                       // while there are still bytes to write...
    {
        dp = outBuffer + outTail;               // calculate destination pointer for this contiguous stretch; then calculate contiguous size
        contiguous = ((outTail >= outHead) ? (OUT_BUFFER_CAPACITY - outTail - (outHead == 0 ? 1 : 0)) : (outHead - outTail - 1));
        if (contiguous == 0) { break; }         // destination at capacity, cannot send any more, abort loop
        if (contiguous > remaining) { contiguous = remaining; } // clamp to number of bytes we'd actually want to add anyway
        remaining -= contiguous;                // adjust remaining count
        outTail = OUT_MASK(outTail + contiguous); // adjust tail pointer
        for(; contiguous > 0; contiguous--)     // while there are bytes to write
        {
            *dp++ = *sp++;                      // write byte, increment pointers
        }
    }
    return (length - remaining);                // return number of bytes written
}


// USB write ROM
unsigned char usb_writer(const rom far void *buffer, unsigned char length)
{
    const rom far unsigned char *sp;            // source pointer
    unsigned char *dp;                          // destination pointer
    int contiguous;                             // number of contiguous entries that could be written
    unsigned char remaining;                    // number of bytes left to write

    sp = (const rom far unsigned char *)buffer;	// source pointer at start of buffer
    remaining = length;                         // all buffer bytes still to write
    while (remaining > 0)                       // while there are still bytes to write...
    {
        dp = outBuffer + outTail;               // calculate destination pointer for this contiguous stretch; then calculate contiguous size
        contiguous = ((outTail >= outHead) ? (OUT_BUFFER_CAPACITY - outTail - (outHead == 0 ? 1 : 0)) : (outHead - outTail - 1));
        if (contiguous == 0) { break; }         // destination at capacity, cannot send any more, abort loop
        if (contiguous > remaining) { contiguous = remaining; } // clamp to number of bytes we'd actually want to add anyway
        remaining -= contiguous;                // adjust remaining count
        outTail = OUT_MASK(outTail + contiguous); // adjust tail pointer
        for(; contiguous > 0; contiguous--)     // while there are bytes to write
        {
            *dp++ = *sp++;                      // write byte, increment pointers
        }
    }
    return (length - remaining);                // return number of bytes written
}


// Buffer space free
int usbOutFree(void)
{
	return (outTail >= outHead) ? (OUT_BUFFER_CAPACITY + outHead - outTail - 1) : (outHead - 1 - outTail);
}


// USB write SLIP (RFC 1055) encoded data
unsigned char usb_slip_encoded(void *buffer, unsigned char length)
{
#if 1       // Faster simple version
    static unsigned char len;
    static unsigned char *sp;
    static unsigned char c;
    static unsigned char newOutTail;

    sp = buffer;
    len = length;
    while (len--)
    {
        c = *sp++;
        switch (c)
        {
            case SLIP_END:
                // Terminate if the queue is full, otherwise add the ESC character at the tail, and increment the tail position
                newOutTail = OUT_MASK(outTail + 1);
                if (newOutTail == outHead) { break; }    
                outBuffer[outTail] = SLIP_ESC;
                outTail = newOutTail; 
    
                // Escaped character code
                c = SLIP_ESC_END;
                break;

            case SLIP_ESC:
                // Terminate if the queue is full, otherwise add the ESC character at the tail, and increment the tail position
                newOutTail = OUT_MASK(outTail + 1);
                if (newOutTail == outHead) { break; }    
                outBuffer[outTail] = SLIP_ESC;
                outTail = newOutTail; 
    
                // Escaped character code
                c = SLIP_ESC_ESC;
                break;
        }

        // Terminate if the queue is full, otherwise add the character at the tail, and increment the tail position
        newOutTail = OUT_MASK(outTail + 1);
        if (newOutTail == outHead) { break; }    
        outBuffer[outTail] = c;
        outTail = newOutTail; 
    }
    return (length - len);
#elif 1       // Simpler version
    unsigned char len;
    unsigned char *sp;

    sp = buffer;
    for (len = length; len != 0; len--)
    {
        switch (*sp)
        {
            case SLIP_END:
                usb_putchar(SLIP_ESC);
                usb_putchar(SLIP_ESC_END);
                break;
            case SLIP_ESC:
                usb_putchar(SLIP_ESC);
                usb_putchar(SLIP_ESC_ESC);
                break;
            default:
                usb_putchar(*sp);
        }
        ++sp;
    }
    return length;
#else       // Attempt at being faster, but probably isn't and is more complicated
    unsigned char *sp;                          // source pointer
    unsigned char *dp;                          // destination pointer
    int contiguous;                             // number of contiguous entries that could be written
    unsigned char remaining;                    // number of bytes left to write
    unsigned char count;                        // count of buffer bytes sent in each contiguous burst
    char lastEscape;                            // flag to indicate last written char was an escape
    unsigned char c;                            // byte being processed

    sp = (unsigned char *)buffer;               // source pointer at start of buffer
    remaining = length;                         // all buffer bytes still to write
    lastEscape = 0;                             // clear 'last char was escape' flag
    while (remaining > 0)                       // while there are still bytes to write...
    {
        dp = outBuffer + outTail;               // calculate destination pointer for this contiguous stretch; then calculate contiguous size
        contiguous = ((outTail >= outHead) ? (OUT_BUFFER_CAPACITY - outTail - (outHead == 0 ? 1 : 0)) : (outHead - outTail - 1));
        if (contiguous == 0) { break; }         // destination at capacity, cannot send any more, abort loop
        count = 0;                              // clear count of written bytes
        do                                      // loop
        {
            c = *sp;                            // read from source

            // Substitute the escaped characters (or write ESC if not yet written)
            if      (c == SLIP_END) { c = lastEscape ? SLIP_ESC_END : SLIP_ESC; }
            else if (c == SLIP_ESC) { c = lastEscape ? SLIP_ESC_ESC : SLIP_ESC; }

            *dp++ = c;                          // write to destination
            --contiguous;                       // one less byte in capacity

            if (c == SLIP_ESC)
            {
                lastEscape = 1;                 // set 'last char was escape' flag
            }
            else
            {
                lastEscape = 0;                 // clear 'last char was escape' flag
                sp++;                           // consume this character: increment source pointer
                count++;                        // consume this character: increment consumed count
            }

        } while (contiguous && count < remaining); // while still bytes to read and capacity to write
        remaining -= count;
        outTail = OUT_MASK(outTail + count);
    }
    return (length - remaining);
#endif
}


// USB write SLIP (RFC 1055) encoded data from ROM
void usb_slip_encoded_rom(const rom far char *buffer, unsigned char length)
{
    for (; length != 0; length--)
    {
        switch (*buffer)
        {
            case SLIP_END:
                usb_putchar(SLIP_ESC);
                usb_putchar(SLIP_ESC_END);
                break;
            case SLIP_ESC:
                usb_putchar(SLIP_ESC);
                usb_putchar(SLIP_ESC_ESC);
                break;
            default:
                usb_putchar(*buffer);
        }
        ++buffer;
    }
    return;
}



// Perform USB Serial I/O tasks -- must be called periodically from ProcessIO() to empty the out-buffer and fill the in- buffers
void USBSerialIO(void)
{
// [Old patch fix]
//#define PATCH_FIX
//#ifdef PATCH_FIX
//	#define SCRATCH_SIZE 64
//	static char scratchBuffer[SCRATCH_SIZE];
//    unsigned char *buffer;
//    unsigned char maxLength;
//    unsigned char numBytes;
//
//    // Total bytes free (not just contiguous)
//    maxLength = IN_BUFFER_CAPACITY - 1 - ((inTail >= inHead) ? (inTail - inHead) : (inHead - inTail));
//    if (maxLength >= SCRATCH_SIZE)
//    {
//        numBytes = getsUSBUSART((char *)scratchBuffer, SCRATCH_SIZE);
//        if (numBytes > 0)
//        {
//			// Contiguous bytes free in buffer
//			maxLength = (inTail >= inHead) ? (IN_BUFFER_CAPACITY - inTail - 1) : (inHead - 1 - inTail);
//			if (maxLength > numBytes) { maxLength = numBytes; };
//			memcpy(inBuffer + inTail, scratchBuffer, maxLength);
//            inTail = (inTail + maxLength) % IN_BUFFER_CAPACITY;
//			// Attempt to read more
//			if (inTail == 0 && inHead > 1)
//			{
//				memcpy(inBuffer + inTail, scratchBuffer + maxLength, numBytes - maxLength);
//				inTail = numBytes - maxLength;
//			}
//        }
//	}
//
//
//#endif

	unsigned char *buffer;
    unsigned char maxLength;
	unsigned char numBytes;

	// Find space in buffer
    maxLength = (inTail >= inHead) ? (IN_BUFFER_CAPACITY + inHead - inTail - 1) : (inHead - 1 - inTail);
	if ((maxLength > 0) && (!USBHandleBusy(CDCDataOutHandle))) 	// Is there any room in the buffer and is there any in-data
    {
		unsigned char cdc_rx_len = 0;
		unsigned char len = USBHandleGetLength(CDCDataOutHandle);
		if (maxLength > len)	// Is there sufficient room for the new data
		{
			// Copy data from dual-ram buffer to user's circular buffer
        	for (cdc_rx_len = 0; cdc_rx_len < len; cdc_rx_len++)
		    {
				inBuffer[inTail] = cdc_data_rx[cdc_rx_len];
				inTail = (inTail+1) % IN_BUFFER_CAPACITY;
			}
	        // Prepare dual-ram buffer for next OUT transaction
	        CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(BYTE*)&cdc_data_rx,sizeof(cdc_data_rx));
		}
	}
	
    
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



// USBProcessIO - User routines
void USBProcessIO(void)
{   
    // Blink the LEDs according to the USB device status
    if(usbDebugStatus)
    {
        USBDebugStatus();
    }

    if ((USBDeviceState >= CONFIGURED_STATE) && (USBSuspendControl != 1))
    {
        USBSerialIO();
        CDCTxService();
    } else {
    	outHead = outTail;
    }
}


// USBDebugStatus - turns on and off LEDs corresponding to the USB device state.
void USBDebugStatus(void)
{
#if 1
    static unsigned short activity = 0;
    static unsigned short counter = 0;
    static unsigned long lastReads = 0;
    char color;
    char flash;

    if (counter == 0) { counter = 20000U; } else { counter--; }
    flash = (counter < 5000) ? 1 : 0;
#ifdef BATT_NCHARGING
    color = BATT_NCHARGING ? 2 : 3;                                     // Charge indicator: GREEN = not charging (full); YELLOW = charging (not full)
#else
color = 3;
#endif

    if (!USBSuspendControl && USBDeviceState == ADDRESS_STATE)          // Address (~2-3 seconds when connecting)
    {
        color = (flash ? color : 4);                                    // Flash BLUE then (YELLOW|GREEN)
    }
    else if (!USBSuspendControl && USBDeviceState == CONFIGURED_STATE)  // Configured
    {
        color = (flash ? color : 7);                                    // Flash WHITE then (YELLOW|GREEN)
    }
    else
    {
        color = color;                                                  // YELLOW/GREEN = charge indicator
    }



    if (activity > 0) { activity--; }
    // TODO: Update 'activity' on activity
    if (activity && ((counter >> 9) & 1))
    {
        LED_SET(0);
    }
    else
    {
        LED_SET(color);
    }


#else
    static WORD led_count = 0;
    static char flash = 0;
    if (led_count == 0) { led_count = 10000U; flash = !flash; }
    led_count--;
    if      (USBSuspendControl)                                        { LED_SET(flash ? 3 : 0); }  // Flash YELLOW/OFF = suspend (~0.5 second once disconnected), when connected to a powered hub (previously fully connected)
    else if (!USBSuspendControl && USBDeviceState == DETACHED_STATE)   { LED_SET(4); }              // BLUE   = detached (when disconnected)
    else if (!USBSuspendControl && USBDeviceState == ATTACHED_STATE)   { LED_SET(3); }              // YELLOW = attached (briefly when connecting??)
    else if (!USBSuspendControl && USBDeviceState == POWERED_STATE)    { LED_SET(1); }              // RED    = powered  (briefly when connecting??)
    else if (!USBSuspendControl && USBDeviceState == DEFAULT_STATE)    { LED_SET(2); }              // GREEN  = default (briefly when connecting), when only connected to a powered hub (never connected)
    else if (!USBSuspendControl && USBDeviceState == ADDRESS_STATE)    { LED_SET(flash ? 1 : 0); }  // Flash RED/OFF = address (~2-3 seconds when connecting)
    else if (!USBSuspendControl && USBDeviceState == CONFIGURED_STATE) { LED_SET(flash ? 2 : 1); }  // Flash RED/GREEN = configured
    else                                                               { LED_SET(7); }              // (shouldn't happen)
#endif
}


#if defined(USB_CDC_SET_LINE_CODING_HANDLER)
// USBSetLineCodingHandler - called when a SetLineCoding command is sent on the bus.  
void USBSetLineCodingHandler(void)
{
    // Evaluate the request and determine whether the application should update the baudrate.
    if(cdc_notice.GetLineCoding.dwDTERate.Val > 115200)
    {
        // The request is not in a valid range, unsupported baud rate request
        // Stall the STATUS stage of the request (causes an exception in the requesting application).
        USBStallEndpoint(0, 1);
    }
    else
    {
        // Update the baudrate info in the CDC driver
        CDCSetBaudRate(cdc_notice.GetLineCoding.dwDTERate.Val);
    }
}
#endif


// USB callback functions
//
// The USB firmware stack will call the callback functions USBCBxxx() in response to certain USB related
// events.  For example, if the host PC is powering down, it will stop sending out Start of Frame (SOF)
// packets to your device.  In response to this, all USB devices are supposed to decrease their power
// consumption from the USB Vbus to <2.5mA each.  The USB module detects this condition (which according
// to the USB specifications is 3+ms of no bus activity/SOF packets) and then calls the USBCBSuspend()
// function.  You should modify these callback functions to take appropriate actions for each of these
// conditions.  For example, in the USBCBSuspend(), you may wish to add code that will decrease power
// consumption from Vbus to <2.5mA (such as by clock switching, turning off LEDs, putting the
// microcontroller to sleep, etc.).  Then, in the USBCBWakeFromSuspend() function, you may then wish to
// add code that undoes the power saving things done in the USBCBSuspend() function.
//
// The USBCBSendResume() function is special, in that the USB stack will not automatically call this
// function.  This function is meant to be called from the application firmware instead.  See the
// additional comments near the function.
//


// _USB1Interrupt - USB interrupt called when the USB interrupt bit is set.
// (Currently the interrupt is only used when the device goes to sleep when it receives a USB suspend command).
#if 0
void __attribute__ ((interrupt)) _USB1Interrupt(void)
{
    #if !defined(self_powered)
        if(U1OTGIRbits.ACTVIF)
        {
            IEC5bits.USB1IE = 0;
            U1OTGIEbits.ACTVIE = 0;
            IFS5bits.USB1IF = 0;
        
            //USBClearInterruptFlag(USBActivityIFReg, USBActivityIFBitNum);
            USBClearInterruptFlag(USBIdleIFReg, USBIdleIFBitNum);
            //USBSuspendControl = 0;
        }
    #endif
}
#endif


// USBCBSuspend - USB callback for when a USB suspend is detected.
void USBCBSuspend(void)
{
    //Example power saving code.  Insert appropriate code here for the desired
    //application behavior.  If the microcontroller will be put to sleep, a
    //process similar to that shown below may be used:
    
    //ConfigureIOPinsForLowPower();
    //SaveStateOfAllInterruptEnableBits();
    //DisableAllInterruptEnableBits();
    //EnableOnlyTheInterruptsWhichWillBeUsedToWakeTheMicro();    //should enable at least USBActivityIF as a wake source
    //Sleep();
    //RestoreStateOfAllPreviouslySavedInterruptEnableBits();    //Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.
    //RestoreIOPinsToNormal();                                    //Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.

    //IMPORTANT NOTE: Do not clear the USBActivityIF (ACTVIF) bit here.  This bit is 
    //cleared inside the usb_device.c file.  Clearing USBActivityIF here will cause 
    //things to not work as intended.    

    /*
    #if defined(__C30__)
        U1EIR = 0xFFFF;
        U1IR = 0xFFFF;
        U1OTGIR = 0xFFFF;
        IFS5bits.USB1IF = 0;
        IEC5bits.USB1IE = 1;
        U1OTGIEbits.ACTVIE = 1;
        U1OTGIRbits.ACTVIF = 1;
        Sleep();
    #endif
    */
}


// USBCBWakeFromSuspend - USB callback for wakeup from USB suspend.
void USBCBWakeFromSuspend(void)
{
    // The host may put USB peripheral devices in low power suspend mode (by "sending" 3+ms of idle).
    // Once in suspend mode, the host may wake the device back up by sending non-idle state signalling.
    
    // If clock switching or other power savings measures were taken when
    // executing the USBCBSuspend() function, now would be a good time to
    // switch back to normal full power run mode conditions.  The host allows
    // a few milliseconds of wakeup time, after which the device must be 
    // fully back to normal, and capable of receiving and processing USB
    // packets.  In order to do this, the USB module must receive proper
    // clocking (IE: 48MHz clock must be available to SIE for full speed USB
    // operation).
}


// USBCB_SOF_Handler - USB callback for SOF packet handling (full-speed devices every 1 ms).
void USBCB_SOF_Handler(void)
{
    // The USB host sends out a SOF packet to full-speed devices every 1 ms. 
    // This interrupt may be useful for isochronous pipes.
    
    // No need to clear UIRbits.SOFIF to 0 here (callback caller is already doing that).
}


// USBCBErrorHandler - USB callback for error handler.
void USBCBErrorHandler(void)
{
    // Mainly for debugging during development.
    // Check UEIR to see which error causes the interrupt.
    
    // No need to clear UEIR to 0 here.
    // Callback caller is already doing that.

    // Typically, user firmware does not need to do anything special
    // if a USB error occurs.  For example, if the host sends an OUT
    // packet to your device, but the packet gets corrupted (ex:
    // because of a bad connection, or the user unplugs the
    // USB cable during the transmission) this will typically set
    // one or more USB error interrupt flags.  Nothing specific
    // needs to be done however, since the SIE will automatically
    // send a "NAK" packet to the host.  In response to this, the
    // host will normally retry to send the packet again, and no
    // data loss occurs.  The system will typically recover
    // automatically, without the need for application firmware
    // intervention.
    
    // Nevertheless, this callback function is provided, such as
    // for debugging purposes.
}


// USBCBCheckOtherReq - USB callback to check other setup requests.
void USBCBCheckOtherReq(void)
{
    // When SETUP packets arrive from the host, some
    // firmware must process the request and respond
    // appropriately to fulfill the request.  Some of
    // the SETUP packets will be for standard
    // USB "chapter 9" (as in, fulfilling chapter 9 of
    // the official USB specifications) requests, while
    // others may be specific to the USB device class
    // that is being implemented.  For example, a HID
    // class device needs to be able to respond to
    // "GET REPORT" type of requests.  This
    // is not a standard USB chapter 9 request, and 
    // therefore not handled by usb_device.c.  Instead
    // this request should be handled by class specific 
    // firmware, such as that contained in usb_function_hid.c.
    USBCheckCDCRequest();
}


// USBCBStdSetDscHandler - USB callback for SETUP/SET_DESCRIPTOR request.
void USBCBStdSetDscHandler(void)
{
    // The USBCBStdSetDscHandler() callback function is
    // called when a SETUP, bRequest: SET_DESCRIPTOR request
    // arrives.  Typically SET_DESCRIPTOR requests are
    // not used in most applications, and it is
    // optional to support this type of request.
    
    // Must claim session ownership if supporting this request.
}


// USBCBInitEP - USB callback to initialize endpoints.
void USBCBInitEP(void)
{
    // This function is called when the device becomes
    // initialized, which occurs after the host sends a
    // SET_CONFIGURATION (wValue not = 0) request.  This 
    // callback function should initialize the endpoints 
    // for the device's usage according to the current 
    // configuration.
    CDCInitEP();
}


// USBCBSendResume - USB callback to wake host PC.
void USBCBSendResume(void)
{
    WORD delay_count;
    USBResumeControl = 1;                                       // Start RESUME signaling
    for (delay_count = 1800U; delay_count > 0; delay_count--);    // Set RESUME line for 1-13 ms    
    USBResumeControl = 0;
}


// USER_USB_CALLBACK_EVENT_HANDLER - USB callback event handler.
BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
    // Called from the USB stack to notify a user application that a USB event occured.
    // Switch on the type of event (pdata/size point to the event data).
    switch (event)
    {
        case EVENT_CONFIGURED: 
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER:
            Nop();
            break;
        default:
            break;
    }      
    return TRUE; 
}

