/* 
 * Copyright (c) 2011-2013, Newcastle University, UK.
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
 * by Daniel Jackson, 2011-2013
 */

/*
    Usage:   waxrec <device> [-log [-tee]] [-osc <hostname>[:<port>] [-timetag]] [-init <string>] [-dump]

    Log example: waxrec <device> -log -tee -init "MODE=1\r\n" > log.csv
    OSC example: waxrec <device> -osc localhost:1234 -init "MODE=1\r\n"

    'device' on Windows "\\.\COM1", Mac "/dev/tty.usbmodem*"
*/

/*
 * TODO: It started as a small, self-contained program, but this massive file has become ridiculous!  It should be split nicely in to many smaller files.
 */


//#define POLL_RECV     // STOMP poll receive
//#define THREAD_WRITE_FROM_KEYS
//#define THREAD_WRITE_FROM_UDP


/* Cross-platform alternatives */
#ifdef _WIN32

#define WIN_HANDLE

    /* Defines and headers */
    #define _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_DEPRECATE
    #include <windows.h>
    #include <io.h>
    
    /* Strings */
    #define strcasecmp _stricmp
    #define snprintf _snprintf    

    /* Files */
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)

    /* Sleep */
    #define sleep(seconds) Sleep(seconds * 1000UL)
    #define usleep(microseconds) Sleep(microseconds / 1000UL)

    /* Time */
    #define gmtime_r(timer, result) gmtime_s(result, timer)
    #define timegm _mkgmtime

    /* Socket */
    #include <winsock.h>
    #define _POSIX_
    typedef int socklen_t;
    #pragma warning( disable : 4996 )    /* allow deprecated POSIX name functions */
    #pragma comment(lib, "wsock32")
    
    /* Thread */
	#define thread_t HANDLE
    #define thread_create(thread, attr_ignored, start_routine, arg) ((*(thread) = CreateThread(attr_ignored, 0, start_routine, arg, 0, NULL)) == NULL)
    #define thread_join(thread, value_ptr_ignored) ((value_ptr_ignored), WaitForSingleObject(thread, INFINITE) != WAIT_OBJECT_0)
    #define thread_cancel(thread) (TerminateThread(thread, -1) == 0)
    #define thread_return_t DWORD WINAPI
    #define thread_return_value(value) ((unsigned int)(value))

    /* Mutex */
	#define mutex_t HANDLE
    #define mutex_init(mutex, attr_ignored) ((*(mutex) = CreateMutex(attr_ignored, FALSE, NULL)) == NULL)
    #define mutex_lock(mutex) (WaitForSingleObject(*(mutex), INFINITE) != WAIT_OBJECT_0)
    #define mutex_unlock(mutex) (ReleaseMutex(*(mutex)) == 0)
    #define mutex_destroy(mutex) (CloseHandle(*(mutex)) == 0)

    /* Device discovery */
    #include <setupapi.h>
    #ifdef _MSC_VER
        #include <cfgmgr32.h>
    #endif
    #pragma comment(lib, "setupapi.lib")
    #pragma comment(lib, "advapi32.lib")    /* For RegQueryValueEx() */

#else

    /* Sockets */
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netinet/tcp.h>
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

    /* Thread */
    #include <pthread.h>
    #define thread_t      pthread_t
    #define thread_create pthread_create
    #define thread_join   pthread_join
    #define thread_cancel pthread_cancel
    typedef void *        thread_return_t;
    #define thread_return_value(value_ignored) ((value_ignored), NULL)

    /* Mutex */
	#define mutex_t       pthread_mutex_t
    #define mutex_init    pthread_mutex_init
    #define mutex_lock    pthread_mutex_lock
    #define mutex_unlock  pthread_mutex_unlock
    #define mutex_destroy pthread_mutex_destroy

#endif


/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/stat.h>


/* USB IDs */
#define DEFAULT_VID 0x04D8           /* USB Vendor ID  */
#define DEFAULT_PID 0x000A           /* USB Product ID */


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


/* WAX9 structures */
// 9-axis packet type (always little-endian, transmitted SLIP-encoded)
typedef struct
{
    // Standard part (26-bytes)
    char packetType;                        // @ 0 ASCII '9' for 9-axis
    char packetVersion;                     // @ 1 Version (0x01 = standard, 0x02 = extended)
    unsigned short sampleNumber;            // @ 2 Sample number (reset on configuration change, inactivity, or wrap-around)
    uint32_t timestamp;                     // @ 4 Timestamp (16.16 fixed-point representation, seconds)
    struct { signed short x, y, z; } accel; // @ 8 Accelerometer
    struct { signed short x, y, z; } gyro;  // @14 Gyroscope
    struct { signed short x, y, z; } mag;   // @20 Magnetometer

    // Extended part
    unsigned short battery;                 // @26 Battery (mV)
    short temperature;                      // @28 Temperature (0.1 degrees C)
    uint32_t pressure;                      // @30 Pressure (Pascal)
    //unsigned short deviceId;                // @34 Device identifier
    //                                        // @36
} Wax9Packet;



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
#define ADDITIONAL_OFFSET(interval) (DATA_OFFSET + BITPACK10_SIZEOF(interval * 2))
#define ADDITIONAL_LENGTH 4
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
//#define TEDDI_MS_PER_SAMPLE 250
typedef struct
{
    char valid;
    unsigned long long timestampReceived;
    unsigned long long timestampEstimated;
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
    unsigned short parentAddress;       // Parent node address (optional)
    unsigned short parentAltAddress;    // Parent node alt. address (optional)
} TeddiPacket;
const unsigned short teddiFrequency[16] = { 4, 8, 16, 32, 64, 128, 256, 512, 1, 1,  1,  1,  1,  1,  1,  2 };
#define NUM_COORDINATOR 64
typedef struct
{
    unsigned char  reportType;          // @ 0  [1] USER_REPORT_TYPE (0x12)
    unsigned char  reportId;            // @ 1  [1] Report identifier (0x53, ASCII 'S')
    unsigned short deviceId;            // @ 2  [2] Device identifier (16-bit)
    unsigned char  version;             // @ 4  [1] Low nibble = packet version (0x3), high nibble = config (0x0)
    unsigned char  power;               // @ 5  [1] Power (top-bit indicates USB, if low-15 are 0x7ffff, unknown)
	unsigned short sequence;			// @ 6  [2] Packet sequence number
    unsigned short shortAddress;		// @ 8  [2] Short address
    unsigned char  lastLQI;				// @ 10 [1] LQI of last received keep-alive broadcast
    unsigned char  lastRSSI;			// @ 11 [1] RSSI of last received keep-alive broadcast
    unsigned short parentAddress;		// @ 12 [2] Parent address
    unsigned short parentAltAddress;	// @ 14 [2] Parent alt. address
    unsigned char  neighbours[NUM_COORDINATOR/8];	// @ 16 [8] Neighbouring routers bitmap
    unsigned long long timestampReceived;
} TeddiStatusPacket;


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

  /* Connect */
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
		fprintf(stderr, "[%d]\n", (int)sendLength);
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
/* reconnect delay -- make time-based instead */
#define STOMP_DELAY 100

typedef struct TinyStompTransmitter_t
{
	int _sock;
	unsigned char *_buffer;
	int _bufferPos;
    char *_host;
    int _port;
    int _delay;
} TinyStompTransmitter;

TinyStompTransmitter *stompTransmitter = NULL;


TinyStompTransmitter *TinyStompTransmitter_New(const char *host, int defaultPort)
{
	TinyStompTransmitter *trans;

	trans = (TinyStompTransmitter *)malloc(sizeof(TinyStompTransmitter));
	trans->_buffer = (unsigned char *)malloc(STOMP_BUFFER_SIZE);
	trans->_bufferPos = 0;
	trans->_sock = SOCKET_ERROR;
    trans->_host = strdup(host);
    trans->_port = defaultPort;
    trans->_delay = 0;

    /* Parse and remove port */
    {
        char *portIndex;
        if ((portIndex = strstr(trans->_host, ":")) != NULL)
        {
            *portIndex++ = '\0';
            trans->_port = atoi(portIndex);
        }
    }

	return trans;
}


void TinyStompTransmitter_Transmit(TinyStompTransmitter *inst, const char *destination, const char *message, const char *user, const char *password)
{
	/* Start packet */
	if (inst == NULL) { return; }
	inst->_bufferPos = 0;

	inst->_bufferPos += sprintf((char *)inst->_buffer + inst->_bufferPos, "SEND\r\ndestination:%s\r\n\r\n%s\r\n", destination, message);
	inst->_buffer[inst->_bufferPos++] = '\0';

	/* Open TCP socket */
	if (inst->_sock == SOCKET_ERROR && inst->_host != NULL)
	{
        if (inst->_delay == 0)
        {
		    inst->_sock = opentcpsocket(inst->_host, inst->_port);
		    if (inst->_sock != SOCKET_ERROR)
		    {
			    char msg[256];
			    size_t tlen;
                sprintf(msg, "CONNECT\r\nlogin:%s\r\npasscode:%s\r\n\r\n", user, password);
			    tlen = tcptransmit((SOCKET)inst->_sock, msg, strlen(msg) + 1);
			    if (tlen != strlen(msg) + 1)
                {
                    fprintf(stderr, "WARNING: Problem transmitting CONNECT: %d / %d\n", (int)tlen, (int)strlen(msg) + 1);
                    inst->_sock = SOCKET_ERROR;
                    inst->_delay = STOMP_DELAY;
                }
		    }
        }
        else
        {
            /* Wait until reconnect attempt */
            inst->_delay--;
        }
	}

	if (inst->_sock != SOCKET_ERROR)
	{ 
		size_t tlen = tcptransmit((SOCKET)inst->_sock, inst->_buffer, inst->_bufferPos);
		if (tlen != inst->_bufferPos)
        {
            fprintf(stderr, "WARNING: Problem transmitting: %d / %d\n", (int)tlen, inst->_bufferPos); 
		    closesocket(inst->_sock);
            inst->_sock = SOCKET_ERROR;
            inst->_delay = STOMP_DELAY;
        }
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
		if (tlen != strlen(msg) + 1) { fprintf(stderr, "WARNING: Problem transmitting DISCONNECT: %d / %d\n", (int)tlen, (int)strlen(msg) + 1); }
		closesocket(inst->_sock);
		inst->_sock = SOCKET_ERROR;
	}
	if (inst->_host != NULL)
    {
        free(inst->_host);
        inst->_host = NULL;
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
#if defined(_WIN32) && defined(WIN_HANDLE)
        {
            int done;
            ReadFile((HANDLE)fd, &c, 1, (DWORD *)&done, 0);
            if (done <= 0) { return received; }
        }
#else
        if (read(fd, &c, 1) <= 0) { return received; }
#endif
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
#if defined(_WIN32) && defined(WIN_HANDLE)
        {
            int done;
            ReadFile((HANDLE)fd, &c, 1, (DWORD *)&done, 0);
            if (done <= 0) { return received; }
        }
#else
        if (read(fd, &c, 1) <= 0) { return received; }
#endif
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


/* Parse a binary WAX9 packet */
Wax9Packet *parseWax9Packet(const void *inputBuffer, size_t len, unsigned long long now)
{
    const unsigned char *buffer = (const unsigned char *)inputBuffer;
    static Wax9Packet wax9Packet;

    if (buffer == NULL || len <= 0) { return 0; }

    if (buffer[0] != '9')
    {
        fprintf(stderr, "WARNING: Unrecognized packet -- ignoring.\n");
    }
    else if (len >= 20)
    {
        wax9Packet.packetType = buffer[0];
        wax9Packet.packetVersion = buffer[1];
        wax9Packet.sampleNumber = buffer[2] | ((unsigned short)buffer[3] << 8);
        wax9Packet.timestamp = buffer[4] | ((unsigned int)buffer[5] << 8) | ((unsigned int)buffer[6] << 16) | ((unsigned int)buffer[7] << 24);

        wax9Packet.accel.x = (short)((unsigned short)(buffer[ 8] | (((unsigned short)buffer[ 9]) << 8)));
        wax9Packet.accel.y = (short)((unsigned short)(buffer[10] | (((unsigned short)buffer[11]) << 8)));
        wax9Packet.accel.z = (short)((unsigned short)(buffer[12] | (((unsigned short)buffer[13]) << 8)));

        if (len >= 20)
        {
            wax9Packet.gyro.x  = (short)((unsigned short)(buffer[14] | (((unsigned short)buffer[15]) << 8)));
            wax9Packet.gyro.y  = (short)((unsigned short)(buffer[16] | (((unsigned short)buffer[17]) << 8)));
            wax9Packet.gyro.z  = (short)((unsigned short)(buffer[18] | (((unsigned short)buffer[19]) << 8)));
        }
        else
        {
            wax9Packet.gyro.x   = 0;
            wax9Packet.gyro.y   = 0;
            wax9Packet.gyro.z   = 0;
        }

        if (len >= 26)
        {
            wax9Packet.mag.x   = (short)((unsigned short)(buffer[20] | (((unsigned short)buffer[21]) << 8)));
            wax9Packet.mag.y   = (short)((unsigned short)(buffer[22] | (((unsigned short)buffer[23]) << 8)));
            wax9Packet.mag.z   = (short)((unsigned short)(buffer[24] | (((unsigned short)buffer[25]) << 8)));
        }
        else
        {
            wax9Packet.mag.x   = 0;
            wax9Packet.mag.y   = 0;
            wax9Packet.mag.z   = 0;
        }

        if (len >= 28)
        {
            wax9Packet.battery = (unsigned short)(buffer[26] | (((unsigned short)buffer[27]) << 8));
        }
        else
        {
            wax9Packet.battery = 0xffff;
        }

        if (len >= 30)
        {
            wax9Packet.temperature = (short)((unsigned short)(buffer[28] | (((unsigned short)buffer[29]) << 8)));
        }
        else
        {
            wax9Packet.temperature = 0xffff;
        }

        if (len >= 34)
        {
            wax9Packet.pressure = buffer[30] | ((unsigned int)buffer[31] << 8) | ((unsigned int)buffer[32] << 16) | ((unsigned int)buffer[33] << 24);
        }
        else
        {
            wax9Packet.pressure = 0xfffffffful;
        }

        return &wax9Packet;
    }
    else
    {
        fprintf(stderr, "WARNING: Unrecognized WAX9 packet -- ignoring.\n");
    }
    return NULL;
}


/* Parse a binary TEDDI packet */
TeddiPacket *parseTeddiPacket(const void *inputBuffer, size_t len, unsigned long long now)
{
    const unsigned char *buffer = (const unsigned char *)inputBuffer;
    static TeddiPacket teddiPacket;
    int i;

    teddiPacket.valid = 0;
    if (buffer == NULL || len <= 0) { return 0; }

    if (len >= 5 && buffer[0] == 0x12 && buffer[1] == 0x54)
    {
        teddiPacket.deviceId = (unsigned short)(buffer[2] | (((unsigned short)buffer[3]) << 8));
        teddiPacket.version = buffer[4];

        if (((teddiPacket.version & 0x0f) == 0x03 || (teddiPacket.version & 0x0f) >= 0x04) && len >= 18)
        {
            /*
            #define DATA_OFFSET 18
            #define ADDITIONAL_OFFSET(interval) (DATA_OFFSET + BITPACK10_SIZEOF(interval * 2))
            #define ADDITIONAL_LENGTH 4
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
            unsigned short parentAddress;		// @ADDITIONAL_OFFSET+0  [2] (optional) Parent address
            unsigned short parentAltAddress;	// @ADDITIONAL_OFFSET+2  [2] (optional) Parent alt. address
            */
            unsigned char config = (unsigned char)(buffer[4] >> 4);
            int msPerSample = 1000 / teddiFrequency[(teddiPacket.version >> 4)];
            teddiPacket.sampleCount = (unsigned char)(buffer[5]);         // Sample count (default config is at 250 msec interval with an equal number of PIR and audio samples; 20 = 5 seconds)
            teddiPacket.sequence = (unsigned short)(buffer[ 6] | (((unsigned short)buffer[ 7]) << 8));
            teddiPacket.unsent =   (unsigned short)(buffer[ 8] | (((unsigned short)buffer[ 9]) << 8));
            teddiPacket.temp =     (unsigned short)(buffer[10] | (((unsigned short)buffer[11]) << 8));
            teddiPacket.light =    (unsigned short)(buffer[12] | (((unsigned short)buffer[13]) << 8));
            teddiPacket.battery =  (unsigned short)(buffer[14] | (((unsigned short)buffer[15]) << 8));

            // Parent address is optional
            teddiPacket.parentAddress = 0;
            teddiPacket.parentAltAddress = 0;

            teddiPacket.valid = 1;

            if ((teddiPacket.version & 0x0f) >= 0x04)
            {
                int additionalOffset = ADDITIONAL_OFFSET(teddiPacket.sampleCount);
                teddiPacket.humidity = (unsigned short)(buffer[16] | (((unsigned short)buffer[17]) << 8));

                if (additionalOffset != (int)len && additionalOffset + 4 != (int)len)
                {
                    fprintf(stderr, "WARNING: TEDDI packet incorrect length (%d, expected %d or 4 less) - probably corrupt?\n", len, additionalOffset + 4);
                    teddiPacket.valid = 0;
                }

                if (additionalOffset + 2 <= (int)len) { teddiPacket.parentAddress = (unsigned short)(buffer[additionalOffset + 0] | (((unsigned short)buffer[additionalOffset + 1]) << 8)); }
                if (additionalOffset + 4 <= (int)len) { teddiPacket.parentAltAddress = (unsigned short)(buffer[additionalOffset + 2] | (((unsigned short)buffer[additionalOffset + 3]) << 8)); }
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
            if ((teddiPacket.version & 0x0f) <= 0x03 && teddiPacket.sampleCount > 0)
            {
                teddiPacket.temp     /= teddiPacket.sampleCount;
                teddiPacket.light    /= teddiPacket.sampleCount;
                teddiPacket.battery  /= teddiPacket.sampleCount;
                teddiPacket.humidity /= teddiPacket.sampleCount;
            }

// [Each PIR/sound sample]
//   time = timestamp - TimeSpan.FromMilliseconds((unsent + sampleCount - 1 - i) * sampleInterval)
//   seq = (unsigned short)(sequence * sampleCount) + i
// [Overall packet]
//   time = timestamp - TimeSpan.FromMilliseconds((unsent + sampleCount - 1) * sampleInterval
teddiPacket.timestampReceived = now;
teddiPacket.timestampEstimated = teddiPacket.timestampReceived - (teddiPacket.unsent + teddiPacket.sampleCount - 1) * msPerSample;

            return &teddiPacket;
        }
        else
        {
            fprintf(stderr, "WARNING: Unrecognized TEDDI packet -- ignoring.\n");
        }
    }
    return NULL;
}

/* Parse a binary TEDDI status packet */
TeddiStatusPacket *parseTeddiStatusPacket(const void *inputBuffer, size_t len, unsigned long long now)
{
    const unsigned char *buffer = (const unsigned char *)inputBuffer;
    static TeddiStatusPacket teddiStatusPacket;
    int i;

    if (buffer == NULL || len <= 0) { return 0; }

    if (len >= 5 && buffer[0] == 0x12 && buffer[1] == 0x53)
    {
        teddiStatusPacket.reportType = buffer[0];
        teddiStatusPacket.reportId = buffer[1];
        teddiStatusPacket.deviceId = (unsigned short)(buffer[2] | (((unsigned short)buffer[3]) << 8));
        teddiStatusPacket.version = buffer[4];

        if (len >= 24)
        {
            /*
            typedef struct
            {
                unsigned char  reportType;          // @ 0  [1] USER_REPORT_TYPE (0x12)
                unsigned char  reportId;            // @ 1  [1] Report identifier (0x53, ASCII 'S')
                unsigned short deviceId;            // @ 2  [2] Device identifier (16-bit)
                unsigned char  version;             // @ 4  [1] Low nibble = packet version (0x3), high nibble = config (0x0)
                unsigned char  power;               // @ 5  [1] Power (top-bit indicates USB)
	            unsigned short sequence;			// @ 6  [2] Packet sequence number
                unsigned short shortAddress;		// @ 8  [2] Short address
                unsigned char  lastLQI;				// @ 10 [1] LQI of last received keep-alive broadcast
                unsigned char  lastRSSI;			// @ 11 [1] RSSI of last received keep-alive broadcast
                unsigned short parentAddress;		// @ 12 [2] Parent address
                unsigned short parentAltAddress;	// @ 14 [2] Parent alt. address
                unsigned char  neighbours[NUM_COORDINATOR/8];	// @ 16 [8] Neighbouring routers bitmap
            } TeddiStatusPacket;
            */
            teddiStatusPacket.power =             (unsigned char)(buffer[5]);         // Sample count (default config is at 250 msec interval with an equal number of PIR and audio samples; 20 = 5 seconds)
            teddiStatusPacket.sequence =          (unsigned short)(buffer[ 6] | (((unsigned short)buffer[ 7]) << 8));
            teddiStatusPacket.shortAddress =      (unsigned short)(buffer[ 8] | (((unsigned short)buffer[ 9]) << 8));
            teddiStatusPacket.lastLQI =           buffer[10];
            teddiStatusPacket.lastRSSI =          buffer[11];
            teddiStatusPacket.parentAddress =     (unsigned short)(buffer[12] | (((unsigned short)buffer[13]) << 8));
            teddiStatusPacket.parentAltAddress =  (unsigned short)(buffer[14] | (((unsigned short)buffer[15]) << 8));

            // Neighbour table
            for (i = 0; i < NUM_COORDINATOR/8; i++)
            {
                teddiStatusPacket.neighbours[i] = buffer[16 + i];
            }

            teddiStatusPacket.timestampReceived = now;

            return &teddiStatusPacket;
        }
        else
        {
            fprintf(stderr, "WARNING: Unrecognized TEDDI status packet -- ignoring.\n");
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
const char *timestamp(unsigned long long ticks, int timeformat)
{
	static char output[] = "00000000000.000,YYYY-MM-DD HH:MM:SS.fff";
	output[0] = '\0';
	if (timeformat & 1)
	{
		double t = ticks / 1000.0;
		if (strlen(output) != 0) { strcat(output, ","); }
		sprintf(output + strlen(output), "%.3f", t);
	}
	if (timeformat & 2)
	{
		struct tm *today;
		struct timeb tp = {0};
		tp.time = (time_t)(ticks / 1000);
		tp.millitm = (unsigned short)(ticks % 1000);
		tzset();
		today = localtime(&(tp.time));
		if (strlen(output) != 0) { strcat(output, ","); }
		sprintf(output + strlen(output), "%04d-%02d-%02d %02d:%02d:%02d.%03d", 1900 + today->tm_year, today->tm_mon + 1, today->tm_mday, today->tm_hour, today->tm_min, today->tm_sec, tp.millitm);
	}
    return output;
}

/* Dumps a WAX packet */
void waxDump(WaxPacket *waxPacket, FILE *ofp, char tee, int timeformat)
{
    int i;
    for (i = 0; i < waxPacket->sampleCount; i++)
    {
        const char *timeString = timestamp(waxPacket->samples[i].timestamp, timeformat);
        fprintf(ofp, "ACCEL,%s,%u,%u,%f,%f,%f\n", timeString, waxPacket->deviceId, waxPacket->samples[i].sampleIndex, waxPacket->samples[i].x / 256.0f, waxPacket->samples[i].y / 256.0f, waxPacket->samples[i].z / 256.0f);
        if (tee) fprintf(stderr, "ACCEL,%s,%u,%u,%f,%f,%f\n", timeString, waxPacket->deviceId, waxPacket->samples[i].sampleIndex, waxPacket->samples[i].x / 256.0f, waxPacket->samples[i].y / 256.0f, waxPacket->samples[i].z / 256.0f);
    }
    return;
}


/* Dumps a WAX9 packet */
void wax9Dump(Wax9Packet *wax9Packet, FILE *ofp, char tee, int timeformat, unsigned long long receivedTime)
{
    int i;
    const char *timeString = timestamp(receivedTime, timeformat);
    for (i = 0; i <= tee ? 1 : 0; i++)
    {
        fprintf((i == 0) ? ofp : stderr, "$WAX9,%s,%u,%u,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", timeString, wax9Packet->sampleNumber, wax9Packet->timestamp, 
            wax9Packet->accel.x / 2048.0f, wax9Packet->accel.y / 2048.0f, wax9Packet->accel.z / 2048.0f, 
            wax9Packet->gyro.x * 0.07f,    wax9Packet->gyro.y * 0.07f,    wax9Packet->gyro.z * 0.07f, 
            wax9Packet->mag.x, wax9Packet->mag.y, wax9Packet->mag.z
            );
    }
    return;
}


/* Dumps a TEDDI packet */
void teddiDump(TeddiPacket *teddiPacket, FILE *ofp, char tee, int format, char ignoreInvalid)
{
    static char line[2048];
    static char number[32];
    char *labelAppend = "";
    int i;

    if (!teddiPacket->valid)
    {
        if (ignoreInvalid == 1) { return; }
        else if (ignoreInvalid == 2) { labelAppend = "!"; }
    }

    if (format == 1)
    {
        unsigned short imin = 0, imax = 0, iav = 0;
        int itot = 0;

        sprintf(line, "TEDDI_SHORT%s,%s,%u,%u,%u,%u,%u,%u,%u,%u,%u", labelAppend, timestamp(teddiPacket->timestampEstimated, 3), teddiPacket->deviceId, teddiPacket->version,
                                            teddiPacket->sampleCount, teddiPacket->sequence, teddiPacket->unsent,
                                            teddiPacket->temp, teddiPacket->light, teddiPacket->battery, teddiPacket->humidity);

        for (i = 0; i < teddiPacket->sampleCount; i++)
        {
            if (teddiPacket->pirData[i] < imin) { imin = teddiPacket->pirData[i]; }
            if (teddiPacket->pirData[i] > imax) { imax = teddiPacket->pirData[i]; }
            itot = itot + teddiPacket->pirData[i];
        }
        iav = itot / 24;

        //fprintf(stderr, "PIR   min max avg: %u,%u,%u\n",imin,imax,iav);
        sprintf(number, ",%u,%u,%d", imin,imax,iav);
        strcat(line, number);

        imin=0, imax=0, iav=0, itot=0;
        for (i = 0; i < teddiPacket->sampleCount; i++)
        {
            if (teddiPacket->audioData[i] < imin) {imin = teddiPacket->audioData[i]; }
            if (teddiPacket->audioData[i] > imax) {imax = teddiPacket->audioData[i]; }
            itot = itot + teddiPacket->audioData[i];
        }
        iav = itot / 24;

        //fprintf(stderr, "Audio min max avg: %u,%u,%u\n",imin,imax,iav);
        sprintf(number, ",%u,%u,%u", imin,imax,iav);
        strcat(line, number);

        strcat(line, "\n");
    }
    else
    {
        sprintf(line, "TEDDI%s,%s,%u,%u,%u,%u,%u,%u,%u,%u,%u", labelAppend, timestamp(teddiPacket->timestampReceived, 3), teddiPacket->deviceId, teddiPacket->version, 
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
    }

    fprintf(ofp, "%s", line);
    if (tee) fprintf(stderr, "%s", line);
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


/* Create an OSC bundle for the WAX9 data */
size_t wax9ToOsc(Wax9Packet *wax9Packet, void *outputBuffer, char timetag, unsigned long long receivedTime)
{
    unsigned char *buffer = (unsigned char *)outputBuffer;
    size_t o = 0;
    char address[16];

    sprintf(address, "/wax9");      //sprintf(address, "/wax9/%d", waxPacket->deviceId);

    o += write_osc_string(buffer + o, "#bundle");                            /* [OSC string] bundle identifier: "#bundle" <pads to 8 bytes> */
    o += write_osc_timetag(buffer + o, receivedTime);                        /* [OSC timetag] timestamp <8 bytes> */

    /* OSC messages */
    {
        int msgLen = 76 + ((strlen(address) >= 8) ? 4 : 0);
        o += write_osc_int(buffer + o, msgLen);                                 /* [OSC int] message length */
        o += write_osc_string(buffer + o, address);                             /* [OSC string] address: "/wax9/#####" <pads to 12 bytes if ID >= 100, or 8 bytes otherwise> */
        o += write_osc_string(buffer + o, ",fffffffffii");                      /* [OSC string] type tag: <pads to 16 bytes> */
        o += write_osc_float(buffer + o, wax9Packet->accel.x / 2048.0f);        /* [OSC float] Accel. X-axis <4 bytes> */
        o += write_osc_float(buffer + o, wax9Packet->accel.y / 2048.0f);        /* [OSC float] Accel. Y-axis <4 bytes> */
        o += write_osc_float(buffer + o, wax9Packet->accel.z / 2048.0f);        /* [OSC float] Accel. Z-axis <4 bytes> */
        o += write_osc_float(buffer + o, wax9Packet->gyro.x * 0.07f);           /* [OSC float] Gyro. X-axis <4 bytes> */
        o += write_osc_float(buffer + o, wax9Packet->gyro.y * 0.07f);           /* [OSC float] Gyro. Y-axis <4 bytes> */
        o += write_osc_float(buffer + o, wax9Packet->gyro.z * 0.07f);           /* [OSC float] Gyro. Z-axis <4 bytes> */
        o += write_osc_float(buffer + o, wax9Packet->mag.x);                    /* [OSC float] Mag. X-axis <4 bytes> */
        o += write_osc_float(buffer + o, wax9Packet->mag.y);                    /* [OSC float] Mag. Y-axis <4 bytes> */
        o += write_osc_float(buffer + o, wax9Packet->mag.z);                    /* [OSC float] Mag. Z-axis <4 bytes> */
        o += write_osc_int(buffer + o, wax9Packet->sampleNumber);               /* [OSC int] sample index <4 bytes> */
        o += write_osc_int(buffer + o, wax9Packet->timestamp);                  /* [OSC int] sample time (16.16) <4 bytes> */
    }

    return o;
}


/* Create an OSC bundle for the TEDDI data */
size_t teddiToOsc(TeddiPacket *teddiPacket, void *outputBuffer, char timetag)
{
    unsigned char *buffer = (unsigned char *)outputBuffer;
    size_t o = 0;
    int i;
    char address[16];

    sprintf(address, "/teddi/%d", teddiPacket->deviceId);

    o += write_osc_string(buffer + o, "#bundle");                              /* [OSC string] bundle identifier: "#bundle" <pads to 8 bytes> */
    o += write_osc_timetag(buffer + o, teddiPacket->timestampEstimated);       /* [OSC timetag] timestamp <8 bytes> */

    /* OSC messages */
    for (i = 0; i < teddiPacket->sampleCount; i++)
    {
        int msgLen = ((strlen(address) >= 8) ? 56 : 52) + (timetag ? 8 : 0);
        o += write_osc_int(buffer + o, msgLen);                                 /* [OSC int] message length: 52 or 56 (8 + 12|16 + 8*4) */
        o += write_osc_string(buffer + o, address);                             /* [OSC string] address: "/teddi/#####" <pads to 12/16 bytes> */
        o += write_osc_string(buffer + o, timetag ? ",iiiiiiiit" : ",iiiiiiii");  /* [OSC string] type tag: <pads to 12 bytes> */

        o += write_osc_int(buffer + o, teddiPacket->deviceId);                  /* [OSC int] Device id <4 bytes> */

        o += write_osc_int(buffer + o, teddiPacket->temp);                      /* [OSC int] Temperature raw value <4 bytes> */
        o += write_osc_int(buffer + o, teddiPacket->light);                     /* [OSC int] Light raw value <4 bytes> */
        o += write_osc_int(buffer + o, teddiPacket->battery);                   /* [OSC int] Battery raw value <4 bytes> */
        o += write_osc_int(buffer + o, teddiPacket->humidity);                  /* [OSC int] Humidity raw value <4 bytes> */

        o += write_osc_int(buffer + o, teddiPacket->pirData[i]);                /* [OSC int] PIR raw value <4 bytes> */
        o += write_osc_int(buffer + o, teddiPacket->audioData[i]);              /* [OSC int] Audio raw value <4 bytes> */

        o += write_osc_int(buffer + o, (unsigned int)teddiPacket->sequence * teddiPacket->sampleCount + i); /* [OSC int] Sample index value <4 bytes> */

        if (timetag)
        {
            o += write_osc_timetag(buffer + o, teddiPacket->timestampEstimated + i * (0xffffffffu / 4));  /* [OSC timetag] timestamp <8 bytes>, each sample at 4Hz */
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

#ifdef _WIN32
        {
            HANDLE hSerial;
            hSerial = CreateFileA(infile, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            if (hSerial == INVALID_HANDLE_VALUE) { fd = -1; fprintf(stderr, "ERROR: CreateFile problem '%s'\n", infile); }
            else
            { 
#ifdef WIN_HANDLE
                fd = (intptr_t)hSerial;
#else
                // This fails for Bluetooth serial ports... keep as a handle
                fd = _open_osfhandle((intptr_t)hSerial, 0); 
                if (fd == -1) { fprintf(stderr, "ERROR: Problem converting handle %x to fd %d for '%s'\n", hSerial, fd, infile);  } 
#endif
            }
        }
#else
        fd = open(infile, flags);
#endif

        if (fd < 0)
        {
            fprintf(stderr, "ERROR: Problem opening input: '%s'\n", infile);
            return -1;
        }

        /* Set serial port parameters (OS-specific) */
#ifdef _WIN32
        {
            HANDLE hSerial;
            DCB dcbSerialParams = {0};
            COMMTIMEOUTS timeouts = {0};

#ifdef WIN_HANDLE
            hSerial = (HANDLE)fd;
#else
            hSerial = (HANDLE)_get_osfhandle(fd);
#endif
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


// _MSC_VER instead of _WIN32 as I can't get it to compile under Cygwin (-lcfgmgr32 doesn't seem to help)
#ifdef _MSC_VER
/* Find port names for a given set of constraints */
static int Win32FindPorts(const char *prefix, const char *substring, char *buffer, size_t bufferSize)
{
    int count = 0;
    int pos = 0;
    GUID *pGuids;
    DWORD dwGuids = 0;
    unsigned int guidIndex;
    int index;
    
    if (buffer != NULL)
    {
        buffer[pos] = '\0';
        buffer[pos + 1] = '\0';
    }
    
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
            if ((prefix == NULL || strncmp(usbId, prefix, strlen(prefix)) == 0) && (substring == NULL || strstr(usbId, substring) != NULL))
            {
                HKEY hDeviceKey;

#if 1
                /* Move up one level to get to the composite device string */
                char usbComposite[MAX_PATH] = "";
                DWORD parent = 0;
                CM_Get_Parent(&parent, devInfo.DevInst, 0);
                CM_Get_Device_IDA(parent, usbComposite, MAX_PATH, 0);
                if (prefix == NULL || strncmp(usbComposite, prefix, strlen(prefix)) != 0)
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
                        pos += sprintf(buffer + pos, "\\\\.\\%s", portName);
                        *(buffer + ++pos) = '\0';

                        count++;
                    }
                }

            }
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
    free(pGuids); 

    return count;
}
#endif


/* Find port names for a given USB VID & PID */
static int FindPorts(unsigned short vid, unsigned short pid, char *buffer, size_t bufferSize)
{
// _MSC_VER instead of _WIN32 as I can't get it to compile under Cygwin (-lcfgmgr32 doesn't seem to help)
#ifdef _MSC_VER
    /* PNPDeviceID to search for */
    char prefix[32];
    sprintf(prefix, "USB\\VID_%04X&PID_%04X", vid, pid);
    return Win32FindPorts(prefix, NULL, buffer, bufferSize);
#else
    // Not implemented
    if (buffer != NULL) { buffer[0] = '\0'; buffer[1] = '\0'; }
    return 0;
#endif
}


static int FindBluetoothPorts(const char *address, char *buffer, size_t bufferSize)
{
// _MSC_VER instead of _WIN32 as I can't get it to compile under Cygwin (-lcfgmgr32 doesn't seem to help)
#ifdef _MSC_VER
    const char *s = address;
    char normalizedAddress[128];
    char *d = normalizedAddress;

    // Copy address, removing non-hex characters, as upper-case.
    do
    {
        char c = *s;
        if (c >= 'a' && c <= 'f') { c = c + 'A' - 'a'; }
        if (c == '\0' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))
        {
            *d++ = c;
        }
    } while (*s++ != '\0');
    
    return Win32FindPorts("BTHENUM\\", normalizedAddress, buffer, bufferSize);
#else

    // Not implemented
    if (buffer != NULL) { buffer[0] = '\0'; buffer[1] = '\0'; }
    return 0;

#endif
}



#ifdef THREAD_WRITE_FROM_KEYS

static thread_t threadReceiveKeys;
static char quitReceiveKeys = 0;
static int receiveKeysFD = 0;

thread_return_t ReceiveKeysThread(void *arg)
{
    fprintf(stderr, "KEYLISTEN: Start\n");
#if 0
    {
        FILE *fp = fdopen(receiveKeysFD, "r+");
        while (!quitReceiveKeys)
        {
            char line[1024];
            if (fgets(line, sizeof(line), fp) == NULL) { break; }
            fprintf(stderr, "<\"%s\">", line);
            //fwrite(line, 1, strlen(line), fp);
            write(receiveKeysFD, line, strlen(line));
        }
        fclose(fp);
    }
#else
    while (!quitReceiveKeys)
    {
        int ch = getc(stdin);
        char c;
        if (ch == EOF) { break; }
        c = (char)ch;
        fprintf(stderr, "<%c>", c);
        if (receiveKeysFD != 0)
        {
            write(receiveKeysFD, &c, 1);
        }
    }
#endif        
    fprintf(stderr, "KEYLISTEN: Stop\n");
    return thread_return_value(0);
}

void ReceiveKeysStart(void)
{
    quitReceiveKeys = 0;
    thread_create(&threadReceiveKeys, NULL, ReceiveKeysThread, NULL);
}

void ReceiveKeysStop(void)
{
    quitReceiveKeys = 1;
    /* thread_join(threadReceiveKeys, NULL); */
    thread_cancel(threadReceiveKeys);
}

#endif


#ifdef THREAD_WRITE_FROM_UDP

static thread_t threadReceiveUdp;
static char quitReceiveUdp = 0;
static int receiveUdpFD = 0;
static int receiveUdpPort = 0;


thread_return_t ReceiveUdpThread(void *arg)
{
    char serverName[64] = "localhost";
    struct sockaddr_in serverAddr;
    SOCKET serverSocket = SOCKET_ERROR;
    const int recvMaxLength = 65535;
    unsigned char *recvBuffer;
    int recvLength = 0;
    struct hostent *hp;

    fprintf(stderr, "UDPRECV: Start\n");
    recvBuffer = (unsigned char *)malloc(recvMaxLength + 2);

    /* Get server host information, name and inet address */
    gethostname(serverName, 64);
    hp = gethostbyname(serverName);
    memcpy(&(serverAddr.sin_addr), hp->h_addr, hp->h_length);
    serverAddr.sin_family = AF_INET; 
    serverAddr.sin_port = htons(receiveUdpPort);
    fprintf(stderr, "UDPRECV: Address: [%s] = %s : %d\n", hp->h_name, inet_ntoa(serverAddr.sin_addr), receiveUdpPort);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    /* Create the socket */
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0); 
    if (serverSocket < 0)
    {
        fprintf(stderr, "UDPRECV: ERROR: Socket creation failed (%s)\n", strerrorsocket());
        return thread_return_value(0);
    }
    
    /* Allow rapid reuse of this socket */
    {
        int option = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option));
    }
    
    /* Bind the socket */
    if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "UDPRECV: ERROR: Socket bind failed (%s)\n", strerrorsocket());
        return thread_return_value(0);
    }

    while (!quitReceiveUdp)
    {
        struct sockaddr_in from;
        int fromlen = sizeof(from);
        int recvLength;
        
        fprintf(stderr, "UDPRECV: Listening...\n");
        recvLength = recvfrom(serverSocket, (char *)recvBuffer, recvMaxLength, 0, (struct sockaddr *)&from, &fromlen);
        
        if (recvLength == 0 || recvLength == SOCKET_ERROR)
        {
            fprintf(stderr, "UDPRECV: ERROR: Receive failed (%s)\n", strerrorsocket());
            return thread_return_value(0);
        }
        
        fprintf(stderr, "UDPRECV: Received from %s:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port));

        /* Check if it appears to be an OSC bundle or OSC packet... */
        if (recvLength >= 1 && recvBuffer[0] == '/')
        {
            fprintf(stderr, "WARNING: OSC packet receive not yet implemented.\n");
            //if (dump) { hexdump(recvBuffer, recvLength); }
        }
        else if (recvLength >= 1 && recvBuffer[0] == '#')
        {
            fprintf(stderr, "WARNING: OSC bundle receive not yet implemented.\n");
            //if (dump) { hexdump(recvBuffer, recvLength); }
        }
        else
        {
            /* Pass-through data */
            //if (dump) { hexdump(recvBuffer, recvLength); }
            if (receiveUdpFD != 0)
            {
                write(receiveUdpFD, recvBuffer, recvLength);
            }
        }

    }

    closesocket(serverSocket);
    free(recvBuffer);

    fprintf(stderr, "UDPRECV: End\n");

    return thread_return_value(0);
}

void ReceiveUdpStart(int port)
{
    receiveUdpPort = port;
    fprintf(stderr, "NOTE: Listening for UDP on port %d.\n", receiveUdpPort);
    quitReceiveUdp = 0;
    thread_create(&threadReceiveUdp, NULL, ReceiveUdpThread, NULL);
}

void ReceiveUdpStop(void)
{
    quitReceiveUdp = 1;
    /* thread_join(threadReceiveUdp, NULL); */
    thread_cancel(threadReceiveUdp);
}

#endif


FILE *openlogfile(const char *logfile, unsigned long long ticks, unsigned long *loginterval)
{
    char temp[260];
    const char *sp;
    int o;
    struct tm *today;
    struct timeb tp = {0};
    int year, month, day, hours, mins, secs;
    int fields = 0;
    FILE *outfp = NULL;
    
    tp.time = (time_t)(ticks / 1000);
    tp.millitm = (unsigned short)(ticks % 1000);
    tzset();
    today = localtime(&(tp.time));
    year = 1900 + today->tm_year;
    month = today->tm_mon + 1;
    day = today->tm_mday;
    hours = today->tm_hour;
    mins = today->tm_min;
    secs = today->tm_sec;
    
    if (logfile == NULL) { *loginterval = 0; return NULL; }
    if (logfile[0] == '\0') { *loginterval = 0; return stdout; }

    sp = logfile;
    o = 0;
    for(;;)
    {
        char c;
        
        c = *sp++;
        if (c == '\\' || c == '/')
        {
            struct stat st;

            /* Make folder part */
            temp[o] = '\0';
            
fprintf(stderr, "DEBUG: Creating log directory: %s\n", temp);

            if (strlen(temp) != 0 && stat(temp, &st) != 0)
            {
                if (mkdir(temp, 0777) != 0)
                {
                    fprintf(stderr, "ERROR: mkdir failed");
                    if (errno != EEXIST)
                    {
                        fprintf(stderr, "ERROR: Cannot make log directory: %s\n", temp);
                        return NULL;
                    }
                }
            }
            else if (strlen(temp) != 0 && !((st.st_mode) & S_IFDIR))
            {
                fprintf(stderr, "ERROR: Path not a directory: %s\n", temp);
                return NULL;
            }

            temp[o++] = '/';

        }
        else if (c == '\0')
        {
            /* Open file for append */
            temp[o] = '\0';
fprintf(stderr, "DEBUG: Opening log file: %s\n", temp);
            outfp = fopen(temp, "a");
            break;
        }
        else if (c == '@' && *sp == 'Y') { sp++; temp[o++] = '0' + (( year / 1000) % 10); temp[o++] = '0' + ((year / 100) % 10); temp[o++] = '0' + ((year / 10) % 10); temp[o++] = '0' + (year % 10); fields |= 1; }
        else if (c == '@' && *sp == 'M') { sp++; temp[o++] = '0' + ((month / 10) % 10); temp[o++] = '0' + (month % 10); fields |= 2;  }
        else if (c == '@' && *sp == 'D') { sp++; temp[o++] = '0' + ((  day / 10) % 10); temp[o++] = '0' + (  day % 10); fields |= 4; }
        else if (c == '@' && *sp == 'h') { sp++; temp[o++] = '0' + ((hours / 10) % 10); temp[o++] = '0' + (hours % 10); fields |= 8; }
        else if (c == '@' && *sp == 'm') { sp++; temp[o++] = '0' + (( mins / 10) % 10); temp[o++] = '0' + ( mins % 10); fields |= 16; }
        else if (c == '@' && *sp == 's') { sp++; temp[o++] = '0' + (( secs / 10) % 10); temp[o++] = '0' + ( secs % 10); fields |= 32; }
        else
        {
            /* Append standard character */
            temp[o++] = c;
        }
    }

    /* Calculate log interval */
    if (fields & 32) { *loginterval = 1 * 1000ul; }                  /* Every second(!) */
    else if (fields & 16) { *loginterval = 60 * 1000ul; }            /* Every minute */
    else if (fields & 8) { *loginterval = 60 * 60 * 1000ul; }        /* Every hour */
    else if (fields) { *loginterval = 24 * 60 * 60 * 1000ul; }       /* Every day */
    
    return outfp;
}


int CreateTestData(char *buffer, const char *fakeType)
{
    int len = 0;

    if (fakeType != NULL)
    {
        if (fakeType[0] == 't')
        {
            static unsigned short sequence = 0;
            buffer[0] = 0x12;                   // reportType
            buffer[1] = 0x54;                   // reportId
            buffer[2] = 0xff; buffer[3] = 0xff; // deviceId
            buffer[4] = 4;                      // version
            buffer[5] = 4;                      // sampleCount
            buffer[6] = (unsigned char)sequence; buffer[7] = (unsigned char)(sequence>>8); sequence++;  // sequence
            buffer[8] = 0; buffer[9] = 0;       // unsent
            buffer[10] = 0; buffer[11] = 0;     // Temperature
            buffer[12] = 0; buffer[13] = 0;     // Light
            buffer[14] = 0; buffer[15] = 0;     // Battery
            buffer[16] = 0; buffer[17] = 0;     // Humidity
            buffer[18] = 0x00; buffer[19] = 0x00;       // PIR and audio energy (4x 2x 10-bit samples = 80 bits, packed into 10 bytes)
                buffer[20] = 0x00; buffer[21] = 0x00;   // "
                buffer[22] = 0x00; buffer[23] = 0x00;   // "
                buffer[24] = 0x00; buffer[25] = 0x00;   // "
                buffer[26] = 0x00; buffer[27] = 0x00;   // "
            buffer[28] = 0xff; buffer[29] = 0xff;   // parentAddress
            buffer[30] = 0xff; buffer[31] = 0xff;   // parentAltAddress
            len = 32;
        }
    }

    return len;
}


/* Parse SLIP-encoded packets, log or convert to UDP packets */
int waxrec(const char *infile, const char *host, const char *initString, const char *logfile, char tee, char dump, char timetag, char sendOnly, const char *stompHost, const char *stompAddress, const char *stompUser, const char *stompPassword, int writeFromUdp, int timeformat, char convertToOsc, int format, char ignoreInvalid)
{
    #define BUFFER_SIZE 0xffff
    static char buffer[BUFFER_SIZE];
    size_t bufferLength = 0;
    int fd = -1;
    struct sockaddr_in serverAddr;
    SOCKET s = SOCKET_ERROR;
    static char ports[1024];
    SOCKET receiveUdpSocket = SOCKET_ERROR;
    const char *fakeType = NULL;

#ifdef _WIN32
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(1, 1), &wsaData);
    }
#endif

    /* fake input for testing */
    if (infile[0] == '*')
    {
        fakeType = infile + 1;
    }
    else if (infile[0] == 'u' && infile[1] == 'd' && infile[2] == 'p' && infile[3] == ':')
    {
        char *name = (char *)infile + 4;
        int receiveUdpPort = 1234;
        char serverName[64] = "localhost";
        struct sockaddr_in serverAddr;
        //const int recvMaxLength = 65535;
        //unsigned char *recvBuffer;
        //int recvLength = 0;
        struct hostent *hp;

        // Ignore preceding slashes
        while (*name == '/') { name++; }

        gethostname(serverName, 64);

        // Check if interface name is specified
        if (*name != '\0' && *name != ':' && !(*name >= '0' && *name <= '9' && strstr(name, ".") == NULL))
        {
            strcpy(serverName, name);
            // Find and remove port separator
            name = strstr(serverName, ":");
            if (name != NULL) { *name++ = '\0'; }
        }

        // Check if port specified
        if (name != NULL && *name != '\0')
        {
            receiveUdpPort = atoi(name);
        }

        /* Get server host information, name and inet address */
        hp = gethostbyname(serverName);
        if (hp == NULL) { fprintf(stderr, "UDPRECV: Can't resolve host: %s\n", serverName); return 8; }
        memcpy(&(serverAddr.sin_addr), hp->h_addr, hp->h_length);
        serverAddr.sin_family = AF_INET; 
        serverAddr.sin_port = htons(receiveUdpPort);
        fprintf(stderr, "UDPRECV: Address: [%s] = %s : %d\n", hp->h_name, inet_ntoa(serverAddr.sin_addr), receiveUdpPort);
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
        /* Create the socket */
        receiveUdpSocket = socket(AF_INET, SOCK_DGRAM, 0); 
        if (receiveUdpSocket < 0)
        {
            fprintf(stderr, "UDPRECV: ERROR: Socket creation failed (%s)\n", strerrorsocket());
            return 5;
        }
    
        /* Allow rapid reuse of this socket */
        {
            int option = 1;
            setsockopt(receiveUdpSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option));
        }
    
        /* Bind the socket */
        if (bind(receiveUdpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            fprintf(stderr, "UDPRECV: ERROR: Socket bind failed (%s)\n", strerrorsocket());
            return 6;
        }

    }
    else
    {

        /* Search for port */
        if (infile[0] == '!')
        {
            unsigned int vidpid = (DEFAULT_VID << 16) | DEFAULT_PID;
            sscanf(infile + 1, "%08x", &vidpid);
            fprintf(stderr, "Searching for VID=%04X&PID=%04X...\n", (unsigned short)(vidpid >> 16), (unsigned short)vidpid);
            if (FindPorts((unsigned short)(vidpid >> 16), (unsigned short)vidpid, ports, 1024) <= 0)
            {
                fprintf(stderr, "ERROR: No ports found.\n");
                return 3;
            }
            /* Choose the first port found */
            infile = ports;
        }
        else if (infile[0] == '@')
        {
            fprintf(stderr, "Searching for Bluetooth port %s...\n", infile + 1);
            if (FindBluetoothPorts(infile + 1, ports, 1024) <= 0)
            {
                fprintf(stderr, "ERROR: No matching Bluetooth ports found.\n");
                return 3;
            }
            /* Choose the first port found */
            infile = ports;
        }
    
        /* Open the serial port */
        fd = openport(infile, 1);   // (initString != NULL)
        if (fd < 0)
        {
            fprintf(stderr, "ERROR: Port not open.\n");
            return 2;
        }

    }
    
    /* Send initialization string */
    if (initString != NULL && fd != -1)
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
                if (c == 'x')
				{
					if (((p[0] >= '0' && p[0] <= '9') && ((p[0] >= 'a' && p[0] <= 'f') || (p[0] >= 'A' && p[0] <= 'F')))
						&& ((p[1] >= '0' && p[1] <= '9') && ((p[1] >= 'a' && p[1] <= 'f') || (p[1] >= 'A' && p[1] <= 'F'))))
					{
						c = 0;
								
						if (*p >= 'A' && *p <= 'F') { c |= *p - 'A' + 10; }
						else if (*p >= 'a' && *p <= 'f') { c |= *p - 'a' + 10; }
						else { c |= *p - '0'; }
						p++;

						c <<= 4;
						if (*p >= 'A' && *p <= 'F') { c |= *p - 'A' + 10; }
						else if (*p >= 'a' && *p <= 'f') { c |= *p - 'a' + 10; }
						else { c |= *p - '0'; }
						p++;
								
#if defined(_WIN32) && defined(WIN_HANDLE)
                        {
                            int written;
                            WriteFile((HANDLE)fd, &c, 1, &written, 0);
                        }
#else
                        write(fd, &c, 1);
#endif
						continue;
					}
				}
            }
            if (c == '\0') { break; }
#if defined(_WIN32) && defined(WIN_HANDLE)
            {
                int written;
                WriteFile((HANDLE)fd, &c, 1, &written, 0);
            }
#else
            write(fd, &c, 1);
#endif
        }
    }

    if (!sendOnly)
    {

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

        /* Start receiver threads */
#ifdef THREAD_RECEIVE_KEYS
        receiveKeysFD = fd;
        ReceiveKeysStart();
#endif
        if (writeFromUdp != 0)
        {
#ifdef THREAD_WRITE_FROM_UDP
            receiveUdpFD = fd;
            ReceiveUdpStart(writeFromUdp);
#else
            fprintf(stderr, "WARNING: UDP Receive not compiled in.\n");
#endif
        }

        /* Read packets and transmit */
        {
            FILE *outfp = NULL;
            unsigned long logInterval = 0;
            unsigned long long logOpen = 0;
            char text = 1;

            for (;;)
            {
                size_t len = 0;
                unsigned long long now;
                
                if (receiveUdpSocket != SOCKET_ERROR)
                {
                    struct sockaddr_in from;
                    int fromlen = sizeof(from);
        
                    len = recvfrom(receiveUdpSocket, (char *)buffer, BUFFER_SIZE, 0, (struct sockaddr *)&from, &fromlen);
        
                    if (len == 0 || len == SOCKET_ERROR) { fprintf(stderr, "UDPRECV: ERROR: Receive failed (%s)\n", strerrorsocket()); break; }
        
                    //fprintf(stderr, "UDPRECV: Received from %s:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port));

                    /* Check if it appears to be an OSC bundle or OSC packet... */
                    if (len >= 1 && buffer[0] == '/') { fprintf(stderr, "WARNING: OSC packet receive not yet implemented.\n"); }
                    else if (len >= 1 && buffer[0] == '#') { fprintf(stderr, "WARNING: OSC bundle receive not yet implemented.\n"); }

                }
                else if (fd == -1)
                {
                    /* Fake input for testing */
                    sleep(1);
                    len = CreateTestData(buffer, fakeType);
                    text = 0;
                }
                else
                {
                    /* Read data */
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
                }

                /* Get time now */
                now = TicksNow();
                
                /* Close existing log file if interval recording */
                if (logInterval && outfp != NULL && (now / (unsigned long long)logInterval) != (logOpen / (unsigned long long)logInterval))
                {
                    fclose(outfp);
                    outfp = NULL;
                }
                
                /* Open a log file if needed */
                if (outfp == NULL && logfile != NULL)
                {
                    outfp = openlogfile(logfile, now, &logInterval);
                    logOpen = now;
                }
                
                if (receiveUdpSocket == SOCKET_ERROR && fd != -1 && text)
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
                        if (outfp != NULL) { waxDump(waxPacket, outfp, tee, timeformat); }

                        /* Create a STOMP packet */
	                    if (stompTransmitter != NULL)
	                    {
	                        static char msg[2048];
                            char *p = msg;
                            int z;

                            p += sprintf(p, "{");
                            p += sprintf(p, "\"Type\":\"WAX\",");
                            p += sprintf(p, "\"Timestamp\":\"%llu\",", waxPacket->timestamp);
                            p += sprintf(p, "\"DeviceId\":\"%u\",", waxPacket->deviceId);
                            p += sprintf(p, "\"SequenceId\":\"%u\",", waxPacket->sequenceId);
                            p += sprintf(p, "\"SampleCount\":\"%u\",", waxPacket->sampleCount);

                            p += sprintf(p, "\"Samples\":[");
                            for (z = 0; z < waxPacket->sampleCount; z++)
                            {
                                p += sprintf(p, "[%llu,%u,%d,%d,%d]%s", waxPacket->samples[z].timestamp, waxPacket->samples[z].sampleIndex, waxPacket->samples[z].x, waxPacket->samples[z].y, waxPacket->samples[z].z, z + 1 < waxPacket->sampleCount ? "," : "");
                            }
                            p += sprintf(p, "]");

                            p += sprintf(p, "}");
	                        TinyStompTransmitter_Transmit(stompTransmitter, stompAddress, msg, stompUser, stompPassword);
                        }

                        /* Create an OSC bundle from the WAX packet */
                        if (convertToOsc) { len = waxToOsc(waxPacket, buffer, timetag); }
                    }
                }

                /* If it appears to be a binary WAX9 packet... */
                if (len > 1 && buffer[0] == '9')
                {
                    unsigned long long now = TicksNow();
                    Wax9Packet *wax9Packet;
                    if (dump) { hexdump(buffer, len); }
                    wax9Packet = parseWax9Packet(buffer, len, now);
                    if (wax9Packet != NULL)
                    {
                        /* Output text version */
                        if (outfp != NULL) { wax9Dump(wax9Packet, outfp, tee, timeformat, now); }

                        /* Create a STOMP packet */
	                    if (stompTransmitter != NULL)
	                    {
	                        static char msg[2048];
                            char *p = msg;

                            p += sprintf(p, "{");
                            p += sprintf(p, "\"Type\":\"WAX9\",");
                            p += sprintf(p, "\"ReceivedTimestamp\":\"%llu\",", now);
                            if ((wax9Packet->packetVersion & 1) == 0)
                            {
                                p += sprintf(p, "\"Battery\":\"%u\",", wax9Packet->battery);
                                p += sprintf(p, "\"Temperature\":\"%d\",", wax9Packet->temperature);
                                p += sprintf(p, "\"Pressure\":\"%u\",", wax9Packet->pressure);
                            }
                            p += sprintf(p, "\"Samples\":[");
                            p += sprintf(p, "[%llu,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d]", wax9Packet->timestamp, wax9Packet->sampleNumber, 
                                wax9Packet->accel.x, wax9Packet->accel.y, wax9Packet->accel.z, 
                                wax9Packet->gyro.x,  wax9Packet->gyro.y,  wax9Packet->gyro.z, 
                                wax9Packet->mag.x,   wax9Packet->mag.y,   wax9Packet->mag.z 
                                );
                            p += sprintf(p, "]");
                            p += sprintf(p, "}");
	                        TinyStompTransmitter_Transmit(stompTransmitter, stompAddress, msg, stompUser, stompPassword);
                        }

                        /* Create an OSC bundle from the WAX packet */
                        if (convertToOsc) { len = wax9ToOsc(wax9Packet, buffer, timetag, now); }
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
                        if (outfp != NULL) { teddiDump(teddiPacket, outfp, tee, format, ignoreInvalid); }

                        /* Create a STOMP packet */
	                    if (stompTransmitter != NULL)
	                    {
	                        static char msg[2048];
                            char *p = msg;
                            int i;
                            int msPerSample = 1000 / teddiFrequency[(teddiPacket->version >> 4)];
                            
                            p += sprintf(p, "{");
                            p += sprintf(p, "\"Type\":\"TEDDI\",");
                            p += sprintf(p, "\"TimestampReceived\":\"%llu\",", teddiPacket->timestampReceived);
                            p += sprintf(p, "\"TimestampEstimated\":\"%llu\",", teddiPacket->timestampEstimated);
                            p += sprintf(p, "\"DeviceId\":\"%u\",", teddiPacket->deviceId);
                            p += sprintf(p, "\"Version\":\"%u\",", teddiPacket->version);
                            p += sprintf(p, "\"SampleCount\":\"%u\",", teddiPacket->sampleCount);
                            p += sprintf(p, "\"Sequence\":\"%u\",", teddiPacket->sequence);
                            p += sprintf(p, "\"Unsent\":\"%u\",", teddiPacket->unsent);
                            p += sprintf(p, "\"Temp\":\"%u\",", teddiPacket->temp);
                            p += sprintf(p, "\"Light\":\"%u\",", teddiPacket->light);
                            p += sprintf(p, "\"Battery\":\"%u\",", teddiPacket->battery);
                            p += sprintf(p, "\"Humidity\":\"%u\",", teddiPacket->humidity);

                            p += sprintf(p, "\"Samples\":[");
                            for (i = 0; i < teddiPacket->sampleCount; i++)
                            {
                                p += sprintf(p, "[%llu,%u,%u]%s", teddiPacket->timestampEstimated + i * msPerSample, teddiPacket->pirData[i], teddiPacket->audioData[i], i + 1 < teddiPacket->sampleCount ? "," : "");
                            }
                            p += sprintf(p, "],");

                            p += sprintf(p, "\"ParentAddress\":\"%u\",", teddiPacket->parentAddress);
                            p += sprintf(p, "\"ParentAltAddress\":\"%u\",", teddiPacket->parentAltAddress);

                            p += sprintf(p, "}");
	                        TinyStompTransmitter_Transmit(stompTransmitter, stompAddress, msg, stompUser, stompPassword);
                        }

                        /* Create an OSC bundle from the TEDDI packet */
                        if (convertToOsc) { len = teddiToOsc(teddiPacket, buffer, timetag); }
                    }
                }
                
                /* If it appears to be a binary TEDDI packet (USER_REPORT_TYPE, 'S') */
                if (len > 1 && buffer[0] == 0x12 && buffer[1] == 0x53)
                {
                    TeddiStatusPacket *teddiStatusPacket;
                    if (dump) { hexdump(buffer, len); }
                    teddiStatusPacket = parseTeddiStatusPacket(buffer, len, TicksNow());
                    if (teddiStatusPacket != NULL)
                    {
                        /* Output text version */
//                        if (outfp != NULL) { teddiStatusDump(teddiStatusPacket, outfp, tee); }

                        /* Create a STOMP packet */
	                    if (stompTransmitter != NULL)
	                    {
	                        static char msg[2048];
                            char *p = msg;
                            int i;
                            int numNeighbours = 0;

                            p += sprintf(p, "{");
                            p += sprintf(p, "\"Type\":\"TEDDI_Status\",");
                            p += sprintf(p, "\"TimestampReceived\":\"%llu\",", teddiStatusPacket->timestampReceived);
                            p += sprintf(p, "\"DeviceId\":\"%u\",", teddiStatusPacket->deviceId);
                            p += sprintf(p, "\"Version\":\"%u\",", teddiStatusPacket->version);
                            p += sprintf(p, "\"Power\":\"%u\",", teddiStatusPacket->power);
                            p += sprintf(p, "\"Sequence\":\"%u\",", teddiStatusPacket->sequence);
                            p += sprintf(p, "\"ShortAddress\":\"%u\",", teddiStatusPacket->shortAddress);
                            p += sprintf(p, "\"LastLQI\":\"%u\",", teddiStatusPacket->lastLQI);
                            p += sprintf(p, "\"LastRSSI\":\"%u\",", teddiStatusPacket->lastRSSI);
                            p += sprintf(p, "\"ParentAddress\":\"%u\",", teddiStatusPacket->parentAddress);
                            p += sprintf(p, "\"ParentAltAddress\":\"%u\",", teddiStatusPacket->parentAltAddress);

                            if (teddiStatusPacket->version > 0)     // V0 had a broken neighbour table
                            {
                                p += sprintf(p, "\"Neighbours\":[");
                                for (i = 0; i < NUM_COORDINATOR; i++)
                                {
                                    if (teddiStatusPacket->neighbours[i / 8] & (1 << (i & 7)))
                                    {
                                        p += sprintf(p, "%s%u", (numNeighbours ? "," : ""), i);
                                        numNeighbours++;
                                    }
                                }
                                p += sprintf(p, "]");
                            }

                            p += sprintf(p, "}");
	                        TinyStompTransmitter_Transmit(stompTransmitter, stompAddress, msg, stompUser, stompPassword);
                        }

                        /* Create an OSC bundle from the TEDDI status packet */
//                        if (convertToOsc) { len = teddiStatusToOsc(teddiStatusPacket, buffer, timetag); }
                    }
                }
                
                /* If we're keeping packets raw, or if it appears to (now) be an OSC bundle or OSC packet... */
                if (!convertToOsc || (len >= 1 && (buffer[0] == '#' || buffer[0] == '/')))
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

        /* Stop receiver threads */
#ifdef THREAD_WRITE_FROM_KEYS
        ReceiveKeysStop();
#endif
#ifdef THREAD_WRITE_FROM_UDP
        ReceiveUdpStop();
#endif

        /* Close incoming UDP */
        if (receiveUdpSocket != SOCKET_ERROR)
        {
            closesocket(receiveUdpSocket);
        }

        /* Close socket */
        if (s != SOCKET_ERROR) { closesocket(s); }

        /* Close the STOMP port */
	    if (stompTransmitter != NULL)
	    {
		    TinyStompTransmitter_Delete(stompTransmitter);
		    stompTransmitter = NULL;
	    }

    }

    /* Close file */
#if defined(_WIN32) && defined(WIN_HANDLE)
    if (fd != -1 && (HANDLE)fd != INVALID_HANDLE_VALUE) { CloseHandle((HANDLE)fd); }
#else
    if (fd != -1 && fd != fileno(stdin)) { close(fd); }
#endif

#ifdef _WIN32
    WSACleanup();
#endif

    return (s != SOCKET_ERROR) ? 0 : 1;
}


/* Parse command-line parameters */
int main(int argc, char *argv[])
{
    char showHelp = 0;
    int i, argPosition = 0, ret;
    char tee = 0, dump = 0, timetag = 0, sendOnly = 0;
    const char *infile = NULL;
    const char *host = NULL;
    const char *initString = NULL;
    char stompHost[128] = ""; /* "localhost"; */
    char stompAddress[128] = "/topic/OpenMovement.Sensor.Wax";
    char stompUser[128] = "";
    char stompPassword[128] = "";
    const char *logfile = NULL;
    int writeFromUdp = 0;
    int timeformat = 2;
    char convertToOsc = 0;
    int format = 0;
    char ignoreInvalid = 0;

    fprintf(stderr, "WAXREC    WAX Receiver\n");
    fprintf(stderr, "V1.93     by Daniel Jackson, 2011-2013\n");
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
            logfile = "";   /* Empty string will map to stdout */
        }
        else if (strcasecmp(argv[i], "-out") == 0)
        {
            logfile = argv[++i];
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
        else if (strcasecmp(argv[i], "-udpwritefrom") == 0)
        {
            writeFromUdp = atoi(argv[++i]);
        }
        else if (strcasecmp(argv[i], "-osc") == 0)
        {
            host = argv[++i];
            convertToOsc = 1;
        }
        else if (strcasecmp(argv[i], "-raw") == 0 || strcasecmp(argv[i], "-udp") == 0)
        {
            host = argv[++i];
            convertToOsc = 0;
        }
        else if (strcasecmp(argv[i], "-format:short") == 0) { format = 1; }
        else if (strcasecmp(argv[i], "-stomphost") == 0)
        {
            strcpy(stompHost, argv[++i]);
        }
        else if (strcasecmp(argv[i], "-stomptopic") == 0)
        {
            strcpy(stompAddress, argv[++i]);
        }
        else if (strcasecmp(argv[i], "-stompuser") == 0)
        {
            strcpy(stompUser, argv[++i]);
        }
        else if (strcasecmp(argv[i], "-stomppassword") == 0)
        {
            strcpy(stompPassword, argv[++i]);
        }
        else if (strcasecmp(argv[i], "-invalid:ignore") == 0)
        {
            ignoreInvalid = 1;
        }
        else if (strcasecmp(argv[i], "-invalid:label") == 0)
        {
            ignoreInvalid = 2;
        }
        else if (strcasecmp(argv[i], "-t:none") == 0) { timeformat = 0; }
        else if (strcasecmp(argv[i], "-t:secs") == 0) { timeformat = 1; }
        else if (strcasecmp(argv[i], "-t:full") == 0) { timeformat = 2; }
        else if (strcasecmp(argv[i], "-t:both") == 0) { timeformat = 3; }
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
        fprintf(stderr, "        [-udp <hostname>[:<port>]]             Send raw packets over UDP to the specified host/port (cannot be used with -osc)\n");
        fprintf(stderr, "        [-stomphost <hostname>[:<port>] [-stomptopic /topic/Topic] [-stompuser <username>] [-stomppassword <password>]]  Send STOMP to the specified server.\n");
        fprintf(stderr, "        [-init <string> [-exit]]               Send initialzing string; immediately exit.\n");
#ifdef THREAD_WRITE_FROM_UDP
        //fprintf(stderr, "        [-udpwritefrom <port>]               Write data received from the specified UDP port.\n");
#endif
		fprintf(stderr, "        [-invalid:ignore|-invalid:label]       Ingore or label invalid packets\n");
		fprintf(stderr, "        [-t:{none|secs|full|both}]             Timestamp format\n");
		fprintf(stderr, "        [-out <file.csv>]                      Output to a specific log file\n");
        fprintf(stderr, "        [-format:short]                        Format as short output (TEDDI packets only)\n");
        fprintf(stderr, "        [-dump]                                Hex-dump raw packets.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Log example: waxrec %s -log -tee -init \"MODE=1\\r\\n\" > log.csv\n", EXAMPLE_DEVICE);
        fprintf(stderr, "OSC example: waxrec %s -osc localhost:1234 -init \"MODE=1\\r\\n\"\n", EXAMPLE_DEVICE);
        fprintf(stderr, "STOMP example: waxrec %s -stomphost localhost:61613 -stomptopic /topic/Kitchen.Sensor.Wax -init \"MODE=1\\r\\n\"\n", EXAMPLE_DEVICE);
        fprintf(stderr, "Hourly log example: waxrec %s -out /log/@Y-@M-@D/@Y-@M-@D-@h-00-00.csv\n", EXAMPLE_DEVICE);
        fprintf(stderr, "\n");
        fprintf(stderr, "NOTE: 'device' can be 'udp://localhost:1234' to receive over UDP\n");
#ifdef _MSC_VER
        // See 'findPorts()'
        fprintf(stderr, "NOTE: 'device' can be '!' or '![VID+PID]' to automatically find the first matching serial port (default: !%04X%04X)\n", DEFAULT_VID, DEFAULT_PID);
        fprintf(stderr, "      or '@' or '@[MAC-address]' to automatically find the first matching Bluetooth serial port\n");
#endif
        fprintf(stderr, "\n");
        return -1;
    }

    fprintf(stderr, "WAXREC: %s -> %s%s%s%s %s:%s@%s%s\n", (infile == NULL) ? "<stdin>" : infile, host, (tee ? " [tee]" : ""), (dump ? " [dump]" : ""), (timetag ? " [timetag]" : ""), stompUser, strlen(stompPassword) > 0 ? "*" : "", stompHost, stompAddress);
    fprintf(stderr, "INIT: %s\n", initString);

    // The function with the most arguments in the world... (I think a configuration structure might help here)
    ret = waxrec(infile, host, initString, logfile, tee, dump, timetag, sendOnly, stompHost, stompAddress, stompUser, stompPassword, writeFromUdp, timeformat, convertToOsc, format, ignoreInvalid);

#if defined(_WIN32) && defined(_DEBUG)
    if (IsDebuggerPresent()) { fprintf(stderr, "Press [enter] to exit..."); getc(stdin); }
#endif
    return ret;
}



