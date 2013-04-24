/*	This bootloader was adapted form the generic microchip USB HID 
	bootloader for PIC24 MCUs. Karim Ladha, Dan Jackson 2011	*/
/********************************************************************
 FileName:     main.c
 Dependencies: See INCLUDES section
 Processor:		PIC24 USB Microcontrollers
 Hardware:		The code is natively intended to be used on the following
 				hardware platforms: 
 				Explorer 16 + PIC24FJ256GB110 PIM.  The firmware may be
 				modified for use on other USB platforms by editing the
 				HardwareProfile.h file.
 Complier:  	Microchip C30 (for PIC24)
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

  Rev   Description
	MY VERSION- KL 2011
	I have ported this code to the CWA1.6 HW using timeouts similar to CWA 1.4
	Many of the included.c files have large areas commented to reduce the c.code size
	Added robust pre-charge code allowing for soft disconnects etc
	Precharge threshold set in HW profile which is common to the CWA1.6 main FW
	Removed send-resume USB handler since it can't be called, this makes room for my other additions
	Noticed error when loading hex files using mchip bootloader:
		First memory location at 0x001400 should read 0x041510 or "goto 0x1510"
		instead it reads 0xffff00ff which means nothing - "nop"
		Changed all jumps out of bootloader to goto 0x5010 instead
		This is a bootloader symptom, the hex files used are OK
	Added timeout and bootloader running detection. 
	Changed reset button in bootloader to goto newly written code i.e. goto 0x1510
  ----- ---------------------------------------------
  v2.2  Adapted from PIC18F87J50 HID Bootloader Firmware
        as basis for BootApplication().  The rest of the
        code was taken from the Simple HID Demo in 
        MCHPFSUSB v2.2.

  v2.6a Fixed race condition where an OUT packet could potentially
        clear the prior IN packet depending on the bus communication
        order.

        Removed ability to reflash the interrupt vectors.  This has
        been depricated for a interrupt remapping scheme documented
        in the migration notes and in the getting started for the HID
        bootloader.

********************************************************************/




#ifndef USBMOUSE_C
#define USBMOUSE_C

/** INCLUDES *******************************************************/
#include <compiler.h>
#include <p24Fxxxx.h>

#include "./USB/usb.h"
#include "HardwareProfile.h"
#include "./USB/usb_function_hid.h"
#include "uart.h"


/** C O N S T A N T S **********************************************************/

//Section defining the address range to erase for the erase device command, along with the valid programming range to be reported by the QUERY_DEVICE command.
#define VectorsStart					0x00000000	
#define	VectorsEnd						0x00000400	//One page of vectors + general purpose program memory.
//Bootloader resides in memory range 0x400-0x17FF
#define ProgramMemStart					0x00001400 //Beginning of application program memory (not occupied by bootloader).  **THIS VALUE MUST BE ALIGNED WITH BLOCK BOUNDRY** Also, in order to work correctly, make sure the StartPageToErase is set to erase this section.

#if defined(__PIC24FJ256GB110__) || defined(__PIC24FJ256GB108__) || defined(__PIC24FJ256GB106__)
	#define	BeginPageToErase			5		 //Bootloader and vectors occupy first six 1024 word (1536 bytes due to 25% unimplemented bytes) pages
	#define MaxPageToEraseNoConfigs		169		 //Last full page of flash on the PIC24FJ256GB110, which does not contain the flash configuration words.
	#define MaxPageToEraseWithConfigs	170		 //Page 170 contains the flash configurations words on the PIC24FJ256GB110.  Page 170 is also smaller than the rest of the (1536 byte) pages.
	#define ProgramMemStopNoConfigs		0x0002A800 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: 
                                                        //IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	

	#define ProgramMemStopWithConfigs	0x0002ABF8 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	
	#define ConfigWordsStartAddress		0x0002ABF8 //0x2ABFA is start of CW3 on PIC24FJ256GB110 Family devices
    #define ConfigWordsStopAddress		0x0002AC00
#elif defined(__PIC24FJ192GB110__) || defined(__PIC24FJ192GB108__) || defined(__PIC24FJ192GB106__)
	#define	BeginPageToErase			5		 //Bootloader and vectors occupy first six 1024 word (1536 bytes due to 25% unimplemented bytes) pages
	#define MaxPageToEraseNoConfigs		129		 //Last full page of flash on the PIC24FJ192GB110, which does not contain the flash configuration words.
	#define MaxPageToEraseWithConfigs	130		 //Page 130 contains the flash configurations words on the PIC24FJ192GB110.  Page 130 is also smaller than the rest of the (1536 byte) pages.
	#define ProgramMemStopNoConfigs		0x00020800 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: 
                                                        //IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	

	#define ProgramMemStopWithConfigs	0x00020BF8 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	
	#define ConfigWordsStartAddress		0x00020BF8 //0x20BFA is start of CW3 on PIC24FJ192GB110 Family devices
    #define ConfigWordsStopAddress		0x00020C00
#elif defined(__PIC24FJ128GB110__) || defined(__PIC24FJ128GB108__) || defined(__PIC24FJ128GB106__)
	#define	BeginPageToErase			5		 //Bootloader and vectors occupy first six 1024 word (1536 bytes due to 25% unimplemented bytes) pages
	#define MaxPageToEraseNoConfigs		84		 //Last full page of flash on the PIC24FJ128GB110, which does not contain the flash configuration words.
	#define MaxPageToEraseWithConfigs	85       //Page 85 contains the flash configurations words on the PIC24FJ128GB110.  Page 85 is also smaller than the rest of the (1536 byte) pages.
	#define ProgramMemStopNoConfigs		0x00015400 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: 
                                                        //IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	

	#define ProgramMemStopWithConfigs	0x000157F8 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	
	#define ConfigWordsStartAddress		0x000157F8 //0x157FA is start of CW3 on PIC24FJ128GB110 Family devices
    #define ConfigWordsStopAddress		0x00015800
#elif defined(__PIC24FJ64GB110__) || defined(__PIC24FJ64GB108__) || defined(__PIC24FJ64GB106__)
	#define	BeginPageToErase			5		 //Bootloader and vectors occupy first six 1024 word (1536 bytes due to 25% unimplemented bytes) pages
	#define MaxPageToEraseNoConfigs		41		 //Last full page of flash on the PIC24FJ64GB110, which does not contain the flash configuration words.
	#define MaxPageToEraseWithConfigs	42		 //Page 42 contains the flash configurations words on the PIC24FJ64GB110.  Page 42 is also smaller than the rest of the (1536 byte) pages.
	#define ProgramMemStopNoConfigs		0x0000A800 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: 
                                                        //IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	

	#define ProgramMemStopWithConfigs	0x0000ABF8 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	
	#define ConfigWordsStartAddress		0x0000ABF8 //0x0ABFA is start of CW3 on PIC24FJ64GB110 Family devices
    #define ConfigWordsStopAddress		0x0000AC00
#else
    #error "This bootloader only covers the PIC24FJ256GB110 family devices.  Please see another folder for the bootloader appropriate for the selected device."
#endif

//Switch State Variable Choices
#define	QUERY_DEVICE				0x02	//Command that the host uses to learn about the device (what regions can be programmed, and what type of memory is the region)
#define	UNLOCK_CONFIG				0x03	//Note, this command is used for both locking and unlocking the config bits (see the "//Unlock Configs Command Definitions" below)
#define ERASE_DEVICE				0x04	//Host sends this command to start an erase operation.  Firmware controls which pages should be erased.
#define PROGRAM_DEVICE				0x05	//If host is going to send a full RequestDataBlockSize to be programmed, it uses this command.
#define	PROGRAM_COMPLETE			0x06	//If host send less than a RequestDataBlockSize to be programmed, or if it wished to program whatever was left in the buffer, it uses this command.
#define GET_DATA					0x07	//The host sends this command in order to read out memory from the device.  Used during verify (and read/export hex operations)
#define	RESET_DEVICE				0x08	//Resets the microcontroller, so it can update the config bits (if they were programmed, and so as to leave the bootloader (and potentially go back into the main application)

//Unlock Configs Command Definitions
#define UNLOCKCONFIG				0x00	//Sub-command for the ERASE_DEVICE command
#define LOCKCONFIG					0x01	//Sub-command for the ERASE_DEVICE command

//Query Device Response "Types" 
#define	TypeProgramMemory			0x01	//When the host sends a QUERY_DEVICE command, need to respond by populating a list of valid memory regions that exist in the device (and should be programmed)
#define TypeEEPROM					0x02
#define TypeConfigWords				0x03
#define	TypeEndOfTypeList			0xFF	//Sort of serves as a "null terminator" like number, which denotes the end of the memory region list has been reached.


//BootState Variable States
#define	IdleState					0x00
#define NotIdleState				0x01

//OtherConstants
#define InvalidAddress				0xFFFFFFFF

//Application and Microcontroller constants
#define BytesPerFlashAddress		0x02	//For Flash memory: One byte per address on PIC18, two bytes per address on PIC24

#define	TotalPacketSize				0x40
#define WORDSIZE					0x02	//PIC18 uses 2 byte instruction words, PIC24 uses 3 byte "instruction words" (which take 2 addresses, since each address is for a 16 bit word; the upper word contains a "phantom" byte which is unimplemented.).
//#define	FlashBlockSize				0x40	//For PIC18F87J50 family devices, a flash block is 64 bytes
#define RequestDataBlockSize 		56	//Number of bytes in the "Data" field of a standard request to/from the PC.  Must be an even number from 2 to 56. 
#define BufferSize 					0x20	//32 16-bit words of buffer

/** PRIVATE PROTOTYPES *********************************************/
void BlinkUSBStatus(void);
BOOL Switch2IsPressed(void);
BOOL Switch3IsPressed(void);
void Emulate_Mouse(void);
void InitializeSystem(void);
void ProcessIO(void);
void UserInit(void);
void YourHighPriorityISRCode();
void YourLowPriorityISRCode();
void USBCBSendResume(void);

void BlinkUSBStatus(void);
void UserInit(void);
void EraseFlash(void);
void WriteFlashSubBlock(void);
void BootApplication(void);
DWORD ReadProgramMemory(DWORD);

/** T Y P E  D E F I N I T I O N S ************************************/

typedef union __attribute__ ((packed)) _USB_HID_BOOTLOADER_COMMAND
{
		unsigned char Contents[64];
		
		struct __attribute__ ((packed)) {
			unsigned char Command;
			WORD AddressHigh;
			WORD AddressLow;
			unsigned char Size;
			unsigned char PadBytes[(TotalPacketSize - 6) - (RequestDataBlockSize)];
			unsigned int Data[RequestDataBlockSize/WORDSIZE];
		};
		
		struct __attribute__ ((packed)) {
			unsigned char Command;
			DWORD Address;
			unsigned char Size;
			unsigned char PadBytes[(TotalPacketSize - 6) - (RequestDataBlockSize)];	
			unsigned int Data[RequestDataBlockSize/WORDSIZE];
		};
		
		struct __attribute__ ((packed)){
			unsigned char Command;
			unsigned char PacketDataFieldSize;
			unsigned char BytesPerAddress;
			unsigned char Type1;
			unsigned long Address1;
			unsigned long Length1;
			unsigned char Type2;
			unsigned long Address2;
			unsigned long Length2;
			unsigned char Type3;		//End of sections list indicator goes here, when not programming the vectors, in that case fill with 0xFF.
			unsigned long Address3;
			unsigned long Length3;			
			unsigned char Type4;		//End of sections list indicator goes here, fill with 0xFF.
			unsigned char ExtraPadBytes[33];
		};
		
		struct __attribute__ ((packed)){						//For lock/unlock config command
			unsigned char Command;
			unsigned char LockValue;
		};
} PacketToFromPC;		
	
/** VARIABLES ******************************************************/
#pragma udata

#define TIME_OUT 99
unsigned char timeout_counter; // added
union { unsigned char handled : 1; } message; // added
// My prototypes
extern void WaitForPrecharge(void);

#pragma udata USB_VARS
PacketToFromPC PacketFromPC;		//64 byte buffer for receiving packets on EP1 OUT from the PC
PacketToFromPC PacketToPC;			//64 byte buffer for sending packets on EP1 IN to the PC
PacketToFromPC PacketFromPCBuffer;

#pragma udata
USB_HANDLE USBOutHandle = 0;
USB_HANDLE USBInHandle = 0;
BOOL blinkStatusValid = TRUE;

unsigned char MaxPageToErase;
unsigned long ProgramMemStopAddress;
unsigned char BootState;
unsigned char ErasePageTracker;
unsigned int ProgrammingBuffer[BufferSize];
unsigned char BufferedDataIndex;
unsigned long ProgrammedPointer;
unsigned char ConfigsProtected;


/** DECLARATIONS ***************************************************/

/********************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/
#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{   
static WORD led_count=0;

//InitIO();
//REMAP_PINS();
LED_INIT_PINS();
BUTTON_INIT_PINS();
//FLASH_INIT_PINS();
//ACCEL_INIT_PINS();
//GYRO_INIT_PINS();
//MAG_INIT_PINS();
//ALT_INIT_PINS();
//DISPLAY_INIT_PINS();
BATT_INIT_PINS();
//TEMP_INIT_PINS();
//LDR_INIT_PINS();
ANALOGUE_INIT_PINS();
//DRIVE_FLOATING_PINS(); /*Reduce leakage*/
USB_BUS_SENSE_INIT();
//BT_INIT_PINS();

// Bail function - no usb voltage - goto user code
if((!USB_BUS_SENSE) || (BUTTON)) // was using RCON reset check here
{
	__asm__("goto 0x1510");
}

WaitForPrecharge();	// Wait until the battery is not in precharge

LED = 1;			// Increase current draw to keep charger active - stops it resetting when no batt
CLOCK_PLL();		// Switch to full speed for USB

// Initialise the timeout variable
// We have to keep clearing this bit to stop the device timeing out
timeout_counter = 0;
message.handled = 0;

//   InitializeSystem(); - this is done mainly by InitIO, the remainder follows
USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
    				//variables to known states.

//initialize the variable holding the handle for the last
// transmission
USBOutHandle = 0;
USBInHandle = 0;

//blinkStatusValid = TRUE; - not used

//Initialize bootloader state variables
MaxPageToErase = MaxPageToEraseNoConfigs;		//Assume we will not allow erase/programming of config words (unless host sends override command)
ProgramMemStopAddress = ProgramMemStopNoConfigs;
ConfigsProtected = LOCKCONFIG;					//Assume we will not erase or program the vector table at first.  Must receive unlock config bits/vectors command first.
BootState = IdleState;
ProgrammedPointer = InvalidAddress;	
BufferedDataIndex = 0;


    #if defined(USB_INTERRUPT)
        USBDeviceAttach();
		#error "This is a bad idea! Dont do it"
    #endif

    while(usb_bus_sense)
    {
        #if defined(USB_POLLING)
		// Check bus status and service USB interrupts.
        USBDeviceTasks(); // Interrupt or polling method.  If using polling, must call
        				  // this function periodically.  This function will take care
        				  // of processing and responding to SETUP transactions 
        				  // (such as during the enumeration process when you first
        				  // plug in).  USB hosts require that USB devices should accept
        				  // and process SETUP packets in a timely fashion.  Therefore,
        				  // when using polling, this function should be called 
        				  // regularly (such as once every 1.8ms or faster** [see 
        				  // inline code comments in usb_device.c for explanation when
        				  // "or faster" applies])  In most cases, the USBDeviceTasks() 
        				  // function does not take very long to execute (ex: <100 
        				  // instruction cycles) before it returns.
        #endif
    		
		// Added 
		if (!USB_BUS_SENSE)
		{
			// USB was disconnected - on re-entry it will goto user code
			Reset();
		}		  

		// Slow loop - flashed LED and detects timeouts
	    if (!led_count) { led_count = 10000U; }
	    if (!--led_count) 
		{
			// Added	- So long as the device does not do a USB bootloader app transfer (app not running) then it will timeout here
			timeout_counter++;
			if (!message.handled)
			{
				//if ((usb_device_state == CONFIGURED_STATE && timeout_counter > TIME_OUT_CONFIGURED) ||
				//    (usb_device_state != CONFIGURED_STATE && timeout_counter > TIME_OUT_UNCONFIGURED))
				if (timeout_counter > TIME_OUT)
				{
					// Device was not bootloaded in time - goto user app
					//T0CON = 0; // Clear the initialised modules etc
					U1CON = 0x0000;				//Disable USB module
					CLOCK_INTOSC();
		       		LED = 0;
					__asm__("goto 0x1510");
					
				}
			}
		
			// A USB bootloader app transfer has occured - timeout will not happen now - LED flashes
			if(USBDeviceState == CONFIGURED_STATE)
		        {
		       		LED = !LED;
		        }//end if(...)
			else
				{
		       		LED = 0;	//LED_B = 1;
				}
		} // end slow LED loop

		// Application-specific tasks.
		// Application related code may be added here, or in the ProcessIO() function.
        ProcessIO();   // On PIC24s this checks if USB is configured 
    }//end while(usb_bus_sense) 
return 0;
}//end main

/********************************************************************
 * Function:        static void InitializeSystem(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        InitializeSystem is a centralize initialization
 *                  routine. All required USB initialization routines
 *                  are called from here.
 *
 *                  User application initialization routine should
 *                  also be called from here.                  
 *
 * Note:            None
 *******************************************************************/
//static void InitializeSystem(void)
//{
//    #if (defined(__18CXX) & !defined(PIC18F87J50_PIM))
//        ADCON1 |= 0x0F;                 // Default all pins to digital
//    #elif defined(__C30__)
//        AD1PCFGL = 0xFFFF;
//    #endif
//
//    
////	The USB specifications require that USB peripheral devices must never source
////	current onto the Vbus pin.  Additionally, USB peripherals should not source
////	current on D+ or D- when the host/hub is not actively powering the Vbus line.
////	When designing a self powered (as opposed to bus powered) USB peripheral
////	device, the firmware should make sure not to turn on the USB module and D+
////	or D- pull up resistor unless Vbus is actively powered.  Therefore, the
////	firmware needs some means to detect when Vbus is being powered by the host.
////	A 5V tolerant I/O pin can be connected to Vbus (through a resistor), and
//// 	can be used to detect when Vbus is high (host actively powering), or low
////	(host is shut down or otherwise not supplying power).  The USB firmware
//// 	can then periodically poll this I/O pin to know when it is okay to turn on
////	the USB module/D+/D- pull up resistor.  When designing a purely bus powered
////	peripheral device, it is not possible to source current on D+ or D- when the
////	host is not actively providing power on Vbus. Therefore, implementing this
////	bus sense feature is optional.  This firmware can be made to use this bus
////	sense feature by making sure "USE_USB_BUS_SENSE_IO" has been defined in the
////	HardwareProfile.h file.    
//    #if defined(USE_USB_BUS_SENSE_IO)
//    tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
//    #endif
//    
////	If the host PC sends a GetStatus (device) request, the firmware must respond
////	and let the host know if the USB peripheral device is currently bus powered
////	or self powered.  See chapter 9 in the official USB specifications for details
////	regarding this request.  If the peripheral device is capable of being both
////	self and bus powered, it should not return a hard coded value for this request.
////	Instead, firmware should check if it is currently self or bus powered, and
////	respond accordingly.  If the hardware has been configured like demonstrated
////	on the PICDEM FS USB Demo Board, an I/O pin can be polled to determine the
////	currently selected power source.  On the PICDEM FS USB Demo Board, "RA2" 
////	is used for	this purpose.  If using this feature, make sure "USE_SELF_POWER_SENSE_IO"
////	has been defined in HardwareProfile.h, and that an appropriate I/O pin has been mapped
////	to it in HardwareProfile.h.
//    #if defined(USE_SELF_POWER_SENSE_IO)
//    tris_self_power = INPUT_PIN;	// See HardwareProfile.h
//    #endif
//    
//    USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
//    					//variables to known states.
//    UserInit();
//
//}//end InitializeSystem

//void UserInit(void)
//{
//    //Initialize all of the LED pins
//    mInitAllLEDs();
//    
//    //Initialize all of the push buttons
//    mInitAllSwitches();
//    
//    //initialize the variable holding the handle for the last
//    // transmission
//    USBOutHandle = 0;
//    USBInHandle = 0;
//
//    blinkStatusValid = TRUE;
//
//	//Initialize bootloader state variables
//	MaxPageToErase = MaxPageToEraseNoConfigs;		//Assume we will not allow erase/programming of config words (unless host sends override command)
//	ProgramMemStopAddress = ProgramMemStopNoConfigs;
//	ConfigsProtected = LOCKCONFIG;					//Assume we will not erase or program the vector table at first.  Must receive unlock config bits/vectors command first.
//	BootState = IdleState;
//	ProgrammedPointer = InvalidAddress;	
//	BufferedDataIndex = 0;
//}//end UserInit


/********************************************************************
 * Function:        void ProcessIO(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is a place holder for other user
 *                  routines. It is a mixture of both USB and
 *                  non-USB tasks.
 *
 * Note:            None
 *******************************************************************/
//void ProcessIO(void) //- This function just calls boot app - removed
//{   
//    //Blink the LEDs according to the USB device status
//    if(blinkStatusValid) // removed
//    {
//        BlinkUSBStatus();
//    }
//
//    // User Application USB tasks
//    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;
//    
//    BootApplication();
//    
//}//end ProcessIO


/********************************************************************
 * Function:        void BlinkUSBStatus(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        BlinkUSBStatus turns on and off LEDs 
 *                  corresponding to the USB device state.
 *
 * Note:            mLED macros can be found in HardwareProfile.h
 *                  USBDeviceState is declared and updated in
 *                  usb_device.c.
 *******************************************************************/

//void BlinkUSBStatus(void)
//{
//    static WORD led_count=0;
//    
//    if(led_count == 0)led_count = 10000U;
//    led_count--;
//
//    #define mLED_Both_Off()         {mLED_1_Off();mLED_2_Off();}
//    #define mLED_Both_On()          {mLED_1_On();mLED_2_On();}
//    #define mLED_Only_1_On()        {mLED_1_On();mLED_2_Off();}
//    #define mLED_Only_2_On()        {mLED_1_Off();mLED_2_On();}
//
//    if(USBSuspendControl == 1)
//    {
//        if(led_count==0)
//        {
//            mLED_1_Toggle();
//            mSetLED_2(mGetLED_1());        // Both blink at the same time
//        }//end if
//    }
//    else
//    {
//        if(USBDeviceState == DETACHED_STATE)
//        {
//            mLED_Both_Off();
//        }
//        else if(USBDeviceState == ATTACHED_STATE)
//        {
//            mLED_Both_On();
//        }
//        else if(USBDeviceState == POWERED_STATE)
//        {
//            mLED_Only_1_On();
//        }
//        else if(USBDeviceState == DEFAULT_STATE)
//        {
//            mLED_Only_2_On();
//        }
//        else if(USBDeviceState == ADDRESS_STATE)
//        {
//            if(led_count == 0)
//            {
//                mLED_1_Toggle();
//                mLED_2_Off();
//            }//end if
//        }
//        else if(USBDeviceState == CONFIGURED_STATE)
//        {
//            if(led_count==0)
//            {
//                mLED_1_Toggle();
//                mSetLED_2(!mGetLED_1());       // Alternate blink                
//            }//end if
//        }//end if(...)
//    }//end if(UCONbits.SUSPND...)
//
//}//end BlinkUSBStatus



/******************************************************************************
 * Function:        void ProcessIO(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is a place holder for other user routines.
 *                  It is a mixture of both USB and non-USB tasks.
 *
 * Note:            None
 *****************************************************************************/
// Modded to allow timeout
//void BootApplication(void)
void ProcessIO(void) // Funilly enuf, look at the function desc, they renamed this already?
{
	unsigned char i;
	DWORD_VAL FlashMemoryValue;

	if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;

    if(BootState == IdleState)
    {
        //Are we done sending the last response.  We need to be before we 
        //  receive the next command because we clear the PacketToPC buffer
        //  once we receive a command
        if(!USBHandleBusy(USBInHandle))
        {
            if(!USBHandleBusy(USBOutHandle))		//Did we receive a command?
            {
                for(i = 0; i < TotalPacketSize; i++)
                {
                    PacketFromPC.Contents[i] = PacketFromPCBuffer.Contents[i];
                }
                
                USBOutHandle = USBRxOnePacket(HID_EP,(BYTE*)&PacketFromPCBuffer,64);
                BootState = NotIdleState;
                
                //Prepare the next packet we will send to the host, by initializing the entire packet to 0x00.	
                for(i = 0; i < TotalPacketSize; i++)
                {
                    //This saves code space, since we don't have to do it independently in the QUERY_DEVICE and GET_DATA cases.
                    PacketToPC.Contents[i] = 0;	
                }
            }
        }
    }
	else //(BootState must be in NotIdleState)
	{	
		switch(PacketFromPC.Command)
		{
			case QUERY_DEVICE:
			{
message.handled = 1;	/* Prevent bootloader exit - this is the one the bootloader app first sets */
				//Prepare a response packet, which lets the PC software know about the memory ranges of this device.
				
				PacketToPC.Command = (unsigned char)QUERY_DEVICE;
				PacketToPC.PacketDataFieldSize = (unsigned char)RequestDataBlockSize;
				PacketToPC.BytesPerAddress = (unsigned char)BytesPerFlashAddress;

                PacketToPC.Type1 = (unsigned char)TypeProgramMemory;
                PacketToPC.Address1 = (unsigned long)ProgramMemStart;
                PacketToPC.Length1 = (unsigned long)(ProgramMemStopAddress - ProgramMemStart);	//Size of program memory area
                PacketToPC.Type2 = (unsigned char)TypeEndOfTypeList;                

				if(ConfigsProtected == UNLOCKCONFIG)						
				{
                    PacketToPC.Type2 = (unsigned char)TypeConfigWords;
                    PacketToPC.Address2 = (unsigned long)ConfigWordsStartAddress;
                    PacketToPC.Length2 = (unsigned long)(ConfigWordsStopAddress - ConfigWordsStartAddress);
                    PacketToPC.Type3 = (unsigned char)TypeEndOfTypeList;
				}
				
				//Init pad bytes to 0x00...  Already done after we received the QUERY_DEVICE command (just after calling HIDRxPacket()).
	
                if(!USBHandleBusy(USBInHandle))
				{
                    USBInHandle = USBTxOnePacket(HID_EP,(BYTE*)&PacketToPC,64);
					BootState = IdleState;
				}
			}
				break;
			case UNLOCK_CONFIG:
			{
message.handled = 1;	/* Prevent bootloader exit */
				if(PacketFromPC.LockValue == UNLOCKCONFIG)
				{
					MaxPageToErase = MaxPageToEraseWithConfigs;		//Assume we will not allow erase/programming of config words (unless host sends override command)
					ProgramMemStopAddress = ProgramMemStopWithConfigs;
					ConfigsProtected = UNLOCKCONFIG;
				}
				else	//LockValue must be == LOCKCONFIG
				{
					MaxPageToErase = MaxPageToEraseNoConfigs;		
					ProgramMemStopAddress = ProgramMemStopNoConfigs;
					ConfigsProtected = LOCKCONFIG;
				}
				BootState = IdleState;
			}
				break;
			case ERASE_DEVICE:
			{
message.handled = 1;	/* Prevent bootloader exit */
				for(ErasePageTracker = BeginPageToErase; ErasePageTracker < (MaxPageToErase + 1); ErasePageTracker++)
				{
					EraseFlash();
					USBDeviceTasks(); 	//Call USBDriverService() periodically to prevent falling off the bus if any SETUP packets should happen to arrive.
				}

                NVMCONbits.WREN = 0;		//Good practice to clear WREN bit anytime we are not expecting to do erase/write operations, further reducing probability of accidental activation.
				BootState = IdleState;				
			}
				break;
			case PROGRAM_DEVICE:
			{
message.handled = 1;	/* Prevent bootloader exit */
				if(ProgrammedPointer == (unsigned long)InvalidAddress)
					ProgrammedPointer = PacketFromPC.Address;
				
				if(ProgrammedPointer == (unsigned long)PacketFromPC.Address)
				{
					for(i = 0; i < (PacketFromPC.Size/WORDSIZE); i++)
					{
                        unsigned int index;

                        index = (RequestDataBlockSize-PacketFromPC.Size)/WORDSIZE+i;
						ProgrammingBuffer[BufferedDataIndex] = PacketFromPC.Data[(RequestDataBlockSize-PacketFromPC.Size)/WORDSIZE+i];	//Data field is right justified.  Need to put it in the buffer left justified.
						BufferedDataIndex++;
						ProgrammedPointer++;
						if(BufferedDataIndex == (RequestDataBlockSize/WORDSIZE))	//Need to make sure it doesn't call WriteFlashSubBlock() unless BufferedDataIndex/2 is an integer
						{
							WriteFlashSubBlock();
						}
					}
				}
				//else host sent us a non-contiguous packet address...  to make this firmware simpler, host should not do this without sending a PROGRAM_COMPLETE command in between program sections.
				BootState = IdleState;
			}
				break;
			case PROGRAM_COMPLETE:
			{
message.handled = 1;	/* Prevent bootloader exit */
				WriteFlashSubBlock();
				ProgrammedPointer = InvalidAddress;		//Reinitialize pointer to an invalid range, so we know the next PROGRAM_DEVICE will be the start address of a contiguous section.
				BootState = IdleState;
			}
				break;
			case GET_DATA:
			{
message.handled = 1;	/* Prevent bootloader exit */
                if(!USBHandleBusy(USBInHandle))
				{
					//Init pad bytes to 0x00...  Already done after we received the QUERY_DEVICE command (just after calling HIDRxReport()).
					PacketToPC.Command = GET_DATA;
					PacketToPC.Address = PacketFromPC.Address;
					PacketToPC.Size = PacketFromPC.Size;
					
					for(i = 0; i < (PacketFromPC.Size/2); i=i+2)
					{
						FlashMemoryValue.Val = ReadProgramMemory(PacketFromPC.Address + i);
						PacketToPC.Data[RequestDataBlockSize/WORDSIZE + i - PacketFromPC.Size/WORDSIZE] = FlashMemoryValue.word.LW;		//Low word, pure 16-bits of real data
						FlashMemoryValue.byte.MB = 0x00;	//Set the "phantom byte" = 0x00, since this is what is in the .HEX file generatd by MPLAB.  
															//Needs to be 0x00 so as to match, and successfully verify, even though the actual table read yeilded 0xFF for this phantom byte.
						PacketToPC.Data[RequestDataBlockSize/WORDSIZE + i + 1 - PacketFromPC.Size/WORDSIZE] = FlashMemoryValue.word.HW;	//Upper word, which contains the phantom byte
					}

                    USBInHandle = USBTxOnePacket(HID_EP,(BYTE*)&PacketToPC.Contents[0],64);
					BootState = IdleState;
				}
				
			}
				break;
			case RESET_DEVICE:
			{
// Cause main loop to exit and run program
__asm__("goto 0x1510"); // Changed to reset to the newly loaded code
/*
				U1CON = 0x0000;				//Disable USB module
				//And wait awhile for the USB cable capacitance to discharge down to disconnected (SE0) state. 
				//Otherwise host might not realize we disconnected/reconnected when we do the reset.
				//A basic for() loop decrementing a 16 bit number would be simpler, but seems to take more code space for
				//a given delay.  So do this instead:
				for(j = 0; j < 0xFFFF; j++)
				{
					Nop();
				}
				Reset();
*/
			}
				break;
		}//End switch
	}//End if/else

}//End ProcessIO()



void EraseFlash(void)
{
	DWORD_VAL MemAddressToErase = {0x00000000};
	MemAddressToErase.Val = (((DWORD)ErasePageTracker) << 10);

	NVMCON = 0x4042;				//Erase page on next WR

	TBLPAG = MemAddressToErase.byte.UB;
	__builtin_tblwtl(MemAddressToErase.word.LW, 0xFFFF);

	asm("DISI #16");					//Disable interrupts for next few instructions for unlock sequence
	__builtin_write_NVM();
    while(NVMCONbits.WR == 1){}

//	EECON1bits.WREN = 0;  //Good practice now to clear the WREN bit, as further protection against any future accidental activation of self write/erase operations.
}	


void WriteFlashSubBlock(void)		//Use word writes to write code chunks less than a full 64 byte block size.
{
	unsigned int i = 0;
	DWORD_VAL Address;

	NVMCON = 0x4003;		//Perform WORD write next time WR gets set = 1.

	while(BufferedDataIndex > 0)		//While data is still in the buffer.
	{
		Address.Val = ProgrammedPointer - BufferedDataIndex;
		TBLPAG = Address.word.HW;
		
		__builtin_tblwtl(Address.word.LW, ProgrammingBuffer[i]);		//Write the low word to the latch
		__builtin_tblwth(Address.word.LW, ProgrammingBuffer[i + 1]);	//Write the high word to the latch (8 bits of data + 8 bits of "phantom data")
		i = i + 2;

		asm("DISI #16");					//Disable interrupts for next few instructions for unlock sequence
		__builtin_write_NVM();
        while(NVMCONbits.WR == 1){}

		BufferedDataIndex = BufferedDataIndex - 2;		//Used up 2 (16-bit) words from the buffer.
	}

	NVMCONbits.WREN = 0;		//Good practice to clear WREN bit anytime we are not expecting to do erase/write operations, further reducing probability of accidental activation.
}


/*********************************************************************
 * Function:        DWORD ReadProgramMemory(DWORD address)
 *
 * PreCondition:    None
 *
 * Input:           Program memory address to read from.  Should be 
 *                            an even number.
 *
 * Output:          Program word at the specified address.  For the 
 *                            PIC24, dsPIC, etc. which have a 24 bit program 
 *                            word size, the upper byte is 0x00.
 *
 * Side Effects:    None
 *
 * Overview:        Modifies and restores TBLPAG.  Make sure that if 
 *                            using interrupts and the PSV feature of the CPU 
 *                            in an ISR that the TBLPAG register is preloaded 
 *                            with the correct value (rather than assuming 
 *                            TBLPAG is always pointing to the .const section.
 *
 * Note:            None
 ********************************************************************/
DWORD ReadProgramMemory(DWORD address) 
{  
      DWORD_VAL dwvResult;
    WORD wTBLPAGSave;
 
      wTBLPAGSave = TBLPAG;
    TBLPAG = ((DWORD_VAL*)&address)->w[1];

    dwvResult.w[1] = __builtin_tblrdh((WORD)address);
    dwvResult.w[0] = __builtin_tblrdl((WORD)address);
    TBLPAG = wTBLPAGSave;
 
      return dwvResult.Val;
}



// ******************************************************************************************************
// ************** USB Callback Functions ****************************************************************
// ******************************************************************************************************
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

// The USBCBSendResume() function is special, in that the USB stack will not automatically call this
// function.  This function is meant to be called from the application firmware instead.  See the
// additional comments near the function.

/******************************************************************************
 * Function:        void USBCBSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Call back that is invoked when a USB suspend is detected
 *
 * Note:            None
 *****************************************************************************/
void USBCBSuspend(void)
{
	//Example power saving code.  Insert appropriate code here for the desired
	//application behavior.  If the microcontroller will be put to sleep, a
	//process similar to that shown below may be used:
	
	//ConfigureIOPinsForLowPower();
	//SaveStateOfAllInterruptEnableBits();
	//DisableAllInterruptEnableBits();
	//EnableOnlyTheInterruptsWhichWillBeUsedToWakeTheMicro();	//should enable at least USBActivityIF as a wake source
	//Sleep();
	//RestoreStateOfAllPreviouslySavedInterruptEnableBits();	//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.
	//RestoreIOPinsToNormal();									//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.

	//IMPORTANT NOTE: Do not clear the USBActivityIF (ACTVIF) bit here.  This bit is 
	//cleared inside the usb_device.c file.  Clearing USBActivityIF here will cause 
	//things to not work as intended.	
	

    #if defined(__C30__)
    #if 0
        U1EIR = 0xFFFF;
        U1IR = 0xFFFF;
        U1OTGIR = 0xFFFF;
        IFS5bits.USB1IF = 0;
        IEC5bits.USB1IE = 1;
        U1OTGIEbits.ACTVIE = 1;
        U1OTGIRbits.ACTVIF = 1;
        Sleep();
    #endif
    #endif
}


/******************************************************************************
 * Function:        void _USB1Interrupt(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when the USB interrupt bit is set
 *					In this example the interrupt is only used when the device
 *					goes to sleep when it receives a USB suspend command
 *
 * Note:            None
 *****************************************************************************/
#if 0
void __attribute__ ((interrupt)) _USB1Interrupt(void)
{
    #if !defined(self_powered)
        if(U1OTGIRbits.ACTVIF)
        {
            IEC5bits.USB1IE = 0;
            U1OTGIEbits.ACTVIE = 0;
            IFS5bits.USB1IF = 0;
        
            //USBClearInterruptFlag(USBActivityIFReg,USBActivityIFBitNum);
            USBClearInterruptFlag(USBIdleIFReg,USBIdleIFBitNum);
            //USBSuspendControl = 0;
        }
    #endif
}
#endif

/******************************************************************************
 * Function:        void USBCBWakeFromSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The host may put USB peripheral devices in low power
 *					suspend mode (by "sending" 3+ms of idle).  Once in suspend
 *					mode, the host may wake the device back up by sending non-
 *					idle state signalling.
 *					
 *					This call back is invoked when a wakeup from USB suspend 
 *					is detected.
 *
 * Note:            None
 *****************************************************************************/
void USBCBWakeFromSuspend(void)
{
	// If clock switching or other power savings measures were taken when
	// executing the USBCBSuspend() function, now would be a good time to
	// switch back to normal full power run mode conditions.  The host allows
	// a few milliseconds of wakeup time, after which the device must be 
	// fully back to normal, and capable of receiving and processing USB
	// packets.  In order to do this, the USB module must receive proper
	// clocking (IE: 48MHz clock must be available to SIE for full speed USB
	// operation).
}

/********************************************************************
 * Function:        void USBCB_SOF_Handler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB host sends out a SOF packet to full-speed
 *                  devices every 1 ms. This interrupt may be useful
 *                  for isochronous pipes. End designers should
 *                  implement callback routine as necessary.
 *
 * Note:            None
 *******************************************************************/
void USBCB_SOF_Handler(void)
{
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
}

/*******************************************************************
 * Function:        void USBCBErrorHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The purpose of this callback is mainly for
 *                  debugging during development. Check UEIR to see
 *                  which error causes the interrupt.
 *
 * Note:            None
 *******************************************************************/
void USBCBErrorHandler(void)
{
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


/*******************************************************************
 * Function:        void USBCBCheckOtherReq(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        When SETUP packets arrive from the host, some
 * 					firmware must process the request and respond
 *					appropriately to fulfill the request.  Some of
 *					the SETUP packets will be for standard
 *					USB "chapter 9" (as in, fulfilling chapter 9 of
 *					the official USB specifications) requests, while
 *					others may be specific to the USB device class
 *					that is being implemented.  For example, a HID
 *					class device needs to be able to respond to
 *					"GET REPORT" type of requests.  This
 *					is not a standard USB chapter 9 request, and 
 *					therefore not handled by usb_device.c.  Instead
 *					this request should be handled by class specific 
 *					firmware, such as that contained in usb_function_hid.c.
 *
 * Note:            None
 *******************************************************************/
void USBCBCheckOtherReq(void)
{
    USBCheckHIDRequest();
}//end


/*******************************************************************
 * Function:        void USBCBStdSetDscHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USBCBStdSetDscHandler() callback function is
 *					called when a SETUP, bRequest: SET_DESCRIPTOR request
 *					arrives.  Typically SET_DESCRIPTOR requests are
 *					not used in most applications, and it is
 *					optional to support this type of request.
 *
 * Note:            None
 *******************************************************************/
void USBCBStdSetDscHandler(void)
{
    // Must claim session ownership if supporting this request
}//end


/*******************************************************************
 * Function:        void USBCBInitEP(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when the device becomes
 *                  initialized, which occurs after the host sends a
 * 					SET_CONFIGURATION (wValue not = 0) request.  This 
 *					callback function should initialize the endpoints 
 *					for the device's usage according to the current 
 *					configuration.
 *
 * Note:            None
 *******************************************************************/
void USBCBInitEP(void)
{
    //enable the HID endpoint
    USBEnableEndpoint(HID_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    //Arm the OUT endpoint for the first packet
    USBOutHandle = HIDRxPacket(HID_EP,(BYTE*)&PacketFromPCBuffer,64);
}

/********************************************************************
 * Function:        void USBCBSendResume(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB specifications allow some types of USB
 * 					peripheral devices to wake up a host PC (such
 *					as if it is in a low power suspend to RAM state).
 *					This can be a very useful feature in some
 *					USB applications, such as an Infrared remote
 *					control	receiver.  If a user presses the "power"
 *					button on a remote control, it is nice that the
 *					IR receiver can detect this signalling, and then
 *					send a USB "command" to the PC to wake up.
 *					
 *					The USBCBSendResume() "callback" function is used
 *					to send this special USB signalling which wakes 
 *					up the PC.  This function may be called by
 *					application firmware to wake up the PC.  This
 *					function will only be able to wake up the host if
 *                  all of the below are true:
 *					
 *					1.  The USB driver used on the host PC supports
 *						the remote wakeup capability.
 *					2.  The USB configuration descriptor indicates
 *						the device is remote wakeup capable in the
 *						bmAttributes field.
 *					3.  The USB host PC is currently sleeping,
 *						and has previously sent your device a SET 
 *						FEATURE setup packet which "armed" the
 *						remote wakeup capability.   
 *
 *                  If the host has not armed the device to perform remote wakeup,
 *                  then this function will return without actually performing a
 *                  remote wakeup sequence.  This is the required behavior, 
 *                  as a USB device that has not been armed to perform remote 
 *                  wakeup must not drive remote wakeup signalling onto the bus;
 *                  doing so will cause USB compliance testing failure.
 *                  
 *					This callback should send a RESUME signal that
 *                  has the period of 1-15ms.
 *
 * Note:            This function does nothing and returns quickly, if the USB
 *                  bus and host are not in a suspended condition, or are 
 *                  otherwise not in a remote wakeup ready state.  Therefore, it
 *                  is safe to optionally call this function regularly, ex: 
 *                  anytime application stimulus occurs, as the function will
 *                  have no effect, until the bus really is in a state ready
 *                  to accept remote wakeup. 
 *
 *                  When this function executes, it may perform clock switching,
 *                  depending upon the application specific code in 
 *                  USBCBWakeFromSuspend().  This is needed, since the USB
 *                  bus will no longer be suspended by the time this function
 *                  returns.  Therefore, the USB module will need to be ready
 *                  to receive traffic from the host.
 *
 *                  The modifiable section in this routine may be changed
 *                  to meet the application needs. Current implementation
 *                  temporary blocks other functions from executing for a
 *                  period of ~3-15 ms depending on the core frequency.
 *
 *                  According to USB 2.0 specification section 7.1.7.7,
 *                  "The remote wakeup device must hold the resume signaling
 *                  for at least 1 ms but for no more than 15 ms."
 *                  The idea here is to use a delay counter loop, using a
 *                  common value that would work over a wide range of core
 *                  frequencies.
 *                  That value selected is 1800. See table below:
 *                  ==========================================================
 *                  Core Freq(MHz)      MIP         RESUME Signal Period (ms)
 *                  ==========================================================
 *                      48              12          1.05
 *                       4              1           12.6
 *                  ==========================================================
 *                  * These timing could be incorrect when using code
 *                    optimization or extended instruction mode,
 *                    or when having other interrupts enabled.
 *                    Make sure to verify using the MPLAB SIM's Stopwatch
 *                    and verify the actual signal on an oscilloscope.
 *******************************************************************/
//void USBCBSendResume(void) // Removed, not sure why this is here anyway ?
//{
//    static WORD delay_count;
//    
//    //First verify that the host has armed us to perform remote wakeup.
//    //It does this by sending a SET_FEATURE request to enable remote wakeup,
//    //usually just before the host goes to standby mode (note: it will only
//    //send this SET_FEATURE request if the configuration descriptor declares
//    //the device as remote wakeup capable, AND, if the feature is enabled
//    //on the host (ex: on Windows based hosts, in the device manager 
//    //properties page for the USB device, power management tab, the 
//    //"Allow this device to bring the computer out of standby." checkbox 
//    //should be checked).
//    if(USBGetRemoteWakeupStatus() == TRUE) 
//    {
//        //Verify that the USB bus is in fact suspended, before we send
//        //remote wakeup signalling.
//        if(USBIsBusSuspended() == TRUE)
//        {
//            USBMaskInterrupts();
//            
//            //Clock switch to settings consistent with normal USB operation.
//            USBCBWakeFromSuspend();
//            USBSuspendControl = 0; 
//            USBBusIsSuspended = FALSE;  //So we don't execute this code again, 
//                                        //until a new suspend condition is detected.
//
//            //Section 7.1.7.7 of the USB 2.0 specifications indicates a USB
//            //device must continuously see 5ms+ of idle on the bus, before it sends
//            //remote wakeup signalling.  One way to be certain that this parameter
//            //gets met, is to add a 2ms+ blocking delay here (2ms plus at 
//            //least 3ms from bus idle to USBIsBusSuspended() == TRUE, yeilds
//            //5ms+ total delay since start of idle).
//            delay_count = 3600U;        
//            do
//            {
//                delay_count--;
//            }while(delay_count);
//            
//            //Now drive the resume K-state signalling onto the USB bus.
//            USBResumeControl = 1;       // Start RESUME signaling
//            delay_count = 1800U;        // Set RESUME line for 1-13 ms
//            do
//            {
//                delay_count--;
//            }while(delay_count);
//            USBResumeControl = 0;       //Finished driving resume signalling
//
//            USBUnmaskInterrupts();
//        }
//    }
//}

/*******************************************************************
 * Function:        BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *                        USB_EVENT event, void *pdata, WORD size)
 *
 * PreCondition:    None
 *
 * Input:           USB_EVENT event - the type of event
 *                  void *pdata - pointer to the event data
 *                  WORD size - size of the event data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called from the USB stack to
 *                  notify a user application that a USB event
 *                  occured.  This callback is in interrupt context
 *                  when the USB_INTERRUPT option is selected.
 *
 * Note:            None
 *******************************************************************/
BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
    switch(event)
    {
        case EVENT_TRANSFER:
            //Add application specific callback task or callback function here if desired.
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
        case EVENT_CONFIGURED: 
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER_TERMINATED:
            //Add application specific callback task or callback function here if desired.
            //The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
            //FEATURE (endpoint halt) request on an application endpoint which was 
            //previously armed (UOWN was = 1).  Here would be a good place to:
            //1.  Determine which endpoint the transaction that just got terminated was 
            //      on, by checking the handle value in the *pdata.
            //2.  Re-arm the endpoint if desired (typically would be the case for OUT 
            //      endpoints).
            break;
        default:
            break;
    }      
    return TRUE; 
}


/** EOF main.c *************************************************/
#endif
