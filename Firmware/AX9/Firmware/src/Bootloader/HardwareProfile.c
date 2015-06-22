// KL 12-06-2011 
// .C file for hardware specific functions like sleep, hibernate etc

#include <Compiler.h>
#include "Ctrl_I2C.h"
#include "HardwareProfile.h"
#include "ScriptCtrl.h"
#include "Ctrl_I2C.h"

// 24 bit config 1 0b<unimp>0000<unimp>0000<res>0<jtag off>0<code prot off>1<prog writes on>1<debug off>1<res>1<EMUD1>11<wdtoff>0<stdrd wdt>1<unimp>0<wdt pres 32>1<wdt posc see table>****>*/
//     WDT bits -- 1111 = 1:32,768; 1110 = 1:16,384; 1101 = 1:8,192; 1100 = 1:4,096; 1011 = 1:2,048; 1010 = 1:1,024; 1001 = ***1:512***; 1000 = 1:256; 0111 = 1:128; 0110 = 1:64; 0101 = 1:32; 0100 = 1:16; 0011 = 1:8; 0010 = 1:4; 0001 = 1:2; 0000 = 1:1;
_CONFIG1(0b0011111101101010)
// 24 bit config 2 0b<unimp>0000<unimp>0000<IESO off>0<plldiv2>001<plldis off>0<init osc = frc>000<clock switch en>00<no clk op>0<iolock on-off en>0<usb vreg off>1<res>1<prim osc off>11>*/
_CONFIG2(0b0001000000001111)
#ifdef __DEBUG
// Code protection all off in debug
_CONFIG3(0b1111111111111111)
#else
// Code protection of config words and first 8 pages of flash (to 0x2000)
_CONFIG3((0b0001111111111111 & 0xFFFE07))
#endif

#ifdef __DEBUG
#warning "Debugging default interrupt on."
// The bootloader normally has no interrupts. Everything is redirected.
// Exceptions and unhandled interrupts
void __attribute__((interrupt,auto_psv)) _DefaultInterrupt(void)
{
 	static unsigned int INTCON1val;
	unsigned char usb_entry, usb_now;
	INTCON1val = INTCON1;
	INTCON1 = 0;
	// Get current usb state
	usb_entry = usb_now = USB_BUS_SENSE;
	// Magenta LED indicates error
	LED_SET(LED_MAGENTA);
	// Wait till usb changes state
	while(usb_now == usb_entry){usb_now = USB_BUS_SENSE;}
	// Break point here, (dis)connect usb and step into the faulty code
	Nop();	Nop(); 	Nop();	
}
#endif


// I2C Sensor turn off
#ifdef HARDWAREPROFILE_AX9_H
#define MULTI_SENSOR_I2C	I2C_BUS_2
#elif defined(HARDWAREPROFILE_CWA31_H)
#define MULTI_SENSOR_I2C	I2C_BUS_1
#endif

#define GYRO_ADDRESS		0xD2	/*I2C address*/
#define GYRO_WHO_AM_I 		0x0F	/*Device ID reg*/
#define GYRO_DEVICE_ID 		0xD3	/*Static responce*/
#define GYRO_MASK_BURST		0x80
#define GYRO_CTRL_REG2 		0x21	/*Set to 0xF0*/

#define MAG_ADDRESS			0x1C	/*I2C address*/
#define	MAG_ADDR_WHO_AM_I	0x07	/*Device ID reg*/
#define MAG_DEVICE_ID		0xC4 	/*Static response*/
#define	MAG_ADDR_CTRL_REG1	0x10	/*Set to 0x00*/

#define ACCEL_ADDRESS		0x38 	/*I2C address*/
#define ACCEL_ADDR_WHO_AM_I	0x0D	/*Device ID reg*/
#define ACCEL_DEVICE_ID		0x1a 	/*Static response*/
#define ACCEL_ADDR_CTRL_REG1 0x2A	/*Set to 0x00*/

const I2C_descriptor_t i2c_desc = {
	.bus = MULTI_SENSOR_I2C,
	.baud = I2C_BAUD_100kHZ,
	.mode = I2C_RAW_MODE
};

const uint8_t accel_off[] =	{ACCEL_ADDRESS, ACCEL_ADDR_CTRL_REG1, 0x00};
const uint8_t gyro_off[] = 	{GYRO_ADDRESS, GYRO_CTRL_REG2, 0xF0};
const uint8_t mag_off[] = 	{MAG_ADDRESS, MAG_ADDR_CTRL_REG1, 0x00};

void SystemInit(void)
{
	// Oscillator selection
	CLOCK_INTOSC();
	CLOCK_SOSCDIS();
	RCONbits.PMSLP = 0;

	// Minimal init i/o and internal peripherals
	// Sets off/low power for: nand, analogue, bluetooth, altimeter, leds and vibe-motor
	InitIOBoot();	

	// Turn off power hungry peripherals
	// Turns off gyro, mag and accelerometer
	SC_ctx_t ctx;
	ctx.desc = (uint8_t*)&i2c_desc;
	SC_i2c_open(&ctx);

	ctx.srcp = gyro_off;
	ctx.len = sizeof(gyro_off);
	SC_i2c_write(&ctx);

	ctx.srcp = mag_off;
	ctx.len = sizeof(mag_off);
	SC_i2c_write(&ctx);

	ctx.srcp = accel_off;
	ctx.len = sizeof(accel_off);
	SC_i2c_write(&ctx);
	
	SC_i2c_close(&ctx);		
}
//EOF
