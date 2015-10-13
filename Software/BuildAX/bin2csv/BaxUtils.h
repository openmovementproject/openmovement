/*
	Code written for BAX2.0 tools 
	Code by K Ladha and D Jackson 2011 - 2014
*/
#ifndef _UTILS_H_
#define _UTILS_H_
/*
	Helpers / compatibility
*/
#define TRUE	1
#define FALSE	0
#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>

	// Cross-platform compatibility
	#define sleep(_t) Sleep((_t) * 1000)
	#define usleep(_t) Sleep((_t) / 1000)
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#elif defined(__GNUC__)
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#include <stdint.h>
#include <stdio.h>
#include "Config.h"
//#include "BaxRx.h"

/*
	RTC DateTime conversion
*/
// 'DateTime' bit pattern:  YYYYYYMM MMDDDDDh hhhhmmmm mmssssss
typedef uint32_t DateTime;
#define DATETIME_FROM_YMDHMS(_year, _month, _day, _hours, _minutes, _seconds) ( (((uint32_t)(_year % 100) & 0x3f) << 26) | (((uint32_t)(_month) & 0x0f) << 22) | (((uint32_t)(_day) & 0x1f) << 17) | (((uint32_t)(_hours) & 0x1f) << 12) | (((uint32_t)(_minutes) & 0x3f) <<  6) | ((uint32_t)(_seconds) & 0x3f) )
#define DATETIME_YEAR(_v)       ((unsigned char)(((_v) >> 26) & 0x3f))
#define DATETIME_MONTH(_v)      ((unsigned char)(((_v) >> 22) & 0x0f))
#define DATETIME_DAY(_v)        ((unsigned char)(((_v) >> 17) & 0x1f))
#define DATETIME_HOURS(_v)      ((unsigned char)(((_v) >> 12) & 0x1f))
#define DATETIME_MINUTES(_v)    ((unsigned char)(((_v) >>  6) & 0x3f))
#define DATETIME_SECONDS(_v)    ((unsigned char)(((_v)      ) & 0x3f))

// Valid range
#define DATETIME_MIN DATETIME_FROM_YMDHMS(2000,1,1,0,0,0)
#define DATETIME_MAX DATETIME_FROM_YMDHMS(2063,12,31,23,59,59)
#define DATETIME_INVALID 0xffffffff

// Todo
// Convert a date/time number from a string ("YY/MM/DD,HH:MM:SS+00" -- AT+CCLK compatible for default format)
DateTime RtcFromString(const char *value);
// Convert a date/time number to a string ("yyYY/MM/DD,HH:MM:SS+00" -- AT+CCLK compatible for default format)
const char *RtcToString(DateTime value);
// Convert a date/time number to a string ("yyYY-MM-DD HH:MM:SS+00" -- for SQL time format)
const char *RtcToStringSql(DateTime value);
// Unused
uint32_t RtcNow(void);

/*
	Comm port operations
*/
const char *comm_gets(Settings_t* settings);

/*
	File operations
*/
#define FSFILE		FILE
#define FSfeof		feof
#define FSfwrite	fwrite
#define FSfprintf	fprintf
#define FSfread		fread
#define FSfseek		fseek
#define FSftell		ftell
#define FSfopen		fopen
#define FSfclose	fclose
#define FSfflush	fflush

extern long FSFileSize (FSFILE* file);

// Binary file settings - written in units
#define INVALID_DATA_NUMBER		0xFFFFFFFF
#define BINARY_DATA_UNIT_SIZE	32
#define BINARY_DATA_SIZE		23
typedef struct {
	unsigned long dataNumber;					// +4
	DateTime dataTime;							// +4
	unsigned char continuation; 				// +1
	unsigned char data[BINARY_DATA_SIZE];		// +23 = 32
}binUnit_t;

// Retrieve a line from a file (very inefficiently)
char *FSfgets(char *str, int num, FSFILE *stream);
// Retrieve a binary unit from a file
binUnit_t* FSfgetUnit(binUnit_t* dest, FSFILE *stream);
// Shim for api cross compatibility to typedef int (*GetByte_t)(Settings_t* settings);
int getcFile(Settings_t* settings);
// Shim for api cross compatibility to typedef int (*PutByte_t)(Settings_t* settings, unsigned char b);
int putcFile(Settings_t* settings, unsigned char b);
/*
	Useful functions
*/
unsigned long long MillisecondsEpoch(void);

/*
	UDP negotiations
*/
// Udp connection petition
#define USERN_PASS_MAX_LEN	32 /*Should be 32 for BAX2 system*/
#define PETITION_PADDING 	(16-((USERN_PASS_MAX_LEN*2)+14)%16)  // to make up block size
#define BAD_PETITION		(0xfffffffful)	
#define INVALID_PETITION	(0ul)
#define PETITION_HASH_LEN	((sizeof(UdpPetition_t))*2+1)	// Includes null
typedef struct {
	unsigned char startToken; 		// Const char token '<' not encrypted 0x3c
	char 		  username[USERN_PASS_MAX_LEN]; // <= 32 chars
	char 		  password[USERN_PASS_MAX_LEN]; // <= 32 chars
	unsigned char macAddr[6];		// Device accepting packet
	unsigned char leaseLen[4];		// LE encoded long time in sec
	unsigned char sessionId[4];		// Used to prevent mimic block
	unsigned char padding[PETITION_PADDING]; // To make integer block len
	unsigned char endToken;			// Const char token '>' not encrypted 0x3e
}UdpPetition_t;
//
#define PETITION_LEN		(1+(USERN_PASS_MAX_LEN*2)+14+PETITION_PADDING+1)
#define PETITION_BLOCK_LEN	(PETITION_LEN/16)
#define UDP_OS_startToken	0
#define UDP_OS_username		(UDP_OS_startToken + 1)
#define UDP_OS_password		(UDP_OS_username + 32)
#define UDP_OS_macAddr		(UDP_OS_password + 32)
#define UDP_OS_leaseLen		(UDP_OS_macAddr + 6)
#define UDP_OS_sessionId	(UDP_OS_leaseLen + 4)
#define UDP_OS_padding		(UDP_OS_sessionId + 4)
#define UDP_OS_endToken		(UDP_OS_padding + PETITION_PADDING)
void UdpMakePetition(unsigned char *buffer, uint32_t time, unsigned char* destMac, const char* un, const char* pw, uint32_t sId);

/*
	BAX packet deconstruction
*/
// Packet field offsets
#define BAX_OFFSET_BINARY_UNIT	9
#define BAX_OFFSET_SI44_EVENT	4
#define BAX_FIELD_OS_address	0
#define BAX_FIELD_OS_rssi		4
#define BAX_FIELD_OS_pktType	5
#define BAX_FIELD_OS_data		6
// Sensor field offsets (in pkt.data)
#define BAX_FIELD_OS_pktId		0
#define BAX_FIELD_OS_xmitPwrdBm	1
#define BAX_FIELD_OS_battmv		2
#define BAX_FIELD_OS_humidSat	4
#define BAX_FIELD_OS_tempCx10	6
#define BAX_FIELD_OS_lightLux	8
#define BAX_FIELD_OS_pirCounts	10
#define BAX_FIELD_OS_pirEnergy	12
#define BAX_FIELD_OS_swCountStat 14	

unsigned char Unpack8(unsigned char* buffer, int offset);
unsigned short UnpackLE16(unsigned char* buffer, int offset);
uint32_t UnpackLE32(unsigned char* buffer, int offset);
//void BaxUnpackPkt(unsigned char* buffer, BaxPacket_t* packet);
//void BaxUnpackSensorVals(BaxPacket_t* packet, BaxSensorPacket_t* sensor);

#endif
