/* 
 * Copyright (c) 2011-2012, Newcastle University, UK.
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
 * by Daniel Jackson, 2011-2012
 */

/*
    Usage:   waxrec <device> [-log [-tee]] [-osc <hostname>[:<port>] [-timetag]] [-init <string>] [-dump]

    Log example: waxrec <device> -log -tee -init "MODE=1\r\n" > log.csv
    OSC example: waxrec <device> -osc localhost:1234 -init "MODE=1\r\n"

    'device' on Windows "\\.\COM1", Mac "/dev/tty.usbmodem*"
*/


//#define POLL_RECV     // STOMP poll receive


/* Cross-platform alternatives */
#ifdef _WIN32

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <winsock.h>
#include <io.h>
#define _POSIX_
typedef int socklen_t;
#define strcasecmp _stricmp
#define usleep(_t) Sleep((DWORD)((_t) / 1000))
#pragma warning( disable : 4996 )    /* allow deprecated POSIX name functions */
#pragma comment(lib, "wsock32")

/* Setup API / USB IDs */
#include <setupapi.h>
#include <cfgmgr32.h>
#pragma comment(lib, "setupapi.lib")
#define DEFAULT_VID 0x04D8           /* USB Vendor ID  */
#define DEFAULT_PID 0x000A           /* USB Product ID */

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
#ifdef POLL_RECV
enum { WSAEWOULDBLOCK = EWOULDBLOCK, };
static int WSAGetLastError() { return errno; }
#endif

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
    unsigned short sequenceId;
    unsigned char sampleCount;
    WaxSample samples[MAX_SAMPLES];
} WaxPacket;


/* TEDDI structure */
// Bit-pack 4x 10-bit samples into 5-byte groups (stored little-endian):
// AAAAAAAA BBBBBBAA CCCCBBBB DDCCCCCC DDDDDDDD
// 76543210 54321098 32109876 10987654 98765432

// Number of bytes required to pack 'n' 10-bit values:  size = ((n / 4) * 5); if ((n % 4) != 0) size += (n % 4) + 1;
#define BITPACK10_SIZEOF(_n) ((((_n) / 4) * 5) + ((((_n) & 0x03) == 0) ? 0 : (((_n) & 0x03) + 1)))

// Un-pack 4x 10-bit samples from each 5-byte group (stored little-endian).
static unsigned short BitUnpack_uint10(const void *buffer, unsigned short index)
{
    const unsigned char *p = (const unsigned char *)buffer + ((index >> 2) * 5);
    switch (index & 0x03)
    {
        case 0: return ((unsigned short)p[0]     ) | (((unsigned short)p[1] & 0x0003) << 8);    // A
        case 1: return ((unsigned short)p[1] >> 2) | (((unsigned short)p[2] & 0x000f) << 6);    // B
        case 2: return ((unsigned short)p[2] >> 4) | (((unsigned short)p[3] & 0x003f) << 4);    // C
        case 3: return ((unsigned short)p[3] >> 6) | (((unsigned short)p[4]         ) << 2);    // D
    }
    return 0;
}
#define DATA_INTERVAL 1        // (20 = 5 seconds, max 28)
#define DATA_MAX_INTERVAL 28    
#define DATA_OFFSET 18
/*
// 'V3' TEDDI Data payload -- all WORD/DWORD stored as little-endian (LSB first)
typedef struct
{
    unsigned char  reportType;          // @ 0  [1] USER_REPORT_TYPE (0x12)
    unsigned char  reportId;            // @ 1  [1] Report identifier (0x54, ASCII 'T')
    unsigned short deviceId;            // @ 2  [2] Device identifier (16-bit)
    unsigned char  version;             // @ 4  [1] Low nibble = packet version (0x3), high nibble = config (0x0)
    unsigned char  sampleCount;         // @ 5  [1] Sample count (default config is at 250 msec interval with an equal number of PIR and audio samples; 20 = 5 seconds)
    unsigned short sequence;            // @ 6  [2] Sequence number (16-bit)
    unsigned short unsent;              // @ 8  [2] Number of unsent samples (default config is in 250 msec units)
    unsigned short temp;                // @10  [2] Temperature sum (divide by sampleCount)
    unsigned short light;               // @12  [2] Light sum (divide by sampleCount)
    unsigned short battery;             // @14  [2] Battery sum (divide by sampleCount)
    unsigned short humidity;            // @16  [2] Humidity (0.2 Hz)
    unsigned char  data[BITPACK10_SIZEOF(DATA_MAX_INTERVAL * 2)];   // @18 [50] PIR and audio energy (4 Hz, 20x 2x 10-bit samples)
} TeddiPayload;
*/
typedef struct
{
    unsigned long long timestamp;
    unsigned short deviceId;            // Device identifier (16-bit)
    unsigned char  version;             // Low nibble = packet version (0x3), high nibble = config (0x0)
    unsigned char  sampleCount;         // Sample count (default config is at 250 msec interval with an equal number of PIR and audio samples; 20 = 5 seconds)
    unsigned short sequence;            // Sequence number (16-bit)
    unsigned short unsent;              // Number of unsent samples (default config is in 250 msec units)
    unsigned short temp;                // Temperature sum (divide by sampleCount)
    unsigned short light;               // Light sum (divide by sampleCount)
    unsigned short battery;             // Battery sum (divide by sampleCount)
    unsigned short humidity;            // Humidity (divide by sampleCount)
    unsigned short pirData[DATA_MAX_INTERVAL];     // PIR (4 Hz, 20x 2x 10-bit samples)
    unsigned short audioData[DATA_MAX_INTERVAL];   // Audio energy (4 Hz, 20x 2x 10-bit samples)
} TeddiPacket;


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




/* Open a TCP socket*/
static SOCKET opentcpsocket(const char *host, int defaultPort)
{
  char serverName[128] = "localhost";
  struct sockaddr_in serverAddr;
  int serverPort = defaultPort;
  SOCKET clientSocket = SOCKET_ERROR;
  struct hostent *hp;

  /* Assigned parameters */
  if (host != NULL && strlen(host) > 0)
  {
    char *portIndex = strstr(serverName, ":");
    strcpy(serverName, host);
    if (portIndex != NULL)
	{
      *portIndex++ = '\0';
      serverPort = atoi(portIndex);
    }
  }

  /* Get server host information, name and inet address */
  hp = gethostbyname(serverName);
  if (hp == NULL)
  {
    serverAddr.sin_addr.s_addr = inet_addr(serverName);
    hp = gethostbyaddr((char *)&serverAddr.sin_addr.s_addr, sizeof(serverAddr.sin_addr.s_addr), AF_INET);
  }
  if (hp == NULL)
  {
    fprintf(stderr, "ERROR: Problem getting host socket information (%s)\n", strerrorsocket());
    return SOCKET_ERROR;
  }
  memcpy(&(serverAddr.sin_addr), hp->h_addr, hp->h_length);
  serverAddr.sin_family = AF_INET; 
  serverAddr.sin_port = htons(serverPort);
  fprintf(stderr, "DEBUG: Server address: [%s] = %s : %d\n", hp->h_name, inet_ntoa(serverAddr.sin_addr), serverPort);
  clientSocket = socket(AF_INET, SOCK_STREAM, 0); 
  if (clientSocket < 0)
  {
    fprintf(stderr, "ERROR: Socket creation failed (%s)\n", strerrorsocket());
    return SOCKET_ERROR;
  }

  /* Allow rapid reuse of this socket */
  {
    int reuseoption = 1;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseoption, sizeof(reuseoption)) < 0)
    {
      fprintf(stderr, "WARNING: Setting rapid reuse socket option failed (%s)\n", strerrorsocket());
    }
  }

  /* TCP no-delay on this socket */
  {
    int nodelayoption = 1;
    if (setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelayoption, sizeof(nodelayoption)) < 0)
    {
      fprintf(stderr, "WARNING: Setting no-delay socket option failed (%s)\n", strerrorsocket());
    }
  }

  // Connect
  if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
  {
    fprintf(stderr, "ERROR: Socket connect failed (%s)\n", strerrorsocket());
	closesocket(clientSocket);
    return SOCKET_ERROR;
  }

#ifdef POLL_RECV
  /* Use non-blocking on this socket for polling with waits */
  {
    unsigned long nonblockingoption = 1;
    if (ioctlsocket(clientSocket, FIONBIO, &nonblockingoption) < 0)
    {
      fprintf(stderr, "WARNING: Setting non-blocking socket option failed (%s)\n", strerrorsocket());
    }
  }
#endif

  return clientSocket;
}


/* TCP-transmit some data */
static size_t tcptransmit(SOCKET s, const void *sendBuffer, size_t sendLength)
{
	size_t sent = 0;
	if (sendLength > 0)
	{
		fprintf(stderr, "[%d]\n", sendLength);
		/*fprintf(stderr, "DEBUG: Sending...\n");*/
		/*hexdump(sendBuffer, sendLength);*/
		while (sent < sendLength)
		{
			int ret = send(s, (const char *)sendBuffer + sent, sendLength - sent, 0);
			if (ret < 0)
			{
				fprintf(stderr, "ERROR: Send failed (%s)\n", strerrorsocket());
				return -1;
			}
			sent += ret;
		}
	}
	return sent;
}


/* TCP Poll-Receive */
#ifdef POLL_RECV
static size_t tcpreceive(SOCKET s, char *recvBuffer, size_t recvBufferLength)
{
  size_t received = 0;
  if (pollReceive)
  {
    while (true)
	{
      int ret = recv(clientSocket, (char *)recvBuffer, recvBufferLength, 0);
      if (ret < 0 && WSAGetLastError() == WSAEWOULDBLOCK)
	  {
        usleep(1000);
        continue;
      }
	  else if (ret < 0)
	  {
        fprintf(stderr, "ERROR: Receive failed (%s)\n", strerrorsocket());
        return -1;
      }
      received = ret;

      /* Dump output incrementally */
      fprintf(stderr, "DEBUG: Received from %s:%d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
      ((char *)recvBuffer)[received] = '\0';
      hexdump(recvBuffer, received);
      printf(">>> '%s'\n", recvBuffer);
    }
  }
  ((char *)recvBuffer)[received] = '\0';

  /* Dump output */
  /*
  fprintf(stderr, "DEBUG: Received from %s:%d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
  hexdump(recvBuffer, received);
  */

  return received;
}
#endif



/** STOMP **/

#define STOMP_PORT 61613
#define STOMP_BUFFER_SIZE 0xffff

typedef struct TinyStompTransmitter_t
{
	int _sock;
	unsigned char *_buffer;
	int _bufferPos;
} TinyStompTransmitter;

TinyStompTransmitter *stompTransmitter = NULL;


TinyStompTransmitter *TinyStompTransmitter_New(const char *host, int defaultPort)
{
	TinyStompTransmitter *trans;

	trans = (TinyStompTransmitter *)malloc(sizeof(TinyStompTransmitter));
	trans->_buffer = (unsigned char *)malloc(STOMP_BUFFER_SIZE);
	trans->_bufferPos = 0;
	trans->_sock = SOCKET_ERROR;

	/* Open TCP socket */
	if (host != NULL)
	{
		trans->_sock = opentcpsocket(host, defaultPort);

		if (trans->_sock != SOCKET_ERROR)
		{
			const char *msg = "CONNECT\r\nlogin:\r\npasscode:\r\n\r\n";
			size_t tlen;
			tlen = tcptransmit((SOCKET)trans->_sock, msg, strlen(msg) + 1);
			if (tlen != strlen(msg) + 1) { fprintf(stderr, "WARNING: Problem transmitting CONNECT: %d / %d\n", tlen, strlen(msg) + 1); }
		}
	}

	return trans;
}


void TinyStompTransmitter_Transmit(TinyStompTransmitter *inst, const char *destination, const char *message)
{
	/* Start packet */
	if (inst == NULL) { return; }
	inst->_bufferPos = 0;

	inst->_bufferPos += sprintf((char *)inst->_buffer + inst->_bufferPos, "SEND\r\ndestination:%s\r\n\r\n%s\r\n", destination, message);
	inst->_buffer[inst->_bufferPos++] = '\0';

	if (inst->_sock != SOCKET_ERROR)
	{ 
		size_t tlen = tcptransmit((SOCKET)inst->_sock, inst->_buffer, inst->_bufferPos);
		if (tlen != inst->_bufferPos) { fprintf(stderr, "WARNING: Problem transmitting: %d / %d\n", tlen, inst->_bufferPos); }
	}
	inst->_bufferPos = 0;
}


void TinyStompTransmitter_Delete(TinyStompTransmitter *inst)
{
	if (inst == NULL) { return; }
	if (inst->_buffer != NULL)
	{
		free(inst->_buffer);
		inst->_buffer = NULL;
	}
	inst->_bufferPos = 0;
	if (inst->_sock != SOCKET_ERROR)
	{
		const char *msg = "DISCONNECT\r\n\r\n";
		size_t tlen;
		tlen = tcptransmit((SOCKET)inst->_sock, msg, strlen(msg) + 1);
		if (tlen != strlen(msg) + 1) { fprintf(stderr, "WARNING: Problem transmitting DISCONNECT: %d / %d\n", tlen, strlen(msg) + 1); }
		closesocket(inst->_sock);
		inst->_sock = SOCKET_ERROR;
	}

}

/** **/




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
            waxPacket.sequenceId = sequenceId;
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


/* Parse a binary TEDDI packet */
TeddiPacket *parseTeddiPacket(const void *inputBuffer, size_t len, unsigned long long now)
{
    const unsigned char *buffer = (const unsigned char *)inputBuffer;
    static TeddiPacket teddiPacket;
    int i;

    if (buffer == NULL || len <= 0) { return 0; }

    if (len >= 5 && buffer[0] == 0x12 && buffer[1] == 0x54)
    {
        teddiPacket.deviceId = (unsigned short)(buffer[2] | (((unsigned short)buffer[3]) << 8));
        teddiPacket.version = buffer[4];

        if (((teddiPacket.version & 0x0f) == 0x03 || (teddiPacket.version & 0x0f) == 0x04) && len >= 18)
        {
            /*
            unsigned char  reportType;          // @ 0  [1] USER_REPORT_TYPE (0x12)
            unsigned char  reportId;            // @ 1  [1] Report identifier (0x54, ASCII 'T')
            unsigned short deviceId;            // @ 2  [2] Device identifier (16-bit)
            unsigned char  version;             // @ 4  [1] Low nibble = packet version (0x3), high nibble = config (0x0)
            unsigned char  sampleCount;         // @ 5  [1] Sample count (default config is at 250 msec interval with an equal number of PIR and audio samples; 20 = 5 seconds)
            unsigned short sequence;            // @ 6  [2] Sequence number (16-bit)
            unsigned short unsent;              // @ 8  [2] Number of unsent samples (default config is in 250 msec units)
            unsigned short temp;                // @10  [2] Temperature (0.2 Hz)
            unsigned short light;               // @12  [2] Light (0.2 Hz)
            unsigned short battery;             // @14  [2] Battery (0.2 Hz)
            unsigned short humidity;            // @16  [2] Humidity (0.2 Hz)
            unsigned char  data[BITPACK10_SIZEOF(DATA_MAX_INTERVAL * 2)];   // @18 [50] PIR and audio energy (4 Hz, 20x 2x 10-bit samples)
            */
            unsigned char config = (unsigned char)(buffer[4] >> 4);
            unsigned short sampleInterval = 250;
            teddiPacket.sampleCount = (unsigned char)(buffer[5]);         // Sample count (default config is at 250 msec interval with an equal number of PIR and audio samples; 20 = 5 seconds)
            teddiPacket.sequence = (unsigned short)(buffer[6] | (((unsigned short)buffer[7]) << 8));
            teddiPacket.unsent = (unsigned short)(buffer[8] | (((unsigned short)buffer[9]) << 8));
            teddiPacket.temp = (unsigned short)(buffer[10] | (((unsigned short)buffer[11]) << 8));
            teddiPacket.light = (unsigned short)(buffer[12] | (((unsigned short)buffer[13]) << 8));
            teddiPacket.battery = (unsigned short)(buffer[14] | (((unsigned short)buffer[15]) << 8));

            if ((teddiPacket.version & 0x0f) >= 0x04)
            {
                teddiPacket.humidity = (unsigned short)(buffer[16] | (((unsigned short)buffer[17]) << 8));
            }
            else
            {
                teddiPacket.humidity = 0x00;
            }

            // Unpack PIR
            for (i = 0; i < teddiPacket.sampleCount; i++)
            {
                teddiPacket.pirData[i] = BitUnpack_uint10(buffer + 18, i);
            }

            // Unpack Audio
            for (i = 0; i < teddiPacket.sampleCount; i++)
            {
                teddiPacket.audioData[i] = BitUnpack_uint10(buffer + 18, teddiPacket.sampleCount + i);
            }

            // Divide temp/light/battery measurement down
            if (teddiPacket.sampleCount > 0)
            {
                teddiPacket.temp /= teddiPacket.sampleCount;
                teddiPacket.light /= teddiPacket.sampleCount;
                teddiPacket.battery /= teddiPacket.sampleCount;
                teddiPacket.humidity /= teddiPacket.sampleCount;
            }

// [Each PIR/sound sample]
//   time = timestamp - TimeSpan.FromMilliseconds((unsent + sampleCount - 1 - i) * sampleInterval)
//   seq = (unsigned short)(sequence * sampleCount) + i
// [Overall packet]
//   time = timestamp - TimeSpan.FromMilliseconds((unsent + sampleCount - 1) * sampleInterval
teddiPacket.timestamp = now;

            return &teddiPacket;
        }
        else
        {
            fprintf(stderr, "WARNING: Unrecognized TEDDI packet -- ignoring.\n");
        }
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


/* Dumps a TEDDI packet */
void teddiDump(TeddiPacket *teddiPacket, char tee)
{
    static char line[2048];
    static char number[16];
    int i;

    sprintf(line, "TEDDI,%s,%u,%u,%u,%u,%u,%u,%u,%u,%u", timestamp(teddiPacket->timestamp), teddiPacket->deviceId, teddiPacket->version, 
                                        teddiPacket->sampleCount, teddiPacket->sequence, teddiPacket->unsent, 
                                        teddiPacket->temp, teddiPacket->light, teddiPacket->battery, teddiPacket->humidity);
    for (i = 0; i < teddiPacket->sampleCount; i++)
    {
        sprintf(number, ",%u", teddiPacket->pirData[i]);
        strcat(line, number);
    }
    for (i = 0; i < teddiPacket->sampleCount; i++)
    {
        sprintf(number, ",%u", teddiPacket->audioData[i]);
        strcat(line, number);
    }

	strcat(line, "\n");

    printf(line);
    if (tee) fprintf(stderr, line);
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


/* Find port names for a given USB VID & PID */
int findPorts(unsigned short vid, unsigned short pid, char *buffer, size_t bufferSize)
{
    int count = 0;
    int pos = 0;
    
    if (buffer != NULL)
    {
        buffer[pos] = '\0';
        buffer[pos + 1] = '\0';
    }
    
#ifdef _WIN32
    {
        GUID *pGuids;
        DWORD dwGuids = 0;
        unsigned int guidIndex;
        char prefix[32];
        int index;

        /* PNPDeviceID to search for */
        sprintf(prefix, "USB\\VID_%04X&PID_%04X", vid, pid);

        /* Convert the name "Ports" to a GUID */
        SetupDiClassGuidsFromNameA("Ports", NULL, 0, &dwGuids) ;
        if (dwGuids == 0) { fprintf(stderr, "ERROR: SetupDiClassGuidsFromName() failed.\n"); return -1; }
        pGuids = (GUID *)malloc(dwGuids * sizeof(GUID));
        if (!SetupDiClassGuidsFromNameA("Ports", pGuids, dwGuids, &dwGuids)) { fprintf(stderr, "ERROR: SetupDiClassGuidsFromName() failed.\n"); free(pGuids); return -1; }

        /* For each GUID returned */
        for (guidIndex = 0; guidIndex < dwGuids; guidIndex++)
        {
            HDEVINFO hDevInfo;

            /* From the root of the device tree, look for all devices that match the interface GUID */
            hDevInfo = SetupDiGetClassDevs(&pGuids[guidIndex], NULL, NULL, DIGCF_PRESENT);
            if (hDevInfo == INVALID_HANDLE_VALUE) { free(pGuids); return -1; }

            for (index = 0; ; index++)
            {
                char usbId[MAX_PATH] = "";

                /* Enumerate the current device */
                SP_DEVINFO_DATA devInfo;
                devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
                if (!SetupDiEnumDeviceInfo(hDevInfo, index, &devInfo)) { break; }

                /* Get USB id for device */
                CM_Get_Device_IDA(devInfo.DevInst, usbId, MAX_PATH, 0);

                /* If this is the device we are looking for */
                if (strncmp(usbId, prefix, strlen(prefix)) == 0)
                {
                    HKEY hDeviceKey;

#if 1
                    /* Move up one level to get to the composite device string */
                    char usbComposite[MAX_PATH] = "";
                    DWORD parent = 0;
                    CM_Get_Parent(&parent, devInfo.DevInst, 0);
                    CM_Get_Device_IDA(parent, usbComposite, MAX_PATH, 0);
                    if (strncmp(usbComposite, prefix, strlen(prefix)) != 0)
                    {
                        usbComposite[0] = '\0';     /* If it doesn't match the vid/pid, this is not a composite device */
                    }
#endif

                    /* Registry key for the ports settings */
                    hDeviceKey = SetupDiOpenDevRegKey(hDevInfo, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
                    if (hDeviceKey)
                    {
                        /* Name of the port */
                        char portName[MAX_PATH] = "";
                        DWORD dwSize = sizeof(portName);
                        DWORD dwType = 0;
                        if ((RegQueryValueExA(hDeviceKey, "PortName", NULL, &dwType, (LPBYTE)portName, &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ))
                        {
                            /* TODO: snprintf with (bufferSize - pos) */
                            pos += sprintf(buffer + pos, "\\\\.\\%s\n", portName);
                            *(buffer + ++pos) = '\0';

                            count++;
                        }
                    }

                }
            }
            SetupDiDestroyDeviceInfoList(hDevInfo);
        }
        free(pGuids); 
    }
#else
    /* Not implemented */
#endif
    return count;
}




/* Parse SLIP-encoded packets, log or convert to UDP packets */
int waxrec(const char *infile, const char *host, const char *initString, char log, char tee, char dump, char timetag, char sendOnly, const char *stompHost, const char *stompAddress)
{
    #define BUFFER_SIZE 0xffff
    static char buffer[BUFFER_SIZE];
    size_t bufferLength = 0;
    int fd;
    struct sockaddr_in serverAddr;
    SOCKET s = SOCKET_ERROR;
    static char ports[1024];

    /* Search for port */
    if (infile[0] == '!')
    {
        unsigned long vidpid = (DEFAULT_VID << 16) | DEFAULT_PID;
        sscanf(infile + 1, "%08x", &vidpid);
        fprintf(stderr, "Searching for VID=%04X&PID=%04X...\n", (unsigned short)(vidpid >> 16), (unsigned short)vidpid);
        if (findPorts((unsigned short)(vidpid >> 16), (unsigned short)vidpid, ports, 1024) <= 0)
        {
            fprintf(stderr, "ERROR: No ports found.\n");
            return 3;
        }
        /* Choose the first port found */
        infile = ports;
    }
    
    /* Open the serial port */
    fd = openport(infile, (initString != NULL));
    if (fd < 0)
    {
        fprintf(stderr, "ERROR: Port not open.\n");
        return 2;
    }
    
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

        /* Open the STOMP port */
        if (stompHost[0] != '\0')
        {
            stompTransmitter = TinyStompTransmitter_New(stompHost, STOMP_PORT);
        }

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
                if (len > 1 && buffer[0] == 0x12 && (buffer[1] == 0x78 || buffer[1] == 0x58))
                {
                    WaxPacket *waxPacket;
                    if (dump) { hexdump(buffer, len); }
                    waxPacket = parseWaxPacket(buffer, len, TicksNow());
                    if (waxPacket != NULL)
                    {
                        /* Output text version */
                        if (log) { waxDump(waxPacket, tee); }

                        /* Create a STOMP packet */
	                    if (stompTransmitter != NULL)
	                    {
	                        static char msg[2048];
                            char *p = msg;
                            int z;

                            p += sprintf(p, "{");
                            p += sprintf(p, "\"Timestamp\":\"%lu\",", waxPacket->timestamp);
                            p += sprintf(p, "\"DeviceId\":\"%u\",", waxPacket->deviceId);
                            p += sprintf(p, "\"SequenceId\":\"%u\",", waxPacket->sequenceId);
                            p += sprintf(p, "\"SampleCount\":\"%u\",", waxPacket->sampleCount);

                            p += sprintf(p, "\"Samples\":[");
                            for (z = 0; z < waxPacket->sampleCount; z++)
                            {
                                p += sprintf(p, "[%lu,%u,%d,%d,%d]%s", waxPacket->samples[z].timestamp, waxPacket->samples[z].sampleIndex, waxPacket->samples[z].x, waxPacket->samples[z].y, waxPacket->samples[z].z, z + 1 < waxPacket->sampleCount ? "," : "");
                            }
                            p += sprintf(p, "]");

                            p += sprintf(p, "}");
	                        TinyStompTransmitter_Transmit(stompTransmitter, stompAddress, msg);
                        }

                        /* Create an OSC bundle from the WAX packet */
                        len = waxToOsc(waxPacket, buffer, timetag);
                    }
                }

                /* If it appears to be a binary TEDDI packet (USER_REPORT_TYPE, 'T') */
                if (len > 1 && buffer[0] == 0x12 && buffer[1] == 0x54)
                {
                    TeddiPacket *teddiPacket;
                    if (dump) { hexdump(buffer, len); }
                    teddiPacket = parseTeddiPacket(buffer, len, TicksNow());
                    if (teddiPacket != NULL)
                    {
                        /* Output text version */
                        if (log) { teddiDump(teddiPacket, tee); }

                        /* Create a STOMP packet */
	                    if (stompTransmitter != NULL)
	                    {
	                        static char msg[2048];
                            char *p = msg;
                            int i;
                            
                            p += sprintf(p, "{");
                            p += sprintf(p, "\"Timestamp\":\"%lu\",", teddiPacket->timestamp);
                            p += sprintf(p, "\"DeviceId\":\"%u\",", teddiPacket->deviceId);
                            p += sprintf(p, "\"Version\":\"%u\",", teddiPacket->version);
                            p += sprintf(p, "\"SampleCount\":\"%u\",", teddiPacket->sampleCount);
                            p += sprintf(p, "\"Sequence\":\"%u\",", teddiPacket->sequence);
                            p += sprintf(p, "\"Unsent\":\"%u\",", teddiPacket->unsent);
                            p += sprintf(p, "\"Temp\":\"%u\",", teddiPacket->temp);
                            p += sprintf(p, "\"Light\":\"%u\",", teddiPacket->light);
                            p += sprintf(p, "\"Battery\":\"%u\",", teddiPacket->battery);
                            p += sprintf(p, "\"Humidity\":\"%u\",", teddiPacket->humidity);

                            p += sprintf(p, "\"Pir\":[");
                            for (i = 0; i < teddiPacket->sampleCount; i++)
                            {
                                p += sprintf(p, "%u%s", teddiPacket->pirData[i], i + 1 < teddiPacket->sampleCount ? "," : "");
                            }
                            p += sprintf(p, "],");

                            p += sprintf(p, "\"Audio\":[");
                            for (i = 0; i < teddiPacket->sampleCount; i++)
                            {
                                p += sprintf(p, "%u%s", teddiPacket->audioData[i], i + 1 < teddiPacket->sampleCount ? "," : "");
                            }
                            p += sprintf(p, "]");

                            p += sprintf(p, "}");
	                        TinyStompTransmitter_Transmit(stompTransmitter, stompAddress, msg);
                        }

                        /* Create an OSC bundle from the TEDDI packet */
                        /* len = teddiToOsc(teddiPacket, buffer, timetag); */
                    }
                }
                
                /* If it appears to (now) be an OSC bundle or OSC packet... */
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

        /* Close the STOMP port */
	    if (stompTransmitter != NULL)
	    {
		    TinyStompTransmitter_Delete(stompTransmitter);
		    stompTransmitter = NULL;
	    }

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
    static char stompHost[128] = ""; //"localhost";
    static char stompAddress[128] = "/topic/OpenMovement.Sensor.Wax";

    fprintf(stderr, "WAXREC    WAX Receiver\n");
    fprintf(stderr, "V1.60     by Daniel Jackson, 2011-2012\n");
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
        else if (strcasecmp(argv[i], "-stomphost") == 0)
        {
            strcpy(stompHost, argv[++i]);
        }
        else if (strcasecmp(argv[i], "-stomptopic") == 0)
        {
            strcpy(stompAddress, argv[++i]);
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
        fprintf(stderr, "Usage:  waxrec <device>\n");
        fprintf(stderr, "        [-log [-tee]]                          Output log to stdout, optionally tee to stderr.\n");
        fprintf(stderr, "        [-osc <hostname>[:<port>] [-timetag]]  Send OSC to the specified host/port, time-tag.\n");
        fprintf(stderr, "        [-stomphost <hostname>[:<port>] [-stomptopic /topic/Topic]]  Send STOMP to the specified server.\n");
        fprintf(stderr, "        [-init <string> [-exit]]               Send initialzing string; immediately exit.\n");
        fprintf(stderr, "        [-dump]                                Hex-dump raw packets.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Log example: waxrec %s -log -tee -init \"MODE=1\\r\\n\" > log.csv\n", EXAMPLE_DEVICE);
        fprintf(stderr, "OSC example: waxrec %s -osc localhost:1234 -init \"MODE=1\\r\\n\"\n", EXAMPLE_DEVICE);
        fprintf(stderr, "STOMP example: waxrec %s -stomphost localhost:61613 -stomptopic /topic/Kitchen.Sensor.Wax -init \"MODE=1\\r\\n\"\n", EXAMPLE_DEVICE);
        fprintf(stderr, "\n");
#ifdef _WIN32
        fprintf(stderr, "NOTE: 'device' can be '!' or '![VID+PID]' to automatically find the first matching serial port.\n");
#endif
        fprintf(stderr, "\n");
        return -1;
    }

    fprintf(stderr, "WAXREC: %s -> %s%s%s%s%s %s%s\n", (infile == NULL) ? "<stdin>" : infile, host, (log ? " [log]" : ""), (tee ? " [tee]" : ""), (dump ? " [dump]" : ""), (timetag ? " [timetag]" : ""), stompHost, stompAddress);
    fprintf(stderr, "INIT: %s\n", initString);

    ret = waxrec(infile, host, initString, log, tee, dump, timetag, sendOnly, stompHost, stompAddress);

#if defined(_WIN32) && defined(_DEBUG)
    if (IsDebuggerPresent()) { fprintf(stderr, "Press [enter] to exit..."); getc(stdin); }
#endif
    return ret;
}

