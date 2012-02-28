// usb_config.h - USB Configuration.
// Portions derived from 'usb_config.h' (CDC Basic Demo) Copyright Microchip Technology (see below).
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
 FileName:     	usb_config.h
 Dependencies: 	Always: GenericTypeDefs.h, usb_device.h
               	Situational: usb_function_hid.h, usb_function_cdc.h, usb_function_msd.h, etc.
 Processor:		PIC18 or PIC24 USB Microcontrollers
 Hardware:		The code is natively intended to be used on the following
 				hardware platforms: PICDEM™ FS USB Demo Board, 
 				PIC18F87J50 FS USB Plug-In Module, or
 				Explorer 16 + PIC24 USB PIM.  The firmware may be
 				modified for use on other USB platforms by editing the
 				HardwareProfile.h file.
 Complier:  	Microchip C18 (for PIC18) or C30 (for PIC24)
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

// Descriptor specific type definitions are defined in: usbd.h

#ifndef USBCFG_H
#define USBCFG_H

// Definitions

// EP0 buffer size - Valid Options: 8, 16, 32, or 64 bytes.
// Using larger options take more SRAM, but does not provide much advantage in most types of applications.
// Exceptions to this, are applications that use EP0 IN or OUT for sending large amounts of application related data.
#define USB_EP0_BUFF_SIZE 8
#define USB_MAX_NUM_INT   1   // For tracking Alternate Setting
#define USB_MAX_EP_NUMBER 3

// Device descriptor - if these two definitions are not defined then a ROM USB_DEVICE_DESCRIPTOR variable by the exact name of device_dsc must exist.
#define USB_USER_DEVICE_DESCRIPTOR &device_dsc
#define USB_USER_DEVICE_DESCRIPTOR_INCLUDE extern ROM USB_DEVICE_DESCRIPTOR device_dsc

// Configuration descriptors - if these two definitions do not exist then a ROM BYTE *ROM variable named exactly USB_CD_Ptr[] must exist.
#define USB_USER_CONFIG_DESCRIPTOR USB_CD_Ptr
#define USB_USER_CONFIG_DESCRIPTOR_INCLUDE extern ROM BYTE *ROM USB_CD_Ptr[]

// Make sure only one of the below "#define USB_PING_PONG_MODE" is uncommented.
//#define USB_PING_PONG_MODE USB_PING_PONG__NO_PING_PONG
#define USB_PING_PONG_MODE USB_PING_PONG__FULL_PING_PONG
//#define USB_PING_PONG_MODE USB_PING_PONG__EP0_OUT_ONLY
//#define USB_PING_PONG_MODE USB_PING_PONG__ALL_BUT_EP0

#define USB_POLLING
//#define USB_INTERRUPT

// [dgj] Use low ISR to handle interrupts (USB_INTERRUPT must also be defined)
#define USB_INTERRUPT_LOW
#if defined(USB_INTERRUPT) && defined(USB_INTERRUPT_LOW)
#warning "Something needs changing to make low-priority USB interrupts work with the newer API stack -- USBEnableInterrupts?"
#endif

// Parameter definitions are defined in usb_device.h
#define USB_PULLUP_OPTION USB_PULLUP_ENABLE
//#define USB_PULLUP_OPTION USB_PULLUP_DISABLED

// External Transceiver support is not available on all product families.
#define USB_TRANSCEIVER_OPTION USB_INTERNAL_TRANSCEIVER
//#define USB_TRANSCEIVER_OPTION USB_EXTERNAL_TRANSCEIVER

#define USB_SPEED_OPTION USB_FULL_SPEED
//#define USB_SPEED_OPTION USB_LOW_SPEED

#define USB_SUPPORT_DEVICE

#define USB_NUM_STRING_DESCRIPTORS 4

//#define USB_INTERRUPT_LEGACY_CALLBACKS
#define USB_ENABLE_ALL_HANDLERS
//#define USB_ENABLE_SUSPEND_HANDLER
//#define USB_ENABLE_WAKEUP_FROM_SUSPEND_HANDLER
//#define USB_ENABLE_SOF_HANDLER
//#define USB_ENABLE_ERROR_HANDLER
//#define USB_ENABLE_OTHER_REQUEST_HANDLER
//#define USB_ENABLE_SET_DESCRIPTOR_HANDLER
//#define USB_ENABLE_INIT_EP_HANDLER
//#define USB_ENABLE_EP0_DATA_HANDLER
//#define USB_ENABLE_TRANSFER_COMPLETE_HANDLER

// Device class usage
#define USB_USE_CDC

// Endpoints allocation
#define USB_MAX_EP_NUMBER	    3

// Endpoints allocation - CDC
#define CDC_COMM_INTF_ID        0x0
#define CDC_COMM_EP              2
#define CDC_COMM_IN_EP_SIZE      8
#define CDC_DATA_INTF_ID        0x01
#define CDC_DATA_EP             3
#define CDC_DATA_OUT_EP_SIZE    64
#define CDC_DATA_IN_EP_SIZE     64

//#define USB_CDC_SET_LINE_CODING_HANDLER USBSetLineCodingHandler
//#define USB_CDC_SUPPORT_HARDWARE_FLOW_CONTROL

// Control management: D1 = Set_Line_Coding/Set_Control_Line_State/Get_Line_Coding/Serial_State; D2 = Send_Break.
//#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1


// [dgj] Low-priority USB interrupt support
#if defined(USB_INTERRUPT) && defined(USB_INTERRUPT_LOW)
    #define USBEnableInterruptsLow() {RCONbits.IPEN = 1;IPR2bits.USBIP = 0;PIE2bits.USBIE = 1;INTCONbits.GIEL = 1;}
    #define SetConfigurationOptionsLow() { U1CNFG1 = USB_PULLUP_OPTION | USB_TRANSCEIVER_OPTION | USB_SPEED_OPTION | USB_PING_PONG_MODE; U1EIE = 0x9F; UIE = 0x39 | USB_SOF_INTERRUPT | USB_ERROR_INTERRUPT; }  
    #define USBDeviceAttachLow() { if(USBDeviceState == DETACHED_STATE && USB_BUS_SENSE == 1) { U1CON = 0; U1IE = 0; SetConfigurationOptionsLow(); USBEnableInterruptsLow(); while (!U1CONbits.USBEN) { U1CONbits.USBEN = 1; } USBDeviceState = ATTACHED_STATE; } }
#endif


#endif
