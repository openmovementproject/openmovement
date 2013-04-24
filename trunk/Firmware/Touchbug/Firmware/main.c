// CWA3.1 Karim Ladha 08/12/12
// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "stdint.h"
#include "HardwareProfile.h"
#include "Util.h"

#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Rtc.h"
#include "Common/Analog.h"
#include "Peripherals/myi2c.h"
#include "USB/USB.h"
#include "USB/usb_function_cdc.h"
#include "USB/USB_CDC_MSD.h"
#include "Settings.h"
#include "App.h"

#define DEVICE 0

// Prototypes
extern void RunAttached(void);
extern void RunClockTasks(unsigned char forceOn);

// Globals 

#define IR_ON 0xff
#define IR_OFF 0x50
             // Frequencies for
             // Device 0 Device 1 Device 2 Device 3 Device 4
static short DelayLEDAon[5] = { 4, 2, 4, 2, 9};
static short DelayLEDAoff[5] = { 18, 9, 10 , 5, 23};
static short DelayLEDBon[5] = { 2, 5, 6, 9, 10};
static short DelayLEDBoff[5] = { 7, 15, 11, 18, 28};

unsigned char IR1_COUNTDOWN = 0, IR2_COUNTDOWN = 0;
unsigned char IR1_STATE = 0, IR2_STATE = 0;
unsigned int IR1_DUTY = 0;
unsigned int IR2_DUTY = 0;

signed int accel_z = 0; // summed lengths of movement
signed int gyro_z = 0; // summed degrees of rotation
short firstpos = 0; // initial forward direction (in degrees from gyro)
char turn = 0; 
char accellock = 0; // internal variable which controls motor strength by accelerometer
int acc = 0;
unsigned int counter = 0;
unsigned int normalpower = 0x70FF;
char turnedoff = 0;

void CheckBatteryState(unsigned short battery)
{
    if (battery < 708) LED_SET(LED_GREEN);
    if (battery < 670) LED_SET(LED_YELLOW);
    if (battery < 630) LED_SET(LED_RED);
    return;
}
// Light sensor colors
char GetDirectionFromIR( unsigned short ir1,  unsigned short ir2)
{
    unsigned short ir1white,ir2white,ir1gray,ir2gray,ir1black,ir2black;
    
    if (DEVICE == 0)
    {
        ir1white = 700;//480; //433; 
        ir2white = 700;//608; // 649 
    
        ir1gray = 370;//257; 
        ir2gray = 370;//301; //371 
    
        ir1black = 18; //18
        ir2black = 40; //40;
    }
    else if (DEVICE == 1)
    {
        ir1white = 500; // 400
        ir2white = 600; // 550
    
        ir1gray = 164;
        ir2gray = 331; // 281
    
        ir1black = 33;
        ir2black = 55; 
    }
    else if (DEVICE == 2)
    {
        ir1white = 600;//924;//530; //530;
        ir2white = 450;//900;//550;//573;
    
        ir1gray = 200;//600;//300; //303; 
        ir2gray = 200;//680; //300;//346;
    
        ir1black = 56;//60; //46;
        ir2black = 0;//0;//51; 
    }
    else if (DEVICE == 3)
    {
        ir1white = 904;//535; 
        ir2white = 950;//900;
    
        ir1gray = 350; //385; 
        ir2gray = 450;//600;
    
        ir1black = 147;//50; //195;
        ir2black = 130;//15; 
    }
	else if (DEVICE == 4)
    {
        ir1white = 900;//565; 
        ir2white = 920;//503;
    
        ir1gray = 618;//411; 
        ir2gray = 520;//449;
    
        ir1black = 58;//50; //195;
        ir2black = 40;//15; 
    }
    //1 black, 2 gray, 3 white
    char i1,i2; // determine the light by finding the nearest neighbour
    if (abs(ir1 -ir1white) < abs(ir1 -ir1gray)) // if its not white
    {
        i1 = 3;
    }
    else if (abs(ir1 -ir1black) < abs(ir1 -ir1gray)) // or black
    {
        i1 = 1; 
    }     
    else i1 = 2; // its gray
    
    if (abs(ir2 -ir2white) < abs(ir2 -ir2gray)) // same for second phototransistor
    {
        i2 = 3;
    }
    else if (abs(ir2 -ir2black) < abs(ir2 -ir2gray))
    {
        i2 = 1; 
    }     
    else i2 = 2;
 
    if (i1 == 3 && i2 == 2) // 1 - ir1 white, ir2 gray  
        return 1;    
    else if (i1 == 3 && i2 == 1) // 2 - ir1 white, ir2 black
        return 2;
    else if (i1 == 2 && i2 == 2) // 3 - ir1 gray, ir2 gray 
        return 3;
    else if (i1 == 2 && i2 == 1) // 4 - ir1 gray, ir2 black
        return 4;
    else if (i1 == 1 && i2 == 2) // 5 - ir1 black, ir2 gray
        return 5;
    else if (i1 == 1 && i2 == 1) // 6 - ir1 black, ir2 black  
        return 6;
    else if (i1 == 2 && i2 == 3) // 7 - ir1 gray, ir2 white  
         return 7;
    else if (i1 == 1 && i2 == 3) // 8 - ir1 black, ir2 white 
        return 8;   
          
    return 0;    // 0 - both white
}

void IrLEDs(char a) // Set the LEDs according to the dectected light from sensor
{
    switch (a)
    {
        case 0: 
            LED_SET1(LED_WHITE); LED_SET2(LED_WHITE);
            break;
        case 1:
            LED_SET1(LED_WHITE); LED_SET2(LED_BLUE);
            break;
        case 2:   
            LED_SET1(LED_WHITE); LED_SET2(LED_OFF);
            break;
        case 3:   
            LED_SET1(LED_BLUE); LED_SET2(LED_BLUE);
            break;
        case 4:   
            LED_SET1(LED_BLUE); LED_SET2(LED_OFF); 
            break;
        case 5: 
            LED_SET1(LED_OFF); LED_SET2(LED_BLUE); 
            break;
        case 6: 
            LED_SET(LED_OFF);
            break; 
        case 7: 
            LED_SET1(LED_BLUE); LED_SET2(LED_WHITE);  
            break;
        case 8: 
            LED_SET1(LED_OFF); LED_SET2(LED_WHITE); 
            break;
    }    
    return;
}    
// Interrupts

void __attribute__((interrupt,auto_psv)) _DefaultInterrupt(void)
{
 	static unsigned int INTCON1val;
	LED_SET(LED_RED);
	INTCON1val = INTCON1;
	Nop();
	Nop();
	Nop();
	Nop();
	INTCON1 = 0;
  //  Reset();
}

// RTC
void __attribute__((interrupt,auto_psv)) _RTCCInterrupt(void)
{
	RtcTasks();
    RtcSwwdtIncrement();    // Increment software RTC, reset if overflow
}

// TMR1
void __attribute__((interrupt, shadow, auto_psv)) _T1Interrupt(void)
{
    RtcTimerTasks();
//	BT_tick_handler();
}

void InitTimer3(void) // 
{
	T3CONbits.TCS 	= 0;	//Internal clock source
 	T3CONbits.TGATE = 0;	//Gated time accumulation disabled
	T3CONbits.TCKPS = 0b00;	//1:1 prescale value
	T3CONbits.TSIDL = 0; 	//continue timer operation in idle mode
	T3CONbits.TON = 0;		//turn off TMR3
	TMR3 = 0; 				//reset(clear TMR3)
	PR3 = 0xFFFF; 			//set PR3 to basically check for overflow
	T3CONbits.TON = 1;		//turn on TMR3
	IFS0bits.T3IF = 0;		//clear TMR3IR flag
	IEC0bits.T3IE = 0;		//Disable TMR3 interrupts
}
void StartTimer3(void)
{
	IFS0bits.T3IF = 0;		//Clear TMR3 interruptflag	
	IEC0bits.T3IE = 1;		//Enable TMR3 interrupts
}

void InitTimer4(void)
{
	T4CONbits.TCS 	= 0;	//Internal clock source
 	T4CONbits.TGATE = 0;	//Gated time accumulation disabled
	T4CONbits.TCKPS = 0b00;	//1:1 prescale value
	T4CONbits.TSIDL = 0; 	//continue timer operation in idle mode
	T4CONbits.TON = 0;		//turn off TMR4
	TMR4 = 0; 				//reset(clear TMR4)
	PR4 = 0xFFFF; 			//set PR4 to basically check for overflow
	T4CONbits.TON = 1;		//turn on TMR4
	IFS1bits.T4IF = 0;		//clear TMR4IR flag
	IEC1bits.T4IE = 0;		//Disable TMR4 interrupts
}
void StartTimer4(void)
{
	IFS1bits.T4IF = 0;		//Clear TMR4 interruptflag	
	IEC1bits.T4IE = 1;		//Enable TMR4 interrupts
}

void InitTimer5(void)
{
	T5CONbits.TCS 	= 0;	//Internal clock source
 	T5CONbits.TGATE = 0;	//Gated time accumulation disabled
	T5CONbits.TCKPS = 0b10; //1:64 prescale value
	T5CONbits.TSIDL = 0; 	//continue timer operation in idle mode
	T5CONbits.TON = 0;		//turn off TMR5
	TMR5 = 0; 				//reset(clear TMR5)
	PR5 = 0xFFFF; 			//set PR5 to basically check for overflow
	T5CONbits.TON = 1;		//turn on TMR5
	IFS1bits.T5IF = 0;		//clear TMR5IR flag
	IEC1bits.T5IE = 0;		//Disable TMR5 interrupts
}
void StartTimer5(void)
{
	IFS1bits.T5IF = 0;		//Clear TMR3 interruptflag	
	IEC1bits.T5IE = 1;		//Enable TMR3 interrupts
}
// deactivates the touchbug when it is laid on its back
void __attribute__((interrupt,auto_psv)) _T5Interrupt(void)
{
    accel_t accelSample;
    AccelSingleSample(&accelSample);

// check if device is the right way round
	if ( accelSample.z < 0 && turnedoff == 0) // turn everything off
	{
		//SystemPwrSave(GYRO_POWER_DOWN|BT_POWER_DOWN|ADC_POWER_DOWN);
    	LED_SET(LED_OFF);
	   
		IEC0bits.T3IE = 0; // turn off the timers
		IEC1bits.T4IE = 0;
		MOTOR_DISABLE();
		GyroStandby();
		AdcOff();
		IR_TX1_DUTY = 0x0;
		IR_TX2_DUTY = 0x0;
		IR_TX_DISABLE();
		turnedoff = 1;
    }
    else if (turnedoff == 1 && accelSample.z > 0)
    {
	    GyroStartup();
        IEC0bits.T3IE = 1;
	    IEC1bits.T4IE = 1;
	    MOTOR_INIT_PWM();
	    AdcInit();
	    IR_TX_INIT_PWM()
	    turnedoff = 0; 
   }
	IFS1bits.T5IF = 0;
}

// manage the IR light
void __attribute__((interrupt,auto_psv)) _T3Interrupt(void)
{ 
    if (IR1_COUNTDOWN-- == 0)
    {
        if (IR1_STATE == 0)
        {
            if (ir_mode == 0) IR1_COUNTDOWN = DelayLEDAon[DEVICE];
            else IR1_COUNTDOWN = DelayLEDAoff[DEVICE];
        }
        else
        {
            if (ir_mode == 0) IR1_COUNTDOWN = DelayLEDAoff[DEVICE];           
            else IR1_COUNTDOWN = DelayLEDAon[DEVICE];
        }
        	IR1_STATE = !IR1_STATE;         
			IR1_DUTY = IR1_STATE ? IR_ON : IR_OFF;
    }
    
     if (IR2_COUNTDOWN-- == 0)
    {
        if (IR2_STATE == 0)
        {
            if (ir_mode == 0) IR2_COUNTDOWN = DelayLEDBon[DEVICE];
            else IR2_COUNTDOWN = DelayLEDBoff[DEVICE];
        }
        else 
        {
            if (ir_mode == 0) IR2_COUNTDOWN = DelayLEDBoff[DEVICE];           
            else IR2_COUNTDOWN = DelayLEDBon[DEVICE];
        }
        	IR2_STATE = !IR2_STATE;         
			IR2_DUTY = IR2_STATE ? IR_ON : IR_OFF;
    }
    IR_TX1_DUTY = IR1_DUTY; 		// depending on the counter set ir led intensity
    IR_TX2_DUTY = IR2_DUTY;
    
	IFS0bits.T3IF = 0;				//Clear TMR3 interruptflag
}

void __attribute__((interrupt,auto_psv)) _T4Interrupt(void)
{ 
   char accelmove = 1; 			    // decide here if you want to increase mottr strength by acceleration, 1 = yes
   
    if (accelmove == 1) 		    // if power is controlled by shaking
    {   
        if (counter > 500) 
        {
            counter = 0;
            accellock = 0;   
        }    
    
    if (counter != 0) counter++;    // counter = 0 means turned off
    }
     // threshold values, forward direction
    short drift1 = 10;		        // tolerance to the left, ignore any deviation under 10 and 
    signed short drift2 = -5;       // tolerance right
    short thr = 50; 		        // threshold, when gyrodata should be stored -- other way round
    signed short thl = -40;         // threshold     

   short c = 20; 			        // factor for linear function for motor strength (differs for each touchbug)
   if (DEVICE == 3) {c = 15;}
   if (DEVICE == 2) { /*normalpower = 0x30FF;*/ c = 20;}
   if (DEVICE == 1) {normalpower = 0x30FF; c = 5;}
   
    unsigned int strength1 = normalpower; //left motor
    unsigned int strength2 = normalpower; //right motor
  
    accel_t accelSample;
    AccelSingleSample(&accelSample);
    
   if (accelmove == 1)              // check if device is shaked
   if (accellock == 0)              // if it is in slow mode
   {
       acc = (abs(accelSample.x) >> 5) + (abs(accelSample.y) >> 5) + (abs(accelSample.z) >> 5);
       strength1 = strength2 = normalpower;
       if (acc > 2500)
       { 
           ir_mode = ! ir_mode;
		   LED_SET1(LED_GREEN);
           counter = 1;
           acc = 0;
           accellock = 1;
       }
   }
    
    // process gyrodata (steer device by setting motor strength) 
    gyro_t gyrodata;
    GyroSingleSample(&gyrodata);
            
    if (gyrodata.z > thr || gyrodata.z < thl)  // filter noise       
        gyro_z += (gyrodata.z / 10); 		   // insert new value, make it smaller (/10)
  
    
    AdcSampleWait();
    // CheckBatteryState(adcResult.batt);
    char direction =  GetDirectionFromIR(adcResult.ir1,adcResult.ir2);
    IrLEDs(direction); 						   // evaluate light sensor data
    char LEDsOn = 0;                           // set leds after deviation

    switch (direction) // three cases here: if the IR returns white, gyro is assuring straight movement, otherwise turn
    {
        case 0:           
            if (turn != 0) 					   // if turn was before and straight is now
            {
                firstpos = gyro_z;    		   // set new forward direction
                 
				turn = 0; 
			break;
            }
            
            if (button_clicked) 			   // on click set new forward direction
                firstpos = gyro_z;
            
            // recalculate range of power and range of min/max in ang/sec
            unsigned long tmp = abs(gyro_z - firstpos); 
            unsigned long factor = tmp * c;    // determine increase of motor strength depending on angle deviation
      
            if (gyro_z - firstpos > drift1)    // if drift to the right
            {
                if (factor >= (0xFFFF  - normalpower)) // if calculated motor strength is higher than max, set to max
                {
                    if (LEDsOn == 1) LED_SET1(LED_RED);
                    strength1 = 0xFFFF;
                    strength2 = 0x4E20;
                    
                     if (DEVICE == 1)
                     {
                         strength1 = 0xC350;
                         strength2 = 0x4E20;
                     }
                }
                else 
                {
                    if (LEDsOn == 1) LED_SET1(LED_YELLOW);
                    strength1 = normalpower + factor;
                    strength2 = normalpower - factor ; // if '- factor' movement is not very smooth..maybe -factor/2
                }
                    
            if (LEDsOn == 1) LED_SET2(LED_GREEN);
            }  
            else if ( gyro_z - firstpos < drift2) // if drift to the left
            {
                if (factor >= (0xFFFF - normalpower)) 
                {
                    if (LEDsOn == 1) LED_SET2(LED_RED);
                    strength1 = 0x4E20;
                    strength2 = 0xFFFF;
                    
                    if (DEVICE == 1)
                    {
                        strength1 = 0x4E20;
                        strength2 = 0xC350;
                    }
                }
                else 
                {              
                    if (LEDsOn == 1) LED_SET2(LED_YELLOW);
                    strength1 = normalpower - factor; // if not, the force is too low to move the device in right direction
                    strength2 = normalpower + factor; 
                }  
                if (LEDsOn == 1) LED_SET1(LED_GREEN);
            }
            else
            {
                strength1 = strength2 = normalpower;
                if (LEDsOn == 1) LED_SET(LED_GREEN);
            }
       break;
        
        case 1:  // turn to the right 
            strength1 = 0xEA60;
            strength2 = 0x0; 
            if (DEVICE == 1)
            {
                strength1 = 0x50FF;
                strength2 = 0x20FF;
            }
            if (LEDsOn == 1) 
            {
                LED_SET1(LED_WHITE);
                LED_SET2(LED_BLUE)
            };
            turn = 1;
       break;
       
       case 6:   // stop    
            strength1 = 0;
            strength2 = 0;  
            turn = 1;
            if (LEDsOn == 1) LED_SET(LED_OFF);
       break;
       
       case 7: // turn to the left
            strength1 = 0x0;
            strength2 = 0xEA60;
            
            if (DEVICE == 1)
            {
                strength1 = 0x20FF;
                strength2 = 0x50FF;
            }
            
            if (LEDsOn == 1) 
            {
                LED_SET1(LED_BLUE);
                LED_SET2(LED_WHITE);
            } 
            turn = 2;      
       break;
   }
   
   MOTOR1_DUTY = strength1;
   MOTOR2_DUTY = strength2;
       
   IFS1bits.T4IF = 0;				//Clear TMR4 interruptflag
}

// Main routine
int main(void)
{
    // Initialization
	InitIO();			// I/O pins
	InitI2C();			// Once only
	CLOCK_SOSCEN(); 	// For the RTC
//	WaitForPrecharge();	// ~0.5mA current
	
	// Peripherals - RTC and ADC always used
	CLOCK_INTOSC();     // 8 MHz
//	LED_SET(LED_CYAN);
    RtcStartup();
	AdcInit();
    AdcSampleWait();    // Ensure we have a valid battery level

	SettingsInitialize();           // Initialize settings from ROM

    // Check if we have an accelerometer
    AccelVerifyDeviceId();
    AccelStandby();
    AccelStartup(ACCEL_RANGE_4G|ACCEL_RATE_100);
    // Check if we have a gyro
    GyroVerifyDeviceId();
    GyroStandby();
    GyroStartup();
    
    IR_TX_INIT_PWM();
	MOTOR_INIT_PWM();  
	IR_DET_ENABLE();
	
        // Load Timers 2,3,4,5
        InitTimer3();
        InitTimer4();
	    InitTimer5();
        StartTimer3();
        StartTimer4();
        StartTimer5();
	// Run as attached or logging
	if (USB_BUS_SENSE)
	{
    	RunAttached();      // Run attached, returns when detatched
    }
    else
    {// Application goes here, executed when not USB connected
    
		
		RunApp();
    }

	Reset();                // Reset
	
	return 0;
}

