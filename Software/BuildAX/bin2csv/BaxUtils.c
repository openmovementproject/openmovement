/*
	Code written for BAX2.0 tools 
	Code by K Ladha and D Jackson 2011 - 2014
*/
#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>
    #include <io.h>
#else
	#include <string.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/timeb.h>
#include <stdint.h>

#include "Config.h"
#include "BaxRx.h"
#include "BaxUtils.h"
#include "aes.h"

// Debug setting
#undef DEBUG_LEVEL
#define DEBUG_LEVEL	2
#define DBG_FILE dbg_file
#if (DEBUG_LEVEL > 0)||(GLOBAL_DEBUG_LEVEL > 0)
static const char* dbg_file = "util.c";
#endif
#include "Debug.h"

/*
	Helpful functions
*/

/*
	Comm/serial port operations
*/
#define SLIP_END     0xC0                   // End of packet indicator
#define SLIP_ESC     0xDB                   // Escape character, next character will be a substitution
#define SLIP_ESC_END 0xDC                   // Escaped sustitution for the END data byte
#define SLIP_ESC_ESC 0xDD                   // Escaped sustitution for the ESC data byte

#ifdef SERIAL_READ_BUFFER_SIZE
const char* comm_gets(Settings_t* settings) 
{
	int i, input;
	unsigned char value;
	static unsigned int index = 0;
	static char buffer[SERIAL_READ_BUFFER_SIZE];

	// Checks
	if(settings->inGetc == NULL) return NULL;

	// For upto a full buffers worth
	for(i=0;i<SERIAL_READ_BUFFER_SIZE;i++)
	{
		// Get a char
		input = settings->inGetc(&gSettings);

		// Check there was one
		if(input == -1) return NULL;

		// Convert to unsigned char
		value = (unsigned char)input;

		// Add to line
		buffer[index] = value;	
		index++;
		
		// Check overrun - lose whole line
		if(index >= (short)SERIAL_READ_BUFFER_SIZE)
			index = 0;

		// Check for end 
		if(settings->encoding == 'S')
		{
			if(value == (unsigned char)SLIP_END)
			{
				if(index == 1) 	// Lone slip end (sync)
				{
					index = 0;
					continue;
				}
				index = 0;		// Restart at begining
				return buffer;
			}
		}
		else if (settings->encoding == 'H')
		{
			if(value == (unsigned char)'\r')
			{
				if(index == 1) 	// Lone CR
				{
					index = 0;
					continue;
				}
				index = 0;		// Restart at begining
				return buffer;
			}
			else if (	(value >= '0' && value <= '9') || 
						(value >= 'a' && value <= 'f') ||
						(value >= 'A' && value <= 'F') )
			{
				// Valid char
			}
			else 
			{
				// Invalid char, discard
				index = 0;
				continue;
			}
		}
		else if (settings->encoding == 'R')
		{
			if(index == BINARY_DATA_UNIT_SIZE) 	// Full size segment read
			{
				index = 0;
				return buffer;
			}
		}
		else
		{
			DBG_ERROR("Unknown source format");
		}

	}// For
	return NULL; // Out of buffer, never gets here
}
#endif
/*
	File operations
*/
long FSFileSize (FSFILE* file)
{
	long current, end;
	if(file == NULL) return 0;
	current = FSftell(file);
	FSfseek(file,0,SEEK_END);
	end = FSftell(file);
	FSfseek(file,current,SEEK_SET);
	return end;
}

// Retrieve a line from a file (very inefficiently)
char *FSfgets(char *str, int num, FSFILE *stream)
{
	char *p;
	size_t n;

	if (num <= 0) { return NULL; }
	if (FSfeof(stream)) { str[0] = '\0'; return NULL; }

	for (p = str; !FSfeof(stream); p++, num--)
	{
		// Painfully read one byte at a time
		n = FSfread(p, 1, 1, stream);

		// If no more buffer space, or end of file or CR or LF...
		if (num <= 1 || n == 0 || *p == '\r' || *p == '\n')
		{
			// NULL-terminate and exit
			*p = '\0';
			break;
		}
	}
	return str;
}

// Shim for api cross compatibility to typedef int (*GetByte_t)(Settings_t* settings);
int getcFile(Settings_t* settings)
{
	unsigned char read;
	int result;
	if(settings->inputFile == NULL) return -1;
	result = fread(&read,sizeof(unsigned char),1,settings->inputFile);
	if(result == 1) return (unsigned int)read;
	else 
	{
		// End of file or read error
		gStatus.app_state = ERROR_STATE;
		return -1;
	}
}
// Shim for api cross compatibility to typedef int (*PutByte_t)(Settings_t* settings, unsigned char b);
int putcFile(Settings_t* settings, unsigned char b)
{
	// Input files are read only
	DBG_ERROR("Write invoked on read only input");
	return -1;
}

binUnit_t* FSfgetUnit(binUnit_t* dest, FSFILE *stream)
{
	long offset;

	// Clear variable
	memset(dest,0,sizeof(binUnit_t));
	dest->dataNumber = INVALID_DATA_NUMBER;
	dest->dataTime = DATETIME_INVALID;

	// Checks
	if ((stream == NULL) || (dest == NULL) || FSfeof(stream)) 
	{ 
		return NULL; 
	}

	// Check misalignment
	offset = FSftell(stream) % BINARY_DATA_UNIT_SIZE;

	// Re-align
	if(offset) FSfseek(stream,-offset,SEEK_CUR);

	// Read
	FSfread(dest, 1, sizeof(binUnit_t), stream);

	// Return
	return dest;
}

/*
	Bax unpack functions. Operate on raw bax packet in buffer
*/
unsigned char Unpack8(unsigned char* buffer, int offset)
{
	return (unsigned char)buffer[offset];
}
unsigned short UnpackLE16(unsigned char* buffer, int offset)
{
	unsigned short ret = buffer[offset+1];
	ret = ret<<8 | buffer[offset];
	return ret;
}
uint32_t UnpackLE32(unsigned char* buffer, int offset)
{
	unsigned long ret = buffer[offset+3];
	ret = ret<<8 | buffer[offset+2];
	ret = ret<<8 | buffer[offset+1];
	ret = ret<<8 | buffer[offset+0];
	return ret;
}

void BaxUnpackPkt(unsigned char* buffer, BaxPacket_t* packet)
{
	if(buffer == NULL) return;
	if(packet != NULL)
	{
		packet->address = UnpackLE32(buffer, BAX_FIELD_OS_address);
		packet->rssi = Unpack8(buffer, BAX_FIELD_OS_rssi);
		packet->pktType = Unpack8(buffer, BAX_FIELD_OS_pktType);
		memcpy(packet->data,&buffer[BAX_FIELD_OS_data],BAX_PKT_DATA_LEN);
	}
	return;
}
void BaxRepackPkt(BaxPacket_t* pkt, unsigned char* buffer)
{
	unsigned char* ptr;
	// Copy 22 byte result to unit observing packing
	ptr = buffer;
	memcpy(ptr,&pkt->address,sizeof(uint32_t));	// Address
	ptr += sizeof(uint32_t);
	memcpy(ptr,&pkt->rssi,sizeof(uint8_t));		// rssi
	ptr += sizeof(uint8_t);	
	memcpy(ptr,&pkt->pktType,sizeof(uint8_t));	// pktType
	ptr += sizeof(uint8_t);
	memcpy(ptr,pkt->data,BAX_PKT_DATA_LEN);		// data
	return;
}

void BaxUnpackSensorVals(BaxPacket_t* packet, BaxSensorPacket_t* sensor)
{
	if(packet == NULL) return;
	if(sensor != NULL)
	{
		sensor->pktId = Unpack8(packet->data, BAX_FIELD_OS_pktId);
		sensor->xmitPwrdBm = Unpack8(packet->data, BAX_FIELD_OS_xmitPwrdBm);
		sensor->battmv = UnpackLE16(packet->data, BAX_FIELD_OS_battmv);
		sensor->humidSat = UnpackLE16(packet->data, BAX_FIELD_OS_humidSat);
		sensor->tempCx10 = UnpackLE16(packet->data, BAX_FIELD_OS_tempCx10);
		sensor->lightLux = UnpackLE16(packet->data, BAX_FIELD_OS_lightLux);
		sensor->pirCounts = UnpackLE16(packet->data, BAX_FIELD_OS_pirCounts);
		sensor->pirEnergy = UnpackLE16(packet->data, BAX_FIELD_OS_pirEnergy);
		sensor->swCountStat = UnpackLE16(packet->data, BAX_FIELD_OS_swCountStat);
	}
	return;
}




/*
	RTC DateTime conversion
*/
static const unsigned char maxDaysPerMonth[12+1] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
// Method to obtain the number of milliseconds since the  Unix (1970) epoch
unsigned long long MillisecondsEpoch(void)
{
	struct timeb tp;
	ftime(&tp);
	return (unsigned long long)tp.time * 1000 + tp.millitm;
}

uint32_t RtcNow(void)
{
	uint32_t packed;
	unsigned char year, month, day, hours, minutes, seconds;
	struct timeb tp;
	time_t epoc;
	struct tm *time;
	ftime(&tp);
	epoc = tp.time;
	time = localtime(&epoc);
	year = time->tm_year;
	month = time->tm_mon + 1; // Zero reffed month
	day = time->tm_mday;    
	hours = time->tm_hour;
	minutes = time->tm_min;
	seconds = time->tm_sec;
	// Returns standard packed datetime value
	packed = DATETIME_FROM_YMDHMS(year, month, day, hours, minutes, seconds);
	return packed;
}
// Convert a date/time number from a string ("YY/MM/DD,HH:MM:SS+00" -- AT+CCLK compatible for default format)
DateTime RtcFromString(const char *value)
{
    int index;
    unsigned int v;
    unsigned char year = 0, month = 0, day = 0, hours = 0, minutes = 0, seconds = 0;
    const char *c;

	if (value[0] == '\0') { return 0; }
	if (value[0] == '0' && value[1] == '\0') { return 0; }
	if (value[0] == '-') { return 0xffffffff; }
    index = 0;
    v = 0xffff;
    c = value;
    for (;;)
    {
        if (*c >= '0' && *c <= '9') // Part of value
        {
            if (v == 0xffff) { v = 0; }
            v = (v * 10) + (*c - '0');
        }
        else
        {
            if (v != 0xffff)  // End of value
            {
                if      (index == 0) { year = (v >= 2000) ? (v - 2000) : v; }
                else if (index == 1) { month = v; }
                else if (index == 2) { day = v; }
                else if (index == 3) { hours = v; }
                else if (index == 4) { minutes = v; }
                else if (index == 5) { seconds = v; }
                else { break; }
                index++;
                v = 0xffff;
                if (index >= 6) { break; }
            }
            if (*c == '\0') { break; }
        }
        c++;
    }

    // Check if parsed six elements and check validity of members
    if (    index == 6 &&
            (month >= 1 && month <= 12) &&
            (day >= 1 && day <= maxDaysPerMonth[month]) &&
            (hours <= 23) &&
            (minutes <= 59) &&
            (seconds <= 59)
        )
    {
		if (month == 2 && day == 29 && (year & 3) != 0) { month = 3; day = 1; }	// Turn 29-Feb in non-leap years into 1-Mar
        return DATETIME_FROM_YMDHMS(year, month, day, hours, minutes, seconds);
    }
    else
    {
        return 0;
    }
}


// Convert a date/time number to a string ("yyYY/MM/DD,HH:MM:SS+00" -- AT+CCLK compatible for default format)
const char *RtcToString(DateTime value)
{
    // "yyYY/MM/DD,HH:MM:SS+00"
	static char rtcString[21];
    char *c = rtcString;
    unsigned int v;
	if (value < DATETIME_MIN) { *c++ = '0'; *c++ = '\0'; }				// "0"
	else if (value > DATETIME_MAX) { *c++ = '-'; *c++ = '1'; *c++ = '\0'; }	// "-1"
	else
	{
	    v = 2000 + DATETIME_YEAR(value); *c++ = '0' + ((v / 1000) % 10); *c++ = '0' + ((v / 100) % 10); *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '/';
	    v = DATETIME_MONTH(value);       *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '/';
	    v = DATETIME_DAY(value);         *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ',';
	    v = DATETIME_HOURS(value);       *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
	    v = DATETIME_MINUTES(value);     *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
	    v = DATETIME_SECONDS(value);     *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10);
	    //*c++ = '+'; *c++ = '0'; *c++ = '0';
	    *c++ = '\0';
	}
    return rtcString;
}

// Convert a date/time number to a string ("yyYY-MM-DD HH:MM:SS+00" -- for use with SQL time format)
const char *RtcToStringSql(DateTime value)
{
    // "yyYY-MM-DD HH:MM:SS+00"
	static char rtcString[21];
    char *c = rtcString;
    unsigned int v;
	if (value < DATETIME_MIN) { *c++ = '0'; *c++ = '\0'; }				// "0"
	else if (value > DATETIME_MAX) { *c++ = '-'; *c++ = '1'; *c++ = '\0'; }	// "-1"
	else
	{
	    v = 2000 + DATETIME_YEAR(value); *c++ = '0' + ((v / 1000) % 10); *c++ = '0' + ((v / 100) % 10); *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '-';
	    v = DATETIME_MONTH(value);       *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '-';
	    v = DATETIME_DAY(value);         *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ' ';
	    v = DATETIME_HOURS(value);       *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
	    v = DATETIME_MINUTES(value);     *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
	    v = DATETIME_SECONDS(value);     *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10);
	    //*c++ = '+'; *c++ = '0'; *c++ = '0';
	    *c++ = '\0';
	}
    return rtcString;
}

/*
	UDP petitioning
*/
// Security layer - aes
static void MakeKey(unsigned char* buffer, unsigned char* mac, unsigned long sId)
{
	// This function just expands the mac address to make a 16 byte key
	memcpy(&buffer[0],mac,	6); // 0 - 6
	memcpy(&buffer[6],mac,	6); // 7 - 12
	memcpy(&buffer[12],&sId,4); // 14 - 16
}
// How to make the udp petition packet
void UdpMakePetition(unsigned char *buffer, uint32_t time, unsigned char* destMac, const char* un, const char* pw, uint32_t sId)
{
	unsigned char aesKey[16], temp[16];
	int i;
	// Make key - the mac and sId is the base for the encrypt key for the aes 
	MakeKey(aesKey, destMac, sId);
	// Roll back the key						
	for(i=0;i<(PETITION_BLOCK_LEN);i++)
	{
		// Assuming 32 byte username/pw we have 5 blocks / ~750us
		aes_decrypt_128(temp,temp,aesKey,aesKey);
	}
	// Assemble the packet
	memcpy(&buffer[UDP_OS_startToken],"<",sizeof(char));
	memcpy(&buffer[UDP_OS_username],un,USERN_PASS_MAX_LEN);	
	memcpy(&buffer[UDP_OS_password],pw,USERN_PASS_MAX_LEN);	
	memcpy(&buffer[UDP_OS_macAddr],destMac,6);
	memcpy(&buffer[UDP_OS_leaseLen],&time,sizeof(uint32_t));
	memcpy(&buffer[UDP_OS_sessionId],&sId,sizeof(uint32_t));
	memset(&buffer[UDP_OS_padding],0,PETITION_PADDING);
	memcpy(&buffer[UDP_OS_endToken],">",sizeof(char));

	// Now encode the packet (least significant block LAST !!!)
	// First byte of last block, exclude frame bytes
	for(i=((PETITION_BLOCK_LEN-1)*N_BLOCK + 1);;)
	{
		// Assuming 32 byte username/pw we have 5 blocks / 750us
		aes_encrypt_128(&buffer[i],&buffer[i],aesKey,aesKey); // +1 for'<'frame
		if(i > N_BLOCK)i-=N_BLOCK;
		else break;
	}
	// Done	
	return;
}


