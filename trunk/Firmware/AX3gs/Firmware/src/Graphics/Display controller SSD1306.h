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
 * DISPLAY functions
 * Re adapted to use a GD buffer
 * Adapted to use the Mchip graphic framework
 * Adapted for 16 bit controllers, Karim Ladha 07/03/2011
 * Dan Jackson, 2010
 */
 
#ifndef _SSD1308_H_
#define _SSD1308_H_

#include "GraphicsConfig.h"

// Setup display constants
#define DISPLAY_WIDTH 128
#define DISPLAY_WIDTH_BITS 7
#define DISPLAY_WIDTH_MASK 0x7f
#define DISPLAY_ROWS 8
#define DISPLAY_ROW_BITS 3
#define DISPLAY_ROW_MASK 0x07


extern unsigned char GDbuffer[(DISPLAY_WIDTH * DISPLAY_ROWS)];
extern unsigned char* GDbufferptr;

/*	Supports following functions in Mchps DisplayDriver.h
void ResetDevice(void);
void PutPixel(SHORT x, SHORT y);
GFX_COLOR GetPixel(SHORT x, SHORT y);
void UpdateDisplayNow(void);
WORD IsDeviceBusy();
void SetClip(BYTE control);
*/

/* 	Does NOT supoort the following from DisplayDriver.h
void TransparentColorEnable(GFX_COLOR color);
void SetActivePage(WORD page);
void SetVisualPage(WORD page);
void SetClipRgn(SHORT left, SHORT top, SHORT right, SHORT bottom); 
WORD PutImageDrv(SHORT left, SHORT top, void *image, BYTE stretch);
WORD CopyWindow( DWORD srcAddr, DWORD dstAddr,WORD srcX, WORD srcY,WORD dstX, WORD dstY,WORD width, WORD height);    
WORD CopyBlock(DWORD srcAddr, DWORD dstAddr, DWORD srcOffset, DWORD dstOffset, WORD width, WORD height); 
void CopyPageWindow( BYTE srcPage, BYTE dstPage,WORD srcX, WORD srcY,WORD dstX, WORD dstY,WORD width, WORD height); 
void SwitchOnDoubleBuffering(void); 
void SwitchOffDoubleBuffering(void); 
extern inline void __attribute__((always_inline)) InvalidateAll(void);
extern inline BYTE __attribute__((always_inline)) IsDisplayUpdatePending(void);
void InvalidateRectangle(WORD left, WORD top, WORD right, WORD bottom);
void RequestDisplayUpdate(void);
*/

// User funtions - not used by microchips graphic library

// Initialize the graphical DISPLAY
extern void DisplayInit(void);

// Refresh the graphical DISPLAY from the specified off-screen buffer
extern void DisplayRefresh(unsigned char *buffer, unsigned char span);

// Power-off the graphical DISPLAY
extern void DisplayOff(void);

// Restore display after power down modes
extern void DisplayRestore(void);

// Partial power down mode - if supported
extern void DisplaySuspend(void);

// Erase all display contents
extern void DisplayClear(void);

// Wipe the display ram only
void DisplayClearRam(unsigned char *buffer, unsigned char span);

// Primative functions for external modules
/* Added for external interfaces to DISPLAY like in DisplayBasicFont.c */
extern void DisplayWriteCommand(unsigned char);
extern void DisplayWriteData(unsigned char);
extern void DisplayClearDevice(void);

#endif // _SSD1308_H_
