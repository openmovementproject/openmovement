/* 
 * Copyright (c) 2011, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */

/*
 * WAX Receiver converter
 * by Daniel Jackson, 2011
 */

/*
    Usage:   waxrec <device> [-log [-tee]] [-osc <hostname>[:<port>] [-timetag]] [-init <string>] [-dump]

    Log example: waxrec <device> -log -tee -init "MODE=1\r\n" > log.csv
    OSC example: waxrec <device> -osc localhost:1234 -init "MODE=1\r\n"

    'device' on Windows "\\.\COM1", Mac "/dev/tty.usbmodem*"
*/


/* Cross-platform alternatives */
#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <winsock.h>
#include <io.h>
#define _POSIX_
typedef int socklen_t;
#define strcasecmp _stricmp
#pragma warning( disable : 4996 )    /* allow deprecated POSIX name functions */
#pragma comment(lib, "wsock32")
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <termios.h>
typedef int SOCKET;
#define SOCKET_ERROR (-1)
#define closesocket close
#define ioctlsocket ioctl
#endif


/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/timeb.h>


/* WAX structures */
#define MAX_SAMPLES 32
typedef struct
{
    unsigned long long timestamp;
    unsigned short sampleIndex;
    short x, y, z;
} WaxSample;
typedef struct
{
    unsigned long long timestamp;
    unsigned short deviceId;
    unsigned char sampleCount;
    WaxSample samples[MAX_SAMPLES];
} WaxPacket;


/* Example serial port device path */
#ifdef _WIN32
#define EXAMPLE_DEVICE "\\\\.\\COM1"
#else
#define EXAMPLE_DEVICE "/dev/tty.usbmodem*"
#endif


/* Debug hex dumps a buffer */
static void hexdump(const void *buffer, size_t length)
{
    unsigned char *buf = (unsigned char *)buffer;
    char w = 16, b, z;
    size_t o;
    for (o = 0; o < length; o += w)
    {
        z = w;
        if (o + z >= length) { z = (char)(length - o); }
        printf("%04x: ", (unsigned int)o);
        for (b = 0; b < w; b++)
        {
            if (b < z) { printf("%02x ", buf[o + b]); }
            else { printf("   "); }
            if ((b & 3) == 3) { printf(" "); }
        }
        printf("  ");
        for (b = 0; b < w; b++)
        {
            if (b < z) { printf("%c", (buf[o + b] < 0x20 || buf[o + b] >= 0x7F) ? '.' : buf[o + b]); }
            else { printf(" "); }
        }
        printf("\r\n");
    }
}


/* Returns a socket error string */
static const char *strerrorsocket(void)
{
#ifdef _WIN32
    static char errorString[256];
    int e;
    e = WSAGetLastError();
    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, e, 0, errorString, 255, NULL))
    {
        return errorString;
    }
    sprintf(errorString, "<unknown #%d 0x%x>", e, e);
    return errorString;
#else
    return strerror(errno);
#endif
}


/* Open a UDP socket*/
static SOCKET opensocket(const char *host, int defaultPort, struct sockaddr_in *serverAddr)
{
    SOCKET s = SOCKET_ERROR;
    char serverName[128] = "localhost"; /* "localhost"; "127.0.0.1"; */
    char *portIndex;
    int serverPort = defaultPort;
    struct hostent *hp;

    /* assigned parameters */
    if (host != NULL && strlen(host) > 0)
    {
        strcpy(serverName, host);
        if ((portIndex = strstr(serverName, ":")) != NULL)
        {
            *portIndex++ = '\0';
            serverPort = atoi(portIndex);
        }
    }

    /* get server host information, name and address */
    hp = gethostbyname(serverName);
    if (hp == NULL)
    {
        serverAddr->sin_addr.s_addr = inet_addr(serverName);
        hp = gethostbyaddr((char *)&serverAddr->sin_addr.s_addr, sizeof(serverAddr->sin_addr.s_addr), AF_INET);
    }
    if (hp == NULL)
    {
        fprintf(stderr, "ERROR: Problem getting host socket information (%s)\n", strerrorsocket());
        return SOCKET_ERROR;
    }
    memcpy(&(serverAddr->sin_addr), hp->h_addr, hp->h_length);
    serverAddr->sin_family = AF_INET; 
    serverAddr->sin_port = htons(serverPort);
    fprintf(stderr, "DEBUG: Server address: [%s] = %s : %d\n", hp->h_name, inet_ntoa(serverAddr->sin_addr), serverPort);
    s = socket(AF_INET, SOCK_DGRAM, 0); 
    if (s < 0) 
    {
        fprintf(stderr, "ERROR: Socket creation failed (%s)\n", strerrorsocket());
        return SOCKET_ERROR;
    }

    return s;
}


/* UDP-transmit a packet */
static size_t transmit(SOCKET s, struct sockaddr_in *serverAddr, const void *sendBuffer, size_t sendLength)
{
    size_t sent = 0;
    if (sendLength > 0)
    {
        fprintf(stderr, "[%d]\n", (unsigned int)sendLength);
        if (sendto(s, (const char *)sendBuffer, sendLength, 0, (struct sockaddr *)serverAddr, sizeof(*serverAddr)) == SOCKET_ERROR)
        {
            fprintf(stderr, "ERROR: Send failed (%s)\n", strerrorsocket());
            return 0;
        }
        sent += (size_t)sendLength;
    }
    return sent;
}


/* Read a line from the device */
static size_t lineread(int fd, void *inBuffer, size_t len)
{
    unsigned char *p = (unsigned char *)inBuffer;
    size_t received = 0;
    unsigned char c;

    if (fd < 0 || inBuffer == NULL) { return 0; }
    *p = '\0';
    for (;;)
    {
        c = '\0';
        if (read(fd, &c, 1) <= 0) { return received; }
        if (c == 0xC0) { return (size_t)-1; }    /* A SLIP_END means the reader should switch to slip reading. */
        if (c == '\r' || c == '\n')
        {
            if (received) { return received; }
        }
        else
        {
            if (received < len - 1) { p[received++] = (char)c; p[received] = 0; }
        }
    }
}


/* Read a SLIP-encoded packet from the device */
static size_t slipread(int fd, void *inBuffer, size_t len)
{
    #define SLIP_END     0xC0                   /* End of packet indicator */
    #define SLIP_ESC     0xDB                   /* Escape character, next character will be a substitution */
    #define SLIP_ESC_END 0xDC                   /* Escaped substitution for the END data byte */
    #define SLIP_ESC_ESC 0xDD                   /* Escaped substitution for the ESC data byte */
    unsigned char *p = (unsigned char *)inBuffer;
    size_t received = 0;
    unsigned char c = '\0';

    if (fd < 0 || inBuffer == NULL) { return 0; }
    for (;;)
    {
        c = '\0';
        if (read(fd, &c, 1) <= 0) { return received; }
        switch (c)
        {
            case SLIP_END:
                if (received) { return received; }
                break;

            case SLIP_ESC:
                c = '\0';
                if (read(fd, &c, 1) <= 0) { return received; }
                switch (c)
                {
                    case SLIP_ESC_END:
                        c = SLIP_END;
                        break;
                    case SLIP_ESC_ESC:
                        c = SLIP_ESC;
                        break;
                    default:
                        fprintf(stderr, "<Unexpected escaped value: %02x>", c);
                        break;
                }
                /* ... fall through to default case with our replaced character ... */

            default:
                if (received < len) { p[received++] = c; }
                break;
        }
    }
}


/* Parse a binary WAX packet */
WaxPacket *parseWaxPacket(const void *inputBuffer, size_t len, unsigned long long now)
{
    const unsigned char *buffer = (const unsigned char *)inputBuffer;
    static WaxPacket waxPacket;

    if (buffer == NULL || len <= 0) { return 0; }

    if (len >= 12 && buffer[0] == 0x12 && buffer[1] == 0x78)
    {
        /*
        typedef struct DataPacket_t
        {
            unsigned char  reportType;        // @0 [1] = 0x12 (USER_REPORT_TYPE)
            unsigned char  reportId;        // @1 [1] = 0x78 (ASCII 'x')
            unsigned short deviceId;        // @2 [2] = Device identifier (16-bit)
            unsigned char  status;            // @4 [1] = Device status (bit 0 is battery warning, top 7 bits reserved)
            unsigned short sample;            // @5 [2] = Analogue sample (top 6 bits indicate measurement information, lower 10 bits are the value)
            unsigned char  format;            // @7 [1] = Accelerometer data format 0xE9 (+/- 2g, 2-bytes, 100Hz); Top two bits is range, next two bits is format [0=3x10+2-bit, 2=signed 16-bit, 3=3x10+2 & 3*16-gyro] (2); lowest four bits is the rate code: frequency = 3200 / (1 << (15-(n & 0x0f)))
            unsigned short sequenceId;        // @8 [2] = Sequence number of first accelerometer reading in this packet (16-bit sample index -- will wrap or be reset if device sleeps and resumes)
            unsigned char  outstanding;        // @10 [1] = Number of samples remaining on device after this packet (0xff: >= 255)
            unsigned char  sampleCount;        // @11 [1] = Number of samples in this packet. 0x0c = 13 samples (number of accelerometer samples)
            //Sample sampleData[BUFFER_PACKET_MAX_SAMPLES];    // @12 bytes: [sampleCount * numAxes * bytesPerSample = 78 -- would be 102 with 17 samples] (sample data)
        } DataPacket;
        */
        unsigned short deviceId = buffer[2] | ((unsigned short)buffer[3] << 8);
        unsigned char status = buffer[4];
        unsigned short adcSample = buffer[5] | ((unsigned short)buffer[6] << 8);
        unsigned char format = buffer[7];
        unsigned short sequenceId = buffer[8] | ((unsigned short)buffer[9] << 8);
        unsigned char outstanding = buffer[10];
        unsigned char sampleCount = buffer[11];
        int bytesPerSample;
        size_t expectedLength;

        /* Format: 0xE0 | AccelCurrentRate() */ 
        /* [1] = Accelerometer data format 0xEA (3-axis, 2-bytes, 100Hz); Top two bits is number of axes (3), next two bits is format [1=unsigned 8-bit,2=signed 16-bit] (2); lowest four bits is the rate code: frequency = 3200 / (1 << (15-(n & 0x0f))) */
        bytesPerSample = 0;
        if (((format >> 4) & 0x03) == 2) { bytesPerSample = 6; }    /* 3*16-bit */
        else if (((format >> 4) & 0x03) == 0) { bytesPerSample = 4; }    /* 3*10-bit + 2 */

        expectedLength = 12 + sampleCount * bytesPerSample;
        if (len < expectedLength) { fprintf(stderr, "WARNING: Ignoring truncated- or unknown-format data packet (received %d expected %d).", (int)len, (int)expectedLength); }
        else
        {
            int i;
            if (len > expectedLength) { fprintf(stderr, "WARNING: Data packet was larger than expected, ignoring additional samples"); }

            waxPacket.timestamp = now;
            waxPacket.deviceId = deviceId;
            waxPacket.sampleCount = sampleCount;

            for (i = 0; i < sampleCount; i++)
            {
                int frequency;
                short millisecondsAgo;
                short x = 0, y = 0, z = 0;

                if (bytesPerSample == 6)
                {
                    x = (short)((unsigned short)(buffer[12 + i * 6] | (((unsigned short)buffer[13 + i * 6]) << 8)));
                    y = (short)((unsigned short)(buffer[14 + i * 6] | (((unsigned short)buffer[15 + i * 6]) << 8)));
                    z = (short)((unsigned short)(buffer[16 + i * 6] | (((unsigned short)buffer[17 + i * 6]) << 8)));
                }
                else if (bytesPerSample == 4)
                {
                    /* Packed accelerometer value
                         [byte-3] [byte-2] [byte-1] [byte-0]
                         eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx
                         10987654 32109876 54321098 76543210
                       Must sign-extend 10-bit value, adjust for exponent
                    */
                    unsigned int value = (unsigned int)buffer[12 + i * 4] | ((unsigned int)buffer[13 + i * 4] << 8) | ((unsigned int)buffer[14 + i * 4] << 16) | ((unsigned int)buffer[15 + i * 4] << 24);

                    x = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value <<  6)) >> (6 - ((unsigned char)(value >> 30))) );
                    y = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value >>  4)) >> (6 - ((unsigned char)(value >> 30))) );
                    z = (short)( (short)((unsigned short)0xffc0 & (unsigned short)(value >> 14)) >> (6 - ((unsigned char)(value >> 30))) );
                }
                frequency = 3200 / ((unsigned short)1 << (15 - (format & 0x0f)));
                millisecondsAgo = !frequency ? 0 : (short)((sampleCount + outstanding - 1 - i) * 1000L / frequency);

                waxPacket.samples[i].timestamp = now - millisecondsAgo;
                waxPacket.samples[i].sampleIndex = sequenceId + i;
                waxPacket.samples[i].x = x;
                waxPacket.samples[i].y = y;
                waxPacket.samples[i].z = z;
            }
            return &waxPacket;
        }
    }
    else if (len >= 12 && buffer[0] == 0x12 && buffer[1] == 0x58)
    {
        fprintf(stderr, "WARNING: Received old WAX packet format -- ignoring.\n");
    }
    else
    {
        fprintf(stderr, "WARNING: Unrecognized packet -- ignoring.\n");
    }
    return NULL;
}


/* Returns the number of milliseconds since the epoch */
unsigned long long TicksNow(void)
{
    struct timeb tp;
    ftime(&tp);
    return (unsigned long long)tp.time * 1000 + tp.millitm;
}


/* Returns a date/time string for the specific number of milliseconds since the epoch */
const char *timestamp(unsigned long long ticks)
{
    static char output[] = "YYYY-MM-DD HH:MM:SS.fff";
    struct tm *today;
    struct timeb tp = {0};
    tp.time = (time_t)(ticks / 1000);
    tp.millitm = (unsigned short)(ticks % 1000);
    tzset();
    today = localtime(&(tp.time));
    sprintf(output, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 1900 + today->tm_year, today->tm_mon + 1, today->tm_mday, today->tm_hour, today->tm_min, today->tm_sec, tp.millitm);
    return output;
}


/* Dumps a WAX packet */
void waxDump(WaxPacket *waxPacket, char tee)
{
    int i;
    for (i = 0; i < waxPacket->sampleCount; i++)
    {
        const char *timeString = timestamp(waxPacket->samples[i].timestamp);
        printf("ACCEL,%s,%u,%u,%f,%f,%f\n", timeString, waxPacket->deviceId, waxPacket->samples[i].sampleIndex, waxPacket->samples[i].x / 256.0f, waxPacket->samples[i].y / 256.0f, waxPacket->samples[i].z / 256.0f);
        if (tee) fprintf(stderr, "ACCEL,%s,%u,%u,%f,%f,%f\n", timeString, waxPacket->deviceId, waxPacket->samples[i].sampleIndex, waxPacket->samples[i].x / 256.0f, waxPacket->samples[i].y / 256.0f, waxPacket->samples[i].z / 256.0f);
    }
    return;
}


/* Write an OSC int (big-endian 32-bit integer) */
int write_osc_int(unsigned char *buffer, signed int value)
{
    buffer[0] = (unsigned char)(value >> 24);
    buffer[1] = (unsigned char)(value >> 16);
    buffer[2] = (unsigned char)(value >>  8);
    buffer[3] = (unsigned char)(value      );
    return 4;
}

/* Write an OSC float (big-endian 32-bit float) */
int write_osc_float(unsigned char *buffer, float value)
{
    return write_osc_int(buffer, *((signed int *)(&value)));
}

/* Write an OSC timetag from a specified millisecond timestamp */
int write_osc_timetag(unsigned char *buffer, unsigned long long timestamp)
{
    unsigned int seconds = (unsigned int)((timestamp / 1000) + 2208988800ul);        /* Convert milliseconds since 1970 to seconds since 1900 */
    unsigned int fractional = (unsigned int)((timestamp % 1000) * 4294967ul);        /* Convert milliseconds to 1/(2^32) fractions of a second */
    int o = 0;
    o += write_osc_int(buffer + o, (signed int)seconds);
    o += write_osc_int(buffer + o, (signed int)fractional);
    return o;
}

/* Writes an OSC string (length prefix, null-padded to multiple of four bytes) */
int write_osc_string(unsigned char *buffer, const char *value)
{
    int len, o = 0;
    len = strlen(value);
    memcpy(buffer + o, value, len);
    o += len;
    buffer[o++] = '\0';
    if ((o & 3) != 0) { buffer[o++] = '\0'; }
    if ((o & 3) != 0) { buffer[o++] = '\0'; }
    if ((o & 3) != 0) { buffer[o++] = '\0'; }
    return o;
}


/* Create an OSC bundle for the WAX data */
size_t waxToOsc(WaxPacket *waxPacket, void *outputBuffer, char timetag)
{
    unsigned char *buffer = (unsigned char *)outputBuffer;
    size_t o = 0;
    int i;
    char address[16];

    sprintf(address, "/wax/%d", waxPacket->deviceId);

    o += write_osc_string(buffer + o, "#bundle");                            /* [OSC string] bundle identifier: "#bundle" <pads to 8 bytes> */
    o += write_osc_timetag(buffer + o, waxPacket->timestamp);                /* [OSC timetag] timestamp <8 bytes> */

    /* OSC messages */
    for (i = 0; i < waxPacket->sampleCount; i++)
    {
        int msgLen = ((strlen(address) >= 8) ? 36 : 32) + (timetag ? 8 : 0);
        o += write_osc_int(buffer + o, msgLen);                                /* [OSC int] message length: 32 or 36 (8 + 8/12 + 4 + 4 + 4 + 4) */
        o += write_osc_string(buffer + o, address);                            /* [OSC string] address: "/wax/#####" <pads to 12 bytes if ID >= 100, or 8 bytes otherwise> */
        o += write_osc_string(buffer + o, timetag ? ",fffit" : ",fffi");    /* [OSC string] type tag: <pads to 8 bytes> */
        o += write_osc_float(buffer + o, waxPacket->samples[i].x / 256.0f);    /* [OSC float] X-axis <4 bytes> */
        o += write_osc_float(buffer + o, waxPacket->samples[i].y / 256.0f);    /* [OSC float] Y-axis <4 bytes> */
        o += write_osc_float(buffer + o, waxPacket->samples[i].z / 256.0f);    /* [OSC float] Z-axis <4 bytes> */
        o += write_osc_int(buffer + o, waxPacket->samples[i].sampleIndex);    /* [OSC int] sample index <4 bytes> */
        if (timetag)
        {
            o += write_osc_timetag(buffer + o, waxPacket->samples[i].timestamp);    /* [OSC timetag] timestamp <8 bytes> */
        }
    }
    return o;
}


/* Open a serial port */
int openport(const char *infile, char writeable)
{
    int fd;
    
    fd = fileno(stdin);
    if (infile != NULL && infile[0] != '\0' && !(infile[0] == '-' && infile[1] == '\0'))
    {
#ifdef _WIN32
        int flags = O_BINARY;
#else
        int flags = O_NOCTTY | O_NDELAY;
#endif
        flags |= (writeable) ? O_RDWR : O_RDONLY;

        fd = open(infile, flags);
        if (fd < 0)
        {
            fprintf(stderr, "ERROR: Problem opening input: %s\n", infile);
            return -1;
        }

        /* Set serial port parameters (OS-specific) */
#ifdef _WIN32
        {
            HANDLE hSerial;
            DCB dcbSerialParams = {0};
            COMMTIMEOUTS timeouts = {0};

            hSerial = (HANDLE)_get_osfhandle(fd);
            if (hSerial == INVALID_HANDLE_VALUE)
            {
                fprintf(stderr, "ERROR: Failed to get HANDLE from file.\n");
            }
            else
            {
                dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
                if (!GetCommState(hSerial, &dcbSerialParams))
                {
                    fprintf(stderr, "ERROR: GetCommState() failed.\n");
                }
                else
                {
                    //dcbSerialParams.BaudRate = CBR_115200;
                    dcbSerialParams.ByteSize = 8;
                    dcbSerialParams.StopBits = ONESTOPBIT;
                    dcbSerialParams.Parity = NOPARITY;
                    if (!SetCommState(hSerial, &dcbSerialParams)){
                        fprintf(stderr, "ERROR: SetCommState() failed.\n");
                    };
                }

                timeouts.ReadIntervalTimeout = 0;
                timeouts.ReadTotalTimeoutConstant = 0;
                timeouts.ReadTotalTimeoutMultiplier = 0;
                timeouts.WriteTotalTimeoutConstant = 0;
                timeouts.WriteTotalTimeoutMultiplier = 0;
                if (!SetCommTimeouts(hSerial, &timeouts))
                {
                    fprintf(stderr, "ERROR: SetCommTimeouts() failed.\n");
                }
            }
        }
#else
        fcntl(fd, F_SETFL, 0);    /* Clear all descriptor flags */
        /* Set the port options */
        {
            struct termios options;
            tcgetattr(fd, &options);
            options.c_cflag = (options.c_cflag | CLOCAL | CREAD | CS8) & ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
            options.c_lflag &= ~(ICANON | ECHO | ISIG); /* Enable data to be processed as raw input */
            tcsetattr(fd, TCSANOW, &options);
        }
#endif

    }
    return fd;
}


/* Parse SLIP-encoded packets, log or convert to UDP packets */
int waxrec(const char *infile, const char *host, const char *initString, char log, char tee, char dump, char timetag, char sendOnly)
{
    #define BUFFER_SIZE 0xffff
    static char buffer[BUFFER_SIZE];
    size_t bufferLength = 0;
    int fd;
    struct sockaddr_in serverAddr;
    SOCKET s = SOCKET_ERROR;

    /* Open the serial port */
    fd = openport(infile, (initString != NULL));
    if (fd < 0) { return 2; }
    
    /* Send initialization string */
    if (initString != NULL)
    {
        const char *p = initString;
        for (;;)
        {
            char c = *p++;
            if (c == '\\' || c == '/')
            {
                c = *p++; 
                if (c == 'r') { c = '\r'; }
                if (c == 'n') { c = '\n'; }
            }
            if (c == '\0') { break; }
            write(fd, &c, 1);
        }
    }

    if (!sendOnly)
    {
#ifdef _WIN32
        {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(1, 1), &wsaData);
        }
#endif

        /* Open UDP socket */
        if (host != NULL && host[0] != '\0')
        {
            s = opensocket(host, 1234, &serverAddr);
            if (s != SOCKET_ERROR)
            {
                fprintf(stderr, "DEBUG: Socket open: %s\n", host);
            }
        }

        /* Read packets and transmit */
        {
            char text = 1;
            for (;;)
            {
                size_t len = 0;

                if (text)
                { 
                    len = lineread(fd, buffer, BUFFER_SIZE); 
                    if (len == (size_t)-1)
                    { 
                        text = 0;
                    } 
                    if (len == 0) { break; } 
                }

                if (!text)
                { 
                    len = slipread(fd, buffer, BUFFER_SIZE);
                    if (len == 0) { break; } 
                }

                if (text)
                {
                    printf("%s\n", buffer);
                    if (tee) { fprintf(stderr, "%s\n", buffer); }
                }

                /* If it appears to be a binary WAX packet... */
                if (len > 0 && buffer[0] == 0x12)
                {
                    WaxPacket *waxPacket;
                    if (dump) { hexdump(buffer, len); }
                    waxPacket = parseWaxPacket(buffer, len, TicksNow());
                    if (waxPacket != NULL)
                    {
                        /* Output text version */
                        if (log) { waxDump(waxPacket, tee); }

                        /* Create an OSC bundle from the WAX packet */
                        len = waxToOsc(waxPacket, buffer, timetag);
                    }
                }

                /* If it appears to be an OSC bundle or OSC packet... */
                if (len >= 1 && (buffer[0] == '#' || buffer[0] == '/'))
                {
                    if (dump) { hexdump(buffer, len); }
                    if (s != SOCKET_ERROR)
                    {
                        size_t tlen;
                        tlen = transmit(s, &serverAddr, buffer, len);
                        if (tlen != len) 
                        { 
                            fprintf(stderr, "WARNING: Problem transmitting: %d / %d\n", (unsigned int)tlen, (unsigned int)len); 
                        }
                    }
                }
            }
        }

        /* Close socket */
        if (s != SOCKET_ERROR) { closesocket(s); }

#ifdef _WIN32
        WSACleanup();
#endif
    }

    /* Close file */
    if (fd != fileno(stdin)) { close(fd); }

    return (s != SOCKET_ERROR) ? 0 : 1;
}


/* Parse command-line parameters */
int main(int argc, char *argv[])
{
    char showHelp = 0;
    int i, argPosition = 0, ret;
    char log = 0, tee = 0, dump = 0, timetag = 0, sendOnly = 0;
    const char *infile = NULL;
    const char *host = NULL;
    const char *initString = NULL;

    fprintf(stderr, "WAXREC    WAX Receiver\n");
    fprintf(stderr, "V1.50     by Daniel Jackson, 2011\n");
    fprintf(stderr, "\n");

    for (i = 1; i < argc; i++)
    {
        if (strcasecmp(argv[i], "-?") == 0 || strcasecmp(argv[i], "-h") == 0 || strcasecmp(argv[i], "--help") == 0
#ifdef _WIN32
            || strcasecmp(argv[i], "/?") == 0 || strcasecmp(argv[i], "/H") == 0 || strcasecmp(argv[i], "/HELP") == 0
#endif
            )
        {
            showHelp = 1;
        }
        else if (strcasecmp(argv[i], "-log") == 0)
        {
            log = 1;
        }
        else if (strcasecmp(argv[i], "-tee") == 0)
        {
            tee = 1;
        }
        else if (strcasecmp(argv[i], "-dump") == 0)
        {
            dump = 1;
        }
        else if (strcasecmp(argv[i], "-timetag") == 0)
        {
            timetag = 1;
        }
        else if (strcasecmp(argv[i], "-exit") == 0)
        {
            sendOnly = 1;
        }
        else if (strcasecmp(argv[i], "-init") == 0)
        {
            initString = argv[++i];
        }
        else if (strcasecmp(argv[i], "-osc") == 0)
        {
            host = argv[++i];
        }
        else if ((argv[i][0] != '-' || argv[i][0] == '\0') && argPosition == 0)
        {
            argPosition++;
            infile = argv[i];
        }
        else
        {
            fprintf(stderr, "ERROR: Unknown parameter: %s\n", argv[i]);
            showHelp = 1;
        }
    }

    if (infile == NULL)
    { 
        fprintf(stderr, "ERROR: Port not specified.\n");
        showHelp = 1; 
    }

    if (showHelp)
    {
        fprintf(stderr, "Usage:   waxrec <device> [-log [-tee]] [-osc <hostname>[:<port>] [-timetag]] [-init <string> [-exit]] [-dump]\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Log example: waxrec %s -log -tee -init \"MODE=1\\r\\n\" > log.csv\n", EXAMPLE_DEVICE);    /* Modes: 1=binary, 2=text, 3=osc */
        fprintf(stderr, "OSC example: waxrec %s -osc localhost:1234 -init \"MODE=1\\r\\n\"\n", EXAMPLE_DEVICE);    /* Modes: 1=binary, 2=text, 3=osc */
        fprintf(stderr, "\n");
        return -1;
    }

    fprintf(stderr, "WAXREC: %s -> %s%s%s%s%s\n", (infile == NULL) ? "<stdin>" : infile, host, (log ? " [log]" : ""), (tee ? " [tee]" : ""), (dump ? " [dump]" : ""), (timetag ? " [timetag]" : ""));
    fprintf(stderr, "INIT: %s\n", initString);

    ret = waxrec(infile, host, initString, log, tee, dump, timetag, sendOnly);

#if defined(_WIN32) && defined(_DEBUG)
    if (IsDebuggerPresent()) { fprintf(stderr, "Press [enter] to exit..."); getc(stdin); }
#endif
    return ret;
}

