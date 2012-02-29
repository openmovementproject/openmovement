// Hardware profile for the FridgeMon1.0 (using PIC24FJ256GB106)
// Karim Ladha, Dan Jackson, 2011-2012

#ifndef HARDWAREPROFILE_FridgeMon1_0_H
#define HARDWAREPROFILE_FridgeMon1_0_H

    #ifndef __PIC24FJ256GB106__
    //    #error "Unexpected device selection"
    #endif

    #ifndef __C30__
    //    #error "Unexpected compiler"
    #endif

    #define 	PIC24
    #define 	FMON1_0
    #define 	HARDWARE_VERSION 0x01           // BCD format (for USB response)
	#define 	SOFTWARE_VERSION 0x01
	#define 	DEFAULT_DEBUGGING 3				// 1=delayed activation, 2=on-tap, 3=always

	// Interrupt priority levels
	#define RTC_INT_PRIORITY				6 
	#define T1_INT_PRIORITY					RTC_INT_PRIORITY 	/*Prevents ms rollover during reads*/
	#define ADC_INT_PRIORITY				5 /*This is very fast*/
	#define FIFO_INTERRUPT_PRIORITY 		5
	#define GLOBAL_I2C_PROCESSOR_PRIORITY	4
	#define DATA_STREAM_INTERRUPT_PRIORITY 	GLOBAL_I2C_PROCESSOR_PRIORITY
	#define ACCEL_INT_PRIORITY				GLOBAL_I2C_PROCESSOR_PRIORITY

	// SW WDT
	#define RTC_SWWDT_TIMEOUT 600

//SystemPwrSave arguments OR together
#define WAKE_ON_USB				0x1
#define WAKE_ON_ADXL1			0x2	
#define WAKE_ON_ADXL2			0x4
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

// Essential Functions in HardwareProfile.c
extern void WaitForPrecharge(void);
extern void SystemPwrSave(unsigned long NapSetting);

	// Used for delays in TimeDelay.h
	#define GetInstructionClock()  4000000

    // USB bus sense I/O
    // #define USE_USB_BUS_SENSE_IO
	#define USB_BUS_SENSE_PIN		TRISFbits.TRISF5
    #define USB_BUS_SENSE       	PORTFbits.RF5
	#define USB_BUS_SENSE_INIT()	USB_BUS_SENSE_PIN=1
	// Change notification int,  vectors to button handler
	#define USB_BUS_SENSE_INT_ENABLE()	{/*New USB int is remappable -> INT3*/\
									IFS3bits.INT3IF = 0;/*Flag*/\
									IEC3bits.INT3IE = 1;/*Unmask*/\
									}
    // MCHPFSUSB frameworkc self-power flag
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
                            ACCEL_INIT_PINS();\
                            LED_INIT_PINS();\
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
                            TRISFbits.TRISF1 = 0; LATFbits.LATF1 = 0;\
                            TRISFbits.TRISF4 = 0; LATFbits.LATF4 = 0;\
                            TRISFbits.TRISF3 = 0; LATFbits.LATF3 = 0;\
                            TRISCbits.TRISC12 = 0; LATCbits.LATC12 = 0;\
							TRISBbits.TRISB3 = 0; LATBbits.LATB3 = 0;\
							TRISBbits.TRISB4 = 0; LATBbits.LATB4 = 0;\
							TRISBbits.TRISB5 = 0; LATBbits.LATB5 = 0;\
							TRISBbits.TRISB11 = 0; LATBbits.LATB11 = 0;\
							TRISBbits.TRISB13 = 0; LATBbits.LATB13 = 0;\
							TRISBbits.TRISB14 = 0; LATBbits.LATB14 = 0;\
                            }
							
	// REMAP pins
	#define REMAP_PINS()	{\
							__builtin_write_OSCCONL(OSCCON & 0xBF);/*Unlock*/\
							RPINR0bits.INT1R = 12;/* AccInt1 RP11 -> Int1 */\
							RPINR1bits.INT2R = 11;/* AccInt2 RP12 -> Int2 */\
							RPINR1bits.INT3R = 17;/* USB det RP17 -> Int3 */\
							/*Then lock the PPS module*/\
							__builtin_write_OSCCONL(OSCCON | 0x40);\
							}						

    // Clock: initialize PLL for 96 MHz USB and 16MIPS
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
    #define LED_R_PIN           TRISBbits.TRISB8
    #define LED_R              	LATBbits.LATB8 
    #define LED_G_PIN  			TRISBbits.TRISB9        
    #define LED_G               LATBbits.LATB9
    #define LED_B_PIN           TRISBbits.TRISB7
    #define LED_B     			LATBbits.LATB7
    #define LED_INIT_PINS()     {LED_R_PIN = LED_R = 0;LED_G_PIN = LED_G = 0;LED_B_PIN = LED_B = 0;}

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


    // Battery - AN15
    #define BATT_MON_PIN        TRISBbits.TRISB15
    #define BATT_INIT_PINS()   	BATT_MON_PIN = 1
    #define BATT_CHARGE_ZERO		614		// FIXED CONST, DO NOT USE
	#define BATT_CHARGE_FULL		708		// FIXED CONST, DO NOT USE
	#define BATT_CHARGE_MIN_LOG		517		// minimum level to update log
	#define BATT_CHARGE_MIN_SAFE	520		// minimum safe running voltage (5%), avoids unwanted RTC resets and file corruptions etc.
    #define BATT_CHARGE_MID_USB		666     // level to count towards recharge cycle counter (75%)
    #define BATT_CHARGE_FULL_USB	708


	// Temperature sensor : Not present but these are in analogue.c
	#define TEMP_ENABLE()	;
	#define TEMP_DISABLE()	;
	#define USE_MCP9700

    // LDR - AN2
    #define LDR_EN_PIN			TRISBbits.TRISB6         
    #define LDR_EN              LATBbits.LATB6
    #define LDR_OUT_PIN         TRISBbits.TRISB2
    #define LDR_ENABLE()        {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 0;}
    #define LDR_DISABLE()       {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 1;}
    #define LDR_INIT_PINS()     LDR_DISABLE()

	// ADC select pins for CWA 1.7 (0 = Analog)
	#define ADC_SELECT_L 		0b0111111111111011
	#define ADC_SELECT_H 		0b1111111111111101 // Note: enabled sample of bandgap ref to make up for missing temp channel (total = 3 samples)
	#define ADC_INIT_PINS()		{AD1PCFGL = ADC_SELECT_L;AD1PCFGH = ADC_SELECT_H;}
    #define ADC_INDEX_BATT		1
    #define ADC_INDEX_LDR 		0
    #define ADC_INDEX_TEMP		2

	/* I2C bus has MCP3421, MCP9800 and MMA8451 on it*/

	// Defines for myI2C.c
	//#define USE_MY_SW_I2C
	#define USE_HW_I2C

	#define InitI2C()			{myI2Cclear();mySCLd = 1;mySDAd=1;I2C1CON=0;myI2CIdle();}
	/*for SW code use : {mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}*/
	#define mySCL				LATDbits.LATD10
	#define mySCLd				TRISDbits.TRISD10	
	#define mySDAr				PORTDbits.RD9
	#define mySDAw				LATDbits.LATD9	
	#define mySDAd				TRISDbits.TRISD9
	// If a bus lockup occurs the following will clear it
	#define myI2Cclear()		{unsigned char i;mySCLd=0;for(i=10;i;i--){mySCL=0;Delay10us(1);mySCL=1;Delay10us(1);}}
	

    // Accelerometer MMA8451Q
	#define ACCEL_MMA8451Q
	#define ACCEL_I2C_MODE
	#define ACCEL_FIFO_WATERMARK 25
    #define ACCEL_INT1_PIN      TRISDbits.TRISD11 
    #define ACCEL_INT2_PIN      TRISDbits.TRISD0 	
    #define ACCEL_INT1          PORTDbits.RD11
    #define ACCEL_INT2          PORTDbits.RD0 
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
								
    // Flash Memory - Hynix/Micron
    #define NAND_DEVICE     NAND_DEVICE_HY27UF082G2B

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
    #define FLASH_CE1_PIN  	TRISDbits.TRISD2  
    #define FLASH_CE1		LATDbits.LATD2
    #define FLASH_CE2_PIN  	TRISDbits.TRISD3  
    #define FLASH_CE2		LATDbits.LATD3                
    #define FLASH_CLE_PIN 	TRISDbits.TRISD6  
    #define FLASH_CLE		LATDbits.LATD6       
    #define FLASH_ALE_PIN 	TRISDbits.TRISD7   
    #define FLASH_ALE		LATDbits.LATD7      
    #define FLASH_RB_PIN 	TRISDbits.TRISD1    
    #define FLASH_RB		PORTDbits.RD1 
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

