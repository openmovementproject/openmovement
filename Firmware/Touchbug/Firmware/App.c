// Write application code here, call from main.c
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Common/Analog.h"

volatile char button_clicked = 0;
char ir_mode = 0;

void RunApp(void)   
{  
    
   // MOTOR1_DUTY = 0x4E20;
    //MOTOR2_DUTY = 0x0;
    //IR_TX1_DUTY = 0xFF;
    //IR_TX2_DUTY = 0xFF;
	while(!USB_BUS_SENSE)
	{  	
        if (!BUTTON) 
	    {
    	    if (button_clicked == 0)
    	    {
        	    button_clicked = 1;
        	   // ir_mode = ! ir_mode; // switch the pwm
    	    }
        }
        else button_clicked = 0; 
	}
    
    IEC0bits.T3IE = 0;
    IEC1bits.T4IE = 0;
}
