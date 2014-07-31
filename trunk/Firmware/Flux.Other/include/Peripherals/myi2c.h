/* 
 * Copyright (c) 2011-2012, Newcastle University, UK.
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

// K Ladha 2011
// My own fast software I2C code using SW I2C routines which are 5x faster than Mchips (and don't hang)
// Max clock is 8MHz if all wait delays are zero
#ifndef MYI2C_H
#define MYI2C_H

#include <Compiler.h>
#include "HardwareProfile.h"
#include <TimeDelay.h>

// Defines
#define I2C_READ_MASK		0x1

// Override to non-default i2c channel or set master switch
#ifdef MY_I2C_OVERIDE // Define this locally in the .c file
	#define LOCAL_MASTER_SWITCH MY_I2C_OVERIDE
#else
	#if   defined (USE_MY_SW_I2C)	
		#define LOCAL_MASTER_SWITCH	-1
	#elif defined (USE_HW_I2C)
		#define LOCAL_MASTER_SWITCH	1 // Assuming I2C 1 is used, legacy
	#elif defined (USE_HW_I2C1)
		#define LOCAL_MASTER_SWITCH	1
	#elif defined (USE_HW_I2C2)
		#define LOCAL_MASTER_SWITCH	2
	#elif defined (USE_HW_I2C3)
		#define LOCAL_MASTER_SWITCH	3
	#else
		#define LOCAL_MASTER_SWITCH 0 // Causes no I2C functions to be gernerated by header
	#endif
#endif

// Are we using a hardware module?
#if (LOCAL_MASTER_SWITCH > 0)

	#ifndef InitI2C		/* Sometimes defined in HardwareProfile.h */
		#define InitI2C()		{mySCLd=1;mySDAd=1;}
	#endif
	
	#ifdef __C30__
	/*
	Notes on baud rate for PIC24:
	I2C_BRG = ((Fcyc/Fscl)-(Fcyc/10Mhz))-1
	@4MIPS
		100kHz	-	39	
		400kHz	-	9
		1MHz	-	3
		2MHz	-	1
		4MHz	-	0 (invalid)
	@16MIPS
		100kHz	-	157
		400kHz	-	37
		1MHz	-	13
		2MHz	-	5
		4MHz	-	1 (invalid)	
	*/
	
	// Use one of these defines in the .c file (HW only)
	// #define LOCAL_I2C_RATE		XXXXXXXXX
	#ifndef __dsPIC33E__	/*PIC24F only HW with no USB xtal*/
		#define I2C_RATE_100kHZ 		((OSCCONbits.COSC==1)? 157 : 39)	
		#define I2C_RATE_200kHZ 		((OSCCONbits.COSC==1)? 72 : 18)
		#define I2C_RATE_400kHZ 		((OSCCONbits.COSC==1)? 37 : 9)	
		#define I2C_RATE_1000kHZ 		((OSCCONbits.COSC==1)? 13 : 3)			
		#define I2C_RATE_2000kHZ		((OSCCONbits.COSC==1)? 7 : 1)		
	#endif
	
		// Needed, or else i2c.h fails
		#define USE_AND_OR
		#include <i2c.h>

		#ifdef __dsPIC33E__
			// Need to undo the and/or masking too
			#define I2C_ON			(~(I2C1_OFF))
			#define I2C_7BIT_ADD	(~(I2C1_10BIT_ADD))
			#define I2C_SLW_DIS		(~(I2C1_SLW_EN))
		#endif
	
		#ifndef MY_I2C_TIMEOUT
		#define MY_I2C_TIMEOUT 	65535
		#warning "Using default timeout, long!"
		#endif

		#if defined(__DEBUG) || !defined(IGNORE_I2C_TIMOUTS)			
			extern void myI2CCheckTimeout(unsigned short timeout); /* My internal prototype*/
			#ifndef CHECK_TIMEOUT				/* You can add your own handler if you need - this type of exeption indicates some HW failure*/
				#warning "USING DEFAULT CHECK FOR BUS TIMEOUTS (P24 only). This will slow the i2c functions a bit."
				#define ENABLE_I2C_TIMOUT_CHECK /* Causes prototyped function to be created in myi2c.c*/
				#define CHECK_TIMEOUT(_t)	myI2CCheckTimeout(_t) /* Assign internal handler to check timouts*/
			#endif
		#elif defined (CHECK_TIMEOUT)
			/* User handler should be declared in hardware profile.h*/
		#else
			/* Ingnore timeouts*/
			#define CHECK_TIMEOUT(_t)	{;}
		#endif
	
		#if	(LOCAL_MASTER_SWITCH == 1)
			// Following defines from the peripheral header
			#define myI2COpen()			{OpenI2C1((I2C_ON  | I2C_7BIT_ADD | I2C_SLW_DIS),(LOCAL_I2C_RATE));}
			#define myI2CIdle	 		IdleI2C1	
			#define myI2CStart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C1CONbits.SEN=1;while(I2C1CONbits.SEN && --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CStop()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C1CONbits.PEN=1;while(I2C1CONbits.PEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CRestart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C1CONbits.RSEN=1;while(I2C1CONbits.RSEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CAck()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C1CONbits.ACKDT = 0;I2C1CONbits.ACKEN=1;while(I2C1CONbits.ACKEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CNack()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C1CONbits.ACKDT = 1;I2C1CONbits.ACKEN=1;while(I2C1CONbits.ACKEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CClose			CloseI2C1
			#define myI2Cputc(_x)		{unsigned short timeout = MY_I2C_TIMEOUT;I2C1TRN=_x;while(I2C1STATbits.TBF&& --timeout);while(I2C1STATbits.TRSTAT&& --timeout);CHECK_TIMEOUT(timeout);} 
			#define myI2Cgetc()			MasterReadI2C1()
			#define mygetsI2C(_data,_length) MastergetsI2C((unsigned int) _length,(unsigned char *)_data, (unsigned int)0xffff/*Timeout*/);
			#define myI2Cputs(_data,_length) MasterputsI2C((unsigned char *)(_data),strlen(_data))
			#define myAckStat()			(!I2C1STATbits.ACKSTAT) /*TRUE or 1 if slave acked*/
		#elif (LOCAL_MASTER_SWITCH == 2)
			// Following defines from the peripheral header
			#define myI2COpen()			{OpenI2C2((I2C_ON  | I2C_7BIT_ADD | I2C_SLW_DIS),(LOCAL_I2C_RATE));}
			#define myI2CIdle	 		IdleI2C2	
			#define myI2CStart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C2CONbits.SEN=1;while(I2C2CONbits.SEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CStop()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C2CONbits.PEN=1;while(I2C2CONbits.PEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CRestart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C2CONbits.RSEN=1;while(I2C2CONbits.RSEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CAck()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C2CONbits.ACKDT = 0;I2C2CONbits.ACKEN=1;while(I2C2CONbits.ACKEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CNack()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C2CONbits.ACKDT = 1;I2C2CONbits.ACKEN=1;while(I2C2CONbits.ACKEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CClose			CloseI2C2
			#define myI2Cputc(_x)		{unsigned short timeout = MY_I2C_TIMEOUT;I2C2TRN=_x;while(I2C2STATbits.TBF&& --timeout);while(I2C2STATbits.TRSTAT&& --timeout);CHECK_TIMEOUT(timeout);} 
			#define myI2Cgetc()			MasterReadI2C2()
			#define mygetsI2C(_data,_length) MastergetsI2C2((unsigned int) _length,(unsigned char *)_data, (unsigned int)0xffff/*Timeout*/);
			#define myI2Cputs(_data,_length) MasterputsI2C2((unsigned char *)(_data),strlen(_data))
			#define myAckStat()			(!I2C2STATbits.ACKSTAT) /*TRUE or 1 if slave acked*/
		#elif (LOCAL_MASTER_SWITCH == 3)
			// Following defines from the peripheral header
			#define myI2COpen()			{OpenI2C3((I2C_ON  | I2C_7BIT_ADD | I2C_SLW_DIS),(LOCAL_I2C_RATE));}
			#define myI2CIdle	 		IdleI2C3	
			#define myI2CStart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C3CONbits.SEN=1;while(I2C3CONbits.SEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CStop()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C3CONbits.PEN=1;while(I2C3CONbits.PEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CRestart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C3CONbits.RSEN=1;while(I2C3CONbits.RSEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CAck()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C3CONbits.ACKDT = 0;I2C3CONbits.ACKEN=1;while(I2C3CONbits.ACKEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CNack()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C3CONbits.ACKDT = 1;I2C3CONbits.ACKEN=1;while(I2C3CONbits.ACKEN&& --timeout);CHECK_TIMEOUT(timeout);}
			#define myI2CClose			CloseI2C3
			#define myI2Cputc(_x)		{unsigned short timeout = MY_I2C_TIMEOUT;I2C3TRN=_x;while(I2C3STATbits.TBF&& --timeout);while(I2C3STATbits.TRSTAT&& --timeout);CHECK_TIMEOUT(timeout);} 
			#define myI2Cgetc()			MasterReadI2C3()
			#define mygetsI2C(_data,_length) MastergetsI2C((unsigned int) _length,(unsigned char *)_data, (unsigned int)0xffff/*Timeout*/);
			#define myI2Cputs(_data,_length) MasterputsI2C((unsigned char *)(_data),strlen(_data))
			#define myAckStat()			(!I2C3STATbits.ACKSTAT) /*TRUE or 1 if slave acked*/
		#endif
	
		#define WaitStartmyI2C()	{}
		#define WaitStopmyI2C() 	{}
		#define WaitRestartmyI2C()	{}

	#elif defined __C32__

	#define I2C_RATE_100kHZ 		((OSCCONbits.COSC&1)? 398 : 38)	
	#define I2C_RATE_200kHZ 		((OSCCONbits.COSC&1)? 198 : 18)
	#define I2C_RATE_400kHZ 		((OSCCONbits.COSC&1)? 98 : 8)	
	#define I2C_RATE_1000kHZ 		((OSCCONbits.COSC&1)? 38 : 2) /*BRG values of 0 or 1 are "expressly forbidden"*/
	#define I2C_RATE_2000kHZ		((OSCCONbits.COSC&1)? 20 : 2) /*I2C rates > 1.5 MHz cause problems too for some reason :( */		
	
		// Needed, or else i2c.h fails
		#define USE_AND_OR
		#include <i2c.h>
	
		#ifndef MY_I2C_TIMEOUT
		#define MY_I2C_TIMEOUT 	65535
		#warning "Using default timeout, long!"
		#endif
		
		#if (LOCAL_MASTER_SWITCH == 1)
			// Following defines from the peripheral header
			#define myI2COpen()			{I2C1CON = 0;I2C1BRG = (LOCAL_I2C_RATE);I2C1CONSET = 0x00008200; myI2CIdle();} /*Standard mode, no slew rate control*/
			#define myI2CIdle()	 		{unsigned short timeout = MY_I2C_TIMEOUT;while((I2C1CONbits.SEN || I2C1CONbits.PEN || I2C1CONbits.RSEN || I2C1CONbits.RCEN || I2C1CONbits.ACKEN || I2C1STATbits.TRSTAT)&& --timeout);}	
			#define myI2CStart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C1CONSET = _I2C1CON_SEN_MASK;while(I2C1CONbits.SEN && --timeout);}
			#define myI2CStop()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C1CONSET = _I2C1CON_PEN_MASK;while(I2C1CONbits.PEN&& --timeout);}
			#define myI2CRestart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C1CONSET = _I2C1CON_RSEN_MASK;while(I2C1CONbits.RSEN&& --timeout);}
			#define myI2CAck()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C1CONCLR =_I2C1CON_ACKDT_MASK;I2C1CONSET=_I2C1CON_ACKEN_MASK;while(I2C1CONbits.ACKEN&& --timeout);}
			#define myI2CNack()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C1CONSET =_I2C1CON_ACKDT_MASK;I2C1CONSET=_I2C1CON_ACKEN_MASK;while(I2C1CONbits.ACKEN&& --timeout);}
			#define myI2CClose()		{I2C1CONCLR = _I2C1CON_ON_MASK;}
			#define myI2Cputc(_x)		{unsigned short timeout = MY_I2C_TIMEOUT;while(I2C1STATbits.TBF&& --timeout);I2C1TRN=_x;while(I2C1STATbits.TRSTAT&& --timeout);} 
			#define myI2Cgetc()			MasterGetcI2C1()
			//#define mygetsI2C(_data,_length) MastergetsI2C((unsigned int) _length,(unsigned char *)_data, (unsigned int)0xffff/*Timeout*/);
			//#define myI2Cputs(_data,_length) MasterputsI2C((unsigned char *)(_data),strlen(_data))
			#define myAckStat()			(!I2C1STATbits.ACKSTAT) /*TRUE or 1 if slave acked*/
		#elif (LOCAL_MASTER_SWITCH == 2)
			// Following defines from the peripheral header
			#define myI2COpen()			{OpenI2C2((I2C_ON  | I2C_7BIT_ADD | I2C_SLW_DIS),(LOCAL_I2C_RATE));}
			#define myI2CIdle()	 		{unsigned short timeout = MY_I2C_TIMEOUT;while((I2C2CONbits.SEN || I2C2CONbits.PEN || I2C2CONbits.RSEN || I2C2CONbits.RCEN || I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT)&& --timeout);	
			#define myI2CStart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C2CONSET = _I2C2CON_SEN_MASK;while(I2C2CONbits.SEN && --timeout);}
			#define myI2CStop()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C2CONSET = _I2C2CON_PEN_MASK;while(I2C2CONbits.PEN&& --timeout);}
			#define myI2CRestart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C2CONSET = _I2C2CON_RSEN_MASK;while(I2C2CONbits.RSEN&& --timeout);}
			#define myI2CAck()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C2CONCLR =_I2C2CON_ACKDT_MASK;I2C2CONSET=_I2C2CON_ACKEN_MASK;while(I2C2CONbits.ACKEN&& --timeout);}
			#define myI2CNack()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C2CONSET =_I2C2CON_ACKDT_MASK;I2C2CONSET=_I2C2CON_ACKEN_MASK;while(I2C2CONbits.ACKEN&& --timeout);}
			#define myI2CClose()		{I2C2CONCLR = _I2C2CON_ON_MASK;}
			#define myI2Cputc(_x)		{unsigned short timeout = MY_I2C_TIMEOUT;I2C2TRN=_x;/*while(I2C2STATbits.TBF&& --timeout);*/while(I2C2STATbits.TRSTAT&& --timeout);} 
			#define myI2Cgetc()			MasterGetcI2C2()
			#define mygetsI2C(_data,_length) MastergetsI2C((unsigned int) _length,(unsigned char *)_data, (unsigned int)0xffff/*Timeout*/);
			#define myI2Cputs(_data,_length) MasterputsI2C((unsigned char *)(_data),strlen(_data))
			#define myAckStat()			(!I2C2STATbits.ACKSTAT) /*TRUE or 1 if slave acked*/
		#elif (LOCAL_MASTER_SWITCH == 3)
			// Following defines from the peripheral header
			#define myI2COpen()			{OpenI2C3((I2C_ON  | I2C_7BIT_ADD | I2C_SLW_DIS),(LOCAL_I2C_RATE));}
			#define myI2CIdle()	 		{unsigned short timeout = MY_I2C_TIMEOUT;while((I2C3CONbits.SEN || I2C3CONbits.PEN || I2C3CONbits.RSEN || I2C3CONbits.RCEN || I2C3CONbits.ACKEN || I2C3STATbits.TRSTAT)&& --timeout);	
			#define myI2CStart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C3CONSET = _I2C3CON_SEN_MASK;while(I2C3CONbits.SEN && --timeout);}
			#define myI2CStop()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C3CONSET = _I2C3CON_PEN_MASK;while(I2C3CONbits.PEN&& --timeout);}
			#define myI2CRestart()		{unsigned short timeout = MY_I2C_TIMEOUT;I2C3CONSET = _I2C3CON_RSEN_MASK;while(I2C3CONbits.RSEN&& --timeout);}
			#define myI2CAck()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C3CONCLR =_I2C3CON_ACKDT_MASK;I2C3CONSET=_I2C3CON_ACKEN_MASK;while(I2C3CONbits.ACKEN&& --timeout);}
			#define myI2CNack()			{unsigned short timeout = MY_I2C_TIMEOUT;I2C3CONSET =_I2C3CON_ACKDT_MASK;I2C3CONSET=_I2C3CON_ACKEN_MASK;while(I2C3CONbits.ACKEN&& --timeout);}
			#define myI2CClose()		{I2C3CONCLR = _I2C3CON_ON_MASK;}
			#define myI2Cputc(_x)		{unsigned short timeout = MY_I2C_TIMEOUT;I2C3TRN=_x;/*while(I2C3STATbits.TBF&& --timeout);*/while(I2C3STATbits.TRSTAT&& --timeout);} 
			#define myI2Cgetc()			MasterGetcI2C3()
			#define mygetsI2C(_data,_length) MastergetsI2C((unsigned int) _length,(unsigned char *)_data, (unsigned int)0xffff/*Timeout*/);
			#define myI2Cputs(_data,_length) MasterputsI2C((unsigned char *)(_data),strlen(_data))
			#define myAckStat()			(!I2C3STATbits.ACKSTAT) /*TRUE or 1 if slave acked*/
		#endif

		// PIC32 only, Following are inline macros to enable the above to work - replaces old legacy M'chip ones
		#ifdef _I2C1CON_RCEN_MASK
		extern __inline__ unsigned char __attribute__((always_inline))	MasterGetcI2C1(void)
		{unsigned short timeout = MY_I2C_TIMEOUT;I2C1CONSET = _I2C1CON_RCEN_MASK;while(!I2C1STATbits.RBF&& --timeout);return I2C1RCV;}
		#endif
		#ifdef _I2C2CON_RCEN_MASK
		extern __inline__ unsigned char __attribute__((always_inline))	MasterGetcI2C2(void)
		{unsigned short timeout = MY_I2C_TIMEOUT;I2C2CONSET = _I2C2CON_RCEN_MASK;while(!I2C2STATbits.RBF&& --timeout);return I2C2RCV;}
		#endif
		#ifdef _I2C3CON_RCEN_MASK
		extern __inline__ unsigned char __attribute__((always_inline))	MasterGetcI2C3(void)
		{unsigned short timeout = MY_I2C_TIMEOUT;I2C3CONSET = _I2C3CON_RCEN_MASK;while(!I2C3STATbits.RBF&& --timeout);return I2C3RCV;}
		#endif

		// Unused but may be called in legacy drivers
		#define WaitStartmyI2C()	{}
		#define WaitStopmyI2C() 	{}
		#define WaitRestartmyI2C()	{}
		
	#endif // PIC32 part

// Not using hardware (LOCAL_MASTER_SWITCH <= 0) - software then?
#elif (LOCAL_MASTER_SWITCH == -1)
	
	// Control of timing - add delays if your clock is faster than 8Mhz
	/*The rise time taken for the pull ups to raise the SDA bus capacitance to Vdd*/
	#define WaitRise() Delay10us(1)
	/*The time taken for the i/o pin to pull the bus capacitance to Vss, should be v.short */	
	#define WaitFall()	Delay10us(1)
	/*The time taken for the i/o pin to drive the bus capacitance to Vdd should be v.short 
		(clock is push pull so not same as WaitRise()) */
	#define WaitClock()	Delay10us(1)
	
	extern void i2cDelay(void);
	
	// Global I2C var - speeds stuff up by leaving stack alone
	extern unsigned char my_I2C_working_var;
	
	
	// Prototypes
	#define myAckStat()		(my_I2C_working_var&0x1) /*TRUE or 1 if slave acked*/
	#define myI2Cputc(_x)	{my_I2C_working_var = _x;_myI2Cputc();}

	extern unsigned char _myI2Cputc(void);
	extern unsigned char myI2Cgetc(void);
	
	// Reads all 127 addresses and returns a string of 1s and 0s, 1 indicates an acknoledge
	extern void I2CBusDebug(char*);
	// Issue a general call reset from the I2C spec
	extern void I2C_GC_Reset(void);

	// Timing is critical and these will only work in a certain order, the commented bits are implicit by timing used
	#ifndef InitI2C		/* Sometimes defined in HardwareProfile.h */
		#define InitI2C()		{mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}
	#endif
	#define myI2CIdle() 	/*{mySCL = 1;mySDAd=1;}This does idle the bus, but if used like the HW i2c then it messes up the bus*/
	#define myI2CStart()	{/*mySCL = 1;WaitRise();*/mySDAd = 0; /*mySDAw = 0;*/WaitFall();}
	#define myI2CStop()		{mySCL = 0;WaitFall();mySDAd = 0;WaitFall();mySCL = 1;WaitRise();mySDAd = 1;WaitRise();}
	#define myI2CRestart()	{/*myI2CStop();*/mySCL = 0;WaitClock();mySDAd = 1;WaitRise();mySCL = 1;WaitClock();myI2CStart();}
	#define myI2CAck()		{mySDAd=0;WaitFall();mySCL = 1;WaitClock();}
	#define myI2CNack()		{mySCL = 1;WaitClock();}
	// Not used for SW I2C
	#define WaitStartmyI2C()
	#define WaitRestartmyI2C()
	#define WaitStopmyI2C()  
	#define myI2CClose()	
	#define myI2COpen()	

	#define mygetsI2C(_data,_length)	{\
										unsigned char i = _length;\
											for (;i>1;i--,_data++)\
											{\
												*_data = myI2Cgetc();\
												myI2CAck();\
											}/*we dont ack the last byte*/\
										*_data = myI2Cgetc();\
										}
	
	#define myI2Cputs(_data,_length)	{\
										unsigned char i = _length;\
											for (;i>0;i--,_data++)\
											{\
												myI2Cputc(*_data);\
											}/*we dont ack the last byte*/\
										}
	
	
	// Some effort was made to allow the functions to be used inline, such as for interrupts
	// Second copy - for ISRs/nesting
	extern unsigned char my_ISR_I2C_working_var;
	#define ISRmyAckStat()		(my_ISR_I2C_working_var&0x1) /*TRUE if slave acked*/
	//extern unsigned char _ISRmyI2Cputc(void);
	
	// Note getc does not return, call and read 'my_ISR_I2C_working_var' for result
	//extern unsigned char ISRmyI2Cgetc(void);
	// Other defines stay the same
	
	#define ISRmyI2Cputc(_x)/*sends byte in 'my_ISR_I2C_working_var'*/\
	{\
		my_ISR_I2C_working_var = _x;\
		mySCL = 0;WaitFall();\
		mySDAd = 1;WaitRise();\
		if (!(my_ISR_I2C_working_var&0x80)){mySDAd = 0;WaitFall();}\
		mySCL = 1;WaitClock();\
		mySCL = 0;WaitFall();\
		mySDAd = 1;WaitRise();\
		if (!(my_ISR_I2C_working_var&0x40)){mySDAd = 0;WaitFall();}\
		mySCL = 1;WaitClock();\
		mySCL = 0;WaitFall();\
		mySDAd = 1;WaitRise();\
		if (!(my_ISR_I2C_working_var&0x20)){mySDAd = 0;WaitFall();}\
		mySCL = 1;WaitClock();\
		mySCL = 0;WaitFall();\
		mySDAd = 1;WaitRise();\
		if (!(my_ISR_I2C_working_var&0x10)){mySDAd = 0;WaitFall();}\
		mySCL = 1;WaitClock();\
		mySCL = 0;WaitFall();\
		mySDAd = 1;WaitRise();\
		if (!(my_ISR_I2C_working_var&0x8)){mySDAd = 0;WaitFall();}\
		mySCL = 1;WaitClock();\
		mySCL = 0;WaitFall();\
		mySDAd = 1;WaitRise();\
		if (!(my_ISR_I2C_working_var&0x4)){mySDAd = 0;WaitFall();}\
		mySCL = 1;WaitClock();\
		mySCL = 0;WaitFall();\
		mySDAd = 1;WaitRise();\
		if (!(my_ISR_I2C_working_var&0x2)){mySDAd = 0;WaitFall();}\
		mySCL = 1;WaitClock();\
		mySCL = 0;WaitFall();\
		mySDAd = 1;WaitRise();\
		if (!(my_ISR_I2C_working_var&0x1)){mySDAd = 0;WaitFall();}\
		mySCL = 1;WaitClock();\
		mySCL = 0;WaitFall();\
		\
		my_ISR_I2C_working_var = 0;\
		mySDAd = 1;WaitRise();\
		\
		if (!mySDAr)\
			{my_ISR_I2C_working_var |= 1;}\
		\
		mySCL = 1;WaitClock();\
		/*if my_ISR_I2C_working_var == 1 then device acked*/\
	}
	
	#define ISRmyI2Cgetc()/*result in 'my_ISR_I2C_working_var'*/\
	{\
		my_ISR_I2C_working_var = 0;\
		mySCL = 0;WaitFall();\
		mySDAd = 1;WaitRise();\
		mySCL = 1;WaitClock();\
		if (mySDAr)	my_ISR_I2C_working_var|=0x80;\
		mySCL = 0;WaitFall();\
		mySCL = 1;WaitClock();\
		if (mySDAr)	my_ISR_I2C_working_var|=0x40;\
		mySCL = 0;WaitFall();\
		mySCL = 1;WaitClock();\
		if (mySDAr)	my_ISR_I2C_working_var|=0x20;\
		mySCL = 0;WaitFall();\
		mySCL = 1;WaitClock();\
		if (mySDAr)	my_ISR_I2C_working_var|=0x10;\
		mySCL = 0;WaitFall();\
		mySCL = 1;WaitClock();\
		if (mySDAr)	my_ISR_I2C_working_var|=0x8;\
		mySCL = 0;WaitFall();\
		mySCL = 1;WaitClock();\
		if (mySDAr)	my_ISR_I2C_working_var|=0x4;\
		mySCL = 0;WaitFall();\
		mySCL = 1;WaitClock();\
		if (mySDAr)	my_ISR_I2C_working_var|=0x2;\
		mySCL = 0;WaitFall();\
		mySCL = 1;WaitClock();\
		if (mySDAr)	my_ISR_I2C_working_var|=0x1;\
		mySCL = 0;WaitFall();\
	}
	
#else // LOCAL_MASTER_SWITCH == 0
	//  Not an error - some drivers have dual interface modes and include this file despite not using it
#endif

#undef MY_I2C_OVERIDE 
#undef LOCAL_MASTER_SWITCH 

#else

#endif

