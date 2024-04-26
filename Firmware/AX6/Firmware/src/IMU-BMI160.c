/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */

// IMU-BMI160.c - Sensor interface
// K Ladha 2018

// Includes
#include "HardwareProfile.h"
#include <stdio.h>
#include <TimeDelay.h>
#include "bmi160.h"
#include "bmm150.h"
#include "IMU-BMI160.h"
#include "Peripherals/Accel.h"


// Variables - Device instance
struct bmi160_dev bmi160;
struct bmm150_dev bmm150;

unsigned char imu_sample_size = sizeof(accel_t); // 6 or 12 bytes depending on gyro
unsigned short imu_sample_rate = 0;
unsigned short imu_accel_range = 0;
unsigned short imu_gyro_range = 0;

uint8_t imu_int1_enabled = 0;
uint8_t imu_int2_enabled = 0;
    
struct bmi160_fifo_frame fifo_frame = {
	.length = 0,
	.data = NULL};

// Definitions
// SPI Control abstraction

volatile uint16_t dummy;

#define IMU_IRQ1			PORTDbits.RD0
#define IMU_IRQ2			PORTDbits.RD11
#define IMU_IRQ1_FLAG		IFS1bits.INT1IF
#define IMU_IRQ2_FLAG		IFS1bits.INT2IF
#define IMU_IRQ1_EN()		{IEC1bits.INT1IE = 1;}
#define IMU_IRQ1_OFF()		{IEC1bits.INT1IE = 0;}
#define IMU_IRQ2_EN()		{IEC1bits.INT2IE = 1;}
#define IMU_IRQ2_OFF()		{IEC1bits.INT2IE = 0;}

#define IMU_CS				LATCbits.LATC15
#define IMU_SPI_OPEN()		{IMU_CS = 0;}
#define IMU_SPI_CLOSE()		{IMU_CS = 1;}

#define IMU_SPI_INIT()		{INTCON2bits.INT1EP = 0;/*Rising edge int*/\
                            SPI1CON1L = 0x0; /*Disable module*/\
                            SPI1CON1L = 0x0260; /* 8bit master, clk idle high, xmit into idle */\
                            SPI1CON1H = 0x0000; /* No audio mode enabled */\
                            dummy = SPI1BUFL;/*Clear rx reg*/\
                            SPI1CON2L = 7; /* Unused, word length */\
                            SPI1STATL = 0; /* Clear status */\
                            SPI1STATH = 0; /* Clear extended status */\
                            SPI1BRGL = 0; /* Baud rate divisor, 16M/(2*(1+0))=8MHz*/\
                            SPI1IMSKL = 0x19ED; /* SPI ISR mask value, all on */\
                            SPI1IMSKH = 0x0; /* SPI watermark ISR mask off */\
							SPI1CON1L |= 0x8000;} /*Note: SMP = 1, CKE = 1, CKP = 1 */
#define IMU_SPI_OFF()		{SPI1CON1L = 0x0;/* Shut off */}

#define IMU_SPI_PUTC(_c)	{\
								dummy = SPI1BUFL;\
								SPI1BUFL = _c;\
								while(!SPI1STATLbits.SPIRBF);\
							}
#define IMU_SPI_GETC()		SPI1BUFL							

void IMU_InterruptsPause(void) 
{
    IMU_IRQ1_OFF();
    IMU_IRQ2_OFF();
}
void IMU_InterruptsResume(void)
{
    if(imu_int1_enabled)
        {IMU_IRQ1_EN();}
    if(imu_int2_enabled)
        {IMU_IRQ2_EN();}
} 

int8_t user_spi_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	IMU_SPI_OPEN();
	IMU_SPI_PUTC(0x80 | reg_addr);	
    if(data == NULL)
        while(len--) // Cope with NULL reads
        {
            IMU_SPI_PUTC(0xff); // Dummy write
            dummy = IMU_SPI_GETC();
        }
    else
        while(len--)
        {
            IMU_SPI_PUTC(0xff); // Dummy write
            *data++ = IMU_SPI_GETC();
        }
	IMU_SPI_CLOSE();
	return BMI160_OK;	
}

int8_t user_spi_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	IMU_SPI_OPEN();
	IMU_SPI_PUTC(0x7F & reg_addr);	
	while(len--)
	{
		IMU_SPI_PUTC(*data++);
	}
	IMU_SPI_CLOSE();
	return BMI160_OK;	
}

void user_delay_ms(uint32_t period)
{
	DelayMs(period);
}

/*wrapper function to match the signature of bmm150.read */
int8_t user_aux_read(uint8_t id, uint8_t reg_addr, uint8_t *aux_data, uint16_t len)
{
	int8_t rslt;
	/* Discarding the parameter id as it is redundant*/
        rslt = bmi160_aux_read(reg_addr, aux_data, len, &bmi160);
	return rslt;
}

/*wrapper function to match the signature of bmm150.write */
int8_t user_aux_write(uint8_t id, uint8_t reg_addr, uint8_t *aux_data, uint16_t len)
{
	int8_t rslt;
	/* Discarding the parameter id as it is redundant */
	rslt = bmi160_aux_write(reg_addr, aux_data, len, &bmi160);
	return rslt;
}
		
int8_t IMU_Init(void)
{
	int8_t rslt, settingCheck = 1;
    int8_t bmm_data_x_lsb_reg = BMM150_DATA_X_LSB;
	/* In BMM150 Mag data starts from register address 0x42 */
	
	// Enable SPI connection
	IMU_SPI_INIT();
	
	/* You may assign a chip select identifier to be handled later */
	bmi160.id = 0;
	bmi160.interface = BMI160_SPI_INTF;
	bmi160.read = user_spi_read;
	bmi160.write = user_spi_write;
	bmi160.delay_ms = user_delay_ms;	

	rslt = bmi160_init(&bmi160);
    if(rslt != BMI160_OK) 
    {
        return BMI160_E_DEV_NOT_FOUND; // Error
    }
	/* After the above function call, accel_cfg and gyro_cfg parameters in the device 
	structure are set with default values, found in the datasheet of the bmi160 */
	
    if(imu_sample_rate == 0)
    {
        // Device has been reset, no further config required
        return rslt;
    }
    
	/* Select the Output data rate, range and power mode of accelerometer bmi160 */
    switch(imu_sample_rate) {
        case 1 :    bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_0_78HZ; 
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_25HZ; 
                    if(imu_gyro_range != 0) settingCheck = 0;
                    break;
        case 2 :    bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_1_56HZ;                  
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_25HZ;
                    if(imu_gyro_range != 0) settingCheck = 0;
                    break;
        case 3 :    bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_3_12HZ; 
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_25HZ;
                    if(imu_gyro_range != 0) settingCheck = 0;
                    break;
        case 6 :    bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_6_25HZ; 
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_25HZ;
                    if(imu_gyro_range != 0) settingCheck = 0;
                    break;
        case 12 :   
        case 13 :   bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_12_5HZ; 
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_25HZ;
                    if(imu_gyro_range != 0) settingCheck = 0;
                    break;
        case 25 :   bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_25HZ;
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_25HZ;
                    break;
        case 50 :   bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_50HZ; 
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_50HZ;
                    break;
        case 100 :  bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_100HZ;
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_100HZ;
                    break;
        case 200 :  bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_200HZ;
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_200HZ;
                    break;
        case 400 :  bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_400HZ; 
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_400HZ;
                    break;
        case 800 :  bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_800HZ; 
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_800HZ;
                    break;
        case 1600 : bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_1600HZ; 
                    bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_1600HZ;
                    break;
        case 0 : 
        default : settingCheck = 0; break;          
    }
    switch(imu_accel_range) {
        case 2 : bmi160.accel_cfg.range = BMI160_ACCEL_RANGE_2G; break;
        case 4 : bmi160.accel_cfg.range = BMI160_ACCEL_RANGE_4G; break;
        case 8 : bmi160.accel_cfg.range = BMI160_ACCEL_RANGE_8G; break;
        case 16 : bmi160.accel_cfg.range = BMI160_ACCEL_RANGE_16G; break;
        case 0 : break;
        default : settingCheck = 0; break;        
    }
	/* Select the Output data rate, range, and power mode of Gyroscope bmi160 */
    switch(imu_gyro_range) {
        case 125 : bmi160.gyro_cfg.range = BMI160_GYRO_RANGE_125_DPS; break;
        case 250 : bmi160.gyro_cfg.range = BMI160_GYRO_RANGE_250_DPS; break;
        case 500 : bmi160.gyro_cfg.range = BMI160_GYRO_RANGE_500_DPS; break;
        case 1000 : bmi160.gyro_cfg.range = BMI160_GYRO_RANGE_1000_DPS; break;
        case 2000 : bmi160.gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS; break;        
        case 0 : break;
        default : settingCheck = 0; break;        
    }    

    // Filter configurations
	bmi160.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
	bmi160.gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;    

    // Settings invalid, device was just reset i.e. in lowest power configuration
    if(settingCheck == 0)
    {
        return BMI160_E_OUT_OF_RANGE;
    }
    
    // Invalid settings
    if((imu_sample_rate == 0) || (settingCheck == 0))
    {
        bmi160.accel_cfg.power = BMI160_ACCEL_SUSPEND_MODE; 
        bmi160.gyro_cfg.power = BMI160_GYRO_SUSPEND_MODE;        
    }
    // Accel and gyro both on
    else if(imu_gyro_range != 0)
    {
        bmi160.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE; 
        bmi160.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;
        imu_sample_size = sizeof(accel_t) * 2;
    }
    // Accel only modes - full power
    else if(imu_sample_rate > 100)             
    {
        bmi160.gyro_cfg.power = BMI160_GYRO_SUSPEND_MODE;
        bmi160.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;
        imu_sample_size = sizeof(accel_t);
    }
    // Accel low power modes, no fifo
    else
    {
        bmi160.gyro_cfg.power = BMI160_GYRO_SUSPEND_MODE;
        bmi160.accel_cfg.power = BMI160_ACCEL_LOWPOWER_MODE; // KL: p.13, breaks fifo functionality
        imu_sample_size = sizeof(accel_t);
    }        

    // Note: No 'gyro only' mode is available, deliberate, for compatibility

    if(bmi160.accel_cfg.power != BMI160_ACCEL_LOWPOWER_MODE)
    {
        // Configure the fifo
        rslt = bmi160_set_fifo_down((BMI160_ACCEL_FIFO_DOWN_ZERO | BMI160_GYRO_FIFO_DOWN_ZERO), &bmi160);  // No filters BMI160_ACCEL_FIFO_FILT_EN, BMI160_GYRO_FIFO_FILT_EN
        if(rslt != BMI160_OK) return rslt; // Error
        bmi160.fifo = &fifo_frame;
        // Set the fifo watermark threshold
        rslt = bmi160_set_fifo_wm(IMU_FIFO_WATERMARK, &bmi160);
        if(rslt != BMI160_OK) return rslt; // Error
        /* Disable other FIFO settings */
        rslt = bmi160_set_fifo_config(  BMI160_FIFO_CONFIG_1_MASK , BMI160_DISABLE, &bmi160);
        if(rslt != BMI160_OK) return rslt; // Error

        if(imu_gyro_range != 0) 
        {
            rslt = bmi160_set_fifo_config(	BMI160_FIFO_ACCEL | BMI160_FIFO_GYRO, BMI160_ENABLE, &bmi160);
            if(rslt != BMI160_OK) return rslt; // Error
        }
        else
        {
            rslt = bmi160_set_fifo_config(	BMI160_FIFO_ACCEL , BMI160_ENABLE, &bmi160);
            if(rslt != BMI160_OK) return rslt; // Error    
        }
    }
// For framed data use the following    
	/* Enable the required FIFO settings - all on */
//	rslt = bmi160_set_fifo_config(	BMI160_FIFO_HEADER |
//									BMI160_FIFO_TIME |
//									BMI160_FIFO_TAG_INT1 |
//									BMI160_FIFO_TAG_INT2 |
//									BMI160_FIFO_ACCEL |
//									BMI160_FIFO_GYRO |
//									BMI160_FIFO_AUX, BMI160_ENABLE, &bmi160);
    


// For use of the magnetometer, use the following    
//	/* Configure device structure for auxiliary bmi160 parameter */
//	bmi160.aux_cfg.aux_sensor_enable = 1; // auxiliary bmi160 enable
//	bmi160.aux_cfg.aux_i2c_addr = BMI160_AUX_BMM150_I2C_ADDR; // auxiliary sensor address
//	bmi160.aux_cfg.manual_enable = 1; // setup mode enable
//	bmi160.aux_cfg.aux_rd_burst_len = 2;// burst read of 2 byte
//
//	/* Configure the BMM150 device structure by 
//	mapping user_aux_read and user_aux_write */
//	bmm150.read = user_aux_read;
//	bmm150.write = user_aux_write;
//	bmm150.dev_id = BMM150_DEFAULT_I2C_ADDRESS; 
//	/* Ensure that sensor.aux_cfg.aux_i2c_addr = bmm150.id
//	   for proper sensor operation */
//	bmm150.delay_ms = user_delay_ms;
//	bmm150.intf = BMM150_I2C_INTF;
//
//	/* Initialize the auxiliary sensor interface */
//	rslt = bmi160_aux_init(&bmi160);
//
//	/* Auxiliary sensor is enabled and can be accessed from this point */
//
//	/* Configure the desired settings in auxiliary BMM150 sensor 
//	 * using the bmm150 APIs */
//
//	/* Initializing the bmm150 sensor */
//	rslt = bmm150_init(&bmm150);
//
//	/* Set the power mode and preset mode to enable Mag data sampling */
//	bmm150.settings.pwr_mode = BMM150_FORCED_MODE;
//	rslt = bmm150_set_op_mode(&bmm150);
//
//	bmm150.settings.preset_mode= BMM150_PRESETMODE_LOWPOWER;
//	rslt = bmm150_set_presetmode(&bmm150);
//
//	/* Set the auxiliary sensor to auto mode */
//	rslt = bmi160_set_aux_auto_mode(&bmm_data_x_lsb_reg , &bmi160);
//	
//	/* Configure the Auxiliary sensor either in auto/manual modes and set the 
//	polling frequency for the Auxiliary interface */	
//	bmi160.aux_cfg.aux_odr = BMI160_AUX_ODR_100HZ; 
//	rslt = bmi160_config_aux_mode(&bmi160);
//	

	/* Set the bmi160 configuration */
	rslt = bmi160_set_sens_conf(&bmi160);
    
	return rslt;	
}

void IMU_Off(void)
{
    int8_t rslt;
    // Disable interrupts
    IMU_IRQ1_OFF();
    IMU_IRQ2_OFF();
	// Turn off devices
	bmi160.accel_cfg.power = BMI160_ACCEL_SUSPEND_MODE; 
	bmi160.gyro_cfg.power = BMI160_GYRO_SUSPEND_MODE; 
	rslt = bmi160_set_power_mode(&bmi160);
//	bmm150.settings.pwr_mode = BMM150_SLEEP_MODE;
//	rslt = bmm150_set_op_mode(&bmm150);
	
	// Shut down SPI interface
	IMU_SPI_OFF();	
}

// Will enable the fifo watermark interrupt on IMU_INT1
void IMU_Enable_Interrupts(uint8_t flags, uint8_t pinMask)
{
	union bmi160_int_status interrupt;
	enum bmi160_int_status_sel int_status_sel;
	struct bmi160_int_settg int_config;
	int8_t result = BMI160_OK;
    
    // Flush the fifo
    result |= bmi160_set_fifo_flush(&bmi160);
    // Read interrupt sources to clear
    result |= bmi160_get_int_status(int_status_sel, &interrupt, &bmi160); 
        
	/* Select the interrupt channel/pin settings */
	int_config.int_pin_settg.output_en = 	BMI160_ENABLE;	// Enabling interrupt pins to act as output pin
	int_config.int_pin_settg.output_mode = 	BMI160_DISABLE;	// Choosing push-pull mode for interrupt pin
	int_config.int_pin_settg.output_type = 	BMI160_ENABLE;	// Choosing active high output
	int_config.int_pin_settg.edge_ctrl = 	BMI160_DISABLE;	// Choosing level triggered output 
	int_config.int_pin_settg.input_en = 	BMI160_DISABLE;	// Disabling interrupt pin to act as input
	int_config.int_pin_settg.latch_dur = 	BMI160_LATCHED;	// Latched output

    if(flags & ACCEL_INT_SOURCE_WATERMARK )
    {
        // Check settings if accel will be in low power mode - use data ready interrupt
        if(imu_gyro_range == 0 && imu_sample_rate <= 100)
        {
            /* Select the Interrupt channel/pin */
            int_config.int_channel = BMI160_INT_CHANNEL_2;// Interrupt channel
            /* Select the Interrupt type */
            int_config.int_type =  BMI160_ACC_GYRO_DATA_RDY_INT;
            /* Set the Any-motion interrupt */
            result |= bmi160_set_int_config(&int_config, &bmi160); /* sensor is an instance of the structure bmi160_dev  */
            // Flag to enable correct interrupt
            imu_int2_enabled = 1;            
        }   
        else // Normal fifo data modes
        {    
            /* Select the Interrupt channel/pin */
            int_config.int_channel = BMI160_INT_CHANNEL_1;// Interrupt channel
            /* Select the Interrupt type */
            int_config.int_type = BMI160_ACC_GYRO_FIFO_WATERMARK_INT ;
            int_config.fifo_WTM_int_en = 1;
            /* Set the Any-motion interrupt */
            result |= bmi160_set_int_config(&int_config, &bmi160); /* sensor is an instance of the structure bmi160_dev  */
            // Check if over run also used
            if(flags & ACCEL_INT_SOURCE_OVERRUN )
            {    
                /* Select the Interrupt channel/pin */
                int_config.int_channel = BMI160_INT_CHANNEL_1;// Interrupt channel
                /* Select the Interrupt type */
                int_config.int_type = BMI160_ACC_GYRO_FIFO_FULL_INT;
                int_config.fifo_full_int_en = 1;
                /* Set the Any-motion interrupt */
                result |= bmi160_set_int_config(&int_config, &bmi160); /* sensor is an instance of the structure bmi160_dev  */
            }    
            // Flag to enable correct interrupt
            imu_int1_enabled = 1;
        }
    }
    if(flags & ACCEL_INT_SOURCE_SINGLE_TAP )
    {
        /* Select the Interrupt channel/pin */
        int_config.int_channel = BMI160_INT_CHANNEL_1;// Interrupt channel
        /* Select the Interrupt type */
        int_config.int_type = BMI160_ACC_SINGLE_TAP_INT ;
        int_config.int_type_cfg.acc_tap_int.tap_data_src = 0; // filtered data
        int_config.int_type_cfg.acc_tap_int.tap_dur = 1; // 100ms
        int_config.int_type_cfg.acc_tap_int.tap_en = 1;
        int_config.int_type_cfg.acc_tap_int.tap_quiet = 1;
        int_config.int_type_cfg.acc_tap_int.tap_shock = 0;
        int_config.int_type_cfg.acc_tap_int.tap_thr = (32 / imu_accel_range);  // 1g            
        /* Set the Any-motion interrupt */
        result |= bmi160_set_int_config(&int_config, &bmi160); /* sensor is an instance of the structure bmi160_dev  */
        // Flag to enable correct interrupt
        imu_int1_enabled = 1;        
    }
    if(flags & ACCEL_INT_SOURCE_DOUBLE_TAP )
    {
        /* Select the Interrupt channel/pin */
        int_config.int_channel = BMI160_INT_CHANNEL_1;// Interrupt channel
        /* Select the Interrupt type */
        int_config.int_type =  BMI160_ACC_DOUBLE_TAP_INT;
        int_config.int_type_cfg.acc_tap_int.tap_data_src = 0; // filtered data
        int_config.int_type_cfg.acc_tap_int.tap_dur = 1; // 100ms
        int_config.int_type_cfg.acc_tap_int.tap_en = 1;
        int_config.int_type_cfg.acc_tap_int.tap_quiet = 1;
        int_config.int_type_cfg.acc_tap_int.tap_shock = 0;
        int_config.int_type_cfg.acc_tap_int.tap_thr = (32 / imu_accel_range);  // 1g           
        /* Set the Any-motion interrupt */
        result |= bmi160_set_int_config(&int_config, &bmi160); /* sensor is an instance of the structure bmi160_dev  */
        // Flag to enable correct interrupt
        imu_int1_enabled = 1;        
    }  
   
    // Read interrupt sources to clear
    result |= bmi160_get_int_status(int_status_sel, &interrupt, &bmi160); 
    
    // Checks
    if(result == BMI160_OK)
    {
    	// Wait for pins to latch, then latch interrupts
    	user_delay_ms(0);
        IMU_IRQ1_FLAG = IMU_IRQ1;
        IMU_IRQ2_FLAG = IMU_IRQ2;
        // Allow vectoring
        if(imu_int1_enabled){IMU_IRQ1_EN();}
        if(imu_int2_enabled){IMU_IRQ2_EN();}
    }
}

// Dummy functions from accel driver...
char accelPresent;

// Read device ID
unsigned char AccelVerifyDeviceId(void)
{
    uint16_t imu_sample_rate_save = imu_sample_rate;
    if(accelPresent)return accelPresent;
    // Only do this once, it resets the device if found
    accelPresent = (IMU_Init() == BMI160_OK) ? 1 : 0;
    imu_sample_rate = imu_sample_rate_save;
	return accelPresent;
}
// Set gyro range
void GyroSetRange(uint16_t gyroRange)
{
    // Update gyro range
    imu_gyro_range = gyroRange;
}
// AccelStartup - KL: Adapt old code to new variables
void AccelStartup(unsigned char samplingRate)
{
	unsigned char rateCode = samplingRate & 0x0f;
    unsigned char rangeCode = samplingRate & 0xf0;
	switch (rateCode)
	{
		case ACCEL_RATE_3200    : imu_sample_rate = 3200; break;
		case ACCEL_RATE_1600    : imu_sample_rate = 1600; break;
		case ACCEL_RATE_800     : imu_sample_rate = 800; break;
		case ACCEL_RATE_400     : imu_sample_rate = 400; break;
		case ACCEL_RATE_200     : imu_sample_rate = 200; break;
		case ACCEL_RATE_100     : imu_sample_rate = 100; break;
		case ACCEL_RATE_50      : imu_sample_rate = 50; break;
		case ACCEL_RATE_25      : imu_sample_rate = 25; break;
		case ACCEL_RATE_12_5    : imu_sample_rate = 12; break;
		case ACCEL_RATE_6_25    : imu_sample_rate = 6; break;
        default                 : imu_sample_rate = 0; break;
	}
	switch (rangeCode)
	{
		case ACCEL_RANGE_16G    : imu_accel_range = 16; break;
		case ACCEL_RANGE_8G     : imu_accel_range = 8; break;
		case ACCEL_RANGE_4G     : imu_accel_range = 4; break;
		case ACCEL_RANGE_2G     : imu_accel_range = 2; break;
        default                 : imu_accel_range = 0; imu_sample_rate = 0; break;        
	}
    
    // Set up the gyro range externally if gyro stream is required
    // Start the IMU sensor
    if(IMU_Init() != BMI160_OK)
    {
        imu_sample_rate = 0; // Error!
    }
    return;
}    
// Shutdown the ADXL to standby mode (standby mode, interrupts disabled)
void AccelStandby(void)
{
    IMU_Off();
}
void AccelSingleSample(accel_t *accelBuffer)
{
    int8_t rslt = BMI160_OK;
    rslt = user_spi_read(0, BMI160_ACCEL_DATA_ADDR, (uint8_t*)accelBuffer, sizeof(accel_t));    
}
void GyroSingleSample(accel_t *gyroBuffer)
{
    int8_t rslt = BMI160_OK;
    rslt = user_spi_read(0, BMI160_GYRO_DATA_ADDR, (uint8_t*)gyroBuffer, sizeof(accel_t));  
}
// Enable interrupts
void AccelEnableInterrupts(unsigned char flags, unsigned char pinMask)
{
    IMU_Enable_Interrupts(flags, pinMask);    
}
// Read FIFO queue length
unsigned char AccelReadFifoLength(void)
{
    uint16_t bytes_to_read = 0;
    int8_t rslt = user_spi_read(0, BMI160_FIFO_LENGTH_ADDR, (uint8_t*)&bytes_to_read, sizeof(uint16_t));
    if(rslt != BMI160_OK)
        return 0;
    return bytes_to_read / sizeof(accel_t); //  KL: Compatibility, should be div by imu_sample_size
}
// Read at most 'maxEntries' 3-axis samples (3 words = 6 bytes) from the ADXL FIFO into the specified RAM buffer
unsigned char AccelReadFIFO(accel_t *accelBuffer, unsigned char maxEntries)
{
    int8_t rslt;
    // Fifo read using BMI160 api
    uint16_t requested_length = (uint16_t)maxEntries * sizeof(accel_t); //  KL: Compatibility, should be multiply by imu_sample_size        
 
    // KL: Streamlined to avoid delays in reading registers
    uint16_t bytes_to_read = 0;
    rslt = user_spi_read(0, BMI160_FIFO_LENGTH_ADDR, (uint8_t*)&bytes_to_read, sizeof(uint16_t));
    if(bytes_to_read < requested_length)
    {
        rslt = user_spi_read(0, BMI160_FIFO_DATA_ADDR, (uint8_t*)accelBuffer, bytes_to_read);
        return (bytes_to_read / sizeof(accel_t));
    }
    else
    {
        rslt = user_spi_read(0, BMI160_FIFO_DATA_ADDR, (uint8_t*)accelBuffer, requested_length);
        return maxEntries;
    }
#if 0     
   // KL: Be careful to read in pairs of samples if using gyro
    bmi160.fifo->length = requested_length;         
    bmi160.fifo->data = (uint8_t*)accelBuffer;              
    rslt = bmi160_get_fifo_data(&bmi160);
    if(requested_length == bmi160.fifo->length)
        return maxEntries;
    return bmi160.fifo->length / sizeof(accel_t);    //  KL: Compatibility, should be div by imu_sample_size
#endif    
}
// Read tap status
unsigned char AccelReadTapStatus(void)
{
    // TODO: Setup interrupts for tap inputs
    // Unused for AX3 anyway - readIntSource() returns taps status as well
	return 0;
}
// Read interrupt source
unsigned char AccelReadIntSource(void)
{
    int8_t source = 0;    
    int8_t rslt;
    // KL: Streamlined to just use data interrupts
    int8_t bmi160_int_status1;

    // Read int status 1 reg
    rslt = user_spi_read(0, BMI160_INT_STATUS_ADDR + 1, (uint8_t*)&bmi160_int_status1, sizeof(int8_t)); 
	/* Check for fifo watermark interrupt */
	if (bmi160_int_status1 & 0x40)
	{
        source |= ACCEL_INT_SOURCE_WATERMARK;
	}
    if(bmi160_int_status1 & 0x20)
    {
        source |= ACCEL_INT_SOURCE_OVERRUN;
    }

#if 0    
	union bmi160_int_status int_status;
	enum bmi160_int_status_sel int_status_sel;
		
	/* Interrupt status selection to read all interrupts */
	int_status_sel = BMI160_INT_STATUS_ALL;
	rslt = bmi160_get_int_status(int_status_sel, &int_status, &bmi160);

	/* Check for fifo watermark interrupt */
	if (int_status.bit.fwm)
	{
        source |= ACCEL_INT_SOURCE_WATERMARK;
	}
    if(int_status.bit.ffull)
    {
        source |= ACCEL_INT_SOURCE_OVERRUN;
    }
	if (int_status.bit.s_tap)
	{
        source |= ACCEL_INT_SOURCE_SINGLE_TAP;
	}
    if(int_status.bit.d_tap)
    {
        source |= ACCEL_INT_SOURCE_DOUBLE_TAP;
    }    
#endif    
	return source;
}
// Return sample frequency
unsigned short AccelFrequency(void)
{
	return imu_sample_rate;
}
// Debug dump registers
void AccelDebugDumpRegisters(void)
{
}
// Packs a buffer of 16-bit (x,y,z) values into an output buffer (4 bytes per entry)
void AccelPackData(short *input, unsigned char *output)
{
    accel_t sample = *(accel_t*)input;
    unsigned short wordl, wordh, accelRange = imu_accel_range;

    // 0=±16g, 1=±8g, 2=±4g, 3=±2g
    switch(imu_accel_range){
        case 16 : accelRange = 0; sample.x >>=  3; sample.y >>=  3; sample.z >>=  3;break;        
        case 8 : accelRange = 1; sample.x >>=  4; sample.y >>=  4; sample.z >>=  4;break;
        case 4 : accelRange = 2; sample.x >>=  5; sample.y >>=  5; sample.z >>=  5;break;        
        case 2 : accelRange = 3; sample.x >>=  6; sample.y >>=  6; sample.z >>=  6;break; 
        default : break;
    }
    
    // Calculate low and high words of packet data
         if (accelRange == 3) { wordl = ((sample.x >> 3) & 0x03ff) | ((sample.y <<  7) & 0xfc00); wordh = 0xc000 | ((sample.z << 1) & 0x3ff0) | ((sample.y >> 9) & 0x000f); }
    else if (accelRange == 2) { wordl = ((sample.x >> 2) & 0x03ff) | ((sample.y <<  8) & 0xfc00); wordh = 0x8000 | ((sample.z << 2) & 0x3ff0) | ((sample.y >> 8) & 0x000f); }
    else if (accelRange == 1) { wordl = ((sample.x >> 1) & 0x03ff) | ((sample.y <<  9) & 0xfc00); wordh = 0x4000 | ((sample.z << 3) & 0x3ff0) | ((sample.y >> 7) & 0x000f); }
    else                      { wordl = ((sample.x     ) & 0x03ff) | ((sample.y << 10) & 0xfc00); wordh =          ((sample.z << 4) & 0x3ff0) | ((sample.y >> 6) & 0x000f); }

    // Output
    ((unsigned short*)output)[0] = wordl;
    ((unsigned short*)output)[1] = wordh;
    return;    
}
// (Mostly private) returns the setting code for the given values
unsigned short AccelSetting(int rate, int range)
{
	unsigned char rateCode = 0x00;
	
	switch (rate)
	{
		case 3200: rateCode |= ACCEL_RATE_3200; break;
		case 1600: rateCode |= ACCEL_RATE_1600; break;
		case  800: rateCode |= ACCEL_RATE_800;  break;
		case  400: rateCode |= ACCEL_RATE_400;  break;
		case  200: rateCode |= ACCEL_RATE_200;  break;
		case  100: rateCode |= ACCEL_RATE_100;  break;
		case   50: rateCode |= ACCEL_RATE_50;   break;
		case   25: rateCode |= ACCEL_RATE_25;   break;
		case   12: rateCode |= ACCEL_RATE_12_5; break;
		case    6: rateCode |= ACCEL_RATE_6_25; break;
	}
	
	switch (range)
	{
		case 16: rateCode |= ACCEL_RANGE_16G; break;
		case  8: rateCode |= ACCEL_RANGE_8G;  break;
		case  4: rateCode |= ACCEL_RANGE_4G;  break;
		case  2: rateCode |= ACCEL_RANGE_2G;  break;
	}
	
	return rateCode;
}
// [Common sensor API] Returns whether given settings are valid
char AccelValidSettings(unsigned short rateHz, unsigned short sensitivityG, unsigned long flags)
{
	return 0;
}	
// [Common sensor API] Starts the device with the given settings
void AccelStartupSettings(unsigned short rateHz, unsigned short sensitivityG, unsigned long flags)
{
	unsigned char rateCode;
	unsigned char interruptMask, pinMask;
	
	// Start accelerometer with given rate/sensitivity
	//rateCode = AccelSetting(rateHz, sensitivityG);
	imu_sample_rate = rateHz;
    imu_accel_range = sensitivityG;
    AccelStartup(rateCode);
	
	// Flags
	interruptMask = 0x00;
	pinMask = 0x00;
	
	// FIFO
	if (flags & ACCEL_FLAG_FIFO_INTERRUPTS) { interruptMask |= (ACCEL_INT_SOURCE_WATERMARK | ACCEL_INT_SOURCE_OVERRUN); }
	
	// TODO: Handle other flags
	
	// Set interrupts
	if (interruptMask != 0x00)
	{
		AccelEnableInterrupts(interruptMask, pinMask);
	}	
	
	return;    
}
// Export the fifo data into the output buffers - ONLY IN FRAMED DATA MODES
uint8_t IMU_ParseFrames(uint8_t* data, uint16_t len)
{
	uint16_t index = 0;	
	while(index < len)
	{
		// Get frame header
		uint8_t frame_header = data[index++];
		uint8_t frame_param = (frame_header & 0x1C) >> 2;				
		// Read data out of frame
		if(frame_header & 0xC0 == 0x40) // Control frame
		{
			// Control frames - Find frame type
			if(frame_param == 0)
			{
				// Skip frames (fifo has overflowed) - 1 byte
				index++;
				continue;
			}
			if(frame_param == 1)
			{
				// bmi160 time frame - 3 bytes
				index += 3;
				continue;
			}
			if(frame_param == 2)
			{
				// bmi160 settings changed frame - 1 bytes
				index++;
				continue;
			}	
			// Error, unknown control frame
			return BMI160_E_INVALID_INPUT;
		}
		else if(frame_header & 0xC0 == 0x80) // Data frame
		{
			// Read the frame data segments out depending on bits
			if(frame_param & 0x01)
			{
				// Accelerometer data - 6 bytes
				index += BMI160_FIFO_A_LENGTH;
			}
			if(frame_param & 0x02)
			{
				// Gyroscope data - 6 bytes
				index += BMI160_FIFO_G_LENGTH;
			}					
			if(frame_param & 0x04)
			{
				// Aux/Magnetometer data - 8 bytes
				index += BMI160_FIFO_M_LENGTH;
			}	
			if(frame_param == 0x00)
			{
				// End of data reached - stop parsing
				break;
			}			
			// Continue to next frame
			continue;
		}
		else
		{
			// Error - invalid frame
			return BMI160_E_INVALID_INPUT;
		}
	}// While data available	
	// Verify the read aligned all frames
	if(index != len)
	{
		// All the fifo frames did not add up to length - read error
		return BMI160_E_INVALID_INPUT;
	}	
	// Done...
	return BMI160_OK;
}