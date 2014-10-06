// Created to hold my graphics generation code

#ifndef myGraphics_h
#define myGraphics_h

// Includes
#include "GenericTypeDefs.h"
#include "Graphics/GOL.h"
#include "GraphicsConfig.h"

#ifdef ENABLE_FILE_FUNCTIONS
#include "MDD File System/FSIO.h"
#endif

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

#ifdef ENABLE_FILE_FUNCTIONS
extern void DrawFromBinaryFile(FSFILE* source, unsigned int x, unsigned int y, unsigned int span, unsigned int columns);
#endif
extern void DrawImageFromFileHelper(const char* fileName);

extern WORD  ProcessMessageGraph( WORD translatedMsg, OBJ_HEADER* pObj, GOL_MSG* pMsg );

#endif
