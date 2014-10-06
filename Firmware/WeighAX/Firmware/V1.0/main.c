/* 
 * Copyright (c) 2009-2013, Newcastle University, UK.
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
 
// NPRI 1.0

// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"

#include "GraphicsConfig.h"
#include "Graphics/GOL.h"
#include "Graphics/DisplayDriver.h"
#include "Graphics/Primitive.h"
#include "Graphics/DisplayBasicFont.h"
#include "Graphics/PIRULEN32PIX.h"

#include "Analog.h"
#include "Utils/Util.h"
#include "Settings.h"

#include "Peripherals/Rtc.h"

#include "TelitModule.h"
#include "Scales.h"
#include "WaveCap.h"

// Prototypes
extern void RunAttached(void);
//extern void RunLogging(void);
extern void PrintTopDisplayLine(void);
extern void RunClockTasks(unsigned char forceOn);

// Version string to include in the message
#define VERSION_STRING "3"

static enum {
	NO_DATA = 0,
	SCALES_DATA = 1,
	TEST_DATA = 2
}dataToSend = 0;

char myString[32]; // For the screen only
char* FormDebugMessage(void);
char* FormNPRIMessage(void);
unsigned char LowPowerWaitingLoop(void);
unsigned char StartSmsLoop(void);
unsigned char SendSmsLoop(void);
unsigned char CloseSmsLoop(void);

// Globals
extern const FONT_FLASH GOLFontDefault; // default GOL font
short dummy;

// Credits
#define MAX_CREDITS 4                           // Maximum number of credits
#define CREDIT_RATE (1 * 60 * 60)               // 1 hour
unsigned int credits = 0;                   	// Initially have a no credits

unsigned int creditTimer = CREDIT_RATE - 30;    // Make first credit increment wait a short while

// Bootloader fix
void _FirstIVTEntry (void)
{
	asm("goto __resetPRI");
}

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
#ifdef __DEBUG
	#warning "Default interrupt won't reset."
#else
    Reset();
#endif
}

// RTC
void __attribute__((interrupt,auto_psv)) _RTCCInterrupt(void)
{
	RtcTasks();
	
	// Credits
	creditTimer++;
	if (creditTimer >= CREDIT_RATE)
	{
		creditTimer = 0;
		credits++;
		if (credits > MAX_CREDITS) { credits = MAX_CREDITS; }
	}
}

// TMR1
void __attribute__((interrupt, shadow, auto_psv)) _T1Interrupt(void)
{
	RtcTimerTasks();
}

const char __attribute__((space(prog),address(GSM_PHONE_NUMBER_ADDRESS)))npri_server_phone_number_string[]  = "07937405006";

char* FormDebugMessage(void)
{
	static char smsBody[161] = {0};
	char* ptr = smsBody;
	DateTime time = RtcNowFractional(NULL);
	unsigned long epoc = SecondsSinceEpoch(DATETIME_YEAR(time),DATETIME_MONTH(time),DATETIME_DAY(time),DATETIME_HOURS(time),DATETIME_MINUTES(time),DATETIME_SECONDS(time));
	memset(smsBody, 0, sizeof(smsBody));

	ptr += sprintf(ptr, "DEBUG" VERSION_STRING ",%s,",gsmState.iccid);
	ptr += sprintf(ptr, "%lu,",epoc);
	ptr += sprintf(ptr,"%d,",(2*AdcToMillivolt(adcResult.batt)));
	ptr +=sprintf(ptr, "-1,,"); // Units = -1, No weight data

	ptr += sprintf(ptr, "%s,",gsmState.imsi );
	ptr += sprintf(ptr, "%d,",gsmState.rssi );
	ptr += sprintf(ptr, "%s",gsmState.networkName );

	gsmState.smsText = smsBody;
	gsmState.phoneNumber = SettingsGetDefaultNumber();
	return smsBody;	


	gsmState.smsText = smsBody;

	gsmState.phoneNumber = SettingsGetDefaultNumber();

	return smsBody;	
}

// This makes the sms text from the scale data
// NPRI,<IMSI>,<RTC-timstamp-seconds>,<battery-mV>,<units>,<weight>

char* FormNPRIMessage(void)
{
	static char smsBody[161] = {0};
	DateTime time;
	unsigned long timeSinceEpoch;
	unsigned int battery;
	char* ptr;
	int len;

	// Collect data
	time = RtcNowFractional(NULL);
	timeSinceEpoch = SecondsSinceEpoch(DATETIME_YEAR(time),DATETIME_MONTH(time),DATETIME_DAY(time),DATETIME_HOURS(time),DATETIME_MINUTES(time),DATETIME_SECONDS(time));
	battery = 2 * AdcToMillivolt(adcResult.batt);
	
	// Reset SMS
	memset(smsBody, '?', sizeof(smsBody));
	smsBody[160] = 0;
	ptr = smsBody;

	// DEBUG: Carefully appending each bit and checking return value
	// Message type and firmware/message version
	len = sprintf(ptr, "NPRI" VERSION_STRING ",");
	if (len >= 0) { ptr += len; }
	
	// SIM Card ICCID
	len = sprintf(ptr, "%s,", gsmState.iccid);
	if (len >= 0) { ptr += len; }
	
	// Seconds since epoch
	len = sprintf(ptr, "%lu,", timeSinceEpoch);
	if (len >= 0) { ptr += len; }
	
	// Battery reading
	len = sprintf(ptr, "%d,", battery);
	if (len >= 0) { ptr += len; }
	
	// Measurement units and weight
	switch (gScaleData.units)
	{
		case 0:		// kgs
		{
			len = sprintf(ptr, "0,%d.%d", (gScaleData.weight/10), (gScaleData.weight%10));	
			break;
		}			
		case 1:		// lbs
		{	
			len = sprintf(ptr, "1,%d.%d", (gScaleData.weight/10), (gScaleData.weight%10));	
			break;
		}
		case 2:		// st:lbs
		{	
			len = sprintf(ptr, "2,%d:%d.%d", ((unsigned short)gScaleData.weightMSB), ((unsigned short)gScaleData.weightLSB/10), ((unsigned short)gScaleData.weightLSB%10));
			break;
		}
		default: 	// (error)
		{
			len = sprintf(ptr, "%d,0", gScaleData.units);
			break;
		}
	}
	if (len >= 0) { ptr += len; }

	// Assign message
	gsmState.smsText = smsBody;
	gsmState.phoneNumber = SettingsGetDefaultNumber();
	
	return smsBody;																						// Total = 134 max
}

//#define USB_BUS_SENSE_LOCAL 0
#define USB_BUS_SENSE_LOCAL USB_BUS_SENSE

// Main routine
int main(void)
{
	unsigned char error;

	Nop();
	RCON = 0;
	
    // Initialization
	InitIO();			// I/O pins
	CLOCK_INTOSC();		// Oscillator
	UpdateAdc();		// Initial value	

	// Display
	DisplayInit();
	DisplayClear();
	PrintTopDisplayLine();
	Display_print_xy("-= NPRI =-",3,2,2);
	SetFont((void *) &pirulen_28);		// Once, new font

	// Scale 'wait for data' loop
	RtcStartup();
	RtcInterruptOn(0); 	// RTC on, will cause the wait function to return every second

	SettingsInitialize(); // Load default settings: device id and default number
	UpdateAdc();

	// The usb state will run if connected at this point - after this, a reset is needed
	if (USB_BUS_SENSE_LOCAL)
	{
		RunAttached();      // Run attached, returns when detatched
    }

	ScalesCommInit();	// Init scales comms - starts receiver

	error = 0;
	while (error == 0)
	{
		if(LowPowerWaitingLoop())
		{
			if (credits <= 0)
			{
				LED_SET(LED_RED);
				DisplayInit();
				DisplayClear();
				Display_print_xy("No credits",0,4,1);
				DelayMs(1000);
				DisplayOff();
				dataToSend = NO_DATA; // Data will be lost
			}
			else
			{
				credits--;
				
				if (StartSmsLoop())
				{
					if(!SendSmsLoop())
					{
						error++;
					}
					else
					{
						dataToSend = NO_DATA;
					}
				}
				else
				{
					error++;
				}
				
				if(!CloseSmsLoop())
				{
					error++;
				}
			}
		}
		if(USB_BUS_SENSE) break;
	}

	// Turn off everything and power down
	ScalesCommOff();
	GsmOff();
	DisplayOff();
	Reset();                // Reset - resets all states, goes back to polling scales
	return 0;
} // end of main

unsigned char LowPowerWaitingLoop(void)
{
	static unsigned short lockeduptime = 0;
	static signed char display_timer = 0;

    // Clear any garbage
    if (ScalesDataWaiting())
    {
        ScalesCommClear();
    }
    
	// Wait for data, usb connect or button
	while((!USB_BUS_SENSE_LOCAL) && (dataToSend == NO_DATA))
	{	
		// Woken by data or rtc, if data is received - check it
		if (ScalesDataReady())
		{
			// If the data is OK - break out of this loop into next one
			if (ScalesParseData() == TRUE)
			{
				// Scale data available and checked for integrity
				ScalesCommClear(); 	// Leave scales receiver running incase a second weighing occurs
				dataToSend = SCALES_DATA;
				break;				// New data in global vaiable may need sending - return				
			}
			else
			{
				DisplayInit();
				DisplayClear();
				Display_print_xy("Data corrupt.",0,0,1);
				display_timer = 3;
			}
			// Clear the receiver for next time if the data is currupt
			ScalesCommClear();
		}

		// Wait for scales activity - should be very low power as it is normally in sleep
		ScalesCommWait(); // Wakes every one second

		// Check for lock up - waiting state means pin went high but still waiting for a packet
		if (ScalesDataWaiting())
		{
			unsigned short now = RtcSeconds();
			
			if (lockeduptime==0)
			{
				lockeduptime = now;
				DisplayInit();		// Indicate wake condition on display
				display_timer = 0; 	// Indicate wake condition on display
			}

			if ((now - lockeduptime) > 	MAX_WAIT_TIME_FROM_SCALES_ON_TO_DATA) // Allow upto 30s for the measurement
			{
				lockeduptime = 0;	// Reset lockup count
				ScalesCommClear();	// Clear lockup condition
			}
		}
		else
		{
			lockeduptime = 0;
		}

		if (!BUTTON)
		{
			dataToSend = TEST_DATA;
			break;
		}
		
		// Show that loop is running and waiting - display stuff
		{	
			if (display_timer == 0)
			{
				DisplayClear();
				Display_print_xy("Awaiting data.",0,0,1);
			}
			else if (display_timer<3)
			{
				Display_putc_basic('.',1);
			}
			else if (display_timer<6)
			{
				display_timer = 100;
				DisplayClear();
				DisplayOff();
			}
			else
			{
				// Latches here
				display_timer = 100;	
			}
			display_timer++;
		}

		// Flash the LED a bit to show its not dead	
		LED_SET(LED_WHITE);
		Delay10us(1);
		LED_SET(LED_OFF);

	}  	// end waiting for data low power state	

	// Indicate new data type
	LED_SET(LED_WHITE);
	DisplayInit();
	DisplayClear();

    {
        const char *mtype = "?";
        if (dataToSend == TEST_DATA) { mtype = "TEST MSG."; }
        else if (dataToSend == SCALES_DATA) { mtype = "NEW DATA."; }
        
        sprintf(myString,"%s-%d", mtype, credits);
		Display_print_xy(myString,0,0,2);
    }
    
	return dataToSend;
}		// end of waiting function

// Starts the radio and drives state machine to point where it can accept sms messages
unsigned char StartSmsLoop(void)
{
	unsigned short timeout = (60 * GSM_1S_TIMEOUT);	// Number of loop iterations
	unsigned char ledToggle = 0;

	// Early out if invalid or no number stored
	if(settings.gsmDefaultNumber == NULL)
	{
		#ifdef __DEBUG
			SettingsSetDefaultNumber((char*)default_phone_number_string);
		#else
			int i;
			DisplayInit();
			DisplayClear();
			Display_print_xy("No number set!",0,4,1);
			for(i=0;i<25;i++){LED_SET(LED_MAGENTA);DelayMs(100);LED_SET(LED_OFF);DelayMs(100);}
		return FALSE;
		#endif
	}

	DisplayInit();
	DisplayClear();
	Display_print_xy("Initing radio..  ",0,4,1);

	// Clear gsm state machine 
	gsmState.machineState = INIT_STATE;
	GsmOff();

	// If no usb attach and no timeout
	while (!USB_BUS_SENSE_LOCAL && --timeout)
    {
		// Tasks for loop
		UpdateAdc();
		GsmStateMachine();
		PrintTopDisplayLine();

		// Return once we have the required parameters
		if(	(gsmState.regState == 1 || gsmState.regState == 5) && 
			(gsmState.imsi != NULL) && 
			(gsmState.iccid != NULL) && 
			(gsmState.networkName != NULL) && 
			(gsmState.machineState == DEFAULT_READY) )
		{
			// Registered on network
			// Have network name
			// Have imsi number
			// Have rssi
			// State machine is awaiting next state	
			DisplayClear();
			Display_print_xy("Radio ready..  ",0,4,1);
			LED_SET(LED_GREEN);
			return TRUE;
		}

		// Debug flash codes
		if ((gsmState.regState != 1 && gsmState.regState != 5))
		{	
			// Awaiting signal - awaiting signal etc., flash red
			if(ledToggle) 	{ledToggle = 0; LED_SET(LED_RED);}
			else 			{ledToggle = 1;	LED_SET(LED_OFF);}
		}
		else 
		{	
			// Signal aquired, waiting for send flash yellow
			if(ledToggle) 	{ledToggle = 0; LED_SET(LED_YELLOW);}
			else 			{ledToggle = 1;	LED_SET(LED_OFF);}
		}
		// We could have a low power state here but it would not make much difference
		DelayMs(100);
	}
	// Timed out
	gsmState.machineState = INIT_STATE;
	GsmOff();
	return FALSE;
}

// Call this is the radio is initialised and ready
unsigned char SendSmsLoop(void) // Looks at global dataToSend 
{
	unsigned short timeout = (30 * GSM_1S_TIMEOUT);	// Number of loop iterations
	unsigned char state = 0;						// Local state
	unsigned char result = FALSE;
	unsigned char ledToggle = 0;

	DisplayInit();
	DisplayClear();
	Display_print_xy("Sending SMS..  ",0,4,1);

	// Checks that we are ready to send an sms
	if(	(gsmState.machineState != DEFAULT_READY) || 
		(gsmState.imsi == NULL) || 
		(gsmState.networkName == NULL) ||
		(gsmState.regState != 1 && gsmState.regState != 5) 	)
	{	
		DisplayClear();
		Display_print_xy("Called too early  ",0,4,1);
		GsmOff();
		return FALSE;		
	}

	// If no usb attach and no timeout
	while (!USB_BUS_SENSE_LOCAL && --timeout)
	{
		if(state == 0)
		{
			// Form a message
			if(dataToSend == 1)			FormNPRIMessage();
			else if(dataToSend == 2)	FormDebugMessage();

			sprintf(myString,"Sending SMS.         ");
			Display_print_xy(myString,0,4,1);

			state = 1;
			// Tell state machine to send it
			gsmState.machineStateQueue = GSM_SEND_SMS;
			gsmState.result = -1;
		}
		else if (state == 1)
		{
			// Waiting on response from state machine
			if (gsmState.result == TRUE)
			{
				LED_SET(LED_GREEN); // Will persist
				sprintf(myString,"SMS sent ok.        ");
				Display_print_xy(myString,0,4,1);
				result = TRUE;
				break;
			}
			else if (gsmState.result == FALSE)
			{
				// SMS failed
				LED_SET(LED_RED); // Will persist
				sprintf(myString,"SMS failed.         ");
				Display_print_xy(myString,0,4,1);
				result = FALSE;
				break;
			}
			// Checks to see if there has been a state machine error
			if(	gsmState.regState != 1 && gsmState.regState != 5) 	
			{	
				DisplayClear();
				Display_print_xy("Signal lost",0,4,1);
				result = FALSE;
				break;	
			}
			// Sending SMS - awaiting confirmation, flash green
			if(ledToggle) 	{ledToggle = 0; LED_SET(LED_GREEN);}
			else 			{ledToggle = 1;	LED_SET(LED_OFF);}
		}

	// Drive machine
	UpdateAdc();
	GsmStateMachine();
	PrintTopDisplayLine();

	// We could have a low power state here but it would not make much difference
	DelayMs(100);
	} // end while

	return result; // Message sent 
} // End sendsms 


// Turn off radio with timeout if unresponsive
unsigned char CloseSmsLoop(void)
{
	unsigned short timeout = (10 * GSM_1S_TIMEOUT);;	// Number of loop iterations
	unsigned char state = 0;	// Local state
	unsigned char result = FALSE;
 
	while (!USB_BUS_SENSE_LOCAL && --timeout)
	{
		// Tasks for loop
		UpdateAdc();
		GsmStateMachine();
		PrintTopDisplayLine();
	
		// Timeout/power down tasks - After time runs out of message is sent
		if (state < 3)
		{
			if (state <= 0)
			{
				sprintf(myString,"Shutting off.       "); 	// Shut down starting
				Display_print_xy(myString,0,4,1);
				state = 1;	
			}
			else if (state == 1)				 			// Turn off initiated in queue	
			{
				gsmState.machineStateQueue = GSM_SHUT_DOWN;
				gsmState.result = -1;
				state = 2;
			}
			else if (state == 2)							// State machine response
			{
				if (gsmState.result == -1)						// State machine response pending	
				{
					;
				}
				if	(gsmState.result == TRUE)
				{
					sprintf(myString,"Shutdown OK.        ");	// State machine responded OK
					Display_print_xy(myString,0,4,1);
					result = TRUE;
					state = 3;						
				}
				else if (gsmState.result == FALSE)
				{
					sprintf(myString,"Shutdown fail.      ");	// State machine said FAIL
					Display_print_xy(myString,0,4,1);
					result = FALSE;
					state = 3;	
				}
			}
			else if (state == 3)
			{
				break;											// Module has powered down
			}

			// We could have a low power state here but it would not make much difference
			DelayMs(100); 
		} // end if
	} // end while

	GsmOff();
	return result;
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

	memcpy(now,(char*)(RtcToString(RtcNow())),20);
	now[10] = '\0'; // null terminate
	now[16] = '\0'; // null terminate
	now[19] = '\0'; // null terminate

	// Time
	Display_print_xy(&now[11], 0, 0, 2);	// Time

	// Battery %
	// Update the battery %
	batt = AdcBattToPercent(adcResult.batt);
	temptr = (const char*)my_itoa((short)batt);
	Display_gotoxy(85, 1);
	Display_putc_basic(' ',1);					// space
	Display_print_xy((char*)temptr, 90, 1, 1);	// Batt
	Display_putc_basic('%',1);					// %

//	batt = 2*AdcToMillivolt(adcResult.batt);
//	temptr = (const char*)my_itoa((short)batt);
//	Display_gotoxy(81, 1);		
//	Display_putc_basic(*temptr++,1);					
//	Display_putc_basic('.',1);				
//	Display_putc_basic(*temptr++,1);
//	Display_putc_basic(*temptr++,1);
//	Display_putc_basic('v',1);

	// Draw battery symbol / level
	Display_gotoxy(112,1);
	DisplayWriteData(0b00011000);
	DisplayWriteData(0b11111111);
	if (!USB_BUS_SENSE_LOCAL) // Not charging
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
	
	{
			 if(!gsmState.present)		Display_print_xy("  radio off", 62, 0, 1);
		else if(gsmState.regState == 4)	Display_print_xy("   starting", 62, 0, 1);
		else if(gsmState.regState == 0)	Display_print_xy(" no network", 62, 0, 1); 
		else if(gsmState.regState == 2)	Display_print_xy("  searching", 62, 0, 1);
		else //(gsmState.regState == 1 || gsmState.regState == 5)
		// Draw the signal strength bars
		{
			unsigned short temp = gsmState.rssi;
			char* tmptr = gsmState.networkName;
			
			// There are 8 char slots max for the name - we use 4 with padding
			if (gsmState.networkName != NULL)
			{
				Display_gotoxy(63, 0);
				for(i=0;i<3;i++){Display_putc_basic(' ',1);}
				for(i=0;i<4;i++){if(*tmptr != '\0')Display_putc_basic(*tmptr++,1);else Display_putc_basic(' ',1);}
				if (gsmState.regState == 5 )Display_putc_basic('R',1);
				else 						Display_putc_basic(' ',1);
			}

			// Signal strength symbol
			if (temp==99)temp = 0; 	// No signal case - no bars
			temp = (temp+4)/5; 		// Into 'bars'
			if(temp>5) temp = 5;	// Clamp
			Display_gotoxy(112,0);
			if(temp>0)	{DisplayWriteData(0b00000000);DisplayWriteData(0b01000000);DisplayWriteData(0b00000000);}
			else		{DisplayWriteData(0b01000000);DisplayWriteData(0b01000000);DisplayWriteData(0b00000000);}
			if(temp>1)	{DisplayWriteData(0b01100000);DisplayWriteData(0b01110000);DisplayWriteData(0b00000000);}
			else		{DisplayWriteData(0b01000000);DisplayWriteData(0b01000000);DisplayWriteData(0b00000000);}
			if(temp>2)	{DisplayWriteData(0b01111000);DisplayWriteData(0b01111100);DisplayWriteData(0b00000000);}
			else		{DisplayWriteData(0b01000000);DisplayWriteData(0b01000000);DisplayWriteData(0b00000000);}
			if(temp>3)	{DisplayWriteData(0b01111110);DisplayWriteData(0b01111111);DisplayWriteData(0b00000000);}
			else		{DisplayWriteData(0b01000000);DisplayWriteData(0b01000000);DisplayWriteData(0b00000000);}
			if(temp>4)	{DisplayWriteData(0b01111111);DisplayWriteData(0b01111111);DisplayWriteData(0b00000000);}
			else		{DisplayWriteData(0b01000000);DisplayWriteData(0b01000000);DisplayWriteData(0b00000000);}
		}
	}

	#ifdef __DEBUG
	sprintf(now,"State=%d    ",gsmState.machineState);
	Display_print_xy(now,0,7,1);
	#endif
	return;
}

