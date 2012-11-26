/* Based on "usb_config.h" from "USB Device - Composite - MSD + CDC" example by Microchip, copyright as below, changes are:
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

// usb_config.h - USB configuration
// Based on Microchip Technology file 'usb_config.h' from "USB Device - Composite - MSD + CDC" example

/********************************************************************
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

********************************************************************
 File Description:

 Change History:
  Rev   Date         Description
  1.0   11/19/2004   Initial release
  2.1   02/26/2007   Updated for simplicity and to use common
                     coding style
 *******************************************************************/

/*********************************************************************
 * Descriptor specific type definitions are defined in: usbd.h
 ********************************************************************/

#ifndef USBCFG_H
#define USBCFG_H


//#define USB_POLLING
#define USB_INTERRUPT

/** DEVICE CLASS USAGE *********************************************/
#define USB_USE_CDC
//#define USB_USE_MSD

/** ENQUIRY RESPONCE **********************************************/
// The enquiry responce tells the host the drive type, name scsi support etc.. Default in microchips demo one
//#define HAVE_OWN_ENQUIRY_RESPONCE	// Otherwise it will create one in USB_CDC_MSD.c with default values

/** DYNAMIC DESCRIPTORS **********************************************/
// If the usb descriptors are dynamic then uncomment this line to enable the function
#define UsbInitDescriptors()	{}

/** DEFINITIONS ****************************************************/
#define USB_EP0_BUFF_SIZE		8	// Valid Options: 8, 16, 32, or 64 bytes.
								// Using larger options take more SRAM, but
								// does not provide much advantage in most types
								// of applications.  Exceptions to this, are applications
								// that use EP0 IN or OUT for sending large amounts of
								// application related data.
									
/** ENDPOINTS ALLOCATION *******************************************/
#ifdef  USB_USE_CDC	
	#ifdef 	USB_USE_MSD						
		#define USB_MAX_NUM_INT     	3   // For tracking Alternate Setting
		#define USB_MAX_EP_NUMBER	    3
	#else
		#define USB_MAX_NUM_INT     	2   // For tracking Alternate Setting
		#define USB_MAX_EP_NUMBER	    2
	#endif
#elif define USB_USE_MSD
		#define USB_MAX_NUM_INT     	1   // For tracking Alternate Setting
		#define USB_MAX_EP_NUMBER	    1
#endif

//Device descriptor - if these two definitions are not defined then
//  a ROM USB_DEVICE_DESCRIPTOR variable by the exact name of device_dsc
//  must exist.
#define USB_USER_DEVICE_DESCRIPTOR &device_dsc
#define USB_USER_DEVICE_DESCRIPTOR_INCLUDE extern ROM USB_DEVICE_DESCRIPTOR device_dsc

//Configuration descriptors - if these two definitions do not exist then
//  a ROM BYTE *ROM variable named exactly USB_CD_Ptr[] must exist.
#define USB_USER_CONFIG_DESCRIPTOR USB_CD_Ptr
#define USB_USER_CONFIG_DESCRIPTOR_INCLUDE extern ROM BYTE *ROM USB_CD_Ptr[]

//Make sure only one of the below "#define USB_PING_PONG_MODE"
//is uncommented.
//#define USB_PING_PONG_MODE USB_PING_PONG__NO_PING_PONG
#define USB_PING_PONG_MODE USB_PING_PONG__FULL_PING_PONG
//#define USB_PING_PONG_MODE USB_PING_PONG__EP0_OUT_ONLY
//#define USB_PING_PONG_MODE USB_PING_PONG__ALL_BUT_EP0		//NOTE: This mode is not supported in PIC18F4550 family rev A3 devices


/* Parameter definitions are defined in usb_device.h */
#define USB_PULLUP_OPTION USB_PULLUP_ENABLE
//#define USB_PULLUP_OPTION USB_PULLUP_DISABLED

#define USB_TRANSCEIVER_OPTION USB_INTERNAL_TRANSCEIVER
//External Transceiver support is not available on all product families.  Please
//  refer to the product family datasheet for more information if this feature
//  is available on the target processor.
//#define USB_TRANSCEIVER_OPTION USB_EXTERNAL_TRANSCEIVER

#define USB_SPEED_OPTION USB_FULL_SPEED
//                       USB_LOW_SPEED (not valid option for PIC24F devices)

#define USB_SUPPORT_DEVICE

// Number of string descriptions should be 4 if we want a serial number (3 otherwise)
/** DESCRIPTORS ***************************************************/
#define USB_NUM_STRING_DESCRIPTORS 		4        // [dgj] 3 -> 4 to include serial number
// This makes the descriptots static
//#define UsbInitDescriptors()	;
#ifndef UsbInitDescriptors
	void UsbInitDescriptors(void);
#endif

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
/** DEVICE CLASS USAGE *********************************************/


/** ENDPOINTS ALLOCATION *******************************************/

/* MSD */
#define MSD_INTF_ID             0x00
#define MSD_IN_EP_SIZE          64
#define MSD_OUT_EP_SIZE         64
#define MAX_LUN                 0   //base 0 number.  0 == 1 LUN
#define MSD_DATA_IN_EP          1
#define MSD_DATA_OUT_EP         1
#define MSD_BUFFER_ADDRESS      0x600

#ifdef USB_USE_MSD
/* CDC */
#define CDC_COMM_INTF_ID        0x01
#define CDC_COMM_EP              2
#define CDC_COMM_IN_EP_SIZE      10

#define CDC_DATA_INTF_ID        0x02
#define CDC_DATA_EP             3
#define CDC_DATA_OUT_EP_SIZE    64
#define CDC_DATA_IN_EP_SIZE     64
#else
/* CDC */
#define CDC_COMM_INTF_ID        0x0
#define CDC_COMM_EP              1
#define CDC_COMM_IN_EP_SIZE      10

#define CDC_DATA_INTF_ID        0x01
#define CDC_DATA_EP             2
#define CDC_DATA_OUT_EP_SIZE    64
#define CDC_DATA_IN_EP_SIZE     64
#endif

//#define USB_CDC_SET_LINE_CODING_HANDLER mySetLineCodingHandler
//#define USB_CDC_SUPPORT_HARDWARE_FLOW_CONTROL

//#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2 //Send_Break command
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1 //Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, and Serial_State commands

/** DEFINITIONS ****************************************************/

#endif //USBCFG_H
