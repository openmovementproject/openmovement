// This file determines the connection behaviour of the bluetooth device
#ifndef _CON_CONFIG_H_
#define _CON_CONFIG_H_

// Definitions
#ifndef DEV_LIST_ENTRY_LEN
#define DEV_LIST_ENTRY_LEN 8
#endif

// Connect settings
const unsigned short gConnectTimeout = 40;	// Duration connect process should run for

// Device list and behaviour - mac addresses (big endian as written)
const unsigned char conDeviceList[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,	0x3F, 	0x11,	/* Test, invalid mac, BR slave */
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,	0x3F, 	0x12,	/* Test, invalid mac, LE slave */
	0x00,0x22,0xd0,0x26,0x18,0xf2,	0x3F,	0x12,	/* Polar band, exact, connect LE slave */
//	0x00,0x17,0xec,0x34,0xa9,0x10,	0x3F,	0x11	/* WAX9 imu, exact, connect LE slave */
};
const unsigned char conDeviceListLen = (sizeof(conDeviceList) / DEV_LIST_ENTRY_LEN);

// LE connection parameters - page 1071 BT4.0 spec, copied into command
// The chosen parameters are default/typical or arbitrary in this example
// Remember this is a little endian binary 'blob', not a struct
const unsigned char conLeCreateConParams[] = {
16,0,		// LE_Scan_Interval(2) - Scan continuously, 10ms/channel
16,0,		// LE_Scan_Window(2)
0,			// Initiator_Filter_Policy(1) - no filter policy, use address
0,			// Peer_Address_Type(1),1=random, over written
0,0,0,0,0,0,// Peer_Address(6), over written
0,			// Own_Address_Type(1),1=random, public address
37,0,		// Conn_Interval_Min(2), 46.25ms
43,0,		// Conn_Interval_Max(2), 53.75ms
0,0,		// Conn_Latency(2), no latency
0xf4,0x01,	// Supervision_Timeout(2) x10ms, 500 = 5sec
2,0,		// Minimum_CE_Length(2) x625us, 2-6 35byte transfers/event
5,0,		// Maximum_CE_Length(2)
};

// BR connection parameters - page 712 BT4.0 spec, copied into command
const unsigned char conBrCreateConParams[] = {
0,0,0,0,0,0,// BDADDR(6), over written
0xFF,0xFF,	// Packet Type(2), LOGICAL AND with supported
0,			// Page_Scan_Repetition_Mode(1), over written
0,			// Reserved(1), must be 0
0,			// Clock_Offset(2), over written
1			// Allow_Role_Switch(1), on, does not affect performance much
};

#endif
