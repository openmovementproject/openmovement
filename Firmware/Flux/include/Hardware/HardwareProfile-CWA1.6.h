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

// Hardware profile for the CWA 1.6 (using PIC24FJ256GB106)
// Karim Ladha, Dan Jackson, 2011-2012

#ifndef HARDWAREPROFILE_CWA16_H
#define HARDWAREPROFILE_CWA16_H

    #ifndef __PIC24FJ256GB106__
    //    #error "Unexpected device selection"
    #endif

    #ifndef __C30__
    //    #error "Unexpected compiler"
    #endif

    #define 	PIC24
    #define 	CWA2

    #define HARDWARE_VERSION 0x16           // BCD format (for USB response)

//#define NOP() __builtin_nop()

// System tick - presently the sample tick
//#define T1_ISR_RATE 100 /*for 100 Hz*/
//#define MS_PER_TICK	10 /*so the ms in the file is correct*/

//SystemPwrSave arguments OR together
#define WAKE_ON_USB		1
#define WAKE_ON_ADXL1	2	
#define WAKE_ON_ADXL2	4
#define WAKE_ON_GYRO1	8	
#define WAKE_ON_GYRO2	16
#define WAKE_ON_BUTTON	32
#define WAKE_ON_WDT		64
#define WAKE_ON_RTC		128
#define WAKE_ON_TIMER1	256
#define LOWER_PWR_SLOWER_WAKE	512
#define ADC_POWER_DOWN			1024
#define LCD_POWER_DOWN			2048
#define GYRO_POWER_DOWN			4096
#define ACCEL_POWER_DOWN		8192
#define SAVE_INT_STATUS			16384
#define ALLOW_VECTOR_ON_WAKE	32768
#define LEAVE_WDT_RUNNING		65536
#define JUST_POWER_DOWN_PERIPHERALS	131072
#define DONT_RESTORE_PERIPHERALS	262144

	// Used for delays in TimeDelay.h
	#define GetInstructionClock()  4000000

    // USB bus sense I/O
    // #define USE_USB_BUS_SENSE_IO
    #define USB_BUS_SENSE_PIN   TRISCbits.TRISC12
    #define USB_BUS_SENSE       PORTCbits.RC12
	#define USB_BUS_SENSE_INIT() USB_BUS_SENSE_PIN=1
	// Change notification interrupt
	#define USB_BUS_SENSE_INTS()	{\
							CNEN2bits.CN23IE = 1;\
							IFS1bits.CNIF = 0;/*Flag*/\
							IEC1bits.CNIE = 1;/*Unmask*/\
							IPC4bits.CNIP = 4;/*Priority*/\
							}
    // MCHPFSUSB frameworkc self-power
    #define self_power          0   // 0 = bus-powered, 1 = self-powered


    // ROM storage
    #define WRITE_BLOCK_SIZE 128		// Write in rows of 64 instructions (only 2/3 bytes available = 128 bytes)
    #define ERASE_BLOCK_SIZE 1024		// Erase in blocks of 8 rows, 512 instructions (only 2/3 bytes available = 1024 bytes)


	// The pins
	#define InitIO()        {\
                            REMAP_PINS();\
							ADC_INIT_PINS();\
							BATT_INIT_PINS();\
                            FLASH_INIT_PINS();\
                            GYRO_INIT_PINS();\
                            ACCEL_INIT_PINS();\
                            LED_INIT_PINS();\
                            TEMP_INIT_PINS();\
                            LDR_INIT_PINS();\
                            EXTRA_IO_INIT_PINS();\
                            DRIVE_FLOATING_PINS(); /*Reduce leakage*/\
                            USB_BUS_SENSE_INIT();\
                            }

	#define DRIVE_FLOATING_PINS() {\
                            TRISDbits.TRISD3 = 0; LATDbits.LATD3 = 0;\
                            TRISDbits.TRISD11 = 0; LATDbits.LATD11 = 0;\
                            TRISDbits.TRISD10 = 0; LATDbits.LATD10 = 0;\
                            TRISDbits.TRISD1 = 0; LATDbits.LATD1 = 0;\
                            TRISDbits.TRISD1 = 0; LATDbits.LATD1 = 0;\
                            TRISDbits.TRISD2 = 0; LATDbits.LATD2 = 0;\
                            TRISDbits.TRISD6 = 0; LATDbits.LATD6 = 0;\
                            TRISDbits.TRISD7 = 0; LATDbits.LATD7 = 0;\
                            TRISGbits.TRISG9 = 0; LATGbits.LATG9 = 0;\
                            }
							
	// REMAP pins
	#define REMAP_PINS()	{\
							__builtin_write_OSCCONL(OSCCON & 0xBF);/*Unlock*/\
							/* NB, Bug on schematic, int1 and int2 on adxl not labeled correctly*/\
							/* therefore HW profile does not reflect schematic exactly*/\
							RPINR0bits.INT1R = 9;/* AccInt1 RP9 -> Int1 */\
							RPINR1bits.INT2R = 8;/* AccInt2 RP8 -> Int2 */\
							RPINR1bits.INT3R = 18;/* GyrInt1 RP18 -> Int3 */\
							RPINR2bits.INT4R = 28;/* GyrInt2 RP28 -> Int4 */\
							RPINR20bits.SCK1R= 13;/* AccGyrSCK RP13 -> SCK1i */\
							RPINR20bits.SDI1R= 7;/* AccGyrSDO RP7 -> SDI1 */\
							RPOR6bits.RP13R = 8;  /* AccGyrSCK RP13 -> SCK1o */\
							RPOR3bits.RP6R = 7;  /* AccGyrSDI RP6 -> SDO1 */\
							/*Then lock the PPS module*/\
							__builtin_write_OSCCONL(OSCCON | 0x40);\
							}						

    // Clock: initialize PLL for 96 MHz USB and 12MIPS
	#define CLOCK_PLL() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							CLKDIVbits.RCDIV = 0b000; 				/*8 MHz */\
							OSCCONBITS_copy.NOSC = 0b001;			/*Internal FRC+PLL oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							while (OSCCONbits.OSWEN == 1);\
							while (OSCCONbits.LOCK != 1); 			/*Wait for PLL lock to engage*/\
							}				

    // Clock: switch to 8 MHz INTOSC
	#define CLOCK_INTOSC() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							OSCCONBITS_copy.NOSC = 0b111;			/*Internal FRC oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							while (OSCCONbits.OSWEN == 1);\
							CLKDIVbits.RCDIV = 0b000; 				/*8 MHz */\
							}

    // Clock: switch to 31 kHz INTRC
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
    #define LED_G_PIN  			TRISFbits.TRISF5         
    #define LED_G               LATFbits.LATF5
    #define LED_R_PIN           TRISFbits.TRISF4
    #define LED_R              	LATFbits.LATF4 
    #define LED_B_PIN           TRISFbits.TRISF3
    #define LED_B     			LATFbits.LATF3
    #define LED_INIT_PINS()     {TRISF&=0b1111111111000111;LATF&=0b1111111111000111;}

	// LED Colours 0bRGB (not to be confused with the LED output registers LED_R, LED_G, LED_B)
	enum { LED_OFF, LED_BLUE, LED_GREEN, LED_CYAN, LED_RED, LED_MAGENTA, LED_YELLOW, LED_WHITE };
	
    // LED set (0bRGB) - should compile to three bit set/clears with a static colour value
    #define LED_SET(_c) {\
                    if ((_c) & 0x4) { LED_R = 1; } else { LED_R = 0; } \
                    if ((_c) & 0x2) { LED_G = 1; } else { LED_G = 0; } \
                    if ((_c) & 0x1) { LED_B = 1; } else { LED_B = 0; } \
                    }

    // LED suspend/resume (for LDR)
    #define LED_SUSPEND()       { LED_R_PIN = 1; LED_G_PIN = 1; LED_B_PIN = 1; }
    #define LED_RESUME()        { LED_R_PIN = 0; LED_G_PIN = 0; LED_B_PIN = 0; }


    // Battery AN12
    #define BATT_MON_PIN        TRISBbits.TRISB12
    #define BATT_INIT_PINS()   	BATT_MON_PIN = 1
    #define BATT_CHARGE_ZERO		614
	#define BATT_CHARGE_MIN_LOG		517		// minimum level to update log
	#define BATT_CHARGE_MIN_SAFE	520		// minimum safe running voltage (5%), avoids unwanted RTC resets and file corruptions etc.
    #define BATT_CHARGE_MID_USB		650     // level to count towards recharge cycle counter
    #define BATT_CHARGE_FULL_USB	670
    #define BATT_CHARGE_FULL		708


    // LDR AN13
    #define LDR_EN_PIN			TRISBbits.TRISB11         
    #define LDR_EN              LATBbits.LATB11
    #define LDR_OUT_PIN         TRISBbits.TRISB13
    #define LDR_ENABLE()        {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 0;}
    #define LDR_DISABLE()       {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 1;}
    #define LDR_INIT_PINS()     LDR_DISABLE()

	// Temp sensor AN15
	#define USE_MCP9701
	#define TEMP_EN_PIN			TRISBbits.TRISB14
	#define TEMP_EN				LATBbits.LATB14
	#define TEMP_OUT_PIN		TRISBbits.TRISB15
	#define TEMP_ENABLE()		{TEMP_OUT_PIN=1;TEMP_EN_PIN=0;TEMP_EN=1;}
	#define TEMP_DISABLE()		{TEMP_OUT_PIN=1;TEMP_EN_PIN=0;TEMP_EN=0;}
	#define TEMP_INIT_PINS()	TEMP_DISABLE()

	// ADC select pins for CWA 1.6 (0 = Analog)
	#define ADC_SELECT_L		0b0100111111111111
	#define ADC_SELECT_H		0b1111111111111111
	#define ADC_INIT_PINS()		{AD1PCFGL = ADC_SELECT_L;AD1PCFGH = ADC_SELECT_H;}
    #define ADC_INDEX_BATT		0
    #define ADC_INDEX_LDR		1
    #define ADC_INDEX_TEMP		2
    

	// For the following interfaces the pins are referenced to the HOST (PIC)
	// i.e. ACCEL_SDO is the SDO pin of the MCU, it is an output. This differs from the schematic
	// All the pins need re-mapping. See RemapPins() in this file.

    // Accelerometer 
    #define ACCEL_CS_PIN		TRISBbits.TRISB10         
    #define ACCEL_CS            LATBbits.LATB10
    #define	ACCEL_SCK           LATBbits.LATB2   needs remapping to RP13	
    #define ACCEL_SCK_PIN       TRISBbits.TRISB2 
    #define ACCEL_SDO           LATBbits.LATB6   needs remapping to RP6
    #define ACCEL_SDO_PIN       TRISBbits.TRISB6   
    #define ACCEL_SDI    		PORTBbits.PORTB7 needs remapping to RP7       
    #define ACCEL_SDI_PIN       TRISBbits.TRISB7
    #define ACCEL_INT1_PIN      TRISBbits.TRISB8 
    #define ACCEL_INT2_PIN      TRISBbits.TRISB9 	
    #define ACCEL_INT1          PORTBbits.PORTB8 needs remapping to RP8
    #define ACCEL_INT2          PORTBbits.PORTB9 needs remapping to RP9  
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

	// Gyro L3G4200
	/*
	The gyro is on the same SPI bus as the accelerometer.
	The only difference is the SPI_CS line and interrupts.
	*/
    #define USE_GYRO
	#define GYRO_IS_L3G4200D
	#define GYRO_FIFO_WATERMARK 25
#ifdef USE_GYRO
	#define GYRO_INT_PRIORITY	4
    #define GYRO_CS_PIN 		TRISBbits.TRISB3
    #define GYRO_CS     		LATBbits.LATB3
    #define	GYRO_SCK           	LATBbits.LATB2   needs remapping to RP13
    #define GYRO_SCK_PIN       	TRISBbits.TRISB2
    #define GYRO_SDO           	LATBbits.LATB6   needs remapping to RP6
    #define GYRO_SDO_PIN       	TRISBbits.TRISB6
    #define GYRO_SDI  			PORTBbits.PORTB7 needs remapping to RP7
    #define GYRO_SDI_PIN       	TRISBbits.TRISB7
    #define GYRO_INT1_PIN     	TRISBbits.TRISB5
    #define GYRO_INT2_PIN     	TRISBbits.TRISB4
    #define GYRO_INT1      		PORTBbits.PORTB5 needs remapping to RP18
    #define GYRO_INT2   		PORTBbits.PORTB4 needs remapping to RP28
    #define GYRO_INT1_IP        IPC13bits.INT3IP
    #define GYRO_INT2_IP        IPC13bits.INT4IP
    #define GYRO_INT1_IF      	IFS3bits.INT3IF
    #define GYRO_INT2_IF      	IFS3bits.INT4IF
    #define GYRO_INT1_IE        IEC3bits.INT3IE
    #define GYRO_INT2_IE        IEC3bits.INT4IE

	#define GYRO_INIT_PINS() {\
								GYRO_CS_PIN	= 0;\
								GYRO_CS = 1;\
								GYRO_SCK_PIN	= 0;\
								GYRO_SDO_PIN	= 0;\
								GYRO_SDI_PIN	= 1;\
								GYRO_INT1_PIN	= 1;\
								GYRO_INT2_PIN	= 1;\
								}
#else
    #define GYRO_INIT_PINS()
#endif

    // Flash Memory - Hynix/Micron
	//#define FLASH_BITBANGED
    #define NAND_DEVICE     NAND_DEVICE_HY27UF084G2B

	#define FLASH_DATA_WR	LATE
	#define FLASH_DATA_RD	PORTE
	#define FLASH_DATA		PMDIN1
	#define FLASH_DATA_TRIS	TRISE
    #define FLASH_RE_PIN  	TRISDbits.TRISD5  
    #define FLASH_RE        LATDbits.LATD5 
	#define FLASH_WE_PIN 	TRISDbits.TRISD4    
    #define FLASH_WE		LATDbits.LATD4  
	
    #define FLASH_CE_PIN  	FLASH_CE1_PIN    
    #define FLASH_CE		FLASH_CE1
    #define FLASH_CE1_PIN  	TRISDbits.TRISD8  
    #define FLASH_CE1		LATDbits.LATD8
    #define FLASH_CE2_PIN  	NOT USED  
    #define FLASH_CE2		NOT USED                 
    #define FLASH_CLE_PIN 	TRISDbits.TRISD9  
    #define FLASH_CLE		LATDbits.LATD9       
    #define FLASH_ALE_PIN 	TRISDbits.TRISD0   
    #define FLASH_ALE		LATDbits.LATD0       
    #define FLASH_RB_PIN 	TRISCbits.TRISC15    
    #define FLASH_RB		PORTCbits.RC15 
    #define FLASH_WP_PIN 	NOT USED    
    #define FLASH_WP		NOT USED        

    #define FLASH_INIT_PINS()	{\
								FLASH_DATA_TRIS = 0x00;\
								FLASH_RE_PIN 	= 0;\
								FLASH_RE		= 1;\
								FLASH_WE_PIN 	= 0;\
								FLASH_WE		= 1;\
								FLASH_CE1_PIN	= 0;\
								FLASH_CE1		= 1;/*Disabled*/\
								/*FLASH_CE2_PIN	= 0;NOT USED*/\
								/*FLASH_CE2		= 1;NOT USED*/\
								FLASH_CLE_PIN	= 0;\
								FLASH_CLE		= 0;\
								FLASH_ALE_PIN 	= 0;\
								FLASH_ALE		= 0;\
								/*FLASH_WP_PIN 	= 0;NOT USED*/\
								/*FLASH_WP		= 1;NOT USED*/\
								FLASH_RB_PIN 	= 1;\
								}				 
   

					
    // Extra I/O pins (RP13 / RB2)
    #define EXP_PWR_PIN     TRISFbits.TRISF1
	#define EXP_PWR			LATFbits.LATF1
    #define EXTRA_IO1_PIN   TRISGbits.TRISG6
    #define EXTRA_IO1		LATGbits.LATG6
    #define EXTRA_IO2_PIN   TRISGbits.TRISG7
    #define EXTRA_IO2		LATGbits.LATG7
    #define EXTRA_IO3_PIN   TRISGbits.TRISG8
    #define EXTRA_IO3		LATGbits.LATG8
    #define EXTRA_IO_INIT_PINS()	{ EXP_PWR_PIN = 0; EXTRA_IO1_PIN = 0; EXTRA_IO2_PIN = 0; EXTRA_IO3_PIN = 0;}


// Legacy compatibility definitions, define NO_LEGACY to ensure using new values
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
