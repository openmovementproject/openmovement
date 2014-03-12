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

// Hardware-specific code
// Karim Ladha, 2013-2014

// Includes
#include <Compiler.h>
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Peripherals/Rtc.h"
#include "Analog.h"
#include "Peripherals/myI2C.h"
#include "Peripherals/Eeprom.h"

// Configuration word
_FICD  ( ICS_PGD1 & JTAGEN_OFF & RSTPRI_AF ) 
_FWDT  ( FWDTEN_OFF & PLLKEN_ON )  //  Watchdog Timer: Disabled 
_FOSC  ( FCKSM_CSECMD & POSCMD_HS & IOL1WAY_OFF & OSCIOFNC_OFF)  /*POSCMD_HS to use xtal*/
_FOSCSEL ( FNOSC_PRI & IESO_OFF) 
_FPOR (ALTI2C1_ON & FPWRT_PWR4 & BOREN_ON)
_FAS (AWRP_OFF & APL_OFF & APLK_OFF)
_FGS (GWRP_OFF & GSS_OFF  & GSSK_OFF )

// Dummy short for adaptation of some microchips drivers
short dummy;

// Sleep mode - External peripherals all allowed to run unless specified
void SystemPwrSave(unsigned long NapSetting)
{
	unsigned int IPLshadow; // Interrupt context saving
	IPLshadow = SRbits.IPL;
	SRbits.IPL = 0b111;	// Stop interrupts

	// External peripherals off as needed
	LED_SET(LED_OFF);

	// Enable wakeup interrupts + peripherals
	if (NapSetting & WAKE_ON_RTC)	IEC3bits.RTCIE = 1; 				// Wake on RTC alarm
	if (NapSetting & WAKE_ON_TIMER1)IEC0bits.T1IE = 1;					// Timer1 
	if (NapSetting & WAKE_ON_WDT)	{ClrWdt();RCONbits.SWDTEN = 1;}	 	// Set period in config word 1 

	// Go to sleep... zzzzzzzzzzzzzz
	__builtin_write_OSCCONL(OSCCON & 0xFB); 	// Primary oscillator off during sleep (sleep with pll will incure wakeup penalty)
	REFOCON = 0;								// Reference oscillator off
	if(NapSetting & LOWER_PWR_SLOWER_WAKE)	RCONbits.VREGS = 0;
	else 									RCONbits.VREGS = 1; 
	Sleep();
	__builtin_nop();

	// ON RETURN FROM SLEEP 
	if ((NapSetting & WAKE_ON_WDT) &&(!(NapSetting & LEAVE_WDT_RUNNING)))
		{ClrWdt();RCONbits.SWDTEN = 0;}	// Turn off WDT

	SRbits.IPL = IPLshadow; // Effectively re-enable ints - will vector if flags are set!
    return ;                       
}

// Read from program memory / eeprom (faster if even address and length)
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

    // If either address, destination or length are odd
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
    return 1;
}


// Write to program memory page / eeprom - no erase performed
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
    if (pageAddress & ((unsigned long)WRITE_BLOCK_SIZE-1)) { return 0; }
	if ((pageAddress < MIN_ALLOWED_WRITE_ADD) || (pageAddress >= MAX_ALLOWED_WRITE_ADD)){ return 0; }
	if (length > WRITE_BLOCK_SIZE) return 0; 	// Cant write past the page

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
        INTCON2bits.GIE = 0;			// Disable interrupts for next few instructions for unlock sequence
        __builtin_write_NVM();			// Write the page
		INTCON2bits.GIE = 1;			// Re-enable ints
    }

    // Restore the TBLPAG register
    TBLPAG = oldTBLPAG;

	// Check for error
	if (NVMCONbits.WRERR) return 0;

    return 1;
}

// Erase program memory - nust be block aligned
char EraseNvm(unsigned long address)
{
	DWORD_VAL eraseAddress;
    unsigned int oldTBLPAG;

	if (address >= EEPROM_START_ADDRESS)
	{
		// Access to external eeprom instead - should erase 1024 bytes to 0xff to match prog space
		// In reality we never need to as long as we never check - eeprom does not need erasing
		return 1;
	}

    // Must start at an erase page boundary
    if (address & ((unsigned long)ERASE_BLOCK_SIZE-1)) { return 0; }
	if ((address < MIN_ALLOWED_WRITE_ADD) || (address >= MAX_ALLOWED_WRITE_ADD)){ return 0; }

    // Erase the page
    // Save the TBLPAG register
    oldTBLPAG = TBLPAG;
	eraseAddress.Val = address;
	TBLPAG = eraseAddress.byte.UB;
	NVMADRU = eraseAddress.word.HW;
    NVMADR = eraseAddress.word.LW;
	__builtin_tblwtl(eraseAddress.word.LW, 0xFFFF);
	NVMCON = 0x4003;				//Erase page on next WR

	INTCON2bits.GIE = 0;			//Disable interrupts for next few instructions for unlock sequence
	__builtin_write_NVM();
    while(NVMCONbits.WR == 1){}
	INTCON2bits.GIE = 1;			// Re-enable the interrupts (if required).
   
	// Restore context
	TBLPAG =  oldTBLPAG;

	// Check for error
	if (NVMCONbits.WRERR) return 0;

	return 1;
}

unsigned int GetClockFreqUsingRTC (unsigned long* oscFreq)
{
	/*This function turns on timer1 to run off the secondary crystal oscillator*/
	/*It uses this time base to gate the system clock on timer2 for 33 counts*/
	/*At 4Mhz periferal clock (Fosc = 8 MHz) this gives 4000000*33/32768 = 4028*/
	unsigned int count;
	unsigned char GIE_save = INTCON2bits.GIE;
	unsigned char T1CON_save = T1CON;
	unsigned char PR1_save = PR1;
	unsigned char T2CON_save = T2CON;
	unsigned char PR2_save = PR2;

	INTCON2bits.GIE = 0;// Interrupts off

	T1CONbits.TON = 0;	// Timer off
	TMR1=0;				// Clear timer count
	T1CONbits.TSYNC= 1;	// Sync - for read/write alignment issue
	T1CONbits.TCS = 1;	// Secondary osc timer input
	T1CONbits.TCKPS = 0;// No prescale
	PR1 = 32;			// ~1ms timebase
	IFS0bits.T1IF=0;	// Clear roll over flag
	T1CONbits.TON = 1;	// Timer on, running

	T2CON = 0; 			// Timer off, no prescaling
	PR2 = 0xffff;		// Max period, note - 65.074MHz max measurable clock
	TMR2 = 0; 			// clear timer

	while(!IFS0bits.T1IF); 	// Wait for first roll over - now we are synchronised
	T2CONbits.TON = 1;		// Timer2 on, begins counting up on next peripheral clock edge
	IFS0bits.T1IF=0;		// Clear the flag
	while(!IFS0bits.T1IF); 	// Wait 1ms again - this time counting
	T2CONbits.TON = 0;		// Timer2 off, stop counting on next peipheral clock edge

	count = TMR2;

	T1CON = T1CON_save;	// Now put everything back as it was..
	PR1 = PR1_save;
	T2CON = T2CON_save;
	PR2 = PR2_save;
	INTCON2bits.GIE = GIE_save;

	// If user needs a frequency value then compute it here
	if (oscFreq != NULL)
	{
		unsigned long temp = count * 32768 * 2; // Note x2 because periferal clock is half osc frequency
		temp /= 33;
		*oscFreq = temp;
	}

	return count;		// Return raw count
}

void ClockTunedFRCPLL(void)
{
	static unsigned long frequency = 0;
	unsigned char tuning = 31; // Fmax

	// Set the clock to the one being tuned
	CLOCK_INTOSC();

	// This tunes the internal oscillator until it is 8Mhz or slightly under
	do
	{
		OSCTUNbits.TUN = tuning--;
		DelayMs(10);
		GetClockFreqUsingRTC(&frequency);
		Nop();
	}while ((frequency > 8000000)&&(tuning != 0));

	// Now switch to the PLL which should be tuned correctly
	CLOCK_INTOSC_PLL();
	return;
}

