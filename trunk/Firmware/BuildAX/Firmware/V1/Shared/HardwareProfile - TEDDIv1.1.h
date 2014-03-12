/* 
 * Copyright (c) 2012, Newcastle University, UK.
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

// KL 12-03-2012
// Hardware profile for TEDDI1.1

#ifndef HARDWAREPROFILE_TEDDI_H
#define HARDWAREPROFILE_TEDDI_H
    
    #include <Compiler.h>
    
    #ifndef __C30__
        #warning "Unexpected compiler"
    #endif
        
    #ifndef __PIC24FJ256GB106__
        #warning "Unexpected device selection"
    #endif
    
    #if !defined(SENSOR) && !defined(ROUTER) && !defined(BOOTLOADER)
        #warning "Unexpected project configuration"
    #endif

    #define HARDWARE_VERSION 0x11
    #define SOFTWARE_VERSION 3			// 2 = first release, 3 = fixed router neighbour table

	#define ADC_INT_PRIORITY 	7
	#define FIFO_INTERRUPT_PRIORITY	ADC_INT_PRIORITY
    #define RTC_INT_PRIORITY	6
    #define T1_INT_PRIORITY		RTC_INT_PRIORITY
	#define CN_INT_PRIORITY		4


    //SystemPwrSave arguments OR together
    #define WAKE_ON_USB		            1
    #define WAKE_ON_ADC		            2
    #define WAKE_ON_BUTTON	            4
    #define WAKE_ON_WDT		            8
    #define WAKE_ON_RTC		            16
    #define WAKE_ON_TIMER1	            32
    #define LOWER_PWR_SLOWER_WAKE	    64
    #define ADC_POWER_DOWN			    128
    #define SAVE_INT_STATUS			    256
    #define ALLOW_VECTOR_ON_WAKE	    512
    #define LEAVE_WDT_RUNNING		    1024
    #define DONT_RESTORE_PERIPHERALS	2048
	#define LEAVE_ON_ANALOGUE			4096
	#define LEAVE_ON_RF					8192	
	#define LEAVE_ON_TEMP				16384
	#define LEAVE_ON_LIGHT				32768
	#define LEAVE_ON_LED				65536

    
    // Essential Functions in HardwareProfile.c
    extern void WaitForPrecharge(void);
    extern void SystemPwrSave(unsigned long NapSetting);
	extern void SystemPowerOff(unsigned long leave_on);
	extern unsigned int GetClockFreqUsingRTC (void);
	void CheckLowPowerCondition(void);

    // ROM storage (non-PSV data address access)
    #define ROM_ADDRESS(_v) (((unsigned long) __builtin_tblpage(_v) << 16) + __builtin_tbloffset(_v))
    #define WRITE_BLOCK_SIZE 128		    // Write in rows of 64 instructions (only 2/3 bytes available = 128 bytes)
    #define ERASE_BLOCK_SIZE 1024		    // Erase in blocks of 8 rows, 512 instructions (only 2/3 bytes available = 1024 bytes)
    // These addresses must match the reserved sections of program memory in the linker script
    #define SETTINGS_ADDRESS      0x29800ul	// 0x9C00ul
    #define LOG_ADDRESS           0x29C00ul	// 0xA000ul
    #define DEVICE_ID_ADDRESS     0x2A000ul	// 0xA400ul
	
	#define ProgramMemStopNoConfigs SETTINGS_ADDRESS // [Bootloader] Must be an instruction word aligned address.  This address does not get updated, but the one just below it does 
	
	
    extern char ReadProgram(unsigned long address, void *buffer, unsigned short length);           // Read from program memory (faster if even address and length)
    extern char WriteProgramPage(unsigned long pageAddress, void *buffer, unsigned short length);  // Write to program memory (must be a page-aligned address; if length is odd, one extra byte of junk will be written)

    //#define CHANNEL_MASK 0x07FFF800 //0b00000222222211111111100000000000 // 0x02000000
    //                                         6543210987654321          
    //#define CHANNEL_MASK              0b00000010000100001000000000000000 // Ch 15, 20, 25
	// [______________]111111111111111100000000000 // Ch 11-26
	
	//                             2222222111111111
	//                             6543210987654321
	#define CHANNEL_MASK_DEFAULT 0b0100001000010000		// Ch 15, 20, 25
	#define CHANNEL_MASK ((unsigned long)settings.channelmask << 11)
    
    
	// Remove noise in PIR on the samples with the light sensor enabled
    #define REMOVE_PIR_NOISE
    
    // Default audio gain
    #define DEFAULT_AUDIO_GAIN 10
    
      
	// USB CDC
	#define OWN_CDC_BUFFER_SIZES
	#define IN_BUFFER_CAPACITY 128
	#define OUT_BUFFER_CAPACITY 128
    
    
    #define 	PIC24


	// Used for delays in TimeDelay.h
	#if defined(RADIO_PLL) || defined(BOOTLOADER)
		#define CLOCK_FREQ 32000000ul
		#define GetInstructionClock()  16000000ul
	#else
		#define CLOCK_FREQ 8000000ul
		#define GetInstructionClock()  ((OSCCONbits.COSC==1)? 16000000ul : 4000000ul)
	#endif
	//#define GetInstructionClock()  ((OSCCONbits.COSC==1)? 16000000ul : 4000000ul)


    // USB bus sense I/O
    // #define USE_USB_BUS_SENSE_IO
    #define USB_BUS_SENSE_PIN   TRISEbits.TRISE4
    #define USB_BUS_SENSE       PORTEbits.RE4
	#define USB_BUS_SENSE_INIT() {USB_BUS_SENSE_PIN=1;}
	// Int2 fires on USB detect
	#define USB_BUS_SENSE_INIT_INT()	{/*Int on change- CN62*/\					
										CNEN4bits.CN62IE = 1;\
										IFS1bits.CNIF = 0;/*Flag*/\
										IEC1bits.CNIE = 1;/*Unmask*/\
										IPC4bits.CNIP = CN_INT_PRIORITY;/*Priority*/}

	// Button
	#define BUTTON_INIT_PINS()	{TRISBbits.TRISB1 = 1;}
	#define BUTTON_INIT_INT()	{/*Int on change- CN2*/\					
									CNEN1bits.CN3IE = 1;\
									IFS1bits.CNIF = 0;/*Flag*/\
									IEC1bits.CNIE = 1;/*Unmask*/\
									IPC4bits.CNIP = CN_INT_PRIORITY;/*Priority*/}
    #define BUTTON PORTBbits.RB1 // active low

	// The pins
	#define InitIO(){\
					REMAP_PINS();\
					USB_BUS_SENSE_INIT();\
					BUTTON_INIT_PINS();\
					LED_INIT_PINS();\
					TEMP_INIT_PINS();\
					LDR_INIT_PINS();\
					ADC_INIT_PINS();\
					AUDIO_INIT_PINS();\
					INIT_POWER_CTRL_PINS_OFF();\
					DriveFloatingPins(); /*Reduce leakage*/\
					RF_INIT_PINS_OFF();\
					}		

	#define DriveFloatingPins() {\
								TRISBbits.TRISB11 = LATBbits.LATB11 = 0;\
								TRISBbits.TRISB12 = LATBbits.LATB12 = 0;\
								TRISBbits.TRISB13 = LATBbits.LATB13 = 0;\
								TRISBbits.TRISB15 = LATBbits.LATB15 = 0;\
								TRISCbits.TRISC12 = LATCbits.LATC12 = 0;\
								TRISCbits.TRISC15 = LATCbits.LATC15 = 0;\
								TRISDbits.TRISD0 = LATDbits.LATD0 = 0;\
								TRISDbits.TRISD7 = LATDbits.LATD7 = 0;\
								TRISDbits.TRISD8 = LATDbits.LATD8 = 0;\
								TRISDbits.TRISD11 = LATDbits.LATD11 = 0;\
								TRISFbits.TRISF3 = LATFbits.LATF3 = 0;\
								TRISFbits.TRISF4 = LATFbits.LATF4 = 0;\
								TRISFbits.TRISF5 = LATFbits.LATF5 = 0;\
								TRISFbits.TRISF4 = LATFbits.LATF4 = 0;\
								}
	
	// Power control pins
    #define POWER_ON  0
    #define POWER_OFF 1
	#define MRF_POWER_PIN	TRISFbits.TRISF1
	#define nMRF_POWER		LATFbits.LATF1
	#define AUDIO_POWER_PIN	TRISGbits.TRISG6
	#define nAUDIO_POWER	LATGbits.LATG6
	#define PIR_POWER_PIN	TRISEbits.TRISE3
	#define nPIR_POWER		LATEbits.LATE3
	#define INIT_POWER_CTRL_PINS_OFF()	{\
										nAUDIO_POWER	= 1;\
										AUDIO_POWER_PIN	= 0;\
										nMRF_POWER		= 1;\
										MRF_POWER_PIN	= 0;\
										nPIR_POWER		= 1;\
										PIR_POWER_PIN 	= 0;\
										}								
										
					
	// REMAP pins
	#define REMAP_PINS()		{\
							__builtin_write_OSCCONL(OSCCON & 0xBF);/*Unlock*/\
							RPINR0bits.INT1R = 22;  /* RP22/MINT -> Int1 */\
							RPINR20bits.SCK1R= 23;  /* MSCK RP23 -> SCK1i */\
							RPINR20bits.SDI1R= 25;   /* MSDO RP25 -> SDI1 */\
							RPOR11bits.RP23R = 8;    /* MSCK RP23 -> SCK1o */\
							RPOR10bits.RP20R = 7;     /* MSDI RP20 -> SDO1 */\
							/*Then lock the PPS module*/\
							__builtin_write_OSCCONL(OSCCON | 0x40);\
							}						


    // Clock: initialize PLL for 96 MHz USB and 12MIPS
	#define CLOCK_PLL() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							asm("DISI #0x3FFF");\
							CLKDIVbits.RCDIV = 0;					/*No FRC prescaler - otherwise it defaults to 4MHz on this device*/\
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
							CLKDIVbits.RCDIV = 0b000; 				/**/\
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
							CLKDIVbits.RCDIV = 0b000; 				/**/\
							}
       
	// Secondary oscillator on
	#define CLOCK_SOSCEN()	{OSCCONbits.SOSCEN=1;}

    // LED
    #define LED_R_PIN           TRISEbits.TRISE1
    #define LED_R              	LATEbits.LATE1 
    #define LED_G_PIN  			TRISEbits.TRISE2        
    #define LED_G               LATEbits.LATE2
    #define LED_B_PIN           TRISEbits.TRISE0
    #define LED_B     			LATEbits.LATE0
	#define LED 				LED_R          
    #define LED_INIT_PINS()     {LED_R_PIN = LED_R = LED_G_PIN = LED_G = LED_B_PIN = LED_B = 0;}

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
    
    // LED current value
    #define LED_CURRENT()       ((LED_R << 2) | (LED_G << 1) | LED_B)


    // Battery AN5 / B5
    #define BATT_MON_PIN        TRISBbits.TRISB5
    #define BATT_INIT_PINS()   	BATT_MON_PIN = 1
	// 5% battery level - avoids unwanted RTC resets
	#define BATT_CHARGE_MIN_SAFE         430
    #define BATT_CHARGE_ZERO		     409
    #define BATT_CHARGE_FULL		     614
    
    // LDR AN4 / B4
    #define LDR_EN_PIN			TRISEbits.TRISE5         
    #define LDR_EN              LATEbits.LATE5
    #define LDR_OUT_PIN         TRISBbits.TRISB4
    #define LDR_ENABLE()        {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 0;}
    #define LDR_DISABLE()       {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 1;}
    #define LDR_INIT_PINS()     LDR_DISABLE()

	// Temp sensor AN3 / B3
	#define USE_MCP9700
	#define TEMP_EN_PIN			TRISEbits.TRISE6
	#define TEMP_EN				LATEbits.LATE6
	#define TEMP_OUT_PIN		TRISBbits.TRISB3
	#define TEMP_ENABLE()		{TEMP_OUT_PIN=1;TEMP_EN_PIN=0;TEMP_EN=1;}
	#define TEMP_DISABLE()		{TEMP_OUT_PIN=1;TEMP_EN_PIN=0;TEMP_EN=0;}
	#define TEMP_INIT_PINS()	TEMP_DISABLE()
	
	// Microphone
	#define AUDIO_RESET_PIN		TRISEbits.TRISE7
	#define AUDIO_RESET			LATEbits.LATE7
	#define AUDIO_GAINx10_PIN	TRISGbits.TRISG8
	#define AUDIO_GAINx10		LATGbits.LATG8
	#define AUDIO_GAINx100_PIN	TRISGbits.TRISG9
	#define AUDIO_GAINx100		LATGbits.LATG9
	#define AUDIO_INIT_PINS()	{AUDIO_RESET_PIN=AUDIO_RESET=0;\
								AUDIO_GAINx10_PIN = AUDIO_GAINx10 = 0;\
								AUDIO_GAINx100_PIN = AUDIO_GAINx100 = 0;}
	#define AUDIO_GAIN_1()		{AUDIO_GAINx10 = 0; AUDIO_GAINx100 = 0;}	
	#define AUDIO_GAIN_10()		{AUDIO_GAINx10 = 1; AUDIO_GAINx100 = 0;}
	#define AUDIO_GAIN_100()	{AUDIO_GAINx10 = 0; AUDIO_GAINx100 = 1;}

	// Analogue select pins - 0 = Analogue, Vref=Ana' too!
	/*
	Vref        AN0
	PIR 		AN2
	Temperature AN3
	Light 		AN4
	Battery 	AN5
	MIC ave		AN8
	MIC peak	AN9
	MIC raw		AN10
	*/

	// Used for conversion to voltaage
	#define VREF_USED_MV 2500

	#define ADC_SELECT_L 0b1111100011000010
	#define ADC_SELECT_H 0b1111111111111111
	#define ADC_INIT_PINS()	{AD1PCFG = ADC_SELECT_L;}

	#define ADC_INDEX_VREF  	- /*Don't sample this, it is always 1023*/
	#define ADC_INDEX_PIR   	0
	#define ADC_INDEX_TEMP  	1
	#define ADC_INDEX_LIGHT   	2
	#define ADC_INDEX_BATT  	3
	#define ADC_INDEX_MIC_AVE	4
	#define ADC_INDEX_MIC_PEAK  5
	#define ADC_INDEX_MIC_RAW   6
	
 	// MRF module
    #define RF_INT_PIN          PORTDbits.RD3
    #define RF_INT_TRIS         TRISDbits.TRISD3
    #define PHY_CS              LATFbits.LATF0
    #define PHY_CS_TRIS         TRISFbits.TRISF0
    #define PHY_RESETn          LATDbits.LATD1
    #define PHY_RESETn_TRIS     TRISDbits.TRISD1
    #define PHY_WAKE            LATDbits.LATD6
    #define PHY_WAKE_TRIS       TRISDbits.TRISD6
    #define RF_USE_SPI          1
    #define RF_SPI_SI_TRIS      TRISDbits.TRISD4
   	#define RF_SPI_SDI          PORTDbits.RD4
    #define RF_SPI_SO_TRIS      TRISDbits.TRISD5
    #define RF_SPI_SDO          LATDbits.LATD5 
    #define RF_SPI_SCK_TRIS     TRISDbits.TRISD2
    #define RF_SPI_SCK          LATDbits.LATD2 
    // Miwi Stack integration
   	#define SPI_SDI             RF_SPI_SDI
    #define SDI_TRIS            RF_SPI_SI_TRIS
    #define SPI_SDO             RF_SPI_SDO 
    #define SDO_TRIS            RF_SPI_SO_TRIS
    #define SPI_SCK             RF_SPI_SCK 
    #define SCK_TRIS            RF_SPI_SCK_TRIS

    #define TMRL                TMR2
    #define RFIF                IFS1bits.INT1IF
    #define RFIE                IEC1bits.INT1IE
    #define RF_IEDG  			INTCON2bits.INT1EP

    #define RF_INIT_PINS()    {\
                                RF_SPI_SI_TRIS = 1;\
                                RF_SPI_SO_TRIS = 0;\
                                RF_SPI_SCK_TRIS = 0;\
                                PHY_CS = 1;\
                                PHY_CS_TRIS = 0;\
                                PHY_RESETn = 0;\
                                PHY_RESETn_TRIS = 0;\
                                PHY_WAKE = 1;\
                                PHY_WAKE_TRIS = 0;\
                                RFIF = 0;\
                                RF_INT_TRIS = 1;\
								RF_IEDG = 1; /*FALLING EDGE*/ \
                            }

    #define RF_INIT_PINS_OFF()    { /*Can't drive pins high if module is off or it will draw excessive current*/\
                                RF_SPI_SI_TRIS = 1;\
                                RF_INT_TRIS = 1;\
                                RF_SPI_SO_TRIS = 0;RF_SPI_SDO=0;\
                                RF_SPI_SCK_TRIS = 0;RF_SPI_SCK=0;\
                                PHY_CS_TRIS = 0;PHY_CS = 0;\
                                PHY_RESETn_TRIS = 0;PHY_RESETn = 0;\
                                PHY_WAKE_TRIS = 0;PHY_WAKE = 0;\
								nMRF_POWER = 1; /* Start off (1) */ \
								MRF_POWER_PIN = 0;\
                            }
                            
	#define MRF_POWER_OFF()	{ \
								nMRF_POWER = 1;	   /* 1 = off */ \
								MRF_POWER_PIN = 0; /* ensure set as output */ \
							}
							
	// Slow, pulsed start code prevents brown-outs
	#define MRF_POWER_ON()	{ \
								nMRF_POWER = 1;	MRF_POWER_PIN = 0; /* Start off (1), ensure set as output */ \
								nMRF_POWER = 0; nMRF_POWER = 1; DelayMs(1); \
								nMRF_POWER = 0; nMRF_POWER = 1; DelayMs(1); \
								nMRF_POWER = 0; nMRF_POWER = 1; DelayMs(1); \
								nMRF_POWER = 0; nMRF_POWER = 1; DelayMs(1); \
								nMRF_POWER = 0; nMRF_POWER = 1; DelayMs(1); \
								nMRF_POWER = 0; nMRF_POWER = 1; DelayMs(1); \
								nMRF_POWER = 0; nMRF_POWER = 1; \
								nMRF_POWER = 0; /* Leave on (0) */ \
							}
							

//    #define USE_AND_OR	/* for SPI */
//    #include <spi.h>
    #define RF_SPI_OPEN(){  \
                        SPI1CON1 = 0x013E;\
                        /* SPI1CON2 = DONT WRITE THIS*/ ;\
                        SPI1STAT = 0x8000;\
                        }
                    
	// Defines for myI2C.c
	//#define USE_HW_I2C
	#define USE_HW_I2C  // was USE_HW_I2C1
	#define I2C_STATbits_ACKSTAT	I2C1STATbits.ACKSTAT
	#define MY_I2C_TIMEOUT 	65535
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

// Legacy includes for MCHIP stacks
 
    // USB (MCHPFSUSB framework)
    //#undef USE_SELF_POWER_SENSE_IO
    //#define tris_self_power     TRIS?bits.TRIS??    // Input
    //#define self_power          PORT?bits.R??
    #define self_power          0   // 0 = bus-powered, 1 = self-powered

	#define usb_bus_sense 		USB_BUS_SENSE

// Really unused stuff follows - just junk really
	#define mInitAllLEDs()      LED_INIT_PINS()
    
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


    // Test interface
    #define PUSH_BUTTON_1 BUTTON
    //#define PUSH_BUTTON_2 "error, not defined"
    #define LED_1 LED_R
    #define LED_2 LED_G

#endif
