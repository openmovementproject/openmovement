// HardwareProfile.h - Hardware Profile switching header
// Karim Ladha, Dan Jackson, 2011-2012

#ifndef HARDWAREPROFILE_H
#define HARDWAREPROFILE_H

#include "HardwareProfile-FridgeMon1.0.h"

    // --- Required functions for all hardware ---

    // Wait until the battery is out of pre-charge
    void WaitForPrecharge(void);

    // Power save the system
    void SystemPwrSave(unsigned long napSetting);


#if defined(__C30__)
    // Non PSV data address access:
    #define ROM_ADDRESS(_v) (((unsigned long) __builtin_tblpage(_v) << 16) + __builtin_tbloffset(_v))
#endif
    
    // Read from program memory (faster if even address and length)
    char ReadProgram(unsigned long address, void *buffer, unsigned short length);

    // Write to program memory (must be a page-aligned address; if length is odd, one extra byte of junk will be written)
    char WriteProgramPage(unsigned long pageAddress, void *buffer, unsigned short length);

    // Read from program memory (all three bytes of each program 'word', even addresses only)
    char ReadProgramFull(unsigned long address, void *buffer, unsigned short length);

    // Write to program memory (all three bytes of each program 'word', must be a page-aligned address)
    char WriteProgramFullPage(unsigned long pageAddress, void *buffer, unsigned short length);

    // Performs a self test, returns a bitmap of failures (0x0000 = success)
    unsigned short SelfTest(void);
    
    #if 0
    	#warning "This is a no-battery build"
    	#undef USB_BUS_SENSE
    	#define USB_BUS_SENSE 0
    #endif

#endif
