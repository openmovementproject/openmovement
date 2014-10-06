/* 
 * Copyright (c) 2011-2013, Newcastle University, UK.
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
// Karim Ladha, Dan Jackson, 2011
// 23-12-2011 : CWA3 base power down current checked - 55uA with all off (RTC running, SOSC on)

// Includes
#include <Compiler.h>
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Peripherals/Rtc.h"
#include "Analog.h"
#include "GraphicsConfig.h"
#include "Graphics/DisplayBasicFont.h"

unsigned char gUsbDetect = 0;
short gButton = 0;

// Configuration word
// 24 bit config 1 0b<unimp>0000<unimp>0000<res>0<jtag off>0<code prot off>1<prog writes on>1<debug off>1<res>1<EMUD1>11<wdtoff>0<stdrd wdt>1<unimp>0<wdt pres 32>1<wdt posc see table>****>*/
//     WDT bits -- 1111 = 1:32,768; 1110 = 1:16,384; 1101 = 1:8,192; 1100 = 1:4,096; 1011 = 1:2,048; 1010 = 1:1,024; 1001 = 1:512; 1000 = 1:256; 0111 = 1:128; 0110 = 1:64; 0101 = 1:32; 0100 = 1:16; 0011 = 1:8; 0010 = 1:4; 0001 = 1:2; 0000 = 1:1;
_CONFIG1(0b000000000011111101001010) //WDT 1024
// 24 bit config 2 0b<unimp>0000<unimp>0000<IESO off>0<plldiv2>001<plldis off>0<init osc = frc>000<clock switch en>00<no clk op>0<iolock on-off en>0<usb vreg off>1<res>1<prim osc off>11>*/
_CONFIG2(0b000000000001000000001111)
//    _CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & FWDTEN_OFF & ICS_PGx2)
//    _CONFIG2( 0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_ON & POSCMOD_HS & FNOSC_FRCPLL & PLLDIV_DIV2 & IOL1WAY_ON)


// Sleep mode - External peripherals all allowed to run unless specified
void SystemPwrSave(unsigned long NapSetting)
{
 
}


// Precharge -- startup code to cope with battery pre-charge state 
void WaitForPrecharge(void)
{
    unsigned int timer = 100;
	
	// Otherwise they could be drawing lots of power - n.b. no way to halt internal nand processes
	SystemPwrSave(JUST_POWER_DOWN_PERIPHERALS|ACCEL_POWER_DOWN|DISP_POWER_DOWN);

	// 32 kHz rc osc
	CLOCK_INTRC();
		
	AdcInit();
	AdcSampleNow(); // Get one sample
	AdcOff();

	// if battery is flat and no charger/ USB present
	if ((adcResult.batt < BATT_CHARGE_MIN_SAFE)&&(!USB_BUS_SENSE))
	{
        // Stall here until charger/USB is connected
    	while (!USB_BUS_SENSE)
        {
            SystemPwrSave(WAKE_ON_WDT|WAKE_ON_RTC|LOWER_PWR_SLOWER_WAKE|WAKE_ON_USB|ADC_POWER_DOWN|ACCEL_POWER_DOWN);
            LED_SET(LED_RED);
            Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
            LED_SET(LED_OFF);
        }
        // Reset once connected
		Reset();
	}

	// Sample the battery
    for (;;)
    {
		ClrWdt();   // Reset WDT while we're in this loop
		timer++;

		// if battery is flat - red 5% flash @ ~2Hz
		if(timer > 100)
		{
			LED_SET(LED_RED);
		}
		if (timer > 105)
		{
			LED_SET(LED_OFF);
			// Check battery voltage
			AdcInit();
			AdcSampleNow(); 
			AdcOff();
			if (adcResult.batt > BATT_CHARGE_MIN_SAFE)
			{
				// Not in precharge
				LED_SET(LED_OFF);
				CLOCK_INTOSC();
				return;	
			}
			timer = 0;
		}
    }

    return;
}


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


// Write to program memory (must be a page-aligned address; if length is odd, one extra byte of junk will be written)
char WriteProgramPage(unsigned long pageAddress, void *buffer, unsigned short length)
{
    unsigned int oldTBLPAG;
    unsigned short i, j;

    // Must start at an erase page boundary
    if (pageAddress & (unsigned long)0x03ff) { return 1; }
	if ((pageAddress < MIN_ALLOWED_WRITE_ADD) || (pageAddress >= MAX_ALLOWED_WRITE_ADD)){ return 1; }
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

		// KL, fixes bug is odd pointer is passed to this function
		if ((short)(void*)(buffer) & 0b1) // Odd pointer
		{
			unsigned short word;
			// Reformulate the word from the odd and even addresses
			word = (unsigned short)(*(unsigned char*)(buffer+1+i))<<8;
			word += (*(unsigned char*)(buffer+i));
	        __builtin_tblwtl((unsigned short)pageAddress, word); // Bottom two bytes
	        pageAddress++;		
		}
		else
		{
	        __builtin_tblwtl((unsigned short)pageAddress, *((unsigned short *)(buffer + i))); // Bottom two bytes
	        pageAddress++;
		}

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
            TBLPAG = (BYTE)(pageAddress >> 16);
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

// Performs a self test, returns a bitmap of failures (0x0000 = success)
unsigned short SelfTest(void)
{
    unsigned short result = 0x0000;

    // Read accelerometer device ID (should be ACCEL_DEVICE_ID = 0xE5)
    //if (AccelReadDeviceId() != ACCEL_DEVICE_ID) { result |= 0x0001; }

    // Read NAND parameters and compare with compiled-in constants
    //if (FtlVerifyNandParameters()) { result |= 0x0002; }

    // FTL

    // FSIO

    // LDR

    // Temperature

    // Battery

    return result;
}

void Pass(void)
{
	LED_SET(LED_GREEN);	
	DelayMs(50);
	LED_SET(LED_OFF);	
	DelayMs(200);
}

void Fail(void)
{
	LED_SET(LED_RED);	
	DelayMs(50);
	LED_SET(LED_OFF);	
	DelayMs(200);
}

#if 0
// Test function for debugging
void test_system_debug (void)
{
  // Initialization
	InitIO();			// I/O pins
	InitI2C();
	CLOCK_SOSCEN(); 	// For the RTC
	//WaitForPrecharge();	// ~0.5mA current
	
	// Peripherals - RTC and ADC always used
	CLOCK_INTOSC();     // 8 MHz
	LED_SET(LED_CYAN);
    RtcStartup();
	AdcInit();
    AdcSampleWait();                    // Ensure we have a valid battery level

	// Check LED
	LED_SET(LED_RED);
	DelayMs(200);
	LED_SET(LED_GREEN);
	DelayMs(200);
	LED_SET(LED_BLUE);
	DelayMs(200);
	LED_SET(LED_OFF);
	DelayMs(200);

	// Check clocks
	{
		static unsigned int period;
		period = GetClockFreqUsingRTC();
		if ((period > 4000)&&(period < 4056)) Pass(); 
		else Fail();
	}

	// Analogue periferals
	AdcInit();
	AdcSampleWait();
	// Batt
	if ((adcResult.batt > 500)&&(adcResult.batt < 751)) Pass(); // <3v = 510, 3v = 512 , 4.4v = 751
	else Fail();

	// I2C Peripherals
    // Accelerometer
    if(AccelVerifyDeviceId())Pass(); // responded OK
	else Fail();

	// Display
	DisplayInit();
	DisplayClear();
	DisplayPrintLine(" CWA3",0,4);
	DisplayPrintLine("FIRST DEMO",5,2);
	DisplayPrintLine("Hardware version 3.1",7,1);

// Implement this in your code after this function to check the USB and BT module
    // Read settings
    LED_SET(LED_GREEN);
    restart = 0;
    inactive = 0;
    SettingsInitialize();           // Initialize settings from ROM
    FtlStartup();                   // FTL & NAND startup
    FSInit();                       // Initialize the filesystem for reading
    SettingsReadFile(SETTINGS_FILE);    // Read settings from script
    // TODO: Make this "single binary file" mode optional on the settings just read
    LoggerReadMetadata(DEFAULT_FILE);   // Read settings from the logging binary file

    // Run as attached or logging
	if (USB_BUS_SENSE)
	{
		RunAttached();      // Run attached, returns when detatched - copy this init script to the msd
    }


}
#endif

