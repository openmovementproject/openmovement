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
 * Created to hold my graphics generation code
 */
 
#ifndef myGraphics_h
#define myGraphics_h

// Includes
#include "GenericTypeDefs.h"
#include "Graphics/GOL.h"
#include "GraphicsConfig.h"
#include "MDD File System/FSIO.h"

// Defines - Add the defines used by the graphic .c files in here
#define MILLISECONDS_PER_TICK       10

#define GRAPH_DATA_BUFFER_SIZE      (GRAPH_GRID_WIDTH + 1)

#define GRAPH_GRID_LEFT             (GRAPH_PANEL_LEFT + GOL_EMBOSS_SIZE)
#define GRAPH_GRID_RIGHT            (GRAPH_PANEL_RIGHT - GOL_EMBOSS_SIZE)
#define GRAPH_GRID_TOP              (GRAPH_PANEL_TOP + GOL_EMBOSS_SIZE)
#define GRAPH_GRID_BOTTOM           (GRAPH_PANEL_BOTTOM - GOL_EMBOSS_SIZE)

#define GRAPH_GRID_WIDTH            (GRAPH_GRID_RIGHT - GRAPH_GRID_LEFT)
#define GRAPH_GRID_HEIGHT           (GRAPH_GRID_BOTTOM - GRAPH_GRID_TOP)

#define GRAPH_PANEL_LEFT            0
#define GRAPH_PANEL_RIGHT           126
#define GRAPH_PANEL_TOP             16
#define GRAPH_PANEL_BOTTOM          63 

#define GRAPH_UPDATE_INTERVAL_INIT  (10 / MILLISECONDS_PER_TICK)
#define GRAPH_UPDATE_INTERVAL_MAX   (2000 / MILLISECONDS_PER_TICK)
#define GRAPH_UPDATE_INTERVAL_MIN   (MILLISECONDS_PER_TICK / MILLISECONDS_PER_TICK)
#define GRAPH_UPDATE_INTERVAL_STEP  (MILLISECONDS_PER_TICK / MILLISECONDS_PER_TICK)

#define GRID_SPACING_VERTICAL       (GRAPH_GRID_HEIGHT >> 3)    // ((GRAPH_GRID_BOTTOM - GRAPH_GRID_TOP) >> 3)
#define GRID_SPACING_HORIZONTAL     (GRAPH_GRID_WIDTH >> 3)     // ((GRAPH_GRID_RIGHT - GRAPH_GRID_LEFT) >> 3)

// Globals
extern unsigned int graphNextSample;

// Prototypes
extern void TickInit( unsigned char clockSpeedMIPS );
extern void TickInitOff(void);
extern void ShowScreenGraph( void );
extern void UpdateGraph( void );
extern void DrawGridVerticals( void );
extern void DrawGridHorizontals( void );
extern void DrawGraphLine( void );
extern void DrawFromBinaryFile(FSFILE* source, unsigned int x, unsigned int y, unsigned int span, unsigned int columns);
extern void DrawImageFromFileHelper(const char* fileName);

// This function draws a binary file onto the screen, the file must be in the correct sgmented format, if not then use the MCHP putImage functions
extern void DrawFromBinaryFile(FSFILE* source, unsigned int x_col, unsigned int y_row, unsigned int span, unsigned int columns);
extern WORD  ProcessMessageGraph( WORD translatedMsg, OBJ_HEADER* pObj, GOL_MSG* pMsg );

#endif
