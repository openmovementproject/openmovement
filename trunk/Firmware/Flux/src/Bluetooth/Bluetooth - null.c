// Dummy Bluetooth driver
#include "Compiler.h"
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "Bluetooth/Bluetooth.h"

signed char btPresent = 0;       	// No BT
bt_state_t bluetoothState = BT_OFF;	// Always off
unsigned char commEchoBt = FALSE;	// Not sure if used

// Code
char BluetoothInit(const char *deviceName, unsigned short deviceId, unsigned long classOfDevice){return FALSE;}
void BluetoothOff(void){}
void BluetoothSuspend(void){}
unsigned char ReturnWhenSafeToClockSwitch(void){return TRUE;}
void BluetoothAdaptToClockSwitch(void){}
unsigned char BluetoothCanSleep(void){return TRUE;}
unsigned short BluetoothGetState(void){return BT_OFF;}
unsigned char BluetoothSendData(const void* source, unsigned short num){return TRUE;}
unsigned char BluetoothTxBusy(void){return FALSE;}
unsigned char BluetoothRxDone(void){return FALSE;}
void Bluetooth_putchar(unsigned char v){return;}
int Bluetooth_getchar(void){return -1;}
char Bluetooth_haschar(void){return FALSE;}
void Bluetooth_write(const void *buffer, unsigned int len){}
void BluetoothSerialIO(void){}
void BluetoothSerialWait(void){}
const char *Bluetooth_gets(void){return NULL;}
bt_power_state_t BluetoothGetPowerState(void){return BT_STANDBY;}
void BT_tick_handler(void){}
//EOF

