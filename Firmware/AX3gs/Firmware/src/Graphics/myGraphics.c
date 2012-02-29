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
 
 * Created to hold my graphics generation code
 * Adapted from Mchips pic24 example
 */
 
// Headers
#include <Compiler.h>
#include <Graphics/Graphics.h>
#include <Graphics/Primitive.h>
#include "GraphicsConfig.h" 
#include "DisplayBasicFont.h"
#include "myGraphics.h"


// Globals
#pragma idata
volatile DWORD tick = 0;
volatile DWORD previousTick = 0;
#pragma udata


// This function draws a binary file onto the screen, the file must be in the correct sgmented format, if not then use the MCHP putImage functions
extern void DrawFromBinaryFile(FSFILE* source, unsigned int x_col, unsigned int y_row, unsigned int span, unsigned int columns)
{
	unsigned char buffer[128]; // onto the stack
	unsigned char i;

	// Check file
	if (source == NULL) return;

	// Goto x,y
	y_row&=0x7; 							// y pos to page, 0-7
	x_col&=0x7f;							// x pos 0-127
	DisplayWriteCommand((x_col>>4)|0x10); 	// high nibble with bit4 set
	DisplayWriteCommand(x_col&0x0f);		// set lower column address
	DisplayWriteCommand(0xb0|y_row);		// set GDRAM page address 0..7 (for 8 rows)
	y_row++; // next line

	// Read bytes to screen
	for (;columns>0;columns--)
	{
		// Buffer a line
		FSfread(buffer, span, 1, source);
		// Output the line
		for(i=0;i<span;i++)
		{
			DisplayWriteData(buffer[i]);
		}
		// Increment to next line, x = same, y+1
		DisplayWriteCommand((x_col>>4)|0x10); 	// high nibble with bit4 set
		DisplayWriteCommand(x_col&0x0f);		// set lower column address
		DisplayWriteCommand(0xb0|y_row++);		// set GDRAM page address 0..7 (for 8 rows)
	}
	
	return;
}

// Draws a 128*64 image from a file name
void DrawImageFromFileHelper(const char* fileName)
{
	FSFILE* image = FSfopen(fileName, FS_READ);
	DisplayRestore();
	if (image != NULL)
	{
		DrawFromBinaryFile(image, 0, 0, 128, 8);
		FSfclose(image);
	}
	else
	{
		DisplayClearDevice();
		Display_print_xy("file not found:", 0, 2, 1);
		Display_print_xy((char*)fileName, 0, 3, 1);
	}
	return;
}


// Graphics library is 'tick' driven, I'm using timer4 for this - 10ms
void TickInit( unsigned char clockSpeedMIPS )
{
    TMR4 = 0;
	switch (clockSpeedMIPS)
	{
		case (16): PR4 = 20000;break;
		case (4) : PR4 = 5000;break;
		default  : PR4 = 20000;break;
	}
    T4CON = 0b1010000000010000;//TIMER_ON | STOP_TIMER_IN_IDLE_MODE | TIMER_SOURCE_INTERNAL |GATED_TIME_DISABLED | TIMER_16BIT_MODE | TIMER_PRESCALER_8;

    IFS1bits.T4IF = 0;              //Clear flag
    IEC1bits.T4IE = 1;              //Enable interrupt
    T4CONbits.TON = 1;              //Run timer
}
void TickInitOff(void)
{
    T4CON = 0;
    IFS1bits.T4IF = 0;              //Clear flag
    IEC1bits.T4IE = 0;              //Enable interrupt
    T4CONbits.TON = 0;              //Run timer
}


// Graph specific globals
BYTE                graphData[GRAPH_DATA_BUFFER_SIZE];
WORD                graphDataHead;
WORD                graphDataTail;
BYTE                graphOffset;
WORD                graphUpdateRate;
/****************************************************************************
  Function:
    void ShowScreenGraph( void )

  Description:
    This function erases the currently displayed screen and replaces it with
    the graphing demonstration.  It draws the initial grid, and initializes
    the variables needed for the demo.

  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    The global list of graphics objects is destroyed and replaced. Therefore,
    this function cannot be called during the message processing of
    GOLMsgCallback().  It must be called during GOLDrawCallback() processing.
  ***************************************************************************/
void ShowScreenGraph( void )
{
//    strcpy( pageInformation.title, "Potentiometer Graph" );
//    InitializeScreen();
//
//    // Create the hidden controls to receive touchpad messages.
//
//    BtnCreate(          ID_GRAPH_FASTER,
//                        GetMaxX()-8, 0, GetMaxX()-6, 2, 3,
//                        BTN_HIDE,
//                        NULL,
//                        NULL,
//                        NULL );
//
//    BtnCreate(          ID_GRAPH_SLOWER,
//                        GetMaxX()-6, 0, GetMaxX()-4, 2, 3,
//                        BTN_HIDE,
//                        NULL,
//                        NULL,
//                        NULL );
//
//    BtnCreate(          ID_GRAPH_HOME,
//                        GetMaxX()-4, 0, GetMaxX()-2, 2, 3,
//                        BTN_HIDE,
//                        NULL,
//                        NULL,
//                        NULL );

    GOLPanelDraw( GRAPH_PANEL_LEFT, GRAPH_PANEL_TOP, GRAPH_PANEL_RIGHT, GRAPH_PANEL_BOTTOM, 0,
        BLACK, NULL, NULL, NULL, GOL_EMBOSS_SIZE);

    GOLPanelDrawTsk();          // draw panel for graph

    // Draw the initial grid.

    SetColor( WHITE );
    SetLineType( DOTTED_LINE );

    previousTick    = tick;
    graphOffset     = 0;
    graphDataHead   = 0;
    graphDataTail   = 0;
    graphUpdateRate = GRAPH_UPDATE_INTERVAL_INIT;

//    DrawGridVerticals();
//    DrawGridHorizontals();
}

/****************************************************************************
  Function:
    void UpdateGraph( void )

  Description:
    This routine updates the display of the scrolling graph.  The update rate
    is controlled by the variable graphUpdateRate, which the user can
    increase or decrease.  First, the old graph lines and vertical are
    erased.  The graph data is then updated with the new potentiometer
    reading.  Then the graph line and the graph horizontal and vertical lines
    are redrawn.  The horizontal lines are not affected by the scrolling,
    but if we do not redraw them, there will gaps where the previous graph
    line crossed the horizontal lines.

  Precondition:
    The graph information (graphData, graphDataHead, and graphDataTail) must
    be valid.

  Parameters:
    None

  Returns:
    None

  Remarks:
    Since normally a graph's 0 point is on the bottom, but our (0,0) is the
    upper left corner, we need to flip the graph.  This way, it matches the
    graph produced by Excel.
  ***************************************************************************/
unsigned int i;
void UpdateGraph( void )
{
    if ((tick - previousTick ) > graphUpdateRate)
    {
        previousTick = tick;

        // Remove the old graph and vertical grid lines
        SetColor( BLACK );
        DrawGraphLine();
        DrawGridVerticals();

        // Get the latest potentiometer reading.
        graphData[graphDataTail++] = graphNextSample ;

        if (graphDataTail == GRAPH_DATA_BUFFER_SIZE)
        {
            graphDataTail = 0;
        }

        // When the graph is full, update the head pointer to wrap and the display offset
        if (graphDataHead == graphDataTail)
        {
            graphDataHead ++;
            if (graphDataHead == GRAPH_DATA_BUFFER_SIZE)
            {
                graphDataHead = 0;
            }

            graphOffset ++;
            if (graphOffset >= GRID_SPACING_HORIZONTAL)
            {
                graphOffset = 0;
            }
        }

        // Draw the new graph
        SetColor( WHITE );
        DrawGraphLine();

        // Draw the updated grid
        //SetColor( WHITE );
        //DrawGridVerticals();
        //DrawGridHorizontals();
		DisplayRefresh(GDbuffer,DISPLAY_WIDTH);
    }
}

/****************************************************************************
  Function:
    void DrawGridVerticals( void )

  Description:
    This routine draws the vertical lines on the scrolling graph.  Since the
    graph is scrolling, the verticals scroll to match.  The current scroll
    offset is given by the variable graphOffset.  The routine uses the
    current line color, so it can be used to erase the current vertical lines
    by first calling SetColor( BLACK ).

  Precondition:
    The line color and scroll offset (graphOffset) must be set prior to
    calling this routine.

  Parameters:
    None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/

void DrawGridVerticals( void )
{
    SHORT   x;

    SetLineType( DOTTED_LINE );
    for (x = GRAPH_GRID_LEFT + GRID_SPACING_HORIZONTAL - graphOffset; x < GRAPH_GRID_RIGHT; x += GRID_SPACING_HORIZONTAL)
    {
        Line( x, GRAPH_GRID_TOP, x, GRAPH_GRID_BOTTOM );
    }
}

/****************************************************************************
  Function:
    void DrawGridHorizontals( void )

  Description:
    This routine draws the horizontal lines on the scrolling graph.  The
    routine uses the current line color, so it can be used to erase the
    current horizontal lines by first calling SetColor( BLACK ).

  Precondition:
    The line color must be set prior to calling this routine.

  Parameters:
    None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/

void DrawGridHorizontals( void )
{
    SHORT   y;

    SetLineType( DOTTED_LINE );
    for (y = GRAPH_GRID_TOP + GRID_SPACING_VERTICAL; y < GRAPH_GRID_BOTTOM; y += GRID_SPACING_VERTICAL)
    {
        Line( GRAPH_GRID_LEFT, y, GRAPH_GRID_RIGHT, y );
    }
}

/****************************************************************************
  Function:
    void DrawGraphLine( void )

  Description:
    This data draws the line graph of the current graph data.  The routine
    uses the current line type and color, so it can be used to erase a
    line by first calling SetColor( BLACK ).

  Precondition:
    The line color must be set prior to calling this routine.  The graph
    variables graphData, graphDataHead, and graphDataTail, must also be
    valid.

  Parameters:
    None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/

void DrawGraphLine( void )
{
    static SHORT    previousY;
    WORD            tempHead;
    SHORT           x;
    SHORT           y;

    tempHead = graphDataHead;

    for (x = 0; (x < GRAPH_GRID_WIDTH) && (tempHead != graphDataTail); x++)
    {
        if (x == 0)
        {
            PutPixel( GRAPH_GRID_LEFT + x, graphData[tempHead] );
        }
        else
        {
            if (graphData[tempHead] > previousY)
            {
                for (y = previousY + GRAPH_GRID_TOP; y <= graphData[tempHead] + GRAPH_GRID_TOP; y++)
                    PutPixel( GRAPH_GRID_LEFT + x, y );
            }
            else
            {
                for (y = previousY + GRAPH_GRID_TOP; y >= graphData[tempHead] + GRAPH_GRID_TOP; y--)
                    PutPixel( GRAPH_GRID_LEFT + x, y );
            }
        }

        previousY = graphData[tempHead];
        tempHead ++;
        if (tempHead == GRAPH_GRID_WIDTH+1)
        {
            tempHead = 0;
        }
    }
}

/*********************************************************************
* Function: WORD GOLMsgCallback(WORD objMsg, OBJ_HEADER* pObj, GOL_MSG* pMsg)
*
* Overview: The user MUST implement this function. GOLMsg() calls 
*			this function when a valid message for an object in the 
*			active list is received. User action for the message should 
*			be implemented here. If this function returns non-zero, 
*			the message for the object will be processed by default. 
*			If zero is returned, GOL will not perform any action.
*
* PreCondition: none
*
* Input: objMsg - Translated message for the object or the action ID response from the object.
*		 pObj - Pointer to the object that processed the message.
*		 pMsg - Pointer to the GOL message from user.
*
* Output: Return a non-zero if the message will be processed by default.
*		  If a zero is returned, the message will not be processed by GOL.
*
* Example:
*	<CODE> 
*	WORD GOLMsgCallback(WORD objMsg, OBJ_HEADER* pObj, GOL_MSG *pMsg){
*		static char focusSwitch = 1;
*
*		switch(GetObjID(pObj)){
*	        case ID_BUTTON1:
*	            // Change text and focus state
*	            if(objMsg == BTN_MSG_RELEASED){
*	                focusSwitch ^= 1;
*	            	if(focusSwitch){
*                   	BtnSetText((BUTTON*)pObj, "Focused");
*                    	SetState(pObj,BTN_FOCUSED);
*                	}else{
*                    	BtnSetText((BUTTON*)pObj, "Unfocused");
*                    	ClrState(pObj,BTN_FOCUSED);
*                	}
*            	}
*            	// Process by default
*            	return 1;
*	        case ID_BUTTON2:
*	            // Change text
*	            if(objMsg == BTN_MSG_PRESSED){
*	                BtnSetText((BUTTON*)pObj, "Pressed");
*            	}
*	            if(objMsg == BTN_MSG_RELEASED){
*	                BtnSetText((BUTTON*)pObj, "Released");
*	            }
*	            // Process by default
*	            return 1;
*	        case ID_BUTTON3:
*	            // Change face picture
*	            if(objMsg == BTN_MSG_PRESSED){
*	                BtnSetBitmap(pObj,arrowLeft);
*	            }
*	            if(objMsg == BTN_MSG_RELEASED){
*	                BtnSetBitmap(pObj,(char*)arrowRight);
*	            }
*	            // Process by default
*	            return 1;
*	        case ID_BUTTON_NEXT:
*	            if(objMsg == BTN_MSG_RELEASED){
*	                screenState = CREATE_CHECKBOXES;
*	            }
*	            // Process by default
*	            return 1;
*	        case ID_BUTTON_BACK:
*	            return 1;
*	        default:
*	            return 1;
*   	}
*	}              	
*	</CODE>	
*
* Side Effects: none
*
********************************************************************/
WORD GOLMsgCallback( WORD translatedMsg, OBJ_HEADER* pObj, GOL_MSG* pMsg )
{
   return ProcessMessageGraph( translatedMsg, pObj, pMsg );
}

/*********************************************************************
* Function: WORD GOLDrawCallback()
*
* Overview: GOLDrawCallback() function MUST BE implemented by 
*			the user. This is called inside the GOLDraw() 
*			function when the drawing of objects in the active 
*			list is completed. User drawing must be done here. 
*			Drawing color, line type, clipping region, graphic 
*			cursor position and current font will not be changed 
*			by GOL if this function returns a zero. To pass 
*			drawing control to GOL this function must return 
*			a non-zero value. If GOL messaging is not using 
*			the active link list, it is safe to modify the 
*			list here.
*
* PreCondition: none
*
* Input: none
*
* Output: Return a one if GOLDraw() will have drawing control 
*		  on the active list. Return a zero if user wants to 
*		  keep the drawing control.
*
* Example:
*	<CODE> 
*	#define SIG_STATE_SET   0
*	#define SIG_STATE_DRAW  1
*	WORD GOLDrawCallback(){
*		static BYTE state = SIG_STATE_SET;
*		if(state == SIG_STATE_SET){
*			// Draw the button with disabled colors
*			GOLPanelDraw(SIG_PANEL_LEFT,SIG_PANEL_TOP,
*						 SIG_PANEL_RIGHT,SIG_PANEL_BOTTOM, 0,
*						 WHITE, altScheme->EmbossLtColor,
*						 altScheme->EmbossDkColor,
*						 NULL, GOL_EMBOSS_SIZE);
*
*			state = SIG_STATE_DRAW;
*		}
*		
*		if(!GOLPanelDrawTsk()){
*	    	// do not return drawing control to GOL
*      		// drawing is not complete
*			return 0;
*		}else{
*			state = SIG_STATE_SET;
*			// return drawing control to GOL, drawing is complete
*			return 1;
*		}
*	}        	
*	</CODE>	
*
* Side Effects: none
*
********************************************************************/
WORD GOLDrawCallback(void)
{
 	UpdateGraph();
	return 1;
}
/****************************************************************************
  Function:
    WORD ProcessMessageGraph( WORD translatedMsg, OBJ_HEADER* pObj,
                    GOL_MSG* pMsg )

  Description:
    This function processes the messages for the potentiometer graphing.
    The right touchpad increases the graph update rate, the left touchpad
    decreases the graph update rate, and the up touchpad exits the demo.

  Precondition:
    Call ShowScreenGraph() prior to using this function to display the
    correct screen.

  Parameters:
    WORD translatedMsg  - The translated control-level message
    OBJ_HEADER* pObj    - Object to which the message applies
    GOL_MSG* pMsg       - The original system message information

  Return Values:
    0   - Do not call the default message handler.
    1   - Call the default message handler.

  Remarks:
    None.
  ***************************************************************************/

WORD ProcessMessageGraph( WORD translatedMsg, OBJ_HEADER* pObj, GOL_MSG* pMsg )
{

//    WORD    controlID;
//
//    controlID = GetObjID(pObj);
//
//    switch (controlID)
//    {
//        case ID_GRAPH_SLOWER:
//            if (translatedMsg == BTN_MSG_PRESSED)
//            {
//                if (graphUpdateRate < GRAPH_UPDATE_INTERVAL_MAX)
//                {
//                    graphUpdateRate += GRAPH_UPDATE_INTERVAL_STEP;
//                }
//            }
//            break;
//
//        case ID_GRAPH_FASTER:
//            if (translatedMsg == BTN_MSG_PRESSED)
//            {
//                if (graphUpdateRate > GRAPH_UPDATE_INTERVAL_MIN)
//                {
//                    graphUpdateRate -= GRAPH_UPDATE_INTERVAL_STEP;
//                }
//            }
//            break;
//
//        case ID_GRAPH_HOME:
//            if (translatedMsg == BTN_MSG_PRESSED)
//            {
//                if (captureFile)
//                {
//                    FSfclose( captureFile );
//                    captureFile = NULL;
//                }
//                SetLineType( SOLID_LINE );
//
//                // Shut down the USB, in case we were capturing.
//                USBHostShutdown();
//
//                screenState = SCREEN_DISPLAY_MAIN;
//            }
//            break;
//
//        default:
//            break;
//    }

    return 0;
}
