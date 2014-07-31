// Written to conglomerate a multi axis sensor platform to one .c file
// The main need for this is to save space and simplify the way we use
// the sensors by making a sampled sensor without fifo capability.

#ifndef _MULTI_SENSOR_H_
#define	_MULTI_SENSOR_H_

#include "Settings.h"

// Types
// Device present flags
typedef union {
	unsigned char all;
	struct{
	unsigned int accel : 1;
	unsigned int gyro : 1;
	unsigned int mag : 1;
	};
}verify_id_t;

// Generic triaxial type
typedef	union {
	struct	{
		short x;
		short y;
		short z;
	};
	struct	{
		unsigned char xl;
		unsigned char xh;
		unsigned char yl;
		unsigned char yh;
		unsigned char zl;
		unsigned char zh;
	};
}triaxial_t;

// Multi sensor type
typedef union {
	unsigned char data[18];
	signed short values[9]; // Be sure to align the type is using this
	struct {
		triaxial_t accel;	// 6
		triaxial_t gyro;	// 6
		triaxial_t mag;		// 6
	};
}sensor_t;

// Defines
#define ACCEL_INTS_OFF 			0
#define ACCEL_INTS_TRANSIENT 	1
#define ACCEL_INTS_ORIENTATION 	2
#define ACCEL_INTS_TAP 			3


// Set in HW profile and pins set to inputs in InitIO()
#define MULTI_SENSOR_I2CCON	I2C1CON
#define MULTI_SENSOR_I2CCONbits	I2C1CONbits
#define MULTI_SENSOR_I2CBRG I2C1BRG
#define MULTI_SENSOR_I2CSTATbits I2C1STATbits

// Sensors
#define ACCEL_ADDRESS		0x38 	/*I2C address*/
#define ACCEL_BAUD			I2C_RATE_1000kHZ
#define ACCEL_DEVICE_ID		0x1a /*Staic response*/

#define MAG_ADDRESS			0x1C	/*I2C address*/
#define MAG_BAUD			I2C_RATE_200kHZ
#define MAG_DEVICE_ID		0xC4 	/*Staic response*/

#define GYRO_ADDRESS		0xD2	/*I2C address*/
#define GYRO_BAUD			I2C_RATE_400kHZ
#define GYRO_DEVICE_ID 0xD3	/*Static responce*/
#define GYRO_MASK_BURST		0x80

// Rate setting for pic24 devices funning at 8/32MHz
#define I2C_RATE_100kHZ 		((OSCCONbits.COSC==1)? 157 : 39)	
#define I2C_RATE_200kHZ 		((OSCCONbits.COSC==1)? 72 : 18)
#define I2C_RATE_400kHZ 		((OSCCONbits.COSC==1)? 37 : 9)	
#define I2C_RATE_1000kHZ 		((OSCCONbits.COSC==1)? 13 : 3)			
#define I2C_RATE_2000kHZ		((OSCCONbits.COSC==1)? 7 : 1)

// Globals
extern verify_id_t multiPresent;	

// Prototypes
extern unsigned char MultiVerifyDeviceId(void);	// Detects devices present
extern void MultiStartup(settings_t* set);		// Turns on present devices if rate setting != 0
extern void MultiStandby(void);					// Turns off PRESENT sensors
extern void MultiSingleSample(sensor_t *sample);// Sample
extern unsigned char AccelReadIntSource(void);	// Clears accel interrupt, returns reason

// Private
void LOCAL_CHECK_TIMEOUT(unsigned short timeout);
unsigned char ReadData(unsigned char busAddress, unsigned char subAddress, unsigned char* dest, unsigned char num, unsigned short baud);
unsigned char WriteData(unsigned char busAddress, unsigned char subAddress, unsigned char* source, unsigned char num, unsigned short baud);

// Other defines

//Accel addresses
// MMAQ8452Q registers
#define 	ACCEL_ADDR_STATUS			0x00
#define 	ACCEL_ADDR_F_STATUS			0x00
#define 	ACCEL_ADDR_OUT_X_MSB		0x01
#define 	ACCEL_ADDR_OUT_X_LSB		0x02
#define 	ACCEL_ADDR_OUT_Y_MSB		0x03
#define 	ACCEL_ADDR_OUT_Y_LSB		0x04
#define 	ACCEL_ADDR_OUT_Z_MSB		0x05
#define 	ACCEL_ADDR_OUT_Z_LSB		0x06
#define 	ACCEL_ADDR_F_SETUP			0x09
#define 	ACCEL_ADDR_TRIG_CFG			0x0A
#define 	ACCEL_ADDR_SYSMOD			0x0B
#define 	ACCEL_ADDR_INT_SOURCE		0x0C
#define 	ACCEL_ADDR_WHO_AM_I			0x0D
#define 	ACCEL_ADDR_XYZ_DATA_CFG		0x0E
#define 	ACCEL_ADDR_HP_FILTER_CUTOFF	0x0F
#define 	ACCEL_ADDR_PL_STATUS		0x10
#define 	ACCEL_ADDR_PL_CFG			0x11
#define 	ACCEL_ADDR_PL_COUNT			0x12
#define 	ACCEL_ADDR_PL_BF_ZCOMP		0x13
#define 	ACCEL_ADDR_P_L_THS_REG		0x14
#define 	ACCEL_ADDR_FF_MT_CFG		0x15
#define 	ACCEL_ADDR_FF_MT_SRC		0x16
#define 	ACCEL_ADDR_FF_MT_THS		0x17
#define 	ACCEL_ADDR_FF_MT_COUNT		0x18
#define 	ACCEL_ADDR_TRANSIENT_CFG	0x1D
#define 	ACCEL_ADDR_TRANSIENT_SRC	0x1E
#define 	ACCEL_ADDR_TRANSIENT_THS	0x1F
#define 	ACCEL_ADDR_TRANSIENT_COUNT	0x20
#define 	ACCEL_ADDR_PULSE_CFG		0x21
#define 	ACCEL_ADDR_PULSE_SRC		0x22
#define 	ACCEL_ADDR_PULSE_THSX		0x23
#define 	ACCEL_ADDR_PULSE_THSY		0x24
#define 	ACCEL_ADDR_PULSE_THSZ		0x25
#define 	ACCEL_ADDR_PULSE_TMLT		0x26
#define 	ACCEL_ADDR_PULSE_LTCY		0x27
#define 	ACCEL_ADDR_PULSE_WIND		0x28
#define 	ACCEL_ADDR_ASLP_COUNT		0x29
#define 	ACCEL_ADDR_CTRL_REG1		0x2A
#define 	ACCEL_ADDR_CTRL_REG2		0x2B
#define 	ACCEL_ADDR_CTRL_REG3		0x2C
#define 	ACCEL_ADDR_CTRL_REG4		0x2D
#define 	ACCEL_ADDR_CTRL_REG5		0x2E
#define 	ACCEL_ADDR_OFF_X			0x2F
#define 	ACCEL_ADDR_OFF_Y			0x30
#define 	ACCEL_ADDR_OFF_Z			0x31
//Gyro addresses
// L3G4200
#define GYRO_WHO_AM_I 		0x0F   // READ ONLY,  default=0b11010011
#define GYRO_CTRL_REG1 		0x20   // READ/WRITE, default=0b00000111
#define GYRO_CTRL_REG2 		0x21   // READ/WRITE, default=0b00000000
#define GYRO_CTRL_REG3 		0x22   // READ/WRITE, default=0b00000000
#define GYRO_CTRL_REG4 		0x23   // READ/WRITE, default=0b00000000
#define GYRO_CTRL_REG5 		0x24   // READ/WRITE, default=0b00000000
#define GYRO_REFERENCE 		0x25   // READ/WRITE, default=0b00000000
#define GYRO_OUT_TEMP 		0x26   // READ ONLY
#define GYRO_STATUS_REG		0x27   // READ ONLY
#define GYRO_OUT_X_L 		0x28   // READ ONLY
#define GYRO_OUT_X_H 		0x29   // READ ONLY
#define GYRO_OUT_Y_L 		0x2A   // READ ONLY
#define GYRO_OUT_Y_H 		0x2B   // READ ONLY
#define GYRO_OUT_Z_L 		0x2C   // READ ONLY
#define GYRO_OUT_Z_H 		0x2D   // READ ONLY
#define GYRO_FIFO_CTRL_REG  0x2E   // READ/WRITE, default=0b00000000
#define GYRO_FIFO_SRC_REG 	0x2F   // READ ONLY
#define GYRO_INT1_CFG 		0x30   // READ/WRITE, default=0b00000000
#define GYRO_INT1_SRC 		0x31   // READ ONLY
#define GYRO_INT1_TSH_XH 	0x32   // READ/WRITE, default=0b00000000
#define GYRO_INT1_TSH_XL 	0x33   // READ/WRITE, default=0b00000000
#define GYRO_INT1_TSH_YH 	0x34   // READ/WRITE, default=0b00000000
#define GYRO_INT1_TSH_YL 	0x35   // READ/WRITE, default=0b00000000
#define GYRO_INT1_TSH_ZH 	0x36   // READ/WRITE, default=0b00000000
#define GYRO_INT1_TSH_ZL 	0x37   // READ/WRITE, default=0b00000000
#define GYRO_INT1_DURATION 	0x38   // READ/WRITE, default=0b00000000
//Mag addresses
// Mag3110 registers
#define	MAG_ADDR_DR_STATUS		0x00
#define	MAG_ADDR_OUT_X_MSB		0x01
#define	MAG_ADDR_OUT_X_LSB		0x02
#define	MAG_ADDR_OUT_Y_MSB		0x03
#define	MAG_ADDR_OUT_Y_LSB		0x04
#define	MAG_ADDR_OUT_Z_MSB		0x05
#define	MAG_ADDR_OUT_Z_LSB		0x06
#define	MAG_ADDR_WHO_AM_I		0x07
#define	MAG_ADDR_SYSMOD			0x08
#define	MAG_ADDR_OFF_X_MSB		0x09
#define	MAG_ADDR_OFF_X_LSB		0x0A
#define	MAG_ADDR_OFF_Y_MSB		0x0B
#define	MAG_ADDR_OFF_Y_LSB		0x0C
#define	MAG_ADDR_OFF_Z_MSB		0x0D
#define	MAG_ADDR_OFF_Z_LSB		0x0E
#define	MAG_ADDR_DIE_TEMP		0x0F
#define	MAG_ADDR_CTRL_REG1		0x10
#define	MAG_ADDR_CTRL_REG2		0x11

#endif
