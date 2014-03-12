// Based on Microchip's USB Bootloader example (see below).
// Changes Copyright Newcastle University (see below).

/* 
 * Copyright (c) 2013-2014, Newcastle University, UK.
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
 
 /*********************************************************************
 *
 * Microchip USB C18 Firmware -  HID Bootloader Version 1.0 for PIC18F87J50 Family Devices
 *
 *********************************************************************
 * FileName:        Boot87J50Family.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 2.30.01+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PICmicro® Microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * File version         Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 1.0					04/09/2008	Original
 ********************************************************************/

// Karim Ladha, 2013-2014
// Bootloader for the WAX9 using PIC24FJ64GB004

/*
KL 06-09-2013
Bootloader for the WAX9 using PIC24FJ64GB004
Must also bootload EEPROM to load init script, onlt first 16k (0x4000) is mapped, remainder is user space
Timout no longer applicable since we always run bootloader on attach
LED codes for precharge and batt status different from other loaders
Based on microchips HID bootloader - licenses conditions persist:
	Part of this project belongs to and is copyrighted by microchip inc
	That part is freely usable and distributable for use with microchips mcu products
*/
#ifndef USBMOUSE_C
#define USBMOUSE_C

/** INCLUDES *******************************************************/
#include <compiler.h>
#include <p24Fxxxx.h>
#include "./USB/usb.h"
#include "HardwareProfile.h"
#include "./USB/usb_function_hid.h"
#include "Peripherals/Eeprom.h"

/** CONFIGURATION **************************************************/
/*24 bit config 1 0b<unimp>0000<unimp>0000<res>0<jtag off>0<code prot off>1<prog writes on>1<debug off>1<res>1<EMUD1>11<wdtoff>0<stdrd wdt>1<unimp>0<wdt pres 32>1<wdt posc see table>****>*/ 
/*WDT bits -> 4s timout*/
_CONFIG1(0b000000000011111101001100) 
/*24 bit config 2 0b<unimp>0000<unimp>0000<IESO off>0<plldiv2>001<plldis off>0<init osc = frc>000<clock switch en>01<no clk op>0<iolock on-off en>0<usb vreg off>1<res>1<prim osc off>11>*/
_CONFIG2(0b000000000001000001001111)
/*Disableing SOSC pins to enable external clock*/
_CONFIG3(0b1111110011111111)

/** C O N S T A N T S **********************************************************/
#define ProgramMemStart					0x00001C00 	//Beginning of application program memory (not occupied by bootloader).  **THIS VALUE MUST BE ALIGNED WITH BLOCK BOUNDRY** Also, in order to work correctly, make sure the StartPageToErase is set to erase this section.
#define ExtEepromMemStart				0x00100000  // Biggest ever PIC <1MB, eeprom mapped to 1MB
#define ExtEepromMemSize			 	0x10000		// Only this ammount is exposed to the bootloader

//64KB variants
#if defined(__PIC24FJ64GB004__) || defined(__PIC24FJ64GB002__)

	#define __APP_START_OF_CODE 		0x1C00
	#define	BeginPageToErase			7		 //Bootloader and vectors occupy first six 1024 word (1536 bytes due to 25% unimplemented bytes) pages
	#define MaxPageToEraseNoConfigs		41		 //Last full page of flash on the PIC24FJ256GB110, which does not contain the flash configuration words.
	#define MaxPageToEraseWithConfigs	42		 //Page 170 contains the flash configurations words on the PIC24FJ256GB110.  Page 170 is also smaller than the rest of the (1536 byte) pages.
	#define ProgramMemStopNoConfigs		0x0000A800 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: 
                                                        //IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	

	#define ProgramMemStopWithConfigs	0x0000ABF8 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	
	#define ConfigWordsStartAddress		0x0000ABF8 //0x2ABFA is start of CW3 on PIC24FJ256GB110 Family devices
    #define ConfigWordsStopAddress		0x0000AC00

//32KB variants
#elif defined(__PIC24FJ32GB004__) || defined(__PIC24FJ32GB002__)

	#define __APP_START_OF_CODE 		0x1C00
	#define	BeginPageToErase			7		 //Bootloader and vectors occupy first six 1024 word (1536 bytes due to 25% unimplemented bytes) pages
	#define MaxPageToEraseNoConfigs		20		 //Last full page of flash on the PIC24FJ256GB110, which does not contain the flash configuration words.
	#define MaxPageToEraseWithConfigs	21		 //Page 170 contains the flash configurations words on the PIC24FJ256GB110.  Page 170 is also smaller than the rest of the (1536 byte) pages.
	#define ProgramMemStopNoConfigs		0x00005400 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: 
                                                        //IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	

	#define ProgramMemStopWithConfigs	0x000057F8 //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: IE: If AddressToStopPopulating = 0x200, 0x1FF is the last programmed address (0x200 not programmed)	
	#define ConfigWordsStartAddress		0x000057F8 //0x2ABFA is start of CW3 on PIC24FJ256GB110 Family devices
    #define ConfigWordsStopAddress		0x00005800

#else
    #error "Device not supported by this boot loader project.  Please select the correct project for your part."
#endif

// SET THESE TO REBOOT LOADER ONLY
#ifdef REBOOT
	#warning "REBOOT ENABLED"
	#undef 	ProgramMemStart
	#undef BeginPageToErase
	#undef MaxPageToEraseNoConfigs
	#undef MaxPageToEraseWithConfigs			// DO NOT TRY TO WRITE CONFIG WITH RE_BOOT		
	#undef ProgramMemStopNoConfigs
	#undef ProgramMemStopWithConfigs
	#undef __APP_START_OF_CODE

	#define __APP_START_OF_CODE 		0x00000000
	// Note: erasing first page makes first address an illegal opcode so just skip it (leave jump table intact)
	#define	BeginPageToErase			1		 //Bootloader and vectors occupy first six 1024 word (1536 bytes due to 25% unimplemented bytes) pages
	#define MaxPageToEraseNoConfigs		5		 //Last full page of flash on the PIC24FJ256GB110, which does not contain the flash configuration words.
	#define MaxPageToEraseWithConfigs	5		 //Page 170 contains the flash configurations words on the PIC24FJ256GB110.  Page 170 is also smaller than the rest of the (1536 byte) pages.
	#define ProgramMemStart				0
	#define ProgramMemStopNoConfigs		0x1C00   //Must be instruction word aligned address.  This address does not get updated, but the one just below it does: 
	#define ProgramMemStopWithConfigs	0x1C00
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
#define RequestDataBlockSize 		56	//Number of bytes in the "Data" field of a standard request to/from the PC.  Must be an even number from 2 to 56. 
#define BufferSize 					0x20	//32 16-bit words of buffer

/** PRIVATE PROTOTYPES *********************************************/
void ProcessIO(void);
void EraseFlash(void);
void WriteFlashSubBlock(void);
DWORD ReadProgramMemory(DWORD);

// My prototypes
void EepromErase(void);
void WaitForPrecharge(void);
void MyDelay(void);
void MyExit(void);
unsigned short GetBatt(void);
void TimedTasks(void);

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
unsigned long eepromEraseTracker;
unsigned short ProgrammingBuffer[BufferSize];
unsigned char BufferedDataIndex;
unsigned long ProgrammedPointer;
unsigned char ConfigsProtected;

// Status
unsigned char softwareDetected = 0;
unsigned short batt = 0;
unsigned long writing = 0;
#define WRITING_PERSIST_VAL 100000 //~10sec

// For debugging
unsigned long gAddrRead = 0; // Last address read


// CODE
void MyDelay(void)//8.2ms delay
{
	unsigned int delay = 0xffff;        
	while (delay--){Nop();} 
}

// Alternate app behaviour is set in the linker
#define toStr(_x) #_x
#define XtoStr(_x)	toStr(_x)

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

// Timed tasks - LED codes etc..
void TimedTasks(void)
{
	static unsigned char battFullLatch = 0, toggle = 0;
	static unsigned short battUpdate = 0;
	unsigned short temp;
	// Update batt value periodically
	if(!battUpdate--)
	{
		battUpdate = 10000; // Used below as timer for leds 
		GetBatt();
	}

	// Writing
	if (writing)
	{
		writing--;
		// Just toggle red so it blinks a bit
		if(battUpdate == 5000){LED_R = !LED_R;}
		return;
	}

	// Battery status - red = empty/busy, green flash = full, yellow = charging
	if (batt < BATT_CHARGE_MIN_SAFE)		
	{
		LED_SET(LED_RED);
		battFullLatch = 0;
	}
	else if (battFullLatch > 10) // Latch charged state
	{
		if(battUpdate == 10000)
		{
			if(++toggle&0b1){LED_SET(LED_GREEN);}
			else			{LED_SET(LED_OFF);}
		}	
	}
	else if (batt >= BATT_CHARGE_FULL_USB)	// Flash green when full
	{
		if(battUpdate == 10000)
		{
			battFullLatch++;
			if(++toggle&0b1){LED_SET(LED_GREEN);}
			else			{LED_SET(LED_OFF);}
		}
	}
	else
	{
		static unsigned short timer = 0;
		battFullLatch = 0;
		temp = batt - BATT_CHARGE_MIN_SAFE; // temp is 0 - 179
		// PWM battery status, red -> yellow -> green
		if(!timer--){timer=179;}			
		if(temp>=timer)	{LED_SET(LED_GREEN);}
		else 			{LED_SET(LED_RED);}
	}

	return;
}


int main(void)
{   
	#ifdef __DEBUG
		RCON = 0;
	#endif

	InitIOBootloader();

	WaitForPrecharge();	// Wait until the battery is not in precharge

	// Bail function - no usb voltage - goto user code
	if(!USB_BUS_SENSE) // was using RCON reset check here
	{
		MyExit();
	}

	// Check for eeprom
	I2C_eeprom_present();

	// Increase current draw to keep charger active - stops it resetting when no batt
	if (eepromPresent) 	{LED_SET(LED_YELLOW);}
	else 				{LED_SET(LED_RED);} // Subtle red flash indicates hw has no eeprom
	MyDelay();			//8.2ms delay
	CLOCK_PLL();		//Switch to full speed for USB
	MyDelay();			//8.2ms delay
	LED_SET(LED_OFF);	

	// Initialise the timeout variable
	// We have to keep clearing this bit to stop the device timeing out
	softwareDetected = 0;

	//   InitializeSystem(); - this is done mainly by InitIO, the remainder follows
	USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware variables to known states.

	//initialize the variable holding the handle for the last
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


    #if defined(USB_INTERRUPT)
        USBDeviceAttach();
    #endif

    while(USB_BUS_SENSE)
    {
        USBDeviceTasks(); 
        ProcessIO();   
		TimedTasks();    
	}

	MyExit();
	return 0;

}//end main

// Precharge function - light weight for bootloader
unsigned short GetBatt(void)
{
	PMD1bits.ADC1MD = 0;    // See device errata

	AD1CON1 = 0x40E5;
	AD1CON2 = 0x8400;
	AD1CON3 = 0xD413;
	AD1CSSL = 0x0800;
	AD1PCFG = 0xf7ff;
	AD1CON1bits.ADON = 1;

	IFS0bits.AD1IF = 0;		// Clear interrupt flag
	AD1CON1bits.ASAM = 1;	// Begin auto sampling
	while (!IFS0bits.AD1IF);// Wait for the allotted number of conversions set by ADC_INTR_?_CONV in config2
	AD1CON1bits.ASAM = 0;	// Stop auto sampling
	while(!AD1CON1bits.DONE);// Wait for partially complete samples

	if(batt==0)batt=ADC1BUF0;

	if(ADC1BUF0>batt)batt++;
	else batt--; 
	
	AD1CON1bits.ADON = 0;

	return batt;
}


// Precharge -- startup code to cope with battery pre-charge state 
void WaitForPrecharge(void)
{
    unsigned short timer = 100;
	unsigned short success = 0;

	// 32 kHz rc osc
	CLOCK_INTRC();
	GetBatt();

	// Sample the battery
    for (;;)
    {
		ClrWdt();   // Reset WDT while we're in this loop
		timer++;

		if (!USB_BUS_SENSE)
		{
			// USB removed - reset bootloader
			break;
		}

		// if battery is flat - red 5% flash @ ~2Hz
		if(timer>100)
		{
			LED_R = 1;
		}
		if (timer>105)			
		{
			LED_R = 0;
			timer = 0;
			// Check battery voltage
			batt = GetBatt();
			if (batt > BATT_CHARGE_MIN_SAFE) //3.1v
			{
				if (++success>5)break;// Not in precharge		
			}
			else
			{
				success = 0;
			}
		}
    }

	CLOCK_INTOSC();
	return;    
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
                //PacketToPC.Type2 = (unsigned char)TypeEndOfTypeList;                

                PacketToPC.Type2 = (unsigned char)TypeEEPROM;
                PacketToPC.Address2 = (unsigned long)ExtEepromMemStart;
                PacketToPC.Length2 = (unsigned long)(ExtEepromMemSize);	//Size of eeprom
                PacketToPC.Type3 = (unsigned char)TypeEndOfTypeList; 


				if(ConfigsProtected == UNLOCKCONFIG)						
				{
                    PacketToPC.Type3 = (unsigned char)TypeConfigWords;
                    PacketToPC.Address3 = (unsigned long)ConfigWordsStartAddress;
                    PacketToPC.Length3 = (unsigned long)(ConfigWordsStopAddress - ConfigWordsStartAddress);
                    PacketToPC.Type4 = (unsigned char)TypeEndOfTypeList;
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
LED_SET(LED_RED);
writing = WRITING_PERSIST_VAL;
softwareDetected = 1;	/* Prevent bootloader exit */
				for(ErasePageTracker = BeginPageToErase; ErasePageTracker < (MaxPageToErase + 1); ErasePageTracker++)
				{
					EraseFlash();
					USBDeviceTasks(); 	//Call USBDriverService() periodically to prevent falling off the bus if any SETUP packets should happen to arrive.
				}
#ifdef ERASE_EEPROM
				// If there is any need to erase eeprom, do it here
				for (eepromEraseTracker = 0;eepromEraseTracker<EEPROM_SIZE_BYTES;eepromEraseTracker+=WRITE_PAGE_SIZE)
				{
					EepromErase();
					USBDeviceTasks();
				}
#endif
                NVMCONbits.WREN = 0;		//Good practice to clear WREN bit anytime we are not expecting to do erase/write operations, further reducing probability of accidental activation.
				BootState = IdleState;				
			}
				break;
			case PROGRAM_DEVICE:
			{
LED_SET(LED_RED);
writing = WRITING_PERSIST_VAL;
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
LED_SET(LED_RED);
writing = WRITING_PERSIST_VAL;
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
				MyExit();
			}
				break;
		}//End switch
	}//End if/else

}//End ProcessIO()



void EraseFlash(void)
{
	DWORD_VAL MemAddressToErase = {0x00000000};
	MemAddressToErase.Val = (((DWORD)ErasePageTracker) << 10);

	TBLPAG = MemAddressToErase.byte.UB;
	__builtin_tblwtl(MemAddressToErase.word.LW, 0xFFFF);

	NVMCON = 0x4042;				//Erase page on next WR
	asm("DISI #16");					//Disable interrupts for next few instructions for unlock sequence
	__builtin_write_NVM();
	Nop();
	Nop();
    while(NVMCONbits.WR == 1){}

//	EECON1bits.WREN = 0;  //Good practice now to clear the WREN bit, as further protection against any future accidental activation of self write/erase operations.
}	

#ifdef ERASE_EEPROM
void EepromErase(void)
{
	I2C_eeprom_erase_page(eepromEraseTracker, 0xff);
}
#endif

void WriteFlashSubBlock(void)		//Use word writes to write code chunks less than a full 64 byte block size.
{
	unsigned int i = 0;
	
	// Detect if bootloader is writing to external memory
	if (ProgrammedPointer >= ExtEepromMemStart) 
	{
		// TODO: Write to eeprom in bigger chunks
		while(BufferedDataIndex > 0)		//While data is still in the buffer.
		{
			unsigned short j;
			unsigned char buffer[WRITE_PAGE_SIZE];		
	
			unsigned long 	progAddr = ProgrammedPointer - BufferedDataIndex; // Address of word to program
			unsigned short* progData = &ProgrammingBuffer[i]; 				// Data to program lower words
		
			// Determine continuous max write size?
			#define WRITE_PAGE_SIZE_MASK (WRITE_PAGE_SIZE - 1)
			unsigned short maxContinuous = progAddr & WRITE_PAGE_SIZE_MASK;	// Find number of entrys until end of eeprom page
			if (maxContinuous == 0) maxContinuous = WRITE_PAGE_SIZE;		// Zero means we are at the page boundary
			
			// Determine write length in bytes
			unsigned short len = maxContinuous;
			if (len > BufferedDataIndex) len = BufferedDataIndex; 			// BufferedDataIndex = number of prog words to write (4 bytes) = num eeprom bytes to write
	
			// Load buffer with 'len' values
			for (j=0;j<len;)
			{
				buffer[j++] = (*progData)&0xff;
				buffer[j++] = (*progData)>>8;
				progData += 2; // Skip empty words
			}
			
			// Write max bytes
			I2C_eeprom_write((progAddr-ExtEepromMemStart), buffer, len);
			I2C_eeprom_wait();
	
			i = i + len; 									//Note: +2 since upper 2 bytes are always empty
			BufferedDataIndex = BufferedDataIndex - len;	//Used up 2 (16-bit) words from the buffer.
		}
		return;
	}
	else
	{
		while(BufferedDataIndex > 0)		//While data is still in the buffer.
		{
			unsigned int offset;
			unsigned long progAddr = ProgrammedPointer - BufferedDataIndex; // Address of word to program
			unsigned int progDataL = ProgrammingBuffer[i]; 					// Data to program lower word
			unsigned char progDataH = ProgrammingBuffer[i+1]; 				// Data to program upper byte
		
			//Set up NVMCON for word programming
			NVMCON = 0x4003; 			// Initialize NVMCON
			//Set up pointer to the first memory location to be written
			TBLPAG = progAddr>>16; 		// Initialize PM Page Boundary SFR
			offset = progAddr & 0xFFFF; // Initialize lower word of address
			//Perform TBLWT instructions to write latches
			__builtin_tblwtl(offset, progDataL); // Write to address low word
			__builtin_tblwth(offset, progDataH); // Write to upper byte
			asm("DISI #16"); 			// Block interrupts with priority < 7
			__builtin_write_NVM(); 		// C30 function to perform unlock
			Nop();
			Nop();
			i = i + 2;
			BufferedDataIndex = BufferedDataIndex - 2;		//Used up 2 (16-bit) words from the buffer.
		}
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

gAddrRead = address;
 
	if (address >= ExtEepromMemStart) 
	{
		DWORD_VAL destination = {0};
		// Is it an odd location? These always read 0x0000 to emulate prog memory
		if (address & 0b1)
		{
			// This is an error
			Nop();
		}
		else
		{
			I2C_eeprom_read((address-ExtEepromMemStart), destination.v, 2);
		}
		return destination.Val;
	}
	else
	{
	    wTBLPAGSave = TBLPAG;
	    TBLPAG = ((DWORD_VAL*)&address)->w[1];
	
	    dwvResult.w[1] = __builtin_tblrdh((WORD)address);
	    dwvResult.w[0] = __builtin_tblrdl((WORD)address);
	    TBLPAG = wTBLPAGSave;
      	return dwvResult.Val;
 	}	
}

#if 1
void __attribute__((address(0x1cc0)))sos_signal(void)
{
	// Some default code here to sleep the device if no USB connection.
	// This is going to be over written as soon as the device is bootloaded.

	unsigned short delay;
	InitIOBootloader();
	CLOCK_INTRC();

	// Default program: quick "SOS" message to indicate we have no main routine

	#define WAIT_DELAY_ON  100
	#define WAIT_DELAY_OFF 1000

	LED = 1; for (delay = WAIT_DELAY_ON ; delay; --delay); LED = 0; for (delay = WAIT_DELAY_OFF; delay; --delay);
	LED = 1; for (delay = WAIT_DELAY_ON ; delay; --delay); LED = 0; for (delay = WAIT_DELAY_OFF; delay; --delay);
	LED = 1; for (delay = WAIT_DELAY_ON ; delay; --delay); LED = 0; for (delay = WAIT_DELAY_OFF; delay; --delay);

	for (delay = WAIT_DELAY_ON; delay; --delay);

	LED = 1; for (delay = 8 * WAIT_DELAY_ON ; delay; --delay); LED = 0; for (delay = WAIT_DELAY_OFF; delay; --delay);
	LED = 1; for (delay = 8 * WAIT_DELAY_ON ; delay; --delay); LED = 0; for (delay = WAIT_DELAY_OFF; delay; --delay);
	LED = 1; for (delay = 8 * WAIT_DELAY_ON ; delay; --delay); LED = 0; for (delay = WAIT_DELAY_OFF; delay; --delay);

	for (delay = WAIT_DELAY_ON; delay; --delay);

	LED = 1; for (delay = WAIT_DELAY_ON ; delay; --delay); LED = 0; for (delay = WAIT_DELAY_OFF; delay; --delay);
	LED = 1; for (delay = WAIT_DELAY_ON ; delay; --delay); LED = 0; for (delay = WAIT_DELAY_OFF; delay; --delay);
	LED = 1; for (delay = WAIT_DELAY_ON ; delay; --delay); LED = 0; for (delay = WAIT_DELAY_OFF; delay; --delay);

	RCONbits.SWDTEN = 1;
	Sleep();
	RCONbits.SWDTEN = 0;

	__asm__("goto 0x00000000");
}
#endif
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
