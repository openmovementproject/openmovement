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
 * UDP Send
 * by Daniel Jackson, 2012
 */



/* Cross-platform alternatives */
#ifdef _WIN32

    /* Defines and headers */
    #define _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_DEPRECATE
    #include <windows.h>
    #include <io.h>
    
    /* Strings */
    #define strcasecmp _stricmp
    #define snprintf _snprintf    

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

#else

    /* Sockets */
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
#include <time.h>
#include <sys/timeb.h>


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


/* Returns the number of milliseconds since the epoch */
unsigned long long TicksNow(void)
{
    struct timeb tp;
    ftime(&tp);
    return (unsigned long long)tp.time * 1000 + tp.millitm;
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


/* Parse command-line parameters */
int main(int argc, char *argv[])
{
    int ret = 0;
    
    fprintf(stderr, "UDPSEND   Simple UDP/OSC Transmitter\n");
    fprintf(stderr, "V1.00     by Daniel Jackson, 2012\n");
    fprintf(stderr, "\n");

    /* Check for invalid arguments */
    if (argc <= 2 || argv[1][0] == '-' || argv[1][0] == '/')
    {
        fprintf(stderr, "Usage:   udpsend <host:port> /<osc-path> [<int-value>...]\n");
        fprintf(stderr, "         udpsend <host:port> <raw-message>\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Example: udpsend localhost:3333 /transmit/100 1 2 3\n");
        fprintf(stderr, "         udpsend localhost:3333 TRANSMIT 100 1 2 3\\r\\n\n");
        fprintf(stderr, "\n");
        ret = -1;
    }
    else
    {
        #define BUFFER_SIZE 0xffff
        static char buffer[BUFFER_SIZE];
        const char *host = NULL;
        struct sockaddr_in serverAddr;
        SOCKET s = SOCKET_ERROR;
        int o = 0;
        int i;

        /* Host */
        host = argv[1];
            
        /* If path sent, then format as an OSC packet */
        if (argv[2][0] == '/')
        {
            int num;
            
            /* Number of parameters */
            num = argc - 3;
            if (num < 0) { num = 0; }
            
            /* Address */
            o += write_osc_string(buffer + o, argv[2]);                         /* [OSC string] address, starting with '/' */
            
            /* Type tag */
            {
                char type[64];
                
                type[0] = ',';
                for (i = 0; i < num; i++)
                {
                    type[i + 1] = 'i';
                }
                type[num + 1] = '\0';
                
                o += write_osc_string(buffer + o, type);                        /* [OSC string] type tag */
            }
            
            /* Parameters (all integers) */
            for (i = 0; i < num; i++)
            {
                int value = atoi(argv[3 + i]);
                o += write_osc_int(buffer + o, value);                          /* [OSC int] value <4 bytes> */
            }
        }
        else
        {
            /* ...otherwise, send a plain text in the UDP packet */
            for (i = 2; i < argc; i++)
            {
                const char *p = argv[i];
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
                    buffer[o++] = c;
                }
                if (i + 1 < argc) { buffer[o++] = ' '; }
            }
    
        }
        
        /* Send data */
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
            else
            {
                fprintf(stderr, "ERROR: Problem opening socket: %s\n", host);
                ret = 1;
            }
        }

        /* Dump */
        hexdump(buffer, o);
        
        /* Transmit */
        if (s != SOCKET_ERROR)
        {
            size_t tlen;
            tlen = transmit(s, &serverAddr, buffer, o);
            if (tlen != o) 
            { 
                fprintf(stderr, "WARNING: Problem transmitting: %d / %d\n", (unsigned int)tlen, (unsigned int)o); 
                ret = 1;
            }
        }

        /* Close socket */
        if (s != SOCKET_ERROR) { closesocket(s); }

#ifdef _WIN32
        WSACleanup();
#endif
    }

#if defined(_WIN32) && defined(_DEBUG)
    if (IsDebuggerPresent()) { fprintf(stderr, "Press [enter] to exit..."); getc(stdin); }
#endif
    return ret;
}


