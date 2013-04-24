// KL 12-06-2011 
// .C file for hardware specific functions like sleep, hibernate etc

#include <Compiler.h>
#include "HardwareProfile.h"
//#include "accel.h"
//#include "Analogue.h"

#define USE_AND_OR /* To enable AND_OR mask setting */
#include<adc.h>
//#include "gyro.h"
//#include "myRTC.h"

/** CONFIGURATION **************************************************/
/*24 bit config 1 0b<unimp>0000<unimp>0000<res>0<jtag off>0<code prot off>1<prog writes on>1<debug off>1<res>1<EMUD1>11<wdtoff>0<stdrd wdt>1<unimp>0<wdt pres 128>0<wdt posc see table>****>*/ 
/* 
WDT bits ms (x4 for prescaler = 128)
1111 = 1:32,768
1110 = 1:16,384
1101 = 1:8,192
1100 = 1:4,096 
1011 = 1:2,048
1010 = 1:1,024
1001 = 1:512 
1000 = 1:256
0111 = 1:128
0110 = 1:64
0101 = 1:32
0100 = 1:16
0011 = 1:8
0010 = 1:4
0001 = 1:2
0000 = 1:1
*/
_CONFIG1(0b000000000011111101001110) 
/*24 bit config 2 0b<unimp>0000<unimp>0000<IESO off>0<plldiv2>001<plldis off>0<init osc = frc>000<clock switch en,FSCM on>00<no clk op>0<iolock on-off en>0<usb vreg off>1<res>1<prim osc off>11>*/
_CONFIG2(0b000000000001000000001111)

// Globals
extern volatile BOOL RTCC_UPDATE;

// Sleep mode - External peripherals all allowed to run unless specified
//void SystemPwrSave(unsigned long NapSetting)
//{
//	unsigned int IEC0_save,IEC1_save,IEC2_save,IEC3_save,IEC4_save,IEC5_save; // Interrupt context saving
//
//	asm("DISI #0x3FF"); // Stop interrupts
//
//	// Pin levels
//	//InitIO(); 		// Normal pin levels - all off - assume this has been called before
//  	
//	// External peripherals off as needed
//	LDR_DISABLE();
//	LED_INIT_PINS();
////	if (NapSetting & GYRO_POWER_DOWN)GyroStandby();
//	if (NapSetting & ADC_POWER_DOWN)InitADCOff();
//	if (NapSetting & ACCEL_POWER_DOWN)AccelStandby();
//
//	// Internal Peripherals off
//	if (!NapSetting & WAKE_ON_TIMER1) T1CONbits.TON = 0;	// Timer1 is the ms count 
//	/*Note: Add/remove power down settings as needed*/
//	T2CONbits.TON = 0;
//	T3CONbits.TON = 0;
//	T4CONbits.TON = 0;
//	T5CONbits.TON = 0;
//	//SPI1STATbits.SPIEN = 0;	
//	U1CONbits.USBEN = 0;
//	U1PWRCbits.USBPWR = 0; // MCHP Bug, not cleared in usb detach +120uA
//	//PMCONbits.PMPEN	= 0;
//
//	/* Note: The power down bits of some modules actually increases power draw - do not use these (commented) */
//	//PMD1 = 0xF8F9;
//	PMD2 = 0xFFFF;
//	//PMD3 = 0x058e;
//	PMD4bits.USB1MD = 1;//PMD4 = 0x006f;
//	PMD5 = 0x0101;
//	PMD6 = 0x0001;
//
//	// Reduce parrallel port leakage
//	TRISE = 0; LATE = 0;
//	// Reduce ICSP leakage
//	TRISBbits.TRISB0 = 0; LATBbits.LATB0 = 0;
//	TRISBbits.TRISB1 = 0; LATBbits.LATB1 = 0;
//
//	if (NapSetting & JUST_POWER_DOWN_PERIPHERALS) // Just shuts down everything internal + selected externals
//	{
//		// Restore powered off peripherals here as well
//		PMD4bits.USB1MD = 0;
//		SRbits.IPL = 0b000; // Effectively re-enable ints - will vector!
//		return;
//	}
//
//	// Disable all interrupts
//	if (NapSetting & SAVE_INT_STATUS)
//	{
//		IEC0_save = IEC0;
//		IEC1_save = IEC1;
//		IEC2_save = IEC2;
//		IEC3_save = IEC3;
//		IEC4_save = IEC4;
//		IEC5_save = IEC5;
//	}
//	IEC0 = IEC1 = IEC2 = IEC3 = IEC4 = IEC5 = 0; // Disable all ints
//
//	// Enable wakeup interrupts + peripherals
//	if (NapSetting==0){return;} // Stops it being bricked by accident
//	if (NapSetting & WAKE_ON_USB)	// Wake on USB detect
//	{
//		SetupUSBsenseInts(); // Shared with button inter on change notifications
//	}	 	
//	if (NapSetting & WAKE_ON_ADXL1)	// Wake on ADXL int1
//	{
//		ACCEL_INT1_IF = 0;
//		ACCEL_INT1_IE = 1;
//	}			
//	if (NapSetting & WAKE_ON_ADXL2)	// Wake on ADXL int2
//	{
//		ACCEL_INT2_IF = 0;
//		ACCEL_INT2_IE = 1;
//	}		 
////	if (NapSetting & WAKE_ON_GYRO1)	// Wake on GYRO int1
////	{
////		GY_INT1_IF = 0;
////		GY_INT1_IE = 1;
////	}			
////	if (NapSetting & WAKE_ON_GYRO2)	// Wake on GYRO int2
////	{
////		GY_INT2_IF = 0;
////		GY_INT2_IE = 1;
////	}		 	
//	if (NapSetting & WAKE_ON_RTC)	// Wake on RTC alarm
//	{
//		TurnOnRTCInt();
//	}		
//	if (NapSetting & WAKE_ON_TIMER1)	// Timer1 is the ms count
//	{
//		IFS0bits.T1IF = 0;
//		IEC0bits.T1IE = 0;
//	}
//	if (NapSetting & WAKE_ON_WDT) 	// Set period in config word 1 - currently 4096 * 1ms
//		{ClrWdt();RCONbits.SWDTEN = 1;}	// Wake on WDT
//
//	// Go to sleep... zzzzzzzzzzzzzz
//	__builtin_write_OSCCONL(OSCCON & 0xFB); 	// Primary oscillator off during sleep (sleep will pll will incure wakeup penalty)
//	REFOCON = 0;			// Reference oscillator off
//	if(NapSetting & LOWER_PWR_SLOWER_WAKE)	RCONbits.PMSLP = 0; // Adds 64ms wakeup time
//	else 									RCONbits.PMSLP = 1;
//	Sleep();
//	__builtin_nop();
//
//	// ON RETURN FROM SLEEP 
//	if ((NapSetting & WAKE_ON_WDT) &&(!(NapSetting & LEAVE_WDT_RUNNING)))
//		{ClrWdt();RCONbits.SWDTEN = 0;}	// Turn off WDT
//
//	// Restore the pins driven during sleep
//	TRISE = 0xFFFF;
//	TRISBbits.TRISB0 = 1; 
//	TRISBbits.TRISB1 = 1; 
//
//	// Resore powered down modules here
//	PMD4bits.USB1MD = 0;
//
//	// Restore peripherals
//	if (!(NapSetting & DONT_RESTORE_PERIPHERALS))
//	{
//		if (NapSetting & ADC_POWER_DOWN)InitADCOn();
////		if (NapSetting & GYRO_POWER_DOWN)GyroStartup();
//		if (NapSetting & ACCEL_POWER_DOWN)AccelStartup(ACCEL_DEFAULT_RATE);
//	}
//
//	// Restor Interrupts
//	if (!(NapSetting & ALLOW_VECTOR_ON_WAKE))
//	{
//		IFS0 = IFS1 = IFS2 = IFS3 = IFS4 = IFS5 = 0; // Clear all flags
//	}
//	if (NapSetting & SAVE_INT_STATUS)
//	{
//		IEC0 = IEC0_save;
//		IEC1 = IEC1_save; 
//		IEC2 = IEC2_save; 
//		IEC3 = IEC3_save; 
//		IEC4 = IEC4_save; 
//		IEC5 = IEC5_save;
//	}
//	asm("DISI #0");; // Effectively re-enable ints - will vector if flags are set!
//    return ;                       
//}
//

/*
To reduce the memory requirements I have moved the analogue functions here.
*/
// Global result
typedef union
{
    unsigned short values[3];  // [0]-batt, [1]-LDR, [2]-Temp
    struct
    {
        unsigned short batt, ldr, temp;
    };
} adc_results_t;

adc_results_t adcResult;

void GetBatt(void)
{
	PMD1bits.ADC1MD = 0;    // See device errata
	CloseADC10();
    // Configure ADC for RCOSC
    OpenADC10(	/*config1*/
                (ADC_MODULE_OFF | 
                ADC_IDLE_STOP | 
                ADC_FORMAT_INTG | 
                ADC_CLK_AUTO |
                ADC_AUTO_SAMPLING_OFF), 
                /*config2*/
                (ADC_VREF_AVDD_AVSS | 
                ADC_SCAN_ON | 
                ADC_INTR_3_CONV| 
                ADC_ALT_BUF_OFF |
                ADC_ALT_INPUT_OFF), 
                /*config3*/
                (ADC_CONV_CLK_INTERNAL_RC |     /* INTERNAL RC */
                ADC_SAMPLE_TIME_20 | 
                ADC_CONV_CLK_20Tcy),
                /*configport_L*/
                (ADC_SELECT_L),
                /*configport_H*/
                (ADC_SELECT_H),
                /*configscan_L*/
                (~ADC_SELECT_L),
                /*configscan_H*/
                (~ADC_SELECT_H)
            );
 	EnableADC1;

	IFS0bits.AD1IF = 0;		// Clear interrupt flag
	AD1CON1bits.ASAM = 1;	// Begin auto sampling
	while (!IFS0bits.AD1IF);// Wait for the allotted number of conversions set by ADC_INTR_?_CONV in config2
	AD1CON1bits.ASAM = 0;	// Stop auto sampling
	while(!AD1CON1bits.DONE);// Wait for partially complete samples
	
    adcResult.batt = ReadADC10(ADC_INDEX_BATT); // Battery
//    adcResult.ldr  = ReadADC10(ADC_INDEX_LDR);  // LDR
//    adcResult.temp = ReadADC10(ADC_INDEX_TEMP); // Temp
	AD1CON1bits.ADON = 0;
	PMD1bits.ADC1MD = 1; // See device errata
}




// Precharge -- startup code to cope with battery pre-charge state 
void WaitForPrecharge(void)
{
    unsigned int timer = 100;

	// Otherwise they could be drawing lots of power - n.b. no way to halt internal nand processes
	//SystemPwrSave(JUST_POWER_DOWN_PERIPHERALS|ACCEL_POWER_DOWN|GYRO_POWER_DOWN);

	// 32 kHz rc osc
	CLOCK_INTRC();

	// Sample the battery
    for (;;)
    {
		ClrWdt();   // Reset WDT while we're in this loop
		timer++;

		// if battery is flat - red 5% flash @ ~2Hz
		if(timer>100)
		{
			LED_R = 1;
		}
		if (timer>105)			
		{
			LED_R = 0;
			timer = 0;
			// Check battery voltage
			GetBatt();
			if (adcResult.batt > 520) // 520 is valid for all standard batt mon circuits (Vbatt/2 -> ADC pin, Vref = 3v)
			{
				// Not in precharge
				return;	
			}
			if (!USB_BUS_SENSE)
			{
				// USB removed - reset bootloader
				Reset();
			}
		}
    }

return;    
}

//void LEDColour (enum COLOUR colour)
//{
//if (colour & 0x1) LED_R = 1;	else LED_R = 0;
//if (colour & 0x2) LED_G = 1;	else LED_G = 0;
//if (colour & 0x4) LED_B = 1;	else LED_B = 0;
//return;
//}
//EOF
