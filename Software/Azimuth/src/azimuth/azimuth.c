// Azimuth - IMU Processor
// Dan Jackson, 2013
// 

// Debug command line:    ! -init "\r\nRATE X 1 100\r\nMODE 2\r\nSTREAM 3\r\n"


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
	#if _MSC_VER < 1900
		#define snprintf _snprintf    
	#endif

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
    #include <cfgmgr32.h>
    #pragma comment(lib, "setupapi.lib")
    #pragma comment(lib, "advapi32.lib")    /* For RegQueryValueEx() */

    #define _USE_MATH_DEFINES       /* For M_PI */

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
#include <math.h>
#include <sys/timeb.h>
#include <sys/stat.h>


/* USB IDs */
//#define DEFAULT_VIDPID 0x04D8000A           /* USB Vendor/Product ID: WAX */
#define DEFAULT_VIDPID 0x04D80057           /* USB Vendor/Product ID: CWA */


/* Example serial port device path */
#ifdef _WIN32
#define EXAMPLE_DEVICE "\\\\.\\COM3"
#else
#define EXAMPLE_DEVICE "/dev/tty.usbmodem*"
#endif


#include "azimuth/azimuth.h"
#include "azimuth/ahrs.h"
#include "azimuth/mathutil.h"



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

			//printf(">>> %s\n", usbId);

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


/* Open a serial port */
static int OpenPort(const char *infile, char writeable)
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
        if (fd == -1)
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
                fprintf(stderr, "ERROR: Failed to get HANDLE.\n");
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
                    dcbSerialParams.BaudRate = CBR_115200;
                    dcbSerialParams.ByteSize = 8;
                    dcbSerialParams.StopBits = ONESTOPBIT;
                    dcbSerialParams.Parity = NOPARITY;
		            dcbSerialParams.fBinary = TRUE;
		            dcbSerialParams.fDsrSensitivity = FALSE;
                    if (!SetCommState(hSerial, &dcbSerialParams)){
                        fprintf(stderr, "ERROR: SetCommState() failed.\n");
                    };
                }

                timeouts.ReadIntervalTimeout = 0;
                timeouts.ReadTotalTimeoutConstant = 100;
                timeouts.ReadTotalTimeoutMultiplier = 1;
                timeouts.WriteTotalTimeoutConstant = 200;
                timeouts.WriteTotalTimeoutMultiplier = 1;
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


/* Read a line from the device */
static size_t lineRead(int fd, void *inBuffer, size_t len)
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
static size_t slipRead(int fd, void *inBuffer, size_t len)
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
        fprintf(stderr, "WARNING: Unrecognized WAX9 packet (type 0x%02x, version 0x%02x, length %d -- ignoring.\n", buffer[0], buffer[1], len);
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
static const char *MakeTimestamp(unsigned long long ticks, int timeformat)
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


// Initialize
int AzimuthInit(azimuth_t *azimuth, const char *infile, const char *initString, int sampleRate, float beta)
{
	// License
	printf("Azimuth\n");
	printf("Available under a BSD 2-clause license: http://www.openmovement.co.uk/\n");
	printf("Uses Sebastian Madgwick's 2009 code: MadgwickAHRS.c & MahonyAHRS.c\n");
	printf("-- see: http://www.x-io.co.uk/open-source-imu-and-ahrs-algorithms/\n");
	printf("\n");

    azimuth->fd = -1;
    azimuth->deviceMode = 3;
    azimuth->sampleCount = 0;
    azimuth->backgroundThread = NULL;

    if (beta < 0.0f)    // Negative number = use default value
    {
        beta = AZIMUTH_DEFAULT_BETA;
    }

	// Command-line options
    if (infile[0] == '!')
    {
        static char ports[1024];
        unsigned int vidpid = DEFAULT_VIDPID;
        sscanf(infile + 1, "%08x", &vidpid);
        fprintf(stderr, "Using first port matching VID=%04X&PID=%04X...\n", (unsigned short)(vidpid >> 16), (unsigned short)vidpid);
        if (FindPorts((unsigned short)(vidpid >> 16), (unsigned short)vidpid, ports, 1024) <= 0)
        {
            fprintf(stderr, "ERROR: No ports found.\n");
            return 3;
        }
        /* Choose the first port found */
        infile = ports;
        azimuth->deviceMode = 2;
    }
    else if (infile[0] == '@')
    {
        static char ports[1024];
        fprintf(stderr, "Using first port matching %s...\n", infile + 1);
        if (FindBluetoothPorts(infile + 1, ports, 1024) <= 0)
        {
            fprintf(stderr, "ERROR: No ports found.\n");
            return 3;
        }
        /* Choose the first port found */
        infile = ports;
        azimuth->deviceMode = 1;
    }
    else
    {
        azimuth->deviceMode = 1;
    }
    
    /* Open the serial port */
    fprintf(stderr, "Opening: %s\n", infile);
    azimuth->fd = OpenPort(infile, 1);   // (initString != NULL)
    if (azimuth->fd < 0)
    {
        fprintf(stderr, "ERROR: Port not open.\n");
        return 2;
    }

    /* Send initialization string */
    if (initString != NULL && initString[0] != '\0' && azimuth->fd != -1)
    {
        const char *p = initString;
        unsigned char buffer[256];
        int o = 0;
        fprintf(stderr, "Sending: %s\n", initString);
        for (;;)
        {
            char c = *p++;
            if (c == '\\' || c == '/')
            {
                c = *p++; 
                if (c == 'r') { c = '\r'; }
                else if (c == 'n') { c = '\n'; }
                else if (c == '#') { c = '0' + azimuth->deviceMode; }
                else if (c == 'x')
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
								
                        fprintf(stderr, "%c", c);
                        buffer[o++] = c;
						continue;
					}
				}
            }
            if (c == '\0') { break; }
            fprintf(stderr, "%c", c);
            buffer[o++] = c;
        }
        fprintf(stderr, "\n");
#if 1
        {
            // 1 byte write
            int j;
            for (j = 0; j < o; j++)
            {
                AzimuthSend(azimuth, buffer + j, 1);
            }
        }
#else
        // multi-byte write
        AzimuthSend(azimuth, buffer, o);
#endif
        fprintf(stderr, "...done.\n");
    }

    // Init AHRS
    fprintf(stderr, "Initializing AHRS...\n");
    AhrsInit(&azimuth->ahrs, 0, (float)sampleRate, beta);
    azimuth->frequency = (float)sampleRate;

    fprintf(stderr, "Reading...\n");
    return 0;
}


// Reset
void AzimuthReset(azimuth_t *azimuth)
{
    AhrsReset(&azimuth->ahrs);
    azimuth->sampleCount = 1;   // 0 causes different type of reset
}


// Close
void AzimuthClose(azimuth_t *azimuth)
{
    AzimuthStopBackgroundPoll(azimuth);

    /* Close file */
    fprintf(stderr, "Closing...\n");
#if defined(_WIN32) && defined(WIN_HANDLE)
    if (azimuth->fd != -1 && (HANDLE)azimuth->fd != INVALID_HANDLE_VALUE) { CloseHandle((HANDLE)azimuth->fd); }
#else
    if (azimuth->fd != -1 && azimuth->fd != fileno(stdin)) { close(azimuth->fd); }
#endif
    azimuth->fd = -1;
    if (azimuth->outfp != NULL && azimuth->outfp != stdout) { fclose(azimuth->outfp); }
    azimuth->outfp = NULL;
    fprintf(stderr, "...done.\n");
}


void AzimuthUpdate(azimuth_t *azimuth, float *gyro, float *accel, float *mag)
{
    float magData[3];
    float *magPointer = NULL;
    //fprintf(stderr, "== %f,%f,%f,%f,%f,%f\n", accel[0], accel[1], accel[2], gyro[0], gyro[1], gyro[2]);

    // If first run
    if (azimuth->sampleCount == 0)
    {
        int i;
        for (i = 0; i < 15 * 100; i++)
        {
            float zero[3] = { 0.0f, 0.0f, 0.0f };
            AhrsUpdate(&azimuth->ahrs, zero, accel, NULL);
        }
    }

    if (azimuth->sampleCount < 25)
    {
        Point3Copy(mag, azimuth->magMin);
        Point3Copy(mag, azimuth->magMax);
        azimuth->pressureOffset = azimuth->pressure;
    }
    else
    {
        int i;
        if (mag != NULL)
        {
            for (i = 0; i < 3; i++)
            {
                if (mag[i] > azimuth->magMax[i]) { azimuth->magMax[i] = mag[i]; }
                if (mag[i] < azimuth->magMin[i]) { azimuth->magMin[i] = mag[i]; }
            }
        }
    }

    azimuth->sampleCount++;

    // Copy Input
    Point3Copy(accel, azimuth->accel);
    Point3Copy(gyro, azimuth->gyro);
    Point3Copy(mag, azimuth->mag);

    // Check for magnetometer data
    if (mag != NULL)
    {
        int i;
        float range[3];
        float mean = 0;
        char ok = 0;

        for (i = 0; i < 3; i++)
        {
            magData[i] = mag[i] - (azimuth->magMax[i] + azimuth->magMin[i]) / 2;
            range[i] = azimuth->magMax[i] - azimuth->magMin[i];
            mean += range[i];
        }
        mean /= 3;

        ok = 1;
        for (i = 0; i < 3; i++)
        {
            float unit = range[i] / 2;
            // Ranges between 25-65 uT.  31.869 at 0 lat/0 long.  Around 36/37 locally? 
            if (unit < 22.0 || unit > 68.0f) { ok = 0; }    // Out of range
            if (fabs(range[i] - mean) > 0.10f * mean) { ok = 0; }    // More than 10% deviation
        }

		// Z-axis is inverted on magnetometer
//		mag[2] = -mag[2];

//        printf("{%f, %f, %f} %s\n", azimuth->magMax[0] - azimuth->magMin[0], azimuth->magMax[1] - azimuth->magMin[1], azimuth->magMax[2] - azimuth->magMin[2], ok ? "OK" : "fail");

        if (ok) { magPointer = magData; }
    }

magPointer = NULL;  // Always ignore the magnetometer
	
	// Mag reporting
	{
		static char wasUsing = 0;
		if (magPointer != NULL && !wasUsing) { printf("MAGNETOMETER: Valid"); }
		else if (magPointer == NULL && wasUsing) { printf("MAGNETOMETER: Not valid"); }
		wasUsing = (magPointer == NULL) ? 0 : 1;
	}

    // Update the IMU
    AhrsUpdate(&azimuth->ahrs, gyro, accel, magPointer);

    // Calculate the output
    {
        // Quaternion
        QuaternionCopy(AhrsGetQuaternion(&azimuth->ahrs), azimuth->quat);

        // Euler
        QuaternionToEuler(azimuth->quat, azimuth->euler);                           // (psi, theta, phi)

        // Yaw, pitch, roll angles;  gravity relative to device
        QuaternionToYawPitchRoll(azimuth->quat, azimuth->ypr, azimuth->gravity);    // (yaw, pitch, roll)

	    // Swap axes to match output
        if (1)
	    { 
		    float t = azimuth->gravity[0];
		    azimuth->gravity[0] = -azimuth->gravity[1];
		    azimuth->gravity[1] = -azimuth->gravity[2];
		    azimuth->gravity[2] = -t;
	    }

        // Calculate relative acceleration for the device after gravity is removed
        {
#if 0

!!! Untested

			float gravity[3];

			// Calculate expected gravity vector
			gravity[0] = 2 * (q[1] * q[3] - q[0] * q[2]);
			gravity[1] = 2 * (q[0] * q[1] + q[2] * q[3]);
			gravity[2] = (q[0] * q[0]) - (q[1] * q[1]) - (q[2] * q[2]) + (q[3] * q[3]);

			// Remove expected gravity vector
			azimuth->relAccel[0] = azimuth->accel[0] - gravity[0];
			azimuth->relAccel[1] = azimuth->accel[1] - gravity[1];
			azimuth->relAccel[2] = azimuth->accel[2] - gravity[2];

#else
            float point[3];

		    MatrixLoadZXYInverseRotation(azimuth->euler[2], azimuth->euler[1], azimuth->euler[0], azimuth->matrixInvRotation);

            // (Optional) Recalculate gravity direction:
		    point[0] = 0.0f; point[1] = -1.0f; point[2] = 0.0f;
    //        		Point3MultiplyMatrix(point, azimuth->matrixInvRotation, azimuth->gravity);

            // Re-order (YZX)
		    azimuth->relAccel[0] = azimuth->accel[1] + azimuth->gravity[0];
		    azimuth->relAccel[1] = azimuth->accel[2] + azimuth->gravity[1];
		    azimuth->relAccel[2] = azimuth->accel[0] + azimuth->gravity[2];
#endif

            // Output
            //fprintf(outfp, "RA,%+f,%+f,%+f\n", azimuth->relAccel[0], azimuth->relAccel[1], azimuth->relAccel[2]);
        }

        // Calculate the linear acceleration in world coordinates
        {
			float point[3];

			//QuaternionToMatrix(azimuth->quat, azimuth->matrixRotation);   // TODO: To use this, need to swap axes? (0=1, 1=2, 2=0)
            MatrixLoadZXYRotation(-azimuth->euler[2], -azimuth->euler[1], -azimuth->euler[0], azimuth->matrixRotation);

            point[0] = azimuth->relAccel[0];
            point[1] = azimuth->relAccel[1];
            point[2] = azimuth->relAccel[2];

            Point3MultiplyMatrix(point, azimuth->matrixRotation, azimuth->linearAccel);                
        }

        // Calculate the screen/gravity-relative acceleration
        {
            float gravity[3];
            float pointYin[3], pointYout[3];
            float pointXin[3], pointXout[3], pointZin[3], pointZout[3];
            float sideToSide[3];

            // Gravity
            gravity[0] = 0.0f; gravity[1] = 1.0f; gravity[2] = 0.0f; 

            // Through-screen vector
            pointYin[0] = 0.0f;
            pointYin[1] = 1.0f;
            pointYin[2] = 0.0f;
            Point3MultiplyMatrix(pointYin, azimuth->matrixRotation, pointYout);

            // Cross-product with gravity for 'side-to-side' vector
            Point3CrossProduct(gravity, pointYout, sideToSide);

            // Potential side movement
            pointXin[0] = 1.0f;
            pointXin[1] = 0.0f;
            pointXin[2] = 0.0f;
            Point3MultiplyMatrix(pointXin, azimuth->matrixRotation, pointXout);
            pointZin[0] = 0.0f;
            pointZin[1] = 0.0f;
            pointZin[2] = 1.0f;
            Point3MultiplyMatrix(pointZin, azimuth->matrixRotation, pointZout);

            // Component along side-to-side vector
            azimuth->screen[0] = Point3DotProduct(azimuth->linearAccel, sideToSide);
            // Component along up-down (gravity) vector
            azimuth->screen[1] = Point3DotProduct(azimuth->linearAccel, gravity);
            // Component along in-out (screen) vector
            azimuth->screen[2] = Point3DotProduct(azimuth->linearAccel, pointYout);
        }

        // Output
        if (azimuth->outfp != NULL) fprintf(azimuth->outfp, "YPR,%f,%f,%f\n", azimuth->ypr[0], azimuth->ypr[1], azimuth->ypr[2]);

    }

    if (azimuth->updateCallback != NULL)
    {
        azimuth->updateCallback(azimuth, azimuth->updateCallbackReference);
    }

}

// Update callback function
void AzimuthSetCallback(azimuth_t *azimuth, azimuth_update_callback_t callback, void *reference)
{
    azimuth->updateCallback = callback;
    azimuth->updateCallbackReference = reference;
}


int AzimuthPoll(azimuth_t *azimuth)
{
    #define BUFFER_SIZE 0xffff
    static char buffer[BUFFER_SIZE];
    size_t bufferLength = 0;
    size_t len = 0;
    unsigned long long now;
    int inputs[6];
    char text = 1;
	int cols = 0;
	char inputsOk = 0;
                
    /* Read data */
    if (text)
    { 
        len = lineRead(azimuth->fd, buffer, BUFFER_SIZE); 
        if (len == (size_t)-1)
        { 
            text = 0;
        } 
        if (len == 0) { return 0; } 
    }
    if (!text)
    { 
        len = slipRead(azimuth->fd, buffer, BUFFER_SIZE);
        if (len == 0) { return 0; } 
    }

    /* Get time now */
    now = TicksNow();

	if (text)
	{
		const char *p;
		cols = 1;
		for (p = buffer; *p != '\0'; p++)
		{
			if (*p == ',') { cols++; }
		}

		if (cols == 10 || cols == 14)
		{
			if (sscanf(buffer, "%*d,%d,%d,%d,%d,%d,%d", &inputs[0], &inputs[1], &inputs[2], &inputs[3], &inputs[4], &inputs[5]) == 6)
			{
				inputsOk = 1;
			}
		}
		else if (cols == 9)
		{
			if (sscanf(buffer, "%d,%d,%d,%d,%d,%d", &inputs[0], &inputs[1], &inputs[2], &inputs[3], &inputs[4], &inputs[5]) == 6)
			{
				inputsOk = 1;
			}
		}
	}

	if (inputsOk)
	{
        float accel[3], gyro[3];

        // Convert to units (g & rad/s)
        accel[0] = inputs[0] / 4096.0f;
        accel[1] = inputs[1] / 4096.0f;
        accel[2] = inputs[2] / 4096.0f;
        gyro[0] = inputs[3] * 0.07f * (float)M_PI / 180;
        gyro[1] = inputs[4] * 0.07f * (float)M_PI / 180;
        gyro[2] = inputs[5] * 0.07f * (float)M_PI / 180;

        AzimuthUpdate(azimuth, gyro, accel, NULL);

//fprintf(stdout, "[T] %s\n", buffer);
        // Debug out
        //fprintf(stderr, "== %s\n", buffer);
        //fprintf(stderr, "== %d,%d,%d,%d,%d,%d\n", inputs[0], inputs[1], inputs[2], inputs[3], inputs[4], inputs[5]);
	}
	else if (!text)
	{
        Wax9Packet *packet = parseWax9Packet(buffer, len, now);

        if (packet != NULL)
        {
            float accel[3], gyro[3], mag[3];

            // Convert to units (g & rad/s)
            accel[0] = packet->accel.x / 4096.0f;
            accel[1] = packet->accel.y / 4096.0f;
            accel[2] = packet->accel.z / 4096.0f;
            gyro[0] = packet->gyro.x * 0.07f * (float)M_PI / 180;
            gyro[1] = packet->gyro.y * 0.07f * (float)M_PI / 180;
            gyro[2] = packet->gyro.z * 0.07f * (float)M_PI / 180;
            mag[0] = packet->mag.x * 0.10f; // Ranges between 25-65 uT.  31.869 at 0 lat/0 long.  Around 37 locally? 
            mag[1] = packet->mag.y * 0.10f;
            mag[2] = packet->mag.z * 0.10f;

//fprintf(stdout, "[B] %02x %02x %02x %02x...\n", buffer[0], buffer[1], buffer[2], buffer[3]);
            // Debug out
            //fprintf(stderr, "== %s\n", buffer);
            //fprintf(stderr, "== %d,%d,%d,%d,%d,%d\n", inputs[0], inputs[1], inputs[2], inputs[3], inputs[4], inputs[5]);

            if (packet->temperature != (short)0xffff) { azimuth->temperature = packet->temperature; }
            if (packet->pressure != 0xfffffffful) { azimuth->pressure = packet->pressure; }
            if (packet->battery != 0xffff) { azimuth->battery = packet->battery; }


			// Bit of a hack -- make this nice
			static int lastSample = -1;
			if (lastSample == -1) { lastSample = packet->sampleNumber - 1; }
			int elapsed = (unsigned short)(packet->sampleNumber - lastSample);	// 16-bit wrap-around
			lastSample = packet->sampleNumber;

			if (elapsed == 0) { fprintf(stderr, "WARNING: Duplicate packet (%d).\n", packet->sampleNumber); }
			else if (elapsed > 1) { fprintf(stderr, "WARNING: Dropped packets detected: %d\n", elapsed - 1); }

			// Catch-up
			{
				int z;
				if (elapsed > 10) { elapsed = 10; }	// Limit to catch-up
				for (z = 0; z < elapsed; z++)
				{
					AzimuthUpdate(azimuth, gyro, accel, mag);
				}
			}

        }

	}
    else
    {
        if (azimuth->outfp != NULL) fprintf(azimuth->outfp, "??? %s\n", buffer);
    }

    return 1;
}




thread_return_t AzimuthBackgroundPoll(void *arg)
{
    azimuth_t *azimuth = (azimuth_t *)arg;

    // Update
    while (!azimuth->quitBackgroundPoll)
    {
        if (!AzimuthPoll(azimuth)) 
        { 
            break; 
        }
    }
 
    return thread_return_value(0);
}


// Start background polling
void AzimuthStartBackgroundPoll(azimuth_t *azimuth)
{
    azimuth->quitBackgroundPoll = 0;
    thread_create(&azimuth->backgroundThread, NULL, AzimuthBackgroundPoll, azimuth);
}


// Stop background polling
void AzimuthStopBackgroundPoll(azimuth_t *azimuth)
{
    if (azimuth->backgroundThread == NULL) { return; }
    azimuth->quitBackgroundPoll = 1;
    thread_cancel(&azimuth->backgroundThread);     // thread_join(&azimuth.backgroundThread, NULL);
    azimuth->backgroundThread = NULL;
}


// Send command
int AzimuthSend(azimuth_t *azimuth, const unsigned char *buffer, size_t length)
{
    size_t total;

    if (buffer == NULL) { return 0; }
    if (azimuth->fd == -1) { return 0; }

    for (total = 0; (int)total < (int)length; )
    {
        int num = length - total;
// Tweak this?
//if (azimuth->deviceMode == 1 && num > 1) { num = 1; }
#if defined(_WIN32) && defined(WIN_HANDLE)
//fwrite(buffer + total, 1, num, stdout);
        WriteFile((HANDLE)azimuth->fd, buffer + total, num, (DWORD *)&num, 0);
#else
        num = write(azimuth->fd, buffer + total, num);
#endif
        if (num == 0) { return 0; }     // Problem writing
        total += num;
    }
    return 1;
}
