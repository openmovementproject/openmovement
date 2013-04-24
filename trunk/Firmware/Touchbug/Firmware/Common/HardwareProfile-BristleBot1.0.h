// Hardware profile for the CWA 1.7 (using PIC24FJ256GB106)
// Karim Ladha, Dan Jackson, 2011

#ifndef HARDWAREPROFILE_CWA17_H
#define HARDWAREPROFILE_CWA17_H

    #ifndef __PIC24FJ256GB106__
        #error "Unexpected device selection"
    #endif

    #ifndef __C30__
        #error "Unexpected compiler"
    #endif

    #define 	PIC24
    #define 	CWA3_1
    #define HARDWARE_VERSION 0x31           // BCD format (for USB response)

	// Interrupt priority levels
	// Bluetooth UART ints
	#define BT_UART_RX_INT_PRIORITY			7 	/* This makes the uart appear to be a DMA device to the CPU if IPL is < 7*/
	#define BT_UART_TX_INT_PRIORITY			6	/* This makes the uart appear to be a DMA device to the CPU if IPL is < 6*/

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
//#define DISP_POWER_DOWN			0x800
#define GYRO_POWER_DOWN			0x1000
#define ACCEL_POWER_DOWN		0x2000
#define SAVE_INT_STATUS			0x4000
#define ALLOW_VECTOR_ON_WAKE	0x8000
#define LEAVE_WDT_RUNNING		0x10000
#define JUST_POWER_DOWN_PERIPHERALS	0x20000
#define DONT_RESTORE_PERIPHERALS	0x40000
#define WAKE_ON_BT_MODULE 			0x80000
//#define MAG_POWER_DOWN				0x100000
#define BT_POWER_DOWN				0x200000
//#define WAKE_ON_MAG					0x400000

// Essential Functions in HardwareProfile.c
extern void WaitForPrecharge(void);
extern void SystemPwrSave(unsigned long NapSetting);

// LED Colours 0bRGB
enum { LED_OFF, LED_BLUE, LED_GREEN, LED_CYAN, LED_RED, LED_MAGENTA, LED_YELLOW, LED_WHITE };

	// Used for delays in TimeDelay.h
	#define GetInstructionClock()  4000000

    // USB bus sense I/O
    // #define USE_USB_BUS_SENSE_IO
	#define USB_BUS_SENSE_PIN	TRISDbits.TRISD11
    #define USB_BUS_SENSE       PORTDbits.RD11
	#define USB_BUS_SENSE_INIT() USB_BUS_SENSE_PIN=1
	// Change notification int,  vectors to button handler
	#define USB_INIT_BUS_SENSE_INTS()	{/*Change notification interrupt CN6 - for wake up purposes*/\
										CNEN4bits.CN56IE;\
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
							ANALOGUE_INIT_PINS();\
							IR_TX_INIT_PINS();\
							BUTTON_INIT_PINS();\
							IR_RX_INIT_PINS();\
							MOTOR_INIT_PINS();\
							LED_INIT_PINS();\
                            ACCEL_INIT_PINS();\
							GYRO_INIT_PINS();\
							BT_INIT_PINS();\
							BATT_INIT_PINS();\
                            LDR_INIT_PINS();\
							BUTTON_INIT_PINS();\
							DRIVE_FLOATING_PINS(); /*Reduce leakage*/\
                            USB_BUS_SENSE_INIT();\
                            }

	#define DRIVE_FLOATING_PINS() {\
                            TRISEbits.TRISE0 = 0; LATEbits.LATE0 = 0;\
                            TRISEbits.TRISE4 = 0; LATEbits.LATE4 = 0;\
                            TRISEbits.TRISE5 = 0; LATEbits.LATE5 = 0;\
                            TRISDbits.TRISD4 = 0; LATDbits.LATD4 = 0;\
                            TRISDbits.TRISD5 = 0; LATDbits.LATD5 = 0;\
                            TRISDbits.TRISD6 = 0; LATDbits.LATD6 = 0;\
                            TRISDbits.TRISD7 = 0; LATDbits.LATD7 = 0;\
                            TRISGbits.TRISG8 = 0; LATGbits.LATG8 = 0;\
                            TRISBbits.TRISB3 = 0; LATBbits.LATB3 = 0;\
                            TRISBbits.TRISB11 = 0; LATBbits.LATB11 = 0;\
                            TRISFbits.TRISF0 = 0; LATFbits.LATF0 = 0;\
                            TRISFbits.TRISF1 = 0; LATFbits.LATF1 = 0;\
                            TRISCbits.TRISC12 = 0; LATCbits.LATC15 = 0;\
                            TRISBbits.TRISB0 = 0; LATBbits.LATB0 = 0;/*PGD*/\
                            TRISBbits.TRISB1 = 0; LATBbits.LATB1 = 0;/*PGC*/\
                            }
							
	// REMAP pins
	#define REMAP_PINS()	{\
							__builtin_write_OSCCONL(OSCCON & 0xBF);/*Unlock*/\
							/*INPUTS*/\
							RPINR0bits.INT1R = 26;/* AccInt1 RP26 -> Int1 */\
							RPINR1bits.INT2R = 21;/* AccInt2 RP21 -> Int2 */\
							RPINR1bits.INT3R = 22;/* GyroInt2 dtrdy RP22 -> Int3*/\
							/*RPINR2bits.INT4R = --;DO NOT USE - SW INT*/\
							RPINR18bits.U1CTSR = 10;/*RP10 = U1CTS (BT_RTS)*/\
							RPINR18bits.U1RXR = 14;	/*RP14 = U1RX (BT_TX)*/\
							/*OUTPUTS*/\
							RPOR14bits.RP29R = 3;	/*RP29 = U1TX (BT_RX)*/\
							/*RPOR8bits.RP17R = 4;*/ 	/*RP17 = U1RTS (BT_CTS)*/\
							RPOR1bits.RP2R = 36;	/*Blutooth 32kHz clock routed through comparator3*/\
							RPOR4bits.RP9R = 18;	/* OC1 module to motor1 - PWM*/\
							RPOR11bits.RP23R = 19;	/* OC2 module to motor2 - PWM*/\
							RPOR4bits.RP8R = 20;	/* OC3 module to motor3 - PWM*/\
							RPOR1bits.RP3R = 21;	/* OC4 module to motor4 - PWM*/\
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
    #define LED_G_PIN  			TRISBbits.TRISB6        
    #define LED_G               LATBbits.LATB6
    #define LED_R_PIN           TRISBbits.TRISB2
    #define LED_R              	LATBbits.LATB2
    #define LED_B_PIN           TRISBbits.TRISB7
    #define LED_B     			LATBbits.LATB7
    #define LED_G1_PIN  		TRISBbits.TRISB6        
    #define LED_G1              LATBbits.LATB6
    #define LED_R1_PIN          TRISBbits.TRISB2
    #define LED_R1              LATBbits.LATB2
    #define LED_B1_PIN          TRISBbits.TRISB7
    #define LED_B1     			LATBbits.LATB7
    #define LED_G2_PIN  		TRISEbits.TRISE3        
    #define LED_G2              LATEbits.LATE3
    #define LED_R2_PIN          TRISEbits.TRISE2
    #define LED_R2              LATEbits.LATE2
    #define LED_B2_PIN          TRISEbits.TRISE1
    #define LED_B2     			LATEbits.LATE1
    #define LED_INIT_PINS()     {	LED_G1_PIN=LED_G1=LED_R1_PIN=LED_R1=LED_B1_PIN=LED_B1=0;\
									LED_G2_PIN=LED_G2=LED_R2_PIN=LED_R2=LED_B2_PIN=LED_B2=0;}

    // LED set (0bRGB) - should compile to three bit set/clears with a static colour value
    #define LED_SET1(_c) {\
                    if ((_c) & 0x4) { LED_R1 = 1; } else { LED_R1 = 0; } \
                    if ((_c) & 0x2) { LED_G1 = 1; } else { LED_G1 = 0; } \
                    if ((_c) & 0x1) { LED_B1 = 1; } else { LED_B1 = 0; } \
                    }
    #define LED_SET2(_c) {\
                    if ((_c) & 0x4) { LED_R2 = 1; } else { LED_R2 = 0; } \
                    if ((_c) & 0x2) { LED_G2 = 1; } else { LED_G2 = 0; } \
                    if ((_c) & 0x1) { LED_B2 = 1; } else { LED_B2 = 0; } \
                    }
    #define LED_SET(_c) {LED_SET1(_c);LED_SET2(_c);}


    // Battery - B12/AN12
    #define BATT_MON_PIN        TRISBbits.TRISB12
    #define BATT_INIT_PINS()   	BATT_MON_PIN = 1
    #define BATT_CHARGE_ZERO		614		// FIXED CONST, DO NOT USE
	#define BATT_CHARGE_FULL		708		// FIXED CONST, DO NOT USE
	#define BATT_CHARGE_MIN_LOG		517		// minimum level to update log
	#define BATT_CHARGE_MIN_SAFE	520		// minimum safe running voltage (5%), avoids unwanted RTC resets and file corruptions etc.
    #define BATT_CHARGE_MID_USB		666     // level to count towards recharge cycle counter (75%)
    #define BATT_CHARGE_FULL_USB	708


    // LDR - B5/AN5
    #define LDR_EN_PIN			TRISGbits.TRISG9         
    #define LDR_EN              LATGbits.LATG9
    #define LDR_OUT_PIN         TRISBbits.TRISB5
    #define LDR_ENABLE()        {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 0;}
    #define LDR_DISABLE()       {LDR_OUT_PIN=1;LDR_EN_PIN = 0;LDR_EN = 1;}
    #define LDR_INIT_PINS()     LDR_DISABLE()

	// IR detectors - AN10, AN13
	#define IR_DET1_PIN	TRISBbits.TRISB10
	#define IR_DET1		PORTBbits.PORTB10
	#define IR_DET2_PIN TRISBbits.TRISB13
	#define IR_DET2		PORTBbits.PORTB13
	#define IR_DET_PWR_PIN	TRISBbits.TRISB4
	#define IR_DET_PWR		LATBbits.LATB4
	#define IR_DET_ENABLE()		{IR_DET_PWR_PIN = 0; IR_DET_PWR = 1;}
	#define IR_DET_DISABLE()	{IR_DET_PWR_PIN = 0; IR_DET_PWR = 0;IR_DET1_PIN = 1; IR_DET2_PIN = 1;}

	// Analogue select pins - 0 = Analogue
	#define ADC_SELECT_L		0b1100101111011111
	#define ADC_SELECT_H		0b1111111111111111
	#define ANALOGUE_INIT_PINS()	{AD1PCFGL = ADC_SELECT_L;AD1PCFGH = ADC_SELECT_H;}
    #define ADC_INDEX_BATT		2
    #define ADC_INDEX_LDR		0
    #define ADC_INDEX_IR1		1
    #define ADC_INDEX_IR2		3

	// Several devices can use the change notification interrups
	#define CN_INTS_ON()	{IFS1bits.CNIF = 0;IPC4bits.CNIP = CN_INTS_PROCESSOR_PRIORITY; IEC1bits.CNIE = 1;}
	#define CN_INTS_OFF()	{IEC1bits.CNIE = 0;}
	#define CN_INT_FIRE()	{IFS1bits.CNIF = 1;}

	// For the following 4 peripherals the interface is via I2C using these pins
	// All the pins need re-mapping. See RemapPins() in this file.

	// Defines for myI2C.c
	#define USE_HW_I2C3
	//#define USE_MY_SW_I2C
	#ifndef USE_MY_SW_I2C
		#define InitI2C()			{myI2Cclear();mySCLd = 1;mySDAd=1;I2C1CON=0;myI2CIdle();}
	#else
		#define InitI2C()			{myI2Cclear();mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}
	#endif
	/*for SW code use : {mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}*/
	#define mySCL				LATEbits.LATE6
	#define mySCLd				TRISEbits.TRISE6	
	#define mySDAr				PORTEbits.RE7
	#define mySDAw				LATEbits.LATE7	
	#define mySDAd				TRISEbits.TRISE7
	// If a bus lockup occurs the following will clear it
	#define myI2Cclear()		{unsigned char i;mySCLd=0;for(i=10;i;i--){mySCL=0;Delay10us(1);mySCL=1;Delay10us(1);}}

	/* I2C devices that may be present
	Accelerometer 	MMAQ8452Q 	0x38
	Gyroscope 		L3G4200D	0xD2
	*/

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
								
	// Gyro L3G4200
	#define USE_GYRO
	#define GYRO_IS_L3G4200D
	#define GYRO_I2C_MODE
	#define GYRO_FIFO_WATERMARK	25
    #define GYRO_INT1_PIN     	
    #define GYRO_INT2_PIN     	TRISDbits.TRISD3   
    #define GYRO_INT1           
    #define GYRO_INT2 			PORTDbits.RD3
//    #define GYRO_INT1_IP        IPC13bits.INT3IP // Data ready on GYRO_INT2
//    #define GYRO_INT1_IF        IFS3bits.INT3IF
//    #define GYRO_INT1_IE        IEC3bits.INT3IE
    #define GYRO_INT2_IP        IPC13bits.INT3IP    
    #define GYRO_INT2_IF        IFS3bits.INT3IF 
    #define GYRO_INT2_IE        IEC3bits.INT3IE  

    #define GYRO_INIT_PINS()  	{\
								GYRO_INT2_PIN	= 1;\
								}
								
	// Bluetooth module test PAN1315 - labled as on schematic, ref to slave
	#define BT_SLW_CLK_PIN	TRISDbits.TRISD8
	#define BT_SLW_CLK	LATDbits.LATD8
	#define BT_EN_PIN	TRISFbits.TRISF3
	#define BT_EN		LATFbits.LATF3
	#define BT_CTS_PIN 	TRISFbits.TRISF5
	#define BT_CTS		LATFbits.LATF5
	#define BT_RTS_PIN	TRISFbits.TRISF4
	#define BT_RTS		PORTFbits.RF4
	#define BT_RX_PIN	TRISBbits.TRISB15
	#define BT_TX_PIN	TRISBbits.TRISB14
	
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
								/*Note: C3OUT to RP2*/\
								BT_SLW_CLK_PIN = 0;}	

	
	// Button - active low
	#define BUTTON_PIN		TRISDbits.TRISD9
	#define BUTTON			PORTDbits.RD9
	#define BUTTON_INIT_PINS()	{BUTTON_PIN=1;}
	#define BUTTON_ENABLE_INTS()		{/*Interrupt on change CN18*/\
										CNEN4bits.CN54IE;\
										CN_INTS_ON();\
										}

	// Motor control pins
	#define MOTOR1_PIN		TRISBbits.TRISB9
	#define MOTOR1			LATBbits.LATB9
	#define MOTOR2_PIN		TRISDbits.TRISD2
	#define MOTOR2			LATDbits.LATD2
	#define MOTOR_INIT_PINS() 	{MOTOR1=0;MOTOR1_PIN=0;MOTOR2=0;MOTOR2_PIN=0;}
	#define MOTOR_INIT_PWM()	{\
								/* Setup PWM output OC1 - mode1 0% duty*/\
								OC1RS				= 0xffff;	/* Period is 1/61Hz */\
								OC1R				= 0;		/* 0% initial duty o/p*/\	
								OC1CON1bits.OCTSEL  = 0b111; 	/* System clock used for PWM (turn off before sleep)*/\
								OC1CON1bits.OCM		= 0b110; 	/* Edge aligned PWM mode*/\
								OC1CON2bits.SYNCSEL = 0x1f;		/* Sync to this module*/\
								/* Setup PWM output OC2 - mode1 0% duty*/\
								OC2RS				= 0xffff;	/* Period is 1/61Hz */\
								OC2R				= 0;		/* 0% initial duty o/p*/\	
								OC2CON1bits.OCTSEL  = 0b111; 	/* System clock used for PWM (turn off before sleep)*/\
								OC2CON1bits.OCM		= 0b110; 	/* Edge aligned PWM mode*/\
								OC2CON2bits.SYNCSEL = 0x1f;		/* Sync to this module*/	}
	#define MOTOR_DISABLE()		{OC1CON1 = OC2CON1 = 0;MOTOR_INIT_PINS();}
	#define MOTOR1_DUTY			OC1R
	#define MOTOR2_DUTY			OC2R
							

	// IR transmision LEDs
	#define IR_TX1_PIN		TRISBbits.TRISB8
	#define IR_TX1			LATBbits.LATB8
	#define IR_TX2_PIN		TRISDbits.TRISD10
	#define IR_TX2			LATDbits.LATD10
	#define IR_TX_INIT_PINS()	{IR_TX1 = IR_TX1_PIN = IR_TX2 = IR_TX2_PIN =0;}
	#define IR_TX_INIT_PWM()	{\
								/* Setup PWM output OC3 - mode1 0% duty*/\
								OC3RS				= 0x00ff;	/* Period is 1/15.6kHz */\
								OC3R				= 0;		/* 0% initial duty o/p*/\	
								OC3CON1bits.OCTSEL  = 0b111; 	/* System clock used for PWM (turn off before sleep)*/\
								OC3CON1bits.OCM		= 0b110; 	/* Edge aligned PWM mode*/\
								OC3CON2bits.SYNCSEL = 0x1f;		/* Sync to this module*/\
								/* Setup PWM output OC4 - mode1 0% duty*/\
								OC4RS				= 0x00ff;	/* Period is 1/15.6kHz */\
								OC4R				= 0;		/* 0% initial duty o/p*/\	
								OC4CON1bits.OCTSEL  = 0b111; 	/* System clock used for PWM (turn off before sleep)*/\
								OC4CON1bits.OCM		= 0b110; 	/* Edge aligned PWM mode*/\
								OC4CON2bits.SYNCSEL = 0x1f;		/* Sync to this module*/	}
	#define IR_TX_DISABLE()		{OC3CON1 = OC3CON1 = 0;IR_TX_INIT_PINS();}
	#define IR_TX1_DUTY			OC3R
	#define IR_TX2_DUTY			OC4R

	// PWM outputs
	/*Common functions to all PWM peripherals*/ 
	#define PWM_PERIPHERALS_ON()	{MOTOR_INIT_PWM();IR_TX_INIT_PWM();}
	#define PWM_PERIPHERALS_OFF()	{MOTOR_DISABLE();IR_TX_DISABLE();}

	// IR receiver
	#define IR_RX_PWR_PIN		TRISDbits.TRISD0
	#define IR_RX_PWR			LATDbits.LATD0
	#define IR_RX_EN()			{IR_RX_PWR_PIN = 0; IR_RX_PWR = 1;}
	#define IR_RX_OUT_PIN		TRISDbits.TRISD1
	#define IR_RX_INIT_PINS()	{IR_RX_PWR_PIN = IR_RX_PWR = 0;IR_RX_OUT_PIN = 1;}
	#define IR_RX_OPEN_UART(_baud)		OpenUART2 (	/*U1MODEvalue*/(UART_EN&UART_IDLE_STOP&UART_IrDA_DISABLE&UART_DIS_WAKE&UART_DIS_LOOPBACK&UART_DIS_ABAUD&UART_UXRX_IDLE_ONE&UART_BRGH_FOUR&UART_NO_PAR_8BIT&UART_1STOPBIT&UART_MODE_FLOW&UART_UEN_00),\
													/*U1STAvalue*/ (UART_INT_TX_LAST_CH&UART_TX_ENABLE&UART_INT_RX_CHAR&UART_ADR_DETECT_DIS&UART_RX_OVERRUN_CLEAR&UART_IrDA_POL_INV_ZERO&UART_SYNC_BREAK_DISABLED),\
													/*baud*/ _baud);


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

