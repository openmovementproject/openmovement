// HardwareProfile.h - Hardware selection and prototypes
// Karim Ladha 2015
#ifndef HARDWAREPROFILE_H
#define HARDWAREPROFILE_H

// Harware selection
#include "..\Common\HardwareProfile-AX9.0.h"
//#include "..\Common\HardwareProfile-CWA3.1.h"

// For legacy drivers, non hardware specific drivers should use config
#include "config.h"

//SystemPwrSave arguments OR together (will also wake on enabled interrupts)
#define WAKE_ON_WDT				0x0001
#define WAKE_ON_RTC				0x0002
#define WAKE_ON_TIMER1			0x0004
#define LOWER_PWR_SLOWER_WAKE	0x0008
extern void SystemPwrSave(unsigned short NapSetting);
// System i/o and peripherals startup function
extern void SystemInit(void);

// LED Colours 0bRGB (SET_LED(c))
enum { LED_OFF, LED_BLUE, LED_GREEN, LED_CYAN, LED_RED, LED_MAGENTA, LED_YELLOW, LED_WHITE };

// Debugging/Error support
typedef enum {
	// Default int fired, no exception
	OTHER_INT = 0,
	// Default int on exception (INTCON1>>1)
	OSC_FAIL = 1,
	STACK_ERR = 2,
	ADDR_ERROR = 4,
	MATH_ERR = 8,
	// Other unrecoverable hardware errors
	BT_ERR = 9,
	I2C_ERR = 10	
}SwResetReason_t;
extern void DebugReset(SwResetReason_t reason);

// Used for delays in TimeDelay.h
#define GetInstructionClock()  	((OSCCONbits.COSC == 0b001)?16000000ul:4000000ul)
#define GetSystemClockMIPS()	((OSCCONbits.COSC == 0b001)?16:4)

#endif
