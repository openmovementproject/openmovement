/* 
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

// KL 12-06-2011 
// .C file for hardware specific functions like sleep, hibernate etc

#include "Compiler.h"
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "Analogue.h"

//#include "SystemProfile.h"
//#include "WirelessProtocols/Console.h"
//#include "WirelessProtocols/LCDBlocking.h"

// Globals

// Config settings
/*24 bit config 1 0b<unimp>0000<unimp>0000<res>0<jtag off>0<code prot off>1<prog writes on>1<debug off>1<res>1<EMUD1>11<wdtoff>0<stdrd wdt>1<unimp>0<wdt pres 32>1<wdt posc see table>****>*/ 
/* WDT bits 1111 = 1:32,768; 1110 = 1:16,384; 1101 = 1:8,192; 1100 = 1:4,096; 1011 = 1:2,048; 1010 = 1:1,024; *1001 = 1:512*; 1000 = 1:256; 0111 = 1:128; 0110 = 1:64; 0101 = 1:32; 0100 = 1:16; 0011 = 1:8; 0010 = 1:4; 0001 = 1:2; 0000 = 1:1 */
_CONFIG1(0b000000000011111001001010) 
/*24 bit config 2 0b<unimp>0000<unimp>0000<IESO off>0<plldiv2>001<plldis off>0<init osc = frc>000<clock switch en>01<no clk op>0<iolock on-off en>0<usb vreg off>1<res>1<prim osc off>11>*/
_CONFIG2(0b000000000001000001001111)
// These settings use PGECD 2 for the debug interface



// Sleep mode - External peripherals all allowed to run unless specified
void SystemPwrSave(unsigned long NapSetting)
{
	unsigned int IEC0_save,IEC1_save,IEC2_save,IEC3_save,IEC4_save,IEC5_save,IPLshadow; // Interrupt context saving
	IPLshadow = SRbits.IPL;
	SRbits.IPL = 7;
  	
	// External peripherals off as needed
	LED_INIT_PINS();

	// Internal Peripherals off
	//if (!NapSetting & WAKE_ON_TIMER1) T1CONbits.TON = 0;	// Timer1 is the ms count 

	U1CONbits.USBEN = 0;
	U1PWRCbits.USBPWR = 0; // MCHP Bug, not cleared in usb detach +120uA

	// Reduce ICSP leakage
	TRISBbits.TRISB6 = 0; LATBbits.LATB6 = 0;
	TRISBbits.TRISB7 = 0; LATBbits.LATB7 = 0;

	// Disable all interrupts
	if (NapSetting & SAVE_INT_STATUS)
	{
		IEC0_save = IEC0;
		IEC1_save = IEC1;
		IEC2_save = IEC2;
		IEC3_save = IEC3;
		IEC4_save = IEC4;
		IEC5_save = IEC5;
	}
	IEC0 = IEC1 = IEC2 = IEC3 = IEC4 = IEC5 = 0; // Disable all ints

	// Enable wakeup interrupts + peripherals
	if (NapSetting==0){return;} // Stops it being bricked by accident
	if (NapSetting & WAKE_ON_USB)	// Wake on USB detect
	{
		USB_BUS_SENSE_INIT_INT(); // Shared with button inter on change notifications
	}	 	
	if (NapSetting & WAKE_ON_RTC)	// Wake on RTC alarm
	{
		IEC3bits.RTCIE = 1;
	}
	if (NapSetting & WAKE_ON_BUTTON)	// Timer1 is the ms count
	{
		BUTTON_INIT_INT();
	}		
	if (NapSetting & WAKE_ON_TIMER1)	// Timer1 is the ms count
	{
		IEC0bits.T1IE = 1;
	}
	if (NapSetting & WAKE_ON_ADC)	// Timer1 is the ms count
	{
		IEC0bits.AD1IE = 1;		// Enable interrupts
	}
	if (NapSetting & WAKE_ON_WDT) 	// Set period in config word 1 - currently 4096 * 1ms
		{ClrWdt();RCONbits.SWDTEN = 1;}	// Wake on WDT

	// Go to sleep... zzzzzzzzzzzzzz
	__builtin_write_OSCCONL(OSCCON & 0xFB); 	// Primary oscillator off during sleep (sleep with pll will incure wakeup penalty)
	REFOCON = 0;			// Reference oscillator off
	if(NapSetting & LOWER_PWR_SLOWER_WAKE)	RCONbits.PMSLP = 0; // Adds 64ms wakeup time
	else 									RCONbits.PMSLP = 1;
	Sleep();
	__builtin_nop();

	// ON RETURN FROM SLEEP 
	if ((NapSetting & WAKE_ON_WDT) &&(!(NapSetting & LEAVE_WDT_RUNNING)))
		{ClrWdt();RCONbits.SWDTEN = 0;}	// Turn off WDT

	// Restore the pins driven during sleep
	TRISBbits.TRISB6 = 1; 
	TRISBbits.TRISB7 = 1; 

	// Restore Interrupts
	if (!(NapSetting & ALLOW_VECTOR_ON_WAKE))
	{
		IFS0 = IFS1 = IFS2 = IFS3 = IFS4 = IFS5 = 0; // Clear all flags
	}
	if (NapSetting & SAVE_INT_STATUS)
	{
		IEC0 = IEC0_save;
		IEC1 = IEC1_save; 
		IEC2 = IEC2_save; 
		IEC3 = IEC3_save; 
		IEC4 = IEC4_save; 
		IEC5 = IEC5_save;
	}
	SRbits.IPL = IPLshadow; // Effectively re-enable ints - will vector!
    return ;                       
}

void SystemPowerOff(unsigned long leave_on) // See HW profile for settings, same as syspwrsave()
{
	AdcOff(); // Should never be left on
	if (!(leave_on & LEAVE_ON_ANALOGUE))	{nAUDIO_POWER = 1;nPIR_POWER = 1;}
	if (!(leave_on & LEAVE_ON_RF))			{MRF_POWER_OFF();}
	if (!(leave_on & LEAVE_ON_TEMP))		{TEMP_DISABLE();}
	if (!(leave_on & LEAVE_ON_LIGHT))		{LDR_DISABLE();}
	if (!(leave_on & LEAVE_ON_LED))			{LED_SET(LED_OFF);}
}

// Indicates which button has been pressed (0=none, 1=short, 2=long, -1=release-from-short, -2=release-from-long)
#define DEBOUNCE_TIME 0x00003FFF
#define LONG_PRESS_TIME 0x00400000
//BYTE PUSH_BUTTON_pressed = 0;
//MIWI_TICK PUSH_BUTTON_press_time;
//BYTE ButtonPressed(void)
//{
//    MIWI_TICK tickDifference;
//        
//    if (PUSH_BUTTON_1 == 0)
//    {
//        //if the button was previously not pressed
//        if (PUSH_BUTTON_pressed == 0)
//        {
//            PUSH_BUTTON_pressed = 1;
//            PUSH_BUTTON_press_time = MiWi_TickGet();
//            return 1;
//        }
//        else if (PUSH_BUTTON_pressed == 1)
//        {
//            // Get the current time
//            MIWI_TICK t = MiWi_TickGet();
//            
//            // If the button has been pressed long enough
//            tickDifference.Val = MiWi_TickGetDiff(t,PUSH_BUTTON_press_time);
//            
//            // Mark it as long pressed
//            if(tickDifference.Val > LONG_PRESS_TIME)
//            {
//                PUSH_BUTTON_pressed = 2;
//                return 2;
//            }
//        }
//    }
//    else if (PUSH_BUTTON_pressed);
//    {
//        // Get the current time
//        MIWI_TICK t = MiWi_TickGet();
//        
//        // If the button has been released long enough
//        tickDifference.Val = MiWi_TickGetDiff(t,PUSH_BUTTON_press_time);
//        
//        // Then we can mark it as not pressed
//        if (tickDifference.Val > DEBOUNCE_TIME)
//        {
//            BYTE ret = -PUSH_BUTTON_pressed;    // negative for release
//            PUSH_BUTTON_pressed = 0;
//            return ret;
//        }
//    }
//    
//    return 0;
//}
//


// Read from program memory (faster if even address and length)
char ReadProgram(unsigned long address, void *buffer, unsigned short length)
{
    unsigned int oldTBLPAG;
    unsigned short i;

    // Save the TBLPAG register
    oldTBLPAG = TBLPAG;

    // If either address or length are odd
    if ((address & 1)  || (length & 1))
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
            TBLPAG = (unsigned char)(address >> 16);                        // Top 8 bits of address
            *(unsigned short *)buffer = __builtin_tblrdl((WORD)address);    // Read the bottom two bytes of the instruction
            buffer += 2;
            address += 2;
        }
    }

    // Restore the TBLPAG register
    TBLPAG = oldTBLPAG;

    return 1;
}


// Write to program memory (must be a page-aligned address; if length is odd, one extra byte of junk will be written)
char WriteProgramPage(unsigned long pageAddress, void *buffer, unsigned short length)
{
    unsigned int oldTBLPAG;
    unsigned short i, j;

    // Must start at an erase page boundary
    if (pageAddress & (unsigned long)0x03ff) { return 1; }
    pageAddress = pageAddress & (unsigned long)0xfffffc00;  // Zero non-page elements of the address

    // Save the TBLPAG register
    oldTBLPAG = TBLPAG;

    // Erase the page
    NVMCON = 0x4042;                                // Page erase on next write
    TBLPAG = (unsigned char)(pageAddress >> 16);        // Top 8 bits of address
    __builtin_tblwtl((unsigned short)pageAddress, 0xffff); // A dummy write to load address of erase page
    asm("DISI #16");                				// Disable interrupts for unlock sequence
    __builtin_write_NVM();

    // Reprogram the erased page (writes only to the PSV-addressable bytes, using 2 bytes of each 3-byte word)
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

        // Program block size boundary -- program the last 128 (of 192) bytes
        if (j >= WRITE_BLOCK_SIZE)
        {
            j -= WRITE_BLOCK_SIZE;
            asm("DISI #16");                				// Disable interrupts for unlock sequence
            __builtin_write_NVM();
        }
    }

    // If partial row written, pad with zeros then write
    if (j > 0)
    {
        while (j < WRITE_BLOCK_SIZE)
        {
            TBLPAG = (unsigned char)(pageAddress >> 16);
            __builtin_tblwtl((unsigned short)pageAddress, 0xffff);
            pageAddress++;
            __builtin_tblwth((unsigned short)pageAddress, 0xff);
            pageAddress++;
            j += 2;
        }
        asm("DISI #16");                				// Disable interrupts for unlock sequence
        __builtin_write_NVM();
    }

    // Restore the TBLPAG register
    TBLPAG = oldTBLPAG;
    
    return 0;
}

unsigned int GetClockFreqUsingRTC (void)
{
	/*This function turns on timer1 to run off the secondary crystal oscillator*/
	/*It uses this time base to gate the system clock on timer2 for 33 counts*/
	/*At 4Mhz this gives 4000000*33/32768 = 4028*/
	TMR1=0;				// Clear timer
	T1CONbits.TSYNC= 1;	// Sync - for read/write alignment issue
	T1CONbits.TCS = 1;	// Secondary osc timer input
	T1CONbits.TCKPS = 0;// No prescale
	T1CONbits.TGATE = 0;// Ignored bit (Not Gated)
	T1CONbits.TSIDL = 1;// Timer on in idle mode
	PR1 = 32;			// ~1ms timebase
	IFS0bits.T1IF=0;	// Clear flag
	T1CONbits.TON = 1;	// Timer on

	T2CON = 0; 			// Timer OFF, timer on intosc 4MHz (or 16 on PLL)
	T2CONbits.TON = 0;	// Timer off
	TMR2 = 0; 			// clear timer

	while(!IFS0bits.T1IF); // Wait for first roll over - now we are synchronised
	T2CONbits.TON = 1;	// Timer2 on counting up
	IFS0bits.T1IF=0;	// Clear the flag
	while(!IFS0bits.T1IF); // Wait 1ms again - this time counting
	T2CONbits.TON = 0;	// Timer2 off
	T1CONbits.TON = 0;	// Timer1 off
	return TMR2;
}

void CheckLowPowerCondition(void)
{
	// Check for disconnected & low power state
	AdcInit(ADC_CLOCK_SYSTEM);
	AdcSampleWait();
	if (!USB_BUS_SENSE && adcResult.batt > 10 && adcResult.batt < BATT_CHARGE_MIN_SAFE)	// BATT_CHARGE_MIN_SAFE  BATT_CHARGE_ZERO
	{
		int i = 0;
		while (!USB_BUS_SENSE)
		{
			if (i++ % 5 == 0) { LED_SET(LED_RED); DelayMs(1); }	
			// Sleep
			SystemPwrSave(WAKE_ON_USB| WAKE_ON_BUTTON| WAKE_ON_RTC| LOWER_PWR_SLOWER_WAKE| SAVE_INT_STATUS| ALLOW_VECTOR_ON_WAKE);
		}
		Reset();
	}    
}
