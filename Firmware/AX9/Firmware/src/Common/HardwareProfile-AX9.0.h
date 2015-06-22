// Hardware configuration, control and description
// Karim Ladha, 2015

#ifndef HARDWAREPROFILE_AX9_H
#define HARDWAREPROFILE_AX9_H

//#define AX9_HW_V1		// Default old version
#define AX9_HW_V2_RGB	// RGB LED version of AX9-V2 hardware

// Hardware name
#define HARDWARE_NAME			"AX9"
#define BT_CHIPSET_TARGET 		2564ul	

#ifdef AX9_HW_V1
	#define HARDWARE_VERSION_BCD	0x10
	#define HARDWARE_VERSION 		"1.0"
#elif defined(AX9_HW_V2_RGB)
	#define HARDWARE_VERSION_BCD	0x20
	#define HARDWARE_VERSION 		"2.0"
#else
	#error "HW version?"
#endif

// IO pins
#define InitIO()        {\
						REMAP_PINS();\
						LED_INIT_PINS();\
						ACCEL_INIT_PINS();\
						GYRO_INIT_PINS();\
						MAG_INIT_PINS();\
						ALT_INIT_PINS();\
						BT_INIT_OFF();\
						FLASH_INIT_PINS();\
						ANALOGUE_INIT_PINS();\
						CLOCK_INIT_PINS();\
						MOTOR_INIT_PINS();\
						DRIVE_FLOATING_PINS();  /*Reduce leakage*/\
						USB_BUS_SENSE_INIT();\
						}

// Cut down InitIO to save space in bootloader
#define InitIOBoot()	{\
							/*  	  111111          */\
							/*		  5432109876543210*/\
							LATB =	0b0100100000100000;\
							TRISB =	0b1011001111010000;\
							LATC =	0b1001000000000000;\
							TRISC =	0b0110111111111111;/*Accel CS high*/\
							LATD =	0b0000000000110000;\
							TRISD =	0b1111111000000010;/*Accel A0 low*/\
							LATE =	0b1111111100000000;\
							TRISE =	0b1111111100000000;\
							LATF =	0b0000000000000010;\
							TRISF =	0b1111111111110100;\
							LATG =	0b0000001010000000;\
							TRISG =	0b1111110100110011;\
							ANALOGUE_INIT_PINS();\
						}

#define DRIVE_FLOATING_PINS() {\
						TRISBbits.TRISB0 = 0; LATBbits.LATB0 = 0;/*PGD*/\
						TRISBbits.TRISB1 = 0; LATBbits.LATB1 = 0;/*PGC*/\
						TRISBbits.TRISB2 = 0; LATBbits.LATB2 = 0;\
						TRISBbits.TRISB3 = 0; LATBbits.LATB3 = 0;\
						TRISFbits.TRISF3 = 0; LATFbits.LATF3 = 0;\
						}

// Motor control pin - V2 HW only
#define MOTOR_PIN			TRISGbits.TRISG6
#define MOTOR				LATGbits.LATG6
#define MOTOR_INIT_PINS() 	{MOTOR=0;MOTOR_PIN=0;}

// REMAP pins
#define REMAP_PINS()	{\
						__builtin_write_OSCCONL(OSCCON & 0xBF);/*Unlock*/\
						/*INPUTS*/\
						RPINR0bits.INT1R = 8;	/* AccInt1 RP8 -> Int1 */\
						RPINR8bits.IC4R = 7;	/* AccInt2 RP7 -> IC4 */\
						RPINR7bits.IC2R = 12;	/* GyroInt2 dtrdy RP12 -> IC2*/\
						RPINR8bits.IC3R = 9;	/* Mag int RP9 -> IC3*/\
						RPINR18bits.U1CTSR = 28;/*RP28 = U1CTS (BT_RTS)*/\
						RPINR1bits.INT2R = 28;  /*RP28 = INT2 (BT_RTS) */\
						RPINR18bits.U1RXR = 24;	/*RP24 = U1RX (BT_TX)*/\
						/*OUTPUTS*/\
						RPOR11bits.RP23R = 3;	/*RP17 = U1TX (BT_RX)*/\
						/*RPOR3bits.RP6R = 4;*/ 	/*RP6 = U1RTS (BT_CTS)*/\
						RPOR5bits.RP11R = 36;	/*Blutooth 32kHz clock routed through comparator3*/\
						/*Then lock the PPS module*/\
						__builtin_write_OSCCONL(OSCCON | 0x40);\
						}	

// The sosci pin needs to be an input					
#define CLOCK_INIT_PINS()	{TRISCbits.TRISC13 = 1; TRISCbits.TRISC14 = 1;}

// Clock: initialize PLL for 96 MHz USB and 12MIPS
#define OSC_PLL_ON()	(OSCCONbits.NOSC == 0b001)
#define CLOCK_PLL() 	{\
						OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
						asm("DISI #0x3FFF");\
						CLKDIV = 0; 							/*8 MHz */\
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
						CLKDIV = 0; 							/*8 MHz */\
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
						CLKDIV = 0; 							/*8 MHz */\
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
						CLKDIV = 0; 							/*8 MHz */\
						}
   
// Secondary oscillator on/off
#define CLOCK_SOSCEN()	{CLOCK_SOSCDIS();} // This hardware always has SOSC off
#define CLOCK_SOSCDIS()	{	OSCCONBITS OSCCONBITS_copy = OSCCONbits;\
							OSCCONBITS_copy.SOSCEN = 0;\
							__builtin_write_OSCCONL( ((unsigned char*)  &OSCCONBITS_copy)[0] );}

// USB bus sense I/O
// #define USE_USB_BUS_SENSE_IO
#ifdef AX9_HW_V2_RGB
#define USB_BUS_SENSE_PIN	TRISBbits.TRISB12
#define USB_BUS_SENSE       PORTBbits.RB12
#else
#define USB_BUS_SENSE_PIN	TRISBbits.TRISB13
#define USB_BUS_SENSE       PORTBbits.RB13
#endif
#define USB_BUS_SENSE_INIT() USB_BUS_SENSE_PIN=1
#define self_power	0	/*Mchips crappy name for setting*/

// LED - all active LOW
#ifdef AX9_HW_V2_RGB
#define LED_R_PIN           TRISBbits.TRISB5
#define LED_R              	LATBbits.LATB5
#define LED_G_PIN  			TRISGbits.TRISG9        
#define LED_G               LATGbits.LATG9
#define LED_B_PIN           TRISCbits.TRISC12
#define LED_B     			LATCbits.LATC12
#define LED_4				LATBbits.LATB14
#define LED_4_PIN			TRISBbits.TRISB14
#define LED_5				LATBbits.LATB11
#define LED_5_PIN			TRISBbits.TRISB11
#define LED_INIT_PINS()     {\
							LED_4_PIN=0;LED_4=1;\
							LED_5_PIN=0;LED_5=1;\
							LED_G_PIN=LED_R_PIN=LED_B_PIN=0;LED_G=LED_R=LED_B=1;}
#else
#define LED_R_PIN           TRISBbits.TRISB14
#define LED_R              	LATBbits.LATB14 
#define LED_G_PIN  			TRISBbits.TRISB11        
#define LED_G               LATBbits.LATB11
#define LED_B_PIN           TRISCbits.TRISC12
#define LED_B     			LATCbits.LATC12
#define LED_INIT_PINS()     {\
							TRISGbits.TRISG9=0;LATGbits.LATG9=0;\
							TRISBbits.TRISB5=0;LATBbits.LATB5=0;\
							LED_G_PIN=LED_R_PIN=LED_B_PIN=0;LED_G=LED_R=LED_B=1;}
#endif

// LED suspend/resume/on/off definitions
#define LED_TURN_ON		0
#define LED_TURN_OFF 	1
#define LED_SUSPEND()       { LED_R_PIN = 1; LED_G_PIN = 1; LED_B_PIN = 1; }
#define LED_RESUME()        { LED_R_PIN = 0; LED_G_PIN = 0; LED_B_PIN = 0; }
// LED set (0bRGB) - should compile to three bit set/clears with a static colour value
#define LED_SET(_c) {\
				if ((_c) & 0x4) { LED_R = LED_TURN_ON; } else { LED_R = LED_TURN_OFF; } \
				if ((_c) & 0x2) { LED_G = LED_TURN_ON; } else { LED_G = LED_TURN_OFF; } \
				if ((_c) & 0x1) { LED_B = LED_TURN_ON; } else { LED_B = LED_TURN_OFF; } }

// Analogue settings
#define ADC_INDEX_BATT		1
#define ADC_INDEX_LIGHT		2
#define ADC_INDEX_TEMP		0
#define ANALOGUE_SETTLING_WAIT	1
#define ADC_SELECT_H		0b1111111111111111

#ifdef AX9_HW_V2_RGB
#define ADC_SELECT_L				0b0101111110111111
#define ANALOGUE_VREF_MV			3000
#else
#define ADC_SELECT_L				0b0110111110111111
#define ANALOGUE_VREF_MV			3300
#endif

#define ANALOGUE_INIT_PINS()	{	ANALOG_EN_PIN = 0; ANALOG_EN = 0;\
									BATT_MON_PIN = LIGHT_OUT_PIN = TEMP_OUT_PIN = 1;\
									AD1PCFGL = ADC_SELECT_L;AD1PCFGH = ADC_SELECT_H;}
				
// Battery - AN12/13
#ifdef AX9_HW_V2_RGB
#define BATT_MON_PIN        TRISBbits.TRISB13
#else
#define BATT_MON_PIN        TRISBbits.TRISB12
#endif
#define BATT_CHARGE_MIN_SAFE	520		// 3.35v
#define BATT_CHARGE_FULL_USB	636		// 4.09v

// Analogue sensors
#define ANALOG_EN_PIN		TRISBbits.TRISB10    
#define ANALOG_EN			LATBbits.LATB10 

// Light sensor - AN15
#define LIGHT_OUT_PIN       TRISBbits.TRISB15

// Temp sensor - AN6
#define TEMP_OUT_PIN		TRISBbits.TRISB6


// Nand Flash Memory - Spansion, 512MB or 1GB
#define NAND_DEVICE 		NAND_DEVICE_S34ML08G1
#define NAND_DEVICE_ALT		NAND_DEVICE_S34ML04G1
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
#define FLASH_CE1_PIN  	TRISFbits.TRISF1	
#define FLASH_CE1		LATFbits.LATF1
#define FLASH_CE2_PIN  	TRISGbits.TRISG7  
#define FLASH_CE2		LATGbits.LATG7                
#define FLASH_CLE_PIN 	TRISDbits.TRISD7  
#define FLASH_CLE		LATDbits.LATD7      
#define FLASH_ALE_PIN 	TRISFbits.TRISF0   
#define FLASH_ALE		LATFbits.LATF0      
#define FLASH_RB_PIN 	TRISGbits.TRISG8    
#define FLASH_RB		PORTGbits.RG8 
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
// AX9 uses two busses, I2C2 contains the 9-axis sensor combo.
// I2C bus for multi sensor? Use bus 2
#define MULTI_SENSOR_ALT_I2C 2

// Defines for myI2C.c on I2C1
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
#define mySCL				LATDbits.LATD10
#define mySCLr				PORTDbits.RD10
#define mySCLw				LATDbits.LATD10
#define mySCLd				TRISDbits.TRISD10	
#define mySDAr				PORTDbits.RD9
#define mySDAw				LATDbits.LATD9	
#define mySDAd				TRISDbits.TRISD9

// If a bus lockup occurs the following will clear it
#define myI2C1clear()		{{mySCLd=1;mySDAd=1;}unsigned char i;mySCLd=0;for(i=10;i;i--){mySCL=0;Delay10us(1);mySCL=1;Delay10us(1);}mySCLd=1;}

/*for SW code use : {mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}*/
#define mySCL2				LATFbits.LATF5
#define mySCL2r				PORTFbits.RF5
#define mySCL2w				LATFbits.LATF5
#define mySCL2d				TRISFbits.TRISF5	
#define mySDA2r				PORTFbits.RF4
#define mySDA2w				LATFbits.LATF4	
#define mySDA2d				TRISFbits.TRISF4

// If a bus lockup occurs the following should clear it
#define myI2C2clear()		{{mySCL2d=1;mySDA2d=1;}unsigned char i;mySCL2d=0;for(i=10;i;i--){mySCL2=0;Delay10us(1);mySCL2=1;Delay10us(1);}mySCL2d=1;}

// Clear buss'
#define myI2Cclear() 		{myI2C1clear();myI2C2clear();}

// Scriptor I2C definitions
#define SCLr				PORTDbits.RD10
#define SCLw				LATDbits.LATD10
#define SCLd				TRISDbits.TRISD10	
#define SDAr				PORTDbits.RD9
#define SDAw				LATDbits.LATD9	
#define SDAd				TRISDbits.TRISD9

// Accelerometer MMA8451Q
#define ACCEL_MMA8451Q
#define ACCEL_I2C_MODE
#define ACCEL_FIFO_WATERMARK 25
#define ACCEL_INT1_PIN      TRISBbits.TRISB8 
#define ACCEL_INT2_PIN      TRISBbits.TRISB7 	
#define ACCEL_INT1          PORTBbits.RB8
#define ACCEL_INT2          PORTBbits.RB7 

#define ACCEL_INT2_VECTOR	_IC4Interrupt
#define ACCEL_INT2_IF		IFS2bits.IC4IF
#define ACCEL_INT2_IP		IPC9bits.IC4IP
#define ACCEL_INT2_IE		IEC2bits.IC4IE
#define ACCEL_INT1_VECTOR	_INT1Interrupt
#define ACCEL_INT1_IP       IPC5bits.INT1IP 
#define ACCEL_INT1_IF       IFS1bits.INT1IF 
#define ACCEL_INT1_IE     	IEC1bits.INT1IE
#define ACCEL_INIT_PINS()  	{IC4CON1 = 7;\
							ACCEL_INT1_PIN	= 1;\
							ACCEL_INT2_PIN	= 1;\
							TRISCbits.TRISC15 = 0; LATCbits.LATC15 = 1; /*SPI mode CS pin high*/\
							TRISDbits.TRISD3 = 0; LATDbits.LATD3 = 0; /*SDO/A0 pin low*/\
							}
							
// Gyro L3G4200
#define USE_GYRO
#define GYRO_IS_L3G4200D
#define GYRO_I2C_MODE
#define GYRO_FIFO_WATERMARK	25  	  
#define GYRO_INT2_PIN     	TRISDbits.TRISD11     	  
#define GYRO_INT2 			PORTDbits.RD11 
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
#define MAG_INT_PIN     	TRISBbits.TRISB9  
#define MAG_INT        		PORTBbits.RB9    
#define MAG_INT_IF 			IFS2bits.IC3IF
#define MAG_INT_IE  		IEC2bits.IC3IE
#define MAG_INT_IP			IPC9bits.IC3IP
#define MAG_INIT_PINS()  	{IC3CON1 = 7;MAG_INT_PIN = 1;MAG_INT_IP=MAG_INT_PRIORITY;}
#define MAG_ENABLE_INTS()	{MAG_INT_IE=1;}
#define MAG_DISABLE_INTS()	{MAG_INT_IE=0;}
							
// Altimeter BMP180 on bus 1
#define ALT_ALTERNATE_I2C	1
#define ALT_IGNORE_EOC
#define USE_CTRL_REG_BUSY_BIT
#define ALT_INIT_PINS()  	{}

// Bluetooth module test PAN1315 
#define BT_EN_PIN		TRISDbits.TRISD8
#define BT_EN			LATDbits.LATD8
#define BT_CTS_PIN 		TRISDbits.TRISD6
#define BT_CTS			LATDbits.LATD6
#define BT_RTS_PIN		TRISBbits.TRISB4
#define BT_RTS			PORTBbits.RB4
#define BT_RX_PIN		TRISDbits.TRISD2
#define BT_TX_PIN		TRISDbits.TRISD1

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

// BT Clock - routed sosc through comparator CM3 to RP11
#define BT_SLW_CLK_PIN		TRISDbits.TRISD0
#define BT_SLW_CLK			LATDbits.LATD0
#define STOP_BT_SLW_CLK()	{CM3CONbits.CEN=0;CVRCONbits.CVREN=0;BT_SLW_CLK_PIN=BT_SLW_CLK=0;}
#define SETUP_BT_SLW_CLK()	{\
							CM3CONbits.CEN = 1;\
							CM3CONbits.COE = 1;\
							CM3CONbits.CREF = 1;\
							CM3CONbits.CCH = 0b10;/*Input D and Vref*/\
							CVRCONbits.CVREN = 1;\
							CVRCONbits.CVRSS = 0;\
							CVRCONbits.CVRR = 0;\
							CVRCONbits.CVR = 0x8;/*Vref = Vdd/2 */\
							/*Note: C3OUT to RP11*/\
							BT_SLW_CLK_PIN = 0;}		

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
									
#endif


