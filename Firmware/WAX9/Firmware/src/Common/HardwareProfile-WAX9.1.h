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

// Hardware profile for WAX9
// Karim Ladha, 2013-2014

#ifndef HARDWAREPROFILE_WAX9_H
#define HARDWAREPROFILE_WAX9_H

	// Interrupt priority levels
	// Bluetooth UART ints
	#define BT_UART_RX_INT_PRIORITY			6 	/* This makes the uart appear to be a DMA device to the CPU if IPL is < 7*/
	#define BT_UART_TX_INT_PRIORITY			5	/* This makes the uart appear to be a DMA device to the CPU if IPL is < 6*/
	#define BT_UART_CTS_INT_PRIORITY		BT_UART_TX_INT_PRIORITY

	// Highest - RTC, TIMER tasks
	#define T1_INT_PRIORITY					7 	/*Prevents ms rollover during reads*/

	// Mid level - FIFO time stamps only
	#define FIFO_INTERRUPT_PRIORITY 		4
	#define BT_EVENT_HANDLER_PRIORITY		FIFO_INTERRUPT_PRIORITY /*If using bt i/o fifos*/

	#define PERIPHERAL_INTERRUPT_PRIORITY	FIFO_INTERRUPT_PRIORITY
	#define ONE_SECOND_TRIGGER_INT_PRIORITY PERIPHERAL_INTERRUPT_PRIORITY
	#define SAMPLER_INT_PRIORITY			PERIPHERAL_INTERRUPT_PRIORITY
	#define ACCEL_INT_PRIORITY				PERIPHERAL_INTERRUPT_PRIORITY
	#define GYRO_INT_PRIORITY				PERIPHERAL_INTERRUPT_PRIORITY
	#define MAG_INT_PRIORITY				PERIPHERAL_INTERRUPT_PRIORITY
	#define CN_INTS_PROCESSOR_PRIORITY		PERIPHERAL_INTERRUPT_PRIORITY

	// Low level - Grabbing data from devices, I2C comms, ADC sample wait etc

	// Unused priority levels
	#define ADC_INT_PRIORITY				0  /*This is very fast, can be used to halt ADC sequence*/

	// Used to restric the write flash function
	#define MIN_ALLOWED_WRITE_ADD 0x1400		// Protects bootloader
	#define MAX_ALLOWED_WRITE_ADD 0xA400		// Protects configuation

	//#define ACCEL_8BIT_MODE
	#define ACCEL_RATE			ACCEL_RATE_100
	#define ACCEL_RANGE			ACCEL_RANGE_8G
	#define ACCEL_DEFAULT_RATE  (ACCEL_RATE | ACCEL_RANGE)

	// Adds barometer temp to ADC results
	#define USE_BAROMETER_TEMP
	
//SystemPwrSave arguments OR together
#define WAKE_ON_WDT				0x40
#define WAKE_ON_RTC				0x80
#define WAKE_ON_TIMER1			0x100
#define LOWER_PWR_SLOWER_WAKE	0x200

// Debugging/Error support
typedef enum {
	OTHER_INT = 0,
	STACK_ERR = 2,
	ADDR_ERROR = 4,
	MATH_ERR = 8,
	BT_ERR = 3,
	I2C_ERR = 5	
}SwResetReason_t;
extern void DebugReset(SwResetReason_t reason);

// Essential Functions in HardwareProfile.c
extern void WaitForPrecharge(void);
extern void SystemPwrSave(unsigned long NapSetting);

// LED Colours 0bRGB
enum { LED_OFF, LED_BLUE, LED_GREEN, LED_CYAN, LED_RED, LED_MAGENTA, LED_YELLOW, LED_WHITE };

	// Used for delays in TimeDelay.h
	#define GetInstructionClock()  	((OSCCONbits.COSC == 0b001)?16000000ul:4000000ul)
	#define GetSystemClockMIPS()	((OSCCONbits.COSC == 0b001)?16:4)

    // USB bus sense I/O
    // #define USE_USB_BUS_SENSE_IO
	#define USB_BUS_SENSE_PIN	TRISAbits.TRISA10
    #define USB_BUS_SENSE       PORTAbits.RA10
	#define USB_BUS_SENSE_INIT() USB_BUS_SENSE_PIN=1
	// Change notification int,  vectors to button handler
	#define USB_INIT_BUS_SENSE_INTS()	{}
										
    // MCHPFSUSB framework self-power
    #define self_power          0   // 0 = bus-powered, 1 = self-powered

    // ROM storage
    #define WRITE_BLOCK_SIZE 128		// Write in rows of 64 instructions (only 2/3 bytes available = 128 bytes)
    #define ERASE_BLOCK_SIZE 1024		// Erase in blocks of 8 rows, 512 instructions (only 2/3 bytes available = 1024 bytes)

	// The pins
	#define InitIO()        {\
                            REMAP_PINS();\
							LED_INIT_PINS();\
                            ACCEL_INIT_PINS();\
							GYRO_INIT_PINS();\
							MAG_INIT_PINS();\
							ALT_INIT_PINS();\
							BT_INIT_OFF();\
							BATT_INIT_PINS();\
                            ANALOGUE_INIT_PINS();\
                            DRIVE_FLOATING_PINS(); /*Reduce leakage*/\
                            USB_BUS_SENSE_INIT();\
                            }

	// Cut down InitIO to save spave in bootloader
	#define InitIOBootloader()	{\
								/*  	  111111          */\
								/*		  5432109876543210*/\
								LATA =	0b1111110001110000;\
								TRISA =	0b1111110001110000;\
								LATB =	0b1011001101001100;\
								TRISB =	0b1011001101001100;\
								LATC =	0b1111111100100101;\
								TRISC =	0b1111111100100101;\
								ANALOGUE_INIT_PINS();\
							}	

	#define DRIVE_FLOATING_PINS() {\
                            TRISBbits.TRISB0 = 0; LATBbits.LATB0 = 0;/*PGD*/\
                            TRISBbits.TRISB1 = 0; LATBbits.LATB1 = 0;/*PGC*/\
							TRISBbits.TRISB4 = 0; LATBbits.LATB4 = 0;\
							TRISBbits.TRISB5 = 0; LATBbits.LATB5 = 0;\
							TRISBbits.TRISB7 = 0; LATBbits.LATB7 = 0;\
							TRISBbits.TRISB14 = 0; LATBbits.LATB14 = 0;\
							TRISAbits.TRISA0 = 0; LATAbits.LATA0 = 0;\
							TRISAbits.TRISA7 = 0; LATAbits.LATA7 = 0;\
							TRISBbits.TRISB4 = 0; LATBbits.LATB4 = 0;\
							TRISCbits.TRISC3 = 0; LATCbits.LATC3 = 0;\
							TRISCbits.TRISC4 = 0; LATCbits.LATC4 = 0;\
							TRISCbits.TRISC6 = 0; LATCbits.LATC6 = 0;\
							TRISCbits.TRISC7 = 0; LATCbits.LATC7 = 0;\
                            }
							
	// REMAP pins
	#define REMAP_PINS()	{\
							__builtin_write_OSCCONL(OSCCON & 0xBF);/*Unlock*/\
							/*INPUTS*/\
							RPINR0bits.INT1R = 24;/* AccInt1 RP24 -> Int1 */\
							RPINR8bits.IC4R = 21;/*  AccInt2 RP21 -> IC4 */\
							RPINR7bits.IC2R = 15;/*  GyroInt2 dtrdy RP15 -> IC2*/\
							RPINR8bits.IC3R = 25;/*  Mag int RP25 -> IC3*/\
							RPINR18bits.U1CTSR = 16;/*RP16 = U1CTS (BT_RTS)*/\
							RPINR1bits.INT2R = 16;  /*RP16 = INT2 (BT_RTS) */\
							RPINR18bits.U1RXR = 18;	/*RP18 = U1RX (BT_TX)*/\
							/*OUTPUTS*/\
							RPOR8bits.RP17R = 3;	/*RP17 = U1TX (BT_RX)*/\
							/*RPOR3bits.RP6R = 4;*/ 	/*RP6 = U1RTS (BT_CTS)*/\
							/*Then lock the PPS module*/\
							__builtin_write_OSCCONL(OSCCON | 0x40);\
							}						

   // Clock: initialize PLL for 96 MHz USB and 12MIPS
	#define CLOCK_PLL() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							CLKDIVbits.RCDIV = 0;					/*No FRC prescaler - otherwise it defaults to 4MHz on this device*/\
							CLKDIVbits.PLLEN = 1;					/*On this device the PLL does not engage on its own*/\
							OSCCONBITS_copy.NOSC = 0b001;			/*Internal FRC+PLL oscillator*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							while (OSCCONbits.OSWEN == 1);			/*Wait for switch over - doesn't care if stable or not*/\
							while (OSCCONbits.LOCK != 1); 			/*Wait for PLL lock to engage*/\
							}				

    // Clock: switch to 8 MHz INTOSC
	#define CLOCK_INTOSC() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							OSCCONBITS_copy.NOSC = 0b111;			/*Internal FRC oscillator + prescaler*/\
							OSCCONBITS_copy.OSWEN = 1;\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							asm("DISI #0");\
							while (OSCCONbits.OSWEN == 1);\
							CLKDIVbits.PLLEN = 0;\
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
							CLKDIVbits.PLLEN = 0;\
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
							CLKDIVbits.PLLEN = 0;\
							CLKDIVbits.RCDIV = 0b000; 				/*8 MHz */\
							}
       
	// Secondary oscillator on
	#define CLOCK_SOSCEN()	{/*No xtal on this HW*/}

    // LED
    #define LED_R_PIN           TRISAbits.TRISA8
    #define LED_R              	LATAbits.LATA8 
    #define LED_G_PIN  			TRISAbits.TRISA2        
    #define LED_G               LATAbits.LATA2
    #define LED_B_PIN           TRISAbits.TRISA3
    #define LED_B     			LATAbits.LATA3
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
    #define BATT_MON_PIN        TRISBbits.TRISB13
    #define BATT_INIT_PINS()   	BATT_MON_PIN = 1
	#define BATT_CHARGE_MIN_SAFE	560		// 3.28v
	#define BATT_CHARGE_FULL_USB	700		// 4.10v
	#define BATT_SMOOTHED					// Makes analogue drive smooth the batt level, recommended


	// Analogue select pins - 0 = Analogue
	#define ADC_SELECT_L		0b1111011111111111
	#define ANALOGUE_INIT_PINS()	{AD1PCFGL = ADC_SELECT_L;}


	// For the following peripherals the interface is via I2C using these pins
	// All the pins need re-mapping. See RemapPins() in this file.

	// Defines for myI2C.c
	#define USE_HW_I2C1
	#define MY_I2C_TIMEOUT 	65535
	#ifdef BOOTLOADER
		#define IGNORE_I2C_TIMOUTS
	#else
		//#define IGNORE_I2C_TIMOUTS 	// Uncomment if timeout checks are not required
		#define ENABLE_I2C_TIMOUT_CHECK	// Creates the default function in myi2c.c
		#define CHECK_TIMEOUT(_val)	myI2CCheckTimeout(_val)	// Using default, or assign a function to ensure val is never zero (i.e. a bus timeout has not occured) 
	#endif

	/*for SW code use : {mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}*/
	#define mySCL				LATBbits.LATB8
	#define mySCLd				TRISBbits.TRISB8	
	#define mySDAr				PORTBbits.RB9
	#define mySDAw				LATBbits.LATB9	
	#define mySDAd				TRISBbits.TRISB9

	// If a bus lockup occurs the following will clear it
	#define myI2C1clear()		{{mySCLd=1;mySDAd=1;}unsigned char i;mySCLd=0;for(i=10;i;i--){mySCL=0;Delay10us(1);mySCL=1;Delay10us(1);}mySCLd=1;}

	/*for SW code use : {mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}*/
	#define mySCL2				LATBbits.LATB3
	#define mySCL2d				TRISBbits.TRISB3	
	#define mySDA2r				PORTBbits.RB2
	#define mySDA2w				LATBbits.LATB2	
	#define mySDA2d				TRISBbits.TRISB2

	// If a bus lockup occurs the following will clear it
	#define myI2C2clear()		{{mySCL2d=1;mySDA2d=1;}unsigned char i;mySCL2d=0;for(i=10;i;i--){mySCL2=0;Delay10us(1);mySCL2=1;Delay10us(1);}mySCL2d=1;}

	// Clear buss'
	#define myI2Cclear() 		{myI2C1clear();myI2C2clear();}
	

    // Accelerometer MMA8451Q
	#define ACCEL_MMA8451Q
	#define ACCEL_I2C_MODE
	#define ACCEL_FIFO_WATERMARK 25
    #define ACCEL_INT1_PIN      TRISCbits.TRISC5 
    #define ACCEL_INT2_PIN      TRISCbits.TRISC8 	
    #define ACCEL_INT1          PORTCbits.RC5
    #define ACCEL_INT2          PORTCbits.RC8 

#if 1
	#define ACCEL_INT2_VECTOR	_IC4Interrupt
	#define ACCEL_INT2_IF		IFS2bits.IC4IF
	#define ACCEL_INT2_IP		IPC9bits.IC4IP
	#define ACCEL_INT2_IE		IEC2bits.IC4IE
#else
	#define ACCEL_INT2_VECTOR	_INT2Interrupt
   	#define ACCEL_INT2_IP       IPC7bits.INT2IP 
    #define ACCEL_INT2_IF       IFS1bits.INT2IF 
    #define ACCEL_INT2_IE       IEC1bits.INT2IE 
#endif
	#define ACCEL_INT1_VECTOR	_INT1Interrupt
   	#define ACCEL_INT1_IP       IPC5bits.INT1IP 
    #define ACCEL_INT1_IF       IFS1bits.INT1IF 
    #define ACCEL_INT1_IE     	IEC1bits.INT1IE
    #define ACCEL_INIT_PINS()  	{IC4CON1 = 7;\
								ACCEL_INT1_PIN	= 1;\
								ACCEL_INT2_PIN	= 1;\
								}
								
	// Gyro L3G4200
	#define USE_GYRO
	#define GYRO_IS_L3G4200D
	#define GYRO_I2C_MODE
	#define GYRO_FIFO_WATERMARK	25
    //#define GYRO_INT1_PIN     	  
    #define GYRO_INT2_PIN     	TRISBbits.TRISB15   
    //#define GYRO_INT1        	  
    #define GYRO_INT2 			PORTBbits.RB15 
	//#define GYRO_INT1_IP       
	//#define GYRO_INT1_IF       
	//#define GYRO_INT1_IE        
    #define GYRO_INT2_IP        IPC1bits.IC2IP    
    #define GYRO_INT2_IF        IFS0bits.IC2IF 
    #define GYRO_INT2_IE        IEC0bits.IC2IE  

    #define GYRO_INIT_PINS()  	{IC2CON1 = 7;GYRO_INT2_PIN	= 1;}
	
	// Several devices can use the change notification interrups
	#define CN_INTS_ON()	{IFS1bits.CNIF = 0;IPC4bits.CNIP = CN_INTS_PROCESSOR_PRIORITY; IEC1bits.CNIE = 1;}
	#define CN_INTS_OFF()	{IEC1bits.CNIE = 0;}
	#define CN_INT_FIRE()	{IFS1bits.CNIF = 1;}
	
	// Magnetometer MAG3110 
	#define MAG_I2C_MODE
	#define MAG_ZERO_ON_POWER_UP
	#define MAG_INT_VECTOR		_IC3Interrupt
	#define MAG_INT_PIN     	TRISCbits.TRISC9  
    #define MAG_INT        		PORTCbits.RC9    
	#define MAG_INT_IF 			IFS2bits.IC3IF
	#define MAG_INT_IE  		IEC2bits.IC3IE
	#define MAG_INT_IP			IPC9bits.IC3IP
    #define MAG_INIT_PINS()  	{IC3CON1 = 7;MAG_INT_PIN = 1;MAG_INT_IP=MAG_INT_PRIORITY;}
	#define MAG_ENABLE_INTS()	{MAG_INT_IE=1;}
	#define MAG_DISABLE_INTS()	{MAG_INT_IE=0;}
								
	// Altimeter BMP180 on bus 2
	#define ALT_ALTERNATE_I2C	2
	#define ALT_IGNORE_EOC
	#define USE_CTRL_REG_BUSY_BIT
    #define ALT_INIT_PINS()  	{}
	
	// Bluetooth module test PAN1315 
	#define BT_EN_PIN		TRISAbits.TRISA9
	#define BT_EN			LATAbits.LATA9
	#define BT_CTS_PIN 		TRISAbits.TRISA1
	#define BT_CTS			LATAbits.LATA1
	#define BT_RTS_PIN		TRISCbits.TRISC0
	#define BT_RTS			PORTCbits.RC0
	#define BT_RX_PIN		TRISCbits.TRISC1
	#define BT_TX_PIN		TRISCbits.TRISC2
	
	#define BT_INIT_PINS()	{/*Enable eHCILL interrupt registers*/\
							BT_EN_PIN=0;BT_EN=0;\
							BT_CTS_PIN=0;BT_CTS=1;\
							BT_RTS_PIN=1;\
							BT_RX_PIN=0;\
							BT_TX_PIN=1;}

	#define BT_INIT_OFF()	{/*Disable eHCILL interrupt registers*/\
							BT_EN_PIN=0;BT_EN=0;\
							BT_CTS_PIN=1;\
							BT_RTS_PIN=1;\
							BT_RX_PIN=1;\
							BT_TX_PIN=1;}

	// BT Clock 
	#define STOP_BT_SLW_CLK()	{}
	#define SETUP_BT_SLW_CLK()	{}	

	// Defines for bleStack
	#define BT_UART_DEFAULT_BAUD 	115200UL
 	#define BT_UART_RTS				BT_CTS
	#define BT_UART_USES_UART_1

	// BT eHCILL signaling support
	#define BT_eHCILL_HANDLER_VECTOR	_INT2Interrupt
   	#define BT_eHCILL_HANDLER_PRIORITY  IPC7bits.INT2IP 
    #define BT_eHCILL_HANDLER_FLAG      IFS1bits.INT2IF 
    #define BT_eHCILL_HANDLER_ENABLE    IEC1bits.INT2IE 

	// BT event trigger support
	#define BT_EVENT_HANDLER_VECTOR		_IC5Interrupt
	#define BT_EVENT_HANDLER_FLAG		IFS2bits.IC5IF
	#define BT_EVENT_HANDLER_IPL		IPC9bits.IC5IP
	#define BT_EVENT_HANDLER_ENABLE		IEC2bits.IC5IE
	
	#define BT_EVENT_HANDLER_ARM()		{IC5CON1 = 7;/*not remapped*/\
										BT_EVENT_HANDLER_IPL = BT_EVENT_HANDLER_PRIORITY;\
										BT_EVENT_HANDLER_ENABLE = 1;}
	#define BT_EVENT_HANDLER_DISARM()	{IC5CON1 = 0;\
										BT_EVENT_HANDLER_ENABLE = 0;}
										
	// EEprom on I2C bus 2
	#define EEROM_ALTERNATE_I2C	2
	#define EEPROM_SIZE_BYTES	0x10000UL
	#define EEPROM_ADDRESS		0xA0
	#define WRITE_PAGE_SIZE		0x80

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

	
	// Legacy
	// Setup the VDMA module - btstack only
	#define VDMA_DEFAULT_BAUD 	115200UL
 	#define VDMA_RTS			BT_CTS
	#define VDMA_CALLBACK_ON_COMPLETE
	#define VDMA_USES_UART_1
	#define VDMA_RX_INT_PRIORITY	BT_UART_RX_INT_PRIORITY
	#define VDMA_TX_INT_PRIORITY	BT_UART_TX_INT_PRIORITY

#endif

