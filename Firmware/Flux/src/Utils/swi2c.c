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

// Fast software I2C code
// Karim Ladha, 2011

#include <Compiler.h>
#include "myI2C.h"
#include "HardwareProfile.h"

#if (!defined(mySCL) || !defined(mySDAr) || !defined(mySDAw))
	#error "definitions needed"
#endif

// I need a very fast variable for my I2C code that does not require banking
#pragma udata access my_access
/* all accesses to these will be unbanked */
near unsigned char my_I2C_working_var;
near unsigned char my_ISR_I2C_working_var;
#pragma udata

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


extern void I2CBusDebug(char* list)
{
unsigned char add;
	for (add=0;add<0x7f;add++,list++)
	{
		// This polls every address
		myI2CStart();
		my_I2C_working_var = (add<<1);
		*list = _myI2Cputc();
		myI2CStop();
	}
return;
}