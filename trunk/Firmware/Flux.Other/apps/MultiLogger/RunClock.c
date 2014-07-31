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

// Karim Ladha, 2012
// Watch function

// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Rtc.h"
#include "Analog.h"
#include "Utils/Util.h"
#include "GraphicsConfig.h"
#include "Graphics/DisplayBasicFont.h"
#include "Graphics/Primitive.h"
#include "Peripherals/myi2c.h"

#include "Apps/MultiLogger/RunClock.h"
#include "Bluetooth/Bluetooth.h"

// This figure must be updated using the accel ISR
extern unsigned char accelOrientation;

// Display override for a period of time for remote content (will revert to clock on timeout)
int overrideTimeout = 0;


// Added to allow external app to handle this test
char TestForWatchGesture(void)
{
	static unsigned char oldOrientation = 0xff;
	unsigned char retval = 0;
	if	(accelOrientation!=oldOrientation)
	{
		retval = -1;
#ifndef ALTERNATE_CLOCK_GESTURE_ORIENTATION
		if (
		(((oldOrientation&0b01000110)==0)&& 	// Normal gesture
		((accelOrientation&0b01000001)==0x41))
		||
		(((oldOrientation&0b01000110)==0)&&		// Extended gesture
		((accelOrientation&0b01000110)==0x02))
		||
		(((oldOrientation&0b01000001)==0x41)&&	// Side gesture
		((accelOrientation&0b01000110)==0x02)) )
#else
		if ( // Gestures are reversed in direction
		(((oldOrientation&0b01000001)==0x41)&&
		((accelOrientation&0b01000110)==0))	// Normal gesture
		||
		(((oldOrientation&0b01000110)==0x02)&&
		((accelOrientation&0b01000110)==0))// Extended gesture
		||
		(((oldOrientation&0b01000110)==0x02)&&
		((accelOrientation&0b01000001)==0x41)))// Side gesture
#endif
		{
			retval = 1;
		}
		oldOrientation=accelOrientation;
	}
	return retval;
}

// Requires you to pre-initialise display
// Requires adc results to be updated if used
// RTC must be running for valid time
// Call every second for usb operation to display a clock
// Call with -1 to stop the clock being shown until called with 0 again
// Returns true if Display is on
unsigned char __attribute__((weak)) RunClockTasks(signed char forceOn)
{
	// The accelOrientation variable is updated in the interrupt
	static unsigned int timer = 0;
	static unsigned char showTime = FALSE;
	char now[6];

	// For USB, the time is on all this time
	if(USB_BUS_SENSE) 
	{
		if (showTime == FALSE) // once on first call
		{
			// Display init
			DisplayInit();
			DisplayClear();
		}
		timer = 1000;
		showTime = TRUE;
	}
	// For force off...
	else if (forceOn == -1)
	{
		DisplayOff();
		timer = 0;
		showTime = FALSE;
	}
	// For forced on call
	else if (forceOn)
	{
		DisplayRestore();
		timer = forceOn;
		showTime = TRUE;
	}
	else if (timer > 0)	// Timer to turn off the display
	{
		if ( --timer==0)
		{
			DisplayOff();
			showTime = FALSE;
		}
	}

	// Count down override timeout
	if (overrideTimeout)
	{
		overrideTimeout--;
	}

	if (showTime == TRUE && overrideTimeout == 0)
	{
		memcpy(now,(RtcToString(RtcNow())+11),5);
		now[5] = '\0'; // null terminate
		//DisplayPrintLine(now,2,4); 
		DisplayClearRam(GDbuffer, DISPLAY_WIDTH);
		OutTextXY(16, 6, now);
		DisplayRefresh(GDbuffer,DISPLAY_WIDTH);

		PrintBluetoothStatus();	// Top left
		PrintBatteryStatus(); 	// Top right
	}
	return showTime;
}

void PrintBatteryStatus(void)
{
	// Battery %, top right
	// Update the battery %
	const char* temptr;
	unsigned short i;
	unsigned short batt;
	
#ifdef DISPLAY_RAW_BATTERY
	batt = AdcBattToMillivolt(adcResult.batt);	
	temptr = my_itoa(batt);
	Display_print_xy((char*)temptr, 86, 0, 1);		// Batt
	Display_putc_basic('m',1);						// %
	Display_putc_basic('mV',1);						// %
	i = 4 - strlen(temptr); 
	for(;i>0;i--){Display_putc_basic(' ',1);}		// Spaces
#else
	batt = AdcBattToPercent(adcResult.batt);
	temptr = my_itoa(batt);
	Display_print_xy((char*)temptr, 86, 0, 1);		// Batt
	Display_putc_basic('%',1);						// %
	i = 3 - strlen(temptr); 
	for(;i>0;i--){Display_putc_basic(' ',1);}		// Spaces

	// Draw battery symbol / level
	Display_gotoxy(112,0);
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
#endif
}

void PrintBluetoothStatus(void)
{
	static unsigned char toggle = 0;
	unsigned short btstatus = BluetoothGetState();
	switch (btstatus){
		case(BT_OFF): 			
		case(BT_STARTING):	
		case(BT_FAIL):			
								Display_gotoxy(0,0);	
								Display_putc_basic(' ',1);
								break;
		case(BT_INITIALISED):
								Display_gotoxy(0,0);	
								if (toggle)	{Display_putc_basic(' ',1);toggle=0;}
								else 		{Display_putc_basic(0x08,1);toggle=1;}
								break;
		case(BT_CONNECTION_REQUEST):		
								Display_print_xy("R",0,0,1);
								break;
		case(BT_CONNECTED):		
								Display_gotoxy(0,0);	
								Display_putc_basic(0x08,1);
								break;
	};
}

// Override the display with remote content (timeout specified, will revert to clock once elapsed)
void DisplayOverride(int timeout)
{
	overrideTimeout = timeout;
	if (timeout == 0)
	{
    	DisplayClear();
	}
}


// This is an example of the clock as a standalone app
#if 0
void RunClock(void)
{
	// Peripherals - RTC and ADC always used
	CLOCK_INTOSC();     // 8 MHz
	LED_SET(LED_CYAN);
    RtcStartup();
	UpdateAdcWait();                   

	// Display
	DisplayInit();
	DelayMs(10);
	DisplayClear();

	AccelStartup(ACCEL_RANGE_4G|ACCEL_RATE_100);
	AccelEnableInterrupts(0b01011000,0b01000000);

	while(1)
	{
		char now[6];
		unsigned char oldOrientation;

		// Now sleep 
		SystemPwrSave(	WAKE_ON_USB	| WAKE_ON_ACCEL2 | WAKE_ON_WDT | WAKE_ON_RTC |				
						LOWER_PWR_SLOWER_WAKE | ADC_POWER_DOWN | DISP_POWER_DOWN |	
						GYRO_POWER_DOWN	| BT_POWER_DOWN	| ALLOW_VECTOR_ON_WAKE);

		//if(USB_BUS_SENSE)return;
		
		if	((accelOrientation!=oldOrientation)&&
			((oldOrientation&0b01000110)==0)&&
			((accelOrientation&0b01000001)==0x41))
		{
			DisplayRestore();
			memcpy(now,(RtcToString(RtcNow())+11),5);
			now[5] = '\0'; // null terminate
			//DisplayPrintLine(now,2,4); 
			OutTextXY(16, 32, now);
			DisplayRefresh(GDbuffer,DISPLAY_WIDTH);
			DelayMs(3000);
			DisplayOff();
		}
		oldOrientation = accelOrientation;
		
		LED_R = !LED_R;
			

	}

	return;
}
#endif

