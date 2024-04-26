// KL 06-01-2012
// Based on the bosch api and datasheet for the BMP280 sensor

// Device settings are for 'forced mode':
// Forced mode lets the user trigger a sample with device handling return to standby
// Change the register struct values before calling driver functions for alternate behaviour
// The sample method in 'forced mode' is to write 0b01 to mode bits in ctrl_reg

#include <stdint.h>
#include "Compiler.h"
#include "HardwareProfile.h"
#include "Utils/Debug.h"
#include "Peripherals/SysTime.h"
#include "Peripherals/Altimeter.h"
#ifdef ENABLE_ALTITUDE_CALC
#include <math.h>
#endif

// Device type check
#ifdef ALTIMETER_BMP280

// Alternate I2C bus?
#ifdef ALT_ALTERNATE_I2C
	#define MY_I2C_OVERIDE	ALT_ALTERNATE_I2C
#endif
// Default I2C settings
#define LOCAL_I2C_RATE	I2C_RATE_400kHZ
#include "Peripherals/myI2C.h"

// Definitions:
#ifndef BMP280_I2C_ADDR
#define BMP280_I2C_ADDR					0xEE
#endif
#ifndef ALT_MAX_SAMP_TIME
#define ALT_MAX_SAMP_TIME				45		// Milliseconds (ultra high res)
#endif
#define BMP280_CHIP_ID					0x58	// Chip id reg value
#define	BMP280_CALIB_DATA_LENGTH	    (24)	// Size in bytes
#define BMP280_CHIP_ID_REG         		(0xD0)  /*Chip ID Register */
#define BMP280_RST_REG             		(0xE0)  /*Softreset Register */
#define BMP280_STAT_REG            		(0xF3)  /*Status Register */
#define BMP280_CTRL_MEAS_REG       		(0xF4)  /*Ctrl Measure Register */
#define BMP280_CONFIG_REG          		(0xF5)  /*Configuration Register */
#define BMP280_PRESSURE_MSB_REG    		(0xF7)  /*Pressure MSB Register */
#define BMP280_PRESSURE_LSB_REG    		(0xF8)  /*Pressure LSB Register */
#define BMP280_PRESSURE_XLSB_REG   		(0xF9)  /*Pressure XLSB Register */
#define BMP280_TEMPERATURE_MSB_REG 		(0xFA)  /*Temperature MSB Reg */
#define BMP280_TEMPERATURE_LSB_REG 		(0xFB)  /*Temperature LSB Reg */
#define BMP280_TEMPERATURE_XLSB_REG		(0xFC)  /*Temperature XLSB Reg */
#define BMP280_CALIB_DIG_T1_LSB_REG     (0x88)	/*First byte of config */

// Driver variables:
uint8_t	altPresent = 0;

// The device internal registers:
// BMP280 registers:
// 		Data: 0xF7-0xFC <Pmsb-Plsb-Pxlsb-Tmsb-Tlsb-Txlsb>
// 		STATUS: 0xF3, measuring<3> im_update<0>
// 		CTRL_MEAS: 0xF4, osrs_t<7-5> osrs_p<4-2> mode<1-0>
// 		CONFIG: 0xF5, t_sb<7-5> filter<4-2> spi2w_en<0>
// 		RESET: 0xE0, write 0xB6 to reset
// 		CHIP_ID: 0xD0, fixed value of 0x58
// 		Calibration data: 0x88 - 0x9F (little endian)
struct bmp280_regs {
	uint8_t		status;		// Address: 0xF3
	uint8_t		ctrl_meas;	// Address: 0xF4
	uint8_t		config;		// Address: 0xF5	
} reg = {
0,			// Not written. This is not a setting but is read into for status polling 
0b00101100, // Over sampling ratio pressure/temperature 4 & 1 respectively. Sleep mode
0b10100000	// Filter off, Tstandby not used (1000ms), 3 wire SPI off
};

// Device calibration data:
struct bmp280_cal {
	uint16_t	dig_T1;		// Address: 0x88 / 0x89 
	int16_t     dig_T2;		// Address: 0x8A / 0x8B 
	int16_t     dig_T3;		// Address: 0x8C / 0x8D 
	uint16_t    dig_P1;		// Address: 0x8E / 0x8F 
	int16_t 	dig_P2;		// Address: 0x90 / 0x91 
	int16_t 	dig_P3;		// Address: 0x92 / 0x93 
	int16_t 	dig_P4;		// Address: 0x94 / 0x95 
	int16_t 	dig_P5;		// Address: 0x96 / 0x97 
	int16_t 	dig_P6;		// Address: 0x98 / 0x99 
	int16_t 	dig_P7;		// Address: 0x9A / 0x9B 
	int16_t 	dig_P8;		// Address: 0x9C / 0x9D 
	int16_t 	dig_P9;		// Address: 0x9E / 0x9F 
} cal;

// Device last data conversion (raw and compensated):
struct bmp280_data {
	int32_t		adc_P;		// Raw, uncompensated pressure
	int32_t		adc_T;		// Raw, uncompensated temperature
	int32_t 	t_fine;		// Value used internally for equation
	uint32_t 	pressure;	// Calculated pressure value in Pascals
	int16_t 	temperature;// Calculated temperature x0.1 Celsius
} data;

// Datasheet defined functions (prototypes):
int32_t bmp280_compensate_T_int32(int32_t adc_T);
uint32_t bmp280_compensate_P_int64(int32_t adc_P);

// I2C hardware abstraction
void altOpen(void)						{myI2COpen();myI2CStart(); WaitStartmyI2C();}
void altAddressRead(unsigned char _r)	{myI2Cputc(BMP280_I2C_ADDR); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(BMP280_I2C_ADDR | I2C_READ_MASK);}
void altAddressWrite(unsigned char _r)	{myI2Cputc(BMP280_I2C_ADDR); myI2Cputc((_r)); }
unsigned char altReadContinue(void)   	{unsigned char ret = myI2Cgetc(); myI2CAck();return ret;}
unsigned char altReadLast(void)       	{unsigned char ret = myI2Cgetc(); myI2CNack();return ret;}
void altWrite(unsigned char _v)       	{myI2Cputc((_v));}
void altClose(void )             		{myI2CStop(); WaitStopmyI2C();myI2CClose();}
void altReopen(void )            		{myI2CRestart(); WaitRestartmyI2C();}

// I2C bus transaction routines
uint8_t bus_write(uint8_t register_addr, uint8_t* register_data, uint8_t wr_len)
{
	if(!altPresent)
		return 0;
	altOpen();
	altAddressWrite(register_addr++);
	altWrite(*register_data++);
	wr_len--;
	while(wr_len--)
	{
		altWrite(register_addr++);
		altWrite(*register_data++);
	}
	altClose();
	return 1;
}
uint8_t bus_read(uint8_t register_addr, uint8_t* register_data, uint8_t rd_len)
{
	if(!altPresent)
		return 0;	
	altOpen();
	altAddressRead(register_addr);
	while(rd_len-- > 1)*register_data++ = altReadContinue();
	*register_data = altReadLast();
	altClose();
	return 1;
}

/* Call first to detect the device, returns true for success */
uint8_t AltVerifyDeviceId(void) 
{
	unsigned char resp;

	ALT_INIT_PINS();

	// Check if there is a device
	altOpen();
	altAddressRead(BMP280_CHIP_ID_REG);
	resp = altReadLast();
	altClose();
	
	// If not, then exit
	if (resp != BMP280_CHIP_ID) 
	{
		altPresent = 0;
		return 0;
	}

	// Set global present flag
	altPresent = 1;

	// Read the calibration in
	AltInit();

  	return 1;
}

/* Initialise altimeter device and read calibration */
uint8_t AltInit(void) 
{
	// Check device is present
	if((!altPresent) && (!AltVerifyDeviceId()))
		return 0;

	// Relying upon struct alignment
	DBG_STATIC_ASSERT(sizeof(cal)==BMP280_CALIB_DATA_LENGTH);

	// Read calibration
	bus_read(BMP280_CALIB_DIG_T1_LSB_REG, (uint8_t*)&cal.dig_T1, BMP280_CALIB_DATA_LENGTH);
		
	// Write operating mode
	bus_write(BMP280_CTRL_MEAS_REG, &reg.ctrl_meas, 2);

	// Read initial values
	AltSample();
	SysTimeDelayMs(ALT_MAX_SAMP_TIME); // Valid for all over sample settings 
	AltReadLast();
	AltCalcPressure(); 

	return 1;
}

/* Reset the device. Resets to standby mode */
void AltStandby(void)
{
	uint8_t reset_val = 0xB6;
	// The device resets to standby
	bus_write(BMP280_RST_REG, &reset_val, 1);
	return;
}

/* Call to convert and return the last temp reading in 0.1^C */
int16_t AltCalcTemp(void) 
{
	data.temperature = bmp280_compensate_T_int32(data.adc_T);
	return data.temperature;
}

/* Call to convert and return the last pressure reading in Pa */
int32_t AltCalcPressure(void)
{
	data.pressure = bmp280_compensate_P_int64(data.adc_P);
	return (int32_t)data.pressure;
}

/* 	Begin sampling Pressure */
void AltSample(void)
{
	// Write 0b01 to mode bits in ctrl_reg to sample
	reg.ctrl_meas |= 0x01;
	bus_write(BMP280_CTRL_MEAS_REG, &reg.ctrl_meas, 1);	
	return;
}

/* Read data output */
uint8_t AltReadLast (void)
{
	uint8_t buffer[6];
	// Read last conversion data out
	bus_read(BMP280_PRESSURE_MSB_REG, buffer, 6);
	// Re-construct int32_t values
	data.adc_P = ((uint16_t)buffer[0] << 8) + buffer[1];
	data.adc_P = (data.adc_P << 4) + (buffer[2] >> 4);
	data.adc_T = ((uint16_t)buffer[3] << 8) + buffer[4];
	data.adc_T = (data.adc_T << 4) + (buffer[5] >> 4);	
	return 1;
}

/* 	Begin sampling temperature */
void AltSampleTemp(void)
{
	// Sampled at same time as pressure
	return;
}

/* 	Read out uncompensated temperature into the internal regs */
uint8_t AltReadLastTemp(void)
{
	// Read at same time as pressure
	return 1;
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
int32_t bmp280_compensate_T_int32(int32_t adc_T)
{
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)cal.dig_T1<<1))) * ((int32_t)cal.dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)cal.dig_T1)) * ((adc_T>>4) - ((int32_t)cal.dig_T1))) >> 12) *
	((int32_t)cal.dig_T3)) >> 14;
	data.t_fine = var1 + var2;
	#if 0
	// For 0.01 output resolution
	T = (data.t_fine * 5 + 128) >> 8;
	#else
	// For 0.1 output resolution
	T = (data.t_fine + 256) >> 9;		
	#endif
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
uint32_t bmp280_compensate_P_int64(int32_t adc_P)
{
	int64_t var1, var2, p;
	var1 = ((int64_t)data.t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)cal.dig_P6;
	var2 = var2 + ((var1*(int64_t)cal.dig_P5)<<17);
	var2 = var2 + (((int64_t)cal.dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t)cal.dig_P3)>>8) + ((var1 * (int64_t)cal.dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)cal.dig_P1)>>33;
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((int64_t)cal.dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)cal.dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)cal.dig_P7)<<4);
	return (uint32_t)((p+128)>>8);
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

#endif // #ifdef ALTIMETER_BMP280
//EOF
