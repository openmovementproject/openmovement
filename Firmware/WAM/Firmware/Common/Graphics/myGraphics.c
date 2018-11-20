// Created to hold my graphics generation code
// Adapted from Mchips pic24 example

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

#ifndef NO_FILE_SYSTEM
// This function draws a binary file onto the screen, the file must be in the correct sgmented format, if not then use the MCHP putImage functions
void DrawFromBinaryFile(FSFILE* source, unsigned int x_col, unsigned int y_row, unsigned int span, unsigned int columns)
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


#endif

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
   return 1;
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
	return 1;
}
//EOF
