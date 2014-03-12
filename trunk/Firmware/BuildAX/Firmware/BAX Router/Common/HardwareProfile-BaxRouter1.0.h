/* 
 * Copyright (c) 2013-2014, Newcastle University, UK.
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

// Hardware-specific code
// Karim Ladha, 2013-2014

#ifndef _HARDWAREPROFILE_
#define _HARDWAREPROFILE_

	// Hardware version for this file
	#define HARDWARE_VERSION 10

	// Interrupt priority levels

	// Highest - RTC, TIMER tasks
	#define RTC_INT_PRIORITY				6 
	#define T1_INT_PRIORITY					RTC_INT_PRIORITY 	/*Prevents ms rollover during reads*/

	// Mid level - data element functions
	#define DATA_ELEMENT_PRIORITY			5
	#define Si44_INT_PRIORITY				DATA_ELEMENT_PRIORITY
	#define MRF_INT_PRIORITY				DATA_ELEMENT_PRIORITY
	// Low level - 
	#define TCIPIP_TICK_PRIORITY			4



	// SW WDT Timeout in seconds - comment to cancel the swwdt
	#define RTC_SWWDT_TIMEOUT 600

	// Used to restrict the write flash function
    #define WRITE_BLOCK_SIZE 256			// Ignores phantom bytes	
    #define ERASE_BLOCK_SIZE 2048			// Ignores phantom bytes 
	#define MIN_ALLOWED_WRITE_ADD 0x0		// Vector table can be written - do not use vectors (interrupts) in bootloader
	#define MAX_ALLOWED_WRITE_ADD 0x2ABFE	// Protects configuation
	
	//SystemPwrSave arguments OR together
	#define WAKE_ON_WDT				0x1
	#define WAKE_ON_RTC				0x2
	#define WAKE_ON_TIMER1			0x4
	#define LOWER_PWR_SLOWER_WAKE	0x8
	#define LEAVE_WDT_RUNNING		0x10
	
	// Essential Functions in HardwareProfile.c
	extern void SystemPwrSave(unsigned long NapSetting);

	// Used for delays in TimeDelay.h - N.B. ACTUAL OSCILLATOR SPEED - Assumes we only clock at 8MHz or 80MHz
	#define GetInstructionClock()  	(( (OSCCONbits.COSC==0b011) || (OSCCONbits.COSC==0b001) ) ? 48000000UL : 3685000UL)
	// Used for SD card driver
	#define GetSystemClock()  		48000000UL /*Warning this should not be used other than in the SD card driver*/
	// Used in SD card code
	#define GetPeripheralClock()	(GetInstructionClock())

	// USB setup
	#define OWN_CDC_BUFFER_SIZES
	#define IN_BUFFER_CAPACITY 	256
	#define OUT_BUFFER_CAPACITY 256

    // USB bus sense I/O (we are always bus powered)
    // #define USE_USB_BUS_SENSE_IO
   	#define USB_BUS_SENSE       	1 
	#define USB_BUS_SENSE_PIN		dummy
	#define USB_BUS_SENSE_INIT() 	{}
	// Change notification int,  vectors to button handler
	#define USB_INIT_BUS_SENSE_INTS()	{}
						
    // MCHPFSUSB frameworkc self-power
    #define self_power          0   // 0 = bus-powered, 1 = self-powered

	// The pins
	#define InitIO()        {\
							REMAP_PINS();\
                            ANALOGUE_INIT_PINS();\
							LED_INIT_PINS();\
							BUTTON_INIT_PINS();\
							GSM_INIT_OFF();\
							SD_INIT_PINS();\
							Si44_INIT_PINS();\
							MRF_INIT_PINS_OFF();\
							ENC_INIT_PINS();\
                            DRIVE_FLOATING_PINS(); \
                            USB_BUS_SENSE_INIT();\
                            }

	#define DRIVE_FLOATING_PINS() {\
									TRISEbits.TRISE6 = 0; LATEbits.LATE6 = 0;\
									TRISBbits.TRISB10 = 0; LATBbits.LATB10 = 0;\
									}
	// REMAP pins
	#define REMAP_PINS()	{	/*SPI1 - SD card*/\
								RPINR20bits.SCK1R= 120;	/*SCK1i */\
								RPINR20bits.SDI1R= 121;	/*SDI1 */\
								RPOR14bits.RP120R = 0x06;/*SCK1o */\
								RPOR13bits.RP118R = 0x05;/*SDO1 */\
								/*SPI3 - TCPIP Tranceiver*/\
								RPINR1bits.INT2R = 45; /*INT2*/\
								RPINR29bits.SCK3R= 99;	/*SCK3i */\
								RPINR29bits.SDI3R= 100;	/*SDI3 */\
								RPOR8bits.RP99R = 0x20;	/*SCK3o */\
								RPOR9bits.RP101R = 0x1f;/*SDO3 */\
								/*SPI4 - Si4432 Tranceiver*/\
								RPINR0bits.INT1R = 75; 	/*INT1*/\
								RPINR31bits.SCK4R= 65;	/*SCK4i */\
								RPINR31bits.SDI4R= 67;	/*SDI4 */\
								RPOR0bits.RP65R = 0x23;	/*SCK4o */\
								RPOR1bits.RP66R = 0x22;/*SDO4 */\
								/*SPI2 - MRF24j40 Tranceiver FAIL (SPI2 not remappable)*/\
								RPINR1bits.INT3R = 70; /*INT3*/\
								/*UART1 - GSM radio*/\
								RPINR18bits.U1CTSR = 81;/*RP18 = U1CTS (GSM_CTS)*/\
								RPINR18bits.U1RXR = 83;	/*RP27 = U1RX (GSM_RX)*/\
								RPOR5bits.RP82R = 1;	/*RP19 = U1TX (GSM_TX)*/\
								/*RPOR14bits.RP80R = 2;*/ 	/*RP22 = U1RTS (GSM_RTS)*/\
								/*Then lock the PPS module*/\
								__builtin_write_OSCCONL(OSCCON | 0x40);}			

	#define REMAP_MRF()	{		__builtin_write_OSCCONL(OSCCON & 0xffBf);\
								RPINR31bits.SCK4R= 87;	/*SCK4i */\
								RPINR31bits.SDI4R= 84;	/*SDI4 */\
								RPOR6bits.RP87R = 0x23;	/*SCK4o */\
								RPOR6bits.RP85R = 0x22;/*SDO4 */\
								__builtin_write_OSCCONL(OSCCON | 0x40);}
	#define REMAP_Si44() {		__builtin_write_OSCCONL(OSCCON & 0xffBf);\
								RPINR31bits.SCK4R= 65;	/*SCK4i */\
								RPINR31bits.SDI4R= 67;	/*SDI4 */\
								RPOR0bits.RP65R = 0x23;	/*SCK4o */\
								RPOR1bits.RP66R = 0x22;/*SDO4 */\
								__builtin_write_OSCCONL(OSCCON | 0x40);}

	#define CLOCK_PLL()	ClockTunedFRCPLL() /* Use internal FRC with tuning*/

    // Clock: initialize PLL 
	#define CLOCK_XTAL_PLL() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							CLKDIVbits.PLLPRE = 0b00011; 			/*N1 div = 5, 20MHz xtal -> 4MHz */\
							PLLFBDbits.PLLDIV = 0b000101110;		/*M div = 48, 4*48 = 192 MHz */\
							CLKDIVbits.PLLPOST = 0b00; 				/*N2 div = 2, 192 -> 96 MHz */\
							ACLKCON3bits.SELACLK = 0; 				/*Use primary PLL to derive USB source*/\
							ACLKCON3bits.APLLPOST = 0b101; 			/*Div by 4, 192->48 MHz*/\
							OSCCONBITS_copy.NOSC = 0b011;			/*Internal Primary+PLL oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							while (OSCCONbits.OSWEN == 1);\
							asm("DISI #0");\
							/*while (OSCCONbits.LOCK != 1);*/  			/*Wait for PLL lock to engage*/\
							}

	// Use the following to switch to the internal osc pll, this is only possible if the secondary oscillator is 
	// stable and running. The internal FRC oscillator is tuned against the 32.768kHz reference. It is a good idea 
	// to monitor the FRC after tuning to ensure temperature drift does not lead to excessive frequency drift. 
	extern void ClockTunedFRCPLL(void); /*Warning - slow to return, disables interrupts, after tuning use CLOCK_INTOSC_PLL()*/
	#define CLOCK_INTOSC_PLL() 	{\
							/*Note: Need to tune FRC (use above function) first or actual clock will be 7.37Mhz x 24 = 88.44 MHz*/\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							CLKDIVbits.FRCDIV = 0b000; 				/*7.37MHz (Tune to 8Mhz for correct Fosc) */\
							CLKDIVbits.PLLPRE = 0b00000; 			/*N1 div = 2, 8Mhz -> 4MHz */\
							PLLFBDbits.PLLDIV = 0b000101110;		/*M div = 48, 4*48 = 192 MHz */\
							CLKDIVbits.PLLPOST = 0b00; 				/*N2 div = 2, 192 -> 96 MHz */\
							ACLKCON3bits.SELACLK = 0; 				/*Use primary PLL to derive USB source*/\
							ACLKCON3bits.APLLPOST = 0b101; 			/*Div by 4, 192->48 MHz*/\
							OSCCONBITS_copy.NOSC = 0b001;			/*Internal FRC+PLL oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							while (OSCCONbits.OSWEN == 1);\
							asm("DISI #0");\
							/*while (OSCCONbits.LOCK != 1); */			/*Wait for PLL lock to engage*/\
							}					

   // Clock: switch to 20 MHz HS XTAL
	#define CLOCK_XTAL() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							OSCCONBITS_copy.NOSC = 0b010;			/*Primary osc*/\
							OSCCONBITS_copy.OSWEN = 1;\
							asm("DISI #0x3FFF");\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							while (OSCCONbits.OSWEN == 1);\
							}

    // Clock: switch to 7.37 MHz INTOSC
	#define CLOCK_INTOSC() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							OSCCONBITS_copy.NOSC = 0b111;			/*Internal FRC-div oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							asm("DISI #0x3FFF");\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							while (OSCCONbits.OSWEN == 1);\
							asm("DISI #0");\
							CLKDIVbits.FRCDIV = 0b000; 				/*7.37MHz */\
							}
	#define CLOCK_INTOSC_NOWAIT() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							OSCCONBITS_copy.NOSC = 0b111;			/*Internal FRC-div oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							asm("DISI #0x3FFF");\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							}


    // Clock: switch to 31.25 kHz INTRC
    #define CLOCK_INTRC() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							OSCCONBITS_copy.NOSC = 0b101;			/*Internal RC oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							while (OSCCONbits.OSWEN == 1);\
							asm("DISI #0");\
							CLKDIVbits.FRCDIV = 0b000; 				/*8 MHz */\
							}

	// SOSC / RTC, 32.768 kHz
    #define CLOCK_32KHZ() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							OSCCONBITS_copy.NOSC = 0b100;			/*External secondary oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							while (OSCCONbits.OSWEN == 1);\
							asm("DISI #0");\
							CLKDIVbits.FRCDIV = 0b000; 				/*8 MHz */\
							}
       
	// Secondary oscillator on
	#define CLOCK_SOSCEN()	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							OSCCONBITS_copy.LPOSCEN=1;\
							asm("DISI #0x3FFF");\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");}

	// LEDs, labled left to right
    #define LED_1_PIN  			TRISBbits.TRISB4        
    #define LED_1               LATBbits.LATB4
    #define LED_2_PIN  			TRISBbits.TRISB3        
    #define LED_2               LATBbits.LATB3
    #define LED_3_PIN  			TRISBbits.TRISB2        
    #define LED_3               LATBbits.LATB2
    #define LED_4_PIN  			TRISBbits.TRISB1        
    #define LED_4               LATBbits.LATB1
    #define LED_5_PIN  			TRISBbits.TRISB0        
    #define LED_5               LATBbits.LATB0
    #define LED_6_PIN  			TRISBbits.TRISB8        
    #define LED_6               LATBbits.LATB8
    #define LED_7_PIN  			TRISBbits.TRISB9        
    #define LED_7               LATBbits.LATB9

    #define LED_INIT_PINS()     {TRISB&=0xfce0;LATB&=0xfce0;}
	#define LED_BLUE 	0b0000001
	#define LED_MAGENTA 0b1111111
	#define LED_OFF 	0b0000000
	#define LED_ALL		0b1111111

    // LED set (0bRGB) - should compile to three bit set/clears with a static colour value
    #define LED_SET(_c) {\
                    if ((_c) & 0x1) { LED_1 = 1; } else { LED_1 = 0; } \
                    if ((_c) & 0x2) { LED_2 = 1; } else { LED_2 = 0; } \
                    if ((_c) & 0x4) { LED_3 = 1; } else { LED_3 = 0; } \
                    if ((_c) & 0x8) { LED_4 = 1; } else { LED_4 = 0; } \
                    if ((_c) & 0x10) { LED_5 = 1; } else { LED_5 = 0; } \
                    if ((_c) & 0x20) { LED_6 = 1; } else { LED_6 = 0; } \
                    if ((_c) & 0x40) { LED_7 = 1; } else { LED_7 = 0; } \
                    }

	// LED assignments
	#define POWER_LED			LED_1
	#define CDC_LED				LED_2
	#define TCP_LED				LED_3
	#define SD_CARD_MOUNT_LED	LED_4
	#define FILE_LED			LED_5
	#define SI44_LED			LED_6
	#define TEL_LED				LED_7

	// Button
	#define BUTTON				(!PORTDbits.RD0)
	#define BUTTON_INIT_PINS() 	{TRISDbits.TRISD0=1;}

	// Analogue select pins - 0 = Analogue AN10 = CVref, AN11=sysTemp, AN12=sysVolts
	#define ADC_SELECT_L			0b1110001111111111
	#define ADC_SELECT_H			0xffff
	#define ANALOGUE_INIT_PINS()	{\
									ANSELB = 0b0001110000000000;\
									ANSELC = 0;\
									ANSELD = 0;\
									ANSELE = 0;\
									ANSELG = 0;\
									}

	// For the following 4 peripherals the interface is via I2C using these pins
	// All the pins need re-mapping. See RemapPins() in this file.

	// Defines for myI2C.c
	#define USE_HW_I2C1
	//#define USE_MY_SW_I2C
	#define ENABLE_I2C_TIMOUT_CHECK
	#define CHECK_TIMEOUT(_t)	myI2CCheckTimeout(_t)
	#define MY_I2C_TIMEOUT 	65535
	// EEPROM
	#define	EEPROM_ADDRESS	0xA0 	/*I2C address*/
	#define WRITE_PAGE_SIZE	0x80

	#define I2C_RATE_100kHZ 		(((OSCCONbits.COSC==0b001)||(OSCCONbits.COSC==0b011))? 628 : 39)	
	#define I2C_RATE_200kHZ 		(((OSCCONbits.COSC==0b001)||(OSCCONbits.COSC==0b011))? 288 : 18)
	#define I2C_RATE_400kHZ 		(((OSCCONbits.COSC==0b001)||(OSCCONbits.COSC==0b011))? 148 : 9)	
	#define I2C_RATE_1000kHZ 		(((OSCCONbits.COSC==0b001)||(OSCCONbits.COSC==0b011))? 52 : 3)			
	#define I2C_RATE_2000kHZ		(((OSCCONbits.COSC==0b001)||(OSCCONbits.COSC==0b011))? 52 : 3) /*The 2MHz rate is not supported*/

	#define mySCL				LATDbits.LATD10
	#define mySCLd				TRISDbits.TRISD10	
	#define mySDAr				PORTDbits.RD9
	#define mySDAw				LATDbits.LATD9	
	#define mySDAd				TRISDbits.TRISD9
	// If a bus lockup occurs the following will clear it
	#define myI2Cclear()		{unsigned char i;mySCLd=0;for(i=10;i;i--){mySCL=0;Delay10us(1);mySCL=1;Delay10us(1);};mySCLd=1;}

	// SD card interface - referenced to HOST
	extern short dummy;

	#define SD_CS			LATGbits.LATG7
	#define SD_CS_TRIS		TRISGbits.TRISG7
	#define SD_SDI			PORTGbits.RG9
	#define SD_SDI_TRIS		TRISGbits.TRISG9
	#define SD_SDO			LATGbits.LATG6
	#define SD_SDO_TRIS		TRISGbits.TRISG6
	#define SD_SCK			LATGbits.LATG8
	#define SD_SCK_TRIS		TRISGbits.TRISG8
	#define SD_CD_TRIS		TRISDbits.TRISD8
	#define SD_CD			PORTDbits.RD8
	//#define MEDIA_SOFT_DETECT
	#define SD_WE_TRIS		dummy
	#define SD_WE			0

    #define 	SD_CS_ANSEL		ANSELBbits.ANSB8
    #define    	SD_SCK_ANSEL	dummy
    #define    	SD_SDI_ANSEL	dummy
    #define    	SD_SDO_ANSEL	dummy

	#define SD_INIT_PINS()	{\
							SD_SCK_TRIS = 0;\
							SD_SCK = 0;\
							SD_SDO_TRIS = 0;\
							SD_SDO = 0;\
							SD_CS_TRIS = 0;\
							SD_CS = 1;\
							SD_SDI_TRIS = 1;\
							}
	#define SD_ENABLE()		{SD_INIT_PINS();}
	#define SD_DISABLE()	{SD_INIT_PINS();}

    /*******************************************************************/
    /******** MDD File System selection options ************************/
    /*******************************************************************/
    #define USE_SD_INTERFACE_WITH_SPI
    #define USE_PIC24F
    #define USE_16BIT

	//SPI Configuration - writes to SPIxCON2 SPRE,PPRE
	//#define SYNC_MODE_FAST GetValidSPICON2SettingFromSysClock()

	// Following value is written to SPIxCON1, sets clock scaling
	//#define SYNC_MODE_FAST 		0x3B	/*48 div2 div1 = 24MHz*/
	#define SYNC_MODE_FAST 		0x37	/*48 div3 div1 = 16MHz*/
	//#define SYNC_MODE_FAST 		0x33	/*48 div4 div1 = 12MHz*/
	//#define SYNC_MODE_FAST 		0x2F	/*48 div5 div1 9.6MHz*/

    // Define the SPI frequency
    // Tris pins for SCK/SDI/SDO lines
    #define SPICLOCK			SD_SCK_TRIS
    #define SPIIN               SD_SDI_TRIS
    #define SPIOUT				SD_SDO_TRIS
    // Registers for the SPI module you want to use
    #define SPICON1				SPI1CON1
    #define SPISTAT				SPI1STAT
    #define SPIBUF				SPI1BUF
    #define SPISTAT_RBF			SPI1STATbits.SPIRBF
    #define SPICON1bits			SPI1CON1bits
    #define SPISTATbits			SPI1STATbits
    #define SPIENABLE           SPI1STATbits.SPIEN

    /*******************************************************************/

	#define GSM_PWR_PIN 	TRISFbits.TRISF0
	#define GSM_PWR			LATFbits.LATF0
	#define GSM_nRST_PIN	TRISFbits.TRISF1
	#define GSM_nRST		LATFbits.LATF1
	#define GSM_CTS_PIN		TRISEbits.TRISE1
	#define GSM_CTS			PORTEbits.RE1
	#define GSM_RTS_PIN 	TRISEbits.TRISE0
	#define GSM_RTS			LATEbits.LATE0
	#define GSM_TX_PIN 		TRISEbits.TRISE2
	#define GSM_TX			LATEbits.LATE2
	#define GSM_RX_PIN		TRISEbits.TRISE3
	#define GSM_RX			PORTEbits.RE3

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

	// ENC28J60 TCPIP Tranceiver
	#define ENC_CS_TRIS			TRISBbits.TRISB14
	#define ENC_CS_IO			LATBbits.LATB14
	#define ENC_INT_PIN			TRISBbits.TRISB13
	#define ENC_INT				PORTBbits.RB13
	#define ENC_RESET_PIN		TRISBbits.TRISB15
	#define ENC_RESET			LATBbits.LATB15
    #define ENC_SCK_TRIS 		TRISFbits.TRISF3
    #define ENC_SDO_TRIS 		TRISFbits.TRISF5
    #define ENC_SDI_TRIS 		TRISFbits.TRISF4

	// SPI SCK, SDI, SDO pins are automatically controlled by the 
	// PIC24/dsPIC SPI module 
	#define ENC_SPI_IF			(IFS5bits.SPI3IF)
	#define ENC_SSPBUF			(SPI3BUF)
	#define ENC_SPISTAT			(SPI3STAT)
	#define ENC_SPISTATbits		(SPI3STATbits)
	#define ENC_SPICON1			(SPI3CON1)
	#define ENC_SPICON1bits		(SPI3CON1bits)
	#define ENC_SPICON2			(SPI3CON2)

	#define ENC_INIT_PINS()		{	ENC_SCK_TRIS = ENC_SDO_TRIS = 0;\
									ENC_CS_IO = 1;ENC_CS_TRIS = 0;\
									ENC_RESET = 0;ENC_RESET_PIN = 0;\
									ENC_SDI_TRIS = ENC_INT_PIN = 1; }
							

	// KL ENC28J60 simpler implementation
	#define ENC_SPI_INIT()		{	SPI3STAT = 0x18;SPI3CON1 = 0x012F;\
									/*SMP = 0, CKE = 1, CKP = 0, 9.6MHz clock*/\
									/*SPI3STAT = 0x18;SPI3CON1 = 0x012f;*/\
									/*SMP = 0, CKE = 0, CKP = 0, 9.6MHz clock*/\
									SPI3STATbits.SPIEN=1;SPI3CON1bits.SMP=0;}
	#define ENC_SPI_OFF()		{	SPI3STATbits.SPIEN = 0;}
	#define ENC_SPI_OPEN()		{ENC_CS_IO = 0;}
	#define ENC_SPI_PUTC(_c)	{\
									dummy = SPI3BUF; 	/*Clear in buffer*/\
									while (SPI3STATbits.SPITBF);/*Wait Tx*/\
									SPI3BUF = _c;		/*Send char out*/\
									while (!SPI3STATbits.SPIRBF);/*Wait Rx*/\
								}
	#define ENC_SPI_GETC()		SPI3BUF							
	#define ENC_SPI_CLOSE()		{ENC_CS_IO = 1;}

	// Radio SPI Control abstraction
	#define Si44_INIT_PINS()	{\
								TRISDbits.TRISD7 = 0; LATDbits.LATD7 = 1;\
								TRISDbits.TRISD1 = 0; LATDbits.LATD1 = 0;\
								TRISDbits.TRISD2 = 0; LATDbits.LATD2 = 0;\
								TRISDbits.TRISD3 = 1;TRISDbits.TRISD11 = 1;}
	#define Si44_CS				LATDbits.LATD7
	#define Si44_nIRQ			PORTDbits.RD11
	#define Si44_IRQ_FLAG		IFS1bits.INT1IF
	#define Si44_IRQ_EN()		{IEC1bits.INT1IE = 1;}
	#define Si44_IRQ_OFF()		{IEC1bits.INT1IE = 0;}
	#define Si44Int				_INT1Interrupt
	#define Si44_SPI_INIT()		{	IPC5bits.INT1IP = Si44_INT_PRIORITY;\
								 	INTCON2bits.INT1EP = 1; /*Falling edge*/\
									SPI4STAT = 0x18;SPI4CON1 = 0x012f; \
									/*SMP = 0, CKE = 1, CKP = 0, 9.6MHz clock*/\
									SPI4STATbits.SPIEN=1;}
	#define Si44_SPI_OFF()		{	SPI4STATbits.SPIEN = 0;}
	#define Si44_SPI_OPEN()		{Si44_CS = 0;}
	#define Si44_SPI_PUTC(_c)	{\
									dummy = SPI4BUF; 	/*Clear in buffer*/\
									while (SPI4STATbits.SPITBF);/*Wait Tx*/\
									SPI4BUF = _c;		/*Send char out*/\
									while (!SPI4STATbits.SPIRBF);/*Wait Rx*/\
								}
	#define Si44_SPI_GETC()		SPI4BUF							
	#define Si44_SPI_CLOSE()	{Si44_CS = 1;}

 	// MRF module
    #define RF_INT_PIN          PORTDbits.RD6
    #define MRF_INT_TRIS         TRISDbits.TRISD6
    #define PHY_CS              LATBbits.LATB5
    #define PHY_CS_TRIS         TRISBbits.TRISB5
    #define PHY_RESETn          LATDbits.LATD4
    #define PHY_RESETn_TRIS     TRISDbits.TRISD4
    #define PHY_WAKE            LATDbits.LATD5
    #define PHY_WAKE_TRIS       TRISDbits.TRISD5
    #define MRF_USE_SPI          4
    #define MRF_SPI_SI_TRIS      TRISEbits.TRISE4
   	#define MRF_SPI_SDI          PORTEbits.RE4
    #define MRF_SPI_SO_TRIS      TRISEbits.TRISE5
    #define MRF_SPI_SDO          LATEbits.LATE5 
    #define MRF_SPI_SCK_TRIS     TRISEbits.TRISE7
    #define MRF_SPI_SCK          LATEbits.LATE7 

    #define MRFIF                IFS3bits.INT3IF
    #define MRFIE                IEC3bits.INT3IE
	#define MRFIP				 IPC13bits.INT3IP
	#define MRFInt				_INT3Interrupt
	#define MRFIsr				MrfInterruptIsr
    #define MRF_IEDG  			INTCON2bits.INT3EP
	extern void MrfInterruptIsr(void);

    // Miwi Stack integration
   	#define SPI_SDI             MRF_SPI_SDI
    #define SDI_TRIS            MRF_SPI_SI_TRIS
    #define SPI_SDO             MRF_SPI_SDO 
    #define SDO_TRIS            MRF_SPI_SO_TRIS
    #define SPI_SCK             MRF_SPI_SCK 
    #define SCK_TRIS            MRF_SPI_SCK_TRIS

    #define TMRL                (TMR2) /*TickLocal.c*/
    #define RFIF                IFS3bits.INT3IF
    #define RFIE                IEC3bits.INT3IE
    #define MRF_INIT_PINS()    {\
                                MRF_SPI_SI_TRIS = 1;\
                                MRF_SPI_SO_TRIS = 0;\
                                MRF_SPI_SCK_TRIS = 0;\
                                PHY_CS = 1;\
                                PHY_CS_TRIS = 0;\
                                PHY_RESETn = 0;\
                                PHY_RESETn_TRIS = 0;\
                                PHY_WAKE = 1;\
                                PHY_WAKE_TRIS = 0;\
                                RFIF = 0;\
                                MRF_INT_TRIS = 1;\
								MRFIP = MRF_INT_PRIORITY;\
								MRF_IEDG = 1; /*FALLING EDGE*/ \
                            }
                            
    #define MRF_INIT_PINS_OFF()    	MRF_INIT_PINS()
	#define MRF_POWER_ON()			MRF_INIT_PINS()                     
	#define MRF_POWER_OFF()			MRF_INIT_PINS() 

	#define MRF_SPI_INIT()		{	SPI4STAT = 0x18;SPI4CON1 = 0x012f; \
									/*SMP = 0, CKE = 1, CKP = 0, 9.6MHz clock*/\
									SPI4STATbits.SPIEN=1;}
	#define MRF_SPI_OFF()		{	SPI4STATbits.SPIEN = 0;}
	#define MRF_SPI_OPEN()		{Si44_CS = 0;}
	#define MRF_SPI_PUTC(_c)	{\
									dummy = SPI4BUF; 	/*Clear in buffer*/\
									while (SPI4STATbits.SPITBF);/*Wait Tx*/\
									SPI4BUF = _c;		/*Send char out*/\
									while (!SPI4STATbits.SPIRBF);/*Wait Rx*/\
								}
	#define MRF_SPI_GETC()		SPI4BUF							
	#define MRF_SPI_CLOSE()		{Si44_CS = 1;}

	#define MRF_REMAP()		{MRF_SPI_OFF();REMAP_MRF();MRF_SPI_INIT();}		// SPI toggled off/on and redirected to other pins
	#define MRF_UNREMAP()	{MRF_SPI_OFF();REMAP_Si44();MRF_SPI_INIT();}	// SPI toggled off/on and redirected to original

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

	#define BUTTON0_IO			!BUTTON
	#define BUTTON1_IO			1
	#define BUTTON2_IO			1
	#define BUTTON3_IO			1

	#define mInitAllLEDs()      LED_INIT_PINS()
    
	#define LED					LED_R
    #define mLED_1              LED_R
    #define mLED_2              LED_G
    #define mLED_3              LED_B
    #define mLED_4              LED_R

	#define 	LED0_TRIS	LED_1_PIN
	#define 	LED1_TRIS	LED_2_PIN
	#define 	LED2_TRIS	LED_3_PIN
	#define 	LED3_TRIS	LED_4_PIN
	#define 	LED4_TRIS	LED_5_PIN
	#define 	LED5_TRIS	LED_6_PIN
	#define 	LED6_TRIS	LED_7_PIN
	#define 	LED7_TRIS	LED_7_PIN
	#define 	LED_PUT(_c)	LED_SET(_c)

	#define LED0_IO LED_1
	#define LED1_IO LED_2
	#define LED2_IO LED_3
	#define LED3_IO LED_4
	#define LED4_IO LED_5
	#define LED5_IO LED_6
	#define LED6_IO LED_7
	#define LED7_IO LED_7


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

