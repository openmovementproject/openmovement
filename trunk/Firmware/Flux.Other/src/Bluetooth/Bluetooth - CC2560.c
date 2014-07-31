#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "Compiler.h"
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "TimeDelay.h"
#include "Bluetooth\Bluetooth.h"
#include "Bluetooth\BTtasks.h"
#include "config.h"
#ifdef USE_T1_FOR_TICK
	#include "Peripherals/Timer1.h"
#elif defined  USE_RTC_FOR_TICK
	#include "Peripherals/Rtc.h" // For timeout
#else
	#warning "Default behaviour"
	#include "Peripherals/Rtc.h" // For timeout
#endif

// External functions
extern void BTClearDataBuffers(void);

signed char btPresent = -1;       // maybe, not yet tried to init it
bt_state_t bluetoothState = BT_OFF;
bt_power_state_t bluetoothPowerState = BT_STANDBY;

// Buffer for get line
#define SERIAL_BUFFER_SIZE 64
unsigned char commEchoBt = FALSE;	
static char lineBuffer[SERIAL_BUFFER_SIZE] = {0};
static int lineBufferLength = 0;

static char device_name_local[32] = BT_DEFAULT_NAME; 	// Inited to default name
static char device_pin_code_local[5] = BT_DEFAULT_LINK_KEY;
static char device_MAC_local[6] = {0};

char* GetBTName(void) {return device_name_local;}
char* GetBTPin(void) {return device_pin_code_local;}
char* GetBTMAC(void) {return device_MAC_local;}

// Code
char BluetoothInit(const char *deviceName, unsigned short deviceId, unsigned long classOfDevice)
{
	unsigned short i;
	unsigned short lastTime = 0xffff;

	// If the btstack is not already initialised
	if (bluetoothState < BT_INITIALISED)
	{
  		#ifdef HAVE_EHCILL
		unsigned short OSCCON_save = OSCCON; 
		// PLL mode significantly speeds up startup times
		if(OSCCONbits.COSC != 0b001) 
		{
			CLOCK_PLL();
			DelayMs(1);
		}
		#endif

		btDeviceName = device_name_local;	// Set pointer to static string (inited on startup)		
		bt_link_key = device_pin_code_local;// Set pointer to static pairing code string

		// Try to recover the nvm name
		#ifdef DEVICE_NAME_NVM
		ReadProgram((DEVICE_NAME_NVM), device_name_local, 32);
		for(i=0;i<32;i++)
		{
			if (device_name_local[i] >= ' ' && device_name_local[i] < 0x7f) ; 	// Valid char
			else if (device_name_local[i] == '\0') break; 						// End of string
			else{i=0;break;	}													//Invalid char}	
		}
		if (i==0 || i==32) 
		{
			memcpy(device_name_local,BT_DEFAULT_NAME,32);						// Invalid entry
			WriteProgramPage_actual((DEVICE_NAME_NVM), device_name_local, 32);	// Write default name
		}
		else ; // Valid name read out of memory
		#endif

		// Try to recover the nvm pin code
		#ifdef DEVICE_PAIRING_CODE
		ReadProgram((DEVICE_PAIRING_CODE), device_pin_code_local, 5);
		for(i=0;i<5;i++)
		{
			if (device_pin_code_local[i] >= '0' && device_pin_code_local[i] <= '9');// Valid char
			else if (device_pin_code_local[i] == '\0') break; 					// End of string
			else{i=0;break;	}													//Invalid char}	
		}
		if (i!=4) 
		{
			memcpy(device_pin_code_local,BT_DEFAULT_LINK_KEY,5);				// Invalid entry
			WriteProgramPage_actual((DEVICE_PAIRING_CODE), device_pin_code_local, 5);// Write default
		}
		else ; // Valid name read out of memory
		#endif

		#ifdef DEVICE_MAC_ADDRESS
		if(deviceId == 0 || deviceId == 0xffff) // Invalid id
		{
			ReadProgram(DEVICE_MAC_ADDRESS,device_MAC_local, 6);
			deviceId = device_MAC_local[4];		// Appropriate byte swaps
			deviceId <<= 8;						// Since bytes are reverse ordered
			deviceId += device_MAC_local[5];
		}
		#endif

		// If we are setting the device name and number
		{
			unsigned short len = strlen(device_name_local); // Length of current name
			
			if (deviceName != NULL)							// Name is being replaced
			{
				len = strlen(deviceName);
				if (len<30){memcpy(device_name_local,deviceName,len+1);} // Copy new name
			}
	
			if ((deviceId != 0xffff) &&(deviceId != 0))		// Id is to be appended to name
			{
				if ((31-len) > 5) // If theres space to append a number to the name
				{
					*(device_name_local+len++) = '0' + (deviceId/10000)%10;
					*(device_name_local+len++) = '0' + (deviceId/1000)%10;
					*(device_name_local+len++) = '0' + (deviceId/100)%10;
					*(device_name_local+len++) = '0' + (deviceId/10)%10;
					*(device_name_local+len++) = '0' + deviceId%10;
					*(device_name_local+len++) = '\0';
				}
			}
			btDeviceName = device_name_local;
		}


//TODO: Set COD - currently ignored

		// Initialise module 
		BTInit();							// Sets up BT stack
		while(BTtasks()<BT_INITIALISED) 	// Load init script
		{
			// Wait upto 5s for inititalisation then fail
			if (lastTime == 0xffff) lastTime = rtcTicksSeconds;
		    if ((rtcTicksSeconds - lastTime) > 6)
		    	{bluetoothState = BT_FAIL;break;}
		}

		// Check it worked
		if (bluetoothState >= BT_INITIALISED) btPresent = TRUE;
		else btPresent = FALSE;

		// If the function was called with a different clock - try and switch it back
		#ifdef HAVE_EHCILL
		if(ReturnWhenSafeToClockSwitch())
		{
			switch ((OSCCON_save&0xf000)){
				case (0x1000): break; 					// Was already in pll mode
				case (0x0000): 							// Was in 8mhz int osc mode
				case (0x7000): CLOCK_INTOSC(); break; 	// Was in 8mhz int osc mode + clkdiv post scale (assume div by 1, 8MHz)
				default		 :{	asm("DISI #0x3FFF");// Unknown clock mode - just switch back
								OSCCON_save |= 0x01; 			// OSWEN enabled
								__builtin_write_OSCCONH( ((unsigned char*)  &OSCCON_save)[1] );
								__builtin_write_OSCCONL( ((unsigned char*)  &OSCCON_save)[0] );
								asm("DISI #0");
								while (OSCCONbits.OSWEN == 1);}	// Wait for switch						
			}
			BluetoothAdaptToClockSwitch();
		}
		else
		{
			bluetoothState = BT_FAIL;
			//btPresent = FALSE; not strictly true - it did init
			CLOCK_INTOSC();
		}
		#endif
	} // If bluetooth is already initialised then this just returns
	return btPresent;
}

void BluetoothOff(void)
{
	// Will clear state and power off, btPresent not affected
	BTOff();
	return;
}

void BluetoothSuspend(void)
{
	//TODO: Figure out to make the device low power but initialised
	if (btPresent == TRUE)
		bluetoothState = BT_INITIALISED;
	return;
}

unsigned char ReturnWhenSafeToClockSwitch(void)
{
	if ((btPresent != TRUE) || (bluetoothState == BT_OFF)) return TRUE;

	#ifdef HAVE_EHCILL
	unsigned short timeout = 3000; // At least 3 secs 
	while(--timeout && uartNeededDuringSleep && btPresent)
	{
		DelayMs(1);
		BluetoothSerialIO();
	}
	return (timeout != 0)?TRUE:FALSE;
	#endif

	return FALSE;
}

void BluetoothAdaptToClockSwitch(void)
{
	// Not recommended for general use, could create a packet curruption is current Tx/Rx is busy
	if (btPresent == TRUE) 
		hal_uart_dma_set_baud(0); // VDMA is responsible for re-setting the baud etc.
}

bt_power_state_t BluetoothGetPowerState(void)
{
	// Early out if not connected or initialised or failed
	if ((btPresent == FALSE)||(bluetoothState == BT_OFF)||(bluetoothState == BT_FAIL)) bluetoothPowerState = BT_OFF;
	else if(bluetoothState == BT_CONNECTION_REQUEST)bluetoothPowerState = BT_ACTIVE;
	else if(uartNeededDuringSleep) 					bluetoothPowerState = BT_IDLE;
	else 											bluetoothPowerState = BT_SLEEP;
	return bluetoothPowerState;
}


unsigned short BluetoothGetState(void)
{
	return bluetoothState;
}

void BluetoothCommClear(void)
{
	BTClearDataBuffers();
}

unsigned char BluetoothSendData(const void* source, unsigned short num)
{
	if (out_data_buffer_queued != NULL)
		return FALSE;
	if ((num == 0) || (source == NULL))
		return TRUE;

	// This will be sent using calls to tasks
	out_data_buffer_queued	= source;
	out_data_buffer_len_queued = num;
	return TRUE;
}

unsigned char BluetoothTxBusy(void)
{
	// Indicate if there is queued data, always assume fifo has outgoing space
	if (out_data_buffer_queued != NULL)
		return TRUE;
	else
		return FALSE;
}

unsigned char BluetoothRxDone(void)
{
	// Not useful, can only say if a char has been received
	return Bluetooth_haschar();
}

// Bluetooth-specific character I/O
void Bluetooth_putchar(unsigned char v)
{
	FifoPush(&bt_out_fifo,&v,1);
	return;
}

int Bluetooth_getchar(void)
{
	int retval = 0;
	if (FifoLength(&bt_in_fifo) > 0) 
	{
		FifoPop(&bt_in_fifo, &retval , 1);
	}
	else
	{
		retval = -1;	
	}
	return retval;
}

char Bluetooth_haschar(void)
{
	char retval = FALSE;
	if (FifoLength(&bt_in_fifo)) 
	{
		retval = TRUE;
	}
	return retval;
}

void Bluetooth_write(const void *buffer, unsigned int len)
{
	unsigned short nowFree, free;

	// Early out if not connected
	if ((btPresent != TRUE) || (bluetoothState != BT_CONNECTED)) return;

	// Look for outgoing fifo space
	free = FifoFree(&bt_out_fifo);

	// If not enough space to send everything
	while (free < len)
	{
		// Try and send something
		if (free !=0)
		{
			FifoPush(&bt_out_fifo, (void*)buffer, free);
			len -= free;
			buffer += len;
			free = 0;
		}

		// Wait for some data to send - if not locked up
		BluetoothSerialWait();

		// Check for tx success
		nowFree = FifoFree(&bt_out_fifo);
		if (nowFree == 0) 	// No progress
			return;
		else free = nowFree;// Some progress

		// Break if connection lost
		if (bluetoothState != BT_CONNECTED) 
			return;
	}

	// Enough space, characters added to fifo
	FifoPush(&bt_out_fifo,(void*)buffer,len);
	return;
}

void BluetoothSerialIO(void)
{
	if ((btPresent == TRUE) && (bluetoothState != BT_OFF))
		BTtasks();
}

void BluetoothSerialWait(void)
{
	// Timeout
	unsigned short timeout = 10000UL;

	if ((btPresent == FALSE) || (bluetoothState != BT_CONNECTED)) return;

	// Call tasks until it sends
	while(timeout--)
	{
		BluetoothSerialIO();
		// Check if there is fifo spaces
		unsigned short free = FifoFree(&bt_out_fifo);
		// Break when no tx queued and fifo space available detected
		if ( !BluetoothTxBusy() && free)
			{break;}
		// Break if connection lost
		if (bluetoothState != BT_CONNECTED) 
			break;
	}
}

const char *Bluetooth_gets(void)
{
    unsigned char i;
    int c;

    for (i = 0; i < BT_IN_BUFFER_SIZE; i++)       // Maximum number of iterations (bails out early if no input)
    {
		c = Bluetooth_getchar();

		if (c <= 0) { break; }
        if (c == 13)                        // CR
        {
            if (commEchoBt && c != 10) { Bluetooth_putchar(c); }
            lineBuffer[lineBufferLength] = '\0';
            lineBufferLength = 0;
            if (commEchoBt) { Bluetooth_putchar(10); }
            return lineBuffer;
        }
        else if (c == 8 || c == 127)        // BS and DEL
        {
            if (lineBufferLength > 0)
            {
                if (commEchoBt && c != 10) { Bluetooth_putchar(8); Bluetooth_putchar(32); Bluetooth_putchar(8); }
                lineBufferLength--;
                lineBuffer[lineBufferLength] = '\0';
            }
        }
        else if (c == 10)                   // LF
        {
            ;   // ignore
        }
        else if (lineBufferLength + 1 < SERIAL_BUFFER_SIZE)
        {
            if (commEchoBt && c != 10) { Bluetooth_putchar(c); }
            lineBuffer[lineBufferLength] = (char)c;
            lineBufferLength++;
            lineBuffer[lineBufferLength] = '\0';
        }
        else
        {
            if (commEchoBt && c != 10) { Bluetooth_putchar('\a'); }
            ;   // Discard (out of buffer)
        }
    }
    return NULL;
}


#endif
