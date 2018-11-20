
#ifndef _HARDWAREPROFILE_
#define _HARDWAREPROFILE_

// SW defines
#define HARDWARE_VERSION 11
#define WAV_NO_READ

#define Sleep()			{mSysUnlockOpLock(OSCCONSET = (1 << _OSCCON_SLPEN_POSITION));asm("WAIT");}
#define Idle()			{mSysUnlockOpLock(OSCCONSET = (1 << _OSCCON_SLPEN_POSITION));asm("WAIT");}

#include "Compiler.h"

	// Interrupt priority levels

	// Highest - RTC, TIMER tasks
	#define RTC_INT_PRIORITY				6 
	#define T1_INT_PRIORITY					FIFO_INTERRUPT_PRIORITY 	/*Prevents ms rollover during reads*/

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

	//  Proximity sensor thresholds (static at the moment)
	#define PROX_LIGHT_TOUCH	65535UL
	#define PROX_TOUCH			65535UL

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


// Used to restric the write flash function
#define MIN_ALLOWED_WRITE_ADD 0		// Protects bootloader
#define MAX_ALLOWED_WRITE_ADD 0		// Protects configuation
	
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
	#define GetInstructionClock()  	((OSCCON&0x1000UL)? 80000000UL : 8000000UL)
	// Used for SD card driver
	#define GetSystemClock()  		80000000UL /*Warning this should not be used other than in the SD card driver*/
	// Used in SD card code
	#define GetPeripheralClock()	(GetInstructionClock())

    // USB bus sense I/O
    //#define USE_USB_BUS_SENSE_IO
	//#define tris_usb_bus_sense	USB_BUS_SENSE_PIN
   	#define USB_BUS_SENSE       PORTDbits.RD1
	#define USB_BUS_SENSE_PIN	TRISDbits.TRISD1
	#define USB_BUS_SENSE_INIT() USB_BUS_SENSE_PIN=1
	// Change notification int,  vectors to button handler
	#define USB_INIT_BUS_SENSE_INTS()	{\
										}
						
    // MCHPFSUSB frameworkc self-power
    #define self_power          0   // 0 = bus-powered, 1 = self-powered

    // ROM storage
    #define WRITE_BLOCK_SIZE 128		// Write in rows of 64 instructions (only 2/3 bytes available = 128 bytes)
    #define ERASE_BLOCK_SIZE 1024		// Erase in blocks of 8 rows, 512 instructions (only 2/3 bytes available = 1024 bytes)

	// The pins
	#define InitIO()        {\
							DDPCON = 0x00000100;/*JTAG off + see errata*/\
							ANALOGUE_INIT_PINS();\
							LED_INIT_PINS();\
							MOTOR_INIT_PINS();\
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

	#define DRIVE_FLOATING_PINS() {/*There are none*/}
							
	// REMAP pins
	#define REMAP_PINS()	{/*There is no remap on this device*/}			

    // Clock: initialize PLL
	#define CLOCK_PLL() 	{SYSTEMConfigPerformance(80000000LU);\
							OSCConfig(OSC_FRC_PLL, OSC_PLL_MULT_20, OSC_PLL_POST_1, OSC_FRC_POST_1);\
							SYSTEMConfigPerformance(80000000LU);}
	
    // Clock: initialize PLL - 2ms minimum startup if used
	#define CLOCK_XTAL_PLL()	{SYSTEMConfigPerformance(80000000LU);\
							OSCConfig(OSC_POSC_PLL, OSC_PLL_MULT_20, OSC_PLL_POST_1, OSC_FRC_POST_1);\
							SYSTEMConfigPerformance(80000000LU);}			

    // Clock: switch to 8 MHz INTOSC
	#define CLOCK_INTOSC() 	{SYSTEMConfigPerformance(80000000LU);\
							OSCConfig(OSC_FRC, 0, 0, 0);\
							SYSTEMConfigPerformance(8000000LU);}

    // Clock: switch to 8 MHz INTOSC with divisor - powers of two i.e. 2^_val_ except no div128 (7-> div 256) 
	#define CLOCK_INTOSCDIV(_val_) 	{OSCConfig(OSC_FRC_DIV, 0, 0, (_val_ << _OSCCON_FRCDIV_POSITION));\
									SYSTEMConfigPerformance(8000000LU);}

    // Clock: switch to 500kHz INTOSC
	#define CLOCK_INTOSCDIV16() {OSCConfig(OSC_FRC_DIV16, 0, 0, 0);SYSTEMConfigPerformance(500000LU);}	

    // Clock: switch to 31.25 kHz INTRC
    #define CLOCK_INTRC() 	{OSCConfig(OSC_LPRC, 0, 0, 0);SYSTEMConfigPerformance(31250LU);}

	// SOSC / RTC, 32.768 kHz
    #define CLOCK_32KHZ() 	{OSCConfig(OSC_SOSC, 0, 0, 0);SYSTEMConfigPerformance(32768LU);}
       
	// Secondary oscillator on
	#define CLOCK_SOSCEN()	{	TRISCbits.TRISC13 = 0; TRISCbits.TRISC14 = 0;\
								LATCbits.LATC13 = 0;LATCbits.LATC14 = 1;\
								Delay10us(1);\
								LATCbits.LATC13 = 1;LATCbits.LATC14 = 0;\
								Delay10us(1);\
								LATCbits.LATC13 = 0;LATCbits.LATC14 = 1;\
								TRISCbits.TRISC13 = 1; TRISCbits.TRISC14 = 1;\
								mOSCEnableSOSC();}

	// Using PBDIV - must use this macro for modifying PBDIV 'OSCSetPBDIV(OSC_PB_DIV_1); 1,2,4,8 are available'

    // LED
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


    // Battery - AN8
    #define BATT_MON_PIN        TRISBbits.TRISB15
    #define BATT_INIT_PINS()   	BATT_MON_PIN = 1
    #define BATT_CHARGE_ZERO		614		// FIXED CONST, DO NOT USE
	#define BATT_CHARGE_FULL		708		// FIXED CONST, DO NOT USE
	#define BATT_CHARGE_MIN_LOG		517		// minimum level to update log
	#define BATT_CHARGE_MIN_SAFE	520		// minimum safe running voltage (5%), avoids unwanted RTC resets and file corruptions etc.
    #define BATT_CHARGE_MID_USB		666     // level to count towards recharge cycle counter (75%)
    #define BATT_CHARGE_FULL_USB	708

    // LDR - AN6
    #define LDR_ENABLE()        
    #define LDR_DISABLE()       
    #define LDR_INIT_PINS()    

	// Temp sensor - AN9
	#define TEMP_ENABLE()		
	#define TEMP_DISABLE()		
	#define TEMP_INIT_PINS()

	// Analogue select pins - 0 = Analogue
	#define ADC_SELECT			0b11111111111111110111111111111111UL
	#define ANALOGUE_INIT_PINS()	{AD1PCFG = ADC_SELECT;}
    #define ADC_INDEX_BATT		0


	// Several devices can use the change notification interrups
	#define CN_INTS_ON()	{}
	#define CN_INTS_OFF()	{}
	#define CN_INT_FIRE()	{}

	// For the following 4 peripherals the interface is via I2C using these pins
	// All the pins need re-mapping. See RemapPins() in this file.

	// Defines for myI2C.c
	#define USE_HW_I2C1
	//#define USE_MY_SW_I2C
	#define MY_I2C_TIMEOUT 	65535

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
	#define ACCEL_FIFO_WATERMARK 25
    #define ACCEL_INT1_PIN      TRISDbits.TRISD8 
    #define ACCEL_INT2_PIN      TRISDbits.TRISD11 	
    #define ACCEL_INT1          PORTDbits.RD8
    #define ACCEL_INT2          PORTDbits.RD11 
   	#define ACCEL_INT1_IP       IPC1bits.INT1IP
   	#define ACCEL_INT2_IP    	IPC2bits.INT2IP   
    #define ACCEL_INT1_IF       IFS0bits.INT1IF
    #define ACCEL_INT2_IF       IFS0bits.INT2IF
    #define ACCEL_INT1_IE     	IEC0bits.INT1IE
    #define ACCEL_INT2_IE       IEC0bits.INT2IE

    #define ACCEL_INIT_PINS()  	{\
								ACCEL_INT1_PIN	= 1;\
								ACCEL_INT2_PIN	= 1;\
								}
								
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
	#define DISPLAY_DATA				PMDIN
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
	extern unsigned char UpdateProximity(unsigned char doWait);
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
	#define CODEC_nPOWER_PIN	TRISBbits.TRISB3
	#define CODEC_nPOWER		LATBbits.LATB3

	#define CODEC_INIT_PINS()	{\
								CODEC_SCK_PIN = 1;\
								CODEC_SDI_PIN = 1;\
								CODEC_SDO_PIN = 0;\
								CODEC_SDO = 0;\
								CODEC_FRAME_PIN = 1;\
								CODEC_nPOWER = 1;\
								CODEC_nPOWER_PIN = 0;\
								}
	#define CODEC_ENABLE()	{\
								CODEC_SDO = 1;\
								DelayMs(10);\
								CODEC_nPOWER = 0;\
								}

	// SD card interface - referenced to HOST
	extern int dummy;

	#define SD_CS			LATBbits.LATB8
	#define SD_CS_TRIS		TRISBbits.TRISB8
	#define SD_SDI			PORTFbits.RF4
	#define SD_SDI_TRIS		TRISFbits.TRISF4
	#define SD_SDO			LATFbits.LATF5
	#define SD_SDO_TRIS		TRISFbits.TRISF5
	#define SD_SCK			LATBbits.LATB14
	#define SD_SCK_TRIS		TRISBbits.TRISB14
	#define SD_EN			LATFbits.LATF3
	#define SD_EN_TRIS		TRISFbits.TRISF3
	#define SD_CD_TRIS		dummy
	#define SD_CD			0
	//#define MEDIA_SOFT_DETECT
	#define SD_WE_TRIS		dummy
	#define SD_WE			0

	#define SD_INIT_PINS()	{\
							SD_EN_TRIS = 0;\
							SD_EN = 0;\
							SD_SCK_TRIS = 0;\
							SD_SCK = 0;\
							SD_SDO_TRIS = 0;\
							SD_SDO = 0;\
							SD_CS_TRIS = 0;\
							SD_CS = 0;\
							SD_SDI_TRIS = 1;\
							}
	#define SD_ENABLE()		{SD_CS = 1;SD_EN = 1;DelayMs(10);}
	#define SD_DISABLE()	{\
							SD_CS = 1;		/*Deselect card*/\
							DelayMs(500);	/*Wait for internal opperations to complete*/\
							SD_INIT_PINS();	/*Power off card*/}

    /*******************************************************************/
    /******** MDD File System selection options ************************/
    /*******************************************************************/
    #define USE_SD_INTERFACE_WITH_SPI

	//SPI Configuration
 	#define SPI_CHANNEL   		4
    #define SPI_FREQUENCY		(20000000)
	#define SPI_START_CFG_1     (ignored)
    #define SPI_START_CFG_2     (ignored) /*Following open function is used in place*/
 	#define OpenSPI(config1, config2) SpiChnOpen(SPI_CHANNEL, SPI_OPEN_MSTEN|SPI_OPEN_CKP_HIGH|SPI_OPEN_SMP_END|SPI_OPEN_MODE8, (GetPeripheralClock())/SPI_FREQUENCY) 

    // Define the SPI frequency
    // Tris pins for SCK/SDI/SDO lines
    #define SPICLOCK			SD_SCK_TRIS
    #define SPIIN               SD_SDI_TRIS
    #define SPIOUT				SD_SDO_TRIS
    // Registers for the SPI module you want to use
    #define SPICON1				SPI4CON
    #define SPISTAT				SPI4STAT
    #define SPIBUF				SPI4BUF
    #define SPISTAT_RBF			SPI4STATbits.SPIRBF
    #define SPICON1bits			SPI4CONbits
    #define SPISTATbits			SPI4STATbits
    #define SPIENABLE           SPI4CONbits.ON
    #define SPIBRG			    SPI4BRG
 	#define putcSPI(data)  		SpiChnPutC(SPI_CHANNEL, data)
	#define getcSPI()    		SpiChnGetC(SPI_CHANNEL)


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

