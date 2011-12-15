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
 * SLIP-to-UDP converter (designed as a USB-Serial-OSC to UDP-OSC converter)
 * by Daniel Jackson, 2011
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


/* Debug dumps a buffer */
static void hexdump(void *buffer, size_t length)
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


/* Convert SLIP-encoded packets into UDP packets */
int sliptoudp(const char *infile, const char *host, char dump)
{
    #define BUFFER_SIZE 0xffff
    static char buffer[BUFFER_SIZE];
    size_t bufferLength = 0;
    int fd;
    struct sockaddr_in serverAddr;
    SOCKET s = SOCKET_ERROR;

    fd = openport(infile, 0);

#ifdef _WIN32
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(1, 1), &wsaData);
    }
#endif

    /* Open UDP socket */
    if (host != NULL)
    {
        s = opensocket(host, 1234, &serverAddr);
    }

    /* Read packets and transmit */
    {
        size_t len;
        while ((len = slipread(fd, buffer, BUFFER_SIZE)) != 0)
        {
            if (dump) { hexdump(buffer, len); }
            if (s != SOCKET_ERROR)
            { 
                size_t tlen = transmit(s, &serverAddr, buffer, len);
                if (tlen != len) { fprintf(stderr, "WARNING: Problem transmitting: %d / %d\n", (unsigned int)tlen, (unsigned int)len); }
            }
        }
    }

    /* Close socket */
    if (s != SOCKET_ERROR) { closesocket(s); }

    /* Close file */
    if (fd != fileno(stdin)) { close(fd); }

#ifdef _WIN32
    WSACleanup();
#endif

    return (s != SOCKET_ERROR) ? 0 : 1;
}


/* Parse command-line parameters */
int main(int argc, char *argv[])
{
    char showHelp = 0;
    int i, argPosition = 0;
    char dump = 0;
    const char *infile = NULL;
    const char *host = NULL;

    fprintf(stderr, "SLIP2UDP  SLIP-to-UDP Converter\n");
    fprintf(stderr, "V1.20     by Daniel Jackson, 2011\n");
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
        else if (strcasecmp(argv[i], "-in") == 0)
        {
            infile = argv[++i];
        }
        else if (strcasecmp(argv[i], "-dump") == 0)
        {
            dump = 1;
        }
        else if (argv[i][0] != '-' && argPosition == 0)
        {
            argPosition++;
            host = argv[i];
        }
        else
        {
            fprintf(stderr, "ERROR: Unknown parameter: %s\n", argv[i]);
            showHelp = 1;
        }
    }

    if (host == NULL) 
    { 
        fprintf(stderr, "ERROR: Host not specified.\n");
        showHelp = 1; 
    }

    if (showHelp)
    {
        fprintf(stderr, "Usage:   slip2udp [-in <device>] <hostname>[:<port>] [-dump]\n");
        fprintf(stderr, "\n");
#ifdef _WIN32
        fprintf(stderr, "Example: slip2udp -in \\\\.\\COM1 localhost:1234\n");
#else
        fprintf(stderr, "Example: slip2udp -in /dev/tty.usbmodem* localhost:1234\n");
#endif
        fprintf(stderr, "\n");
        return -1;
    }

    fprintf(stderr, "SLIP2UDP: %s -> %s%s\n", (infile == NULL) ? "<stdin>" : infile, host, (dump ? " [dump]" : ""));

    return sliptoudp(infile, host, dump);
}

