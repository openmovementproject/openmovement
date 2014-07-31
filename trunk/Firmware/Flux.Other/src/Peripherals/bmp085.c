// KL 06-01-2012
// Re-written from the bosch api for the BMP085 sensor
// This is to add the ability to use the end_of_conversion interrupt
// avoiding the delay function and allowing the device to be non-polled.
// Notes: 28-06-2012  KL - Extended compatibility to share driver between BMP085 and BMP180


#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "Peripherals/bmp085.h"

// Alternate I2C bus?
#ifdef ALT_ALTERNATE_I2C
	#define MY_I2C_OVERIDE	ALT_ALTERNATE_I2C
#endif
#define USE_AND_OR
#include "Peripherals/myI2C.h"



#ifdef ENABLE_ALTITUDE_CALC
#include <math.h>
#endif

static unsigned char 	altRate = 0;
static unsigned short 	altFrequency = 0;
				char 	altPresent = 0;
				char	altDataReady = 0;

unsigned char AltRate(void) 		{ return altRate; }
unsigned short AltFrequency(void) { return altFrequency; }

// I2C routines
#define LOCAL_I2C_RATE		I2C_RATE_400kHZ

// I2C 
#define CUT_DOWN_I2C_CODE_SIZE
#ifndef CUT_DOWN_I2C_CODE_SIZE
#define altOpen()              myI2COpen();myI2CStart(); WaitStartmyI2C();
#define altAddressRead(_r)     myI2Cputc(BMP085_I2C_ADDR); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(BMP085_I2C_ADDR | I2C_READ_MASK);
#define altAddressWrite(_r)    myI2Cputc(BMP085_I2C_ADDR); myI2Cputc((_r)); 
#define altReadContinue()      myI2Cgetc(); myI2CAck()
#define altReadLast()          myI2Cgetc(); myI2CNack()
#define altWrite(_v)           myI2Cputc((_v));
#define altClose()             myI2CStop(); WaitStopmyI2C();myI2CClose();
#define altReopen()            myI2CRestart(); WaitRestartmyI2C();
#else
void altOpen(void)						{myI2COpen();myI2CStart(); WaitStartmyI2C();}
void altAddressRead(unsigned char _r)	{myI2Cputc(BMP085_I2C_ADDR); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(BMP085_I2C_ADDR | I2C_READ_MASK);}
void altAddressWrite(unsigned char _r)	{myI2Cputc(BMP085_I2C_ADDR); myI2Cputc((_r)); }
unsigned char altReadContinue(void)   	{unsigned char ret = myI2Cgetc(); myI2CAck();return ret;}
unsigned char altReadLast(void)       	{unsigned char ret = myI2Cgetc(); myI2CNack();return ret;}
void altWrite(unsigned char _v)       	{myI2Cputc((_v));}
void altClose(void )             		{myI2CStop(); WaitStopmyI2C();myI2CClose();}
void altReopen(void )            		{myI2CRestart(); WaitRestartmyI2C();}
#endif


/** this global structure holds all device specific parameters 
*/
struct {
	short ac1;
	short ac2;
	short ac3;
	unsigned short ac4;
	unsigned short ac5;
	unsigned short ac6;
	short b1;
	short b2;
	short mb;
	short mc;
	short md;   
	long param_b5;   
	int number_of_samples;
	short oversampling_setting;
	unsigned short ut;
	unsigned long up;		   
} bmp085_params;

/* 	Initialize BMP085 
	This function initializes the BMP085 pressure sensor
	Returns TRUE for success  */
char BMP085_init(void) 
{
	unsigned char resp;
	unsigned char i;
	unsigned char data[BMP085_PROM_DATA__LEN];

	ALT_INIT_PINS();
	#ifdef ALT_XCLR /*Extended compatibility to share driver between BMP085 and BMP180*/
	ALT_XCLR = 0;
	DelayMs(1);
	ALT_XCLR = 1;
	DelayMs(10);
	#endif

	// Check if there is a device
	altOpen();
	altAddressRead(BMP085_CHIP_ID_REG);
	resp = altReadLast();
	altClose();
	
	// If not then bail
	if (resp != BMP085_CHIP_ID) 
	{
		altPresent = 0;
		return FALSE;
	}

	// Set global present flag
	altPresent = 1;

	// Set some parameters
	bmp085_params.number_of_samples = 1; 
 
//	bmp085_params.oversampling_setting=0; // Low power, 	3uA @ 1Hz
//	bmp085_params.oversampling_setting=1; // Standard, 		5uA @ 1Hz
//	bmp085_params.oversampling_setting=2; // High resolution, 	7uA @ 1Hz
	bmp085_params.oversampling_setting=3; // Higher resolution,	12uA @ 1Hz

	// Read out calibration data from the eeprom
	altOpen();
	altAddressRead(BMP085_PROM_START__ADDR);
	for (i=0;i<(BMP085_PROM_DATA__LEN-1);i++)
	{
		data[i] = altReadContinue();
	}
	data[BMP085_PROM_DATA__LEN-1] = altReadLast();
	altClose();

	//Convert the parameters
	/*parameters AC1-AC6*/
	bmp085_params.ac1 =  (data[0] <<8) | data[1];
	bmp085_params.ac2 =  (data[2] <<8) | data[3];
	bmp085_params.ac3 =  (data[4] <<8) | data[5];
	bmp085_params.ac4 =  (data[6] <<8) | data[7];
	bmp085_params.ac5 =  (data[8] <<8) | data[9];
	bmp085_params.ac6 = (data[10] <<8) | data[11];
	
	/*parameters B1,B2*/
	bmp085_params.b1 =  (data[12] <<8) | data[13];
	bmp085_params.b2 =  (data[14] <<8) | data[15];
	
	/*parameters MB,MC,MD*/
	bmp085_params.mb =  (data[16] <<8) | data[17];
	bmp085_params.mc =  (data[18] <<8) | data[19];
	bmp085_params.md =  (data[20] <<8) | data[21];

	// Turns out you need to complete a temp and pressure conversion at least once to init vars
	// This temp value is used for future pressure readings
	BMP085_Initiate_ut_conversion();
	DelayMs(BMP085_TEMP_CONVERSION_TIME);
	BMP085_read_ut();
	BMP085_calc_temperature(); // This value will be used for pressure conversions

	// Read pressure too - sets some of the values in params
	BMP085_Initiate_up_conversion();
	DelayMs(30); // Valid for all over sample settings 
	BMP085_read_up();
	BMP085_calc_pressure(); // This value will be used for pressure conversions

	/*Note: pressure compensation is corrected for temperature, the outside temp is not important for altitude calcs etc*/

  	return TRUE;
}

void BMP085_standby(void)
{
	// The device is in standby by default when not sampling
	return;
}

/* 	Turn on the end of conversion interrupt and start a conversion - will fire on completion*/
#ifndef ALT_IGNORE_EOC
void BMP085_int_enable(void)
{
	if(!altPresent)return;
	Initiate_ut_conversion(); 	// EOC pin goes low
	CN_INTS_OFF();				// Interrupts off
	ALT_CN_INT = 1;				// Enable the CN int for this pin
	CN_INTS_ON();				// Re-enable the CN ints
	if (ALT_EOC)				// If the conversion is over already
	{
		CN_INT_FIRE();			// The CN interrupt must handle the rest
	}
	return;
}
#endif

/* 	Calculate temperature from uncompensated temp
	Returns temperature in steps of 0.1 deg celsius */
short BMP085_calc_temperature(void) 
{
	short temperature;
	long x1,x2;  
	
	if(!altPresent)return -1;
	
	x1 = (((long) bmp085_params.ut - (long) bmp085_params.ac6) * (long) bmp085_params.ac5) >> 15;
	x2 = ((long) bmp085_params.mc << 11) / (x1 + bmp085_params.md);
	bmp085_params.param_b5 = x1 + x2;
	
	temperature = ((bmp085_params.param_b5 + 8) >> 4);  // temperature in 0.1°C
	
	return (temperature);
}

/* 	Calculate pressure from uncompensated pressure
	Return pressure in steps of 1.0 Pa */
long BMP085_calc_pressure(void)
{
	long pressure,x1,x2,x3,b3,b6;
	unsigned long b4, b7;
	
	if(!altPresent)return -1;
	
	b6 = bmp085_params.param_b5 - 4000;
	//*****calculate B3************
	x1 = (b6*b6) >> 12;	 	 
	x1 *= bmp085_params.b2;
	x1 >>=11;
	
	x2 = (bmp085_params.ac2*b6);
	x2 >>=11;
	
	x3 = x1 +x2;
	
	b3 = (((((long)bmp085_params.ac1 )*4 + x3) <<bmp085_params.oversampling_setting) + 2) >> 2;
	
	//*****calculate B4************
	x1 = (bmp085_params.ac3* b6) >> 13;
	x2 = (bmp085_params.b1 * ((b6*b6) >> 12) ) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (bmp085_params.ac4 * (unsigned long) (x3 + 32768)) >> 15;
	
	b7 = ((unsigned long)(bmp085_params.up - b3) * (50000>>bmp085_params.oversampling_setting));   
	if (b7 < 0x80000000)
	{
		pressure = (b7 << 1) / b4;
	}
	else
	{ 
		pressure = (b7 / b4) << 1;
	}
	
	x1 = pressure >> 8;
	x1 *= x1;
	x1 = (x1 * SMD500_PARAM_MG) >> 16;
	x2 = (pressure * SMD500_PARAM_MH) >> 16;
	pressure += (x1 + x2 + SMD500_PARAM_MI) >> 4;	// pressure in Pa  
	
	return (pressure);
}

/* 	Begin sampling temperature */
void BMP085_Initiate_ut_conversion(void)
{
	if(!altPresent) return; // Not initialised
#ifndef ALT_IGNORE_EOC
	if (!ALT_EOC) return;	// Indicates current conversion is incomplete
#endif
	altOpen();
	altAddressWrite(BMP085_CTRL_MEAS_REG);
	altWrite(BMP085_T_MEASURE);
	altClose();

	return;
}

/* 	Read out uncompensated temperature into the internal regs */
#ifdef USE_CTRL_REG_BUSY_BIT
	unsigned char BMP085_read_ut (void)
#else
	void BMP085_read_ut (void)
#endif
{
	unsigned char data[2];    
#ifndef ALT_IGNORE_EOC
	if (!ALT_EOC) return;	// Indicates current conversion is incomplete
#endif
#ifdef USE_CTRL_REG_BUSY_BIT
	if(!altPresent) return 0; // Not initialised
	altOpen();
	altAddressRead(0xc2);
	data[0] = altReadLast();
	altClose();	
	if (data[0] != 0xbc) // If busy according to: "http://hwswbits.blogspot.co.uk/2012/09/eoc-bit-in-bosch-bmp085-i2c-registers.html"
	{
		return 0;	
	}
#else
	if(!altPresent) return; // Not initialised		
#endif
	altOpen();
	altAddressRead(BMP085_ADC_OUT_MSB_REG);
	data[0] = altReadContinue();
	data[1] = altReadLast();
	altClose();

	bmp085_params.ut = (data[0] <<8) | data[1];

#ifdef USE_CTRL_REG_BUSY_BIT
	return 1;
#else
	return;
#endif
}

/* 	Begin sampling Pressure */
void BMP085_Initiate_up_conversion(void)
{
	if(!altPresent) return; // Not initialised
#ifndef ALT_IGNORE_EOC
	if (!ALT_EOC) return;	// Indicates current conversion is incomplete
#endif
	altOpen();
	altAddressWrite(BMP085_CTRL_MEAS_REG);
	altWrite((BMP085_P_MEASURE + (bmp085_params.oversampling_setting << 6)));
	altClose();
	return;
}

/* 	Read out uncompensated pressure into global reg for pressure conversion
  	depending on the oversampling ratio setting up can be 16 to 19 bit */
#ifdef USE_CTRL_REG_BUSY_BIT
	unsigned char BMP085_read_up (void)
#else
	void BMP085_read_up (void)
#endif
{
	unsigned char data[3];    

#ifndef ALT_IGNORE_EOC
	if (!ALT_EOC) return;	// Indicates current conversion is incomplete
#endif
#ifdef USE_CTRL_REG_BUSY_BIT
	if(!altPresent) return 0; // Not initialised
	altOpen();
	altAddressRead(0xc2);
	data[0] = altReadLast();
	altClose();	
	if (data[0] != 0xbc) // If busy according to: "http://hwswbits.blogspot.co.uk/2012/09/eoc-bit-in-bosch-bmp085-i2c-registers.html"
	{
		return 0;	
	}
#else
	if(!altPresent) return; // Not initialised	
#endif
	altOpen();
	altAddressRead(BMP085_ADC_OUT_MSB_REG);
	data[0] = altReadContinue();
	data[1] = altReadContinue();
	data[2] = altReadLast();
	altClose();

	bmp085_params.up = (((unsigned long) data[0] << 16) | ((unsigned long) data[1] << 8) | (unsigned long) data[2]) >> (8-bmp085_params.oversampling_setting);

	bmp085_params.number_of_samples = 1;

#ifdef USE_CTRL_REG_BUSY_BIT
	return 1;
#else
	return;
#endif
}

/*	Call to convert pressure to altitude, you need to know the local zero
	altitude pressure in Pa (~101000Pa). The equation is from the BMP085
	datasheet on p14.
	Returns altitude in cm steps (0.01m)*/
#ifdef ENABLE_ALTITUDE_CALC
long CalculateAltitude(long pressure, long pressure_sea_level)
{
	long altitude = 0;
	#ifdef ENABLE_ALTITUDE_CALC
	float temp1,temp2; /*Using 2 to avoid potential stack issue*/
	temp1 = ((float)pressure)/((float)pressure_sea_level);
	temp2 = powf(temp1,0.19029495718363463368220742150333);/*note: powf() takes floats*/
	temp1 = (float)4433000*(1-temp2); /*note extra zeros for cm resolution*/
	altitude = ((temp1 >= 0) ? (long)(temp1+0.5) : (long)(temp1-0.5));// Avoid truncation
	#endif
	return altitude;
}
#endif
//EOF



