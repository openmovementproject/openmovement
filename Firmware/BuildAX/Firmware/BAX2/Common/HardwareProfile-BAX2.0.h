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

// Hardware Profile: BAX sensor node v2.0
// Karim Ladha, 2013-2014

#ifndef _PROFILE_BAX1_0_H_
#define _PROFILE_BAX1_0_H_

extern volatile unsigned char dummy;

// IO pins
#define InitIO()	{\
					TRISA = 0b00101111;\
					LATA =  0b00101111;\
					TRISB = 0b00000101;\
					LATB =  0b00000111;\
					TRISC = 0b11010100;\
					LATC =  0b00010100;\
					ANALOG_INIT();\
					}

// Oscillator block
#define CLOCK_PLL64			{OSCCON = 0b01110011;OSCTUNE = 0b01000000;}
#define CLOCK_INTOSC16()	{OSCCON = 0b01110011;}
#define CLOCK_INTOSC4()		{OSCCON = 0b01010011;}
#define CLOCK_INTOSC1()		{OSCCON = 0b00110011;}
#define CLOCK_INTRC()		{OSCCON = 0b00000011;}

// Peripherals
#define BUTTON				(!PORTCbits.RC2)

// External switch - hall effect like Panasonic-AN48836, uses 3uA, same as reed sw but powered
#define SWITCH_PWR_TRIS		TRISBbits.TRISB3
#define SWITCH_PWR			LATBbits.LATB3
#define SWITCH_TRIS			TRISBbits.TRISB2
#define SWITCH				PORTBbits.RB2
#define SWITCH_ON()			{SWITCH_TRIS=1;SWITCH_PWR = 1;SWITCH_PWR_TRIS = 0;}
#define SWITCH_OFF()		{SWITCH_TRIS=1;SWITCH_PWR = 0;SWITCH_PWR_TRIS = 0;}

// LED colours
#define LED_OFF				0b00
#define LED_RED				0b01
#define LED_GREEN			0b10
#define LED_SET(_c)			{LATC&=0b11111100;LATC|=(_c);}

// Expansion pins
#define EXP0				LATBbits.LATB4
#define EXP1				LATBbits.LATB5

// Analogue control
#define TEMP_CHANNEL		0
#define LIGHT_CHANNEL		4
#define HUMID_CHANNEL		1
#define PIR_CHANNEL 		2
#define FVR_CHANNEL			15
#define ANALOG_INIT()		{ANSEL = 0b11111111;ANSELH=0b00000000;} // Set analogue pins

#define SENSORS_ENABLE			LATAbits.LATA4
#define HUMIDITY_PULSE			LATAbits.LATA7
#define HUMIDITY_GAIN_SEL_1()	{TRISAbits.TRISA6 = 1; LATAbits.LATA6 = 0;}
#define HUMIDITY_GAIN_SEL_10()	{TRISAbits.TRISA6 = 0; LATAbits.LATA6 = 0;}
#define HUMIDITY_GAIN_SEL_50()	{TRISAbits.TRISA6 = 0; LATAbits.LATA6 = 1;}

// Radio SPI Control abstraction
#define Si44_CS				LATBbits.LATB1
#define Si44_nIRQ			PORTBbits.RB0
#define Si44_IRQ_FLAG		INTCONbits.INT0IF
#define Si44_IRQ_EN()		{INTCONbits.INT0IE = 1;}
#define Si44_IRQ_OFF()		{INTCONbits.INT0IE = 0;}
#define Si44_SPI_INIT()		{INTCON2bits.INTEDG0 = 0;/*Falling edge int*/\
							SSPSTAT = 0b01000000;\
							SSPCON1 = 0b00000000; \
							SSPCON1 |= 0b00100000;} /*SMP = 0, CKE = 1, CKP = 0, Fosc/4*/
#define Si44_SPI_OFF()		{SSPCON1 = 0;}
#define Si44_SPI_OPEN()		{Si44_CS = 0;}
#define Si44_SPI_PUTC(_c)	{\
								dummy = SSPBUF;\
								SSPBUF = _c;\
								while (!SSPSTATbits.BF);\
							}
#define Si44_SPI_GETC()		SSPBUF							
#define Si44_SPI_CLOSE()	{Si44_CS = 1;}


// UART Debug out - 115200 baud
#define COMM_INIT()		{\
							unsigned char baud = 0;\
							if(OSCTUNEbits.PLLEN) baud = 138;\
							else if (OSCCONbits.IRCF == 0b111) baud = 34;/*16MHz default*/\
							else if (OSCCONbits.IRCF == 0b101) baud = 8;/*4MHz*/\
							else baud = 26; 			/*Note: 9600 baud at 1 MHz*/\
							TRISCbits.TRISC6 = 1;		/*Tx input, as in datasheet*/\
							TRISCbits.TRISC7 = 1;		/*Rx input, as in datasheet*/\
							RCSTA = TXSTA = 0;			/*Set all off*/\
							SPBRGH = 0; 				/*Always zero at this baud*/\
							SPBRG = baud; 				/*Set baud*/\
							BAUDCON = 0b00001000; 		/*16 bit baud, Tx high*/\
							TXSTA = 0b00100100; 		/*Fast baud, Tx enabled*/\
							RCSTAbits.CREN = 1;			/*Turn on receiver*/\
							RCSTAbits.SPEN = 1;			/*Turn on transmit*/\
							baud = RCREG;				/*Dummy read to clear flags*/\
							Delay100us();\
							}
#define RX_LEVEL			PORTCbits.RC7
#define UART_ON				RCSTAbits.SPEN
#define COMM_CLEAR_ERRS()	{if(RCSTA&0x06){dummy=RCREG;RCSTAbits.CREN=0;Nop();RCSTAbits.CREN=1;dummy=RCREG;}}
#define COMM_CLEAR()		{dummy=RCREG;RCSTAbits.CREN=0;Nop();RCSTAbits.CREN=1;dummy=RCREG;}
#define COMM_PUTC(_c)		{TXREG = _c;while(!TXSTAbits.TRMT);}
#define COMM_HASCHAR()		(PIR1bits.RCIF)
#define COMM_GETC()			(RCREG)
#define COMM_OFF()			{TXSTA = 0;\
							TRISCbits.TRISC6 = 0;LATCbits.LATC6 = 0;\
							TRISCbits.TRISC7 = 0;LATCbits.LATC7 = 0;}

// Stop watch for testing (note: cycles is 1/8 actual)
/*
Usage:
volatile unsigned short CYCLES;
STOP_CLOCK_START();
Function();
STOP_CLOCK_STOP();

*/
extern volatile unsigned short CYCLES; // Make one of these
#define STOP_CLOCK_START() {LED_SET(LED_RED);\
							T1CON=0b11110100;/*Div8*/\
							TMR1H=0;TMR1L=0;\
							CYCLES=0;\
							PIR1bits.TMR1IF=0;\
							T1CONbits.TMR1ON=1;}
#define STOP_CLOCK_STOP()	{T1CONbits.TMR1ON = 0;\
							LED_SET(LED_OFF);\
							CYCLES=TMR1L;\
							CYCLES|=((short)TMR1H)<<8;\
							if(PIR1bits.TMR1IF)CYCLES=0xffff;\
							else if (CYCLES)CYCLES=CYCLES;}

#endif 
