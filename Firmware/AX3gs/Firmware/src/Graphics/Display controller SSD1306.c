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
 *
 *
 * Display driver for controller SSD1306 (similar to 1308)
 * Written in a new format which will be used to control all page orientated displays in future
 * Karim Ladha 08-12-11
 */

#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "TimeDelay.h"
#include "HardwareProfile.h"

// This has the constants in for this .c file in
#include "GraphicsConfig.h" 			


#ifndef EMULATOR

// Globals
// GD buffer
#pragma idata
unsigned char GDbuffer[(DISPLAY_WIDTH * DISPLAY_ROWS)] = {0};
unsigned char* GDbufferptr = GDbuffer;
#pragma udata

/*Microchip display drive functions*/
//#include "Graphics/Graphics.h"
//#include "Graphics/CustomDisplayDriver.h"

// Used by the Mchips pixel read/write
#define PIXEL_VAL_ON 	0xffff
#define PIXEL_VAL_OFF 	0x0000

// Color
WORD_VAL    _color = {0};	// = (WORD_VAL)PIXEL_VAL_ON;

// Clipping region control
SHORT       _clipRgn;

// Clipping region borders
SHORT       _clipLeft;
SHORT       _clipTop;
SHORT       _clipRight;
SHORT       _clipBottom;

// Controller specific functions

/*If used with the parralel port with a NAND flash device, be careful not to call these at the
  same time since they share a HW module. If bit banged then both devices must be too. */
#ifdef DISPLAY_BITBANGED
	#error "Depreciated - This mode is too slow to be useful, only ever written for PIC18 devices"
#else 
	// Hardware port used
	#define DelaySetup()	__builtin_nop()
	#ifndef DISPLAY8080
		#error "No suport in HW for this mode, should be 8080 to be same as NAND flash devices"
	#endif
	
	// If we know the PSP is configured to complet in one clock cycle then we dont need to wait.
	#define PSPWait();				{while (PMMODEbits.BUSY);}			/*Waits for byte to be clocked out*/
	#define DisplayWriteRaw(_value) {DISPLAY_DATA = _value;PSPWait();}	/*Initiates HW to write byte*/
	#define DisplayReadRaw()		(DISPLAY_DATA){PSPWait();}			/*You can't use NandReadRaw() as a dummy read, use x=NandReadRaw()*/
	// Initialise the PSP module to interface to the DISPLAY
	void DisplayInterfaceOn(void)
	{ // Tested 07/03/2011 K.Ladha
		PMCON = 	0b1010001100000000;  /*	:MSB: PMPEN=1 : NA = 0 : PSIDL = 1 : ADRMUX = 00 : PTBEEN = 0 : PTWREN = 1 : PTRDEN = 1 */\
										 /*	:LSB: CSF = 00 : ALP = 0 : CS2P = 0 : CS1P = 0 : BEP = 0 : WRSP = 0 : RDSP = 0 */
		PMMODE = 	0b0000001000000100;  /* :MSB: BUSY = 0 : IRQM = 00 : INCM = 00 : MODE16 = 0 : MODE = 10 */\
										 /*	:LSB: WAITB = 00 : WAITM = 0001 : WAITE = 00 (2 Tcy byte write) Set to 1 tcy if at 4 MIPS*/
		PMADDR =	0b0000000000000000;  /* CS2 = 0 : CS1 = 0 : ADDR : 0b00000000000000 */
		PMAEN  = 	0b0000000000000000;  /* PTEN = 0000 0000 0000 00 : PTEN = 00 */  
	}
#endif

// Write a command to the DISPLAY
void DisplayWriteCommand(unsigned char command)
{
    DISPLAY_CHIP_SELECT = 0;
    DISPLAY_COMMAND_DATA = 0;          // Command
    DelaySetup();
    DisplayWriteRaw(command);
    DISPLAY_CHIP_SELECT = 1;
	DelaySetup();
}

// Write data to the DISPLAY
void DisplayWriteData(unsigned char data)
{
    DISPLAY_CHIP_SELECT = 0;
	DISPLAY_COMMAND_DATA = 1;          // Data
    DelaySetup();
    DisplayWriteRaw(data);
    DISPLAY_CHIP_SELECT = 1;
	DelaySetup();
}
	
// Initialize the graphical DISPLAY display
void DisplayInit(void)
{
	DISPLAY_INIT_PINS();		// HW profile - inits to inactive	
	DisplayInterfaceOn();

    DISPLAY_RESET = 0; 			// reset device
	DelayMs(50);
    DISPLAY_RESET = 1;
	DelayMs(50);

	DisplayWriteCommand(0xa8); // Set MUX ratio	
	DisplayWriteCommand(0x3f); // Set MUX ratio	

	DisplayWriteCommand(0xd3); // Set display offset	
	DisplayWriteCommand(0x00); // Set display offset
	
	DisplayWriteCommand(0x40); // Set display start line to 0	

	//DisplayWriteCommand(0xa0); // Set segment remap	
	DisplayWriteCommand(0xa1); // Set segment remap	reverse

	//DisplayWriteCommand(0xc0); // Com direction normal
	DisplayWriteCommand(0xc8); // Com direction inverse
	
	DisplayWriteCommand(0xda); // Set COM pins configuration	
	DisplayWriteCommand(0x12); // configuration - set to default (disable com L/R remap, Alternate com config)

    DisplayWriteCommand(0x81); // Set Contrast Control:
    DisplayWriteCommand(0xff); // default: 0x7f
	
	DisplayWriteCommand(0xa4); // Disable entire display on
	
	DisplayWriteCommand(0xa6); // Set normal display mode
	//DisplayWriteCommand(0xa7); // Set Inverse Display mode

	DisplayWriteCommand(0xd5); // Set oscillator frequency
	DisplayWriteCommand(0x80); // 0-f freq (8), 0-f prescale (1)
	
	DisplayWriteCommand(0x8d); // Enable charge pump regulator	
	DisplayWriteCommand(0x14); // turn on	
	//DisplayWriteCommand(0x10); // turn off

    DisplayWriteCommand(0x20); // Set Memory Addressing Mode:
    //DisplayWriteCommand(0x00); // horizontal addressing mode
    //DisplayWriteCommand(0x01); // vertical addressing mode
    DisplayWriteCommand(0x02); // page addressing mode

	DisplayWriteCommand(0x21); // set column address range
    DisplayWriteCommand(0x00); // first column
    DisplayWriteCommand(0x7f); // last column

    DisplayWriteCommand(0x22); // set page address range
    DisplayWriteCommand(0x00); // set low col address
    DisplayWriteCommand(0x07); // set high col address
   	DisplayWriteCommand(0xb0); // start writing from page 0

	//DisplayWriteCommand(0xa0); // segment address remap normal
	DisplayWriteCommand(0xa1); // segment address remap reverse
	
	//DisplayWriteCommand(0xd9); // Set precharge segment period?
	//DisplayWriteCommand(0x22); // Left as default

	//DisplayWriteCommand(0xdb); // Set vcomh deselect level?
	//DisplayWriteCommand(0x20); // Left as default
  
    DisplayWriteCommand(0xaf); // display on 
    //DisplayWriteCommand(0xae); // display off - sleep 

    // Wait for changes
	Delay10us(100);
	// Clear display
	DisplayClear();
}

// Power saving modes
void DisplayOff(void) // <1uA - call DisplayInit / Restore to resume
{
	DisplayWriteCommand(0xae); // display off - sleep
}
extern void DisplaySuspend(void)
{
	DisplayWriteCommand(0xae); // display off - Only LCD's support partial power down
}
void DisplayRestore(void)
{
	DisplayWriteCommand(0xaf); // display on 
}

// Clear the display
void DisplayClearDevice(void)
{
	unsigned char column, page;
	
    DISPLAY_CHIP_SELECT = 0;
    DelaySetup();
	for (page = 0; page < DISPLAY_ROWS; page++)
	{
		DISPLAY_COMMAND_DATA = 0;					// command select
		DelaySetup();
		DisplayWriteRaw(0x10);						// set higher column address = 0
		DisplayWriteRaw(0x00);						// set lower column address	= 0
		DisplayWriteRaw(0xb0 | page);				// set page
		for (column = 0; column < DISPLAY_WIDTH; column++)
		{
			DISPLAY_COMMAND_DATA = 1;			// data select
			DelaySetup();
#ifdef DISPLAY_DEBUG
			DisplayWriteRaw(((column >> 2) & 1) ? 0xf0 : 0x0f); // debug pattern - should be 4x4 pixel checkerboard on full display
#else
			DisplayWriteRaw(0x00);					// clear
#endif
		}
	}
	DISPLAY_COMMAND_DATA = 0;					// command select
	DelaySetup();
	DisplayWriteRaw(0x10);							// set higher column address = 0
	DisplayWriteRaw(0x00);							// set lower column address	= 0
	DisplayWriteRaw(0xb0);							// page increment = 0
    DISPLAY_CHIP_SELECT = 1;
	DelaySetup();
}

// Clear from RAM
void DisplayClearRam(unsigned char *buffer, unsigned char span)
{
	// GDbuffer is is same page orientated structure as the display
	// i,e, the first 128 bytes is the top line 128x8 pixels
	unsigned char i;
	if (span == DISPLAY_WIDTH)
	{
		memset(buffer ,0, (DISPLAY_WIDTH * DISPLAY_ROWS));
	}
	for (i=0;i<8;i++) // all 8 lines
	{
		memset(buffer ,0, span);
		buffer += (DISPLAY_WIDTH - span);
	}
}

void DisplayClear(void)
{
	DisplayClearRam(GDbuffer, DISPLAY_WIDTH);
	DisplayClearDevice();
}

void DisplayRefresh(unsigned char *buffer, unsigned char span)
{
	unsigned char row,col;
	unsigned char *src = buffer;

    DISPLAY_CHIP_SELECT = 0;
    DelaySetup();

// Note: I'm not going to update the top two lines so
// I can show the time etc on them.
#ifndef DONT_REFRESH_TOP_DISP_LINES
	for(row=0;row<DISPLAY_ROWS;row++)
	{
#else
	src += span;src += span;
	for(row=2;row<DISPLAY_ROWS;row++)
	{
#endif
		// Load DISPLAY data address column and row 
		DISPLAY_COMMAND_DATA = 0;				// command select
		DelaySetup();

		// Select the byte
		DisplayWriteRaw(0x10);						// set higher column address = 0
		DisplayWriteRaw(0x00);						// set lower column address	= 0
		DisplayWriteRaw(0xb0 | row);				// set page

		DISPLAY_COMMAND_DATA = 1;			// data select
		DelaySetup();

		for(col=0;col<DISPLAY_WIDTH;col++)
		{
			DisplayWriteRaw(*src++);
		}

		// Skip any padding bytes between rows
		src += span - DISPLAY_WIDTH;
	}

    DISPLAY_CHIP_SELECT = 1;					// Deselect
	DelaySetup();
}



// Microchips compatibility functions

/*********************************************************************
* Function:  void ResetDevice()
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: resets DISPLAY, initializes PMP
*
* Note: none
*
********************************************************************/
void ResetDevice(void)
{ 
	//DisplayInit();
	DisplayClear();
}

/*********************************************************************
* Function:  void UpdateDisplayNow(void)
*
* Overview: Synchronizes the draw and frame buffers immediately. 
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void UpdateDisplayNow(void)
{
	DisplayRefresh(GDbuffer, DISPLAY_WIDTH);
}

/*********************************************************************
* Function: void PutPixel(SHORT x, SHORT y)
*
* PreCondition: none
*
* Input: x,y - pixel coordinates
*
* Output: none
*
* Side Effects: none
*
* Overview: puts pixel
*
* Note: none
*
********************************************************************/
void PutPixel(SHORT x, SHORT y)
{ 
	unsigned char column,temp,bit_mask;//page;
	unsigned int page;
	unsigned char* GDptr = GDbufferptr;
	
	// Convert coordinates
	column = x&DISPLAY_WIDTH_MASK; // 0-128
	page = (y>>DISPLAY_ROW_BITS)&DISPLAY_ROW_MASK; // 0-8
	temp = y&DISPLAY_ROW_MASK;
	bit_mask = 1<<temp;

	GDptr+= column+(DISPLAY_WIDTH*page);	

	// Set the bit
	if (_color.Val) // Pixel on
	{
		*GDptr|=bit_mask;
	}
	else // Pixel off
	{
		*GDptr&=~bit_mask;
	}
	
	return;
}

/*********************************************************************
* Function: WORD GetPixel(SHORT x, SHORT y)
*
* PreCondition: none
*
* Input: x,y - pixel coordinates 
*
* Output: pixel color
*
* Side Effects: none
*
* Overview: returns pixel color at x,y position
*
* Note: none
*
********************************************************************/
WORD GetPixel(SHORT x, SHORT y)
{
	unsigned char column,temp,bit_mask;//page;
	unsigned int page;
	unsigned char* GDptr = GDbufferptr;
	
	// Convert coordinates
	column = x&DISPLAY_WIDTH_MASK; // 0-128
	//page = (y>>DISPLAY_ROW_BITS)&DISPLAY_ROW_MASK; // 0-8
	temp = y&DISPLAY_ROW_MASK;
	bit_mask = 1<<temp;

	//GDptr+= column+(DISPLAY_WIDTH*page);	

	// Optimise
	page = (y<<(DISPLAY_WIDTH_BITS-DISPLAY_ROW_BITS)); // 0-8 * 128
	GDptr+= column+page;

	// Set the bit
	if (*GDptr&bit_mask) // Pixel on
	{
		return PIXEL_VAL_ON;
	}
	else // Pixel off
	{
		return PIXEL_VAL_OFF;
	}
}
/*********************************************************************
* Function: SetClip(control)
*
* Overview: Enables/disables clipping.
*
* PreCondition: none
*
* Input: control - Enables or disables the clipping.
*			- CLIP_DISABLE: Disable clipping
*			- CLIP_ENABLE: Enable clipping
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void SetClip(BYTE control)
{
	//TODO: Figure out what this function is supposed to do? 
	return;
}
/*********************************************************************
* Function: IsDeviceBusy()
*
* Overview: Returns non-zero if LCD controller is busy 
*           (previous drawing operation is not completed).
*
* PreCondition: none
*
* Input: none
*
* Output: Busy status.
*
* Side Effects: none
*
********************************************************************/
WORD IsDeviceBusy()
{
	// TODO: add function that checks whether current GD buffer is on screen
	return 0;
}

// End Mchip compatibility code




#else



// Emulated DISPLAY update functions
void DisplayInit(void) { ; }
void DisplayOff(void) { ; }
void DisplayRefresh(unsigned char *buffer, unsigned char span)
{
    EmuDisplayRefresh(buffer, span);
}


#endif
