// Structure of a .CWA binary file.
// This documentation is in the form of 'C' data structures.
// All values are stored in little-endian order (LSB first), and structures are tightly-packed.
// .CWA files start with a single cwa_header_t block and followed by or more cwa_data_t blocks.


#ifndef CWA_FORMAT_H
#define CWA_FORMAT_H


// These definitions use the "stdint" C types: [u]int{8|16|32}_t
#include <stdint.h>


// Timestamps are packed into a 32-bit value: (MSB) YYYYYYMM MMDDDDDh hhhhmmmm mmssssss (LSB)
typedef enum uint32_t cwa_timestamp_t;
#define OM_DATETIME_YEAR(dateTime)    ((unsigned int)((unsigned char)(((dateTime) >> 26) & 0x3f)) + 2000) ///< Extract the year from a packed date/time value.
#define OM_DATETIME_MONTH(dateTime)   ((unsigned char)(((dateTime) >> 22) & 0x0f))  ///< Extract the month (1-12) from a packed date/time value.
#define OM_DATETIME_DAY(dateTime)     ((unsigned char)(((dateTime) >> 17) & 0x1f))  ///< Extract the day (1-31) from a packed date/time value.
#define OM_DATETIME_HOURS(dateTime)   ((unsigned char)(((dateTime) >> 12) & 0x1f))  ///< Extract the hours (0-23) from a packed date/time value.
#define OM_DATETIME_MINUTES(dateTime) ((unsigned char)(((dateTime) >>  6) & 0x3f))  ///< Extract the minutes (0-59) from a packed date/time value.
#define OM_DATETIME_SECONDS(dateTime) ((unsigned char)(((dateTime)      ) & 0x3f))  ///< Extract the seconds (0-59) from a packed date/time value.


// CWA binary file header block.
// Always located at offset 0 in the file, always 1024 bytes in length.
// The odd alignment of elements is inherited from the original 8-bit version.
#pragma pack(push, 1)
typedef struct
{
    uint16_t packetHeader;                      ///< @ 0  +2   ASCII "MD", little-endian (0x444D)
    uint16_t packetLength;                      ///< @ 2  +2   Packet length (1020 bytes, with header (4) = 1024 bytes total)
    uint8_t  hardwareType;                      ///< @ 4  +1 * Hardware type (0x00/0xff/0x17 = AX3, 0x64 = AX6)
    uint16_t deviceId;                          ///< @ 5  +2   Device identifier (lower 16-bits)
    uint32_t sessionId;                         ///< @ 7  +4   Unique session identifier
    uint16_t upperDeviceId;                     ///< @11  +2 * Upper word of device id (if 0xffff is read, treat as 0x0000)
    cwa_timestamp_t loggingStartTime;           ///< @13  +4   Start time for delayed logging
    cwa_timestamp_t loggingEndTime;             ///< @17  +4   Stop time for delayed logging
    uint32_t loggingCapacity;                   ///< @21  +4   (Deprecated: preset maximum number of samples to collect, 0 = unlimited)
    uint8_t  reserved1[1];                      ///< @25  +1   (1 byte reserved)
    uint8_t  flashLed;                          ///< @26  +1   Flash LED during recording
    uint8_t  reserved2[8];                      ///< @27  +8   (8 bytes reserved)
    uint8_t  sensorConfig;                      ///< @35  +1 * Fixed rate sensor configuration, 0x00 or 0xff means accel only, otherwise bottom nibble is gyro range (8000/2^n dps): 2=2000, 3=1000, 4=500, 5=250, 6=125, top nibble non-zero is magnetometer enabled.
    uint8_t  samplingRate;                      ///< @36  +1   Sampling rate code, frequency (3200/(1<<(15-(rate & 0x0f)))) Hz, range (+/-g) (16 >> (rate >> 6)).
    cwa_timestamp_t lastChangeTime;             ///< @37  +4   Last change metadata time
    uint8_t  firmwareRevision;                  ///< @41  +1   Firmware revision number
    int16_t  timeZone;                          ///< @42  +2   (Unused: originally reserved for a "Time Zone offset from UTC in minutes", 0xffff = -1 = unknown)
    uint8_t  reserved3[20];                     ///< @44  +20  (20 bytes reserved)
    uint8_t  annotation[OM_METADATA_SIZE];      ///< @64  +448 Scratch buffer / meta-data (448 ASCII characters, ignore trailing 0x20/0x00/0xff bytes, url-encoded UTF-8 name-value pairs)
    uint8_t  reserved[512];                     ///< @512 +512 Reserved for device-specific meta-data (512 bytes, ASCII characters, ignore trailing 0x20/0x00/0xff bytes, url-encoded UTF-8 name-value pairs, leading '&' if present?)
} cwa_header_t;
#pragma pack(pop)


// CWA binary file data block.
// Always 512 bytes in length at a 512-byte offset, the first at offset 1024 after the header
#pragma pack(push, 1)
typedef struct
{
    uint16_t packetHeader;                      ///< @ 0  +2   ASCII "AX", little-endian (0x5841)
    uint16_t packetLength;                      ///< @ 2  +2   Packet length (508 bytes, with header (4) = 512 bytes total)
    uint16_t deviceFractional;                  ///< @ 4  +2   Top bit set: 15-bit fraction of a second for the time stamp, the timestampOffset was already adjusted to minimize this assuming ideal sample rate; Top bit clear: 15-bit device identifier, 0 = unknown;
    uint32_t sessionId;                         ///< @ 6  +4   Unique session identifier, 0 = unknown
    uint32_t sequenceId;                        ///< @10  +4   Sequence counter (0-indexed), each packet has a new number (reset if restarted)
    cwa_timestamp_t timestamp;                  ///< @14  +4   Last reported RTC value, 0 = unknown
    uint16_t lightScale;                        ///< @18  +2   AAAGGGLLLLLLLLLL Bottom 10 bits is last recorded light sensor value in raw units, 0 = none; top three bits are unpacked accel scale (1/2^(8+n) g); next three bits are gyro scale	(8000/2^n dps)
    uint16_t temperature;                       ///< @20  +2   Last recorded temperature sensor value in raw units, 0 = none
    uint8_t  events;                            ///< @22  +1   Event flags since last packet, b0 = resume logging, b1 = reserved for single-tap event, b2 = reserved for double-tap event, b3 = reserved, b4 = reserved for diagnostic hardware buffer, b5 = reserved for diagnostic software buffer, b6 = reserved for diagnostic internal flag, b7 = reserved)
    uint8_t  battery;                           ///< @23  +1   Last recorded battery level in scaled/cropped raw units (double and add 512 for 10-bit ADC value), 0 = unknown
    uint8_t  sampleRate;                        ///< @24  +1   Sample rate code, frequency (3200/(1<<(15-(rate & 0x0f)))) Hz, range (+/-g) (16 >> (rate >> 6)).
    uint8_t  numAxesBPS;                        ///< @25  +1   0x32 (top nibble: number of axes, 3=Axyz, 6=Gxyz/Axyz, 9=Gxyz/Axyz/Mxyz; bottom nibble: packing format - 2 = 3x 16-bit signed, 0 = 3x 10-bit signed + 2-bit exponent)
    int16_t  timestampOffset;                   ///< @26  +2   Relative sample index from the start of the buffer where the whole-second timestamp is valid
    uint16_t sampleCount;                       ///< @28  +2   Number of sensor samples (if this sector is full -- Axyz: 80 or 120 samples, Gxyz/Axyz: 40 samples)
    uint8_t  rawSampleData[480];                ///< @30  +480 Raw sample data.  Each sample is either 3x/6x/9x 16-bit signed values (x, y, z) or one 32-bit packed value (The bits in bytes [3][2][1][0]: eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx, e = binary exponent, lsb on right)
    uint16_t checksum;                          ///< @510 +2   Checksum of packet (16-bit word-wise sum of the whole packet should be zero)
} OM_READER_DATA_PACKET;
#pragma pack(pop)


// Light is least significant 10 bits, accel scale 3-MSB, gyro scale next 3 bits: 
// Accel scaling for 16-bit unpacked values: 1/2^(8+n) g
// 0: AX3   1/256   (2^8,  8+0)
// 3: +-16g 1/2048  (2^11, 8+3)
// 4: +-8g  1/4096  (2^12, 8+4)
// 5: +-4g  1/8192  (2^13, 8+5)
// 6: +-2g  1/16384 (2^14, 8+6)

// Gyro scaling for 16-bit unpacked values: 4000/2^n dps 
// 1: 2000 dps
// 2: 1000 dps
// 3:  500 dps
// 4:  250 dps
// 5:  125 dps


// Packed accelerometer value - must sign-extend each component value and adjust for exponent
//        [byte-3] [byte-2] [byte-1] [byte-0]
//        eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx
//        10987654 32109876 54321098 76543210

// Access the packed i-th 4-byte value in the buffer in an endian-agnostic way:
#define PACKED_VALUE(buffer, i) ((uint32_t)((uint8_t *)buffer)[30 + i * 4] | ((uint32_t)((uint8_t *)buffer)[31 + i * 4] << 8) | ((uint32_t)((uint8_t *)buffer)[32 + i * 4] << 16) | ((uint32_t)((uint8_t *)buffer)[33 + i * 4] << 24))

// Split the x/y/z/ values out, using the supplied exponent:
#define UNPACK_X(value) ((short)( (short)((unsigned short)0xffc0 & (unsigned short)(value <<  6)) >> (6 - ((unsigned char)(value >> 30))) ))
#define UNPACK_Y(value) ((short)( (short)((unsigned short)0xffc0 & (unsigned short)(value >>  4)) >> (6 - ((unsigned char)(value >> 30))) ))
#define UNPACK_Z(value) ((short)( (short)((unsigned short)0xffc0 & (unsigned short)(value >> 14)) >> (6 - ((unsigned char)(value >> 30))) ))


#endif
