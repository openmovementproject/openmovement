// Write application code here, call from main.c
// Diana Nowacka
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Common/Analog.h"

volatile char button_clicked = 0;
char ir_mode = 0;

void RunApp(void)   
{  
	while(!USB_BUS_SENSE)
	{  	
        if (!BUTTON) 
	    {
    	    if (button_clicked == 0)
    	    {
        	    button_clicked = 1;
        	   // nothing is happening here really
    	    }
        }
        else button_clicked = 0; 
	}
    
    IEC0bits.T3IE = 0;
    IEC1bits.T4IE = 0;
}
