#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

// Types
typedef enum {
		BT_OFF,				/*0 BT Module is not powered*/
		BT_STARTING,		/*1 BT Module is powered but not initialised*/
		BT_FAIL,			/*2 BT Could not start*/
		BT_INITIALISED,		/*3 BT Module is initialised and discoverable*/
		BT_CONNECTED,		/*4 BT Module has an active connection*/
		BT_CONNECTION_REQUEST /*5 A device has tried to initiate a connection (service stack quickly)*/
		}bt_state_t;
typedef enum {
		BT_STANDBY,			/*0 Can sleep, BT Module is not powered*/
		BT_SLEEP,			/*1 Can sleep, Stack will allow sleep*/
		BT_IDLE,			/*2 Can idle, stack is using UART*/
		BT_ACTIVE			/*3 Can not idle, time critical tasks are in progress*/
		}bt_power_state_t;

// State variables
extern signed char btPresent;
extern bt_power_state_t bluetoothPowerState;
extern bt_state_t bluetoothState;
extern bt_power_state_t bluetoothPowerState; // DO NOT USE, Call BluetoothGetPowerState()
extern volatile unsigned char uartNeededDuringSleep;
extern unsigned char commEchoBt;
 
// Prototypes

// Control functions
char BluetoothInit(const char *deviceName, unsigned short deviceId, unsigned long classOfDevice);
void BluetoothOff(void);
void BluetoothSuspend(void);
void BluetoothCommClear(void);
void BluetoothSerialIO(void);
unsigned short BluetoothGetState(void);
bt_power_state_t BluetoothGetPowerState(void);

// Not recommended for constant useage, could create a packet curruption is current Tx/Rx is busy
unsigned char ReturnWhenSafeToClockSwitch(void);
void BluetoothAdaptToClockSwitch(void);


// RFCOMM functions
unsigned char BluetoothSendData(const void* source, unsigned short num);
unsigned char BluetoothTxBusy(void);
unsigned char BluetoothRxDone(void);

// Bluetooth-specific character I/O
void Bluetooth_putchar(unsigned char v);
int  Bluetooth_getchar(void);
char Bluetooth_haschar(void);
void Bluetooth_write(const void *buffer, unsigned int len);
void BluetoothSerialWait(void);
const char *Bluetooth_gets(void);
void BTClearDataBuffers(void);

// Other
extern void BT_tick_handler(void);

// Name, cod, mac and pin - do not alter these if connected
char* GetBTName(void);
char* GetBTPin(void); 
char* GetBTMAC(void);
void SaveBTMAC(void); // Writes ram mac to nvm
unsigned long GetBTCOD(void);

// 24-bit Bluetooth class of device/service
#define BLUETOOTH_COD_CAPTURING_WEARABLE_WRISTWATCH 0x080704ul	// Major Service Class: 0x08 = "Capturing", Major Device Class:  0x07 = "Wearable", Minor Device Class: 0x04 = "Wrist Watch"

#endif
