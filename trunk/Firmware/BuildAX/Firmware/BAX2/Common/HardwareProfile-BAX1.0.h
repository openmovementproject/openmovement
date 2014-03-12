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

// Hardware Profile: BAX sensor node v1.0
// Karim Ladha, 2013-2014

#ifndef _PROFILE_BAX1_0_H_
#define _PROFILE_BAX1_0_H_

extern volatile unsigned char dummy;

// IO pins
#define InitIO()	{\
					TRISA = 0b01111110;\
					LATA =  0b00000000;\
					TRISB = 0b00001101;\
					LATB =  0b00000010;\
					TRISC = 0b00010000;\
					LATC =  0b00111000;\
					ANALOG_INIT();\
					}

// Oscillator block
#define CLOCK_PLL64			{OSCCON = 0b01110011;OSCTUNE = 0b01000000;}
#define CLOCK_INTOSC16()	{OSCCON = 0b01110011;}
#define CLOCK_INTOSC4()		{OSCCON = 0b01010011;}
#define CLOCK_INTOSC1()		{OSCCON = 0b00110011;}
#define CLOCK_INTRC()		{OSCCON = 0b00000011;}

// Peripherals
#define BUTTON				(!PORTAbits.RA6)

// External switch - hall effect like Panasonic-AN48836, uses 3uA, same as reed sw but powered
#define SWITCH_PWR_TRIS		TRISCbits.TRISC0
#define SWITCH_PWR			LATCbits.LATC0
#define SWITCH_TRIS			TRISCbits.TRISC1
#define SWITCH				PORTCbits.RC1
#define SWITCH_ON()			{SWITCH_TRIS=1;SWITCH_PWR = 1;SWITCH_PWR_TRIS = 0;}
#define SWITCH_OFF()		{SWITCH_TRIS=1;SWITCH_PWR = 0;SWITCH_PWR_TRIS = 0;}

// LED colours
#define LED_OFF				0b00
#define LED_RED				0b01
#define LED_GREEN			0b10
#define LED_SET(_c)			{LATB&=0b11001111;LATB|=(_c<<4);}

// Expansion pins
#define EXP0				LATCbits.LATC0
#define EXP1				LATCbits.LATC1

// Analogue control
#define TEMP_CHANNEL		9
#define LIGHT_CHANNEL		4
#define HUMID_CHANNEL		8
#define PIR_CHANNEL 		2
#define FVR_CHANNEL			15
#define ANALOG_INIT()		{ANSEL = 0b00011100;ANSELH=0b00000011;} // Set analogue pins

#define SENSORS_ENABLE			LATAbits.LATA0
#define HUMIDITY_PULSE			LATAbits.LATA7
#define HUMIDITY_GAIN_SEL_1()	{TRISAbits.TRISA1 = 1; LATAbits.LATA1 = 0;}
#define HUMIDITY_GAIN_SEL_10()	{TRISAbits.TRISA1 = 0; LATAbits.LATA1 = 0;}
#define HUMIDITY_GAIN_SEL_50()	{TRISAbits.TRISA1 = 0; LATAbits.LATA1 = 1;}

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
							else if ((OSCCON & 0x70) == 0b01110000) baud = 34;/*16MHz default*/\
							else if ((OSCCON & 0x70) == 0b01010000) baud = 8;/*4MHz*/\
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

#endif 
