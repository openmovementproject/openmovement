// External I2C Rtc Karim Ladha 2013

// Includes
#include "HardwareProfile.h"

// Alternate I2C bus?
#ifdef EXRTC_ALTERNATE_I2C
	#define MY_I2C_OVERIDE EXRTC_ALTERNATE_I2C
#endif
#define USE_AND_OR
#include "Peripherals/myI2C.h"

// Types
typedef unsigned long DateTime;

// Globals
char 	exRtcPresent = 0;
//                                         - Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
const unsigned char maxDaysPerMonth[12+1] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Macros
#define FromBcd(_value) ((((_value) & 0xf0) >> 4) * 10 + ((_value) & 0x0f))
#define ToBcd(_value) ((((_value) / 10) << 4) | ((_value) % 10))
#define BadBcd(_v) ((_v) > 0x99 || ((_v) & 0x0f) > 0x09)
#define DATETIME_FROM_YMDHMS(_year, _month, _day, _hours, _minutes, _seconds) ( (((unsigned long)(_year % 100) & 0x3f) << 26) | (((unsigned long)(_month) & 0x0f) << 22) | (((unsigned long)(_day) & 0x1f) << 17) | (((unsigned long)(_hours) & 0x1f) << 12) | (((unsigned long)(_minutes) & 0x3f) <<  6) | ((unsigned long)(_seconds) & 0x3f) )
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

// I2C register addresses       addr    //    b7  /   b6  /   b5  /   b4  /   b3  /   b2  /   b1  /   b0  
#define RTC_ADDR_CONTROL_1      0x00    //   test /   0   /  stop /   0   /  test /   0   /   0   /   0   
#define RTC_ADDR_CONTROL_2      0x01    //    0   /   x   /   0   / TI/TP /   AF  /   TF  /  AIE  /  TIE  
#define RTC_ADDR_SECONDS        0x02    //    VL  /   40  /   20  /   10  /   8   /   4   /   2   /   1   
#define RTC_ADDR_MINUTES        0x03    //    x   /   40  /   20  /   10  /   8   /   4   /   2   /   1   
#define RTC_ADDR_HOURS          0x04    //    x   /   x   /   20  /   10  /   8   /   4   /   2   /   1   
#define RTC_ADDR_DAYS           0x05    //    x   /   x   /   20  /   10  /   8   /   4   /   2   /   1   
#define RTC_ADDR_WEEKDAYS       0x06    //    x   /   x   /   x   /   x   /   x   /   4   /   2   /   1   
#define RTC_ADDR_MONTHS_CENTURY 0x07    //    C   /   x   /   x   /   10  /   8   /   4   /   2   /   1   
#define RTC_ADDR_YEARS          0x08    //    80  /   40  /   20  /   10  /   8   /   4   /   2   /   1   
#define RTC_ADDR_MINUTE_ALARM   0x09    //    AE  /   40  /   20  /   10  /   8   /   4   /   2   /   1   
#define RTC_ADDR_HOUR_ALARM     0x0a    //    AE  /   x   /   20  /   10  /   8   /   4   /   2   /   1   
#define RTC_ADDR_DAY_ALARM      0x0b    //    AE  /   x   /   20  /   10  /   8   /   4   /   2   /   1   
#define RTC_ADDR_WEEKDAY_ALARM  0x0c    //    AE  /   x   /   x   /   x   /   x   /   4   /   2   /   1   
#define RTC_ADDR_CLKOUT_FREQ    0x0d    //    FE  /   x   /   x   /   x   /   x   /   x   /  FD1  /  FD0  
#define RTC_ADDR_TIMER_CONTROL  0x0e    //    TE  /   x   /   x   /   x   /   x   /   x   /  TD1  /  TD0  
#define RTC_ADDR_TIMER          0x0f    //   128  /   64  /   32  /   16  /   8   /   4   /   2   /   1   


// I2C Definitions
#define LOCAL_I2C_RATE		I2C_RATE_100kHZ
#define RTC_ADDRESS			0xA2	/*I2C address*/

// I2C 
#define CUT_DOWN_I2C_CODE_SIZE
#ifndef CUT_DOWN_I2C_CODE_SIZE
#define ExRtcOpen()              myI2COpen();myI2CStart(); WaitStartmyI2C();
#define ExRtcAddressRead(_r)     myI2Cputc(RTC_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(RTC_ADDRESS | I2C_READ_MASK);
#define ExRtcAddressWrite(_r)    myI2Cputc(RTC_ADDRESS); myI2Cputc((_r)); 
#define ExRtcReadContinue()      myI2Cgetc(); myI2CAck()
#define ExRtcReadLast()          myI2Cgetc(); myI2CNack()
#define ExRtcWrite(_v)           myI2Cputc((_v));
#define ExRtcClose()             myI2CStop(); WaitStopmyI2C();myI2CClose();
#define ExRtcReopen()            myI2CRestart(); WaitRestartmyI2C();
#else
void ExRtcOpen(void)						{myI2COpen();myI2CStart(); WaitStartmyI2C();}
void ExRtcAddressRead(unsigned char _r)		{myI2Cputc(RTC_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(RTC_ADDRESS | I2C_READ_MASK);}
void ExRtcAddressWrite(unsigned char _r)	{myI2Cputc(RTC_ADDRESS); myI2Cputc((_r)); }
unsigned char ExRtcReadContinue(void)   	{unsigned char ret = myI2Cgetc(); myI2CAck();return ret;}
unsigned char ExRtcReadLast(void)       	{unsigned char ret = myI2Cgetc(); myI2CNack();return ret;}
void ExRtcWrite(unsigned char _v)       	{myI2Cputc((_v));}
void ExRtcClose(void )             			{myI2CStop(); WaitStopmyI2C();myI2CClose();}
void ExRtcReopen(void )            			{myI2CRestart(); WaitRestartmyI2C();}
#endif

// Code

// Returns whether a specified date/time is valid
char RtcIsValid(DateTime value)
{
	unsigned char year;
	unsigned char month;
	unsigned char day;

	// Any year valid (0-63 = 2000-2063)
	year = DATETIME_YEAR(value);		// +2000

	// Months 1-12 valid
	month = DATETIME_MONTH(value);
	if (month == 0 || month > 12) { return 0; }

	// Days 1-31 valid
	day = DATETIME_DAY(value);
	if (day == 0 || day > maxDaysPerMonth[month]) { return 0; }
	if (month == 2 && day == 29 && (year & 3) != 0) { return 0; }	// Fail 29-Feb in non-leap years

	// Hours 0-23 valid
	if (DATETIME_HOURS(value) > 23) { return 0; }

	// Minutes 0-59 valid
	if (DATETIME_MINUTES(value) > 59) { return 0; }

	// Seconds 0-59 valid
	if (DATETIME_SECONDS(value) > 59) { return 0; }

	// Date valid
    return 1;
}

// Read device ID
unsigned char ExRtcVerifyDeviceId(void)
{
	ExRtcOpen();	
	ExRtcAddressWrite(RTC_ADDR_CONTROL_1);
	exRtcPresent = myAckStat(); // Did it ack?
	ExRtcWrite(0x00); // Ctrl reg 1 - on
	ExRtcWrite(0x00); // Ctrl reg 2
	ExRtcClose();
	return exRtcPresent;
}

// Initialise
void ExRtcStartup(void)
{
	if(!exRtcPresent)return;

	ExRtcOpen();	
	ExRtcAddressWrite(RTC_ADDR_MINUTE_ALARM);
	ExRtcWrite(0x00); // Minute alarm off
	ExRtcWrite(0x00); // Hour alarm off
	ExRtcWrite(0x00); // Day alarm off
	ExRtcWrite(0x00); // Weekday alarm off
	ExRtcWrite(0x00); // Clockout off
	ExRtcWrite(0x00); // Timer ctrl off
	ExRtcWrite(0x00); // Timer
	ExRtcClose();
	return;
}

// Read time
DateTime ExRtcReadTime(void)
{
    unsigned char i, values[6], rtcBadTime = 0;
	DateTime ret;
	
	if(!exRtcPresent) return DATETIME_INVALID;
	
	// Read time
	ExRtcOpen();
	ExRtcAddressRead(RTC_ADDR_SECONDS);
	values[0] = ExRtcReadContinue(); 
	if(values[0] & 0x80) rtcBadTime |= 0x80;// Low voltage bit set
	values[0] &= 0x7f; 						// Seconds
	values[1] = 0x7f & ExRtcReadContinue(); // Minutes
	values[2] = 0x3f & ExRtcReadContinue(); // Hours
	values[3] = 0x3f & ExRtcReadContinue(); // Days
	ExRtcReadContinue(); // Weekday (discarded)

	values[4] = 0x1f & ExRtcReadContinue(); // Month
	values[5] = ExRtcReadLast(); 		// Year
	ExRtcClose();
	
	// Check for bad BCD vals
	for(i=0;i<6;i++)
	{
		if (BadBcd(values[i])) { rtcBadTime |= (1<<i); }
		values[i] = FromBcd(values[i]); // Convert to binary
	}

	// Make integer representation
	ret = DATETIME_FROM_YMDHMS(values[5], values[4], values[3], values[2], values[1], values[0]);
	
	// Check it is valid
	if((ret <= DATETIME_MIN) || (ret > DATETIME_MAX)) rtcBadTime |= 0x40;
	if(!RtcIsValid(ret)) rtcBadTime |= 0x40;
	
	// If there was a problem return the invalid time
	if(rtcBadTime) ret = DATETIME_INVALID;
	
    return ret;
}

// Write time
void ExRtcWriteTime(DateTime time)
{
	if(!exRtcPresent)return;
	
	unsigned char values[6];
	
	// Convert time to correct format
	values[5] = ToBcd(DATETIME_YEAR(time));
	values[4] = ToBcd(DATETIME_MONTH(time));
	values[3] = ToBcd(DATETIME_DAY(time));
	values[2] = ToBcd(DATETIME_HOURS(time));
	values[1] = ToBcd(DATETIME_MINUTES(time));
	values[0] = ToBcd(DATETIME_SECONDS(time));
	
	// Write time
	ExRtcOpen();
	ExRtcAddressWrite(RTC_ADDR_SECONDS);
	ExRtcWrite(values[0]); 
	ExRtcWrite(values[1]); 
	ExRtcWrite(values[2]); 
	ExRtcWrite(values[3]); 
	ExRtcWrite(1);// Week day - not used
	ExRtcWrite(values[4]); 
	ExRtcWrite(values[5]); 
	ExRtcClose();

    return;
}


