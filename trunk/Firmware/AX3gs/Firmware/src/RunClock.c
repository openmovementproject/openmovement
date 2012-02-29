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
 */
 
// Source code for watch function
// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Rtc.h"
#include "Peripherals/Analog.h"
#include "Util.h"
#include "GraphicsConfig.h"
#include "DisplayBasicFont.h"
#include "Graphics/Primitive.h"
#include "myi2c.h"

// This figure must be updated using the accel ISR
extern unsigned char accelOrientation;
extern void PrintTopDisplayLine(void);

// Requires you to pre-initialise display
// Requires accel interrupts to be running
// Requires adc results to be updated if used
// RTC must be running for valid time
// Call every accelerometer orientation interrupt for watch function
// Call every second for usb operation to display a clock
// Call with -1 to stop the clock being shown until called with 0 again
void RunClockTasks(signed char forceOn)
{
	// The accelOrientation variable is updated in the interrupt
	static unsigned int timer = 0;
	static unsigned char oldOrientation;
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
	// For non-USB, we check for orientation change
	else if	(accelOrientation!=oldOrientation)
	{
		if (
		(((oldOrientation&0b01000110)==0)&& 	// Normal gesture
		((accelOrientation&0b01000001)==0x41))
		||
		(((oldOrientation&0b01000110)==0)&&		// Extended gesture
		((accelOrientation&0b01000110)==0x02))
		||
		(((oldOrientation&0b01000001)==0x41)&&// Side gesture
		((accelOrientation&0b01000110)==0x02)) )

		{
			if (timer == 0)
			{
				DisplayRestore();
				timer = 3;
				showTime = TRUE;
			}
			else
			{
				timer = 5;	// Keep display on
			}
		}
		else
		{
			DisplayOff();
			showTime = FALSE;
			timer=0;
		}
	}
	oldOrientation = accelOrientation;

	// Timer to turn off the display
	if (timer > 0)
	{
		if ( --timer==0)
		{
			DisplayOff();
			showTime = FALSE;
		}
	}

	if (showTime == TRUE)
	{
		memcpy(now,(RtcToString(RtcNow())+11),5);
		now[5] = '\0'; // null terminate
		//DisplayPrintLine(now,2,4); 
		DisplayClearRam(GDbuffer, DISPLAY_WIDTH);
		OutTextXY(16, 16, now);
		DisplayRefresh(GDbuffer,DISPLAY_WIDTH);
		PrintTopDisplayLine();
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
	AdcInit();
    AdcSampleWait();                   

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

