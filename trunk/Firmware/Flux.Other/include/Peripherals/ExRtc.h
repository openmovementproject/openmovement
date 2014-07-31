// External I2C Rtc Karim Ladha 2013
#ifndef EX_RTC_H
#define EX_RTC_H

#include "Rtc.h"
// Note: Include the other rtc.h to access macros etc. 
// DateTime - a single long integer date/time value - compaible with internal rtc code from flux
/*
    typedef union
    {
        unsigned long value;                // 32-bit
        struct
        {
            unsigned char year    : 6;      // Wraps every 64 years
            unsigned char month   : 4;      // 1-12
            unsigned char day     : 5;      // 1-31
            unsigned char hours   : 5;      // 0-23
            unsigned char minutes : 6;      // 0-59
            unsigned char seconds : 6;      // 0-59
        };
    } DateTime;
*/
extern char exRtcPresent;
// Read device ID - FIRST, checks for response
unsigned char ExRtcVerifyDeviceId(void);
// Initialise
void ExRtcStartup(void);
// Read time
DateTime ExRtcReadTime(void);
// Write time
void ExRtcWriteTime(DateTime time);

#endif

