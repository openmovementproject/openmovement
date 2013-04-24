// CWA3.1 Karim Ladha 08/12/12
// Includes
#include <Compiler.h>
#include <TimeDelay.h>
#include "stdint.h"
#include "HardwareProfile.h"
#include "Util.h"
#include "Settings.h"
//#include "BTtasks.h"

#include "Peripherals/Accel.h"
#include "Peripherals/Gyro.h"
#include "Peripherals/Rtc.h"
#include "Common/Analog.h"
#include "Peripherals/myi2c.h"

#include "USB/USB.h"
#include "USB/usb_function_cdc.h"
#include "USB/USB_CDC_MSD.h"

// Prototypes
void RunAttached(void);
void TimedTasks(void);
void LedTasks(void);

// Needed for clock switch
extern int hal_uart_dma_set_baud(uint32_t baud);

// Globals
static unsigned char restart = 0;
static unsigned char inactive = 0;
static unsigned short lastTime = 0;

// Attached to USB
void RunAttached(void)
{
    // Enable peripherals
	RtcInterruptOn(0); // Keeps time up to date

    // Initialize sensors - for debugging, not needed really
    AccelStartup(ACCEL_RANGE_4G|ACCEL_RATE_100);
    GyroStartup();
	IR_DET_ENABLE();

	// Clock switch
    CLOCK_PLL();	// HS PLL clock for the USB module 12MIPS
    DelayMs(1); 	// Allow PLL to stabilise

	settings.btEnable = TRUE;

    USBInitializeSystem(); 	// Initializes buffer, USB module SFRs and firmware

    #ifdef USB_INTERRUPT
    USBDeviceAttach();
    #endif

    while(USB_BUS_SENSE && restart != 1)
    {
        // Check bus status and service USB interrupts.
        #ifndef USB_INTERRUPT
        USBDeviceTasks(); 	// Interrupt or polling method.  If using polling, must call
        #endif
        USBProcessIO();
        if ((USBGetDeviceState() >= CONFIGURED_STATE) && (USBIsDeviceSuspended() == FALSE))
        {
            const char *line = _user_gets();
            if (line != NULL)
            {
                status.stream = 0;  // Disable streaming
                SettingsCommand(line, SETTINGS_USB);
            }
        }
        else
        {
            status.attached = -1;
        }
	

	    TimedTasks();
		
        // Stream accelerometer data
        if (status.stream)
		{
			#define STREAM_RATE 10
			#define STREAM_INTERVAL (0x10000UL / STREAM_RATE)
			static unsigned long lastSampleTicks = 0;
           unsigned long now = RtcTicks();
            if (lastSampleTicks == 0) { lastSampleTicks = now; }
            if (now - lastSampleTicks > STREAM_INTERVAL)
           {
                accel_t accelSample;
				gyro_t gyroSample;
                lastSampleTicks += STREAM_INTERVAL;
                if (now - lastSampleTicks > 2 * STREAM_INTERVAL) { lastSampleTicks = now; } // not keeping up with sample rate
                
                AccelSingleSample(&accelSample);
				GyroSingleSample(&gyroSample);
			//	printf("$ACCEL=%d,%d,%d\r\n", accelSample.x, accelSample.y, accelSample.z); 
		    //	printf("%d\n",gyroSample.z);
                
                printf("%i,%i,%i,%i,%i,%i\n", accelSample.y* 10 , accelSample.x* 10, accelSample.z* 10, gyroSample.y * 10, gyroSample.x* 10, gyroSample.z* 10);
                
              //  AdcSampleWait();
               // printf("%d,\r",adcResult.ir1);
               // DelayMs(50);
                //DelayMs(500);
                USBCDCWait();
            }
        }


    }
	#if defined(USB_INTERRUPT)
    USBDeviceDetach();
	#endif
    status.attached = -1;

    return;
}


// Timed tasks
void TimedTasks(void)
{
    if (lastTime != rtcTicksSeconds)
    {
        lastTime = rtcTicksSeconds;

		// WDT
		RtcSwwdtReset();

		// Read ADC and update battery status
        AdcSampleNow();
        if (adcResult.batt > BATT_CHARGE_FULL_USB && status.batteryFull<60) // Add extra minute of full detection to stop it latching prematurely
        {
            status.batteryFull++;
        }
		else
		{
			status.batteryFull = 0;
		}
		
		LedTasks();


    }
    return;
}


// Led status while attached
void LedTasks(void)
{
    if (status.ledOverride >= 0)
    {
        LED_SET(status.ledOverride);
		return;
    }
    else if (status.actionCountdown)
    {
        LED_SET(LED_RED);
    }
    else
    {
        if (inactive == 0)
        {
            if (status.batteryFull) {LED_SET(LED_GREEN);}       // full - flushed
            else                    {LED_SET(LED_YELLOW);}      // charging - flushed
        }
        else					
        {
            LED_SET(LED_RED);     								// unflushed
        }
    }
    return;
}



