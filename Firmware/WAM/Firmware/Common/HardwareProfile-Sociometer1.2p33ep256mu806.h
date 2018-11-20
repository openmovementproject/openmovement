
#ifndef _HARDWAREPROFILE_
#define _HARDWAREPROFILE_

// SW defines
#define HARDWARE_VERSION 12
#define WAV_NO_READ
#define BASIC_DISPLAY_FONTS
//#define WRITE_LOG
#ifdef WRITE_LOG
#warning "Something strange going on meaning writing to a log file corrupts WAV files (e.g. no headers) - something funny in FSIO/FsUtils? Does logging OMX cause problems?"
#endif
//#define ACCEL_SWAP_INTERRUPTS
//#define HAS_PROX


	// Interrupt priority levels

	// Highest - RTC, TIMER tasks
	#define RTC_INT_PRIORITY				6 
	#define T1_INT_PRIORITY					FIFO_INTERRUPT_PRIORITY 	/*Prevents ms rollover during reads*/

	// Mid level - FIFO time stamps only, safe for rtc to interrupt "DataStreamUpdateTimestamp()"
	#define FIFO_INTERRUPT_PRIORITY 		5
	#define CN_INTS_PROCESSOR_PRIORITY 		FIFO_INTERRUPT_PRIORITY
	#define ACCEL_INT_PRIORITY				FIFO_INTERRUPT_PRIORITY
	#define GYRO_INT_PRIORITY				FIFO_INTERRUPT_PRIORITY

	// Low level - Grabbing data from devices, I2C comms, ADC sample wait etc, "FifoContiguousSpaces()" ,"FifoExternallyAdded()", etc...
	#define GLOBAL_I2C_PROCESSOR_PRIORITY	4
	#define DATA_STREAM_INTERRUPT_PRIORITY 	GLOBAL_I2C_PROCESSOR_PRIORITY
	#define DMA0_INT_PRIORITY				GLOBAL_I2C_PROCESSOR_PRIORITY

	// Unused priority levels
	#define ADC_INT_PRIORITY				3  /*Can be used to halt ADC sequence*/

	// SW WDT Timeout in seconds - comment to cancel the swwdt
	#define RTC_SWWDT_TIMEOUT 600

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


// Used to restrict the write flash function
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
	#define GetInstructionClock()  	(((OSCCONbits.COSC == 0b011)||(OSCCONbits.COSC == 0b001))? 48000000UL : 3685000UL)
	// Used for SD card driver
	#define GetSystemClock()  		48000000UL /*Warning this should not be used other than in the SD card driver*/
	// Used in SD card code
	#define GetPeripheralClock()	(GetInstructionClock())

	// USB setup
	#define OWN_CDC_BUFFER_SIZES
	#define IN_BUFFER_CAPACITY 64
	#define OUT_BUFFER_CAPACITY 64
    // USB bus sense I/O
    // #define USE_USB_BUS_SENSE_IO
   	#define USB_BUS_SENSE       PORTDbits.RD1
	#define USB_BUS_SENSE_PIN	TRISDbits.TRISD1
	#define USB_BUS_SENSE_INIT() USB_BUS_SENSE_PIN=1
	// Change notification int,  vectors to button handler
	#define USB_INIT_BUS_SENSE_INTS()	{}
						
    // MCHPFSUSB frameworkc self-power
    #define self_power          0   // 0 = bus-powered, 1 = self-powered

    // ROM storage
    #define WRITE_BLOCK_SIZE 256		// Write in rows of 64 instructions (only 2/3 bytes available = 128 bytes)
    #define ERASE_BLOCK_SIZE 2048		// Erase in blocks of 8 rows, 512 instructions (only 2/3 bytes available = 1024 bytes)

	// The pins
	#define InitIO()        {\
							REMAP_PINS();\
                            ANALOGUE_INIT_PINS();\
							LED_INIT_PINS();\
                            ACCEL_INIT_PINS();\
							DISPLAY_INIT_PINS();\
							BATT_INIT_PINS();\
                            TEMP_INIT_PINS();\
                            LDR_INIT_PINS();\
							BUTTON_INIT_PINS();\
							SD_INIT_PINS();\
							CODEC_INIT_PINS();\
                            DRIVE_FLOATING_PINS(); /*Reduce leakage*/\
                            USB_BUS_SENSE_INIT();\
                            }

	#define DRIVE_FLOATING_PINS() {\
									TRISBbits.TRISB13 = 0; LATBbits.LATB13 = 0;\
									TRISBbits.TRISB12 = 0; LATBbits.LATB12 = 0;\
									TRISBbits.TRISB6 = 0; LATBbits.LATB6 = 0;\
									TRISBbits.TRISB5 = 0; LATBbits.LATB5 = 0;\
									TRISBbits.TRISB4 = 0; LATBbits.LATB4 = 0;\
									TRISBbits.TRISB2 = 0; LATBbits.LATB2 = 0;\
									TRISDbits.TRISD6 = 0; LATDbits.LATD6 = 0;\
									TRISDbits.TRISD3 = 0; LATDbits.LATD3 = 0;\
									TRISDbits.TRISD2 = 0; LATDbits.LATD2 = 0;\
									TRISCbits.TRISC15 = 0;LATCbits.LATC15 =0;/*CLKO*/\
									TRISG&=0xfff9;LATG&=0xfff9;/*D+ and D-*/\				
									TRISBbits.TRISB1 = 0; LATBbits.LATB1 = 0;/*PGEC*/\
									TRISBbits.TRISB0 = 0; LATBbits.LATB0 = 0;/*PGED*/\
									}
							
	// REMAP pins
	#define REMAP_PINS()	{	/*SPI1 - SD*/\
								RPINR20bits.SCK1R= 46;/*SCK1i */\
								RPINR20bits.SDI1R= 100;/*SDI1 */\
								/*RPOR7bits.RP46R = 0x06; FIX ASAP*/ /*SCK1o */\
								RPOR0bits.RP64R = 0x06;   /* externally wired SCK1o */\
								RPOR9bits.RP101R = 0x05;  /*SDO1 */\
								/*SPI2 - CODEC*/\
								/*RPINR23bits.SS2R = 121;Now using DCI*//*SS2*/\
								RPINR25bits.COFSR = 121; 	/*DCI Frame pin input*/\
								RPINR24bits.CSCKR = 118; 	/*DCI CK pin input*/\
								RPINR24bits.CSDIR = 119;	/*DCI DI pin input*/\
								RPOR14bits.RP120R = 11;		/*DCI DO pin output*/\
								/*ACCEL INTS - INT1/2*/\
								RPINR0bits.INT1R = 72;/* AccInt1 RPI72 -> Int1 */\
								RPINR1bits.INT2R = 75;/* AccInt2 RPI75 -> Int2 */\
								/*Then lock the PPS module*/\
								__builtin_write_OSCCONL(OSCCON | 0x40);}			

	#define CLOCK_PLL()	ClockTunedFRCPLL() /* Use internal FRC with tuning*/

    // Clock: initialize PLL for 96 MHz USB and 16MIPS
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

	// Switch over to the external clock - WARNING!!! If an external clock is absent then this will hang indefinitely
	#define CLOCK_EC() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							OSCCONBITS_copy.NOSC = 0b010;			/*Primary osc*/\
							OSCCONBITS_copy.OSWEN = 1;\
							asm("DISI #0x3FFF");\
							__builtin_write_OSCCONH( ((unsigned char*)  &OSCCONBITS_copy)[1] );\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );\
							while (OSCCONbits.OSWEN == 1);\
							asm("DISI #0");\
							}
	#define CLOCK_EC_NOWAIT() 	{\
							OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							OSCCONBITS_copy.NOSC = 0b010;			/*Primary osc*/\
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

    #define LED_G_PIN  			TRISBbits.TRISB11        
    #define LED_G               LATBbits.LATB11
    #define LED_R_PIN           TRISBbits.TRISB10
    #define LED_R              	LATBbits.LATB10 
    #define LED_B_PIN           TRISBbits.TRISB9
    #define LED_B     			LATBbits.LATB9
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


    // Battery - AN15/RB15
    #define BATT_MON_PIN        TRISBbits.TRISB15
    #define BATT_INIT_PINS()   	BATT_MON_PIN = 1

	// As used in the analogue.c file to calculate percentages - see 'calculator + curve fit.xlsx'
	#define BATT_CEILING	642	/* 100%	*/
	#define BATT_FLOOR	559		/* 0%	*/
	#define BATT_C1		605		/*Range 1 605 - 642*/
	#define BATT_C2		180	
	#define BATT_C3		73
	#define BATT_C4		8
	#define BATT_C5		559		/*Range 2 559 - 605*/
	#define BATT_C6		373
	#define BATT_C7		6
	#define BATT_C8		8
	#define BATT_CHARGE_MIN_LOG		BATT_FLOOR 		// minimum level to update log
	#define BATT_CHARGE_MIN_LOGGING BATT_FLOOR 		// Threshold for logging stop condition
	#define BATT_CHARGE_MIN_SAFE	BATT_FLOOR 		// minimum safe running voltage (5%), avoids unwanted RTC resets and file corruptions etc.
    #define BATT_CHARGE_FULL_USB	BATT_CEILING		
    #define BATT_CHARGE_MID_USB		608     // level to count towards recharge cycle counter (75%)

    // LDR 
    #define LDR_ENABLE()        
    #define LDR_DISABLE()       
    #define LDR_INIT_PINS()     

	// Temp sensor 
	#define TEMP_ENABLE()		
	#define TEMP_DISABLE()		
	#define TEMP_INIT_PINS()	

	// Analogue select pins - 0 = Analogue
	#define ADC_SELECT_L			0b0111111111111111
	#define ADC_SELECT_H			0xffff
	#define ANALOGUE_INIT_PINS()	{\
									ANSELB = 0b1011000001110100;/*Floaters are now analogue too*/\
									ANSELC = 0;\
									ANSELD = 0;\
									ANSELE = 0;\
									ANSELG = 0;\
									}
    #define ADC_INDEX_BATT		0


	// For the following 4 peripherals the interface is via I2C using these pins
	// All the pins need re-mapping. See RemapPins() in this file.

	// Defines for myI2C.c
	#define USE_HW_I2C1
	//#define USE_MY_SW_I2C
	#define MY_I2C_TIMEOUT 	65535

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

	/* I2C devices that may be present
	Accelerometer 	MMAQ8452Q 	0x38
	*/

    // Accelerometer MMA8451Q
	#define ACCEL_MMA8451Q
	#define ACCEL_I2C_MODE
	#define ACCEL_I2C_RATE 		I2C_RATE_400kHZ
	#define ACCEL_FIFO_WATERMARK 25
    #define ACCEL_INT1_PIN      TRISDbits.TRISD8 
    #define ACCEL_INT2_PIN      TRISDbits.TRISD11 	
    #define ACCEL_INT1          PORTDbits.RD8
    #define ACCEL_INT2          PORTDbits.RD11 
   	#define ACCEL_INT1_IP       IPC5bits.INT1IP
   	#define ACCEL_INT2_IP    	IPC7bits.INT2IP   
    #define ACCEL_INT1_IF       IFS1bits.INT1IF
    #define ACCEL_INT2_IF       IFS1bits.INT2IF
    #define ACCEL_INT1_IE     	IEC1bits.INT1IE
    #define ACCEL_INT2_IE       IEC1bits.INT2IE

    #define ACCEL_INIT_PINS()  	{\
								ACCEL_INT1_PIN	= 1;\
								ACCEL_INT2_PIN	= 1;\
								}
								
	// Graphical DISPLAY - OLED on SSD1306,8080
	#define NO_DISPLAY
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
	
	#define DISPLAY_CHIP_SELECT_TRIS    TRISDbits.TRISD7
	#define DISPLAY_CHIP_SELECT         LATDbits.LATD7
	#define DISPLAY_RESET_TRIS          TRISFbits.TRISF0
	#define DISPLAY_RESET               LATFbits.LATF0
	#define DISPLAY_COMMAND_DATA_TRIS   TRISFbits.TRISF1
	#define DISPLAY_COMMAND_DATA        LATFbits.LATF1

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
					
	// Button not present - use proximity
	extern unsigned char gButton;
	extern unsigned char gButtonClick;
	extern unsigned char UpdateProximity(unsigned char doWait);
	//  Proximity sensor thresholds (static at the moment)
//	#define PROX_DEFAULT_LIGHT_TOUCH	10000UL
	#define PROX_DEFAULT_TOUCH			10000UL
	// Dummy defines
	#define BUTTON 		gButton
	#define BUTTON_INIT_PINS()	
	#define BUTTON_ENABLE_INTS()		

	// Codec interface - referenced to HOST
	#define CODEC_SCK_PIN	TRISGbits.TRISG6
	#define CODEC_SCK		PORTGbits.RG6
	#define CODEC_SDI_PIN	TRISGbits.TRISG7
	#define CODEC_SDI		PORTGbits.RG7
	#define CODEC_SDO_PIN	TRISGbits.TRISG8
	#define CODEC_SDO		LATGbits.LATG8
	#define CODEC_FRAME_PIN	TRISGbits.TRISG9
	#define CODEC_FRAME		PORTGbits.RG9
	#define CODEC_POWER_PIN	TRISBbits.TRISB3
	#define CODEC_POWER		LATBbits.LATB3

	#define CODEC_INIT_PINS()	{\
								CODEC_SCK_PIN = 1;\
								CODEC_SDI_PIN = 1;\
								CODEC_SDO_PIN = 0;\
								CODEC_SDO = 0;\
								CODEC_FRAME_PIN = 1;\
								CODEC_POWER = 0;\
								CODEC_POWER_PIN = 0;\
								}
	#define CODEC_ENABLE()	{\
								CODEC_POWER = 1;\
								DelayMs(100);\
								}

	// SD card interface - referenced to HOST
	extern short dummy;
	extern unsigned char gSD_CARD_DETECT;

	#define SD_CS			LATBbits.LATB8
	#define SD_CS_TRIS		TRISBbits.TRISB8
	#define SD_SDI			PORTFbits.RF4
	#define SD_SDI_TRIS		TRISFbits.TRISF4
	#define SD_SDO			LATFbits.LATF5
	#define SD_SDO_TRIS		TRISFbits.TRISF5
	//#define SD_SCK			LATBbits.LATB14 Replaced by wire
	//#define SD_SCK_TRIS		TRISBbits.TRISB14
	#define SD_SCK			LATDbits.LATD0
	#define SD_SCK_TRIS		TRISDbits.TRISD0
	#define SD_EN			LATFbits.LATF3
	#define SD_EN_TRIS		TRISFbits.TRISF3
	#define SD_CD_TRIS		dummy
	#define SD_CD			(!gSD_CARD_DETECT)
	//#define MEDIA_SOFT_DETECT
	#define SD_WE_TRIS		dummy
	#define SD_WE			0

    #define 	SD_CS_ANSEL		ANSELBbits.ANSB8
    #define    	SD_SCK_ANSEL	dummy
    #define    	SD_SDI_ANSEL	dummy
    #define    	SD_SDO_ANSEL	dummy

	#define SD_INIT_PINS()	{\
							SD_EN_TRIS = 0;\
							SD_EN = 0;\
							TRISBbits.TRISB14 = 1;/*Old pin replaced by wire*/\
							SD_SCK_TRIS = 0;\
							SD_SCK = 0;\
							SD_SDO_TRIS = 0;\
							SD_SDO = 0;\
							SD_CS_TRIS = 1;/*Pull ups are on internal to card*/\
							SD_CS = 1;\
							SD_SDI_TRIS = 0;\
							}
	#define SD_ENABLE()		{SD_SDI_TRIS = 1;\
							SD_SCK = 1;\
							SD_SDO = 1;\
							SD_CS_TRIS = 0;\
							SD_CS = 1;\
							SD_EN = 1;\
							DelayMs(10);}
	#define SD_DISABLE()	{\
							SD_CS = 1;		/*Deselect card*/\
							DelayMs(500);	/*Wait for internal opperations to complete*/\
							SD_INIT_PINS();	/*Power off card*/}

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

	// Motor control pin
	#define MOTOR_PIN		TRISBbits.TRISB7
	#define MOTOR			LATBbits.LATB7
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

