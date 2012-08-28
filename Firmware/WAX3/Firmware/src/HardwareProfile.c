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
 */

// HardwareProfile.c - Hardware-specific code


// Includes
//#include "ConfigApp.h"
#include "HardwareProfile.h"
//#include <stdio.h>
//#include <usart.h>
#include <delays.h>
#include <reset.h>
//#include <flash.h>
#include "accel.h"
//#include "USB/USB.h"
//#include "USB/usb_function_cdc.h"
#include "WirelessProtocols\MCHP_API.h"
//#include "WirelessProtocols\SymbolTime.h"
#include "data.h"
#if (DEVICE_TYPE==2)
#include "myi2c.h"
#include "gyro.h"
#endif


// Check large code model
#if !defined(__LARGE__)
	// Large codel model:  Project / Build Options / Project / MPLAB C18 / Memory Model / Large code model (> 64K bytes)
	#warning "Should compile with 'Large code model' so that TBLPTRU is preserved."
#endif

// Configuration (NOTE: This should match the configuration in the bootloader)
#if defined(WAX)                        // Configuration bits for PIC18F26J50 WAX Device
	#pragma config XINST    = OFF   	// Extended instruction set
    #pragma config STVREN   = ON      	// Stack overflow reset
    #pragma config WDTEN    = OFF     	// under SW control Watch Dog Timer (WDT)
    #pragma config CP0      = OFF      	// Code protect
    #pragma config IESO     = ON      	// Internal External (clock) Switchover
    #pragma config FCMEN    = OFF     	// Fail Safe Clock Monitor
    #pragma config PLLDIV   = 2	    	// Divide 8 MHz internal oscillator input by 2 to give required 4 MHz to PLL
    #pragma config OSC      = INTOSCPLL	// Use INTOSC ("Fosc2=0") as input to PLL and use PLL output divided by CPUDIV as primary clock source
// NOTE: RF timing delays are encoded at compile time...
#ifdef FAST_USB_CLOCK
    #pragma config CPUDIV   = OSC1		// Divide PLL output of 48 MHz by 1 to give 48 MHz for primary clock source
#else
    #pragma config CPUDIV   = OSC4_PLL6 // Divide PLL output of 48 MHz by 6 to give 8 MHz for primary clock source
#endif
    #pragma config WDTPS    = 1024    // WDT counter is (31250 Hz / 128 =) 0.004096 s multiplied by post-scaler = 4.2s
    //#pragma config T1DIG    = ON        // Sec Osc clock source may be selected
    //#pragma config LPT1OSC  = OFF       // high power Timer1 mode
    #pragma config DSWDTOSC = INTOSCREF // DSWDT uses INTOSC/INTRC as clock
    #pragma config RTCOSC   = T1OSCREF  // RTCC uses T1OSC/T1CKI as clock
    #pragma config DSBOREN  = OFF       // Zero-Power BOR disabled in Deep Sleep
    #pragma config DSWDTEN  = OFF       // Disabled
    #pragma config DSWDTPS  = 8192      // 1:8,192 (8.5 seconds)
    #pragma config IOL1WAY  = OFF       // IOLOCK bit can be set and cleared
    #pragma config MSSP7B_EN = MSK7     // 7 Bit address masking
    #pragma config WPFP     = PAGE_1    // Write Protect Program Flash Page 0
    #pragma config WPEND    = PAGE_0    // Start protection at page 0
    #pragma config WPCFG    = OFF       // Write/Erase last page protect Disabled
    #pragma config WPDIS    = OFF       // WPFP[5:0], WPEND, and WPCFG bits ignored  
#else
    #error No hardware board defined (see "HardwareProfile.h")
#endif


// Variables
#pragma udata

// Private prototypes
extern void UserInterruptHandler(void);		// Defined in main.c code
extern void HighISR(void);					// Defined in MRF24J40 tranceiver code
extern void LowISR(void);					// Defined in main.c code
extern void DisableIO(void);
extern void WaitForPrecharge(void);			// Wait for pre-charge to complete
extern void CriticalError(void);			// Critical error handler (flash LED, could add logging)



// Redirect locations
extern void _startup(void);        // from c018i.c 
#define HIGH_ISR_LOCATION	0x1008 
#define LOW_ISR_LOCATION	0x1018

// Patch HEX file so it performs the redirects even without the bootloader
#pragma code BootMirrorReset = 0x00
void BootRedirResetVect(void) {_asm goto _startup _endasm}
#pragma code BootMirrorHigh = 0x08
void BootRedirISRHigh(void) {_asm goto HIGH_ISR_LOCATION _endasm}
#pragma code BootMirrorLow = 0x18
void BootRedirISRLow(void) {_asm goto LOW_ISR_LOCATION _endasm}
#pragma code

#ifdef PRIORITY_INTERRUPT
extern void PRIORITY_INTERRUPT(void);
#endif

// The redirected interrupt vectors chain to the actual handlers
#pragma code REDIRECT_HIGH_INTERRUPT_VECTOR = HIGH_ISR_LOCATION 
void REDIRECT_HIGH_INTERRUPT_VECTOR(void)
{
#ifdef PRIORITY_INTERRUPT
	PRIORITY_INTERRUPT_CONDITION _asm goto PRIORITY_INTERRUPT _endasm
#endif
	_asm goto HighISR _endasm
}
#pragma code REDIRECT_LOW_INTERRUPT_VECTOR = LOW_ISR_LOCATION
void REDIRECT_LOW_INTERRUPT_VECTOR() { _asm goto LowISR _endasm }
#pragma code



/*
// Vector remapping for PIC18 devices with bootloader (0x00=reset, 0x08=high, 0x18=low).
#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
    #define REMAPPED_RESET_VECTOR_ADDRESS           0x1000
    #define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS  0x1008
    #define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS   0x1018
#elif defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
    #define REMAPPED_RESET_VECTOR_ADDRESS           0x800
    #define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS  0x808
    #define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS   0x818
#else
    #define REMAPPED_RESET_VECTOR_ADDRESS           0x00
    #define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS  0x08
    #define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS   0x18
#endif


// Remapped reset
#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER) || defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
    extern void _startup (void);        // See c018i.c in your C18 compiler dir
    #pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
    void _reset(void) { _asm goto _startup _endasm }
#endif

// Remapped ISRs from bootloader
#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
void Remapped_High_ISR(void) { _asm goto HighISR _endasm }
#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
void Remapped_Low_ISR(void) { _asm goto LowISR _endasm }

// Patch hex file so it will work even when written to a device without a bootloader.
#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER) || defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
#pragma code _redirect_reset = 0x00
void _redirect_reset(void) { _asm goto REMAPPED_RESET_VECTOR_ADDRESS _endasm }
    #pragma code HIGH_INTERRUPT_VECTOR = 0x08
    void High_ISR (void) { _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm }
    #pragma code LOW_INTERRUPT_VECTOR = 0x18
    void Low_ISR (void) { _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm }
#endif
*/



// Precharge -- startup code to cope with battery pre-charge state
void WaitForPrecharge(void)
{
    unsigned char timer;
    unsigned int successCount;
	unsigned char i;
	char suspectLow;

	// This needs to run VERY FAST - The MRF module trys to grab 25mA on a POR and must be stopped quickly
	CLOCK_INTOSC_1M(); 
	ClrWdt();

	// We have to init the pins
//	WDTCONbits.SWDTEN = 0;
#if defined(DEBUG_USART2_TX) || defined(UART2_DATA_OUTPUT)
		USART_REMAP_PINS();
	#endif
#if (DEVICE_TYPE==2)
		GYRO_REMAP_PINS();
#endif
#ifdef MOTOR_DEVICE
		MOTOR_REMAP_PINS();
#endif
	REMAP_PINS();

	// Disable all IO (sets pins to known state, disables MRF, disables ADXL, disables USB)
	DisableIO();

    // See if we suspect that the battery may be low (brown-out or low-voltage detect)
    suspectLow = (isBOR() || isLVD());
	StatusReset();	    // Reset the POR and BOR bits.

	// Now we can slow down the PIC a bit to lower its current draw
	CLOCK_INTRC();

	// Bandgap ref on
	ANCON1bits.VBGEN = 1; // Bandgap ref on
	Delay10TCYx(1);		// 125ms

    // Sample the battery
	timer = 0;
	successCount = 0;
    for (;;)
    {
        unsigned int value;

        ClrWdt();   // Reset WDT while we're in this loop

	    // Sample
        //ADCON0bits.GO = 1;
        ADCON0 = 0b00111111;    // Enabled and GO, reading from Vbg conversion ~1.2v
		ADCON1 = 0b10111011;	// 20Tad, RC clock
        while (ADCON0bits.GO);

        // Read A/D Result registers (ADRESH:ADRESL);
        value = ADRES; // ((unsigned int)ADRESH  << 8) | (unsigned int)ADRESL;
        
		// ref/Vdd*2^10 ==> 1.2 / 3 * 1024
		// 2.8 -> 439
		// 2.9 -> 424
		// 2.95 -> 417 - I tested this and 3v Vbat was actually observed which is slightly too high (420 is better)
		// 3.0 -> 410

        if (value < 420)
        {
            successCount++;
            if (successCount >= 10) 
            {
                break;   
            }
        }
        else
        {
            // If no bus power
            if (BATT_NCHARGING)
            {
				// TODO: Go to sleep (but wake on power interrupt)
				if (++timer == 0) LED = 1;
				else LED = 0;
            }
			else
			{
				if (++timer == 0) LED =1;
				else LED =0;
			}
            successCount = 0;
        }
    }

    LED = 0;
	ANCON1bits.VBGEN = 0;	// Bandgap ref off
    ADCON0bits.ADON = 0;    // Disabled ADC
    return;
}


// Critical error handler -- flash then reset
void CriticalError(void)
{
    unsigned char i;
    INTCONbits.GIE = 0;                 // Disable interrupts
    for (i = 0; i < 100; i++)
    {
       	LED = !LED;						// Strobe LED
		Delay10KTCYx(20);               // Wait
		ClrWdt();						
    }
    Reset();							// Something's gone very wrong, we'll just reset.
}


// DisableIO -- Disables I/O pins
void DisableIO(void)
{
    // Disable all interrupts, interrupt priority enabled
    INTCONbits.GIE = 0;     
    RCONbits.IPEN = 1;    // Interrupt priority (to be a known state)

    // Initially set all tristates as inputs
    TRISA = TRISB = TRISC = 0xff;

#ifdef MOTOR_DEVICE
	MOTOR_OFF();
#else
    // Kim says: drive the spare pin low anyway
    LATAbits.LATA5 = 0; TRISAbits.TRISA5 = 0;
#endif

    // Initially configure all ADC pins as digital
    ANCON0 = 0xff;          // AN0-AN7 as digital
    ANCON1 = 0x0f;          // AN8-AN? as digital, bandgap ref off

	// This code reduced the MRF chip current to ~3mA but it is still in its powered state, not in sleep yet
	// STOP ADXL and MRF chip
	ACCEL_INIT_PINS();	// ACCEL_CS = 1; ACCEL_CS_PIN = 0; // Active low
	RF_INIT_PINS();		// PHY_CS = 1; PHY_CS_TRIS = 0; PHY_RESETn = 0; PHY_RESETn_TRIS = 0;
	LED_INIT_PINS();    // LED_PIN = 0; LED = 0;
	BATT_INIT_PINS();	// BATT_NCHARGING_PIN  = 1;
	USB_INIT_PINS();	// Configure USB pins
#if (DEVICE_TYPE==2)
	GYRO_INIT_PINS();
#endif

	// Disable RF chip
 	MiApp_ProtocolInit(FALSE);
	while (MiApp_TransceiverPowerState(POWER_STATE_SLEEP) != SUCCESS);  // RF to sleep mode ~2uA

	// Disable accelerometer
	AccelStandby();

#if (DEVICE_TYPE==4)
	// Initialize pins for TEDDI expansion board (on certain hardware configurations)
	EXP_INIT_PINS();
#endif

#if (DEVICE_TYPE==2)
	// Disable gyroscope
	GyroStandby();
#endif

    // Disable any other pins
	//LATXbits.LATX0 = 0; TRISXbits.TRISX0 = 0;	// RX0 as output low (n/c)

    // Disable on-chip peripherals
    T0CONbits.TMR0ON = 0;   // Disable timer0
    T1CONbits.TMR1ON = 0;   // Disable timer1
    T2CONbits.TMR2ON = 0;   // Disable timer2
    T3CONbits.TMR3ON = 0;   // Disable timer3
    T4CONbits.TMR4ON = 0;   // Disable timer4
    CCP1CON = 0;			// Disable ECCP1
    CCP2CON = 0;			// Disable ECCP2
    ADCON0bits.ADON = 0;    // Disable ADC module
    UCONbits.SUSPND = 1;    // Suspend USB (USBEN cannot be set unless module clocked)
    UCONbits.USBEN = 0;    	// If it is clocked then power down
    CM1CONbits.CON = 0;     // Disable comparator 1
    CM2CONbits.CON = 0;     // Disable comparator 2

    // Clear all interrupt enable flags 
    INTCONbits.PEIE = 0;  // Disable (low priority) peripheral interrupts (GIE still off)
    INTCONbits.RBIE = 0;
    //INTCONbits.TMR0IE = 0;	// Leave timer-0 interrupt (RF overflow)
    //INTCONbits.INT0IE = 0;	// Leave interrupt 0 (RF)
    //INTCON3bits.INT1IE = 0;	// Leave interrupt 1 (Accel)
    //INTCON3bits.INT2IE = 0;	// Leave interrupt 2 (USB bus sense)
#if (DEVICE_TYPE==2)
    //INTCON3bits.INT3IE = 0;	// Leave interrupt 3 (Gyro)
#else
    INTCON3bits.INT3IE = 0;	// Disable interrupt 3 (unused)
#endif
    PIE1 = 0x00;
    PIE2 = 0x00;
    PIE3 = 0x00;
    UEIE = 0x00;
    UIE = 0x00;

    return;
}


// Shipping mode -- pulses while device remains connected, sleeps once device disconnected, returns to normal after re-connect.
void ShippingMode(void)
{
    unsigned short i;

    // Shutdown USB
    #if defined(USB_INTERRUPT)
        USBDeviceDetach();    // Must wait >100ms before re-attach
    #endif
    UCONbits.SUSPND = 0;
    UCON = 0;
    
    // Switch to internal oscillator, disable PLL
    CLOCK_INTOSC();

    // Shutdown peripherals, set IO pins to a known low-power state
    DisableIO();

    // Config.
    INTCONbits.GIE = 0;         // Interrupt vectoring off
    WDTCONbits.SWDTEN = 0;      // Ensure WDT is off to stop it waking/resetting the device
    ACCEL_INT1_IE = 0;          // Disable wake on ADXL
    INTCONbits.INT0IE = 0;      // Disable wake on RF
    USB_BUS_SENSE_IEDG = 1;     // Rising edge on USB detect
    USB_BUS_SENSE_IE = 1;       // Enable wake on USB
    USB_BUS_SENSE_TRIS = 1;     // Ensure we can check the bus sense pin
    OSCCONbits.IDLEN = 0;       // Real SLEEP not IDLE (peripheral clock off too)
    WDTCONbits.REGSLP = 1;      // Power off the VREG while asleep for ultra low power

    // While we're still connected, pulse the LED to show we're entering shipping mode
    for (;;)
    {
        // Fade down
        for (i = 0xffff; i != 0; i--) { LED = (unsigned char)i < (unsigned char)(i >> 8); }

        // If disconnected, end loop while the LED is off...
        if (!USB_BUS_SENSE) { break; }

        // Fade up
        for (i = 0; i != 0xffff; i++) { LED = (unsigned char)i < (unsigned char)(i >> 8); }

        // Delay while LED lit
        Delay10KTCYx(72);        // LongDelay(); Wait 360 msec:  0.360 * 8000000 / 4 => 720K Tcy
    }

    // Ensure LED is off while asleep
    LED = 0;

    // Sleep while USB is not connected...
    while (!USB_BUS_SENSE)
    {
        USB_BUS_SENSE_IF = 0;   // Clear bus sense interrupt
        Sleep();                // Should only continue after a USB bus sense interrupt
        Nop();
    }

    // USB was connected, fade up LED to show we're leaving shipping mode
    for (i = 0; i != 0xffff; i++) { LED = (unsigned char)i < (unsigned char)(i >> 8); }

    // Reset (will run bootloader or enter attached mode)
    Reset();
}
