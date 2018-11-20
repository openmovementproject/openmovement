// Sociometer 1.0 19-04-2012
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

#include "Utils/Wav.h"
#include "Utils/Fifo.h"
#include "Utils/Util.h"
#include "Utils/FSUtils.h"

#include "Logger.h"
#include "AudioStream.h"
#include "MDD File System/FSIO.h"

#include "Peripherals/myI2C.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Prox.h"
#include "Codec AIC111 DCI.h"
#include "Analog.h"

// Prototypes
extern void RunAttached(void);
//extern void RunLogging(void);
extern void PrintTopDisplayLine(void);
extern void RunClockTasks(unsigned char forceOn);

// Globals
extern const FONT_FLASH GOLFontDefault; // default GOL font
short dummy;

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
#ifndef __DEBUG
	while (!USB_BUS_SENSE)
	{
		LED_SET(LED_MAGENTA);
		DelayMs(250);
		LED_SET(LED_WHITE);
		DelayMs(250);
	}
    Reset();
#else
	#warning "Default interrupt does not reset"
#endif
}

// RTC
void __attribute__((interrupt,auto_psv)) _RTCCInterrupt(void)
{
	RtcTasks();
	LoggerRTCTasks();
	RtcSwwdtIncrement();    // Increment software RTC, reset if overflow
}
// TMR1
void __attribute__((interrupt, shadow, auto_psv)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;
	// It would be better not to use this in this project
}

// Main routine
int main(void)
{
	unsigned short rcon = RCON;
	RCON = 0;

    // Initialization
	CLOCK_INTOSC();		// Oscillator
	InitIO();			// I/O pins
	myI2Cclear();		// Once
	InitI2C();			// Once

	// Make *certain* these are off at this point
	gDataReady.all = 0;
	gStreamEnable.all = 0;
	
	// Ensure our first ADC reading is valid 
	AdcInit();
	AdcSampleNow();
	AdcOff();

	// Do this before the prox. may get updated in the RTC interrupt
	AccelVerifyDeviceId();
	AccelStandby();
#ifdef HAS_PROX
	ProxVerifyDeviceId();
	ProxStandby();
#endif

	// Start RTC and the ISR
	RtcStartup();		// RTC
	RtcInterruptOn(0); 	// RTC + Timer1

	// Display
	#ifndef NO_DISPLAY
		DisplayInit();
		DisplayClear();
		Display_print_xy(" -= WAM =-",0,2,2);
		#ifndef BASIC_DISPLAY_FONTS
			SetFont((void *) &pirulen_28);		// Once, new font
		#endif
		DelayMs(250);
		LED_SET(LED_OFF);
	#endif

	SettingsInitialize();
	status.resetReason = rcon;
	
	DisplayOff();	
	CodecInitOff();	// It should be off anyway after InitIO
	SD_DISABLE(); 	// It should be off anyway after InitIO

	// Init FS & read new settings from disk
	SD_ENABLE(); 			// Turn on SD card
	CLOCK_PLL();			// Oscillator


	// Check card is OK, the load settings if it is
	// Retry by power cycling card upto 5 times.
	unsigned int timeout = 5;
	while(timeout--)
	{
		MEDIA_INFORMATION *mediaInformation = MDD_SDSPI_MediaInitialize();
	    if (mediaInformation->errorCode == MEDIA_NO_ERROR)
	    {
			if (FSInit() == TRUE)	// Init FS
			{
				SettingsReadFile(SETTINGS_FILE);
				break;
			}
			break;					// No file system, unformatted card?
	    }
		else
		{
			// SD init error
			SD_DISABLE(); 			// 500ms, power off card
			SD_ENABLE();			// Power on card
			LED_SET(LED_MAGENTA);	// Indicate error
			DelayMs(500);			// Wait extra time for card to power up
			LED_SET(LED_OFF);		// Turn off LED
			if(timeout == 0) gSD_CARD_DETECT = FALSE; // Card not present or HW fail
			continue;				// Retry init
		}
	}

	// Run as attached or logging
	if (USB_BUS_SENSE)
	{
		RunAttached();      // Run attached, returns when detatched
    }
    else
    {
		RunLogging();
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

	memcpy(now,(char*)(RtcToString(RtcNow())),20);
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
	temptr = (const char*)my_itoa((short)batt);
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

unsigned char gButton = 0;
unsigned char gButtonClick = 0;
int gButtonTimer = 0;
#define BUTTON_TIMER 2
unsigned char UpdateProximity(unsigned char doWait)
{
#ifdef HAS_PROX
	unsigned char newButton;
	
	if (doWait){
		// Method 1
		unsigned short timeout = 0xfff;
		while(!ProxSampleReady() && timeout--);
		ProxStartSample();
		Delay10us(40);
		while(!ProxSampleReady() && timeout--);
		ProxReadSample();}
	else {
		// Method 2, no waits, not instant though
		ProxReadSample();
		ProxStartSample();}

	if (settings.proximity != 0 && prox.proximity > settings.proximity)			newButton = 2;	
//	else if (prox.proximity > PROX_LIGHT_TOUCH) newButton = 1;
	else 										newButton = 0;	
	
	// Perform 'click' calculation
	if (gButton < newButton) { gButtonTimer = BUTTON_TIMER; }			// reset timer when pressed (or pressed harder)
	gButtonClick = (newButton == 0 && gButton && gButtonTimer > 0);		// fully released within a small interval of being pressed
	if (gButtonTimer) { gButtonTimer--; }
	
	// Update button
	gButton = newButton;
#endif
	
	return gButton;
}



// Media Detect
BYTE MDD_My_MediaDetect(void)
{
	if (!status.diskMounted) { return FALSE; }
	return MDD_SDSPI_MediaDetect();
}


// Sector read that allows intentionally corrupt first sectors ("DISKBREAK")
// - Repair them on-the-fly while reading through the device
// - A direct card read will still return the invalid data
BYTE MDD_My_SectorRead(DWORD sector_addr, BYTE* buffer)
{
	BYTE ret;
	
	// NOTE: This enables a non-standard SD card to work on the device, but not the pass-through USB (see SD-SPI.c for that patch)
	
	ret = MDD_SDSPI_SectorRead(sector_addr, buffer);
	
#ifdef ENABLE_NONSTANDARD_DISK
	// Always correct a swapped tail byte in sector 0 or 1 -- #ifdef ENABLE_NONSTANDARD_DISK
	if (sector_addr <= 1 && buffer != NULL && buffer[510] == 0xAA && buffer[511] == 0x55)
	{ 
		buffer[510] = 0x55; buffer[511] = 0xAA;
	}
#endif
	return ret;
}	

// Test for lowest power logging current - place in code somewhere to use
#if 0
	AudioStreamInit();
	CodecInit();						
	AudioSamplingInit();
	// Peripherals powered down
	NVMCONbits.NVMSIDL = 1; // Flash voltage regulator goes into Stand-by mode during Idle mode +30us resume
	PMD1 = 0b1110011001110110; // Leave on T1,T2,DCI,I2C1,SPI1,AD1
	PMD2 = 0b1111111111111110; // Leave on OC1
	PMD3 = 0b1111010010101011; // Leave on RTCC,PMP
	PMD4 = 0b0000000000101001; // Leave on USB
	PMD5 = 0b1111111111111111;
	PMD6 = 0b0111111100000011;
	PMD7 = 0b0000000011100000; // Leave on DMA0
	
	// Set clock for idle here		
	//CLOCK_INTOSC();CLKDIVbits.FRCDIV = 0b101; /* 230kHz Fosc*/ // 1.550mA
	CLOCK_EC(); // 1.568mA
	Sleep();
	//CLOCK_INTRC();	// 1.18mA
	//CLOCK_32KHZ(); // 1.18mA
	
	while(1)
	{
	Idle();
	LED_R=1;
	Delay10us(1);
	LED_R=0;
	}
#endif



//EOF

