// Hardware-specific code
// Karim Ladha, Dan Jackson, 2011
// 23-12-2011 : CWA3 base power down current checked - 55uA with all off (RTC running, SOSC on)

// Includes
#include <Compiler.h>
//#include "TimeDelay.h"
#include "HardwareProfile.h"
//#include "Peripherals/Accel.h"
//#include "Peripherals/Rtc.h"
#include "Analog.h"
//#include "Peripherals/myI2C.h"
//#include "GraphicsConfig.h"
//#include "Graphics/DisplayBasicFont.h"

// Configuration word
	#pragma config FSRSSEL = PRIORITY_7		// Which priority level owns the shadow set?
    #pragma config FUSBIDIO = OFF
    #pragma config FVBUSONIO = OFF

//    #pragma config FNOSC    = PRIPLL        // Oscillator Selection
    #pragma config FNOSC 	= FRC		// Oscillator Selection

    #pragma config UPLLEN   = ON        	// USB PLL Enabled
    #pragma config FPLLMUL  = MUL_15        // PLL Multiplier 4x20-> 80MHz, 4*15->60MHz
    #pragma config UPLLIDIV = DIV_5         // USB PLL Input Divider 20MHz -> 4MHz
    #pragma config FPLLIDIV = DIV_5         // PLL Input Divider 20MHz/5 -> 4MHz
    #pragma config FPLLODIV = DIV_1         // PLL Output Divider
    #pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
    #pragma config FWDTEN   = OFF           // Watchdog Timer
    #pragma config WDTPS    = PS4096        // Watchdog Timer Postscale
    #pragma config FCKSM    = CSECME		// Clock Switch Enable, FSCM Enabled  
    #pragma config OSCIOFNC = OFF           // CLKO Enable off
    #pragma config POSCMOD  = HS            // Primary Oscillator 20MHz xtal
    #pragma config IESO     = ON           	// Internal/External Switch-over
    #pragma config FSOSCEN  = ON            // Secondary Oscillator Enable (KL was off)

    #pragma config CP       = OFF           // Code Protect
    #pragma config BWP      = OFF           // Boot Flash Write Protect
    #pragma config PWP      = OFF           // Program Flash Write Protect
    #pragma config ICESEL   = ICS_PGx1      // ICE/ICD Comm Channel Select PGEC/D1


// Sleep mode - External peripherals all allowed to run unless specified
void SystemPwrSave(unsigned long NapSetting)
{
    return ;                       
}


// Precharge -- startup code to cope with battery pre-charge state 
void WaitForPrecharge(void)
{
    return ; 
}


// Read from program memory (faster if even address and length)
char ReadProgram(unsigned long address, void *buffer, unsigned short length)
{
    return 0; 
}


// Write to program memory (must be a page-aligned address; if length is odd, one extra byte of junk will be written)
char WriteProgramPage(unsigned long pageAddress, void *buffer, unsigned short length)
{
    return 0; 
}



unsigned int GetClockFreqUsingRTC (void)
{
    return 0; 
}

// Performs a self test, returns a bitmap of failures (0x0000 = success)
unsigned short SelfTest(void)
{
    return 0; 
}


#define Uart1TxStringPolled(...) printf(__VA_ARGS__)

 static enum {
      EXCEP_IRQ = 0,            // interrupt
      EXCEP_AdEL = 4,            // address error exception (load or ifetch)
      EXCEP_AdES,                // address error exception (store)
      EXCEP_IBE,                // bus error (ifetch)
      EXCEP_DBE,                // bus error (load/store)
      EXCEP_Sys,                // syscall
      EXCEP_Bp,                // breakpoint
      EXCEP_RI,                // reserved instruction
      EXCEP_CpU,                // coprocessor unusable
      EXCEP_Overflow,            // arithmetic overflow
      EXCEP_Trap,                // trap (possible divide by zero)
      EXCEP_IS1 = 16,            // implementation specfic 1
      EXCEP_CEU,                // CorExtend Unuseable
      EXCEP_C2E                // coprocessor 2
  } _excep_code;
  
  static unsigned int _epc_code;
  static unsigned int _excep_addr;
  
  void _general_exception_handler(void)
  {
      asm volatile("mfc0 %0,$13" : "=r" (_excep_code));
      asm volatile("mfc0 %0,$14" : "=r" (_excep_addr));
  
      _excep_code = (_excep_code & 0x0000007C) >> 2;
      
      Uart1TxStringPolled("\r\nGeneral Exception ");
      switch(_excep_code){
        case EXCEP_IRQ: Uart1TxStringPolled ("interrupt");break;
        case EXCEP_AdEL: Uart1TxStringPolled ("address error exception (load or ifetch)");break;
        case EXCEP_AdES: Uart1TxStringPolled ("address error exception (store)");break;
        case EXCEP_IBE: Uart1TxStringPolled ("bus error (ifetch)");break;
        case EXCEP_DBE: Uart1TxStringPolled ("bus error (load/store)");break;
        case EXCEP_Sys: Uart1TxStringPolled ("syscall");break;
        case EXCEP_Bp: Uart1TxStringPolled ("breakpoint");break;
        case EXCEP_RI: Uart1TxStringPolled ("reserved instruction");break;
        case EXCEP_CpU: Uart1TxStringPolled ("coprocessor unusable");break;
        case EXCEP_Overflow: Uart1TxStringPolled ("arithmetic overflow");break;
        case EXCEP_Trap: Uart1TxStringPolled ("trap (possible divide by zero)");break;
        case EXCEP_IS1: Uart1TxStringPolled ("implementation specfic 1");break;
        case EXCEP_CEU: Uart1TxStringPolled ("CorExtend Unuseable");break;
        case EXCEP_C2E: Uart1TxStringPolled ("coprocessor 2");break;
      }
      //Uart1TxStringPolled(" at 0x");
      //Uart1TxUint32HexPolled(_excep_addr);
	  Uart1TxStringPolled("0x%02X",_excep_addr);
      Uart1TxStringPolled("\r\n");
      while (1) {
		DelayMs(100);
		LED_SET(LED_GREEN);
		DelayMs(100);
		LED_SET(LED_RED);
          // Examine _excep_code to identify the type of exception
          // Examine _excep_addr to find the address that caused the exception
      }
  }

//EOF
