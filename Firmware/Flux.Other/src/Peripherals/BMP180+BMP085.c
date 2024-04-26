// KL 06-01-2012
// Based on the bosch api and datasheet for the BMP280 sensor

// Device settings are for 'forced mode':
// Forced mode lets the user trigger a sample with device handling return to standby
// Change the register struct values before calling driver functions for alternate behaviour
// The sample method in 'forced mode' is to write 0b01 to mode bits in ctrl_reg

#include <stdint.h>
#include "Compiler.h"
#include "HardwareProfile.h"
#include "Peripherals/SysTime.h"
#include "Peripherals/Altimeter.h"
#ifdef ENABLE_ALTITUDE_CALC
#include <math.h>
#endif

// Device type check
#if defined(ALTIMETER_BMP180) || defined(ALTIMETER_BMP085)

// Alternate I2C bus?
#ifdef ALT_ALTERNATE_I2C
	#define MY_I2C_OVERIDE	ALT_ALTERNATE_I2C
#endif
// Default I2C settings
#define LOCAL_I2C_RATE	I2C_RATE_400kHZ
#include "Peripherals/myI2C.h"

// Definitions:
#ifndef BMP085_I2C_ADDR
#define BMP085_I2C_ADDR				0xEE
#endif
#define BMP085_CHIP_ID_REG			0xD0
#define BMP085_CHIP_ID				0x55
#define BMP085_PROM_DATA__LEN		22
#define BMP085_PROM_START__ADDR		0xAA
#define BMP085_MAX_SAMP_TIME_TEMP  	5
#define BMP085_MAX_SAMP_TIME_PRESS	30
#define BMP085_CTRL_MEAS_REG		0xF4
#define BMP085_ADC_OUT_MSB_REG		0xF6
#define BMP085_P_MEASURE        	0x34
#define BMP085_T_MEASURE        	0x2E
/* Specific constants */
#define SMD500_PARAM_MG     		3038 
#define SMD500_PARAM_MH     		-7357 
#define SMD500_PARAM_MI     		3791

// Driver variables:
uint8_t	altPresent = 0;

// Global structure holds all device specific parameters 
struct {
	int16_t ac1;
	int16_t ac2;
	int16_t ac3;
	uint16_t ac4;
	uint16_t ac5;
	uint16_t ac6;
	int16_t b1;
	int16_t b2;
	int16_t mb;
	int16_t mc;
	int16_t md;   
	int32_t param_b5;   
	int16_t number_of_samples;
	int16_t oversampling_setting;
	uint16_t ut;
	uint32_t up;
	int16_t	temperature;
	int32_t pressure;		   
} params;

// I2C routines
#define LOCAL_I2C_RATE		I2C_RATE_400kHZ

// I2C hardware abstraction
void altOpen(void)						{myI2COpen();myI2CStart(); WaitStartmyI2C();}
void altAddressRead(uint8_t _r)			{myI2Cputc(BMP085_I2C_ADDR); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(BMP085_I2C_ADDR | I2C_READ_MASK);}
void altAddressWrite(uint8_t _r)		{myI2Cputc(BMP085_I2C_ADDR); myI2Cputc((_r)); }
uint8_t altReadContinue(void)   		{uint8_t ret = myI2Cgetc(); myI2CAck();return ret;}
uint8_t altReadLast(void)       		{uint8_t ret = myI2Cgetc(); myI2CNack();return ret;}
void altWrite(uint8_t _v)       		{myI2Cputc((_v));}
void altClose(void )             		{myI2CStop(); WaitStopmyI2C();myI2CClose();}
void altReopen(void )            		{myI2CRestart(); WaitRestartmyI2C();}

uint8_t AltVerifyDeviceId(void) 
{
	uint8_t resp;

	ALT_INIT_PINS();

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

	return 1;
}

/* Initialise altimeter device and read calibration */
uint8_t AltInit(void)
{
	uint8_t i;
	uint8_t data[BMP085_PROM_DATA__LEN];
	
	// Check device is present
	if((!altPresent) && (!AltVerifyDeviceId()))
		return 0;
	
	// Set some parameters
	params.number_of_samples = 1; 
 
//	params.oversampling_setting=0; // Low power, 	3uA @ 1Hz
//	params.oversampling_setting=1; // Standard, 		5uA @ 1Hz
//	params.oversampling_setting=2; // High resolution, 	7uA @ 1Hz
	params.oversampling_setting=3; // Higher resolution,	12uA @ 1Hz

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
	params.ac1 =  (data[0] <<8) | data[1];
	params.ac2 =  (data[2] <<8) | data[3];
	params.ac3 =  (data[4] <<8) | data[5];
	params.ac4 =  (data[6] <<8) | data[7];
	params.ac5 =  (data[8] <<8) | data[9];
	params.ac6 = (data[10] <<8) | data[11];
	
	/*parameters B1,B2*/
	params.b1 =  (data[12] <<8) | data[13];
	params.b2 =  (data[14] <<8) | data[15];
	
	/*parameters MB,MC,MD*/
	params.mb =  (data[16] <<8) | data[17];
	params.mc =  (data[18] <<8) | data[19];
	params.md =  (data[20] <<8) | data[21];

	// Turns out you need to complete a temp and pressure conversion at least once to init vars
	// This temp value is used for future pressure readings
	AltSampleTemp();
	SysTimeDelayMs(BMP085_MAX_SAMP_TIME_TEMP);
	AltReadLastTemp();
	AltCalcTemp(); // This value will be used for pressure conversions

	// Read pressure too - sets some of the values in params
	AltSample();
	SysTimeDelayMs(BMP085_MAX_SAMP_TIME_PRESS); // Valid for all over sample settings 
	AltReadLast();
	
	params.temperature = AltCalcTemp();
	params.pressure = AltCalcPressure(); 

	/*Note: pressure compensation is corrected for temperature, the outside temp is not important for altitude calcs etc*/

  	return 1;
}

void AltStandby(void)
{
	// The device is in standby by default when not sampling
	return;
}

/* 	Calculate temperature from uncompensated temp
	Returns temperature in steps of 0.1 deg celsius */
int16_t AltCalcTemp(void) 
{
	int16_t temperature;
	int32_t x1,x2;  
	
	// Exit if not initialised
	if(!altPresent) 
		return 0;
	
	x1 = (((int32_t) params.ut - (int32_t) params.ac6) * (int32_t) params.ac5) >> 15;
	x2 = ((int32_t) params.mc << 11) / (x1 + params.md);
	params.param_b5 = x1 + x2;
	
	temperature = ((params.param_b5 + 8) >> 4);  // temperature in 0.1°C
	
	return (temperature);
}

/* 	Calculate pressure from uncompensated pressure
	Return pressure in steps of 1.0 Pa */
int32_t AltCalcPressure(void)
{
	int32_t pressure,x1,x2,x3,b3,b6;
	uint32_t b4, b7;
	
	// Exit if not initialised
	if(!altPresent) 
		return 0;
	
	b6 = params.param_b5 - 4000;
	//*****calculate B3************
	x1 = (b6*b6) >> 12;	 	 
	x1 *= params.b2;
	x1 >>=11;
	
	x2 = (params.ac2*b6);
	x2 >>=11;
	
	x3 = x1 +x2;
	
	b3 = (((((int32_t)params.ac1 )*4 + x3) <<params.oversampling_setting) + 2) >> 2;
	
	//*****calculate B4************
	x1 = (params.ac3* b6) >> 13;
	x2 = (params.b1 * ((b6*b6) >> 12) ) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (params.ac4 * (uint32_t) (x3 + 32768)) >> 15;
	
	b7 = ((uint32_t)(params.up - b3) * (50000>>params.oversampling_setting));   
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

/* 	Begin sampling Pressure */
void AltSample(void)
{
	// Exit if not initialised
	if(!altPresent) 
		return;

	altOpen();
	altAddressWrite(BMP085_CTRL_MEAS_REG);
	altWrite((BMP085_P_MEASURE + (params.oversampling_setting << 6)));
	altClose();
	return;
}

/* 	Read out uncompensated pressure into global reg for pressure conversion
  	depending on the oversampling ratio setting up can be 16 to 19 bit */
uint8_t AltReadLast(void)
{
	uint8_t data[3];    

	// Exit if not initialised
	if(!altPresent) 
		return 0;
	
	altOpen();
	altAddressRead(0xc2);
	data[0] = altReadLast();
	altClose();	
	if (data[0] != 0xbc) // If busy according to: "http://hwswbits.blogspot.co.uk/2012/09/eoc-bit-in-bosch-bmp085-i2c-registers.html"
	{
		return 0;	
	}

	altOpen();
	altAddressRead(BMP085_ADC_OUT_MSB_REG);
	data[0] = altReadContinue();
	data[1] = altReadContinue();
	data[2] = altReadLast();
	altClose();

	params.up = (((uint32_t) data[0] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[2]) >> (8-params.oversampling_setting);

	params.number_of_samples = 1;

	return 1;
}

/* 	Begin sampling temperature */
void AltSampleTemp(void)
{
	// Exit if not initialised
	if(!altPresent) 
		return; 

	altOpen();
	altAddressWrite(BMP085_CTRL_MEAS_REG);
	altWrite(BMP085_T_MEASURE);
	altClose();

	return;
}

/* 	Read out uncompensated temperature into the internal regs */
uint8_t AltReadLastTemp(void)
{
	uint8_t data[2];    

	// Exit if not initialised
	if(!altPresent) 
		return 0;
	
	altOpen();
	altAddressRead(0xc2);
	data[0] = altReadLast();
	altClose();	
	if (data[0] != 0xbc) // If busy according to: "http://hwswbits.blogspot.co.uk/2012/09/eoc-bit-in-bosch-bmp085-i2c-registers.html"
	{
		return 0;	
	}

	altOpen();
	altAddressRead(BMP085_ADC_OUT_MSB_REG);
	data[0] = altReadContinue();
	data[1] = altReadLast();
	altClose();

	params.ut = (data[0] <<8) | data[1];

	return 1;
}

/*	Call to convert pressure to altitude, you need to know the local zero
	altitude pressure in Pa (~101000Pa). The equation is from the BMP280
	datasheet on p14.
	Returns altitude in cm steps (0.01m)*/
#ifdef ENABLE_ALTITUDE_CALC
#warning "Do you wan't to actually use this? It takes 0.5sec at 4MIPs on a PIC24!"
int32_t CalculateAltitude(int32_t pressure, int32_t pressure_sea_level)
{
	int32_t altitude = 0;
	float temp1,temp2; /*Using 2 to avoid potential stack issue*/
	temp1 = ((float)pressure)/((float)pressure_sea_level);
	temp2 = powf(temp1,0.19029495718363463368220742150333);/*note: powf() takes floats*/
	temp1 = (float)4433000*(1-temp2); /*note extra zeros for cm resolution*/
	altitude = ((temp1 >= 0) ? (int32_t)(temp1+0.5) : (int32_t)(temp1-0.5));// Avoid truncation
	return altitude;
}
#endif

#endif // #ifdef ALTIMETER_BMP180
//EOF

