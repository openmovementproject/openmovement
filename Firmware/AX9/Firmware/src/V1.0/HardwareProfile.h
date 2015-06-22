// HardwareProfile.h - Hardware selection and prototypes
// Karim Ladha 2015

#ifndef HARDWAREPROFILE_H
#define HARDWAREPROFILE_H

// Harware selection
#include "..\Common\HardwareProfile-AX9.0.h"
//#include "..\Common\HardwareProfile-CWA3.1.h"

// For legacy drivers, non hardware specific drivers should use config
#include "config.h"

// SystemPwrSave arguments OR together (will also wake on enabled interrupts)
#define WAKE_ON_WDT				0x01
#define WAKE_ON_RTC				0x02
#define WAKE_ON_TIMER1			0x04
#define LOWER_PWR_SLOWER_WAKE	0x08
extern void SystemPwrSave(unsigned short NapSetting);

// Other hardware functions for the system
void SystemInit(void);

// LED Colours 0bRGB (SET_LED(c))
enum { LED_OFF, LED_BLUE, LED_GREEN, LED_CYAN, LED_RED, LED_MAGENTA, LED_YELLOW, LED_WHITE };

// Debugging/Error reset reasons
typedef enum {
	OTHER_INT,
	STACK_ERR,
	ADDR_ERR,
	MATH_ERR,
	SW_WDT,
	BT_ERR,
	I2C_ERR,
}SwResetReason_t;
extern void DebugReset(SwResetReason_t reason);

// Used for legacy delays/drivers etc.
#define GetInstructionClock()  	((OSCCONbits.COSC == 0b001)?16000000ul:4000000ul)

#endif
