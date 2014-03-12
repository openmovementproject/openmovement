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

// Hardware profile for WAX9
// Karim Ladha, Dan Jackson, 2011-2014

// 23-12-2011 : CWA3 base power down current checked - 55uA with all off (RTC running, SOSC on)

// Includes
#include <Compiler.h>
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Mag.h"
#include "Analog.h"
#include "Peripherals/myI2C.h"
#include "Bluetooth/Bluetooth.h"
#include "Peripherals/Eeprom.h"

// Config settings
/*24 bit config 1 0b<unimp>0000<unimp>0000<res>0<jtag off>0<code prot off>1<prog writes on>1<debug off>1<res>1<EMUD1>11<wdtoff>0<stdrd wdt>1<unimp>0<wdt pres 32>1<wdt posc see table>****>*/ 
/* WDT bits 1111 = 1:32,768; 1110 = 1:16,384; 1101 = 1:8,192; 1100 = 1:4,096; 1011 = 1:2,048; 1010 = 1:1,024; *1001 = 1:512*; 1000 = 1:256; 0111 = 1:128; 0110 = 1:64; 0101 = 1:32; 0100 = 1:16; 0011 = 1:8; 0010 = 1:4; 0001 = 1:2; 0000 = 1:1 */
_CONFIG1(0b000000000011111101001100)	/*4096ms*/ 
/*24 bit config 2 0b<unimp>0000<unimp>0000<IESO off>0<plldiv2>001<plldis off>0<init osc = frc>000<clock switch en>01<no clk op>0<iolock on-off en>0<usb vreg off>1<res>1<prim osc off>11>*/
_CONFIG2(0b000000000001000001001111)
/*Disableing SOSC pins to enable external clock*/
_CONFIG3(0b1111010011111111)
/*Deepsleep off, RTCC uses LPRC (inaccurate)*/
_CONFIG4(0b1111111100001111)

// Sleep mode - External peripherals all allowed to run unless specified
void SystemPwrSave(unsigned long NapSetting)
{
	//KL WARNING - modified for simpler behaviour, manage your own interrupt wake sources
	unsigned int IPLshadow; // Interrupt context saving

	IPLshadow = SRbits.IPL;
	SRbits.IPL = 0b111;	// Stop all interrupts from vectoring

	/*Note: Add/remove power down settings as needed*/
	U1PWRCbits.USBPWR = 0; // MCHP Bug, not cleared in usb detach +120uA

	// Wdt
	if (NapSetting & WAKE_ON_WDT)
		{ClrWdt();RCONbits.SWDTEN = 1;}	// Turn on WDT

	// Go to sleep... zzzzzzzzzzzzzz
	if(NapSetting & LOWER_PWR_SLOWER_WAKE)	RCONbits.PMSLP = 0;
	else 									RCONbits.PMSLP = 1; 
	Sleep();
	__builtin_nop();

	// ON RETURN FROM SLEEP 
	if (NapSetting & WAKE_ON_WDT)
		{ClrWdt();RCONbits.SWDTEN = 0;}	// Turn off WDT

	SRbits.IPL = IPLshadow; // Effectively re-enable ints - will vector if flags are set!
    return ;                       
}

// Read from program memory (faster if even address and length)
char ReadNvm(unsigned long address, void *buffer, unsigned short length)
{
    unsigned int oldTBLPAG;
    unsigned short i;

if (address >= EEPROM_START_ADDRESS)
{
	// Access to external eeprom instead
	address -= EEPROM_START_ADDRESS;
	return (I2C_eeprom_read( address, buffer, length));
}

    // Save the TBLPAG register
    oldTBLPAG = TBLPAG;

unsigned short IPL_save = SRbits.IPL;	
SRbits.IPL=7;

    // If either address or length are odd OR DESTINATION [KL]
    if ((address & 1)  || (length & 1) || ((unsigned short)(buffer) & 1))
    {
        // (Slower) read the data from the program memory one byte at a time
        for(i = 0; i < length; i++)
        {
            unsigned short temp;
            TBLPAG = (unsigned char)(address >> 16);            // Top 8 bits of address
            temp = __builtin_tblrdl(((WORD)address) & 0xfffe);  // Read the bottom two bytes of the instruction (don't actually need the mask for word reads)
            if (address & 1)
                *(unsigned char *)buffer = (unsigned char)(temp >> 8);  // Odd address / high byte
            else
                *(unsigned char *)buffer = (unsigned char)temp;         // Even address / low byte
            buffer++;
            address++;
        }
    }
    else
    {
        // (Faster) read the data from the program memory one word at a time
        for(i = 0; i < length; i += 2)
        {
            TBLPAG = (WORD)(address >> 16);                        			// Top 8 bits of address
            *(unsigned short *)buffer = __builtin_tblrdl((WORD)address);    // Read the bottom two bytes of the instruction
            buffer += 2;
            address += 2;
        }
    }

    // Restore the TBLPAG register
    TBLPAG = oldTBLPAG;

SRbits.IPL=IPL_save;

    return 1;
}


// Write to program memory page - no erase performed
char WriteNvm(unsigned long pageAddress, void *buffer, unsigned short length)
{
    unsigned int oldTBLPAG;
    unsigned short i, j;

if (pageAddress >= EEPROM_START_ADDRESS)
{
	// Access to external eeprom instead
	pageAddress -= EEPROM_START_ADDRESS;
	char retval = I2C_eeprom_write(pageAddress, buffer,length);
	I2C_eeprom_wait();
	return retval;
}

    // Must start at an erase page boundary
    if (pageAddress & (unsigned long)0x007f) { return 1; }
	if ((pageAddress < MIN_ALLOWED_WRITE_ADD) || (pageAddress >= MAX_ALLOWED_WRITE_ADD)){ return 1; }
	if (length > WRITE_BLOCK_SIZE) return 1; 	// Cant write past the page
    pageAddress &= (unsigned long)0xffffff80;  	// Zero non-page elements of the address

unsigned short IPL_save = SRbits.IPL;	
SRbits.IPL=7;

    // Save the TBLPAG register
    oldTBLPAG = TBLPAG;

    // Load the page buffer with the data
    NVMCON = 0x4001;                                // Internal flash program page
    j = 0;
    for (i = 0; i < length; i += 2)
    {
        TBLPAG = (unsigned char)(pageAddress >> 16);
        __builtin_tblwtl((unsigned short)pageAddress, *((unsigned short *)(buffer + i))); // Bottom two bytes
        pageAddress++;
        __builtin_tblwth((unsigned short)pageAddress, 0xff);                              // Set high byte (0x00 or 0xff = NOP instruction, 0x3f = invalid instruction)
        pageAddress++;

        j += 2;

	}

    // Pad the other bytes if a partial row with 0xFFFFFF
    if (j > 0)
    {
        while (j < WRITE_BLOCK_SIZE)
        {
            TBLPAG = (BYTE)(pageAddress >> 16);
            __builtin_tblwtl((unsigned short)pageAddress, 0xffff);
            pageAddress++;
            __builtin_tblwth((unsigned short)pageAddress, 0xff);
            pageAddress++;
            j += 2;
        }
        asm("DISI #16");                	// Disable interrupts for unlock sequence
        __builtin_write_NVM();				// Write the page
    }

    // Restore the TBLPAG register
    TBLPAG = oldTBLPAG;

SRbits.IPL=IPL_save;
    
    return 0;
}

// Write to program memory (must be a page-aligned address; if length is odd, one extra byte of junk will be written)
char EraseNvm(unsigned long address)
{
    unsigned int oldTBLPAG;

if (address >= EEPROM_START_ADDRESS)
{
	// Access to external eeprom instead - should erase 1024 bytes to 0xff to match prog space
	// In reality we never need to as long as we never check - eeprom does not need erasing
	return 1;
}

    // Must start at an erase page boundary
    if (address & (unsigned long)0x03ff) { return 0; }
	if ((address < MIN_ALLOWED_WRITE_ADD) || (address >= MAX_ALLOWED_WRITE_ADD)){ return 1; }
    address &= (unsigned long)0xfffffc00;  // Zero non-page elements of the address

    // Save the TBLPAG register
    oldTBLPAG = TBLPAG;

    // Erase the page
unsigned short IPL_save = SRbits.IPL;	
SRbits.IPL=7;
    NVMCON = 0x4042;                                		// Page erase on next write
    TBLPAG = (unsigned char)(address >> 16);        		// Top 8 bits of address (bottom bits not needed)
    __builtin_tblwtl((unsigned short)address, 0xffff); 	// A dummy write to load address of erase page
    asm("DISI #16");                						// Disable interrupts for unlock sequence
    __builtin_write_NVM();
SRbits.IPL=IPL_save;
	// Restore context
	TBLPAG =  oldTBLPAG;
	return 0;
}

