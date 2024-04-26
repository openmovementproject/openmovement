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

// Hardware profile for the CWA 1.7 (using PIC24FJ256GB106)
// Karim Ladha, Dan Jackson, 2011-2012

#ifndef HARDWAREPROFILE_AX9v4_H
#define HARDWAREPROFILE_AX9v4_H

    #define 	PIC24
    #define 	CWA9_4

    // HARDWARE_TYPE -- original was:  ("AX3") - disk volume label ("AX317_#####"), product string descriptor (USB#2) "AX3 Composite Device 3.7  ", USB inquiry response vendor="AX3     " and product="AX3 Mass Storage",
    #define     HARDWARE_TYPE_0     'A'
    #define     HARDWARE_TYPE_1     'X'
    #define     HARDWARE_TYPE_2     '6' // '6'
    #define     HARDWARE_VERSION    0x64 //0x94           // BCD format (for USB response), original AX3 was 0x17

    // HARDWARE_LABEL -- original was:  ("CWA") - "ID" command response "ID=CWA,...", USB serial number string descriptor (USB#3) "CWA17_,,,,,"
    #define     HARDWARE_LABEL_0    HARDWARE_TYPE_0
    #define     HARDWARE_LABEL_1    HARDWARE_TYPE_1
    #define     HARDWARE_LABEL_2    HARDWARE_TYPE_2

//SystemPwrSave arguments OR together
#define WAKE_ON_USB				0x1
#define WAKE_ON_ACCEL1			0x2	
#define WAKE_ON_ACCEL2			0x4
#define WAKE_ON_GYRO1			0x8	
#define WAKE_ON_GYRO2			0x10
#define WAKE_ON_BUTTON			0x20
#define WAKE_ON_WDT				0x40
#define WAKE_ON_RTC				0x80
#define WAKE_ON_TIMER1			0x100
#define LOWER_PWR_SLOWER_WAKE	0x200
#define ADC_POWER_DOWN			0x400
#define LCD_POWER_DOWN			0x800
#define GYRO_POWER_DOWN			0x1000
#define ACCEL_POWER_DOWN		0x2000
#define SAVE_INT_STATUS			0x4000
#define ALLOW_VECTOR_ON_WAKE	0x8000
#define LEAVE_WDT_RUNNING		0x10000
#define JUST_POWER_DOWN_PERIPHERALS	0x20000
#define DONT_RESTORE_PERIPHERALS	0x40000

#define  ENABLE_printhexdump


// Essential Functions in HardwareProfile.c
extern void WaitForPrecharge(void);
extern void SystemPwrSave(unsigned long NapSetting);

	// Used for delays in TimeDelay.h
#if 1
// KL: The device has two internal oscillators
#define CLOCK_INTOSC CLOCK_INTOSC_DCO
#define GetInstructionClock()  12250000 // KL: Delay ms is not working on new compiler, adjusted here for accuracy
#else
#define CLOCK_INTOSC CLOCK_INTOSC_FRC
#define GetInstructionClock()  4000000
#endif

    // USB bus sense I/O
    //#define USE_USB_BUS_SENSE_IO
	#define USB_BUS_SENSE_PIN		TRISFbits.TRISF5
    #define USB_BUS_SENSE       	PORTFbits.RF5
	#define USB_BUS_SENSE_INIT()	USB_BUS_SENSE_PIN=1
	// Change notification int,  vectors to button handler
	#define USB_INIT_BUS_SENSE_INTS()   {} // KL: Not used {/*New USB int is remappable -> INT3*/\
									IFS3bits.INT3IF = 0;/*Flag*/\
									IEC3bits.INT3IE = 1;/*Unmask*/\
									}
	#define USB_BUS_SENSE_INTS()	USB_INIT_BUS_SENSE_INTS() /*This define is now legacy, it doen't make gramatical sense*/

    // MCHPFSUSB frameworkc self-power flag
    #define self_power          0   // 0 = bus-powered, 1 = self-powered

    // ROM storage
    #define WRITE_BLOCK_SIZE 256		// 2xAX3 HW, Write in rows of 64 instructions (only 2/3 bytes available = 128 bytes)
    #define ERASE_BLOCK_SIZE 2048		// 2xAX3 HW, Erase in blocks of 8 rows, 512 instructions (only 2/3 bytes available = 1024 bytes)

#define PER_PWR_PIN TRISFbits.TRISF4
#define PER_PWR_EN  LATFbits.LATF4
#define PER_ENABLE()    {PER_PWR_PIN = 0; PER_PWR_EN = 1; DelayMs(5);}
#define PER_DISABLE()   {\
                        LDR_EN          = 0;\
                        ACCEL_CS        = 0;\
                        ACCEL_SCK       = 0;\
                        ACCEL_SDO       = 0;\
                        FLASH_DATA_TRIS = 0xFF;\
						FLASH_RE		= 0;\
						FLASH_WE		= 0;\
						FLASH_CE1		= 0;\
						FLASH_CE2		= 0;\
						PER_PWR_PIN     = 0;\
                        PER_PWR_EN      = 0;}

	// The pins
	#define InitIO()        {\
                            PER_ENABLE();\
                            REMAP_PINS();\
							ADC_INIT_PINS();\
							BATT_INIT_PINS();\
                            FLASH_INIT_PINS();\
                            ACCEL_INIT_PINS();\
                            LED_INIT_PINS();\
                            TEMP_INIT_PINS();\
                            LDR_INIT_PINS();\
                            DRIVE_FLOATING_PINS(); /*Reduce leakage*/\
                            USB_BUS_SENSE_INIT();\
                            }

	#define DRIVE_FLOATING_PINS() {\
                            TRISGbits.TRISG6 = 0; LATGbits.LATG6 = 0;\
                            TRISGbits.TRISG7 = 0; LATGbits.LATG7 = 0;\
                            TRISGbits.TRISG8 = 0; LATGbits.LATG8 = 0;\
                            TRISGbits.TRISG9 = 0; LATGbits.LATG9 = 0;\
                            TRISFbits.TRISF0 = 0; LATFbits.LATF0 = 0;\
                            TRISFbits.TRISF3 = 0; LATFbits.LATF3 = 0;\
                            TRISCbits.TRISC12 = 0; LATCbits.LATC12 = 0;\
							TRISBbits.TRISB3 = 0; LATBbits.LATB3 = 0;\
							TRISBbits.TRISB4 = 0; LATBbits.LATB4 = 0;\
							TRISBbits.TRISB5 = 0; LATBbits.LATB5 = 0;\
							TRISBbits.TRISB11 = 0; LATBbits.LATB11 = 0;\
							TRISBbits.TRISB13 = 0; LATBbits.LATB13 = 0;\
							TRISBbits.TRISB14 = 0; LATBbits.LATB14 = 0;\
							TRISBbits.TRISB0 = 0; LATBbits.LATB0 = 0;/*PGC*/\
							TRISBbits.TRISB1 = 0; LATBbits.LATB1 = 0;/*PGD*/\
                            TRISGbits.TRISG2 = 0; LATGbits.LATG2 = 0;/*D+*/\
                            TRISGbits.TRISG3 = 0; LATGbits.LATG3 = 0;/*D-*/\
                            }
							
	// REMAP pins
	#define REMAP_PINS()	{\
							__builtin_write_OSCCONL(OSCCON & 0xBF);/*Unlock*/\
							RPINR0bits.INT1R = 11;/* AccInt1 RP11 -> Int1 */\
							RPINR1bits.INT2R = 12;/* AccInt2 RP12 -> Int2 */\
							RPINR1bits.INT3R = 17;/* USB det RP17 -> Int3 */\
							RPINR20bits.SCK1R= 3;/* AccSCK RP3 -> SCK1i */\
							RPINR20bits.SDI1R= 4;/* AccSDO RP4 -> SDI1 */\
							RPOR1bits.RP3R = 8;  /* AccGyrSCK RP3 -> SCK1o */\
							RPOR1bits.RP2R = 7;  /* AccGyrSDI RP2 -> SDO1 */\
							/*Then lock the PPS module*/\
							__builtin_write_OSCCONL(OSCCON | 0x40);\
							}						

    // Clock: initialize PLL for 96 MHz USB and 16MIPS
	#define CLOCK_PLL() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							CLKDIV = 0x3100;    					/*CPUDIV to 1:1 (which is a USB compatible setting*/\
							asm("DISI #0x3FFF");\
							CLKDIVbits.RCDIV = 0b000; 				/*8 MHz */\
							OSCCONBITS_copy.NOSC = 0b001;			/*Internal FRC+PLL oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							while (OSCCONbits.OSWEN == 1);\
							while (OSCCONbits.LOCK != 1); 			/*Wait for PLL lock to engage*/\
							OSCTUN = 0x9000;        				/*Enable active clock tuning from USB host reference*/\
							}				

    // Clock: switch to 8 MHz INTOSC
	#define CLOCK_INTOSC_FRC() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");                    /*  8 MHz */\
							OSCCONBITS_copy.NOSC = 0b000;			/*Internal FRC oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							while (OSCCONbits.OSWEN == 1);\
							}

	#define CLOCK_INTOSC_DCO() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
                            DCOCON = 0x0F00;                            /* 30 MHz */\
							OSCCONBITS_copy.NOSC = 0b110;			/*Internal DCO oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							while (OSCCONbits.OSWEN == 1);\
							}

    // Clock: switch to 31.25 kHz INTRC
    #define CLOCK_INTRC() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							OSCCONBITS_copy.NOSC = 0b101;			/*Internal RC oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							while (OSCCONbits.OSWEN == 1);\
							CLKDIVbits.RCDIV = 0b000; 				/*8 MHz */\
							}
	// SOSC / RTC, 32.768 kHz							
    #define CLOCK_32KHZ() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							OSCCONBITS_copy.NOSC = 0b100;			/*External secondary oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							while (OSCCONbits.OSWEN == 1);\
							CLKDIVbits.RCDIV = 0b000; 				/*8 MHz */\
							}
       
	// Secondary oscillator on
	#define CLOCK_SOSCEN()	{OSCCONbits.SOSCEN=1;}

    // LED
    #define LED_G_PIN  			TRISBbits.TRISB7        
    #define LED_G               LATBbits.LATB7
    #define LED_R_PIN           TRISBbits.TRISB6
    #define LED_R              	LATBbits.LATB6 
    #define LED_B_PIN           TRISBbits.TRISB2
    #define LED_B     			LATBbits.LATB2
    #define LED_INIT_PINS()     {TRISB&=0b1111111100111011;LATB|=0b0000000011000100;}

	// LED Colours 0bRGB (not to be confused with the LED output registers LED_R, LED_G, LED_B)
	enum { LED_OFF, LED_BLUE, LED_GREEN, LED_CYAN, LED_RED, LED_MAGENTA, LED_YELLOW, LED_WHITE };
#define LED_SET_OFF 1
#define LED_SET_ON  0
    
    // LED set (0bRGB) - should compile to three bit set/clears with a static colour value
    #define LED_SET(_c) {\
                    if ((_c) & 0x4) { LED_R = LED_SET_ON; } else { LED_R = LED_SET_OFF; } \
                    if ((_c) & 0x2) { LED_G = LED_SET_ON; } else { LED_G = LED_SET_OFF; } \
                    if ((_c) & 0x1) { LED_B = LED_SET_ON; } else { LED_B = LED_SET_OFF; } \
                    }

    // LED suspend/resume (for LDR)
    #define LED_SUSPEND()       { LED_R_PIN = 1; LED_G_PIN = 1; LED_B_PIN = 1; }
    #define LED_RESUME()        { LED_R_PIN = 0; LED_G_PIN = 0; LED_B_PIN = 0; }


    // Battery - AN15
    #define BATT_ANALOG_PIN		15
    #define BATT_MON_PIN        TRISBbits.TRISB15
    #define BATT_INIT_PINS()   	BATT_MON_PIN = 1
    #define BATT_CHARGE_ZERO		614
	#define BATT_CHARGE_MIN_LOG		517		// minimum level to update log
	#define BATT_CHARGE_MIN_SAFE	520		// minimum safe running voltage (5%), avoids unwanted RTC resets and file corruptions etc.
    #define BATT_CHARGE_MID_USB		650     // level to count towards recharge cycle counter
    #define BATT_CHARGE_FULL_USB	670
    #define BATT_CHARGE_FULL		708


    // LDR - AN9
    #define LDR_EN_PIN			TRISBbits.TRISB8         
    #define LDR_EN              LATBbits.LATB8
    #define LDR_OUT_PIN         TRISBbits.TRISB9
    #define LDR_ENABLE()        {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 0;}
    #define LDR_DISABLE()       {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 1;}
    #define LDR_INIT_PINS()     LDR_DISABLE()

	// Temp sensor - AN10
	#define USE_MCP9700
	#define TEMP_EN_PIN			TRISBbits.TRISB12
	#define TEMP_EN				LATBbits.LATB12
	#define TEMP_OUT_PIN		TRISBbits.TRISB10
	#define TEMP_ENABLE()		{TEMP_OUT_PIN=1;TEMP_EN_PIN=0;TEMP_EN=1;}
	#define TEMP_DISABLE()		{TEMP_OUT_PIN=1;TEMP_EN_PIN=0;TEMP_EN=0;}
	#define TEMP_INIT_PINS()	TEMP_DISABLE()

	// ADC select pins for CWA 1.7 (0 = Analog)
	#define ADC_SELECT_L 		0b0111100111111111
	//#define ADC_SELECT_H 		0b1111111111111111
    // Scan select value
    #define ADC_SCAN_SELECT_L   0b1000011000000000 /*AN15,10,9*/
	#define ADC_INIT_PINS()		{ANSB = 0b1000011000000000; ANSC = ANSD = ANSE = 0;} //AD1PCFGL = ADC_SELECT_L;AD1PCFGH = ADC_SELECT_H;}
    #define ADC_INDEX_BATT		2
    #define ADC_INDEX_LDR 		0
    #define ADC_INDEX_TEMP		1

	// For the following interfaces the pins are referenced to the HOST (PIC)
	// i.e. ACCEL_SDO is the SDO pin of the MCU, it is an output. This differs from the schematic
	// All the pins need re-mapping. See RemapPins() in this file.

    // Accelerometer 
    #define ACCEL_ADXL345 // Compatibility in accel.h
    #define USE_BMI160_IMU
    
    #define ACCEL_CS_PIN		TRISCbits.TRISC15         
    #define ACCEL_CS            LATCbits.LATC15
    #define	ACCEL_SCK           LATDbits.LATD10 /*needs remapping*/   	
    #define ACCEL_SCK_PIN       TRISDbits.TRISD10 
    #define ACCEL_SDO           LATDbits.LATD8  /*needs remapping*/
    #define ACCEL_SDO_PIN       TRISDbits.TRISD8   
    #define ACCEL_SDI    		needs remapping       
    #define ACCEL_SDI_PIN       TRISDbits.TRISD9
    #define ACCEL_INT1_PIN      TRISDbits.TRISD0 
    #define ACCEL_INT2_PIN      TRISDbits.TRISD11 	
    #define ACCEL_INT1          needs remapping
    #define ACCEL_INT2          needs remapping 
    #define ACCEL_INT1_IF       IFS1bits.INT1IF 
    #define ACCEL_INT2_IF       IFS1bits.INT2IF 
    #define ACCEL_INT1_IE     	IEC1bits.INT1IE   
    #define ACCEL_INT2_IE       IEC1bits.INT2IE 

    #define ACCEL_INIT_PINS()  	{\
								ACCEL_CS_PIN	= 0;\
								ACCEL_CS = 1;\
								ACCEL_SCK_PIN	= 0;\
								ACCEL_SDO_PIN	= 0;\
								ACCEL_SDI_PIN	= 1;\
								ACCEL_INT1_PIN	= 1;\
								ACCEL_INT2_PIN	= 1;\
								}
								
									
    // Flash Memory - Hynix/Micron
#define FLASH_BITBANGED
    #define NAND_DEVICE     		NAND_DEVICE_MX30LF4G18AC
    #define NAND_DEVICE_ALT         NAND_DEVICE_S34ML08G1
    #define NAND_DEVICE_ALT2		NAND_DEVICE_S34ML04G1
	#define NAND_BYTES_PER_PAGE 	2112 /*2048 + 64 extra*/

	#define FLASH_DATA_WR	LATE
	#define FLASH_DATA_RD	PORTE
	#define FLASH_DATA		PMDIN1
	#define FLASH_DATA_TRIS	TRISE
    #define FLASH_RE_PIN  	TRISDbits.TRISD5  
    #define FLASH_RE        LATDbits.LATD5 
	#define FLASH_WE_PIN 	TRISDbits.TRISD4    
    #define FLASH_WE		LATDbits.LATD4  
	
    #define FLASH_CE1_PIN  	TRISDbits.TRISD2  
    #define FLASH_CE1		LATDbits.LATD2
    #define FLASH_CE2_PIN  	TRISDbits.TRISD3  
    #define FLASH_CE2		LATDbits.LATD3                
    #define FLASH_CLE_PIN 	TRISDbits.TRISD6  
    #define FLASH_CLE		LATDbits.LATD6       
    #define FLASH_ALE_PIN 	TRISDbits.TRISD7   
    #define FLASH_ALE		LATDbits.LATD7      
    #define FLASH_RB1_PIN 	TRISDbits.TRISD1  
    #define FLASH_RB2_PIN 	TRISFbits.TRISF1     
    #define FLASH_RB1		PORTDbits.RD1 
    #define FLASH_RB2		PORTFbits.RF1
    #define FLASH_WP_PIN 	NOT USED    
    #define FLASH_WP		NOT USED        
 
    // Compatibility - if multi nand is off
#ifndef NAND_MULTI_PLANEWISE
    #define FLASH_RB		FLASH_RB1
    #define FLASH_RB_PIN	FLASH_RB1_PIN
    #define FLASH_CE		FLASH_CE1
    #define FLASH_CE_PIN  	FLASH_CE1_PIN   
#endif

    #define FLASH_INIT_PINS()	{\
								FLASH_DATA_TRIS = 0xFF;\
								FLASH_RE_PIN 	= 0;\
								FLASH_RE		= 1;\
								FLASH_WE_PIN 	= 0;\
								FLASH_WE		= 1;\
								FLASH_CE1_PIN	= 0;\
								FLASH_CE1		= 1;/*Disabled*/\
								FLASH_CE2_PIN	= 0;\
								FLASH_CE2		= 1;/*Disabled*/\
								FLASH_CLE_PIN	= 0;\
								FLASH_CLE		= 0;\
								FLASH_ALE_PIN 	= 0;\
								FLASH_ALE		= 0;\
								/*FLASH_WP_PIN 	= 0;NOT USED*/\
								/*FLASH_WP		= 1;NOT USED*/\
								FLASH_RB1_PIN 	= 1;\
                                FLASH_RB2_PIN 	= 1;\
								}				 
					
	
// Legacy includes for MCHIP stacks, define NO_LEGACY to ensure using current values
#ifndef NO_LEGACY

	// Pre-standardized names compatibility
	#define MINIMUM_SAFE_BATTERY_RUNNING_VOLTAGE BATT_CHARGE_MIN_SAFE
	#define MINIMUM_UPDATE_LOG_VOLTAGE	BATT_CHARGE_MIN_LOG
    #define batt_zero_charge			BATT_CHARGE_ZERO
    #define batt_full_charge			BATT_CHARGE_FULL
    #define batt_full_charge_with_USB	BATT_CHARGE_FULL_USB
    #define batt_mid_charge_with_USB	BATT_CHARGE_MID_USB
	#define ANALOGUE_SELECT_L 			ADC_SELECT_L
	#define ANALOGUE_SELECT_H 			ADC_SELECT_H
	#define ANALOGUE_INIT_PINS 			ADC_INIT_PINS

	// Microchip Solutions example project compatibility
	#define usb_bus_sense 		USB_BUS_SENSE
	
	#define CLOCK_FREQ 			32000000

	#define mInitAllLEDs()      LED_INIT_PINS()
    
	#define LED					LED_R
    #define mLED_1              LED_R
    #define mLED_2              LED_G
    #define mLED_3              LED_B
    #define mLED_4              LED_R

    #define mGetLED_1()         mLED_1
    #define mGetLED_2()         mLED_2
    #define mGetLED_3()         mLED_3
    #define mGetLED_4()         mLED_4     
    
    #define mLED_1_On()         mLED_1 = 1;
    #define mLED_2_On()         mLED_2 = 1;
    #define mLED_3_On()         mLED_3 = 1;
    #define mLED_4_On()         mLED_4 = 1;
    
    #define mLED_1_Off()        mLED_1 = 0;
    #define mLED_2_Off()        mLED_2 = 0;
    #define mLED_3_Off()        mLED_3 = 0;
    #define mLED_4_Off()        mLED_4 = 0;
    
    #define mLED_1_Toggle()     mLED_1 = !mLED_1;
    #define mLED_2_Toggle()     mLED_2 = !mLED_2;
    #define mLED_3_Toggle()     mLED_3 = !mLED_3;
    #define mLED_4_Toggle()     mLED_4 = !mLED_4;
    
    /** SWITCH *********************************************************/
    #define mInitSwitch2()      
    #define mInitSwitch3()      
    #define mInitAllSwitches()  

    /** I/O pin definitions ********************************************/
    #define INPUT_PIN 1
    #define OUTPUT_PIN 0
#endif



#endif

