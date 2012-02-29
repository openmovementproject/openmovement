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
 *
 * Hardware profile for the CWA 1.7 (using PIC24FJ256GB106)
 * Karim Ladha, Dan Jackson, 2011
 */
 
#ifndef HARDWAREPROFILE_CWA17_H
#define HARDWAREPROFILE_CWA17_H

    #ifndef __PIC24FJ256GB106__
    //    #error "Unexpected device selection"
    #endif

    #ifndef __C30__
    //    #error "Unexpected compiler"
    #endif

    #define 	PIC24
    #define 	CWA3_1
    #define HARDWARE_VERSION 0x31           // BCD format (for USB response)

	// Interrupt priority levels
	// Bluetooth UART ints
	#define BT_UART_INT_PRIORITY			7 /* This makes the uart appear to be a DMA device to the CPU*/

	// Highest - RTC, TIMER tasks
	#define RTC_INT_PRIORITY				6 
	#define T1_INT_PRIORITY					RTC_INT_PRIORITY 	/*Prevents ms rollover during reads*/

	// Mid level - FIFO time stamps only
	#define FIFO_INTERRUPT_PRIORITY 		5
	#define CN_INTS_PROCESSOR_PRIORITY 		FIFO_INTERRUPT_PRIORITY
	#define ACCEL_INT_PRIORITY				FIFO_INTERRUPT_PRIORITY
	#define GYRO_INT_PRIORITY				FIFO_INTERRUPT_PRIORITY

	// Low level - Grabbing data from devices, I2C comms, ADC sample wait etc
	#define GLOBAL_I2C_PROCESSOR_PRIORITY	4
	#define DATA_STREAM_INTERRUPT_PRIORITY 	GLOBAL_I2C_PROCESSOR_PRIORITY

	// Unused priority levels
	#define ADC_INT_PRIORITY				3  /*This is very fast, can be used to halt ADC sequence*/

	// SW WDT Timeout in seconds
	#define RTC_SWWDT_TIMEOUT 600

// Used to restric the write flash function
#define MIN_ALLOWED_WRITE_ADD 0x1400		// Protects bootloader
#define MAX_ALLOWED_WRITE_ADD 0x2ABFE		// Protects configuation
	
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
#define DISP_POWER_DOWN			0x800
#define GYRO_POWER_DOWN			0x1000
#define ACCEL_POWER_DOWN		0x2000
#define SAVE_INT_STATUS			0x4000
#define ALLOW_VECTOR_ON_WAKE	0x8000
#define LEAVE_WDT_RUNNING		0x10000
#define JUST_POWER_DOWN_PERIPHERALS	0x20000
#define DONT_RESTORE_PERIPHERALS	0x40000
#define WAKE_ON_BT_MODULE 			0x80000
#define MAG_POWER_DOWN				0x100000
#define BT_POWER_DOWN				0x200000
#define WAKE_ON_MAG					0x400000

// Essential Functions in HardwareProfile.c
extern void WaitForPrecharge(void);
extern void SystemPwrSave(unsigned long NapSetting);

// LED Colours 0bRGB
enum { LED_OFF, LED_BLUE, LED_GREEN, LED_CYAN, LED_RED, LED_MAGENTA, LED_YELLOW, LED_WHITE };

	// Used for delays in TimeDelay.h
	#define GetInstructionClock()  4000000

    // USB bus sense I/O
    // #define USE_USB_BUS_SENSE_IO
	#define USB_BUS_SENSE_PIN	TRISBbits.TRISB4
    #define USB_BUS_SENSE       PORTBbits.RB4
	#define USB_BUS_SENSE_INIT() USB_BUS_SENSE_PIN=1
	// Change notification int,  vectors to button handler
	#define USB_INIT_BUS_SENSE_INTS()	{/*Change notification interrupt CN6 - for wake up purposes*/\
										CNEN1bits.CN6IE;\
										IPC4bits.CNIP=4;\
										IFS1bits.CNIF = 0;\
										IEC1bits.CNIE=1;\
										}
										
    // MCHPFSUSB frameworkc self-power
    #define self_power          0   // 0 = bus-powered, 1 = self-powered

    // ROM storage
    #define WRITE_BLOCK_SIZE 128		// Write in rows of 64 instructions (only 2/3 bytes available = 128 bytes)
    #define ERASE_BLOCK_SIZE 1024		// Erase in blocks of 8 rows, 512 instructions (only 2/3 bytes available = 1024 bytes)

	// The pins
	#define InitIO()        {\
                            REMAP_PINS();\
							LED_INIT_PINS();\
                            FLASH_INIT_PINS();\
                            ACCEL_INIT_PINS();\
							GYRO_INIT_PINS();\
							MAG_INIT_PINS();\
							ALT_INIT_PINS();\
							DISPLAY_INIT_PINS();\
							BT_INIT_PINS();\
							BATT_INIT_PINS();\
                            TEMP_INIT_PINS();\
                            LDR_INIT_PINS();\
							BUTTON_INIT_PINS();\
							MOTOR_INIT_PINS();\
                            ANALOGUE_INIT_PINS();\
                            DRIVE_FLOATING_PINS(); /*Reduce leakage*/\
                            USB_BUS_SENSE_INIT();\
                            }

	#define DRIVE_FLOATING_PINS() {\
                            TRISCbits.TRISC14 = 0; LATCbits.LATC14 = 0;\
                            TRISDbits.TRISD6 = 0; LATDbits.LATD6 = 0;\
                            TRISBbits.TRISB0 = 0; LATBbits.LATB0 = 0;/*PGD*/\
                            TRISBbits.TRISB1 = 0; LATBbits.LATB1 = 0;/*PGC*/\
                            }
							
	// REMAP pins
	#define REMAP_PINS()	{\
							__builtin_write_OSCCONL(OSCCON & 0xBF);/*Unlock*/\
							/*INPUTS*/\
							RPINR0bits.INT1R = 22;/* AccInt1 RP22 -> Int1 */\
							RPINR1bits.INT2R = 23;/* AccInt2 RP23 -> Int2 */\
							RPINR1bits.INT3R = 24;/* GyroInt2 dtrdy RP11 -> Int3*/\
							/*RPINR2bits.INT4R = --;DO NOT USE - SW INT*/\
							RPINR18bits.U1CTSR = 19;/*RP19 = U1CTS (BT_RTS)*/\
							RPINR18bits.U1RXR = 21;	/*RP2 = U1RX (BT_TX)*/\
							/*OUTPUTS*/\
							RPOR13bits.RP26R = 3;	/*RP26 = U1TX (BT_RX)*/\
							/*RPOR13bits.RP27R = 4;*/ 	/*RP27 = U1RTS (BT_CTS)*/\
							RPOR9bits.RP18R = 36;	/*Blutooth 32kHz clock routed through comparator3*/\
							RPOR8bits.RP16R = 18;/* OC1 module to motor pin - PWM*/\
							/*Then lock the PPS module*/\
							__builtin_write_OSCCONL(OSCCON | 0x40);\
							}						

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

    // LED
    #define LED_G_PIN  			TRISCbits.TRISC12        
    #define LED_G               LATCbits.LATC12
    #define LED_R_PIN           TRISCbits.TRISC15
    #define LED_R              	LATCbits.LATC15 
    #define LED_B_PIN           TRISDbits.TRISD8
    #define LED_B     			LATDbits.LATD8
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


    // Battery - AN2
    #define BATT_MON_PIN        TRISBbits.TRISB2
    #define BATT_INIT_PINS()   	BATT_MON_PIN = 1
    #define BATT_CHARGE_ZERO		614		// FIXED CONST, DO NOT USE
	#define BATT_CHARGE_FULL		708		// FIXED CONST, DO NOT USE
	#define BATT_CHARGE_MIN_LOG		517		// minimum level to update log
	#define BATT_CHARGE_MIN_SAFE	520		// minimum safe running voltage (5%), avoids unwanted RTC resets and file corruptions etc.
    #define BATT_CHARGE_MID_USB		666     // level to count towards recharge cycle counter (75%)
    #define BATT_CHARGE_FULL_USB	708


    // LDR - AN15
    #define LDR_EN_PIN			TRISFbits.TRISF4         
    #define LDR_EN              LATFbits.LATF4
    #define LDR_OUT_PIN         TRISBbits.TRISB15
    #define LDR_ENABLE()        {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 0;}
    #define LDR_DISABLE()       {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 1;}
    #define LDR_INIT_PINS()     LDR_DISABLE()

	// Temp sensor - AN14
	#define USE_MCP9701
	#define TEMP_EN_PIN			TRISDbits.TRISD11
	#define TEMP_EN				LATDbits.LATD11
	#define TEMP_OUT_PIN		TRISBbits.TRISB14
	#define TEMP_ENABLE()		{TEMP_OUT_PIN=1;TEMP_EN_PIN=0;TEMP_EN=1;}
	#define TEMP_DISABLE()		{TEMP_OUT_PIN=1;TEMP_EN_PIN=0;TEMP_EN=0;}
	#define TEMP_INIT_PINS()	TEMP_DISABLE()

	// Analogue select pins - 0 = Analogue
	#define ADC_SELECT_L		0b0011111111111011
	#define ADC_SELECT_H		0b1111111111111111
	#define ANALOGUE_INIT_PINS()	{AD1PCFGL = ADC_SELECT_L;AD1PCFGH = ADC_SELECT_H;}
    #define ADC_INDEX_BATT		0
    #define ADC_INDEX_LDR		2
    #define ADC_INDEX_TEMP		1

	// Several devices can use the change notification interrups
	#define CN_INTS_ON()	{IFS1bits.CNIF = 0;IPC4bits.CNIP = CN_INTS_PROCESSOR_PRIORITY; IEC1bits.CNIE = 1;}
	#define CN_INTS_OFF()	{IEC1bits.CNIE = 0;}
	#define CN_INT_FIRE()	{IFS1bits.CNIF = 1;}

	// For the following 4 peripherals the interface is via I2C using these pins
	// All the pins need re-mapping. See RemapPins() in this file.

	// Defines for myI2C.c
	#define USE_HW_I2C
	//#define USE_MY_SW_I2C
	#ifndef USE_MY_SW_I2C
		#define InitI2C()			{myI2Cclear();mySCLd = 1;mySDAd=1;I2C1CON=0;myI2CIdle();}
	#else
		#define InitI2C()			{myI2Cclear();mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}
	#endif
	/*for SW code use : {mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}*/
	#define mySCL				LATDbits.LATD10
	#define mySCLd				TRISDbits.TRISD10	
	#define mySDAr				PORTDbits.RD9
	#define mySDAw				LATDbits.LATD9	
	#define mySDAd				TRISDbits.TRISD9
	// If a bus lockup occurs the following will clear it
	#define myI2Cclear()		{unsigned char i;mySCLd=0;for(i=10;i;i--){mySCL=0;Delay10us(1);mySCL=1;Delay10us(1);}}

	/* I2C devices that may be present
	Accelerometer 	MMAQ8452Q 	0x38
	Gyroscope 		L3G4200D	0xD2
	Magnetometer	MAG3110		0x1C
	Altimeter		BMP085		0xEE
	*/

    // Accelerometer MMA8451Q
	#define ACCEL_MMA8451Q
	#define ACCEL_I2C_MODE
	#define ACCEL_FIFO_WATERMARK 25
    #define ACCEL_INT1_PIN      TRISDbits.TRISD3 
    #define ACCEL_INT2_PIN      TRISDbits.TRISD2 	
    #define ACCEL_INT1          PORTDbits.RD3
    #define ACCEL_INT2          PORTDbits.RD2 
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
								
	// Gyro L3G4200
	#define USE_GYRO
	#define GYRO_IS_L3G4200D
	#define GYRO_I2C_MODE
	#define GYRO_FIFO_WATERMARK	25
    #define GYRO_INT1_PIN     	TRISDbits.TRISD0  
    #define GYRO_INT2_PIN     	TRISDbits.TRISD1   
    #define GYRO_INT1        	PORTDbits.RD0    
    #define GYRO_INT2 			PORTDbits.RD1 
//    #define GYRO_INT1_IP        IPC13bits.INT3IP // Data ready on GYRO_INT2
//    #define GYRO_INT1_IF        IFS3bits.INT3IF
//    #define GYRO_INT1_IE        IEC3bits.INT3IE
    #define GYRO_INT2_IP        IPC13bits.INT3IP    
    #define GYRO_INT2_IF        IFS3bits.INT3IF 
    #define GYRO_INT2_IE        IEC3bits.INT3IE  

    #define GYRO_INIT_PINS()  	{\
								GYRO_INT1_PIN	= 1;\
								GYRO_INT2_PIN	= 1;\
								}
								
	// Magnetometer MAG3110 - interrupt on CN69
	#define MAG_I2C_MODE
	#define MAG_INT_PIN     	TRISFbits.TRISF1  
    #define MAG_INT        		PORTFbits.RF1    
	#define MAG_CN_INT_IE		CNEN5bits.CN69IE
    #define MAG_INIT_PINS()  	{\
								MAG_INT_PIN	= 1;\
								}
	#define MAG_ENABLE_INTS()	{/*Interrupt on change CN18*/\
									CNEN2bits.CN18IE;\
									CN_INTS_ON();\
								}
								
	// Altimeter BMP085
	#define ALT_IGNORE_EOC
	#define ALT_EOC_PIN     	TRISFbits.TRISF0  
    #define ALT_EOC        		PORTFbits.RF0 
	#define ALT_XCLR_PIN		TRISDbits.TRISD7	
	#define ALT_XCLR			PORTDbits.RD7
	#define ALT_CN_INT			CNEN5bits.CN68IE
    #define ALT_INIT_PINS()  	{\
								ALT_EOC_PIN	= 1;\
								ALT_XCLR_PIN= 0;\
								ALT_XCLR = 1;\
								}
	
    // Flash Memory - Hynix/Micron
	#define NAND_DEVICE 	NAND_DEVICE_HY27UF084G2B
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
    #define FLASH_CE1_PIN  	TRISBbits.TRISB10 
    #define FLASH_CE1		LATBbits.LATB10
    #define FLASH_CE2_PIN  	TRISBbits.TRISB9  
    #define FLASH_CE2		LATBbits.LATB9                
    #define FLASH_CLE_PIN 	TRISBbits.TRISB12  
    #define FLASH_CLE		LATBbits.LATB12       
    #define FLASH_ALE_PIN 	TRISBbits.TRISB13   
    #define FLASH_ALE		LATBbits.LATB13      
    #define FLASH_RB_PIN 	TRISBbits.TRISB11    
    #define FLASH_RB		PORTBbits.RB11 
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
								FLASH_CE2_PIN	= 0;\
								FLASH_CE2		= 1;/*Disabled*/\
								FLASH_CLE_PIN	= 0;\
								FLASH_CLE		= 0;\
								FLASH_ALE_PIN 	= 0;\
								FLASH_ALE		= 0;\
								/*FLASH_WP_PIN 	= 0;NOT USED*/\
								/*FLASH_WP		= 1;NOT USED*/\
								FLASH_RB_PIN 	= 1;\
								}	


	// Bluetooth module test PAN1315 
	#define BT_SLW_CLK_PIN	TRISBbits.TRISB5
	#define BT_SLW_CLK	LATBbits.LATB5
	#define BT_EN_PIN	TRISBbits.TRISB3
	#define BT_EN		LATBbits.LATB3
	#define BT_CTS_PIN 	TRISGbits.TRISG9
	#define BT_CTS		LATGbits.LATG9
	#define BT_RTS_PIN	TRISGbits.TRISG8
	#define BT_RTS		PORTGbits.RG8
	#define BT_RX_PIN	TRISGbits.TRISG7
	#define BT_TX_PIN	TRISGbits.TRISG6
	
	#define BT_UART_RX_IF	IFS0bits.U1RXIF  
	#define BT_UART_RX_IE	IEC0bits.U1RXIE
	#define BT_UART_TX_IF	IFS0bits.U1TXIF
	#define BT_UART_TX_IE	IEC0bits.U1TXIE
//	#define BT_CTS_IE		IEC3bits.INT3IE
//	#define BT_CTS_IF		IFS3bits.INT3IF
	#define BT_BAUD_REG	U1BRG
	#define BT_UART_TX_REG	U1TXREG
	#define BT_UART_RX_REG 	U1RXREG
	#define BT_INIT_PINS()	{BT_EN_PIN=0;BT_EN=0;\
							BT_CTS_PIN=0;BT_CTS=1;\
							BT_RTS_PIN=1;\
							BT_RX_PIN=0;\
							BT_TX_PIN=1;}
	// BT Clock - HACK, routed the sosc through the comparator
	#define STOP_BT_SLW_CLK()	{CM3CONbits.CEN = 1;CVRCONbits.CVREN=0;BT_SLW_CLK=0;}
	#define SETUP_BT_SLW_CLK()	{\
								CM3CONbits.CEN = 1;\
								CM3CONbits.COE = 1;\
								CM3CONbits.CREF = 1;\
								CM3CONbits.CCH = 0b10;/*Input D*/\
								CVRCONbits.CVREN = 1;\
								CVRCONbits.CVRR = 0;\
								CVRCONbits.CVR = 0x8;/*Vref = Vdd/2 */\
								/*Note: C3OUT to RP21*/\
								BT_SLW_CLK_PIN = 0;}	

	// Graphical DISPLAY - OLED on SSD1306,8080
    #define DISPLAY_CONTROLLER CUSTOM_CONTROLLER

    #define DISP_HOR_RESOLUTION 128
    #define DISP_VER_RESOLUTION 64
    #define COLOR_DEPTH 1
    #define DISP_ORIENTATION 0

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
	
	#define DISPLAY_CHIP_SELECT_TRIS    TRISBbits.TRISB7
	#define DISPLAY_CHIP_SELECT         LATBbits.LATB7
	#define DISPLAY_RESET_TRIS          TRISBbits.TRISB6
	#define DISPLAY_RESET               LATBbits.LATB6
	#define DISPLAY_COMMAND_DATA_TRIS   TRISBbits.TRISB8
	#define DISPLAY_COMMAND_DATA        LATBbits.LATB8

	#define DISPLAY_INIT_PINS()		{\
								DISPLAY_DATA_TRIS 			= 0;\
								DISPLAY_DATA 				= 0;\
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
					
	// Button CWA3.2 only - active low
	#define BUTTON_PIN		TRISFbits.TRISF5
	#define BUTTON			PORTFbits.RF5
	#define BUTTON_INIT_PINS()	{BUTTON_PIN=1;}
	#define BUTTON_ENABLE_INTS()		{/*Interrupt on change CN18*/\
										CNEN2bits.CN18IE;\
										CN_INTS_ON();\
										}

	// Motor control pin
	#define MOTOR_PIN		TRISFbits.TRISF3
	#define MOTOR			LATFbits.LATF3
	#define MOTOR_INIT_PINS() 	{MOTOR=0;MOTOR_PIN=0;}

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

