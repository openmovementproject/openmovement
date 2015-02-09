// AHRS Command-Line Container
// Dan Jackson, 2013
// 

// TODO: OSC output?

// Debug command line:    ! -init "\r\nRATE X 1 100\r\nMODE 2\r\nSTREAM 3\r\n"


/* Cross-platform alternatives */
#ifdef _WIN32

    /* Defines and headers */
    #define _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_DEPRECATE
    
    /* Strings */
    #define strcasecmp _stricmp
    #define snprintf _snprintf    

    #include <windows.h>

#endif


/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Example serial port device path */
#ifdef _WIN32
#define EXAMPLE_DEVICE "\\\\.\\COM3"
#else
#define EXAMPLE_DEVICE "/dev/tty.usbmodem*"
#endif


#include "azimuth/azimuth.h"


/* Parse command-line parameters */
int main(int argc, char *argv[])
{
    char showHelp = 0;
    int i, argPosition = 0, ret = -1;
    const char *initString = NULL;
    const char *infile = NULL;
    azimuth_t azimuth = {0};

    fprintf(stderr, "Azimuth   AHRS Translation\n");
    fprintf(stderr, "V1.00     by Daniel Jackson, 2013\n");
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
        else if (strcasecmp(argv[i], "-init") == 0)
        {
            initString = argv[++i];
        }
        else if (strcasecmp(argv[i], "-in") == 0)
        {
            ;
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
        fprintf(stderr, "Usage:  azimuth <device>\n");
        fprintf(stderr, "        [-init <string>]                       Send initializing string\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Example: azimuth %s -init \"\\r\\nRATE X 1 100\\r\\nMODE 2\\r\\nSTREAM 3\\r\\n\"\n", EXAMPLE_DEVICE);
        fprintf(stderr, "\n");
#ifdef _WIN32
        fprintf(stderr, "NOTE: 'device' can be '!' or '![VID+PID]' to automatically find the first matching serial port\n");
#endif
        fprintf(stderr, "\n");
        return -1;
    }

    if (AzimuthInit(&azimuth, infile, initString, 100, -1.0f) == 0)
    {
        do
        {
            ret = AzimuthPoll(&azimuth);
fprintf(stdout, "%f,%f,%f,%f,%f,%f\n", azimuth.accel[0], azimuth.accel[1], azimuth.accel[2], azimuth.gyro[0], azimuth.gyro[1], azimuth.gyro[2]);
        } while (ret);
        AzimuthClose(&azimuth);
    }

#if defined(_WIN32) && defined(_DEBUG)
    if (IsDebuggerPresent()) { fprintf(stderr, "Press [enter] to exit..."); getc(stdin); }
#endif
    return ret;
}

