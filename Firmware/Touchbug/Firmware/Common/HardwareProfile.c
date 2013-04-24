// Hardware-specific code
// Karim Ladha, Dan Jackson, 2011
// 23-12-2011 : CWA3 base power down current checked - 55uA with all off (RTC running, SOSC on)

// Includes
#include <Compiler.h>
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Rtc.h"
#include "Common/Analog.h"
#include "Peripherals/myI2C.h"


// Configuration word
// 24 bit config 1 0b<unimp>0000<unimp>0000<res>0<jtag off>0<code prot off>1<prog writes on>1<debug off>1<res>1<EMUD1>11<wdtoff>0<stdrd wdt>1<unimp>0<wdt pres 32>1<wdt posc see table>****>*/
//     WDT bits -- 1111 = 1:32,768; 1110 = 1:16,384; 1101 = 1:8,192; 1100 = 1:4,096; 1011 = 1:2,048; 1010 = 1:1,024; 1001 = 1:512; 1000 = 1:256; 0111 = 1:128; 0110 = 1:64; 0101 = 1:32; 0100 = 1:16; 0011 = 1:8; 0010 = 1:4; 0001 = 1:2; 0000 = 1:1;
_CONFIG1(0b000000000011111101001010) //WDT 1024
// 24 bit config 2 0b<unimp>0000<unimp>0000<IESO off>0<plldiv2>001<plldis off>0<init osc = frc>000<clock switch en>00<no clk op>0<iolock on-off en>0<usb vreg off>1<res>1<prim osc off>11>*/
_CONFIG2(0b000000000001000000001111)
//    _CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & FWDTEN_OFF & ICS_PGx2)
//    _CONFIG2( 0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_ON & POSCMOD_HS & FNOSC_FRCPLL & PLLDIV_DIV2 & IOL1WAY_ON)


// Sleep mode - External peripherals all allowed to run unless specified
void SystemPwrSave(unsigned long NapSetting)
{
	unsigned int IPLshadow, IEC0_save,IEC1_save,IEC2_save,IEC3_save,IEC4_save,IEC5_save; // Interrupt context saving

	if (NapSetting==0){return;} // Stops it being bricked by accident   // [dgj] Moved here

	IPLshadow = SRbits.IPL;
	SRbits.IPL = 0b111;	// Stop interrupts

	// Pin levels
	//InitIO(); 		// Normal pin levels - all off - assume this has been called before
  	
	// External peripherals off as needed
	LDR_DISABLE();
	LED_INIT_PINS();
	if (NapSetting & GYRO_POWER_DOWN)GyroStandby();
	if (NapSetting & ADC_POWER_DOWN)AdcOff();
	if (NapSetting & ACCEL_POWER_DOWN)AccelStandby();
	if (NapSetting & BT_POWER_DOWN){BT_INIT_PINS();STOP_BT_SLW_CLK();}
	if (!NapSetting & WAKE_ON_TIMER1) T1CONbits.TON = 0;	// Timer1 is the ms count 

	/*Note: Add/remove power down settings as needed*/
	T2CONbits.TON = 0;T3CONbits.TON = 0;T4CONbits.TON = 0;T5CONbits.TON = 0;
	U1CONbits.USBEN = 0;
	U1PWRCbits.USBPWR = 0; // MCHP Bug, not cleared in usb detach +120uA
	//PMCONbits.PMPEN	= 0;

	/* Note: The power down bits of some modules actually increases power draw - do not use these (commented) */
	//PMD1 = 0xF8F9;
	PMD2 = 0xFFFE; // OC1 left on in sleep
	//PMD3 = 0x058e;
	PMD4bits.USB1MD = 1;//PMD4 = 0x006f;
	PMD5 = 0x0101;
	PMD6 = 0x0001;

	// Reduce parrallel port leakage
	TRISE = 0; LATE = 0;
	// Reduce ICSP leakage
	TRISBbits.TRISB0 = 0; LATBbits.LATB0 = 0;
	TRISBbits.TRISB1 = 0; LATBbits.LATB1 = 0;

	if (NapSetting & JUST_POWER_DOWN_PERIPHERALS) // Just shuts down everything internal + selected externals
	{
		// Restore powered off peripherals here as well
		PMD2 = PMD4 = PMD5 = PMD6 = 0;
		SRbits.IPL = IPLshadow; // Effectively re-enable ints - will vector!
		return;
	}

	// Disable all interrupts
	if (NapSetting & SAVE_INT_STATUS)
	{
		IEC0_save = IEC0;
		IEC1_save = IEC1;
		IEC2_save = IEC2;
		IEC3_save = IEC3;
		IEC4_save = IEC4;
		IEC5_save = IEC5;
	}
	else { IEC0_save = 0; IEC1_save = 0; IEC2_save = 0; IEC3_save = 0; IEC4_save = 0; IEC5_save = 0; }	// This is not needed and is only here to remove the warning in MPLAB C compiler's default settings
	
	IEC0 = IEC1 = IEC2 = IEC3 = IEC4 = IEC5 = 0; // Disable all ints

	// Enable wakeup interrupts + peripherals
	if (NapSetting & WAKE_ON_USB)USB_INIT_BUS_SENSE_INTS();	// Wake on USB detect
	if (NapSetting & WAKE_ON_BUTTON)BUTTON_ENABLE_INTS();// Wake on Button 
	if (NapSetting & WAKE_ON_ACCEL1)ACCEL_INT1_IE = 1;		// Wake on accel int1
	if (NapSetting & WAKE_ON_ACCEL2)ACCEL_INT2_IE = 1;		// Wake on accel int2
	if (NapSetting & WAKE_ON_GYRO1);//GYRO_INT1_IE = 1;		// Wake on GYRO int1
	if (NapSetting & WAKE_ON_GYRO2)GYRO_INT2_IE = 1;		// Wake on GYRO int2
	if (NapSetting & WAKE_ON_RTC)IEC3bits.RTCIE = 1; 		// Wake on RTC alarm
	if (NapSetting & WAKE_ON_TIMER1)IEC0bits.T1IE = 1;		// Timer1 is the ms count
	if (NapSetting & WAKE_ON_WDT){ClrWdt();RCONbits.SWDTEN = 1;}	 	// Set period in config word 1 

	// Go to sleep... zzzzzzzzzzzzzz
	__builtin_write_OSCCONL(OSCCON & 0xFB); 	// Primary oscillator off during sleep (sleep with pll will incure wakeup penalty)
	REFOCON = 0;								// Reference oscillator off
	if(NapSetting & LOWER_PWR_SLOWER_WAKE)	RCONbits.PMSLP = 0;
	else 									RCONbits.PMSLP = 1; 
	Sleep();
	__builtin_nop();

	// ON RETURN FROM SLEEP 
	if ((NapSetting & WAKE_ON_WDT) &&(!(NapSetting & LEAVE_WDT_RUNNING)))
		{ClrWdt();RCONbits.SWDTEN = 0;}	// Turn off WDT

	// Restore the pins driven during sleep
	TRISE = 0xFFFF;
	TRISBbits.TRISB0 = 1; 
	TRISBbits.TRISB1 = 1; 
	PMD2 = PMD4 = PMD5 = PMD6 = 0;

	// Restore peripherals
//	if (!(NapSetting & DONT_RESTORE_PERIPHERALS))
//	{
//		if (NapSetting & ADC_POWER_DOWN)AdcInit();
//	}

	// Restore Interrupts
	if (!(NapSetting & ALLOW_VECTOR_ON_WAKE))
	{
		IFS0 = IFS1 = IFS2 = IFS3 = IFS4 = IFS5 = 0; // Clear all flags
	}
	if (NapSetting & SAVE_INT_STATUS)
	{
		IEC0 = IEC0_save;
		IEC1 = IEC1_save; 
		IEC2 = IEC2_save; 
		IEC3 = IEC3_save; 
		IEC4 = IEC4_save; 
		IEC5 = IEC5_save;
	}
	SRbits.IPL = IPLshadow; // Effectively re-enable ints - will vector if flags are set!
    return ;                       
}


// Precharge -- startup code to cope with battery pre-charge state 
void WaitForPrecharge(void)
{
    unsigned int timer = 100;
	
	// Otherwise they could be drawing lots of power - n.b. no way to halt internal nand processes
	SystemPwrSave(JUST_POWER_DOWN_PERIPHERALS|ACCEL_POWER_DOWN|GYRO_POWER_DOWN|BT_POWER_DOWN);

	// 32 kHz rc osc
	CLOCK_INTRC();
		
	AdcInit();
	AdcSampleNow(); // Get one sample
	AdcOff();

	// if battery is flat and no charger/ USB present
	if ((adcResult.batt < BATT_CHARGE_MIN_SAFE)&&(!USB_BUS_SENSE))
	{
        // Stall here until charger/USB is connected
    	while (!USB_BUS_SENSE)
        {
            SystemPwrSave(WAKE_ON_WDT|WAKE_ON_RTC|LOWER_PWR_SLOWER_WAKE|WAKE_ON_USB|ADC_POWER_DOWN|ACCEL_POWER_DOWN|GYRO_POWER_DOWN);
            LED_SET(LED_RED);
            Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
            LED_SET(LED_OFF);
        }
        // Reset once connected
		Reset();
	}

	// Sample the battery
    for (;;)
    {
		ClrWdt();   // Reset WDT while we're in this loop
		timer++;

		// if battery is flat - red 5% flash @ ~2Hz
		if(timer > 100)
		{
			LED_SET(LED_RED);
		}
		if (timer > 105)
		{
			LED_SET(LED_OFF);
			// Check battery voltage
			AdcInit();
			AdcSampleNow(); 
			AdcOff();
			if (adcResult.batt > BATT_CHARGE_MIN_SAFE)
			{
				// Not in precharge
				LED_SET(LED_OFF);
				CLOCK_INTOSC();
				return;	
			}
			timer = 0;
		}
    }

    return;
}


// Read from program memory (faster if even address and length)
char ReadProgram(unsigned long address, void *buffer, unsigned short length)
{
    unsigned int oldTBLPAG;
    unsigned short i;

    // Save the TBLPAG register
    oldTBLPAG = TBLPAG;

    // If either address or length are odd
    if ((address & 1)  || (length & 1))
    {
        // (Slower) read the data from the program memory one byte at a time
        for(i = 0; i < length; i++)
        {
            unsigned short temp;
            TBLPAG = (unsigned char)(address >> 16);            // Top 8 bits of address
            temp = __builtin_tblrdl(((WORD)address) & 0xfffe);  // Read the bottom two bytes of the instruction (don't actually need the mask for word reads)
            if (address & 1)
                *(unsigned char *)buffer = (unsigned char)(temp >> 8);  // Odd address / high byte
            else
                *(unsigned char *)buffer = (unsigned char)temp;         // Even address / low byte
            buffer++;
            address++;
        }
    }
    else
    {
        // (Faster) read the data from the program memory one word at a time
        for(i = 0; i < length; i += 2)
        {
            TBLPAG = (unsigned char)(address >> 16);                        // Top 8 bits of address
            *(unsigned short *)buffer = __builtin_tblrdl((WORD)address);    // Read the bottom two bytes of the instruction
            buffer += 2;
            address += 2;
        }
    }

    // Restore the TBLPAG register
    TBLPAG = oldTBLPAG;

    return 1;
}


// Write to program memory (must be a page-aligned address; if length is odd, one extra byte of junk will be written)
char WriteProgramPage(unsigned long pageAddress, void *buffer, unsigned short length)
{
    unsigned int oldTBLPAG;
    unsigned short i, j;

    // Must start at an erase page boundary
    if (pageAddress & (unsigned long)0x03ff) { return 1; }
	if ((pageAddress < MIN_ALLOWED_WRITE_ADD) || (pageAddress >= MAX_ALLOWED_WRITE_ADD)){ return 1; }
    pageAddress = pageAddress & (unsigned long)0xfffffc00;  // Zero non-page elements of the address

    // Save the TBLPAG register
    oldTBLPAG = TBLPAG;

    // Erase the page
    NVMCON = 0x4042;                                // Page erase on next write
    TBLPAG = (unsigned char)(pageAddress >> 16);        // Top 8 bits of address
    __builtin_tblwtl((unsigned short)pageAddress, 0xffff); // A dummy write to load address of erase page
    asm("DISI #16");                				// Disable interrupts for unlock sequence
    __builtin_write_NVM();

    // Reprogram the erased page (writes only to the PSV-addressable bytes, using 2 bytes of each 3-byte word)
    NVMCON = 0x4001;                                // Internal flash program page
    j = 0;
    for (i = 0; i < length; i += 2)
    {
        TBLPAG = (unsigned char)(pageAddress >> 16);
        __builtin_tblwtl((unsigned short)pageAddress, *((unsigned short *)(buffer + i))); // Bottom two bytes
        pageAddress++;
        __builtin_tblwth((unsigned short)pageAddress, 0xff);                              // Set high byte (0x00 or 0xff = NOP instruction, 0x3f = invalid instruction)
        pageAddress++;

        j += 2;

        // Program block size boundary -- program the last 128 (of 192) bytes
        if (j >= WRITE_BLOCK_SIZE)
        {
            j -= WRITE_BLOCK_SIZE;
            asm("DISI #16");                				// Disable interrupts for unlock sequence
            __builtin_write_NVM();
        }
    }

    // If partial row written, pad with zeros then write
    if (j > 0)
    {
        while (j < WRITE_BLOCK_SIZE)
        {
            TBLPAG = (BYTE)(pageAddress >> 16);
            __builtin_tblwtl((unsigned short)pageAddress, 0xffff);
            pageAddress++;
            __builtin_tblwth((unsigned short)pageAddress, 0xff);
            pageAddress++;
            j += 2;
        }
        asm("DISI #16");                				// Disable interrupts for unlock sequence
        __builtin_write_NVM();
    }

    // Restore the TBLPAG register
    TBLPAG = oldTBLPAG;
    
    return 0;
}


unsigned int GetClockFreqUsingRTC (void)
{
	/*This function turns on timer1 to run off the secondary crystal oscillator*/
	/*It uses this time base to gate the system clock on timer2 for 33 counts*/
	/*At 4Mhz this gives 4000000*33/32768 = 4028*/
	TMR1=0;				// Clear timer
	T1CONbits.TSYNC= 1;	// Sync - for read/write alignment issue
	T1CONbits.TCS = 1;	// Secondary osc timer input
	T1CONbits.TCKPS = 0;// No prescale
	T1CONbits.TGATE = 0;// Ignored bit (Not Gated)
	T1CONbits.TSIDL = 1;// Timer on in idle mode
	PR1 = 32;			// ~1ms timebase
	IFS0bits.T1IF=0;	// Clear flag
	T1CONbits.TON = 1;	// Timer on

	T2CON = 0; 			// Timer OFF, timer on intosc 4MHz (or 16 on PLL)
	T2CONbits.TON = 0;	// Timer off
	TMR2 = 0; 			// clear timer

	while(!IFS0bits.T1IF); // Wait for first roll over - now we are synchronised
	T2CONbits.TON = 1;	// Timer2 on counting up
	IFS0bits.T1IF=0;	// Clear the flag
	while(!IFS0bits.T1IF); // Wait 1ms again - this time counting
	T2CONbits.TON = 0;	// Timer2 off
	T1CONbits.TON = 0;	// Timer1 off
	return TMR2;
}
/*
unsigned int GetClockFreqUsingRTC (void)
{
	TMR1=0;				// Clear timer
	T1CONbits.TSYNC= 1;	// Sync - for read/write alignment issue
	T1CONbits.TCS = 1;	// Secondary osc timer input
	T1CONbits.TCKPS = 0;// No prescale
	T1CONbits.TGATE = 0;// Ignored bit (Not Gated)
	T1CONbits.TSIDL = 1;// Timer on in idle mode
	PR1 = 32;			// ~1ms timebase
	IFS0bits.T1IF=0;	// Clear flag
	T1CONbits.TON = 1;	// Timer on

	T2CON = 0; 			// Timer OFF, timer on intosc 4MHz (or 16 on PLL)
	T2CONbits.TON = 0;	// Timer off
	TMR2 = 0; 			// clear timer

	T2CONbits.TCS = 1;	// Secondary osc timer input (32khz)
	T2CONbits.TCKPS = 0;// No prescale
	IEC0bits.T2IE = 1;

	while(!IFS0bits.T1IF); // Wait for first roll over - now we are synchronised
	T2CONbits.TON = 1;	// Timer2 on counting up
	IFS0bits.T1IF=0;	// Clear the flag
	while(!IFS0bits.T1IF); // Wait 1ms again - this time counting
	T2CONbits.TON = 1;	// Timer2 off
	T1CONbits.TON = 1;	// Timer1 off
	return TMR2;
}*/

#if 0

// Performs a self test, returns a bitmap of failures (0x0000 = success)
unsigned short SelfTest(void)
{
    unsigned short result = 0x0000;

    // Read accelerometer device ID (should be ACCEL_DEVICE_ID = 0xE5)
    //if (AccelReadDeviceId() != ACCEL_DEVICE_ID) { result |= 0x0001; }

    // Read NAND parameters and compare with compiled-in constants
    //if (FtlVerifyNandParameters()) { result |= 0x0002; }

    // FTL

    // FSIO

    // LDR

    // Temperature

    // Battery

    return result;
}

void Pass(void)
{
	LED_SET(LED_GREEN);	
	DelayMs(50);
	LED_SET(LED_OFF);	
	DelayMs(200);
}

void Fail(void)
{
	LED_SET(LED_RED);	
	DelayMs(50);
	LED_SET(LED_OFF);	
	DelayMs(200);
}

// Test function for debugging
void test_system_debug (void)
{
  // Initialization
	InitIO();			// I/O pins
	InitI2C();
	CLOCK_SOSCEN(); 	// For the RTC
	//WaitForPrecharge();	// ~0.5mA current
	
	// Peripherals - RTC and ADC always used
	CLOCK_INTOSC();     // 8 MHz
//	LED_SET(LED_CYAN);
    RtcStartup();
	AdcInit();
    AdcSampleWait();                    // Ensure we have a valid battery level

	// Check LED
	LED_SET(LED_RED);
	DelayMs(200);
	LED_SET(LED_GREEN);
	DelayMs(200);
	LED_SET(LED_BLUE);
	DelayMs(200);
	LED_SET(LED_OFF);
	DelayMs(200);

	// Check clocks
	{
		static unsigned int period;
		period = GetClockFreqUsingRTC();
		if ((period > 4000)&&(period < 4056)) Pass(); 
		else Fail();
	}

	// Analogue periferals
	AdcInit();
	AdcSampleWait();
	// LDR
	if ((adcResult.ldr > 300)&&(adcResult.ldr < 1000)) Pass(); // Typical values
	else {Fail();}
	// Batt
	if ((adcResult.batt > 500)&&(adcResult.batt < 751)) Pass(); // <3v = 510, 3v = 512 , 4.4v = 751
	else Fail();
	// Temp
	if ((adcResult.temp > 200)&&(adcResult.temp < 400)) Pass(); // 21 degrees is 242
	else Fail();

	// I2C Peripherals
    // Accelerometer
    if(AccelVerifyDeviceId())Pass(); // responded OK
	else Fail();
    // Gyro
    if(GyroVerifyDeviceId())Pass(); // responded OK
	else Fail();


// Implement this in your code after this function to check the USB and BT module
    // Read settings
    LED_SET(LED_GREEN);
    restart = 0;
    inactive = 0;
    SettingsInitialize();           // Initialize settings from ROM
    FtlStartup();                   // FTL & NAND startup
    FSInit();                       // Initialize the filesystem for reading
    SettingsReadFile(SETTINGS_FILE);    // Read settings from script
    // TODO: Make this "single binary file" mode optional on the settings just read
    LoggerReadMetadata(DEFAULT_FILE);   // Read settings from the logging binary file

    // Run as attached or logging
	if (USB_BUS_SENSE)
	{
		RunAttached();      // Run attached, returns when detatched - copy this init script to the msd
    }

	// Check bluetooth
    FtlStartup();   // FTL & NAND startup
    FSInit();       // Initialize the filesystem for reading

    CLOCK_PLL();	// HS PLL clock for the USB module 12MIPS
    DelayMs(1); 	// Allow PLL to stabilise
	BTInit();
	while(bt_state_local < BT_INITIALISED){BTtasks();}
	Pass();
	while(1){BTtasks();}
	// Does not get past here

}

#endif

