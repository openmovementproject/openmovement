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
 *
 * CWA3.1 Karim Ladha 08/12/12
 */
 
// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "stdint.h"
#include "HardwareProfile.h"
#include "Peripherals/Nand.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Mag.h"
#include "Peripherals/Rtc.h"
#include "USB/USB.h"
#include "USB/usb_function_msd.h"
#include "USB/usb_function_cdc.h"
#include "USB/USB_CDC_MSD.h"
#include "MDD File System/FSIO.h"
#include "Ftl/FsFtl.h"
#include "Peripherals/Analog.h"
#include "Utils/Fifo.h"
#include "Settings.h"
#include "Logger.h"
#include "Util.h"
#include "BTtasks.h"
#include "Graphics/GOL.h"
#include "GraphicsConfig.h"
#include "Graphics/DisplayDriver.h"
#include "Graphics/Primitive.h"
#include "DisplayBasicFont.h"
#include "myGraphics.h"
#include "myi2c.h"
#include "PIRULEN32PIX.h"
#include "Peripherals/bmp085.h"

// Prototypes
extern void RunAttached(void);
//extern void RunLogging(void);
extern void PrintTopDisplayLine(void);
extern void RunClockTasks(unsigned char forceOn);

// Globals
extern const FONT_FLASH GOLFontDefault; // default GOL font

// Interrupts
// Interrupts
void __attribute__((interrupt,auto_psv)) _DefaultInterrupt(void)
{
 	static unsigned int INTCON1val;
	LED_SET(LED_MAGENTA);
	INTCON1val = INTCON1;
	Nop();
	Nop();
	Nop();
	Nop();
	INTCON1 = 0;
//    Reset();
}

// RTC
void __attribute__((interrupt,auto_psv)) _RTCCInterrupt(void)
{
	RtcTasks();
    RtcSwwdtIncrement();    // Increment software RTC, reset if overflow
	LoggerRTCTasks(); 
}
// TMR1
void __attribute__((interrupt, shadow, auto_psv)) _T1Interrupt(void)
{
    RtcTimerTasks();
	BT_tick_handler();
	LoggerTimerTasks();
}


// Main routine
int main(void)
{
    // Initialization
	InitIO();			// I/O pins
	InitI2C();			// Once only
	CLOCK_SOSCEN(); 	// For the RTC
//	WaitForPrecharge();	// ~0.5mA current
	
	// Peripherals - RTC and ADC always used
	CLOCK_INTOSC();     // 8 MHz
	LED_SET(LED_CYAN);
    RtcStartup();
	AdcInit();
    AdcSampleWait();                    // Ensure we have a valid battery level

	// Display
	DisplayInit();
	DisplayClear();
	Display_print_xy("CWA3",20,2,4);
	//SetFont((void *) &GOLFontDefault);  // Once
	SetFont((void *) &pirulen_28);		// Once, new font
	SetColor(WHITE);					// Once

    // Read settings
    LED_SET(LED_GREEN);
    SettingsInitialize();           // Initialize settings from ROM
    FtlStartup();                   // FTL & NAND startup
    FSInit();                       // Initialize the filesystem for reading
    SettingsReadFile(SETTINGS_FILE);    // Read settings from script
    LoggerReadMetadata(DEFAULT_FILE);   // Read settings from the logging binary file
	DisplayClear();

    // Check if we have an accelerometer
    AccelVerifyDeviceId();
    // Check if we have a gyro
    GyroVerifyDeviceId();
	// Check if we have a magnetometer
	MagVerifyDeviceId();
	// Check if we have an altimeter + initialise it
	AltVerifyDeviceId();

	settings.btEnable = TRUE;

	// Run as attached or logging
	if (0)//USB_BUS_SENSE)
	{
		RunAttached();      // Run attached, returns when detatched
    }
	else if (settings.btEnable == TRUE)
	{
		BTInit();				// Sets up BT stack
		while(1)
		{
			BTtasks();			// Call to service the BT stack
		}
	}
    else
    {
		DisplayClear();
    	settings.loggingStartTime = 0; // force log always
		RunLogging();       // Run in logging mode, returns when attached
    }

	Reset();                // Reset
	return 0;
}

// Writes the battery state and time to the top of the display
void PrintTopDisplayLine(void)
{
	unsigned char i;
	unsigned int batt; 
	const char* temptr;
	char now[20];
	
	// Clear display lines
	Display_clear_line(0);
	Display_clear_line(1);

	memcpy(now,(RtcToString(RtcNow())),20);
	now[10] = '\0'; // null terminate
	now[16] = '\0'; // null terminate
	now[19] = '\0'; // null terminate

	// Time
	Display_print_xy(&now[2], 80, 0, 1); 	// Date
	Display_print_xy(&now[11], 0, 0, 2);	// Time
	Display_print_xy(&now[17], 63, 0, 1);	// Sec

	// Battery %
	// Update the battery %
	batt = AdcBattToPercent(adcResult.batt);
	temptr = my_itoa(batt);
	Display_print_xy((char*)temptr, 86, 1, 1);		// Batt
	Display_putc_basic('%',1);					// %

	// Draw battery symbol / level
	Display_gotoxy(112,1);
	DisplayWriteData(0b00011000);
	DisplayWriteData(0b11111111);
	if (!USB_BUS_SENSE) // Not charging
	{
		batt /= 8; // convert % to an int 0->12
		for (i=12;i>0;i--)		// to position 127/128
		{
			if (i>batt) 	{DisplayWriteData(0b10000001);}
			else 			{DisplayWriteData(0b11111111);}
		}
	}
	else // Charging - change symbol 
	{
		DisplayWriteData(0b10000001);
		DisplayWriteData(0b10100101);
		DisplayWriteData(0b10100101);
		DisplayWriteData(0b10111101);
		DisplayWriteData(0b10111101);
		DisplayWriteData(0b10111101);
		DisplayWriteData(0b10111101);
		DisplayWriteData(0b10111101);
		DisplayWriteData(0b10011001);
		DisplayWriteData(0b10011001);
		DisplayWriteData(0b10011001);
		DisplayWriteData(0b10000001);
	
	}
	DisplayWriteData(0b11111111);
	
	return;
}

