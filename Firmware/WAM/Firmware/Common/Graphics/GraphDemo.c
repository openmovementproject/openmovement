// KL 09-06-2011 
// Main.c split up to aid navigation

// Includes
#include <string.h>
#include <Compiler.h>
//#include <p24Fxxxx.h>
#include <TimeDelay.h>
#include <Graphics/Graphics.h>
#include <Graphics/Primitive.h>
#include "GraphicsConfig.h" 
#include "DisplayBasicFont.h"
#include "myGraphics.h"
#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Analog.h"
#include <Rtcc.h>
#include "Peripherals/Rtc.h"


// Variables
extern volatile DWORD tick;
extern volatile DWORD previousTick;
extern volatile BOOL RTCC_UPDATE;
static unsigned short lastTime = 0;

// Prototypes
void RunGraphDemo(void);

// Interrupts
void __attribute__((interrupt, shadow, auto_psv)) _T4Interrupt(void)
{tick++;IFS1bits.T4IF = 0;}

// Code

// Keep the global var updated to the latest val
unsigned int graphNextSample;
void RunGraphDemo(void)
{

char label[] = "Graph";
unsigned char i, displayedValue = 0;
accel_t AccRes;
gyro_t GyRes;

// Inits
LDR_ENABLE(); // on
AdcInit();
AccelStartup(ACCEL_DEFAULT_RATE);
GyroStartup();
AdcSampleWait();AdcSampleWait();   // Update global battery level value

// Mcp graphics library
TickInit(4);// 4mips
GOLInit();
GOLFree();
SetColor(WHITE);
ShowScreenGraph();

while(	(!USB_BUS_SENSE)&&
		(adcResult.batt>BATT_CHARGE_MIN_SAFE))
{

	GOLDraw();

    if (lastTime != rtcTicksSeconds)
    {
        lastTime = rtcTicksSeconds;

		switch (displayedValue)
		{
			case (0)	:	{	// LDR
								sprintf(label,"LDR");
								Display_gotoxy(60,1); 	// Print label at top
								for (i=0;i<4;i++){Display_putc_basic(label[i],0);}
								break;
							}
			case (1)	:	{	// Accel
								sprintf(label,"AccX");
								Display_gotoxy(60,1); 	// Print label at top
								for (i=0;i<4;i++){Display_putc_basic(label[i],0);}
								break;
							}
			case (2)	:	{	// Accel
								sprintf(label,"AccY");
								Display_gotoxy(60,1); 	// Print label at top
								for (i=0;i<4;i++){Display_putc_basic(label[i],0);}
								break;
							}
			case (3)	:	{	// Accel
								sprintf(label,"AccZ");
								Display_gotoxy(60,1); 	// Print label at top
								for (i=0;i<4;i++){Display_putc_basic(label[i],0);}
								break;
							}
			case (4)	:	{	// Gyro
								sprintf(label,"GyrX");
								Display_gotoxy(60,1); 	// Print label at top
								for (i=0;i<4;i++){Display_putc_basic(label[i],0);}
								break;
							}
			case (5)	:	{	// Gyro
								sprintf(label,"GyrY");
								Display_gotoxy(60,1); 	// Print label at top
								for (i=0;i<4;i++){Display_putc_basic(label[i],0);}
								break;
							}
			case (6)	:	{	// Gyro
								sprintf(label,"GyrZ");
								Display_gotoxy(60,1); 	// Print label at top
								for (i=0;i<4;i++){Display_putc_basic(label[i],0);}
								break;
							}
			default		: {}
		}
	}

//	if (ButtonAppLink==1)
//	{
//		displayedValue++;
//		displayedValue%=7;
//		while(ButtonAppLink!=0); // release
//	}
	
	switch (displayedValue)
	{
		case (0)	:	{	// LDR
							// sample, scale, return...
							AdcSampleWait();
							graphNextSample = adcResult.ldr;
							break;
						}
		case (1)	:	{	// Accel
							// sample, scale, return...
							AccelSingleSample(&AccRes); // locals
							graphNextSample = (AccRes.x+512); // offset for sign
							break;
						}
		case (2)	:	{	// Accel
							// sample, scale, return...
							AccelSingleSample(&AccRes); // locals
							graphNextSample = (AccRes.y+512); // offset for sign

						}
		case (3)	:	{	// Accel
							// sample, scale, return...
							AccelSingleSample(&AccRes); // locals
							graphNextSample = (AccRes.z+512); // offset for sign
							break;
						}
		case (4)	:	{	// Gyro
							// sample, scale, return...
							GyroSingleSample(&GyRes); // locals
							graphNextSample = ((GyRes.x+32768)>>6);
							break;
						}
		case (5)	:	{	// Gyro
							// sample, scale, return...
							GyroSingleSample(&GyRes); // locals
							graphNextSample = ((GyRes.y+32768)>>6);
							break;
						}
		case (6)	:	{	// Gyro
							// sample, scale, return...
							GyroSingleSample(&GyRes); // locals
							graphNextSample = ((GyRes.z+32768)>>6);
							break;
						}
						
		default		: graphNextSample=10;
	}
	
	// Scale - 10bit down size
	graphNextSample *= GRAPH_GRID_HEIGHT;
	graphNextSample >>= 10;
	graphNextSample = GRAPH_GRID_HEIGHT - graphNextSample; // invert
	
	// Limit
	if (graphNextSample>GRAPH_GRID_HEIGHT)	graphNextSample = GRAPH_GRID_HEIGHT;
	//if (graphNextSample<0)	graphNextSample = 0;

}

TickInitOff();
return;
}







