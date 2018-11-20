// Source code for watch function
// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Rtc.h"
#include "Peripherals/Analog.h"
#include "Utils/Util.h"
#include "GraphicsConfig.h"
#include "Graphics/DisplayBasicFont.h"
#include "Graphics/Primitive.h"
#include "Peripherals/myi2c.h"

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
// Returns true if Display is on
unsigned char RunClockTasks(signed char forceOn)
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
		DisplayClearRam(GDbuffer, DISPLAY_WIDTH);
#ifndef BASIC_DISPLAY_FONTS
		OutTextXY(16, 16, now);
		DisplayRefresh(GDbuffer,DISPLAY_WIDTH);
#else
		DisplayPrintLine(now,3,4);
#endif
		PrintTopDisplayLine();
	}
	return showTime;
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

