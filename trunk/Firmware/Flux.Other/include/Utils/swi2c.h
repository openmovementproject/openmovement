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

// Fast software I2C code using software I2C routines (5x faster than Microchip's, and won't hang)
// Karim Ladha, 2011

// Max clock is 8MHz if all wait delays are zero
#include <Compiler.h>
#include "HardwareProfile.h"

// Control of timing - add delays if your clock is faster than 8Mhz
/*The rise time taken for the pull ups to raise the SDA bus capacitance to Vdd*/
#define WaitRise()	Nop();Nop();Nop();Nop()
/*The time taken for the i/o pin to pull the bus capacitance to Vss, should be v.short */	
#define WaitFall()	Nop();Nop();Nop();Nop()
/*The time taken for the i/o pin to drive the bus capacitance to Vdd should be v.short 
	(clock is push pull so not same as WaitRise()) */
#define WaitClock()	Nop();Nop();Nop();Nop()

// Global I2C var - speeds stuff up by leaving stack alone
extern near unsigned char my_I2C_working_var;


// Defines
#define I2C_READ_MASK		0x1

// Prototypes
#define myAckStat()		(my_I2C_working_var&0x1) /*TRUE if slave acked*/
#define myI2Cputc(_x)	{my_I2C_working_var = _x;_myI2Cputc();}
extern unsigned char _myI2Cputc(void);

extern unsigned char myI2Cgetc(void);

// Reads all 127 addresses and returns a string of 1s and 0s, 1 indicates an acknoledge
extern void I2CBusDebug(char*);

// Timing is critical and these will only work in a certain order, the commented bits are implicit by timing used
#define InitI2C()		{mySCLd = 0;mySCL = 1;mySDAd=1;mySDAw=0;}
#define myI2CIdle() 	/*{mySCL = 1;mySDAd=1;}This does idle the bus, but if used like the HW i2c then it messes up the bus*/
#define myI2CStart()	{/*mySCL = 1;WaitRise();*/mySDAd = 0; /*mySDAw = 0;*/}
#define myI2CStop()		{mySCL = 0;WaitFall();mySDAd = 0;WaitFall();mySCL = 1;WaitRise();mySDAd = 1;Nop();WaitRise();}
#define myI2CRestart()	{myI2CStop();WaitRise();myI2CStart();}
#define myI2CAck()		{mySDAd=0;WaitFall();mySCL = 1;WaitClock();}
#define myI2CNack()		{mySCL = 1;WaitClock();}
// Not used
#define WaitStartmyI2C()
#define WaitStopmyI2C()  

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
extern near unsigned char my_ISR_I2C_working_var;
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
