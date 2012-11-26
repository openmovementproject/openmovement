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
// Fast software I2C code
#include <Compiler.h>
#include "Peripherals\myI2C.h"
#include "HardwareProfile.h"

#if (!defined(mySCL) || !defined(mySDAr) || !defined(mySDAw))
	#error "definitions needed"
#endif

#if !( defined(USE_HW_I2C1) || defined(USE_HW_I2C2) || defined(USE_HW_I2C3) || defined(USE_HW_I2C))
	// I need a very fast variable for my I2C code that does not require banking
	/* all accesses to these will be unbanked */
	unsigned char my_I2C_working_var;
	unsigned char my_ISR_I2C_working_var;
	
	unsigned char _myI2Cputc(void)
	{
		mySCL = 0;WaitFall();
		mySDAd = 1;WaitRise();
		if (!(my_I2C_working_var&0x80)){mySDAd = 0;WaitFall();} // bit 7 first
		mySCL = 1;WaitClock();
		mySCL = 0;WaitFall();
		mySDAd = 1;WaitRise();
		if (!(my_I2C_working_var&0x40)){mySDAd = 0;WaitFall();}				
		mySCL = 1;WaitClock();
		mySCL = 0;WaitFall();
		mySDAd = 1;WaitRise();
		if (!(my_I2C_working_var&0x20)){mySDAd = 0;WaitFall();}			
		mySCL = 1;WaitClock();
		mySCL = 0;WaitFall();
		mySDAd = 1;WaitRise();
		if (!(my_I2C_working_var&0x10)){mySDAd = 0;WaitFall();}				
		mySCL = 1;WaitClock();
		mySCL = 0;WaitFall();
		mySDAd = 1;WaitRise();
		if (!(my_I2C_working_var&0x8)){mySDAd = 0;WaitFall();}				
		mySCL = 1;WaitClock();
		mySCL = 0;WaitFall();
		mySDAd = 1;WaitRise();
		if (!(my_I2C_working_var&0x4)){mySDAd = 0;WaitFall();}				
		mySCL = 1;WaitClock();
		mySCL = 0;WaitFall();
		mySDAd = 1;WaitRise();
		if (!(my_I2C_working_var&0x2)){mySDAd = 0;WaitFall();}				
		mySCL = 1;WaitClock();
		mySCL = 0;WaitFall();
		mySDAd = 1;WaitRise();
		if (!(my_I2C_working_var&0x1)){mySDAd = 0;WaitFall();} 					
		mySCL = 1;WaitClock();
		mySCL = 0;WaitFall();
	
		// Check for Ack
		my_I2C_working_var = 0;
		mySDAd = 1;WaitRise(); 	// Slave will hold low for ack	
	
		if (!mySDAr)			
			{my_I2C_working_var |= 1;} // return 1 indicates ack
	
		mySCL = 1;WaitClock();
		return my_I2C_working_var; 
	}
	
	unsigned char myI2Cgetc(void)
	{
		my_I2C_working_var = 0;
		mySCL = 0;WaitFall();
		mySDAd = 1;WaitRise();// Allow slave to control SDA
		mySCL = 1;WaitClock();
		if (mySDAr)	my_I2C_working_var|=0x80; 	// bit 7 first
		mySCL = 0;WaitFall();
		mySCL = 1;WaitClock();
		if (mySDAr)	my_I2C_working_var|=0x40;
		mySCL = 0;WaitFall();
		mySCL = 1;WaitClock();
		if (mySDAr)	my_I2C_working_var|=0x20;
		mySCL = 0;WaitFall();
		mySCL = 1;WaitClock();
		if (mySDAr)	my_I2C_working_var|=0x10;
		mySCL = 0;WaitFall();
		mySCL = 1;WaitClock();
		if (mySDAr)	my_I2C_working_var|=0x8;
		mySCL = 0;WaitFall();
		mySCL = 1;WaitClock();
		if (mySDAr)	my_I2C_working_var|=0x4;
		mySCL = 0;WaitFall();
		mySCL = 1;WaitClock();
		if (mySDAr)	my_I2C_working_var|=0x2;
		mySCL = 0;WaitFall();
		mySCL = 1;WaitClock();
		if (mySDAr)	my_I2C_working_var|=0x1;
		mySCL = 0;WaitFall();
		return my_I2C_working_var; 
	}
	
	// 100 nops
	void i2cDelay(void)
	{
		#if (defined(__C30__)||defined(__C32__))
			Delay10us(1);
		#else
			int i;
			for(i=5;i;i--){Nop();}
		#endif
	}

#else

	#define LOCAL_I2C_RATE I2C_RATE_100kHZ

	extern void I2CBusDebug(char* list)
	{
		unsigned char add;
		for (add=0;add<0x7f;add++,list++)
		{
			// This polls every address - 1 indicates a device acked
			myI2COpen();myI2CStart(); WaitStartmyI2C();
			myI2Cputc((add<<1));*list = myAckStat();
			myI2CStop(); WaitStopmyI2C();myI2CClose();
		}
		return;
	}

	void I2C_GC_Reset(void)
	{
		myI2COpen();myI2CStart(); WaitStartmyI2C();
		myI2Cputc(0); myI2Cputc(0x06); 
		myI2CStop(); WaitStopmyI2C();myI2CClose();
	}
#endif
