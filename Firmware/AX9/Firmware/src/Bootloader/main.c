/*	This bootloader was adapted form the generic microchip USB HID 
	bootloader for PIC24 MCUs. Karim Ladha, Dan Jackson 2011-2015   */
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
#define	VectorsEnd						0x00000400	// One page of vectors + general purpose program memory.

// KL: Modifications
// Modified 1-6-2015 for AX9 bootloader revision
#define NEW_BOOTLOADER
#define __APP_START_OF_CODE 			0x2000		// The exit address used when branching to the application
#define PLACEHOLDER_ADDRESS				0x2400		// Address value is unimportant. Location of the dummp flasher function.
#define PRECHARGE_FVR_THRESHOLD			420			// 2.92v, uses FVR to measure Vdd
#define PRECHARGE_LED_FLASH_TIMER		500			// Controls how fast the precharge LED flashes, ~15Hz
#define PRECHARGE_TIMEOUT				6000		// ~10 min precharge timeout
#define SOFTWARE_DETECT_TIME_OUT 		100			// Sets how long the device waits for bootloader SW (~6 sec)
/*KL: added string to describe bootloader version.*/
const char __attribute__((space(prog), address(0x380))) bootVersion[] = HARDWARE_NAME " v" HARDWARE_VERSION " BL v1.0";


//Bootloader resides in memory range 0x400-0x1FFF, blocks 0,1,..7 
#define	BeginPageToErase			8		 // 8 x 1024 = 0x2000
#define MaxPageToEraseNoConfigs		167		 // 167 x 1024 = 0x29C00. Blocks 0x2A000, 0x2A400 and 0x2A800 (config block) protected
#define MaxPageToEraseWithConfigs	169		 // 169 x 1024 = 0x2A400 (SECOND last block)

// These defines determine the ranges passed to the bootloader in reply to query
// Prog
#define ProgramMemStart				0x00002000  // First address after bootloader region
#define ProgramMemStopNoConfigs		0x0002A000  // 0x2A000 device id block is protected unless config is unlocked 
#define ProgramMemStopWithConfigs	0x0002ABF8	// 0x2ABF8 is the last word of the last block containing the cofig words 
// Config 
#define ConfigWordsStartAddress		0x0002ABFA 	// Config words are in page address 0x2A800
#define ConfigWordsStopAddress		0x0002AC00



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
unsigned char timeout_counter; // added
union { unsigned char handled : 1; } message; // added
unsigned char softwareDetected = 0;
unsigned short batt = 0;

PacketToFromPC PacketFromPC;		//64 byte buffer for receiving packets on EP1 OUT from the PC
PacketToFromPC PacketToPC;			//64 byte buffer for sending packets on EP1 IN to the PC
PacketToFromPC PacketFromPCBuffer;

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

// PROTOTYPES
int main(void);
void MyDelay(void);
void MyExit(void);
unsigned char BattStat(void);
void WaitForPrecharge(void);
void ProcessIO(void);
void EraseFlash(void);
void WriteFlashSubBlock(void);
DWORD ReadProgramMemory(DWORD);
void DummyLowPowerWaitFunction(void);

// CODE
int main(void)
{   
	static WORD led_count=0;

	// Initialise i/o and internal/external peripherals
	SystemInit();

	// Bail function - no usb voltage - goto user code
	if(!USB_BUS_SENSE) // was using RCON reset check here
	{
		MyExit();
	}

	WaitForPrecharge();	// Wait until the battery is not in precharge
	
	LED_R = LED_TURN_ON;	// Increase current draw to keep charger active - stops it resetting when no batt
	MyDelay();
	CLOCK_PLL();			// Switch to full speed for USB
	MyDelay();

	// Initialise the timeout variable
	// We have to keep clearing this bit to stop the device timeing out
	timeout_counter = 0;
	softwareDetected = 0;
	
	//   InitializeSystem(); - this is done mainly by InitIO, the remainder follows
	USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
	    				//variables to known states.
	
	// Initialize the variable holding the handle for the last
	// transmission
	USBOutHandle = 0;
	USBInHandle = 0;
	
	//Initialize bootloader state variables
	MaxPageToErase = MaxPageToEraseNoConfigs;		//Assume we will not allow erase/programming of config words (unless host sends override command)
	ProgramMemStopAddress = ProgramMemStopNoConfigs;
	ConfigsProtected = LOCKCONFIG;					//Assume we will not erase or program the vector table at first.  Must receive unlock config bits/vectors command first.
	BootState = IdleState;
	ProgrammedPointer = InvalidAddress;	
	BufferedDataIndex = 0;
	
    while(1)
    {
		// Usb tasks
        USBDeviceTasks(); 
        ProcessIO();
    		
		// Added, reset on disconnect
		if (!USB_BUS_SENSE)
		{
			// USB was disconnected - on re-entry it will goto user code
			Reset();
		}		  

		// Slow loop - flashed LED and detects timeouts
	    if (++led_count > 10000) 
		{
			led_count = 0;
			// Added	- So long as the device does not do a USB bootloader app transfer (app not running) then it will timeout here
			timeout_counter++;
			if ((!softwareDetected) && (timeout_counter > SOFTWARE_DETECT_TIME_OUT))
			{
				// Device was not bootloaded in time - goto user app with USB attached
				MyExit();
			}

			// A USB bootloader app transfer has occured - timeout will not happen now - LED flashes
			if(USBDeviceState == CONFIGURED_STATE)
		    {
		       	LED_R = !LED_R;
		    }//end if(...)
			else
			{
		    	LED_R = LED_TURN_ON;
			}
		} // end slow LED loop
    }//end while(usb_bus_sense) 
}//end main

// Delay loop
void MyDelay(void)//8.2ms delay
{
	unsigned int delay = 0xffff;        
	while (delay--){Nop();} 
}

// Alternate app behaviour is set in the linker
#define toStr(_x) #_x
#define XtoStr(_x)	toStr(_x)

// Exit to app
void MyExit(void)
{
	LED_SET(LED_OFF);
	CLOCK_INTOSC();
	U1CON = 0x0000;				//Disable USB module
	//And wait awhile for the USB cable capacitance to discharge down to disconnected (SE0) state. 
	//Otherwise host might not realize we disconnected/reconnected when we do the reset.
	//A basic for() loop decrementing a 16 bit number would be simpler, but seems to take more code space for
	//a given delay.  So do this instead:
	MyDelay();//8.2ms delay
	__asm__("goto " XtoStr(__APP_START_OF_CODE));
}


// Precharge function - light weight for bootloader
unsigned char BattStat(void)
{
	unsigned short fvr, i;
	PMD1bits.ADC1MD = 0;    // See device errata
	AD1CON1bits.ADON = 0;

	AD1CON1 = 0x00E0; 	// Scan using Vdd as ref
	AD1CON2 = 0x0000; 	// Sample channel A
	AD1CON3 = 0x1414; 	// Tad=Tcy/20,Tsamp=20Tad
	AD1CHS = 0x1111;	// Set channel A to band gap
	AD1CON1bits.ADON = 1;

	// Wait for ADC to power up
	for(i=100;i>0;i--)Nop(); 

	// FVR on and > 1ms delay for FVR settle on
	AD1PCFGH = ADC_SELECT_H & 0xfffd; 
	AD1PCFGL = ADC_SELECT_L;
	for(i=10000;i>0;i--)Nop(); 

	// Sample
	IFS0bits.AD1IF = 0;	
	AD1CON1bits.SAMP = 1;
	while (!IFS0bits.AD1IF);
	fvr = ADC1BUF0;

	// Fvr off, undocumented errata, fvr settle off
	AD1PCFGH = ADC_SELECT_H; 
	AD1PCFGL = ADC_SELECT_L;
	for(i=100;i>0;i--)Nop(); 

	AD1CON1bits.ADON = 0;
	PMD1bits.ADC1MD = 1; // See device errata

	// Check Vdd using 1.2v fvr, not accurate but ok
	if(fvr < PRECHARGE_FVR_THRESHOLD) 
		return TRUE; // Vdd > precharge threshold
	else
		return FALSE; // In precherge
}

// Precharge -- startup code to cope with battery pre-charge state 
void WaitForPrecharge(void)
{
	unsigned short timeout = PRECHARGE_TIMEOUT; // KL: To allow BL exit if battery is broken 60000 ~= 10min
    unsigned short timer = 0;
	unsigned short success = 0;

	// Slow down clock to 1MHz
	CLKDIVbits.RCDIV = 0b011;

	// Get initial value
	BattStat();

	// Sample the battery
    for (;;)
    {
		ClrWdt();   // Reset WDT while we're in this loop
		timer++;

		if ((!USB_BUS_SENSE)||(!timeout))
		{
			// USB removed or timeout - exit precharge
			break;
		}

		// If battery is flat - red low duty flash
		if(timer > PRECHARGE_LED_FLASH_TIMER - 10)
		{
			LED_R = LED_TURN_ON;
		}
		if (timer > PRECHARGE_LED_FLASH_TIMER)			
		{
			LED_R = LED_TURN_OFF;
			timer = 0;
			// Check battery voltage
			if (BattStat()) 
			{
				if (++success>5)break;// Not in precharge		
			}
			else
			{
				success = 0;
				timeout--;
			}
		}
    }

	CLKDIVbits.RCDIV = 0;
	return;    
}

/*
	KL: Low power 'dummy' function, 50uA current for the AX9. Prevents battery discharge if there 
	is a delay between soldering the battery on and programming. The battery will still discharge
	but it should take ~100 days.
*/
void __attribute__((address(PLACEHOLDER_ADDRESS)))DummyLowPowerWaitFunction(void)
{
	unsigned char i = 0;
	InitIOBoot();
	RCONbits.SWDTEN = 1;
	while(!USB_BUS_SENSE)
	{
		LED_SET(i++);
		Nop();Nop();Nop();Nop();Nop();
		Nop();Nop();Nop();Nop();Nop();
		LED_SET(LED_OFF);
		Sleep();
	}
	RCONbits.SWDTEN = 0;
	CLOCK_INTOSC();
	Reset();
}

// Modded to allow timeout
void ProcessIO(void) 
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
softwareDetected = 1;	/* Prevent bootloader exit - this is the one the bootloader app first sets */
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
softwareDetected = 1;	/* Prevent bootloader exit */
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
softwareDetected = 1;	/* Prevent bootloader exit */
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
softwareDetected = 1;	/* Prevent bootloader exit */
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
softwareDetected = 1;	/* Prevent bootloader exit */
				WriteFlashSubBlock();
				ProgrammedPointer = InvalidAddress;		//Reinitialize pointer to an invalid range, so we know the next PROGRAM_DEVICE will be the start address of a contiguous section.
				BootState = IdleState;
			}
				break;
			case GET_DATA:
			{
softwareDetected = 1;	/* Prevent bootloader exit */
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
				MyExit();// Changed to reset to the newly loaded code
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

//KL: Block config writes from bootloader, block writes to id unless enabled
if((Address.Val < ConfigWordsStartAddress)&&(Address.Val < ProgramMemStopAddress))
{
		asm("DISI #16");					//Disable interrupts for next few instructions for unlock sequence
		__builtin_write_NVM();
        while(NVMCONbits.WR == 1){}
}

		BufferedDataIndex = BufferedDataIndex - 2;		//Used up 2 (16-bit) words from the buffer.
	}

	NVMCONbits.WREN = 0;		//Good practice to clear WREN bit anytime we are not expecting to do erase/write operations, further reducing probability of accidental activation.
}
DWORD ReadProgramMemory(DWORD address) 
{  
    DWORD_VAL dwvResult;
    WORD wTBLPAGSave;
 
    wTBLPAGSave = TBLPAG;
    TBLPAG = ((DWORD_VAL*)&address)->w[1];

    dwvResult.w[1] = __builtin_tblrdh((WORD)address);
    dwvResult.w[0] = __builtin_tblrdl((WORD)address);
    TBLPAG = wTBLPAGSave;

// KL: Patch for misbehaving mplab/c30 regarding verifiying upper bytes of config
if(address > ConfigWordsStartAddress)
{
	dwvResult.word.HW = 0;
}
      return dwvResult.Val;
}

// Place holders for unimplemented usb functions
void USBCBSuspend(void){}
void USBCBWakeFromSuspend(void){}
void USBCB_SOF_Handler(void){}
void USBCBErrorHandler(void){}
void USBCBCheckOtherReq(void){USBCheckHIDRequest();}//end
void USBCBStdSetDscHandler(void){}

// Usb functions
void USBCBInitEP(void)
{
    //enable the HID endpoint
    USBEnableEndpoint(HID_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    //Arm the OUT endpoint for the first packet
    USBOutHandle = HIDRxPacket(HID_EP,(BYTE*)&PacketFromPCBuffer,64);
}
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
