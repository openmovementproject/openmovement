// Hardware configuration, control and description
// Karim Ladha, 2015

#ifndef HARDWAREPROFILE_CWA31_H
#define HARDWAREPROFILE_CWA31_H

// Hardware name
#define HARDWARE_NAME			"CWA3"
#define BT_CHIPSET_TARGET 		2564ul	
#define HARDWARE_VERSION_BCD	0x32
#define HARDWARE_VERSION 		"3.2"

// IO pins
#define InitIO()        {\
						REMAP_PINS();\
						LED_INIT_PINS();\
						FLASH_INIT_PINS();\
						ACCEL_INIT_PINS();\
						GYRO_INIT_PINS();\
						MAG_INIT_PINS();\
						ALT_INIT_PINS();\
						DISPLAY_INIT_PINS();\
						BT_INIT_OFF();\
						BATT_INIT_PINS();\
						TEMP_INIT_PINS();\
						LDR_INIT_PINS();\
						BUTTON_INIT_PINS();\
						MOTOR_INIT_PINS();\
						ANALOGUE_INIT_PINS();\
						DRIVE_FLOATING_PINS(); /*Reduce leakage*/\
						USB_BUS_SENSE_INIT();\
						}

// Cut down InitIO to save space in bootloader
#define InitIOBootloader()	{\
							/*  	  111111          */\
							/*		  5432109876543210*/\
							LATB =	0b0000011111000000;\
							TRISB =	0b1100100000110100;\
							LATC =	0b0000000000000000;\
							TRISC =	0b0010000000000000;\
							LATD =	0b0000000010110000;\
							TRISD =	0b0000011000001111;\
							LATE =	0;\
							TRISE =	0;\
							LATF =	0b0000000000010000;\
							TRISF =	0b0000000000100011;\
							LATG =	0b0000000000000000;\
							TRISG =	0b0000001111000000;\
							ANALOGUE_INIT_PINS();\
						}
#define InitIOBoot InitIOBootloader
						
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
						RPINR2bits.INT4R = 19;/*RP19 = INT4 (BT_RTS)*/\
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
#define OSC_PLL_ON()	(OSCCONbits.NOSC == 0b001)
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
						CLKDIV = 0; 							/*8 MHz */\
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
						CLKDIV = 0; 							/*8 MHz */\
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
						CLKDIV = 0; 							/*8 MHz */\
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
						CLKDIV = 0; 							/*8 MHz */\
						}
   
// Secondary oscillator on/off
#define CLOCK_SOSCEN()	{OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
						OSCCONBITS_copy.SOSCEN=1;\
						__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );}
#define CLOCK_SOSCDIS()	{OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
						OSCCONBITS_copy.SOSCEN = 0;\
						__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );}

// USB bus sense I/O
// #define USE_USB_BUS_SENSE_IO
#define USB_BUS_SENSE_PIN	TRISBbits.TRISB4
#define USB_BUS_SENSE       PORTBbits.RB4
#define USB_BUS_SENSE_INIT() USB_BUS_SENSE_PIN=1
#define self_power  0   /*Mchips crappy name for setting*/

						
// LED - active HIGH
#define LED_G_PIN  			TRISCbits.TRISC12        
#define LED_G               LATCbits.LATC12
#define LED_R_PIN           TRISCbits.TRISC15
#define LED_R              	LATCbits.LATC15 
#define LED_B_PIN           TRISDbits.TRISD8
#define LED_B     			LATDbits.LATD8
#define LED_INIT_PINS()     {LED_G_PIN=LED_G=LED_R_PIN=LED_R=LED_B_PIN=LED_B=0;}

// LED suspend/resume/on/off definitions
#define LED_TURN_ON	1
#define LED_TURN_OFF 0
#define LED_SUSPEND()       { LED_R_PIN = 1; LED_G_PIN = 1; LED_B_PIN = 1; }
#define LED_RESUME()        { LED_R_PIN = 0; LED_G_PIN = 0; LED_B_PIN = 0; }
// LED set (0bRGB) - should compile to three bit set/clears with a static colour value
#define LED_SET(_c) {\
				if ((_c) & 0x4) { LED_R = LED_TURN_ON; } else { LED_R = LED_TURN_OFF; } \
				if ((_c) & 0x2) { LED_G = LED_TURN_ON; } else { LED_G = LED_TURN_OFF; } \
				if ((_c) & 0x1) { LED_B = LED_TURN_ON; } else { LED_B = LED_TURN_OFF; } }


// Analogue select pins - 0 = Analogue
#define ADC_SELECT_L		0b0011111111111011
#define ADC_SELECT_H		0b1111111111111111
#define ANALOGUE_INIT_PINS()	{AD1PCFGL = ADC_SELECT_L;AD1PCFGH = ADC_SELECT_H;}
#define ANALOGUE_VREF_MV	3000
#define ANALOGUE_SETTLING_WAIT	1
#define ADC_INDEX_BATT		0
#define ADC_INDEX_LIGHT		2
#define ADC_INDEX_TEMP		1

// Battery - AN2
#define BATT_MON_PIN        TRISBbits.TRISB2
#define BATT_INIT_PINS()   	BATT_MON_PIN = 1
#define BATT_CHARGE_ZERO		614		// FIXED CONST, DO NOT USE
#define BATT_CHARGE_FULL		708		// FIXED CONST, DO NOT USE
#define BATT_CHARGE_WARNING		618		// Low battery warning at 5% (Value = Percentage * 256 / 375 + 614)
#define BATT_CHARGE_MIN_LOG		517		// minimum level to update log
#define BATT_CHARGE_MIN_SAFE	520		// minimum safe running voltage (5%), avoids unwanted RTC resets and file corruptions etc.
#define BATT_CHARGE_MID_USB		666     // level to count towards recharge cycle counter (75%)
#define BATT_CHARGE_FULL_USB	708
#define BATT_SMOOTHED			

// LDR - AN15
#define LDR_EN_PIN			TRISFbits.TRISF4         
#define LDR_EN              LATFbits.LATF4
#define LDR_OUT_PIN         TRISBbits.TRISB15
#define LDR_ENABLE()        {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 0;}
#define LDR_DISABLE()       {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 1;}
#define LDR_INIT_PINS()     LDR_DISABLE()

// Temp sensor - AN14
#define USE_MCP9700
#define TEMP_EN_PIN			TRISDbits.TRISD11
#define TEMP_EN				LATDbits.LATD11
#define TEMP_OUT_PIN		TRISBbits.TRISB14
#define TEMP_ENABLE()		{TEMP_OUT_PIN=1;TEMP_EN_PIN=0;TEMP_EN=1;}
#define TEMP_DISABLE()		{TEMP_OUT_PIN=1;TEMP_EN_PIN=0;TEMP_EN=0;}
#define TEMP_INIT_PINS()	TEMP_DISABLE()

// Nand Flash Memory - Hynix
#define NAND_DEVICE 	NAND_DEVICE_HY27UF084G2B
#define NAND_BYTES_PER_PAGE 2112
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

// For the following peripherals the interface is via I2C using these pins.
// Interrupts etc. will require pin remaps.
// CWA3 uses one bus for all sensors.

// Defines for myI2C.c on I2C1
#define USE_HW_I2C1
#define MY_I2C_TIMEOUT 	65535
//#define IGNORE_I2C_TIMOUTS // Uncomment if timeout checks are not required
#define ENABLE_I2C_TIMOUT_CHECK	// Creates the default function in myi2c.c
#define CHECK_TIMEOUT(_val)	myI2CCheckTimeout(_val)	// Using default, or assign a function to ensure val is never zero (i.e. a bus timeout has not occured) 
#define InitI2C()			{mySDAd=1;myI2Cclear();mySCLd = 1;}

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
#define ACCEL_INT1_VECTOR	_INT1Interrupt
#define ACCEL_INT2_VECTOR	_INT2Interrupt
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
#define MAG_ZERO_ON_POWER_UP
#define MAG_INT_VECTOR	_CNInterrupt
#define MAG_INT_PIN     	TRISFbits.TRISF1  
#define MAG_INT        		PORTFbits.RF1    
#define MAG_INT_IF 			IFS1bits.CNIF
#define MAG_INT_IE  		IEC1bits.CNIE
#define MAG_CN_INT_IE		CNEN5bits.CN69IE
#define MAG_INIT_PINS()  	{\
							MAG_INT_PIN	= 1;\
							}
#define MAG_ENABLE_INTS()	{/*Interrupt on change CN18*/\
								CNEN2bits.CN18IE=1;\
								CN_INTS_ON();\
							}
#define MAG_DISABLE_INTS()	{CNEN2bits.CN18IE=0;}

// Several devices can use/cause the change notification interrups
#define CN_INTS_ON()	{IFS1bits.CNIF = 0;IPC4bits.CNIP = CN_INTS_PROCESSOR_PRIORITY; IEC1bits.CNIE = 1;}
#define CN_INTS_OFF()	{IEC1bits.CNIE = 0;}
#define CN_INT_FIRE()	{IFS1bits.CNIF = 1;}
	
// Altimeter BMP085
#define ALT_IGNORE_EOC
#define USE_CTRL_REG_BUSY_BIT
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

// Bluetooth module test PAN1315 or PAN1316 
#define BT_SLW_CLK_PIN	TRISBbits.TRISB5
#define BT_SLW_CLK		LATBbits.LATB5
#define BT_EN_PIN		TRISBbits.TRISB3
#define BT_EN			LATBbits.LATB3
#define BT_CTS_PIN 		TRISGbits.TRISG9
#define BT_CTS			LATGbits.LATG9
#define BT_RTS_PIN		TRISGbits.TRISG8
#define BT_RTS			PORTGbits.RG8
#define BT_RX_PIN		TRISGbits.TRISG7
#define BT_TX_PIN		TRISGbits.TRISG6

#define BT_INIT_PINS()	{BT_EN_PIN=0;BT_EN=0;\
						BT_CTS_PIN=0;BT_CTS=1;\
						BT_RTS_PIN=1;\
						BT_RX_PIN=0;\
						BT_TX_PIN=1;}

#define BT_INIT_OFF()	{BT_EN_PIN=0;BT_EN=0;\
						BT_CTS_PIN=1;\
						BT_RTS_PIN=1;\
						BT_RX_PIN=1;\
						BT_TX_PIN=1;}

// BT Clock - routed sosc through comparator CM3, C3IND/RC13->C3OUT-> to RP18/RB5
#define STOP_BT_SLW_CLK()	{CM3CONbits.CEN = 1;CVRCONbits.CVREN=0;BT_SLW_CLK=0;}
#define SETUP_BT_SLW_CLK()	{\
							CM3CONbits.CEN = 1;/*Enable C3*/\
							CM3CONbits.COE = 1;/*Output on*/\
							CM3CONbits.CREF = 1;/*Use Cref*/\
							CM3CONbits.CCH = 0b10;/*Input D*/\
							CVRCONbits.CVREN = 1;/*Enable Cref*/\
							CVRCONbits.CVRR = 0;/*Setup Cref*/\
							CVRCONbits.CVR = 0x8;/*Set Cref = Vdd/2 */\
							BT_SLW_CLK_PIN = 0;}/*Set pin output*/	

// Defines for bleStack
#define BT_UART_DEFAULT_BAUD 	115200UL
#define BT_UART_RTS				BT_CTS
#define BT_UART_USES_UART_1

// BT eHCILL signaling support
#define BT_eHCILL_HANDLER_VECTOR		_INT4Interrupt
#define BT_eHCILL_HANDLER_FLAG			IFS3bits.INT4IF
#define BT_eHCILL_HANDLER_PRIORITY		IPC13bits.INT4IP
#define BT_eHCILL_HANDLER_ENABLE		IEC3bits.INT4IE	

// BT event trigger support
#define BT_EVENT_HANDLER_VECTOR		_IC4Interrupt
#define BT_EVENT_HANDLER_FLAG		IFS2bits.IC4IF
#define BT_EVENT_HANDLER_IPL		IPC9bits.IC4IP
#define BT_EVENT_HANDLER_ENABLE		IEC2bits.IC4IE

#define BT_EVENT_HANDLER_ARM()		{IC4CON1 = 7;\
									BT_EVENT_HANDLER_IPL = BT_EVENT_HANDLER_PRIORITY;\
									BT_EVENT_HANDLER_ENABLE = 1;}
#define BT_EVENT_HANDLER_DISARM()	{IC4CON1 = 0;\
									BT_EVENT_HANDLER_ENABLE = 0;}	


// Graphical DISPLAY - OLED on SSD1306,8080
#define DISPLAY_CONTROLLER CUSTOM_CONTROLLER
#define DONT_REFRESH_TOP_DISP_LINES

#define DISP_HOR_RESOLUTION 128
#define DISP_VER_RESOLUTION 64
#define COLOR_DEPTH 1
#define DISP_ORIENTATION 0

#define DISPLAY8080
//#define DISPLAY_DEBUG // Prints checkerboard pattern, shows up faults

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
extern unsigned char gButton;
#if (HARDWARE_VERSION_BCD==0x32)
	#define BUTTON_PIN		TRISFbits.TRISF5
	#define BUTTON			PORTFbits.RF5
	#define BUTTON_INIT_PINS()	{BUTTON_PIN=1;}
	#define BUTTON_ENABLE_INTS()		{/*Interrupt on change CN18*/\
										CNEN2bits.CN18IE;\
										CN_INTS_ON();}
#else
	#define BUTTON 					1
	#define BUTTON_INIT_PINS()		{}
	#define BUTTON_ENABLE_INTS()	{}
#endif

// Motor control pin
#define MOTOR_PIN		TRISFbits.TRISF3
#define MOTOR			LATFbits.LATF3
#define MOTOR_INIT_PINS() 	{MOTOR=0;MOTOR_PIN=0;}

#endif

