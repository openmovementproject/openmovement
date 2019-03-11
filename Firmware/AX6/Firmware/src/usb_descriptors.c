// usb_descriptors.h - USB descriptors
// Based on Microchip Technology file 'usb_descriptors.h' from "USB Device - Composite - MSD + CDC" example.

/********************************************************************
 FileName:     	usb_descriptors.c
 Dependencies:	See INCLUDES section
 Processor:		PIC18 or PIC24 USB Microcontrollers
 Hardware:		The code is natively intended to be used on the following
 				hardware platforms: PICDEM� FS USB Demo Board, 
 				PIC18F87J50 FS USB Plug-In Module, or
 				Explorer 16 + PIC24 USB PIM.  The firmware may be
 				modified for use on other USB platforms by editing the
 				HardwareProfile.h file.
 Complier:  	Microchip C18 (for PIC18) or C30 (for PIC24)
 Company:		Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the �Company�) for its PIC� Microcontroller is intended and
 supplied to you, the Company�s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

*********************************************************************
-usb_descriptors.c-
-------------------------------------------------------------------
Filling in the descriptor values in the usb_descriptors.c file:
-------------------------------------------------------------------

[Device Descriptors]
The device descriptor is defined as a USB_DEVICE_DESCRIPTOR type.  
This type is defined in usb_ch9.h  Each entry into this structure
needs to be the correct length for the data type of the entry.

[Configuration Descriptors]
The configuration descriptor was changed in v2.x from a structure
to a BYTE array.  Given that the configuration is now a byte array
each byte of multi-byte fields must be listed individually.  This
means that for fields like the total size of the configuration where
the field is a 16-bit value "64,0," is the correct entry for a
configuration that is only 64 bytes long and not "64," which is one
too few bytes.

The configuration attribute must always have the _DEFAULT
definition at the minimum. Additional options can be ORed
to the _DEFAULT attribute. Available options are _SELF and _RWU.
These definitions are defined in the usb_device.h file. The
_SELF tells the USB host that this device is self-powered. The
_RWU tells the USB host that this device supports Remote Wakeup.

[Endpoint Descriptors]
Like the configuration descriptor, the endpoint descriptors were 
changed in v2.x of the stack from a structure to a BYTE array.  As
endpoint descriptors also has a field that are multi-byte entities,
please be sure to specify both bytes of the field.  For example, for
the endpoint size an endpoint that is 64 bytes needs to have the size
defined as "64,0," instead of "64,"

Take the following example:
    // Endpoint Descriptor //
    0x07,                       //the size of this descriptor //
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP02_IN,                   //EndpointAddress
    _INT,                       //Attributes
    0x08,0x00,                  //size (note: 2 bytes)
    0x02,                       //Interval

The first two parameters are self-explanatory. They specify the
length of this endpoint descriptor (7) and the descriptor type.
The next parameter identifies the endpoint, the definitions are
defined in usb_device.h and has the following naming
convention:
_EP<##>_<dir>
where ## is the endpoint number and dir is the direction of
transfer. The dir has the value of either 'OUT' or 'IN'.
The next parameter identifies the type of the endpoint. Available
options are _BULK, _INT, _ISO, and _CTRL. The _CTRL is not
typically used because the default control transfer endpoint is
not defined in the USB descriptors. When _ISO option is used,
addition options can be ORed to _ISO. Example:
_ISO|_AD|_FE
This describes the endpoint as an isochronous pipe with adaptive
and feedback attributes. See usb_device.h and the USB
specification for details. The next parameter defines the size of
the endpoint. The last parameter in the polling interval.

-------------------------------------------------------------------
Adding a USB String
-------------------------------------------------------------------
A string descriptor array should have the following format:

rom struct{byte bLength;byte bDscType;word string[size];}sdxxx={
sizeof(sdxxx),DSC_STR,<text>};

The above structure provides a means for the C compiler to
calculate the length of string descriptor sdxxx, where xxx is the
index number. The first two bytes of the descriptor are descriptor
length and type. The rest <text> are string texts which must be
in the unicode format. The unicode format is achieved by declaring
each character as a word type. The whole text string is declared
as a word array with the number of characters equals to <size>.
<size> has to be manually counted and entered into the array
declaration. Let's study this through an example:
if the string is "USB" , then the string descriptor should be:
(Using index 02)
rom struct{byte bLength;byte bDscType;word string[3];}sd002={
sizeof(sd002),DSC_STR,'U','S','B'};

A USB project may have multiple strings and the firmware supports
the management of multiple strings through a look-up table.
The look-up table is defined as:
rom const unsigned char *rom USB_SD_Ptr[]={&sd000,&sd001,&sd002};

The above declaration has 3 strings, sd000, sd001, and sd002.
Strings can be removed or added. sd000 is a specialized string
descriptor. It defines the language code, usually this is
US English (0x0409). The index of the string must match the index
position of the USB_SD_Ptr array, &sd000 must be in position
USB_SD_Ptr[0], &sd001 must be in position USB_SD_Ptr[1] and so on.
The look-up table USB_SD_Ptr is used by the get string handler
function.

-------------------------------------------------------------------

The look-up table scheme also applies to the configuration
descriptor. A USB device may have multiple configuration
descriptors, i.e. CFG01, CFG02, etc. To add a configuration
descriptor, user must implement a structure similar to CFG01.
The next step is to add the configuration descriptor name, i.e.
cfg01, cfg02,.., to the look-up table USB_CD_Ptr. USB_CD_Ptr[0]
is a dummy place holder since configuration 0 is the un-configured
state according to the definition in the USB specification.

********************************************************************/
 
/*********************************************************************
 * Descriptor specific type definitions are defined in:
 * usb_device.h
 *
 * Configuration options are defined in:
 * usb_config.h
 ********************************************************************/
#ifndef __USB_DESCRIPTORS_C
#define __USB_DESCRIPTORS_C

#include "HardwareProfile.h"

/** INCLUDES *******************************************************/
#include "./USB/usb.h"

#include "./USB/usb_function_msd.h"
#include "./USB/usb_function_cdc.h"

#ifndef UsbInitDescriptors
	#include "Settings.h"
#else
	#error "You have set your descriptors as static and called the dynamic descriptor functions too?"
#endif

/** CONSTANTS ******************************************************/
#if defined(__18CXX)
#pragma romdata
#endif

// New constants
#define bcdCDC 0x0120 // [dgj] Was 0x0110 // CDC spec version: old  V1.10 0x0110, newer V1.20 0x0120

/* Device Descriptor */
ROM USB_DEVICE_DESCRIPTOR device_dsc=
{
    0x12,   				// Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,  // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    0xEF,                   // Class Code
    0x02,                   // Subclass code
    0x01,                   // Protocol code
    USB_EP0_BUFF_SIZE,      // Max packet size for EP0, see usb_config.h
    0x04D8,                 // Vendor ID
    0x0057,                 // Product ID: mass storage device demo
    0x0000 | HARDWARE_VERSION,  // Device release number in BCD format
    0x01,                   // Manufacturer string index
    0x02,                   // Product string index
#if USB_NUM_STRING_DESCRIPTORS > 3
    0x03,                   // Device serial number string index
#else
    0x00,                   // No device serial number string index
#endif
    0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
ROM BYTE configDescriptor1[]={
    /* Configuration Descriptor */
    9,    // Size of this descriptor in bytes
    USB_DESCRIPTOR_CONFIGURATION,                // CONFIGURATION descriptor type
    98, 0,                  // Total length of data for this cfg
    3,                      // Number of interfaces in this cfg
    1,                      // Index value of this configuration
    2,                      // Configuration string index
    _DEFAULT | _SELF,       // Attributes, see usb_device.h
    250,                    // Max power consumption (2X mA)

//---------------MSD Function 1 Descriptors------------------------

    /* Interface Descriptor */
    9,   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
    MSD_INTF_ID,            // Interface Number
    0,                      // Alternate Setting Number
    2,                      // Number of endpoints in this intf
    MSD_INTF,               // Class code
    MSD_INTF_SUBCLASS,      // Subclass code
    MSD_PROTOCOL, 		    // Protocol code
    0,                      // Interface string index
    
    /* Endpoint Descriptor */
    7,
    USB_DESCRIPTOR_ENDPOINT,
    _EP01_IN,_BULK,
    MSD_IN_EP_SIZE,0x00,
    0x01,
    
    /* Endpoint Descriptor */
    7,
    USB_DESCRIPTOR_ENDPOINT,
    _EP01_OUT,
    _BULK,
    MSD_OUT_EP_SIZE,0x00,
    0x01,
    
    
//---------------IAD Descriptor------------------------------------

    /* Interface Association Descriptor: CDC Function 1*/ 
	0x08,             //sizeof(USB_IAD_DSC), // Size of this descriptor in bytes 
	0x0B,             // Interface assocication descriptor type 
	CDC_COMM_INTF_ID, // The first associated interface 
	2,                // Number of contiguous associated interface 
	COMM_INTF,        // bInterfaceClass of the first interface 
	ABSTRACT_CONTROL_MODEL, // bInterfaceSubclass of the first interface 
	V25TER,           // bInterfaceProtocol of the first interface 
	0,                // Interface string index 						


//---------------CDC Function 1 Descriptors------------------------
							
    /* Interface Descriptor: CDC Function 1, Status (communication) Interface */
    0x09,   //sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
    CDC_COMM_INTF_ID,       // Interface Number
    0,                      // Alternate Setting Number
    1,                      // Number of endpoints in this intf
    COMM_INTF,              // Class code
    ABSTRACT_CONTROL_MODEL, // Subclass code
    V25TER,                 // Protocol code
    0,                      // Interface string index

    /* CDC Class-Specific Descriptors */
    //5 bytes: Header Functional Descriptor
    sizeof(USB_CDC_HEADER_FN_DSC), //Size of this descriptor in bytes (5)
    CS_INTERFACE,               //bDescriptorType (class specific)
    DSC_FN_HEADER,              //bDescriptorSubtype (header functional descriptor)
    ((unsigned char)bcdCDC), ((unsigned char)(bcdCDC >> 8)), //bcdCDC (CDC spec version this fw complies with: v1.20 [stored in little endian])

    //4 bytes: Abstract Control Management Functional Descriptor
    sizeof(USB_CDC_ACM_FN_DSC), //Size of this descriptor in bytes (4)
    CS_INTERFACE,               //bDescriptorType (class specific)
    DSC_FN_ACM,                 //bDescriptorSubtype (abstract control management)
    USB_CDC_ACM_FN_DSC_VAL,     // [dgj] Was 0x02, now (0x04|0x02=)0x06?.  bmCapabilities: (see PSTN120.pdf Table 4)

    //5 bytes: Union Functional Descriptor
    sizeof(USB_CDC_UNION_FN_DSC), //Size of this descriptor in bytes (5)
    CS_INTERFACE,                 //bDescriptorType (class specific)
    DSC_FN_UNION,                 //bDescriptorSubtype (union functional)
    CDC_COMM_INTF_ID,             //bControlInterface: Interface number of the communication class interface (1)
    CDC_DATA_INTF_ID,             //bSubordinateInterface0: Data class interface #2 is subordinate to this interface

    //5 bytes: Call Management Functional Descriptor
    sizeof(USB_CDC_CALL_MGT_FN_DSC), //Size of this descriptor in bytes (5)
    CS_INTERFACE,                    //bDescriptorType (class specific)
    DSC_FN_CALL_MGT,                 //bDescriptorSubtype (call management functional)
    0x00,                            //bmCapabilities: device doesn't handle call management
    CDC_DATA_INTF_ID,                //bDataInterface: Data class interface ID used for the optional call management

    /* Endpoint Descriptor */
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP02_IN,                   //EndpointAddress
    _INTERRUPT,                 //Attributes
    CDC_COMM_IN_EP_SIZE,0x00,   //size
    0x02,                       //Interval

    /* Interface Descriptor: CDC Function 1, Data Interface*/
    0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,      // INTERFACE descriptor type
    CDC_DATA_INTF_ID,       // Interface Number
    0,                      // Alternate Setting Number
    2,                      // Number of endpoints in this intf
    DATA_INTF,              // Class code
    0,                      // Subclass code
    NO_PROTOCOL,            // Protocol code
    0,                      // Interface string index
    
    /* Endpoint Descriptor */
    //sizeof(USB_EP_DSC),DSC_EP,_EP03_OUT,_BULK,CDC_BULK_OUT_EP_SIZE,0x00,
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP03_OUT,            //EndpointAddress
    _BULK,                       //Attributes
    CDC_DATA_OUT_EP_SIZE,0x00,                  //size
    0x00,                       //Interval

    /* Endpoint Descriptor */
    //sizeof(USB_EP_DSC),DSC_EP,_EP03_IN,_BULK,CDC_BULK_IN_EP_SIZE,0x00
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP03_IN,            //EndpointAddress
    _BULK,                       //Attributes
    CDC_DATA_IN_EP_SIZE,0x00,                  //size
    0x00                       //Interval
};


//Language code string descriptor
ROM struct{BYTE bLength;BYTE bDscType;WORD string[1];}sd000={
    sizeof(sd000),
    USB_DESCRIPTOR_STRING,
    {0x0409
    }
};
//Manufacturer string descriptor
ROM struct{BYTE bLength;BYTE bDscType;WORD string[25];}sd001={
    sizeof(sd001), USB_DESCRIPTOR_STRING,
    {'N','e','w','c','a','s','t','l','e',' ','U','n','i','v','e','r','s','i','t','y',' ',' ',' ',' ',' '}
};

//Product string descriptor
ROM struct{BYTE bLength;BYTE bDscType;WORD string[28];}sd002={
    sizeof(sd002), USB_DESCRIPTOR_STRING,
    {HARDWARE_TYPE_0,HARDWARE_TYPE_1,HARDWARE_TYPE_2,' ','C','o','m','p','o','s','i','t','e',' ','D','e','v','i','c','e',' ','0' + (HARDWARE_VERSION >> 4),'.','0' + (HARDWARE_VERSION & 0x0f),' ',' '}
};


#if USB_NUM_STRING_DESCRIPTORS > 3
// USB Product serial number -- serial number is overwritten and is initially ',' 
// characters as that triggers the "invalid serial number" behaviour on Windows
struct { BYTE bLength; BYTE bDscType; WORD string[11+5]; } sd003 =
{
    sizeof(sd003), USB_DESCRIPTOR_STRING,		// sizeof(sd003) = 24, USB_DESCRIPTOR_STRING = 0x03
    {HARDWARE_LABEL_0,HARDWARE_LABEL_1,HARDWARE_LABEL_2,'0' + (HARDWARE_VERSION >> 4),'0' + (HARDWARE_VERSION & 0x0f),'_',',',',',',',',',',',  ',',',',',',',',','}     // Serial number is still 'CWA' for detection code
}; //                                                                            14  16  18  20  22
// word:  "65535" (as words)
// dword: "4294967295" (as words)
#endif


//Array of configuration descriptors
ROM BYTE *ROM USB_CD_Ptr[]=
{
    (ROM BYTE *ROM)&configDescriptor1
};

//Array of string descriptors
BYTE *USB_SD_Ptr[]=
{
    (BYTE *)&sd000,
    (BYTE *)&sd001,
    (BYTE *)&sd002,
#if USB_NUM_STRING_DESCRIPTORS > 3
    (BYTE *)&sd003,      // Serial number
#endif
};

#if (USB_NUM_STRING_DESCRIPTORS <= 3)
#warning "This build doesn't have a USB serial number"
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
    {HARDWARE_TYPE_0,HARDWARE_TYPE_1,HARDWARE_TYPE_2,' ',' ',' ',' ',' '},                                  // T10 assigned Vendor identification
    {HARDWARE_TYPE_0,HARDWARE_TYPE_1,HARDWARE_TYPE_2,' ','M','a','s','s',' ','S','t','o','r','a','g','e'},  // Product identification
    {'0' + ((HARDWARE_VERSION >> 12) & 0x0f),'0' + ((HARDWARE_VERSION >> 8) & 0x0f),'0' + ((HARDWARE_VERSION >> 4) & 0x0f),'0' + (HARDWARE_VERSION & 0x0f)} // Product revision level
};

void UsbInitDescriptors(void)
{
#if USB_NUM_STRING_DESCRIPTORS > 3
    // If we have a valid serial number, patch the USB response with the serial number
    if (settings.deviceId != DEVICEID_UNSET)
	{
        unsigned short *number = &sd003.string[6];
		// 10 digits for full 32-bit number range
        if (settings.deviceId > 9999999ul)
		{
	        *number++ = (signed short)'0' + ((settings.deviceId / 1000000000ul) % 10);
	        *number++ = (signed short)'0' + ((settings.deviceId /  100000000ul) % 10);
	        *number++ = (signed short)'0' + ((settings.deviceId /   10000000ul) % 10);
		}
		// 7 digits to match volume label limit
        if (settings.deviceId > 99999ul)
		{
	        *number++ = (signed short)'0' + ((settings.deviceId /    1000000ul) % 10);
	        *number++ = (signed short)'0' + ((settings.deviceId /     100000ul) % 10);
		}
		// 5 digits appear fully backwards-compatible with 16-bit device ids
        *number++ = (signed short)'0' + ((settings.deviceId / 10000) % 10);
        *number++ = (signed short)'0' + ((settings.deviceId /  1000) % 10);
        *number++ = (signed short)'0' + ((settings.deviceId /   100) % 10);
        *number++ = (signed short)'0' + ((settings.deviceId /    10) % 10);
        *number++ = (signed short)'0' + ((settings.deviceId        ) % 10);

		// Trim length
		sd003.bLength = (char *)number - (char *)&sd003;
    }
#else
	#error "you have made a mistake"
#endif
}


/** EOF usb_descriptors.c ***************************************************/

#endif
