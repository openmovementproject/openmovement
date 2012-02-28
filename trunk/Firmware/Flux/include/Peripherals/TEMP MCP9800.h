// Device driver for the MCP9800 12 bit I2C temperature sensor 
// KL 20-01-2012

// Device defines
#define MCP9800_RESULT_REG 0x0
#define MCP9800_CONFIG_REG 0x1
#define MCP9800_HYSTERESIS_REG 	0x2
#define MCP9800_LIMIT_REG 		0x3

#define MCP9800_ONE_SHOT				0x80
#define MCP9800_ONE_SHOT_nCOMPLETE		0x80
#define MCP9800_SHUTDOWN				0x01
#define MCP9800_ONE_SHOT_ENABLE			0x01
#define MCP9800_RESOLUTION_9BIT_30MS	0x00
#define MCP9800_RESOLUTION_10BIT_60MS 	0x20
#define MCP9800_RESOLUTION_11BIT_120MS 	0x40
#define MCP9800_RESOLUTION_12BIT_240MS 	0x60
#define MCP9800_FAULT_QUEUE_0			0x00
#define MCP9800_FAULT_QUEUE_2			0x08
#define MCP9800_FAULT_QUEUE_4			0x10
#define MCP9800_FAULT_QUEUE_5			0x18
#define MCP9800_ALERT_POL_HIGH			0x04
#define MCP9800_ALERT_POL_LOW			0x00
#define MCP9800_COMPARE_MODE			0x00
#define MCP9800_INTERRUPT_MODE			0x02

/*Default setting, note - 12bit mode uses alot of power at 1Hz, suggest 0.1Hz max for 12bit setting!*/
#define MCP9800_DEFAULT	(MCP9800_ONE_SHOT_ENABLE|MCP9800_RESOLUTION_10BIT_60MS) //12uA at 1Hz

// Prototypes

// Initialise the sensor
void MCP9800Init(unsigned char setup);
// Shut down the sensor
void MCP9800Off(void);
// Start a conversion
void MCP9800Sample(void);
// See if the current conversion has comlete - suggest just waiting for low power
unsigned char MCP9800CheckComplete(void);
// Read the latest conversion - assumes the current conversion is over
signed int MCP9800Read(void);
//EOF
