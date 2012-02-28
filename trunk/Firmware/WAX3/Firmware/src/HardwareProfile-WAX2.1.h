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
 */

// HardwareProfile-WAX.h - Device hardware profile.

#ifndef HARDWAREPROFILE_WAX_H
#define HARDWAREPROFILE_WAX_H

	#define WIFCO_RECEIVER

	// Tweak settings for WAX hand-out demo devices (default to OSC messages at startup, fast transmit rate, etc.)
	//#define DEMO_DEVICE	
	
	// WAX+Gyro Inertial Measurement Unit transmitter (only when DEVICE_ID > 0)
//#define IMU_DEVICE
//#define IMU_REMOVE_ACCEL
	
	// TEDDI measurement unit (when DEVICE_ID & 0xff > 0), or TEDDI-compatible receiver (when DEVICE_ID & 0xff == 0)
//	#define TEDDI_DEVICE
	// TEDDI_AUDIO_INTERRUPT -- process audio in an interrupt (otherwise in main loop)
	//#define TEDDI_AUDIO_INTERRUPT


	// Compile-time device identifier (lowest 16-bits of address) -- IMPORTANT: low-byte zero will compile as a receiver
	#define DEVICE_ID 0			// Receiver=0, transmitter=1+

		
	// Use new packed format for accelerometer data
	#define ACCEL_DWORD_FORMAT

	// Place MiWi stack into "Turbo Mode" (not compatible with packet sniffer, lower range)
	#ifdef WIFCO_RECEIVER
		#define TURBO_MODE
	#endif

	// Initial channel config -- old WAX was on 12, clearest channels likely to be 15, 21, 22.
	#ifdef WIFCO_RECEIVER
		#define DEBUG_FIXED_CHANNEL 15		// WIFCO default is 15 (11-26)
	    #define DEFAULT_PAN 0x1234			// WIFCO default PAN id 0x1234 -- (note: need to alter MiWi stack if we want to be able to change this at run-time)
	#elif defined(TEDDI_DEVICE)
		#define DEBUG_FIXED_CHANNEL 21		// TEDDI default is 21 (11-26)
	    #define DEFAULT_PAN 0x7ED1			// TEDDI default PAN id 0x7ED1 -- (note: need to alter MiWi stack if we want to be able to change this at run-time)
	#elif defined(IMU_DEVICE)
		#define DEBUG_FIXED_CHANNEL 11		// IMU_DEVICE default is 11 (11-26)
	    #define DEFAULT_PAN 0x4321			// IMU_DEVICE default PAN id 0x4321 -- (note: need to alter MiWi stack if we want to be able to change this at run-time)
	#else
		#define DEBUG_FIXED_CHANNEL 15		// WAX default is 15 (11-26)
	    #define DEFAULT_PAN 0x1234			// WAX default PAN id 0x1234 -- (note: need to alter MiWi stack if we want to be able to change this at run-time)
	#endif


    // Choose transmitter/receiver based on identifier...
	#if ((DEVICE_ID & 0xFF) == 0)
	    #define DEVICE_TYPE 0								// 0 = FFD Attached receiver (compatible with any transmitters)
	#else
		// Choose transmitter type based on defines...
		#if defined(WIFCO_RECEIVER) && !defined(IMU_DEVICE) && !defined(TEDDI_DEVICE)
		    #define DEVICE_TYPE 0							// 0 = WIFCO is always a FFD Attached receiver
		#elif !defined(WIFCO_RECEIVER) && !defined(IMU_DEVICE) && defined(TEDDI_DEVICE)
		    #define DEVICE_TYPE 3							// 3 = RFD TEDDI
		#elif !defined(WIFCO_RECEIVER) && defined(IMU_DEVICE) && !defined(TEDDI_DEVICE)
		    #define DEVICE_TYPE 2							// 2 = RFD WAX+GYRO
		#elif !defined(WIFCO_RECEIVER) && !defined(IMU_DEVICE) && !defined(TEDDI_DEVICE)
		    #define DEVICE_TYPE 1							// 1 = RFD WAX
	    #else
	    	#error "DEVICE_TYPE cannot be decided - check for incompatible #define directives"
	    #endif
	#endif
	
	#if defined(WIFCO_RECEIVER) && (DEVICE_ID != 0)
    	#warning "Device ID for WIFCO receiver should be 0"
	#endif

	// Both transmitter/receivers now have 48 MHz system clock while attached to USB
	#define FAST_USB_CLOCK

	// Debugging options
	#if (DEVICE_TYPE == 0)  // FFD Receiver
		// Receivers only have the radio on while attached to USB (radio code compiles with fast-clock timing)
		#define RADIO_WHEN_ATTACHED
		//#define DEBUG_VERBOSE
	#else                   // RFD Transmitter
		#if (DEVICE_TYPE == 3)
			#define FAST_WHEN_DETATCHED
		#endif
		//#define DEBUG_INTERRUPTS
		//#define DEBUG_VERBOSE
		//#define DEBUG_PACKETS
		//#define DEBUG_SEND_DUMMY_PACKET
		//#define DEBUG_RECEIVED_PACKET_SEQUENCE
	#endif

    // Compiler must be C18
    #if !defined(__18CXX)
      #error "ERROR: Compiler not supported."
    #endif
    #define USE_PIC18

    // Processor type should be: PIC18F26J50
    #if !defined(__18F26J50)
      #error "ERROR: Processor type is not supported."
    #endif
	
    // Include processor header
    #include <p18cxxx.h>     // Auto-select header  // #include <p18f26j50.h>


    // [Not used] Make the output HEX of this project able to be bootloaded using the HID bootloader
    #define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER
    
	// Erase block size for this PIC
	#define ERASE_BLOCK_SIZE 1024

	// Serial number and ROM config in PIC memory f800 to fbff (1024 bytes), check matches with linker and HardwareProfile.h #define IDLOC
    #define IDLOC ((unsigned const rom unsigned char *)0xf800L)


    #ifdef __DEBUG
	    // Don't use the WDT in debug builds
	#else
	    // Use the WDT in release builds
        //#define USE_WDT
    #endif

    // Board definition for WAX (PIC18F26J50)
    #define WAX 0x21     // WAX V1
    #define DEVICE_PROFILE WAX

	#define HARDWARE_VERSION WAX
	#define SOFTWARE_VERSION 0x27

	// Clock frequency for RF timing
	#if defined(FAST_USB_CLOCK) && (defined(RADIO_WHEN_ATTACHED) || defined(FAST_WHEN_DETATCHED))
		// Receivers only have the radio on while attached to USB (48 MHz MiWi timing)
	    #define CLOCK_FREQ 48000000
	#else                   // RFD
		// Transmitters only have the radio on while detatched from USB (8 MHz MiWi timing)
	    #define CLOCK_FREQ 8000000
	#endif
	
    //#define GetSystemClock()  CLOCK_FREQ      
	#define GetInstructionClock() (CLOCK_FREQ)

    // Transmit jitter (TODO: base this value on expected interval instead)
#if defined(TEDDI_DEVICE)
    #define TRANSMIT_INTERVAL 0              // Not using transmit timer
    #define JITTER_MASK	0x1f                 // Not using transmit timer - this jitter delay is in 10k Tcy, only bottom 8-bits valid (48 MHz clock -> 0.833 msec per unit), 31 ~= 26 msec
#elif defined(DEMO_DEVICE) || defined(IMU_DEVICE)
    #define TRANSMIT_INTERVAL (20 * 250UL)      // 25 msec -- lower bound (will be plus jitter) [4us per tick (250 kHz), 1 ms = 250 ticks]
    #define JITTER_MASK	0x07ff               // up to 8 msec jitter could be added
#else
    #define TRANSMIT_INTERVAL (125 * 250UL)     // 125 msec -- lower bound (will be plus jitter) [4us per tick (250 kHz), 1 ms = 250 ticks]
    #define JITTER_MASK	0x1fff               // up to 32 msec jitter could be added
#endif

	#if (DEVICE_TYPE==3)
		#ifdef TEDDI_AUDIO_INTERRUPT
			#define PRIORITY_INTERRUPT AudioTimerISR
			#define PRIORITY_INTERRUPT_CONDITION if (PIE1bits.TMR2IE)
		#endif
	#endif

    // USART2 TX
    #ifdef __DEBUG
        // Debug mode debugging
        //#define DEBUG_USART2_TX
    #else
        // Release mode debugging
    #endif

    // Remappable pins
    #define REMAP_LOCK()    {\
                                _asm\
                                    MOVLB 0x0E \
                                    BCF INTCON, GIE \
                                    MOVLF 0x55 \
                                    MOVWF EECON2, 0 \
                                    MOVLW EECON2, 0 \
                                    BCF PPSCON, 0 \
                                _endasm \
                            }

    #define REMAP_UNLOCK()    { \
                                _asm \
                                    BCF INTCON, 7 \
                                    MOVLW 0x55 \
                                    MOVWF EECON2, 0 \
                                    MOVLW 0xAA \
                                    MOVWF EECON2, 0 \
                                    BSF PPSCON, 0 \
                                _endasm \
                            }

	#define REMAP_PINS() { \
		RPINR2  =  6;	/* USB_BUS_SENSE_INT */ \
		RPINR1  =  3;	/* RF_INT */ \
		RPINR21 = 12;	/* RF_SPI_SI */ \
		RPOR13  =  9;	/* RF_SPI_SO */ \
		RPINR22 = 11;	/* RF_SPI_SCK_IN */ \
		RPOR11  = 10;	/* RF_SPI_SCK_OUT */ \
		RPINR1  =  0;	/* ACCEL_INT1 */ \
		PPSCONbits.IOLOCK = TRUE; \
	}	


    // USB (MCHPFSUSB framework)
    //#define USE_SELF_POWER_SENSE_IO
    //#define tris_self_power     ?    // Input
    #define self_power          (1)   // 0 = bus-powered, 1 = self-powered

    // USB bus sense I/O
    #define USE_USB_BUS_SENSE_IO
    #define USB_BUS_SENSE_TRIS  TRISBbits.TRISB3    // Input
    #define USB_BUS_SENSE       PORTBbits.RB3
    #define usb_bus_sense       USB_BUS_SENSE
    #define USB_BUS_SENSE_IF    INTCON3bits.INT2IF
    #define USB_BUS_SENSE_IE    INTCON3bits.INT2IE
    #define USB_BUS_SENSE_IP    INTCON3bits.INT2IP
    #define USB_BUS_SENSE_IEDG  INTCON2bits.INTEDG2
	#define USB_IS_DETECTED()   (USB_BUS_SENSE != 0)
    #define USB_INIT_PINS()    	{ \
									ANCON1bits.PCFG9 = 1; /* AN9 shares RB3 */ \
									USB_BUS_SENSE_TRIS = 1; \
									USB_BUS_SENSE_IEDG = 1; \
									USB_BUS_SENSE_IP = 1; \
									USB_BUS_SENSE_IF = 0; \
									USB_BUS_SENSE_IE = 1; \
								}


    // Fast clock: initialize PLL (48 MHz input to USB), CPU at 48 MHz (FAST_USB_CLOCK) or 8 MHz
    #define CLOCK_USB_XTAL() {\
        OSCTUNE = 0b01000000;  		/* PLLEN=1 */ \
        Delay1KTCYx(24*2);            /* Wait 2 ms for PLL to lock:  2m / (4 / 48M) => 24K Tcy  (or 12 ms at 8 MHz) */ \
        OSCCON = 0b01110000;   		/* Switch to primary clock source = 48MHz with CPDIV at 1, or 8MHz with CPDIV at 6 (INTOSC at 8Mhz) */  \
        /* while (!OSCCONbits.OSTS); */   /* Wait until using primary oscillator */ \
        Delay100TCYx(6);            /* Wait 600 cycles */ \
    }

    // Clock: switch to 8 MHz INTOSC with PLL off
    #define CLOCK_INTOSC() { \
        OSCCON = 0b01110011;   		/* Switch to INTOSC at 8Mhz */  \
        OSCTUNE = 0b00000000;  		/* PLLEN=0 */ \
    }

    // Clock: switch to 1 MHz INTOSC with PLL off
    #define CLOCK_INTOSC_1M() {\
        OSCCON = 0b01000011;        /* Switch to INTOSC at 1 MHz */ \
        OSCTUNEbits.PLLEN = 0;      /* Disable PLL */ \
    }

    // Clock: switch to 31 kHz INTRC
    #define CLOCK_INTRC() {\
        OSCTUNEbits.INTSRC = 0;     /* bit-7: 0 = use INTRC for 31 kHz signal (rather than INTOSC/256) */ \
        OSCCON = 0b00000011;        /* Switch to INTOSC at 31 kHz, use INTRC rather than post-scaled internal clock */ \
        /*while (OSCCONbits.OSTS);*//* Wait until not using primary oscillator */ \
        OSCTUNEbits.PLLEN = 0;      /* Disable PLL */ \
    }


    // LED
    #define LED_TRIS            TRISAbits.TRISA7
    #define LED                 LATAbits.LATA7
    #define LED_INIT_PINS()     { LED = 0; LED_TRIS = 0; }
    #define LED_SET(_v)         { LED = (_v) & 1; }


    // Debugging USART transmit pin
    #ifdef DEBUG_USART2_TX
        // USART -- (BRG16 + BRGH = 16-bit asynchronous baud generation):  n = Fosc / rate / 4 - 1
        // 115200 @  4 MHz:  4000000 / 115200 / 4 - 1 =   8 (0x08)
        // 115200 @  8 MHz:  8000000 / 115200 / 4 - 1 =  16 (0x10)
        // 115200 @ 48 MHz: 48000000 / 115200 / 4 - 1 = 103 (0x67)
		#define USART_TRIS      TRISAbits.TRISA5
        #define USART_INIT_PINS() { USART_TRIS = 0; }
    
        #define USARTStartup() {\
                                USART_INIT_PINS();\
                                TXSTA2 = 0x24;\
                                RCSTA2 = 0x90;\
                                SPBRG2 = 0x10; /* BAUD */ \
                                SPBRGH2 = 0x00;\
                                BAUDCON2 = 0x08;\
                            }

		// USART_DT2 -- RP2/RA5 as TX2/CK2 EUSART2 Asynchronous Transmit
		#define USART_REMAP_PINS() { RPOR2 = 5; }

    #endif


    // Battery
    #define BATT_NCHARGING_TRIS  TRISAbits.TRISA6
    #define BATT_NCHARGING       PORTAbits.RA6
    //#define BATT_MON_TRIS      TRISAbits.TRISA0
    //#define BATT_MASK          ADC_CH0
    #define BATT_INIT_PINS()    { BATT_NCHARGING_TRIS = 1; } // BATT_USB_POWER_TRIS = 1; BATT_MON_TRIS = 1; WDTCONbits.ADSHR = 1; ANCON0bits.PCFG0 = 0; WDTCONbits.ADSHR = 0; 

        
    // Accelerometer
    #define ACCEL_USE_SPI       1
    #define ACCEL_CS_TRIS       TRISAbits.TRISA1
    #define ACCEL_CS            LATAbits.LATA1
    #define ACCEL_SCK           PORTBbits.RB4
    #define ACCEL_SCK_TRIS      TRISBbits.TRISB4
    #define ACCEL_SDO           PORTBbits.RB5
    #define ACCEL_SDO_TRIS      TRISBbits.TRISB5     
    #define ACCEL_SDI           LATCbits.LATC7
    #define ACCEL_SDI_TRIS      TRISCbits.TRISC7    

    #define ACCEL_INT1_TRIS     TRISAbits.TRISA0                                        
    //#define ACCEL_INT1_PIN      PORTAbits.RA0
    #define ACCEL_INT1_IF       INTCON3bits.INT1IF
    #define ACCEL_INT1_IE       INTCON3bits.INT1IE
    #define ACCEL_INT1_IP       INTCON3bits.INT1IP
	#define ACCEL_INT1_IEDG		INTCON2bits.INTEDG1

    #define ACCEL_INIT_PINS() {\
                                ACCEL_CS = 1; /* active low */ \
                                ACCEL_CS_TRIS = 0;\
                                ACCEL_SCK_TRIS = 0;\
                                ACCEL_SDO_TRIS = 1;\
                                ACCEL_SDI_TRIS = 0;\
                                ACCEL_INT1_TRIS = 1;\
                                ACCEL_INT1_IEDG = 1;\
								ACCEL_INT1_IF = 0;\
								ACCEL_INT1_IP = 0;\
                              }
                              
                              
	// Expansion board I2C configuration
	// -- (additional pin: TEDDI = power-down; GYRO = interrupt;)
	#if defined(DEBUG_USART2_TX) && (DEVICE_TYPE == 2 || DEVICE_TYPE == 3)
		#error "Cannot use USART debugging with the expansion board."
	#endif

#if (DEVICE_TYPE==3)
	#define PeripheralEn		LATAbits.LATA5
    #define EXP_INIT_PINS()     { mySCL = 1; mySCLd = 0; mySDAw=0; mySDAd=1; ANCON0bits.PCFG4 = 1; PeripheralEn = 0; TRISAbits.TRISA5 = 0; }
#endif
	#define mySCL				LATBbits.LATB6
	#define mySCLd				TRISBbits.TRISB6	
	#define mySDAr				PORTBbits.RB7
	#define mySDAw				LATBbits.LATB7	
	#define mySDAd				TRISBbits.TRISB7
	//#define ExpResetDevices() { myI2CStart(); myI2Cputc(0x00); myI2Cputc(0x06); myI2CStop(); }	 		 // general call reset intruction

	// WAX 'gyro' expansion peripheral board (RA5/AN4/RP2)
	#define GYRO_I2C_MODE
	#define GYRO_INT2_TRIS		TRISAbits.TRISA5
	#define GYRO_INT2_PIN		PORTAbits.RA5	
	#define GYRO_INT2_IF        INTCON3bits.INT3IF
	#define GYRO_INT2_IE        INTCON3bits.INT3IE
	#define GYRO_INT2_IP        INTCON2bits.INT3IP
	#define GYRO_INT2_IEDG		INTCON2bits.INTEDG3
	#define GYRO_REMAP_PINS() { RPINR3 = 2; }
    #define GYRO_INIT_PINS()  { mySCL = 1; mySCLd = 0; mySDAw=0; mySDAd=1; ANCON0bits.PCFG4 = 1; GYRO_INT2_IEDG = 1; GYRO_INT2_IP = 0; GYRO_INT2_IE = 0; GYRO_INT2_TRIS = 1; GYRO_INT2_IF = 0; }

    // RF module interface (from Zena Stack Configuration for MiWi protocol)
    #define TMRL                TMR0L
    #define RFIF                INTCONbits.INT0IF
    #define RFIE                INTCONbits.INT0IE
    #define RF_IEDG  			INTCON2bits.INTEDG0
    #define RF_INT_PIN          PORTBbits.RB0
    #define RF_INT_TRIS         TRISBbits.TRISB0
    #define PHY_CS              LATCbits.LATC6
    #define PHY_CS_TRIS         TRISCbits.TRISC6
    #define PHY_RESETn          LATBbits.LATB2
    #define PHY_RESETn_TRIS     TRISBbits.TRISB2
    #define PHY_WAKE            LATBbits.LATB1
    #define PHY_WAKE_TRIS       TRISBbits.TRISB1
    #define RF_USE_SPI          2
    #define RF_SPI_SI_TRIS      TRISCbits.TRISC1
    #define RF_SPI_SO_TRIS      TRISCbits.TRISC2
    #define RF_SPI_SCK_TRIS     TRISCbits.TRISC0

    #define RF_INIT_PINS()    {\
                                RF_SPI_SI_TRIS = 1;\
                                RF_SPI_SO_TRIS = 0;\
                                RF_SPI_SCK_TRIS = 0;\
                                SSP2STAT = 0xC0; /* 0b11000000 -- SMP=1 (sample at end) CKE=1 (transmit on active to idle) */ \
                                SSP2CON1 = 0x20; /* 0b00100000 -- SSPEN=1 (serial port enable) CKP=0 (clock idle low) SSPM=0 (Fosc/4 = 12 MHz or 2 MHz) */ \
                                PHY_CS = 1;\
                                PHY_CS_TRIS = 0;\
                                PHY_RESETn = 0;\
                                PHY_RESETn_TRIS = 0;\
                                PHY_WAKE = 1;\
                                PHY_WAKE_TRIS = 0;\
                                RFIF = 0;\
                                RF_INT_TRIS = 1;\
								RF_IEDG = 0;\
                                RFIE = 1;\
                            }



    // Timer 3
    #define TIMER_IF PIR2bits.TMR3IF
    #define TIMER_IE PIE2bits.TMR3IE
    #define TIMER_IP IPR2bits.TMR3IP
    
    // Enable timer-3 as low priority interrupt; at 8 MHz (/4/8) this is 4us per tick (250 kHz)
    #define TIMER_START() {\
    	TIMER_IE = 0; \
    	T3CON = 0x33;           /* [00110011] 00 = Fosc/4; 11 = 1:8 prescale; 0 = crystal off; 0 = no-sync; 1 = 16-bit r/w enable; 1 = TMR3ON */ \
    	TIMER_IP = 0; \
    	TIMER_IF = 0; \
    }
    
    #define TIMER_STOP() { \
    	T3CONbits.TMR3ON = 0; \
    	PIE1bits.TMR3IE = 0; \
    }
    
    // Preset the timer value
    #define TIMER_SET(_v) { \
    	TMR3H = (unsigned char)((_v) >> 8); \
    	TMR3L = (unsigned char)(_v); \
    }
    
    // Minimum timer interval (1/4 msec)
    #define TIMER_CUTOFF 62






    // ---- For Bootloader ----
    
	#define		CLOCK_USB_XTAL_NO_DELAY() 			{ OSCTUNE |= 0b01000000; OSCCON = 0b00000000; }
	//#define		SetClockIntOSC() 		{ OSCTUNE = 0b00000000; OSCCON = 0b01110011; }
	//#define		SetClockIntRC() 		{ OSCTUNE = 0b00000000; OSCCON = 0b00000011; }
			
	#define		SIMPLE_INIT_ALL()			{\
									ANCON0 = 0xFF;/*All digital*/\
									ANCON1 = 0x0F;/*Bandgap ref off*/\
									BATT_NCHARGING_TRIS = 1;\
									USB_BUS_SENSE_TRIS = 1;\
									LED_INIT_PINS();\
									ACCEL_INIT_PINS();\
									RF_INIT_PINS();\
									}

	#define SIMPLE_SLEEP()	{\
							LED_INIT_PINS();\
							LED = 0;		/*LED off */\
							ANCON0 = 0b11110001; /*Analogue pins and Bandgap off */\
							ANCON1 = 0b00011111;\
							USB_BUS_SENSE_TRIS = 1;\
							USB_BUS_SENSE_IEDG = 1; 	/*Rising edge on USB detect*/\
							USB_BUS_SENSE_IP = 0;\
							USB_BUS_SENSE_IF = 0;\
							USB_BUS_SENSE_IE = 1; 	/*Enable wake on USB */\
							Sleep();\
							Nop();\
							}						
							
	// Based on PDQ shutdown...
	#define SHUTDOWN()	{\
							INTCONbits.GIEH = 0;	/*Interrupts off*/\
							INTCONbits.GIEL = 0; \
							LED=0;		/*LEDs off */\
							TRISA 	= 0b01000001;/* Floating pins are driven low to reduce current draw*/\
							LATA 	= 0b00000010;\
							TRISB 	= 0b00101000;\
							LATB 	= 0b00000000;\
							TRISC 	= 0b00000000;\
							LATC 	= 0b00000000;\
							USB_BUS_SENSE_TRIS = 1;\
							LED_INIT_PINS();\
							T0CON = 0;				/*Internal modules off*/\
							T1CON = 0;\
							T2CON = 0;\
							T3CON = 0;\
							T4CON = 0;\
							CCP1CON = 0;\
							CCP2CON = 0;\
							ADCON0 = 0;\
							ANCON0 = 0b11110001; /*Analogue pins and Bandgap off */\
							ANCON1 = 0b00011111;\
							RCSTA1 = 0;\
							TXSTA1 = 0;\
							RCSTA2 = 0;\
							TXSTA2 = 0;\
							UCON = 0;\
							OSCTUNE = 0;			/*PLL off*/\
							/*Now we are ready to enable the USB and ADXL wakeup interrupts*/\
							INTCON = INTCON2 = INTCON3 = 0; /*Clear any existing sources*/\
							PIE1 = PIE2 = PIE3 = 0;	 	/*Clear any pre-existing peripheral interrupts*/\
							PIE2bits.USBIE = 0; \
							INTCON2bits.RBPU = 1;		/*PortB pullups off */\
							INTCON2bits.INTEDG2 = 1; 	/*Rising edge on USB detect*/\
							INTCON3bits.INT2IP = 0;\
							INTCON3bits.INT1IE = 0; 	/*Disable wake on ADXL*/\
							INTCON3bits.INT2IE = 1; 	/*Enable wake on USB */\
							INTCONbits.INT0IE = 0; 		/*Disable wake on RF*/\
							OSCCONbits.IDLEN = 0; 		/*Sleep, not idle*/\
							WDTCONbits.REGSLP = 1; 		/*Turn off internal Vreg in sleep*/\
							Sleep();					/*Sleep operation*/\
							Nop();						/*Skid */\
							}

#endif

