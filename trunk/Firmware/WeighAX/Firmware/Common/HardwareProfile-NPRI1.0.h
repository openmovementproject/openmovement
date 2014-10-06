/* 
 * Copyright (c) 2012-2013, Newcastle University, UK.
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
 
#ifndef _HARDWAREPROFILE_
#define _HARDWAREPROFILE_

// SW defines
#define HARDWARE_VERSION "1.0"
#define SOFTWARE_VERSION "1.2"

// These addresses must match the reserved sections of program memory in the linker script

#define GSM_PHONE_NUMBER_ADDRESS 0x29400ul

#define DEVICE_ID_ADDRESS 0x2A000ul
//ROM BYTE __attribute__ ((address(DEVICE_ID_ADDRESS),space(prog),aligned(ERASE_BLOCK_SIZE),section(".device_id"),noload)) DeviceIdData[ERASE_BLOCK_SIZE];

#define LOG_ADDRESS  0x29C00ul
//ROM BYTE __attribute__ ((address(LOG_ADDRESS),space(prog),aligned(ERASE_BLOCK_SIZE),section(".log" ),noload)) LogData[ERASE_BLOCK_SIZE];

#define SETTINGS_ADDRESS  0x29800ul
//ROM BYTE __attribute__ ((address(SETTINGS_ADDRESS),space(prog),aligned(ERASE_BLOCK_SIZE),section(".settings" ),noload)) SettingsData[ERASE_BLOCK_SIZE];


#include "Compiler.h"

	// Interrupt priority levels

	// Highest - RTC, TIMER tasks
	#define RTC_INT_PRIORITY				6 
	#define T1_INT_PRIORITY					RTC_INT_PRIORITY 	/*Prevents ms rollover during reads*/

	// Mid level - FIFO time stamps only, safe for rtc to interrupt
	#define FIFO_INTERRUPT_PRIORITY 		5
	#define CN_INTS_PROCESSOR_PRIORITY 		FIFO_INTERRUPT_PRIORITY
	#define ACCEL_INT_PRIORITY				FIFO_INTERRUPT_PRIORITY
	#define GYRO_INT_PRIORITY				FIFO_INTERRUPT_PRIORITY

	// Low level - Grabbing data from devices, I2C comms, ADC sample wait etc
	#define GLOBAL_I2C_PROCESSOR_PRIORITY	4
	#define DATA_STREAM_INTERRUPT_PRIORITY 	GLOBAL_I2C_PROCESSOR_PRIORITY

	// Unused priority levels
	#define ADC_INT_PRIORITY				3  /*This is very fast, can be used to halt ADC sequence*/

	// SW WDT Timeout in seconds - comment to cancel the swwdt
	//#define RTC_SWWDT_TIMEOUT 600

	// Enabled logger channels
	//#define LOGGING_LED_DEBUG

	#define MODE_OFF	0
	#define MODE_USE	1
	#define MODE_LOG	2

	#define MODE_ACCEL	MODE_LOG
	#define MODE_ADC	MODE_LOG
	//#define ACCEL_8BIT_MODE
	#define ACCEL_RATE			ACCEL_RATE_100
	#define ACCEL_RANGE			ACCEL_RANGE_8G
	#define ACCEL_DEFAULT_RATE  (ACCEL_RATE | ACCEL_RANGE)

// USB CDC driver
#define IN_BUFFER_CAPACITY 128	
#define OUT_BUFFER_CAPACITY 256		

// Used to restric the write flash function
#define MIN_ALLOWED_WRITE_ADD 0x1400		// Protects bootloader
#define MAX_ALLOWED_WRITE_ADD 0x2ABFE		// Protects configuation
	
//SystemPwrSave arguments OR together
#define WAKE_ON_USB				0x1
#define WAKE_ON_ACCEL1			0x2	
#define WAKE_ON_ACCEL2			0x4
#define WAKE_ON_BUTTON			0x20
#define WAKE_ON_WDT				0x40
#define WAKE_ON_RTC				0x80
#define WAKE_ON_TIMER1			0x100
#define LOWER_PWR_SLOWER_WAKE	0x200
#define ADC_POWER_DOWN			0x400
#define DISP_POWER_DOWN			0x800
#define ACCEL_POWER_DOWN		0x2000
#define SAVE_INT_STATUS			0x4000
#define ALLOW_VECTOR_ON_WAKE	0x8000
#define LEAVE_WDT_RUNNING		0x10000
#define JUST_POWER_DOWN_PERIPHERALS	0x20000
#define DONT_RESTORE_PERIPHERALS	0x40000

// Essential Functions in HardwareProfile.c
extern void WaitForPrecharge(void);
extern void SystemPwrSave(unsigned long NapSetting);

// LED Colours 0bRGB
enum { LED_OFF, LED_BLUE, LED_GREEN, LED_CYAN, LED_RED, LED_MAGENTA, LED_YELLOW, LED_WHITE };

	// Used for delays in TimeDelay.h - N.B. ACTUAL OSCILLATOR SPEED - Assumes we only clock at 8MHz or 80MHz
	#define GetInstructionClock()  	((OSCCONbits.COSC == 0b001)? 16000000UL : 4000000UL)
	// Used for SD card driver
	#define GetSystemClock()  		16000000UL /*Warning this should not be used other than in the SD card driver*/
	// Used in SD card code
	#define GetPeripheralClock()	(GetInstructionClock())

    // USB bus sense I/O
    // #define USE_USB_BUS_SENSE_IO
   	#define USB_BUS_SENSE       PORTFbits.RF1
	#define USB_BUS_SENSE_PIN	TRISFbits.TRISF1
	#define USB_BUS_SENSE_INIT() {USB_BUS_SENSE_PIN=1;CNPD5bits.CN69PDE = 1;} // KL: Added pull down to account for diode leakage
	#define USB_INIT_BUS_SENSE_INTS()	{}
						
    // MCHPFSUSB frameworkc self-power
    #define self_power          0   // 0 = bus-powered, 1 = self-powered

    // ROM storage
    #define WRITE_BLOCK_SIZE 128		// Write in rows of 64 instructions (only 2/3 bytes available = 128 bytes)
    #define ERASE_BLOCK_SIZE 1024		// Erase in blocks of 8 rows, 512 instructions (only 2/3 bytes available = 1024 bytes)

	// The pins
	#define InitIO()        {\
							REMAP_PINS();\
                            ANALOGUE_INIT_PINS();\
							LED_INIT_PINS();\
							DISPLAY_INIT_PINS();\
							BATT_INIT_PINS();\
                            ACCEL_INIT_PINS();\
							BUTTON_INIT_PINS();\
							SCALES_INIT_PINS();\
							GSM_INIT_OFF();\
                            DRIVE_FLOATING_PINS(); /*Reduce leakage*/\
                            USB_BUS_SENSE_INIT();\
                            }
	
	#define InitIOBootloader()	{\
								TRISB = 0b1110000000100000;\
								LATB =  0b0000001000000000;\
								TRISC = 0b1110111111111111;\
								LATC =  0b0000000000000000;\
								TRISD = 0b1111111100000001;\
								LATD =  0b0000000000000010;\
								TRISE = 0;LATE = 0;\
								TRISF = 0b1111111111110110;\
								LATF =  0b0000000000000000;\
								TRISG = 0b1111111011110011;\
								LATG =  0b0000000000000000;}

	#define DRIVE_FLOATING_PINS() {\
									TRISBbits.TRISB10 = 0; LATBbits.LATB10 = 0;\
									TRISBbits.TRISB11 = 0; LATBbits.LATB11 = 0;\
									TRISBbits.TRISB12 = 0; LATBbits.LATB12 = 0;\
									TRISCbits.TRISC12 = 0; LATCbits.LATC12 = 0;\
									TRISFbits.TRISF3 = 0; LATFbits.LATF3 = 0;}
							
	// REMAP pins
	#define REMAP_PINS()	{ /*NOTE!!!!! GSM pin names on datasheet are reffed to host (PIC)!!!!*/\
							__builtin_write_OSCCONL(OSCCON & 0xBF);/*Unlock*/\
							/*INPUTS*/\
							RPINR18bits.U1CTSR = 18;/*RP18 = U1CTS (GSM_CTS)*/\
							RPINR18bits.U1RXR = 27;	/*RP27 = U1RX (GSM_RX)*/\
							RPINR0bits.INT1R = 26;	/* AccInt1 RP26 -> Int1 */\
							RPINR1bits.INT2R = 21;	/* AccInt2 RP21 -> Int2 */\
							RPINR22bits.SCK2R = 7;	/* SPI2_SCK Flash*/\
							RPINR22bits.SDI2R = 8;	/* SPI2_SDI Flash*/\	
							/*OUTPUTS*/\
							RPOR9bits.RP19R = 3;		/*RP19 = U1TX (GSM_TX)*/\
							/*RPOR14bits.RP28R = 4;*/ 	/*RP22 = U1RTS (GSM_RTS)*/\
							RPOR3bits.RP7R = 11;	/*SPI2_SCK Flash*/\
							RPOR3bits.RP6R = 10;	/*SPI2_SDO Flash*/\
							/*The scales connections*/\
							RPINR19bits.U2RXR= 2; 	/* Scales RP2/D8 	GPIO1 -> U2RX */\
							RPINR7bits.IC1R = 2;	/* Map both IC1 to uart pin for SW uart*/\
							RPINR2bits.INT4R = 2; 	/* New Scales RP12/D8 GPIO2 -> INT4 */\
							/*PINR2bits.INT4R = 12; OLD HW Scales RP12/D11 GPIO2 -> INT4 */\
							/* Scales RP11/D0 GPIO3*/\
							/* Scales RP17/F5 GPIO4*/\
							/* Scales RP10/F4 GPIO5*/\
							/* Scales RP14/B14 GPIO6*/\
							/* Scales B13 GPIO7*/\
							__builtin_write_OSCCONL(OSCCON | 0x40);}			

    // Clock: initialize PLL for 96 MHz USB and 16MIPS
	#define CLOCK_PLL() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							OSCCONBITS_copy.NOSC = 0b001;			/*Internal FRC+PLL oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							while (OSCCONbits.OSWEN == 1);\
							while (OSCCONbits.LOCK != 1); 			/*Wait for PLL lock to engage*/\
							CLKDIVbits.RCDIV = 0b000; 				/*8 MHz */\
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

    #define LED_R_PIN           TRISDbits.TRISD7
    #define LED_R              	LATDbits.LATD7 
    #define LED_G_PIN  			TRISFbits.TRISF0        
    #define LED_G               LATFbits.LATF0
    #define LED_B_PIN           TRISDbits.TRISD6
    #define LED_B     			LATDbits.LATD6
    #define LED_INIT_PINS()     {LED_G_PIN=LED_G=LED_R_PIN=LED_R=LED_B_PIN=LED_B=0;}

    // LED set (0bRGB) - should compile to three bit set/clears with a static colour value
    #define LED_SET(_c) {\
                    if ((_c) & 0x4) { LED_R = 1; } else { LED_R = 0; } \
                    if ((_c) & 0x2) { LED_G = 1; } else { LED_G = 0; } \
                    if ((_c) & 0x1) { LED_B = 1; } else { LED_B = 0; } \
                    }

    // LED suspend/resume (for LDR)
    #define LED_SUSPEND()       { LED_R_PIN = 1; LED_G_PIN = 1; LED_B_PIN = 1; }
    #define LED_RESUME()        { LED_R_PIN = 0; LED_G_PIN = 0; LED_B_PIN = 0; }


    // Battery - AN7
	#define VREF_3v3
	#define BATT_6V_ALKALINE_3V3REF
    #define BATT_MON_PIN        TRISBbits.TRISB7
    #define BATT_INIT_PINS()   	BATT_MON_PIN = 1
    #define BATT_CHARGE_ZERO		BATT_FLOOR
	#define BATT_CHARGE_FULL		BATT_CEILING
	#define BATT_CHARGE_MIN_LOG		BATT_FLOOR		// minimum level to update log
	#define BATT_CHARGE_MIN_SAFE	BATT_FLOOR		// minimum safe running voltage (5%), avoids unwanted RTC resets and file corruptions etc.

	// Analogue select pins - 0 = Analogue
	#define ADC_SELECT_L			0b0111111111111111
	#define ADC_SELECT_H			0xffff
	#define ANALOGUE_INIT_PINS()	{AD1PCFGL = ADC_SELECT_L;AD1PCFGH = ADC_SELECT_H;}
    #define ADC_INDEX_BATT		0

	// Graphical DISPLAY - OLED on SSD1306,8080
    #define DISPLAY_CONTROLLER CUSTOM_CONTROLLER

    #define DISP_HOR_RESOLUTION 128
    #define DISP_VER_RESOLUTION 64
    #define COLOR_DEPTH 1
    #define DISP_ORIENTATION 0

	//#define DISPLAY_BITBANGED
	#define DISPLAY8080
	//#define DISPLAY_DEBUG // Prints checkerboard pattern when calling DISPLAYInit()

	#define DISPLAY_DATA_WR				LATE
	#define DISPLAY_DATA_RD				PORTE
	#define DISPLAY_DATA				PMDIN1
	#define DISPLAY_DATA_TRIS			TRISE
	#define DISPLAY_RE_PIN				TRISDbits.TRISD5
	#define DISPLAY_RE					LATDbits.LATD5
	#define DISPLAY_WR_PIN				TRISDbits.TRISD4
	#define DISPLAY_WR					LATDbits.LATD4
	
	#define DISPLAY_CHIP_SELECT_TRIS    TRISDbits.TRISD1
	#define DISPLAY_CHIP_SELECT         LATDbits.LATD1
	#define DISPLAY_RESET_TRIS          TRISDbits.TRISD2
	#define DISPLAY_RESET               LATDbits.LATD2
	#define DISPLAY_COMMAND_DATA_TRIS   TRISDbits.TRISD3
	#define DISPLAY_COMMAND_DATA        LATDbits.LATD3

	#define DISPLAY_INIT_PINS()		{\
								DISPLAY_DATA_TRIS 			= 0;\
								DISPLAY_DATA_WR 			= 0;\
								DISPLAY_RE_PIN 				= 0;\
								DISPLAY_RE 					= 1;\
								DISPLAY_WR_PIN 				= 0;\
								DISPLAY_WR 					= 1;\
								DISPLAY_CHIP_SELECT_TRIS 	= 0;\
								DISPLAY_CHIP_SELECT 		= 1;/*Disabled*/\
								DISPLAY_RESET_TRIS 			= 0;\
								DISPLAY_RESET 				= 1;\
								DISPLAY_COMMAND_DATA_TRIS 	= 0;\
								DISPLAY_COMMAND_DATA 		= 1;\
								}								

	// Scales
	#define MAX_WAIT_TIME_FROM_SCALES_ON_TO_DATA 30	// 30 Seconds, otherwise the comms are reset (will keep waiting if scales are on)

	// OLD #define SCALES_WAKE_PIN	PORTDbits.RD11
	#define SCALES_INIT_PINS()	{TRISDbits.TRISD8 = 1;}
	#define SCALES_WAKE	PORTDbits.RD8

	#define SCALES_DATA_INT	_U2RXInterrupt
	#define SCALES_DATA_IF	IFS1bits.U2RXIF
	#define SCALES_DATA_IE	IEC1bits.U2RXIE
	#define SCALES_DATA_IP	IPC7bits.U2RXIP
	
	#define SCALES_WAKE_INT	_INT4Interrupt
	#define SCALES_WAKE_IF	IFS3bits.INT4IF
	#define SCALES_WAKE_IE	IEC3bits.INT4IE
	#define SCALES_WAKE_IP	IPC13bits.INT4IP
	
	#define SCALES_BUFFER_SIZE	32
	#define SCALES_BUFFER_SIZE_MASK 0x1f
	#define SCALES_EXPECTED_PAYLOAD_LENGTH 5
	
	#define SCALES_BAUD	9600UL
	#define SCALES_BRG	(((GetInstructionClock()+(2UL*SCALES_BAUD))/(4UL*SCALES_BAUD))-1)


	// GSM module

	// VDMA setup 
	#define VDMA_DEFAULT_BAUD 	115200UL
 	#define VDMA_RTS			GSM_RTS
	#define VDMA_CALLBACK_ON_CHAR
	#define VDMA_RX_FIFO_SIZE 512
	#define VDMA_USES_UART_1
	#define VDMA_RX_INT_PRIORITY	7
	#define VDMA_TX_INT_PRIORITY	4

	#define GSM_PWR_PIN 	TRISBbits.TRISB2
	#define GSM_PWR			LATBbits.LATB2
	#define GSM_nRST_PIN	TRISBbits.TRISB3
	#define GSM_nRST		LATBbits.LATB3
	#define GSM_CTS_PIN		TRISBbits.TRISB5
	#define GSM_CTS			PORTBbits.RB5
	#define GSM_RTS_PIN 	TRISBbits.TRISB4
	#define GSM_RTS			LATBbits.LATB4
	#define GSM_TX_PIN 		TRISGbits.TRISG8
	#define GSM_TX			LATGbits.LATG8
	#define GSM_RX_PIN		TRISGbits.TRISG9
	#define GSM_RX			PORTGbits.RG9

	#define GSM_INIT_OFF()	{GSM_RX_PIN = 1;GSM_CTS_PIN = 1;\
							GSM_TX = 0; GSM_TX_PIN = 0;\
							GSM_RTS = 0; GSM_RTS_PIN = 0;\
							GSM_PWR = 0;	GSM_PWR_PIN = 0;\
							GSM_nRST_PIN = 0; GSM_nRST = 0;}
	#define GSM_INIT_ON()	{GSM_RX_PIN = 1;GSM_CTS_PIN = 1;\
							GSM_TX = 0; GSM_TX_PIN = 0;\
							GSM_RTS = 0; GSM_RTS_PIN = 0;\
							GSM_nRST = 1; GSM_nRST_PIN = 0; /*Hold in reset*/\
							GSM_PWR = 1; GSM_PWR_PIN = 0;	/*Power module*/\
							DelayMs(500);					/*Wait for minimum reset + pwr up time*/\
							GSM_nRST = 0; 					/*Relese from reset*/}							

	// Button - active low
	#define BUTTON1_PIN	TRISCbits.TRISC15
	#define BUTTON1		PORTCbits.RC15
	#define BUTTON_INIT_PINS()	{BUTTON1_PIN = 1;}
	extern short gButton;	
	#define BUTTON			BUTTON1

	// Flash Memory - SST SPI Flash - ref'd to host
	#define USE_4B_SPI_FLASH_OPCODES			/*Required for extended size flash memories*/
    #define FLASH_CE_PIN  	TRISBbits.TRISB9     
    #define FLASH_CE		LATBbits.LATB9
    #define FLASH_SCK_PIN 	TRISBbits.TRISB7 
    #define FLASH_SCK		LATBbits.LATB7
    #define FLASH_SDO_PIN  	TRISBbits.TRISB8 
    #define FLASH_SDO		LATBbits.LATB8            
    #define FLASH_SDI_PIN 	TRISBbits.TRISB6  
    #define FLASH_SDI		LATBbits.LATB6       
 
    #define FLASH_INIT_PINS()	{FLASH_CE_PIN = 0; FLASH_CE = 1;\
								FLASH_SCK_PIN = 0; FLASH_SCK = 0;\
								FLASH_SDO_PIN = 0; FLASH_SDO = 0;\
								FLASH_SDI_PIN = 1;}	

	// External flash
	 /*Needed for Mchips files*/
	#define SPIFLASH_CS_TRIS		FLASH_CE_PIN
	#define SPIFLASH_CS_IO			FLASH_CE
	#define SPIFLASH_SCK_TRIS		FLASH_SCK_PIN
	#define SPIFLASH_SDI_TRIS		FLASH_SDI_PIN 
	#define SPIFLASH_SDI_IO			FLASH_SDI
	#define SPIFLASH_SDO_TRIS		FLASH_SDO_PIN
	#define SPIFLASH_SPICON2		SPI1CON2

	#define SPIFLASH_SPI_IF			IFS0bits.SPI1IF
	#define SPIFLASH_SSPBUF			SPI1BUF
	#define SPIFLASH_SPICON1		SPI1CON1
	#define SPIFLASH_SPICON1bits	(SPI1CON1bits)
	#define SPIFLASH_SPISTAT		SPI1STAT
	#define SPIFLASH_SPISTATbits	SPI1STATbits	

	// Defines for myI2C.c
	#define USE_HW_I2C1
	#define MY_I2C_TIMEOUT 	65535
	//#define USE_MY_SW_I2C
	#ifdef USE_MY_SW_I2C
		#define InitI2C()			{myI2Cclear();mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}
	#endif
	/*for SW code use : {mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}*/
	#define mySCL				LATDbits.LATD10
	#define mySCLd				TRISDbits.TRISD10	
	#define mySDAr				PORTDbits.RD9
	#define mySDAw				LATDbits.LATD9
	#define mySDAd				TRISDbits.TRISD9
	// If a bus lockup occurs the following will clear it
	#define myI2Cclear()		{unsigned char timeout=100;mySCLd=0;mySDAd=1;while((--timeout)){mySCL=0;Delay10us(1);mySCL=1;Delay10us(1);}mySCLd=1;}
	

	// Accelerometer MMA8451Q
	#define ACCEL_MMA8451Q
	#define ACCEL_I2C_MODE
	#define ACCEL_FIFO_WATERMARK 25
    #define ACCEL_INT1_PIN      TRISGbits.TRISG7 
    #define ACCEL_INT2_PIN      TRISGbits.TRISG6 	
    #define ACCEL_INT1          PORTGbits.RG7
    #define ACCEL_INT2          PORTGbits.RG6 
   	#define ACCEL_INT1_IP       IPC5bits.INT1IP 
   	#define ACCEL_INT2_IP       IPC7bits.INT2IP 
    #define ACCEL_INT1_IF       IFS1bits.INT1IF 
    #define ACCEL_INT2_IF       IFS1bits.INT2IF 
    #define ACCEL_INT1_IE     	IEC1bits.INT1IE   
    #define ACCEL_INT2_IE       IEC1bits.INT2IE 

    #define ACCEL_INIT_PINS()  	{\
								ACCEL_INT1_PIN	= 1;\
								ACCEL_INT2_PIN	= 1;\
								}
								
// Legacy includes for MCHIP stacks
/* 
	After discussions between DGJ and KL, this was includes 
	to allow MCHPs example projects to work directly with our 
	hardware. The examples in Microchip Solutions use these
	defines to talk with thier demo boards. Clock switching etc
	should still be performed with the other macros in this file.
	Always call InitIO() before anything else when using MCHPs code.
*/ 
    #define self_power          0   // 0 = bus-powered, 1 = self-powered
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

