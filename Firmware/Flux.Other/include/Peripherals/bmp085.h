// KL 06-01-2012
// Re-written from the bosch api for the BMP085 sensor
// This is to add the ability to use the end_of_conversion interrupt
// avoiding the delay function and allowing the device to be non-polled.
// 21-03-2016 KL: This header is a shim to make the BMP085 driver behave like the general altimeter driver

#ifndef __BMP085_H__
#define __BMP085_H__

//#include "GenericTypeDefs.h"

/* Example:
BMP085_init();			// Startup
ALT_SAMPLE();		// Initiate a sample 
DelayMs(30);			// Wait for a while (>26ms)
BMP085_read_up();		// Read the value at the i2c interrupt priority
BMP085_calc_pressure();	// Convert the last read value to Pascals
*/

#define AltSingleSample(result)	{\
								BMP085_Initiate_up_conversion();\
								DelayMs(50);\
								BMP085_read_up ();\
								*result = BMP085_calc_pressure();\
								}

typedef long alt_t;

/*
	CHIP_TYPE CONSTANTS
*/

#define BMP085_CHIP_ID			0x55
#define BMP085_I2C_ADDR			0xEE

#define BMP085_PROM_START__ADDR		0xaa
#define BMP085_PROM_DATA__LEN		  22

#define BMP085_CHIP_ID_REG			0xD0
#define BMP085_VERSION_REG			0xD1

#define BMP085_CTRL_MEAS_REG		0xF4
#define BMP085_ADC_OUT_MSB_REG		0xF6
#define BMP085_ADC_OUT_LSB_REG		0xF7

#define BMP085_SOFT_RESET_REG		0xE0

#define BMP085_T_MEASURE        0x2E				// temperature measurent 
#define BMP085_P_MEASURE        0x34				// pressure measurement

#define BMP085_TEMP_CONVERSION_TIME  5				// ms


/* Specific constants */
#define SMD500_PARAM_MG      3038        //calibration parameter
#define SMD500_PARAM_MH     -7357        //calibration parameter
#define SMD500_PARAM_MI      3791        //calibration parameter

// Globals
extern char	altPresent;
extern char	altDataReady;

// Prototypes
/* Call first to init the device and read the calibration, returns true for success - uses I2C*/
char BMP085_init(void);
#define AltVerifyDeviceId() BMP085_init()
#define AltInit() 			BMP085_init()
/* This function is added only for completeness, the device has auto standby*/
void BMP085_standby(void);
#define AltStandby() BMP085_standby()
/* Turn on interrups and initate a conversion*/
void BMP085_int_enable(void);
/* Call to convert and return the last temp reading in 0.1^C - doesn't use I2C */
#define AltCalcTemp() BMP085_calc_pressure()
short BMP085_calc_temperature(void); 
/* Call to convert and return the last pressure reading in Pa - doesn't use I2C */
#define AltCalcPressure() BMP085_calc_pressure()
long BMP085_calc_pressure(void);
/* Begin the pressure conversion */
#define AltSample() BMP085_Initiate_up_conversion() 
void BMP085_Initiate_up_conversion(void);
/* Call from the I2C process priority to read the last pressure conversion into the global struct*/
#define AltReadLast()	BMP085_read_up()
#ifdef USE_CTRL_REG_BUSY_BIT
	unsigned char BMP085_read_up (void);
#else
	void BMP085_read_up (void);
#endif
/* Begin the temperature conversion */
#define AltSampleTemp() BMP085_Initiate_ut_conversion() 
void BMP085_Initiate_ut_conversion(void);
/* Call from the I2C process priority to read the last temp conversion into the global struct*/
#define AltReadLastTemp()	BMP085_read_ut()
#ifdef USE_CTRL_REG_BUSY_BIT
	unsigned char BMP085_read_ut (void);
#else
	void BMP085_read_ut (void);
#endif

/* Call to convert pressure to altitude*/
//#define ENABLE_ALTITUDE_CALC
long CalculateAltitude(long pressure, long pressure_sea_level);

// Rename all driver functions to newer altimeter API
// KL: Probably not possible really. Will need a newer driver to be written...
#if 0
#ifndef BMP085_init	
#define BMP085_init						AltVerifyDeviceId
#define BMP085_init						AltInit
#define BMP085_standby					AltStandby
#define BMP085_calc_temperature			AltCalcTemp
#define BMP085_calc_pressure			AltCalcPressure
#define BMP085_Initiate_up_conversion	AltSample
#define BMP085_read_up					AltReadLast
#define BMP085_Initiate_ut_conversion	AltSampleTemp
#define BMP085_read_ut					AltReadLastTemp
#endif
#endif

#endif   // __BMP085_H__





