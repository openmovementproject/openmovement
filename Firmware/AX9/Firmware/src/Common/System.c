// Hardware inspecific functions for PIC24 devices
// Karim Ladha 2015

// Includes
#include <stdint.h>
#include "Compiler.h"
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Peripherals/SysTime.h"

// Config settings
_CONFIG1(0b0011111101101010) // JTAG off, Code prot off, ICSP1, SWWDT, WDT div32, WDT per=1024
_CONFIG2(0b0001000000001111) // IESO on, PLL div2 (8MHZ), PLL off, Use FRC, FSCM on, PRIOSC off, OSC pins = port, USB reg off  
_CONFIG3(0b1111111111111111) // Code protection bits all off, these are set in the bootloader code anyway

// Globals
uint16_t rcon_at_startup;	// Reset control register holding reason for last reset

// Exceptions and un-handled interrupts
void __attribute__((interrupt,auto_psv)) _DefaultInterrupt(void)
{
	// Set reset reason and call reset function
	SwResetReason_t reason = OTHER_INT;
	if		(INTCON1bits.STKERR)	reason = STACK_ERR;
	else if	(INTCON1bits.ADDRERR)	reason = ADDR_ERR;
	else if	(INTCON1bits.MATHERR)	reason = MATH_ERR;	
	INTCON1 = 0;
#ifndef __DEBUG
    DebugReset(reason);
#else
	{
		uint8_t usb_entry, usb_now;
		usb_entry = usb_now = USB_BUS_SENSE;
		// Magenta LED
		LED_SET(LED_MAGENTA);
		// Wait till usb changes state
		while(usb_now == usb_entry){usb_now = USB_BUS_SENSE;}
		// Break here, (dis)connect usb and step into the faulty code
		Nop();	Nop(); 	Nop();	
	}
#endif
}

// For release hardware this can help identify crash sources
void DebugReset(SwResetReason_t reason)
{
	LED_SET(LED_MAGENTA);
	DelayMs(3000);
	switch (reason){
		case (OTHER_INT):	LED_SET(LED_OFF);		break;
		case (STACK_ERR):	LED_SET(LED_MAGENTA);	break;
		case (ADDR_ERR):	LED_SET(LED_RED);		break;
		case (MATH_ERR):	LED_SET(LED_BLUE);		break;
		case (BT_ERR):		LED_SET(LED_CYAN);		break;
		case (I2C_ERR):		LED_SET(LED_GREEN);		break;
		case (SW_WDT):		LED_SET(LED_YELLOW);	break;
		default:			LED_SET(LED_WHITE);		break;
	}
	DelayMs(3000);
	Reset();
}

// Hardware startup sequence
void SystemInit(void)
{
	// Log the reason for resetting
	rcon_at_startup = RCON;
	RCON = 0;

	// 8 MHz FRC system clock (default)
	CLOCK_INTOSC();     

	// Set mcu to known state
	InitIO();			// IO pins
	CLOCK_SOSCDIS();	// We have an ex.osc, turn off internal
	SysTimeInit();		// Time module

	// Hardware regs - not needed really because these are defaults
	U1PWRCbits.USBPWR = 0; 	// Incase usb module is not disabled 
	RCONbits.PMSLP = 0;		// Ensure we use lowest power sleep

	// Set other hardware to known state
	myI2Cclear();		// Once only, finish any unfinished i2c comms
}

// Sleep mode - External peripherals all allowed to run unless specified
void SystemPwrSave(unsigned short NapSetting)
{
	//KL WARNING - modified for simpler behaviour, manage your own interrupt wake sources
	uint16_t IPLshadow; // Interrupt context saving

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

// CPU instruction clock rate
inline uint16_t __attribute__((always_inline)) GetSystemMIPS(void)
{
	return((OSCCONbits.COSC == 0b001)?16:4);
}

// Software watchdog timer
#ifdef RTC_SWWDT_TIMEOUT
static volatile uint16_t swwdtCount = RTC_SWWDT_TIMEOUT;
// Called every second
inline void __attribute__((always_inline)) SwwdtTasks(void){if(--swwdtCount == 0)DebugReset(SW_WDT);}
// Clear count
inline void __attribute__((always_inline)) SwwdtReset(void){swwdtCount = RTC_SWWDT_TIMEOUT;}
#endif

// Stop watch functions using timer2/3
// Cycle overhead
#ifdef __OPTIMIZE__
#define STOPWATCH_OVERHEAD	7
#else
#define STOPWATCH_OVERHEAD	12
#endif
// Useful for measuring accurate cycle counts
uint32_t stopwatch_count;
inline void __attribute__((always_inline)) StopWatchStart(void)
{
	IEC0bits.T3IE = 0;	// Disable isr
	T2CON = 0x0008; 	// 32bit mode
	T3CON = 0x0000; 	// Ignored reg
	PR3 = 0xffff;		// Set period	
	PR2 = 0xffff;		// Set period
	TMR3 = 0;			// Clear counter
	TMR2 = 0;			// Clear counter
	IFS0bits.T3IF = 0;	// Clear int flag
	T2CONbits.TON = 1;	// Start timer
	// Done.. timer is counting at Tcy
}
inline void __attribute__((always_inline)) StopWatchStop(void)
{
	// Stop the timer, read cycle count...
	T2CONbits.TON = 1;	// Start timer
	stopwatch_count = ((uint32_t)TMR3 << 16) + TMR2;
	stopwatch_count -= STOPWATCH_OVERHEAD;
	if(IFS0bits.T3IF)stopwatch_count = 0;
	Nop();
	Nop();
	Nop();
	Nop();
}
