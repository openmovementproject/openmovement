

// LIS3DH accelerometer interface. Initially for NRF51822 and SPI
// Karim Ladha 2016

// Includes
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "Peripherals/LIS3DH.h"
#include "HardwareProfile.h"
#include "Config.h"


#ifndef ACCEL_FIFO_WATERMARK
	#warning "Default watermark used, 25."
	#define ACCEL_FIFO_WATERMARK 25
#endif

// The device id, static response to reading ACCEL_ADDR_WHO_AM_I
#define ACCEL_DEVICE_ID		0x33 

/* 	Notes: Migration *to* LIS2DH - KL 30-07-2018
	The devices are identical except for the following:
	Reg STATUS_AUX has changed, no external ADC
	Reg OUT_ADC3_L/H now contains the temperature sensor data
	New register INT_COUNTER
	CTRL_REG0 is not present, only change requiring driver mod
	CTRL_REG3 now had a 'data ready 2' flag, unused
	CLICK_THS has had the 'latch click' bit removed
	#define LIS2DH to change driver, untested
*/
// Accelerometer register addresses
#define	ACCEL_ADDR_STATUS_REG_AUX	0x07
#define	ACCEL_ADDR_OUT_ADC1_L		0x08
#define	ACCEL_ADDR_OUT_ADC1_H		0x09
#define	ACCEL_ADDR_OUT_ADC2_L		0x0A
#define	ACCEL_ADDR_OUT_ADC2_H		0x0B
#define	ACCEL_ADDR_OUT_ADC3_L		0x0C
#define	ACCEL_ADDR_OUT_ADC3_H		0x0D
#define	ACCEL_ADDR_INT_COUNTER_REG	0x0E
#define	ACCEL_ADDR_WHO_AM_I			0x0F
#define	ACCEL_ADDR_CTRL_REG0		0x1E
#define	ACCEL_ADDR_TEMP_CFG_REG		0x1F
#define	ACCEL_ADDR_CTRL_REG1		0x20
#define	ACCEL_ADDR_CTRL_REG2		0x21
#define	ACCEL_ADDR_CTRL_REG3		0x22
#define	ACCEL_ADDR_CTRL_REG4		0x23
#define	ACCEL_ADDR_CTRL_REG5		0x24
#define	ACCEL_ADDR_CTRL_REG6		0x25
#define	ACCEL_ADDR_REFERENCE		0x26
#define	ACCEL_ADDR_STATUS_REG2		0x27
#define	ACCEL_ADDR_OUT_X_L			0x28
#define	ACCEL_ADDR_OUT_X_H			0x29
#define	ACCEL_ADDR_OUT_Y_L			0x2A
#define	ACCEL_ADDR_OUT_Y_H			0x2B
#define	ACCEL_ADDR_OUT_Z_L			0x2C
#define	ACCEL_ADDR_OUT_Z_H			0x2D
#define	ACCEL_ADDR_FIFO_CTRL_REG	0x2E
#define	ACCEL_ADDR_FIFO_SRC_REG		0x2F
#define	ACCEL_ADDR_INT1_CFG			0x30
#define	ACCEL_ADDR_INT1_SOURCE		0x31
#define	ACCEL_ADDR_INT1_THS			0x32
#define	ACCEL_ADDR_INT1_DURATION	0x33

#define	ACCEL_ADDR_INT2_CFG			0x34
#define	ACCEL_ADDR_INT2_SOURCE		0x35
#define	ACCEL_ADDR_INT2_THS			0x36
#define	ACCEL_ADDR_INT2_DURATION	0x37

#define	ACCEL_ADDR_CLICK_CFG		0x38
#define	ACCEL_ADDR_CLICK_SRC		0x39
#define	ACCEL_ADDR_CLICK_THS		0x3A
#define	ACCEL_ADDR_TIME_LIMIT		0x3B
#define	ACCEL_ADDR_TIME_LATENCY		0x3C
#define	ACCEL_ADDR_TIME_WINDOW		0x3D

// Variables
uint8_t accelPresent = 0;

accel_settings_t accel_regs;

// Default writeable register settings used for start up - unless externally defined
#ifndef ACCEL_LIS3DH_DEFAULT_REGS
#warning "Defaults not explicitly set."
const accel_settings_t accel_regs_startup = {
	.ctrl_reg0	= 0x90,			// SDO PU off KL: 30-07-2018 Changed from 0x10 to 0x90 to disable pullups on SDO
	.temp_cfg	= 0x00,			// Aux/Temp off
	.ctrl_reg1	= 0x27,			// 10Hz, XYZ on, Normal-power (ctrl4.hr=1)	
	.ctrl_reg2	= 0x04,			// HPF normal, Freq: ODR/50, HPF for tap on	
	.ctrl_reg3	= 0x06,			// Enable INT1: Watermark, overrun	
	.ctrl_reg4	= 0xA8,			// RW collision prot, LSB first, +/-8g, normal-power (ctrl1.lp=0)		
	.ctrl_reg5	= 0x48,			// FIFO on, Interrupts latch	
	.ctrl_reg6	= 0xC0,			// Enable INT2: Tap, AOI-1, I/O active high	
	.reference	= 0x00,			// Unused static reference	

	.fifo_ctrl	= 0x80 + ACCEL_FIFO_WATERMARK,	// FIFO stream mode, watermark threshold

	.int1_cfg	= 0xFF,			// Movement detect, AND signals, all 6 axis stable for time threshold	

	.int1_ths	= 12,			// Threshold (NxFS/128, 12 = 0.75g @ +/-8)	
	.int1_dur	= 5,			// Minimum duration (1/ODR, 5 = 500ms @ 10Hz)	

	.int2_cfg	= 0xFF,			// Movement detect, AND signals, all 6 axis stable for time threshold	

	.int2_ths	= 12,			// Threshold (NxFS/128, 12 = 0.75g @ +/-8)	
	.int2_dur	= 5,			// Minimum duration (1/ODR, 5 = 500ms @ 10Hz)	
	.click_cfg	= 0x3F,			// Single and double tap on all axis	

	.click_ths	= 16,			// Tap threshold (N x FS/128, 16 = @ +/-8g = 16*8/128 = 1g)
	.time_limit	= 3,			// 3 samples to fall below threshold	
	.time_latency = 1,			// De-bounce time between taps if double tap used
	.time_window = 10			// Window for second tap if double tap used
};
#else
const accel_settings_t accel_regs_startup = ACCEL_LIS3DH_DEFAULT_REGS;
//	User set defaults e.g.
//	define ACCEL_LIS3DH_DEFAULT_REGS {\
//	.temp_cfg	= 0x00,			/* Aux/Temp off	*/\
//	.ctrl_reg1	= 0x27,			/* 10Hz, XYZ on, Normal-power (ctrl4.hr=1)								*/\
//	.ctrl_reg2	= 0x04,			/* HPF normal, Freq: ODR/50, HPF for tap on								*/\
//	.ctrl_reg3	= 0x06,			/* Enable INT1: Watermark, overrun										*/\
//	.ctrl_reg4	= 0xA8,			/* RW collision prot, LSB first, +/-8g, normal-power (ctrl1.lp=0)		*/\
//	.ctrl_reg5	= 0x48,			/* FIFO on, Interrupts latch											*/\
//	.ctrl_reg6	= 0xC0,			/* Enable INT2: Tap, AOI-1, I/O active high								*/\
//	.reference	= 0x00,			/* Unused static reference												*/\
//	.fifo_ctrl	= 0x80 + ACCEL_FIFO_WATERMARK,	/* FIFO stream mode, threshold: setting (default = 25)	*/\
//	.int1_cfg	= 0xFF,			/* Movement detect, AND signals, all 6 axis stable for time threshold	*/\
//	.int1_ths	= 8,			/* Threshold (NxFS/128, 8 = 0.5g @ +/-8)								*/\
//	.int1_dur	= 5,			/* Minimum duration (1/ODR, 5 = 500ms @ 10Hz)							*/\
//	.int2_cfg	= 0xFF,			/* Movement detect, AND signals, all 6 axis stable for time threshold	*/\
//	.int2_ths	= 8,			/* Threshold (NxFS/128, 8 = 0.5g @ +/-8)								*/\
//	.int2_dur	= 5,			/* Minimum duration (1/ODR, 5 = 500ms @ 10Hz)							*/\
//	.click_cfg	= 0x3F,			/* Single and double tap on all axis									*/\
//	.click_ths	= 16,			/* Tap threshold (N x FS/128, 16 = @ +/-8g = 16*8/128 = 1g)				*/\
//	.time_limit	= 3,			/* 3 samples to fall below threshold									*/\
//	.time_latency = 1,			/* De-bounce time between taps if double tap used						*/\
//	.time_window = 10}			/* Window for second tap if double tap used								*/\

#endif

#ifndef ACCEL_TWI
// Using SPI interface mode
/*
Definitions - Connection, add to HardwareProfile.h 
// Pin configuration - (example pinout)
#define ACCEL_CS	2
#define ACCEL_SCK	5
#define ACCEL_SDO	3
#define ACCEL_SDI	4
#define ACCEL_INT1	0
#define ACCEL_INT2	1

GPIO configuration - Once, add to InitIO() function
#define INIT_ACCEL_PINS()	{	/* Setup the GPIO pins for SPI for master mode + interrupts *
							NRF_GPIO->PIN_CNF[ACCEL_CS] =	0x00000011; /*Sense-off, Drive S0S1, Pull-off, In-Off, Output *
							NRF_GPIO->PIN_CNF[ACCEL_SCK] =	0x00000001; /*Sense-off, Drive S0S1, Pull-off, In-On, Output *
							NRF_GPIO->PIN_CNF[ACCEL_SDI] =	0x00000011; /*Sense-off, Drive S0S1, Pull-off, In-Off, Output *
							NRF_GPIO->OUTSET = (1 << ACCEL_CS)|(1 << ACCEL_SCK)|(1 << ACCEL_SDI);/* Drive SPI outputs high *
							NRF_GPIO->PIN_CNF[ACCEL_SDO] =	0x00000000; /*Sense-off, Drive S0S1, Pull-off, In-On, Input *
							NRF_GPIO->PIN_CNF[ACCEL_INT1] = 0x00000000; /*Sense-off, Drive S0S1, Pull-off, In-On, Input *
							NRF_GPIO->PIN_CNF[ACCEL_INT2] = 0x00000000;}/*Sense-off, Drive S0S1, Pull-off, In-On, Input *

SPI module configuration - All configured in HardwareProfile.h, called by the driver
#define ACCEL_SPI			NRF_SPI0
#define ACCEL_OPEN_SPI()	{	/* Configure and enable the SPI module in master mode, select the device *
							ACCEL_SPI->POWER		= SPI_POWER_POWER_Enabled; 
							ACCEL_SPI->ENABLE		= SPI_ENABLE_ENABLE_Disabled;
							ACCEL_SPI->INTENCLR		= SPI_INTENCLR_READY_Msk;	   
							ACCEL_SPI->PSELSCK		= ACCEL_SCK;
							ACCEL_SPI->PSELMOSI		= ACCEL_SDI;
							ACCEL_SPI->PSELMISO		= ACCEL_SDO;
							ACCEL_SPI->FREQUENCY	= SPI_FREQUENCY_FREQUENCY_M8;   
							ACCEL_SPI->CONFIG		= 6UL;	/* MSB 1st, Active low, Sample 2nd *
							ACCEL_SPI->EVENTS_READY	= 0UL;	/* Clear ready event if set *
							ACCEL_SPI->ENABLE		= SPI_ENABLE_ENABLE_Enabled; }
#define ACCEL_CLOSE_SPI()	{	/* Disable the SPI module, de-select device, float outputs with pull-ups to save power *
							ACCEL_SPI->ENABLE		= SPI_ENABLE_ENABLE_Disabled;}


SPI transfer macros - All configured in HardwareProfile.h, called by the driver
extern uint32_t g_spi_rx_last;
#define ACCEL_SPI_WRITE(_x)	{ACCEL_SPI->TXD = (uint32_t)_x; while(!ACCEL_SPI->EVENTS_READY); g_spi_rx_last = ACCEL_SPI->RXD; ACCEL_SPI->EVENTS_READY = 0UL;}
#define ACCEL_SPI_READ_REG	((uint8_t)g_spi_rx_last)							
*/

// SPI protocol definitions
#define ACCEL_MASK_READ		0x80		/*SPI_READ_MASK*/
#define ACCEL_MASK_WRITE	0x00		/*SPI_WRITE_MASK*/
#define ACCEL_MASK_BURST	0x40		/*SPI_MULTIPLE_READ OR WRITE*/

// SPI read and write routines for NRF51
uint32_t g_spi_rx_last; // Required variable to buffer the rx byte in
static void ACCEL_SPI_Open(void)
{
nrf_delay_us(20);	// KL: Test
	NRF_GPIO->DIRSET = (1 << ACCEL_SCK)|(1 << ACCEL_SDI);	// Ensure output are enabled 
	ACCEL_OPEN_SPI();										// Enable the SPI module
	NRF_GPIO->OUTCLR = (1 << ACCEL_CS);						// Select the device, CS set low
}	
static void ACCEL_SPI_Reopen(void)
{
	volatile uint32_t delayCount;
	NRF_GPIO->OUTSET = (1 << ACCEL_CS);						// Deselect the device
	nrf_delay_us(10);										// Very brief delay
	NRF_GPIO->OUTCLR = (1 << ACCEL_CS);						// Select the device, CS set low
}	
static void ACCEL_SPI_Close(void )
{
	NRF_GPIO->OUTSET = (1 << ACCEL_CS);						// Deselect the device
	ACCEL_CLOSE_SPI();										// Disable the SPI module
	NRF_GPIO->DIRCLR = (1 << ACCEL_SCK)|(1 << ACCEL_SDI);	// Float outputs with pull-ups 
}				
static void ACCEL_SPI_SetReadReg(uint8_t reg)
{
	ACCEL_SPI_WRITE((reg|ACCEL_MASK_READ));
}
static void ACCEL_SPI_SetWriteReg(uint8_t reg)
{
	ACCEL_SPI_WRITE((reg|ACCEL_MASK_WRITE));
}
static uint8_t ACCEL_SPI_Read(void)
{
	ACCEL_SPI_WRITE(0xFF);	
	return ((uint8_t)ACCEL_SPI_READ_REG);
}
static void ACCEL_SPI_Write(uint8_t val)
{
	ACCEL_SPI_WRITE(val);	
}
static bool AccelRead(uint8_t* dest, uint8_t len, uint8_t reg)
{
	ACCEL_SPI_Open();
	if(len > 1) reg |= ACCEL_MASK_BURST;
	ACCEL_SPI_SetReadReg(reg);
	while(len--){*dest++ = ACCEL_SPI_Read();}
	ACCEL_SPI_Close();
    return true;
}
static bool AccelWrite(uint8_t* src, uint8_t len, uint8_t reg)
{
	ACCEL_SPI_Open();
	if(len > 1) reg |= ACCEL_MASK_BURST;
	ACCEL_SPI_SetWriteReg(reg);
	while(len--){ACCEL_SPI_Write(*src++);}
	ACCEL_SPI_Close();
	return true;
}
#else
// Using TWI mode

#define ACCEL_MASK_BURST	0x80		/*I2C MULTIPLE READ OR WRITE*/
#define ACCEL_MASK_READ		0x01		/*I2C READ MASK*/
#define TWI_TIMEOUT_COUNT	(20000UL)

#if(ACCEL_TWI_A0 == 1)
#define ACCEL_TWI_ADD	0x32
#else
#define ACCEL_TWI_ADD	0x30
#endif

static bool AccelTwiRecoverBus(void)
{
	bool recovered = false;
	if((NRF_GPIO->IN >> ACCEL_SCL) & 0x1UL)
	{
		if((NRF_GPIO->IN >> ACCEL_SDA) & 0x1UL)
		{
			// SCL and SDA are high - ok
			return true;
		}
		uint32_t count, twi_enabled = ACCEL_TWI->ENABLE;
		// Possible slave lockout, try unlocking
        ACCEL_TWI->ENABLE = 0;
		// Send several clocks at 25 kHz
		for(count = 18; count > 0; count--)
		{
			NRF_GPIO->OUTCLR = (1UL << ACCEL_SCL);
			nrf_delay_us(20);
			NRF_GPIO->OUTSET = (1UL << ACCEL_SCL);
			nrf_delay_us(20);
			// Check if resolved
			if((NRF_GPIO->IN >> ACCEL_SDA) & 0x1UL)
				break;
		}
		// Check it released the pin
		if( ((NRF_GPIO->IN >> ACCEL_SDA) & 0x1UL) &&
			((NRF_GPIO->IN >> ACCEL_SCL) & 0x1UL) )
		{
			// SCL and SDA are high - ok
			recovered = true;
		}
		// Turn on module if it was enabled before
        ACCEL_TWI->ENABLE = twi_enabled;
	}
	// SCL shouldn't be low if not driven
	return recovered;
}

static bool AccelTwiRecoverHw(void)
{
	// Recover the peripheral as indicated by PAN 56: "TWI: TWI module lock-up." found at
	// Product Anomaly Notification document found at 
	// https://www.nordicsemi.com/eng/Products/Bluetooth-R-low-energy/nRF51822/#Downloads
	ACCEL_TWI->EVENTS_ERROR = 0;
	ACCEL_TWI->ENABLE       = TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos; 
	ACCEL_TWI->POWER        = 0;
	nrf_delay_us(5);
	ACCEL_TWI->POWER        = 1;
	ACCEL_TWI->ENABLE       = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;
	return true;
}

#ifdef ACCEL_OPTIMISE_TWI
static __attribute__ ((section(".fast"))) bool AccelRead(uint8_t* dest, uint8_t len, uint8_t reg)
#else
static bool AccelRead(uint8_t* dest, uint8_t len, uint8_t reg)
#endif
{
	// Timeout counter
	uint32_t timeout;
	// Checks
	if((len == 0) || (dest == NULL))
		return false;
	// Initiate send of start + bus_address|write + register_address|burst
    NRF_SHORTS_BB_NONE();
	if(len > 1) reg |= ACCEL_MASK_BURST;
    ACCEL_TWI->TASKS_STARTTX = 1;
	ACCEL_TWI->TXD = reg;
	// Wait for send to complete
    timeout = TWI_TIMEOUT_COUNT;
	while (ACCEL_TWI->EVENTS_TXDSENT == 0 && ACCEL_TWI->EVENTS_ERROR == 0 && (--timeout)){;}
	// Clear events
    ACCEL_TWI->EVENTS_TXDSENT = 0;
	// Check for errors and recover
	if (timeout == 0 || ACCEL_TWI->EVENTS_ERROR != 0)
	{
		AccelTwiRecoverHw();
	}
	// Setup event shorts
	if(len > 1)	{NRF_SHORTS_BB_SUSPEND();}	// 2 or more bytes
	else		{NRF_SHORTS_BB_STOP();}		// 1 byte
	// Initiate send of repeat_start + bus_address|read + [rx-process]xN.... + stop
	ACCEL_TWI->EVENTS_RXDREADY = 0;
    ACCEL_TWI->EVENTS_STOPPED = 0;
	ACCEL_TWI->TASKS_STARTRX   = 1;

	// Read each byte
	while(1)
	{
		// Wait for read complete
		timeout = TWI_TIMEOUT_COUNT;
		while (ACCEL_TWI->EVENTS_RXDREADY == 0 && ACCEL_TWI->EVENTS_ERROR == 0 && (--timeout)){;}
		ACCEL_TWI->EVENTS_RXDREADY = 0;
		// Check for errors and recover
		if (timeout == 0 || ACCEL_TWI->EVENTS_ERROR != 0)
		{
			AccelTwiRecoverHw();
		}

		// Read the data byte
		*dest++ = ACCEL_TWI->RXD;
		len--;
		// Setup event shorts
		if(len > 1)			{NRF_SHORTS_BB_SUSPEND();}	// 2 or more bytes
		else if(len == 1)	{NRF_SHORTS_BB_STOP();}		// Last byte
		else	// Exit on complete
		{
			NRF_SHORTS_BB_NONE();
			break;
		}
		// Resume reception
#ifdef ACCEL_OPTIMISE_TWI
		__NOP(); // KL: Tested, faster + lower power. No idea how many nops are 'required'
#else
		nrf_delay_us(20); // Same as Nordic driver
#endif
		ACCEL_TWI->TASKS_RESUME = 1;
	}
	// Wait for stop
    timeout = TWI_TIMEOUT_COUNT;
    while (ACCEL_TWI->EVENTS_STOPPED == 0 && ACCEL_TWI->EVENTS_ERROR == 0 && (--timeout)){;}
    ACCEL_TWI->EVENTS_STOPPED = 0;
	return true;
}

static bool AccelWrite(uint8_t* src, uint8_t len, uint8_t reg)
{
	// Timout counter
	uint32_t timeout;
	// Checks
	if((len == 0) || (src == NULL))
		return false;
	// Initiate send of start + bus_address|write + register_address|burst
    NRF_SHORTS_BB_NONE();
	if(len > 1) reg |= ACCEL_MASK_BURST;
    ACCEL_TWI->TASKS_STARTTX = 1;
	ACCEL_TWI->TXD = reg;
	// Wait for send to complete
    timeout = TWI_TIMEOUT_COUNT;
	while (ACCEL_TWI->EVENTS_TXDSENT == 0 && ACCEL_TWI->EVENTS_ERROR == 0 && (--timeout)){;}
	// Clear events
    ACCEL_TWI->EVENTS_TXDSENT = 0;
	// Check for errors and recover
	if (timeout == 0 || ACCEL_TWI->EVENTS_ERROR != 0)
	{
		AccelTwiRecoverHw();
	}
	// Send each byte
	while(len)
	{
		// Send each data byte
		ACCEL_TWI->TXD = *src++;
		// Decrement count
		len--;
		// Wait for send to complete
		timeout = TWI_TIMEOUT_COUNT;
		while (ACCEL_TWI->EVENTS_TXDSENT == 0 && ACCEL_TWI->EVENTS_ERROR == 0 && (--timeout)){;}
		// Clear events
		ACCEL_TWI->EVENTS_TXDSENT = 0;
		// Check for errors and recover
		if (timeout == 0 || ACCEL_TWI->EVENTS_ERROR != 0)
		{
			AccelTwiRecoverHw();
		}
	}
	// Initiate stop sequence
	ACCEL_TWI->EVENTS_STOPPED = 0;
	ACCEL_TWI->TASKS_STOP     = 1;
	// Wait until stop sent
    timeout = TWI_TIMEOUT_COUNT;
    while (ACCEL_TWI->EVENTS_STOPPED == 0 && ACCEL_TWI->EVENTS_ERROR == 0 && (--timeout)){;}
    ACCEL_TWI->EVENTS_STOPPED = 0;
	return true;
}

#endif

// Read device ID
uint8_t AccelPresent(void)
{
	uint8_t id;	

    accelPresent = false;
	#ifdef ACCEL_TWI
	ACCEL_OPEN_TWI(ACCEL_TWI_ADD);
	if(AccelTwiRecoverBus() == false)

		return accelPresent;
	#elif defined ACCEL_SPI
	ACCEL_OPEN_SPI();
	#else
	#error "Select interface"
	#endif
	// Power off - prevent interference from data sources/interrupts
    AccelRead(&id, 1, ACCEL_ADDR_WHO_AM_I);
	accelPresent = (id == ACCEL_DEVICE_ID) ? 1 : 0;

	return accelPresent;
}

// Initialize the accelerometer

uint8_t AccelStartup(accel_settings_t* settings)
{

	uint8_t ctrl_reg1;
	// Exit if not present
	if(!accelPresent)return 0;
	#ifdef ACCEL_TWI
	ACCEL_OPEN_TWI(ACCEL_TWI_ADD);
	if(AccelTwiRecoverBus() == false)

		return accelPresent;
	#elif defined ACCEL_SPI
	ACCEL_OPEN_SPI();
	#else
	#error "Select interface"
	#endif
	// Accept new settings if set, otherwise use global
	if((settings != NULL) && (settings != &accel_regs))
	{
		// External settings provided
		memcpy(&accel_regs,settings,sizeof(accel_settings_t));
	}
	else
	{
		// No settings provided, set global settings to defaults then modify
		memcpy(&accel_regs,&accel_regs_startup,sizeof(accel_settings_t));
	}
	// Start with device off
	ctrl_reg1 = accel_regs.ctrl_reg1;
	accel_regs.ctrl_reg1 = 0;
	// Write registers

	AccelWriteRegs();
	// Next set the control1 register
	accel_regs.ctrl_reg1 = ctrl_reg1;
    AccelWrite(&accel_regs.ctrl_reg1, 1, ACCEL_ADDR_CTRL_REG1);
	// Then read back the register values



	AccelReadRegs();
	// User should check read results
	return 1;
}				

// Shut down the accelerometer (standby mode)
uint8_t AccelShutdown(void)
{
	// Exit if not present
	if(!accelPresent)return 0;
	// Power off sensor through control reg 1
	accel_regs.ctrl_reg1 = 0;
	// Write device register
    AccelWrite(&accel_regs.ctrl_reg1, 1, ACCEL_ADDR_CTRL_REG1);
	// Interface off
	#ifdef ACCEL_TWI
	ACCEL_CLOSE_TWI();
	#elif defined ACCEL_SPI
	ACCEL_CLOSE_SPI();
	#endif
	return 1;	
}

// Clear interrupts, read interrupt registers (FIFO may also need emptying)
uint8_t AccelReadEvents(void)
{
	// Exit if not present
	if(!accelPresent)return 0;
	
    AccelRead(&accel_regs.click_src, 1, ACCEL_ADDR_CLICK_SRC);
	AccelRead(&accel_regs.int1_src, 1, ACCEL_ADDR_INT1_SOURCE);
	AccelRead(&accel_regs.int2_src, 1, ACCEL_ADDR_INT2_SOURCE);
	AccelRead(&accel_regs.fifo_src, 1, ACCEL_ADDR_FIFO_SRC_REG);

	// For 6D orientation *change-only* detection 
	#ifdef ACCEL_DYNAMIC_6D_ORIENTATION_DETECT
	{
		// Disable detection of current orientation to stop re-interrupting
		uint8_t new_int1_cfg = 0xC0 | (~accel_regs.int1_src);
		// Update hardware physical register setting
		AccelWrite(&new_int1_cfg, 1, ACCEL_ADDR_INT1_CFG);
	}
	#endif

	return 1;	
}

// Reads a 3-axis value from the accelerometer 
uint8_t AccelReadSample(accel_t *value)
{
	// Exit if not present
	if(!accelPresent)return 0;	
	// Read the data registers
    AccelRead(&accel_regs.xl, sizeof(accel_t), ACCEL_ADDR_OUT_X_L);
	// Copy the global value to output if required
	if((value != NULL)&&((void*)value != &accel_regs.xl))
	{
		memcpy(&value->xl, &accel_regs.xl, 6);
	}
	return 1;
}

// Read FIFO queue length
uint8_t AccelReadCount(void)
{
	// Exit if not present
	if(!accelPresent)return 0;	
    AccelRead(&accel_regs.fifo_src, 1, ACCEL_ADDR_FIFO_SRC_REG);
	// Check overflow flag
	if(accel_regs.fifo_src & 0x40)
		return ACCEL_MAX_FIFO_SAMPLES;
	else
		return (accel_regs.fifo_src & 0x1F);
}

// Reads the accelerometer FIFO (bytes = 6 * entries)
uint8_t AccelReadFifo(accel_t *buffer, uint8_t maxEntries)
{
	uint8_t count;
	// Exit if not present
	if(!accelPresent)return 0;	
	AccelRead(&accel_regs.fifo_src, 1, ACCEL_ADDR_FIFO_SRC_REG);
	count = (accel_regs.fifo_src & 0x1F);
	if(maxEntries > count)
		maxEntries = count;
	else
		count = maxEntries;
	// Exit if no samples
	if(maxEntries == 0)
		return 0;
	// Set output data pointer or overwrite global values repeatedly
	if(buffer != NULL)
	{
		// If writing an external buffer
		AccelRead(&buffer->xl, sizeof(accel_t) * maxEntries, ACCEL_ADDR_OUT_X_L);
		// Point to last read sample
		buffer += maxEntries - 1;
		// Copy last sample to accelerometer regs variable
		memcpy(&accel_regs.xl, &buffer->xl, sizeof(accel_t));
	}
	else
	{
		// Null buffer pointer, dump all fifo samples
		while(maxEntries--)
			AccelRead(&accel_regs.xl, sizeof(accel_t), ACCEL_ADDR_OUT_X_L);
	}
	return count;
}

// Read all 'readable' registers to the global struct
void AccelReadRegs(void)
{
	uint8_t* regPtr;
	// Exit if not present
	if(!accelPresent)return;	
	// Read registers
	/* 0x1E->0x2D (xyz reg read wraps around, can't read in a one'r)*/
    AccelRead(&accel_regs.ctrl_reg0,(1 + ACCEL_ADDR_OUT_Z_H - ACCEL_ADDR_CTRL_REG0), ACCEL_ADDR_CTRL_REG0);
	/* 0x2E->0x3D */
	AccelRead(&accel_regs.fifo_ctrl,(1 + ACCEL_ADDR_TIME_WINDOW - ACCEL_ADDR_FIFO_CTRL_REG), ACCEL_ADDR_FIFO_CTRL_REG);
}

// Write all 'writeable' registers to the global struct
void AccelWriteRegs(void)
{
	uint8_t *regPtr;
	// Exit if not present
	if(!accelPresent)return;
	// Write registers
#ifdef LIS2DH // No ctrl_reg0
	AccelWrite(&accel_regs.temp_cfg, (1 + ACCEL_ADDR_REFERENCE - ACCEL_ADDR_TEMP_CFG_REG), ACCEL_ADDR_TEMP_CFG_REG);
#else
	AccelWrite(&accel_regs.ctrl_reg0, (1 + ACCEL_ADDR_REFERENCE - ACCEL_ADDR_CTRL_REG0), ACCEL_ADDR_CTRL_REG0);
#endif
	/* 0x2E */
	AccelWrite(&accel_regs.fifo_ctrl, 1, ACCEL_ADDR_FIFO_CTRL_REG);
	/* 0x30 */
	AccelWrite(&accel_regs.int1_cfg, 1, ACCEL_ADDR_INT1_CFG);
	/* 0x32->0x33 */
	AccelWrite(&accel_regs.int1_ths, (1 + ACCEL_ADDR_INT1_DURATION - ACCEL_ADDR_INT1_THS), ACCEL_ADDR_INT1_THS);
	/* 0x34 */
	AccelWrite(&accel_regs.int2_cfg, 1, ACCEL_ADDR_INT2_CFG);
	/* 0x36->0x38 */
	AccelWrite(&accel_regs.int2_ths, (1 + ACCEL_ADDR_CLICK_CFG - ACCEL_ADDR_INT2_THS), ACCEL_ADDR_INT2_THS);
	/* 0x3A->0x3D */
	AccelWrite(&accel_regs.click_ths, (1 + ACCEL_ADDR_TIME_WINDOW - ACCEL_ADDR_CLICK_THS), ACCEL_ADDR_CLICK_THS);
	return;
}

// Device settings translation. Values to register values
uint8_t AccelSetting(accel_settings_t* settings, uint8_t range, uint16_t rate)
{

	uint8_t ctrlreg, not_found = 0;
	// Check an output was provided or update the global copy to modified defaults
	if(settings == NULL)
	{
		// No settings provided, set global settings to defaults then modify
		memcpy(&accel_regs,&accel_regs_startup,sizeof(accel_settings_t));

		settings = &accel_regs;
	}
	
	// Rate 
	// Modify rate setting
	ctrlreg = settings->ctrl_reg1 & 0x0F; 
	switch (rate) {
		case (5000)	:{ctrlreg|=0b10010000;break;}
		case (1600)	:{ctrlreg|=0b10000000;break;}
		case (400)	:{ctrlreg|=0b01110000;break;}
		case (200)	:{ctrlreg|=0b01100000;break;}
		case (100)	:{ctrlreg|=0b01010000;break;}
		case (50)	:{ctrlreg|=0b01000000;break;}
		case (25)	:{ctrlreg|=0b00110000;break;}
		case (10)	:{ctrlreg|=0b00100000;break;}
		case (1)	:{ctrlreg|=0b00010000;break;}
		case (0)	:{ctrlreg =0b00000000;break;}	// Zero rate = Off	
		default	:{
			ctrlreg|=0b01000000;// Default is 50Hz
			not_found = 1;		// Indicate not supported
			break;
		}	
	}
	// Update control register 1
	settings->ctrl_reg1 = ctrlreg;
	
	// Range
	// Modify range setting
	ctrlreg = settings->ctrl_reg4 & 0xCF;		
	switch (range){
		case (2)	:{ctrlreg|= 0x00;break;}
		case (4)	:{ctrlreg|= 0x10;break;}
		case (8)	:{ctrlreg|= 0x20;break;}
		case (16)	:{ctrlreg|= 0x30;break;}
		default	:{
			ctrlreg|= 0x20;	// +/-8g
			not_found = 1;	// Indicate not supported
			break;
		}	
	}
	// Update control register 4
	settings->ctrl_reg4 = ctrlreg;
	// Return result
	return (not_found)?0:1;
}
