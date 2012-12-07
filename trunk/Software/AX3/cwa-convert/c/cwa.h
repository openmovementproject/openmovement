// CWA
// Dan Jackson, 2010

#ifndef _CWA_H_
#define _CWA_H_

#pragma pack(push)
#pragma pack(1)


#define HEADER_UNDEFINED_BLOCK 0xFFFF

#define HEADER_METADATA 0x444D
#define LENGTH_METADATA 0xFFFC

#define HEADER_USAGEBLOCK 0x4255
#define LENGTH_USAGEBLOCK 0xFFFC

#define HEADER_ACCELEROMETER 0x5841
#define LENGTH_ACCELEROMETER 0x01FC

#define HEADER_GYRO 0x5947
#define LENGTH_GYRO 0x01FC

#define HEADER_SESSIONINFO 0x4953
#define LENGTH_SESSIONINFO 0x01FC


typedef struct Sample_t
{
    union
    {
        struct
        {
            short accelX;
            short accelY;
            short accelZ;
        };
        short accel[3];
    };
} Sample;

// 512-byte data packet
typedef struct DataPacket_t
{
	unsigned short packetHeader;	// [2] = 0x5841 (ASCII "AX", little-endian)
	unsigned short packetLength;	// [2] = 508 bytes (contents of this packet is 508 bytes long, + 2 + 2 = 512 bytes total)
	unsigned short deviceId;		// [2] Top bit set: 15-bit fraction of a second for the time stamp, the timestampOffset was already adjusted to minimize this assuming ideal sample rate; Top bit clear: 15-bit device identifier, 0 = unknown; 
    unsigned int sessionId;			// [4] (32-bit unique session identifier, 0 = unknown)
    unsigned int sequenceId;		// [4] (32-bit sequence counter, each packet has a new number -- reset if restarted?)
    unsigned int timestamp;			// [4] (last reported RTC value, 0 = unknown)
	unsigned short light;			// [2] (last recorded light sensor value in raw units, 0 = none)
	unsigned short temperature;		// [2] (last recorded temperature sensor value in raw units)
	unsigned char  events;			// [1] (event flags since last packet, b0 = resume logging from standby, b1 = single-tap event, b2 = double-tap event, b3-b7 = reserved)
	unsigned char  battery;			// [1] (last recorded battery level in 6/256V, 0 = unknown)
	unsigned char  sampleRate;		// <was reserved> [1] = sample rate code (3200/(1<<(15-(rate & 0x0f)))) Hz, if 0, then old format where sample rate stored in 'timestampOffset' field as whole number of Hz
	unsigned char  numAxesBPS;		// [1] = 0x32 (top nibble: number of axes = 3; bottom nibble: number of bytes per axis sample - 2 = 3x 16-bit signed, 0 = 3x 10-bit signed + 2-bit exponent)
	signed short   timestampOffset;	// <was sampleRate> [2] = [if sampleRate is non-zero:] Relative sample index from the start of the buffer where the whole-second timestamp is valid [otherwise, if sampleRate is zero, this is the old format with the sample rate in Hz]
	unsigned short sampleCount;		// [2] = 80 samples (number of accelerometer samples)
	Sample sampleData[80];	// [sampleCount * numAxes * bytesPerSample = 480] (sample data)
	//unsigned char  reservedData[0];	// [packetLength - 28 - (sampleCount * numAxes * bytesPerSample) = 0] (reserved data area)
	unsigned short checksum;		// [2] = Checksum of packet (0 = either checksum is zero or no checksum is recorded)
} DataPacket;


// Event flags
#define DATA_EVENT_NONE                0x00
#define DATA_EVENT_RESUME              0x01
#define DATA_EVENT_SINGLE_TAP          0x02
#define DATA_EVENT_DOUBLE_TAP          0x04
#define DATA_EVENT_EVENT               0x08
#define DATA_EVENT_FIFO_OVERFLOW       0x10
#define DATA_EVENT_BUFFER_OVERFLOW     0x20
#define DATA_EVENT_UNHANDLED_INTERRUPT 0x40
#define DATA_EVENT_CHECKSUM_FAIL       0x80	// Not used internally on CWA


// Buffer parameters
#define BUFFER_SIZE 512		// Must be 512b


// Data status structure
typedef struct
{
    // Header
    unsigned short header;					// @0 0x444D = ("MD") Meta data block
    unsigned short blockSize;				// @2 0xFFFC = Packet size (2^16 - 2 - 2)

	// Stored data
	unsigned char performClear;				// @4
	unsigned short deviceId;				// @5
//	unsigned long sessionId;
    unsigned int sessionId;					// @7
	unsigned short shippingMinLightLevel;	// @11
//	unsigned long loggingStartTime;
//	unsigned long loggingEndTime;
//	unsigned long loggingCapacity;
    unsigned int loggingStartTime;			// @13
    unsigned int loggingEndTime;			// @17
    unsigned int loggingCapacity;			// @21
	unsigned char allowStandby;				// @25 user allowed to transition LOGGING->STANDBY (and if POWERED->STANDBY/LOGGING)
	unsigned char debuggingInfo;			// @26 Additional LED debugging info
	unsigned short batteryMinimumToLog;		// @27 Minimum battery level required for logging
	unsigned short batteryWarning; 			// @29 Battery level below which show warning colour for logging status
											// @31
} DataMeta;

extern DataMeta dataMeta;

typedef struct
{
    // Header
    unsigned short header;              // 0x4255 = ("UB") Usage block data
    unsigned short blockSize;           // 0xFFFC = Packet size (2^16 - 2 - 2)
} DataBlocksAvailable;


// DateTime - our own packed date/time value with creation and extraction methods
// 'DateTime' bit pattern:  YYYYYYMM MMDDDDDh hhhhmmmm mmssssss
typedef unsigned long DateTime;
//#define DATETIME_FROM_YMDHMS(_year, _month, _day, _hours, _minutes, _seconds) ( (((unsigned long)(_year) & 0x3f) << 26) | (((unsigned long)(_month) & 0x0f) << 22) | (((unsigned long)(_day) & 0x1f) << 17) | (((unsigned long)(_hours) & 0x1f) << 12) | (((unsigned long)(_minutes) & 0x3f) <<  6) | ((unsigned long)(_seconds) & 0x3f) )
#define DATETIME_FROM_YMDHMS(_year, _month, _day, _hours, _minutes, _seconds) ( (((unsigned int)(_year) & 0x3f) << 26) | (((unsigned int)(_month) & 0x0f) << 22) | (((unsigned int)(_day) & 0x1f) << 17) | (((unsigned int)(_hours) & 0x1f) << 12) | (((unsigned int)(_minutes) & 0x3f) <<  6) | ((unsigned int)(_seconds) & 0x3f) )
#define DATETIME_YEAR(_v)    ((unsigned char)(((_v) >> 26) & 0x3f))
#define DATETIME_MONTH(_v)   ((unsigned char)(((_v) >> 22) & 0x0f))
#define DATETIME_DAY(_v)     ((unsigned char)(((_v) >> 17) & 0x1f))
#define DATETIME_HOURS(_v)   ((unsigned char)(((_v) >> 12) & 0x1f))
#define DATETIME_MINUTES(_v) ((unsigned char)(((_v) >>  6) & 0x3f))
#define DATETIME_SECONDS(_v) ((unsigned char)(((_v)      ) & 0x3f))


#pragma pack(pop)

#endif // _CWA_H_
