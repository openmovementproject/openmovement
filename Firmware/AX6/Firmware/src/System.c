// Hardware inspecific functions for PIC24 devices
// Karim Ladha 2015

// Includes
#include <stdint.h>
#include "Compiler.h"
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Peripherals/SysTime.h"
#include "Peripherals/GenericIO.h"
#include "Peripherals/xNand.h"
#include "../Common/BootCheck.h"
#include "../Common/Analog.h"


#ifndef AX9_HW_V3_RGB
#warning "Remember, this project is not being compiled for V3 hardware."
#endif

// Config settings
#if defined(__PIC24FJ256GB106__)
// PIC24FJ1024GB106 Configuration Bit Settings
_CONFIG1(0b0011111101101010) 	// JTAG off, Code prot off, ICSP1, SWWDT, WDT div32, WDT per=1024
_CONFIG2(0b1001000000001111) 	// IESO on, PLL div2 (8MHZ), PLL en, Use FRC, CLKSW en, FSCM en, OSC pins = port, USB reg dis, PRIOSC dis 
_CONFIG3(0b1111111111111111) 	// Code protection bits all off, these are set in the bootloader code anyway
#elif defined(__PIC24FJ256GB206__)
// PIC24FJ1024GB206 Configuration Bit Settings
_CONFIG1(0x3F6A)				// Writes on, PGC1, SWWDT, 1ms TWDT
_CONFIG2(0x170F)				// PLL div2 (8MHZ), SWPLL, Posc off, use FRC
_CONFIG3(0xF4FF)				// No code protect in debug, SOSC clkin, todo:test fast wake (on)
#elif defined(__PIC24FJ1024GB606__) 
#include <xc.h>
// PIC24FJ1024GB606 Configuration Bit Settings
TODO: Change format of config words to either: 
#pragma config CONFIG1 = value
_CONFIG1(value)
// FSEC
#pragma config BWRP = ON                // Boot Segment Write-Protect bit (Boot Segment is write protected)
#pragma config BSS = DISABLED           // Boot Segment Code-Protect Level bits (No Protection (other than BWRP))
#pragma config BSEN = OFF               // Boot Segment Control bit (No Boot Segment)
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GSS = DISABLED           // General Segment Code-Protect Level bits (No Protection (other than GWRP))
#pragma config CWRP = ON                // Configuration Segment Write-Protect bit (Configuration Segment is write protected)
#pragma config CSS = DISABLED           // Configuration Segment Code-Protect Level bits (No Protection (other than CWRP))
#pragma config AIVTDIS = ON             // Alternate Interrupt Vector Table bit (Enabled AIVT)
// FBSLIM
#pragma config BSLIM = 0x1FF6           // Boot Segment Flash Page Address Limit bits (Boot Segment Flash page address  limit)
// FSIGN
// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Source Selection (Internal Fast RC (FRC))
#pragma config PLLMODE = PLL96DIV1      // PLL Mode Selection (96 MHz PLL. Oscillator input is divided by 1 (4 MHz input))
#pragma config IESO = ON                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)
// FOSC
#pragma config POSCMD = NONE            // Primary Oscillator Mode Select bits (Primary Oscillator disabled)
#pragma config OSCIOFCN = ON            // OSC2 Pin Function bit (OSC2 is general purpose digital I/O pin)
#pragma config SOSCSEL = OFF            // SOSC Power Selection Configuration bits (Digital (SCLKI) mode)
#pragma config PLLSS = PLL_FRC          // PLL Secondary Selection Configuration bit (PLL is fed by the on-chip Fast RC (FRC) oscillator)
#pragma config IOL1WAY = OFF            // Peripheral pin select configuration bit (Allow multiple reconfigurations)
#pragma config FCKSM = CSECMD           // Clock Switching Mode bits (Clock switching is enabled,Fail-safe Clock Monitor is disabled)
// FWDT
#pragma config WDTPS = PS1024           // Watchdog Timer Postscaler bits (1:1,024)
#pragma config FWPSA = PR32             // Watchdog Timer Prescaler bit (1:32)
#pragma config FWDTEN = ON_SWDTEN       // Watchdog Timer Enable bits (WDT Enabled/Disabled (controlled using SWDTEN bit))
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
#pragma config WDTWIN = WIN25           // Watchdog Timer Window Select bits (WDT Window is 25% of WDT period)
#pragma config WDTCMX = WDTCLK          // WDT MUX Source Select bits (WDT clock source is determined by the WDTCLK Configuration bits)
#pragma config WDTCLK = LPRC            // WDT Clock Source Select bits (WDT uses LPRC)
// FPOR
#pragma config BOREN = SBOREN           // Brown Out Enable bit (Controlled by SBOREN)
#pragma config LPCFG = OFF              // Low power regulator control (No Retention Sleep)
#pragma config DNVPEN = ENABLE          // Downside Voltage Protection Enable bit (Downside protection enabled using ZPBOR when BOR is inactive)
// FICD
#pragma config ICS = PGD1               // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)
#pragma config BTSWP = OFF              // BOOTSWP Disable (BOOTSWP instruction disabled)
// FDEVOPT1
#pragma config ALTCMPI = DISABLE        // Alternate Comparator Input Enable bit (C1INC, C2INC, and C3INC are on their standard pin locations)
#pragma config TMPRPIN = OFF            // Tamper Pin Enable bit (TMPRN pin function is disabled)
#pragma config SOSCHP = ON              // SOSC High Power Enable bit (valid only when SOSCSEL = 1 (Enable SOSC high power mode (default))
#pragma config ALTVREF = ALTREFEN       // Alternate Voltage Reference Location Enable bit (VREF+ and CVREF+ on RA10, VREF- and CVREF- on RA9)
#else
// Unknown device selection?
#error "Select device"
#endif


        
// Globals
uint16_t rcon_at_startup;	// Reset control register holding reason for last reset
SwResetReason_t __attribute__((persistent)) last_swreset_reason;

#ifndef WAX9_BUILD
// Compatibility strings for boot check
const char __attribute__((space(prog), address(DEVICE_INFO_ADDRESS))) deviceInfo[] = "HW=" HARDWARE_NAME ", REV=" HARDWARE_VERSION ", REL=1.1";/* Simulate bootloader presence */
const char __attribute__((space(prog), address(TARGET_INFO_ADDRESS))) targetInfo[] = "HW=" HARDWARE_NAME ", REV=" HARDWARE_VERSION ; /* Read by the bootloader */
#endif

// Nand memory configuration constants
const IO_pin_t nandPins[TOTAL_NAND_PINS] = {
	{(void*)&NAND_CLE_PORT, (1<<NAND_CLE_PORT_PIN_NUM)},
	{(void*)&NAND_ALE_PORT, (1<<NAND_ALE_PORT_PIN_NUM)},
	{(void*)NULL, 0 /* Write protect pin unused */},
	{(void*)&NAND_DEVICE1_CE_PORT, (1<<NAND_DEVICE1_CE_PIN_NUM)},
	{(void*)&NAND_DEVICE2_CE_PORT, (1<<NAND_DEVICE2_CE_PIN_NUM)},
	{(void*)&NAND_DEVICE1_RB_PORT, (1<<NAND_DEVICE1_RB_PIN_NUM)},
	{(void*)&NAND_DEVICE2_RB_PORT, (1<<NAND_DEVICE2_RB_PIN_NUM)}};


// Exceptions and un-handled interrupts
void __attribute__((interrupt,auto_psv)) _DefaultInterrupt(void)
{
	// Set reset reason and call reset function
	last_swreset_reason = OTHER_INT;
	if		(INTCON1bits.STKERR)	last_swreset_reason = STACK_ERR;
	else if	(INTCON1bits.ADDRERR)	last_swreset_reason = ADDR_ERR;
	else if	(INTCON1bits.MATHERR)	last_swreset_reason = MATH_ERR;	
	INTCON1 = 0;
#ifndef __DEBUG
    DebugReset(last_swreset_reason);
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
// Debug address traps - decodes the return address from the stack
#ifdef __DEBUG
uint32_t trap_return_address;
__attribute__((no_auto_psv,__interrupt__(
	/* Unstack the return address */\
	__preprologue__( \
	"mov #(_trap_return_address+2),w1 \n\t"\
	"pop [w1--] \n\t"\
	"pop [w1++] \n\t"\
	"push [w1--] \n\t"\
	"push [w1++] \n\t")))) 
void _AddressError(void)
{
	/* Check value of trap_return_address after clearing top byte */
	/* It holds the location after the fault operation */
	trap_return_address &= 0xffffff;
	_DefaultInterrupt();
}
#endif

// For release hardware this can help identify crash sources
void DebugReset(SwResetReason_t reason)
{
#ifndef __DEBUG
if(reason != SW_RESET)
#endif
	{
		LED_SET(LED_MAGENTA);
		DelayMs(3000);
		switch (reason){
			case (MCU_ERR):
			case (OTHER_INT):	LED_SET(LED_WHITE);		break;
			case (STACK_ERR):	LED_SET(LED_MAGENTA);	break;
			case (ADDR_ERR):	LED_SET(LED_RED);		break;
			case (MATH_ERR):	LED_SET(LED_BLUE);		break;
			case (BT_ERR):		LED_SET(LED_CYAN);		break;
			case (I2C_ERR):		LED_SET(LED_GREEN);		break;
			case (SW_WDT):		LED_SET(LED_YELLOW);	break;
			case (SW_RESET):	// Software reset from debug only
			default:			LED_SET(LED_OFF);		break;
		}
		DelayMs(3000);
	}
	Reset();
}
// Optional useful debug function. Stops execution until user action 
// Similar to break point but works if not in debug
// Changing USB state will resume execution
void DebugPause(void)
{
	uint8_t usb_entry, usb_now;
	usb_entry = usb_now = USB_BUS_SENSE;
	// Magenta LED
	LED_SET(LED_MAGENTA);
	// Wait till usb changes state
	while(usb_now == usb_entry)
		{usb_now = USB_BUS_SENSE;}
	// Can break here, if debugging
	Nop();	Nop(); 	Nop();	
	// Change USB cable state to resume
}
// Hardware startup sequence
void SystemInit(void)
{
	// Log the reason for resetting
	rcon_at_startup = RCON;
	RCON = 0;
	
	// Pre-boot entry check
	if(!TargetProcessorCheck() || !TargetHardwareCheck())
	{
		// Can't do anything but reset here...
		Reset();
	}
	
	// 8 MHz FRC system clock (default)
	CLOCK_INTOSC();     

	// Set mcu to known state
	InitIO();			// IO pins
	CLOCK_SOSCDIS();	// We have an ex.osc, turn off internal
	SysTimeInit();		// Time module

	// Set flag to re-load persistent battery state
	adcResult.battState = BATT_STATE_UNKNOWN;	

	// Hardware regs - not needed really because these are defaults
	U1PWRCbits.USBPWR = 0; 	// In case usb module is not disabled 
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

// Instruction clock (half the oscillator rate for PIC24)
inline uint16_t __attribute__((always_inline)) GetSystemMIPS(void)
{
	unsigned int mips;
	switch(OSCCONbits.COSC){
		case 0b000 : 
			mips = 4; break;	
		case 0b001 : 
			mips = 16; break;
		case 0b111 :
			mips = (4 >> CLKDIVbits.RCDIV); 
			if(!mips)mips = 1;
			break;
		default:
			mips = 4; break;
	}
	return mips;
}


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



// EOF
