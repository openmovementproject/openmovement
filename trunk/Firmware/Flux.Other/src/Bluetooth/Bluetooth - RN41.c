// Source file for sending a receiving data over bluetooth

#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "TimeDelay.h"
#include "HardwareProfile.h"

#include "Peripherals/Uart_vdma.h"
#include "Bluetooth/Bluetooth.h"
#include "Peripherals/Rtc.h"

// Module commands
const char* bt_cmd_enter_command_mode = "$$$";
const char* bt_cmd_exit_command_mode = "---\r";
const char* bt_cmd_factory_defaults = "SF,1\r";
const char* bt_cmd_set_1mbaud = "SZ,4096\r";
const char* bt_cmd_set_device_name = "SN,%s_%05u\r";
const char* bt_cmd_set_device_serial_name = "S-,%s\r";
//const char* bt_cmd_set_service_name = "SS,RFCOMM Port\r";
//const char* bt_cmd_master_mode = "SM,3\r";
const char* bt_cmd_slave_mode = "SM,0\r";
//const char* bt_cmd_rf_off = "Q\r";
//const char* bt_cmd_rf_on = "W\r";
const char* bt_cmd_set_service_class = "SC,%04x\r";		// Major Service Class
const char* bt_cmd_set_device_class = "SD,%04x\r";		// Major Device Class


static volatile unsigned char bt_tx_active = TRUE;
static volatile unsigned char bt_rx_complete = FALSE;
signed char btPresent = -1;       // ="maybe" (not yet tried to init. it)
bt_state_t bluetoothState = BT_OFF;
bt_power_state_t bluetoothPowerState = BT_STANDBY;
unsigned char commEchoBt = FALSE;

// Privates
void BluetoothTxCompleteHandler(void);
void BluetoothRxCompleteHandler(void);
unsigned char BluetoothCommInit(void);
unsigned char BluetoothSendCommand(const char* cmd, unsigned char doWait);

// Circular buffer stuff
#define BT_IN_BUFFER_CAPACITY 128
#define BT_OUT_BUFFER_CAPACITY 128
#define SERIAL_BUFFER_SIZE 128
unsigned char inHeadBt = 0, inTailBt = 0;
unsigned char outHeadBt = 0, outTailBt = 0;
static unsigned char inBufferBt[BT_IN_BUFFER_CAPACITY];
static unsigned char outBufferBt[BT_OUT_BUFFER_CAPACITY];
static unsigned char PutCircularBufferOverBluetooth(void);
static unsigned char GetBluetoothBytesToCircularBuffer(void);
static char lineBuffer[SERIAL_BUFFER_SIZE] = {0};
static int lineBufferLength = 0;

char BluetoothInit(const char *deviceName, unsigned short deviceId, unsigned long classOfDevice)
{
	char command[32];

	// Serial buffers
    inHeadBt = 0; inTailBt = 0;
    outHeadBt = 0; outTailBt = 0;

	if (!HAS_BLUETOOTH()) {btPresent = FALSE; return 0;}
	//if (deviceName == NULL) { deviceName = "Bluetooth"; };

	bluetoothState = BT_STARTING;

	// Turn it on
	BT_INIT_ON();

	// Setup VDMA 
	UartVdmaInit();
	UartVdmaSetTxCallback(&BluetoothTxCompleteHandler);
	UartVdmaSetRxCallback(&BluetoothRxCompleteHandler);
	UartVdmaSetRxCallBackChar('\r');
	UartVdmaRxFifoInit();

	// Reset module
	BT_RESET_PIN = 0;
	BT_RESET = 0;
	DelayMs(100);
	BT_RESET = 1;
	BT_RESET_PIN = 1;
	DelayMs(500);

	// Enable tx
	bt_tx_active = FALSE;

	// Init comms with module - will set comms to 1M baud
	BluetoothCommInit();
//	UartVdmaRxFifoInit();

	if (btPresent)
	{
		// Do some configuration changes here

		// Use slave mode
		BluetoothSendCommand(bt_cmd_slave_mode,1);
DelayMs(200);

		// Set device name (use auto-generated from MAC if we don't have a valid device ID)
		if (deviceName != NULL)
		{
			if (deviceId == 0xffff)
			{
				sprintf(command, bt_cmd_set_device_serial_name, deviceName);
			}
			else
			{
				sprintf(command, bt_cmd_set_device_name, deviceName, deviceId);
			}	
			BluetoothSendCommand(command,1);
DelayMs(200);
		}

		// Set service and device class
		if (classOfDevice != 0)
		{
			sprintf(command, bt_cmd_set_service_class, (unsigned short)(classOfDevice >> 16));
			BluetoothSendCommand(command, 1);
DelayMs(200);
			sprintf(command, bt_cmd_set_device_class, (unsigned short)(classOfDevice));
			BluetoothSendCommand(command, 1);
DelayMs(200);
		}

		// Exit config mode
		BluetoothSendCommand(bt_cmd_exit_command_mode,1);
DelayMs(200);

		bluetoothState = BT_INITIALISED;
	}
	else
	{
		bluetoothState = BT_OFF;
		BluetoothOff();
	}

	return btPresent;
}

void BluetoothCommClear(void)
{

	UartVdmaRxFifoInit();
}

void BluetoothOff(void)
{
	bluetoothState = BT_OFF;
	BT_INIT_OFF();
	if (!(HAS_BLUETOOTH())) {return;}
	UartVdmaOff();
	return;
}

void BluetoothSuspend(void)
{
	// No supported suspend states
	return;
}

unsigned short BluetoothGetState(void)
{
	if (BT_STAT) 					bluetoothState = BT_CONNECTED;
	else if (btPresent&&BT_POWER)	bluetoothState = BT_INITIALISED;
	else 							bluetoothState = BT_OFF;
	return bluetoothState;
}

void BluetoothTxCompleteHandler(void)
{
	bt_tx_active = FALSE;
	return;
}
unsigned char BluetoothTxBusy(void)
{
	return bt_tx_active;
}

unsigned char ReturnWhenSafeToClockSwitch(void)
{
	// We have to just incomming data to be lost if it collides
	unsigned short timeout = 0xffff;
	while(BluetoothTxBusy() && timeout--);
	return TRUE; 
}

void BluetoothAdaptToClockSwitch(void)
{
	// Zero indicated vdma should adapt to new clock
	UartVdmaSetBaud(0);
}

bt_power_state_t BluetoothGetPowerState(void)
{
	if (BT_STAT) 					bluetoothState = BT_IDLE; // Bluetooth has active connection, expect chars, don't sleep
	else if (btPresent&&BT_POWER)	bluetoothState = BT_IDLE; // If powered and module is present (succefful init)	
	else  							bluetoothState = BT_SLEEP;// Bluetooth is either not powered or not present, can sleep
	return bluetoothState;
}


void BluetoothRxCompleteHandler(void)
{
	bt_rx_complete = TRUE;
	return;
}
unsigned char BluetoothRxDone(void)
{
	unsigned char retVal = bt_rx_complete;
	bt_rx_complete = FALSE;
	return retVal;
}

// Bluetooth-specific put char
void Bluetooth_putchar(unsigned char v)
{
    // If the queue is not full...
    if (((outTailBt + 1) % BT_OUT_BUFFER_CAPACITY) != outHeadBt)
    {
        // Add the character at the tail, and increment the tail position
        outBufferBt[outTailBt] = v;
        outTailBt = ((outTailBt + 1) % BT_OUT_BUFFER_CAPACITY);
    }
}
int Bluetooth_getchar(void)
{
    int v = -1;
    // If the buffer is not empty...
    if (inHeadBt != inTailBt)
    {
        // Remove the head item and increment the head
        v = inBufferBt[inHeadBt];
        inHeadBt = ((inHeadBt + 1) % BT_IN_BUFFER_CAPACITY);
    }
    return v;
}
char Bluetooth_haschar(void)
{
	return (inHeadBt != inTailBt);
}

// write() redirect for Bluetooth
void Bluetooth_write(const void *buffer, unsigned int len)
{
	// LATER: Make this perform a lot faster than one byte at at time
	unsigned int i;
	if (!BT_STAT) { return; }
	if (!(HAS_BLUETOOTH())) {return ;}
    for (i = len; i; --i)
    {
        if (((outTailBt + 1) % BT_OUT_BUFFER_CAPACITY) == outHeadBt)
        {
            BluetoothSerialWait();
            if (((outTailBt + 1) % BT_OUT_BUFFER_CAPACITY) == outHeadBt) { return; }
        }
        Bluetooth_putchar(*(char*)buffer++);
    }
}

// Potentially dangerous synchronous wait for Bluetooth data to complete transmit
void BluetoothSerialWait(void)
{
	// For safety -- only loop while the USB connection state stays the same, and while BT is connected
	int initialUsbBusSense = USB_BUS_SENSE;
    unsigned char oldHeadBt = outHeadBt;
	if (!HAS_BLUETOOTH()) {return ;}
	while ((outHeadBt == oldHeadBt) && (outTailBt != outHeadBt) && BT_STAT && (USB_BUS_SENSE == initialUsbBusSense))    // (outTailBt != outHeadBt)
	{
		BluetoothSerialIO();
	}
}

void BluetoothSerialIO(void)
{
	if (!HAS_BLUETOOTH()) {return ;}
	// Copy received bytes to inBufferBt
	GetBluetoothBytesToCircularBuffer();
    // Send any pending bytes
	PutCircularBufferOverBluetooth();
}

static unsigned char PutCircularBufferOverBluetooth(void)
{
	unsigned char *buffer;
    unsigned char maxOutLength;
	unsigned char numBytes = 0;

    // Transmit USB CDC data
    buffer = outBufferBt + outHeadBt;
    maxOutLength = (outTailBt >= outHeadBt) ? (outTailBt - outHeadBt) : (BT_OUT_BUFFER_CAPACITY - outHeadBt);

    if (maxOutLength > 0)
    {
        if (!BluetoothTxBusy())
        {
			BluetoothSendData(buffer, maxOutLength);
            numBytes = maxOutLength;   // We have to assume that they were all written
            if (numBytes > 0)
            {
                outHeadBt = (outHeadBt + numBytes) % BT_OUT_BUFFER_CAPACITY;
            }
        }
    } 
	return 	numBytes;
}

static unsigned char GetBluetoothBytesToCircularBuffer(void)
{
#if 0
    // Safe and slow
	unsigned char len = UartVdmaSafeFifoLength();
    unsigned char bt_rx_len = 0;
    while (len-- && ((inTailBt + 1) % BT_IN_BUFFER_CAPACITY) != inHeadBt)
    {
        UartVdmaSafeFifoPop(&inBufferBt[inTailBt], 1);
        inTailBt = (inTailBt + 1) % BT_IN_BUFFER_CAPACITY;
        bt_rx_len++;
    }
    return bt_rx_len;
#else
    unsigned char totalFree, maxContiguousFree;
    unsigned char bt_rx_len = 0;
	unsigned char len, i;

	/* Find space in buffer for fresh data*/
	if (inTailBt >= inHeadBt)
		{totalFree = BT_IN_BUFFER_CAPACITY + inHeadBt - inTailBt - 1;}   
	else
		{totalFree = inHeadBt - inTailBt - 1;}

	len = UartVdmaSafeFifoLength();

	if ((totalFree > 0)&&(len)) 	// Is there any room in the buffer and is there any in-data
    {
		// Truncate to fit in buffer
		if (len > totalFree)
		{
			len = totalFree;
		}
		
		// If there is sufficient room for the new data
		if (len <= totalFree)	
		{
			for (i=0;i<2;i++)
			{
				if (inTailBt >= inHeadBt)
					{maxContiguousFree = BT_IN_BUFFER_CAPACITY - inTailBt - ((inHeadBt==0)?1:0);  }   
				else
					{maxContiguousFree = inHeadBt - inTailBt - 1;}

				if (len <= maxContiguousFree)
				{
					UartVdmaSafeFifoPop(&inBufferBt[inTailBt], len);
					inTailBt = (inTailBt + len) % BT_IN_BUFFER_CAPACITY;        // [dgj] You forgot to wrap at the end
					bt_rx_len += len;
					break;
				}
				else
				{	
					UartVdmaSafeFifoPop(&inBufferBt[inTailBt], maxContiguousFree);
					inTailBt = (inTailBt + len) % BT_IN_BUFFER_CAPACITY;        // [dgj] You forgot to wrap at the end
					len -= maxContiguousFree;
					bt_rx_len += maxContiguousFree;			
				}
			}
		}
	}
	/*Return written bytes*/
	return bt_rx_len;
#endif
}

const char *Bluetooth_gets(void)
{
    unsigned char i;
    int c;

    for (i = 0; i < 100; i++)       // Maximum number of iterations (bails out early if no input)
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


unsigned char BluetoothSendData(const void* source, unsigned short num)
{
	if (bt_tx_active) return 0; // Currently busy
	bt_tx_active = TRUE;
	UartVdmaSendBlock((const unsigned char*) source, num);
	return 1;	
}

unsigned char BluetoothSendCommand(const char* cmd, unsigned char doWait)
{
	unsigned short len = strlen(cmd);		
#if 0
static unsigned short debugLength;
static char debugCommand[64];
debugLength = len;
if (len < 64) { strcpy(debugCommand, cmd); } else { memcpy(debugCommand, cmd, 63); debugCommand[63] = '\0'; }
Nop();
#endif	
	if (BluetoothSendData(cmd, len))
	{
		if (doWait)
		{
            unsigned short timeout = 1000;
			while (BluetoothTxBusy() && timeout--) { DelayMs(1); }
		}
		return 1;
	}
	else return 0;
}

unsigned char BluetoothCommInit(void)
{
	// Send init sequence to enter command mode
	int i,j;
	btPresent = FALSE;

	// Try our fast baud first
	UartVdmaSetBaud(0);
	UartVdmaRxFifoInit();	// [dgj] Discard any junk in the FIFO

	for (i=10;i>0;i--)
	{
		BluetoothSendCommand(bt_cmd_enter_command_mode,1);	
		DelayMs(100); 					// Await response
		j = UartVdmaSafeFifoLength();	// If responce found then check it is ok
		if (j)
		{
			char k[10];
			j %= 10;
			UartVdmaSafeFifoPop(&k, j);
			if (strncmp("CMD",k,3)==0) 
			{
				// Set global based on respose
				btPresent = TRUE;
				return btPresent;
			}
		}
	}

	// Try the default baud next
	UartVdmaSetBaud(115200);
	for (i=10;i>0;i--)
	{
UartVdmaRxFifoInit();	// [dgj] Discard any junk in the FIFO
		BluetoothSendCommand(bt_cmd_enter_command_mode,1);	
		DelayMs(100); 					// Await response
		j = UartVdmaSafeFifoLength();	// If responce found then check it is ok
		if (j)
		{
			char k[10];
			j %= 10;
			UartVdmaSafeFifoPop(&k, j);
			if (strncmp("CMD",k,3)==0) 
			{
				// Set global based on respose
				btPresent = TRUE;
				break;
			}
		}
	}

	// Then set the default to our fast rate
	if (btPresent)
	{
		BluetoothSendCommand(bt_cmd_factory_defaults,1);
		DelayMs(10); 
		BluetoothSendCommand(bt_cmd_set_1mbaud,1);
		UartVdmaSetBaud(1000000);
		DelayMs(100);
	}

	return btPresent;	
}

//EOF
