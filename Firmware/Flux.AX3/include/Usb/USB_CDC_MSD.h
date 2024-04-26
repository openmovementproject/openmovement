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

// USB CDC and MSD handler
// Dan Jackson, Karim Ladha, 2010-2012.

#ifndef USB_MSD_CDC_H
#define USB_MSD_CDC_H

#include "USB/usb.h"

void usb_putchar(unsigned char);
int usb_getchar(void);
char usb_haschar(void);
void usb_write(const void *buffer, unsigned int len);

char USBCDCBusy(void);
void USBCDCWait(void);  // wait until empty
//void USBCDCWaitNotFull(void);   // wait until not full
void USBProcessIO(void);
void USBSerialIO(void);
unsigned char GetCDCBytesToCircularBuffer(void);
void USBInitializeSystem(void);

#if defined(USB_CDC_SET_LINE_CODING_HANDLER)
extern void mySetLineCodingHandler(void);
#endif
//void USBDeviceTasks(void);
void USBCBWakeFromSuspend(void);
void USBCB_SOF_Handler(void);
void USBCBErrorHandler(void);
void USBCBCheckOtherReq(void);
void USBCBStdSetDscHandler(void);
void USBCBInitEP(void);
void USBCBSendResume(void);
void USBCBSuspend(void);
BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size);
#define BlinkUSBStatus()

#if 0 // use following for isr - see .c file
extern void __attribute__ ((interrupt)) _USB1Interrupt(void)
#endif


#endif   
