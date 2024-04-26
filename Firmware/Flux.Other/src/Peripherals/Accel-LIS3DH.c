// LIS3DH accelerometer interface. Initially for NRF51822 and SPI
// Karim Ladha 2016

// Includes
#include "HardwareProfile.h"
#include "Peripherals/Accel.h"

// I2C routines
#ifdef ACCEL_I2C_MODE
	#include "myi2c.h"
	#error "I2C routines not tested yet"
	// Set interface speed to default if not defined
	#ifndef ACCEL_I2C_RATE
		#define LOCAL_I2C_RATE		I2C_RATE_400kHZ
	#else
		#define LOCAL_I2C_RATE		ACCEL_I2C_RATE
	#endif
	#ifndef ACCEL_ADDRESS
		#define ACCEL_ADDRESS		0x30 	/*I2C address*/
	#endif
	#define ACCEL_MASK_READ  	0x01 		/*I2C_READ_MASK*/
	#define ACCEL_MASK_BURST  	0x00 		/*Implicit for all I2C access*/
	
	// I2C comms
	#define CUT_DOWN_I2C_CODE_SIZE
	#ifndef CUT_DOWN_I2C_CODE_SIZE
	#define ACCELOpen()              myI2COpen();myI2CStart(); WaitStartmyI2C();
	#define ACCELAddressRead(_r)     myI2Cputc(ACCEL_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(ACCEL_ADDRESS | I2C_READ_MASK);
	#define ACCELAddressWrite(_r)    myI2Cputc(ACCEL_ADDRESS); myI2Cputc((_r)); 
	#define ACCELReadContinue()      myI2Cgetc(); myI2CAck()
	#define ACCELReadLast()          myI2Cgetc(); myI2CNack()
	#define ACCELWrite(_v)           myI2Cputc((_v));
	#define ACCELClose()             myI2CStop(); WaitStopmyI2C();myI2CClose();
	#define ACCELReopen()            myI2CRestart(); WaitRestartmyI2C();
	#else
	void ACCELOpen(void)					{myI2COpen();myI2CStart(); WaitStartmyI2C();}
	void ACCELAddressRead(unsigned char _r)	{myI2Cputc(ACCEL_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(ACCEL_ADDRESS | I2C_READ_MASK);}
	void ACCELAddressWrite(unsigned char _r){myI2Cputc(ACCEL_ADDRESS); myI2Cputc((_r)); }
	unsigned char ACCELReadContinue(void)   {unsigned char ret = myI2Cgetc(); myI2CAck();return ret;}
	unsigned char ACCELReadLast(void)       {unsigned char ret = myI2Cgetc(); myI2CNack();return ret;}
	void ACCELWrite(unsigned char _v)       {myI2Cputc((_v));}
	void ACCELClose(void )             		{myI2CStop(); WaitStopmyI2C();myI2CClose();}
	void ACCELReopen(void )            		{myI2CRestart(); WaitRestartmyI2C();}
	#endif

#elif defined (ACCEL_SPI_MODE)

	#define ACCEL_MASK_READ  	0x80 	/*SPI_READ_MASK*/
	#define ACCEL_MASK_WRITE  	0x00 	/*SPI_WRITE_MASK*/
	#define ACCEL_MASK_BURST  	0x40 	/*SPI_MULTIPLE_READ OR WRITE*/
	
	// SPI read and write routines
	inline void WriteSPIx(unsigned int data)	{
												unsigned int dummy;
												if(ACCEL_SPI_Rx_Buf_Full)
													dummy = ACCEL_SPIBUF; 				// Clear flag
												while(ACCEL_SPI_Tx_Buf_Full);			// Dont write till last byte is txed	
												ACCEL_SPIBUF=(data);
												while(!ACCEL_SPI_Rx_Buf_Full);			// Wait till byte clocked in
												dummy = ACCEL_SPIBUF;					// Read it
												}												
	inline unsigned int ReadSPIx(void)			{
												unsigned int data;
												if(ACCEL_SPI_Rx_Buf_Full)
													data = ACCEL_SPIBUF; 				// Clear flag
												while(ACCEL_SPI_Tx_Buf_Full);			// Dont write till last byte is txed	
												ACCEL_SPIBUF=(0xff);
												while(!ACCEL_SPI_Rx_Buf_Full);			// Wait till byte clocked in
												data = ACCEL_SPIBUF;						// Read it
												return (data&0xff);
												}
	#define OpenSPIACCEL(){  \
	                    if (OSCCONbits.NOSC == 1) \
	                        ACCEL_OpenSPI((ENABLE_SCK_PIN | ENABLE_SDO_PIN | SPI_MODE8_ON | SPI_SMP_OFF | SPI_CKE_OFF | CLK_POL_ACTIVE_LOW | MASTER_ENABLE_ON | SEC_PRESCAL_1_1 | PRI_PRESCAL_4_1), /*config 1*/ \
									 (FRAME_ENABLE_OFF | FRAME_SYNC_OUTPUT | FRAME_SYNC_ACTIVE_LOW | SPI_FRM_PULSE_PREV_CLK | SPI_ENH_BUFF_DISABLE ), /*config 2*/ \
									 (SPI_ENABLE | SPI_IDLE_STOP | SPI_RX_OVFLOW_CLR) /*config 3*/ ); \
	                    else    \
	                        ACCEL_OpenSPI((ENABLE_SCK_PIN | ENABLE_SDO_PIN | SPI_MODE8_ON | SPI_SMP_OFF | SPI_CKE_OFF | CLK_POL_ACTIVE_LOW | MASTER_ENABLE_ON | SEC_PRESCAL_1_1 | PRI_PRESCAL_1_1), /*config 1*/ \
									 (FRAME_ENABLE_OFF | FRAME_SYNC_OUTPUT | FRAME_SYNC_ACTIVE_LOW | SPI_FRM_PULSE_PREV_CLK | SPI_ENH_BUFF_DISABLE ), /*config 2*/ \
									 (SPI_ENABLE | SPI_IDLE_STOP | SPI_RX_OVFLOW_CLR) /*config 3*/ ); \
	                     }	

	void ACCELOpen(void)					{OpenSPIACCEL();ACCEL_CS = 0;}
	void ACCELAddressRead(unsigned char _r)	{WriteSPIx(_r|ACCEL_MASK_READ);}
	void ACCELAddressWrite(unsigned char _r){WriteSPIx(_r|ACCEL_MASK_WRITE);}
	unsigned char ACCELReadContinue(void)   {return ReadSPIx();}
	unsigned char ACCELReadLast(void)       {return ReadSPIx();}
	void ACCELWrite(unsigned char _v)       {WriteSPIx(_v);}
	void ACCELClose(void )             		{ACCEL_CS = 1; ACCEL_CloseSPI();}
	void ACCELReopen(void )            		{ACCEL_CS = 1; ACCEL_CloseSPI();OpenSPIACCEL(); ACCEL_CS = 0;}
#endif


// The device id
#define ACCEL_DEVICE_ID		0x33 /*Staic response*/

#define 	ACCEL_ADDR_STATUS_REG_AUX	0x07
#define 	ACCEL_ADDR_OUT_ADC1_L		0x08
#define 	ACCEL_ADDR_OUT_ADC1_H		0x09
#define 	ACCEL_ADDR_OUT_ADC2_L		0x0A
#define 	ACCEL_ADDR_OUT_ADC2_H		0x0B
#define 	ACCEL_ADDR_OUT_ADC3_L		0x0C
#define 	ACCEL_ADDR_OUT_ADC3_H		0x0D
#define 	ACCEL_ADDR_INT_COUNTER_REG	0x0E
#define 	ACCEL_ADDR_WHO_AM_I			0x0F
#define 	ACCEL_ADDR_TEMP_CFG_REG		0x1F
#define 	ACCEL_ADDR_CTRL_REG1		0x20
#define 	ACCEL_ADDR_CTRL_REG2		0x21
#define 	ACCEL_ADDR_CTRL_REG3		0x22
#define 	ACCEL_ADDR_CTRL_REG4		0x23
#define 	ACCEL_ADDR_CTRL_REG5		0x24
#define 	ACCEL_ADDR_CTRL_REG6		0x25
#define 	ACCEL_ADDR_REFERENCE		0x26
#define 	ACCEL_ADDR_STATUS_REG2		0x27
#define 	ACCEL_ADDR_OUT_X_L			0x28
#define 	ACCEL_ADDR_OUT_X_H			0x29
#define 	ACCEL_ADDR_OUT_Y_L			0x2A
#define 	ACCEL_ADDR_OUT_Y_H			0x2B
#define 	ACCEL_ADDR_OUT_Z_L			0x2C
#define 	ACCEL_ADDR_OUT_Z_H			0x2D
#define 	ACCEL_ADDR_FIFO_CTRL_REG	0x2E
#define 	ACCEL_ADDR_FIFO_SRC_REG		0x2F
#define 	ACCEL_ADDR_INT1_CFG			0x30
#define 	ACCEL_ADDR_INT1_SOURCE		0x31
#define 	ACCEL_ADDR_INT1_THS			0x32
#define 	ACCEL_ADDR_INT1_DURATION	0x33
#define 	ACCEL_ADDR_CLICK_CFG		0x38
#define 	ACCEL_ADDR_CLICK_SRC		0x39
#define 	ACCEL_ADDR_CLICK_THS		0x3A
#define 	ACCEL_ADDR_TIME_LIMIT		0x3B
#define 	ACCEL_ADDR_TIME_LATENCY		0x3C
#define 	ACCEL_ADDR_TIME_WINDOW		0x3D

#ifndef ACCEL_CTRL_REG2_VAL
	#define ACCEL_CTRL_REG2_VAL 0b00000000
#else
	#warning "Accel ctrlreg2 not default, hpf off?" 
#endif

//static unsigned char accelRate = 0;
static unsigned char accelRange = 0;
static unsigned short accelFrequency = 0;

//unsigned char 	AccelRate(void) 	 { return accelRate; }
unsigned short 	AccelFrequency(void) { return accelFrequency; }
char 			accelPresent = 0;


// Read device ID
unsigned char AccelVerifyDeviceId(void)
{
	static unsigned char id;	
	// Power off - prevent interference from data sources/interrupts
	ACCEL_INT1_IE = 0;
	ACCEL_INT2_IE = 0;
	ACCELOpen();	
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG1 | ACCEL_MASK_BURST); 
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG1	- device off
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG2	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG3	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG4	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG5	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG6	
	// Read id register
	ACCELReopen();	
	ACCELAddressRead(ACCEL_ADDR_WHO_AM_I);
	id = ACCELReadLast();
	ACCELClose();
	// Compare to expected
    accelPresent = (id == ACCEL_DEVICE_ID) ? 1 : 0;
	return accelPresent;
}


// AccelStartup
void AccelStartup(unsigned char setting)
{
	unsigned char ctrlreg1, ctrlreg4;
	// Blocks access if not detected
	if (!accelPresent) return;

	// DISABLE INTERRUPTS
	ACCEL_INT1_IE = 0;
	ACCEL_INT2_IE = 0;

	ACCELOpen();	
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG1 | ACCEL_MASK_BURST); 
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG1	- device off
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG2	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG3	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG4	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG5	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG6	

	ACCELReopen();
	ACCELAddressWrite(ACCEL_ADDR_FIFO_CTRL_REG); 
	ACCELWrite(0x00);	/*Bypass mode - for single samples 0x00, fifo bypass*/

	//accelRate = setting&0xf; - legacy only
	ctrlreg1 = 0b00000111; // Init - axis all on
	switch (setting&0xf) {
	case (ACCEL_RATE_3200) 	:	{ctrlreg1|=0b10011000;accelFrequency  = 5000;break;} // 5000Hz	Note: low power only
	case (ACCEL_RATE_1600) 	:	{ctrlreg1|=0b10001000;accelFrequency  = 1600;break;} // 1600Hz Note: low power only
	case (ACCEL_RATE_800) 	:	{ctrlreg1|=0b10010000;accelFrequency  = 1250;break;} // 1250Hz
	case (ACCEL_RATE_400) 	:	{ctrlreg1|=0b01110000;accelFrequency  = 400;break;} // 400Hz
	case (ACCEL_RATE_200) 	: 	{ctrlreg1|=0b01100000;accelFrequency  = 200;break;} // 200Hz
	case (ACCEL_RATE_100) 	:	{ctrlreg1|=0b01010000;accelFrequency  = 100;break;} // 100Hz
	case (ACCEL_RATE_50) 	:	{ctrlreg1|=0b01000000;accelFrequency  = 50;break;} // 50Hz
	case (ACCEL_RATE_25) 	:	{ctrlreg1|=0b00110000;accelFrequency  = 25;break;} // 25Hz
	case (ACCEL_RATE_12_5) 	:	{ctrlreg1|=0b00100000;accelFrequency  = 10;break;} // 10Hz
	case (ACCEL_RATE_6_25) 	:	{ctrlreg1|=0b00010000;accelFrequency  = 10;break;} // 10Hz
	case (ACCEL_RATE_3_125)	:	{ctrlreg1|=0b00010000;accelFrequency  = 1;break;} // 1Hz
	case (ACCEL_RATE_1_56)	:	{ctrlreg1|=0b00010000;accelFrequency  = 1;break;} // 1Hz
	default	:	{accelFrequency=100;ctrlreg1|=0b01010000;break;}}/*100Hz default*/

	//accelRange = setting&0xf0; - legacy only
	ctrlreg4 = 0x80; 		// Init BDU on, Little endian data, High res off, Test off, 4 wire SPI
	switch (setting&0xf0){
		case (ACCEL_RANGE_16G) 	: {accelRange = 16; ctrlreg4 |= 0x30 ;break;}
		case (ACCEL_RANGE_8G) 	: {accelRange = 8; ctrlreg4 |= 0x20 ;break;}
		case (ACCEL_RANGE_4G) 	: {accelRange = 4; ctrlreg4 |= 0x10 ;break;}
		case (ACCEL_RANGE_2G) 	: {accelRange = 2; ctrlreg4 |= 0x00 ;break;}
		default 				: {accelRange = 4; ctrlreg4 |= 0x10 ;break;} // 4g default
	}

	ACCELReopen();
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG1 | ACCEL_MASK_BURST); 
	ACCELWrite(ctrlreg1); 	// ACCEL_ADDR_CTRL_REG1 -> Set above
	ACCELWrite(ACCEL_CTRL_REG2_VAL); // ACCEL_ADDR_CTRL_REG2 -> High pass filtering disabled
	ACCELWrite(0b00000000); // ACCEL_ADDR_CTRL_REG3 -> Interrupts all disabled
	ACCELWrite(ctrlreg4); 	// ACCEL_ADDR_CTRL_REG4 -> BDU on, Little endian data, FS = +/- 4g, High res off, Test off, 4 wire SPI 
	ACCELWrite(0b00000000); // ACCEL_ADDR_CTRL_REG5 -> Defaults, FIFO off, IRQ non latched, 4D off
	ACCELWrite(0b00000000); // ACCEL_ADDR_CTRL_REG6 -> INT2 click off, INT1 direct to INT2 , Boot status on int2 off, Interrupts active high
	ACCELWrite(0b00000000); // ACCEL_ADDR_REFERENCE -> Reference for int generation, 0 (assume its a counter?)

	ACCELClose();
	return;
}

// Shutdown the accelerometer to standby mode (standby mode, interrupts disabled)
void AccelStandby(void)
{
	// Blocks access if not detected
	if (!accelPresent)  return;

	// Power off
	ACCELOpen();
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG1);
	ACCELWrite(0);
	ACCELClose();
}


/*
	Not all pin masks are possible for this device, 
	data ready interrupts must be on int1 and can not be remapped
	click and orientation interrupt1 can be remapped to int2
*/
// Enable interrupts - FIFO enabled and interrupting
void AccelEnableInterrupts(unsigned char flags, unsigned char pinMask)
{
	unsigned char ctrlreg1, ctrlreg2, ctrlreg3, ctrlreg4, ctrlreg5, ctrlreg6;
	unsigned char int1_cfg, click_cfg, fifoctrlreg;
	unsigned char click_threshold, click_time_limit, click_latency, click_window;
	unsigned char act_threshold = 0, act_duration = 0;

	// Blocks access if not detected
	if (!accelPresent) return;

	// DISABLE INTERRUPTS
	ACCEL_INT1_IE = 0;
	ACCEL_INT2_IE = 0;

	/*POWER OFF DEVICE, cache ctrlreg1*/
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_CTRL_REG1 | ACCEL_MASK_BURST);
	ctrlreg1 = ACCELReadContinue(); 	// Cache ctr1
	ctrlreg2 = ACCELReadContinue();		// Cache ctr2	
	ctrlreg3 = ACCELReadContinue();		// Cache ctr3
	ctrlreg4 = ACCELReadContinue();		// Cache ctr4
	ctrlreg5 = ACCELReadContinue();		// Cache ctr5
	ctrlreg6 = ACCELReadLast();			// Cache ctr6

	ACCELReopen();	
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG1 | ACCEL_MASK_BURST); 
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG1	- device off
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG2	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG3	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG4	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG5	
	ACCELWrite(0); // ACCEL_ADDR_CTRL_REG6	
	
	// Clear fifo control
	ACCELReopen();
	ACCELAddressWrite(ACCEL_ADDR_FIFO_CTRL_REG); 
	ACCELWrite(0);	

	// Init other affected control regs - (N.B. must clear ctrl regs due to OR masks)
	ctrlreg2 = 0;
	ctrlreg3 = 0; 
	ctrlreg5 = 0;
	ctrlreg6 = 0;
	int1_cfg = 0;
	click_cfg = 0;
	fifoctrlreg = 0;

	// Watermark int - always int1
	if (flags & ACCEL_INT_SOURCE_WATERMARK) 
	{ 	
		ctrlreg3 |= 0x4;
		ctrlreg5 |= 0x40; 
		fifoctrlreg = (0x80 | (ACCEL_FIFO_WATERMARK-1));// Fifo on, stream
	}
	// Data ready int - always int1
	if (flags & ACCEL_INT_SOURCE_DATA_READY) 	
	{
		ctrlreg3 |= 0x10;
	}
	// Activity detect
	if (flags & ACCEL_INT_SOURCE_ACTIVITY)
	{
		ctrlreg2 |= 0x81; 	// Normal HPF for AOI1 (prevent gravity asserting interrupt)
		int1_cfg |= 0x7f;	// Enable all axis movement sensing 
		act_threshold = 3;	// Set activity threshold
		act_duration = 0;	// Set activity duration
		if (pinMask & ACCEL_INT_SOURCE_ACTIVITY)
			ctrlreg3 |= 0x40; // Send threshold ints to int1
		else
			ctrlreg6 |= 0x40; // Send threshold ints to int2 (according to app note only, not in datasheet!!!)
		
	}
	// Orientation interrupt
	if (flags & ACCEL_INT_SOURCE_ORIENTATION) 	
	{
		int1_cfg |= 0x7f;	// Enable all axis position sensing 
		act_threshold = 12;	// Set activity threshold (at +/-8g, 12 (.75g) is ok)
		act_duration = 50;	// Set duration
		if (pinMask & ACCEL_INT_SOURCE_ORIENTATION)
			ctrlreg3 |= 0x40; // Send threshold ints to int1
		else
			ctrlreg6 |= 0x40; // Send threshold ints to int2 (according to app note only, not in datasheet!!!)
	
	}
	// Tap or double tap detection
	if (flags & (ACCEL_INT_SOURCE_SINGLE_TAP | ACCEL_INT_SOURCE_DOUBLE_TAP))	
	{
		if (flags & ACCEL_INT_SOURCE_SINGLE_TAP)click_cfg |= 0b00010101;
		if (flags & ACCEL_INT_SOURCE_DOUBLE_TAP)click_cfg |= 0b00101010;
		ctrlreg2 |= 0x84; 		// Normal HPF for click 
		click_threshold	 = 50; 	// Set click threshold
		click_time_limit = 5;	// Set max click duration (must go below threshold within this time)		
		click_latency = 10;		// Set click latency, duration pin is driven and click debounced	
		click_window = 50;		// Set window limit for double click	
		if (pinMask & (ACCEL_INT_SOURCE_SINGLE_TAP | ACCEL_INT_SOURCE_DOUBLE_TAP))
			ctrlreg3 |= 0x80; // Send click ints to int1
		else
			ctrlreg6 |= 0x80; // Send click ints to int2 (according to app note only, not in datasheet!!!)
		
	}

	// Setup for tap thresholds if used
	if (flags & (ACCEL_INT_SOURCE_DOUBLE_TAP|ACCEL_INT_SOURCE_SINGLE_TAP))
	{
		ACCELReopen();	
		ACCELAddressWrite(ACCEL_ADDR_CLICK_THS | ACCEL_MASK_BURST); 
		ACCELWrite(click_threshold); 	// Threshold
		ACCELWrite(click_time_limit); 	// Time limit 
		ACCELWrite(click_latency); 		// Latency
		ACCELWrite(click_window); 		// Window (double click)

		ACCELReopen();	
		ACCELAddressWrite(ACCEL_ADDR_CLICK_CFG); 
		ACCELWrite(click_cfg); 			// Enable selection
	}

	// Setup for activity thresholds if used
	if (flags & (ACCEL_INT_SOURCE_ACTIVITY | ACCEL_INT_SOURCE_ORIENTATION))
	{
		ACCELReopen();	
		ACCELAddressWrite(ACCEL_ADDR_INT1_THS | ACCEL_MASK_BURST); 
		ACCELWrite(act_threshold); 	// Threshold
		ACCELWrite(act_duration); 	// Duration
	}

	ACCELReopen();
	ACCELAddressWrite(ACCEL_ADDR_INT1_CFG); 
	ACCELWrite(int1_cfg); 

	// Write remaining registers
	ACCELReopen();
	ACCELAddressWrite(ACCEL_ADDR_FIFO_CTRL_REG); 
	ACCELWrite(fifoctrlreg);

	// Write new ctrlreg values
  	ACCELReopen();	
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG2 | ACCEL_MASK_BURST); 
	ACCELWrite(ctrlreg2); 
	ACCELWrite(ctrlreg3); 
	ACCELWrite(ctrlreg4); 
	ACCELWrite(ctrlreg5); 
	ACCELWrite(ctrlreg6); 

	// TURN ON DEVICE
	ACCELReopen();	
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG1); 
	ACCELWrite(ctrlreg1); /*Re-enable device*/

	// End comms
	ACCELClose();

	// Empty the fifo, read tap and orientation regs to clear flags
	AccelReadTapStatus();
	AccelReadOrientaion();
	AccelReadTransientSource();
while(ACCEL_INT1)
{	
	fifoctrlreg = AccelReadFIFO(NULL,32);
}

	ACCEL_INT1_IP = ACCEL_INT_PRIORITY;
	ACCEL_INT2_IP = ACCEL_INT_PRIORITY;

	// Now enable the interrupts
	if(pinMask&flags) // If any ints mapped to int1
	{
		ACCEL_INT1_IF = ACCEL_INT1;
		ACCEL_INT1_IE = 1;
	}
	if(flags && (pinMask&flags)!=flags) //If any ints enabled on int2 
	{
		ACCEL_INT2_IF = 0;
		ACCEL_INT2_IE = 1;
	}
}



// Read number of bytes in fifo - if the fifo has not been initialised this will return unknown
unsigned char AccelReadFifoLength(void)
{
	unsigned char number_in_fifo;
	// Blocks access if not detected
	if (!accelPresent)  return 0;

	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_FIFO_SRC_REG); // Bottom 5 bits are number of samples
	number_in_fifo = ACCELReadLast();
	if (number_in_fifo & 0x20) number_in_fifo = 0; 			// Empty flag set
	else if (number_in_fifo & 0x40) number_in_fifo = 32; 	// Overrun flag set
	else number_in_fifo &= 0x1f;							// Otherwise take actual value	
	ACCELClose();
	return number_in_fifo;
}

/*Note: The samples are Left justified signed and 12bit*/
void AccelSingleSample(accel_t *value)
{
	// Blocks access if not detected
	if (accelPresent==0)  
	{
		value->x = 0;
		value->y = 0;
		value->z = 0;	
		return;
	}

#ifdef ACCEL_8BIT_MODE // In 8 bit mode we just take the top data byte (left justified data)
	volatile unsigned char dummy;
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_OUT_X_L | ACCEL_MASK_BURST);
	dummy = ACCELReadContinue();
	value->x = ACCELReadContinue();
	dummy = ACCELReadContinue();
	value->y = ACCELReadContinue();
	dummy = ACCELReadContinue();
	value->z = ACCELReadLast();
	ACCELClose();
#else
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_OUT_X_L | ACCEL_MASK_BURST);
	value->xl = ACCELReadContinue();
	value->xh = ACCELReadContinue();
	value->yl = ACCELReadContinue();
	value->yh = ACCELReadContinue();
	value->zl = ACCELReadContinue();
	value->zh = ACCELReadLast();
	ACCELClose();
#endif
	return;
}


// Read at most 'maxEntries' 3-axis samples (3 words = 6 bytes) from the accelerometer FIFO into the specified RAM buffer
unsigned char AccelReadFIFO(accel_t *accelBuffer, unsigned char maxEntries)
{
	unsigned char number_in_fifo, number_read=0;
	unsigned char * ptr = (unsigned char*)accelBuffer; // Pointer cast to bytes
	// Blocks access if not detected
	if (!accelPresent) return 0;

	// Early out
	if (maxEntries == 0) return 0;

	// Early out
	number_in_fifo = AccelReadFifoLength();
	if (number_in_fifo == 0) return 0;

	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_OUT_X_L | ACCEL_MASK_BURST); // The first reg to read will be F_STATUS

	while ((number_in_fifo>1)&&(maxEntries>1))
	{
		if (accelBuffer == NULL)
			{ACCELReadContinue();ACCELReadContinue();ACCELReadContinue();
			ACCELReadContinue();ACCELReadContinue();ACCELReadContinue();}
		else
		{
#ifdef ACCEL_8BIT_MODE // In 8 bit mode we just take the top data byte (left justified data)
volatile unsigned char dummy;
			dummy 	 = 	ACCELReadContinue(); // XL
			*(ptr) = 	ACCELReadContinue(); // XH
			dummy = 	ACCELReadContinue(); // YL
			*(ptr+1) = 	ACCELReadContinue(); // YH
			dummy = 	ACCELReadContinue(); // ZL
			*(ptr+2) = 	ACCELReadContinue(); // ZH
			ptr+=3;
#else
			*(ptr) 	 = 	ACCELReadContinue(); // XL
			*(ptr+1) = 	ACCELReadContinue(); // XH
			*(ptr+2) = 	ACCELReadContinue(); // YL
			*(ptr+3) = 	ACCELReadContinue(); // YH
			*(ptr+4) = 	ACCELReadContinue(); // ZL
			*(ptr+5) = 	ACCELReadContinue(); // ZH
			ptr+=6;
#endif
		}
		number_in_fifo--;
		maxEntries--;
		number_read++;		
	}

	if (accelBuffer == NULL)
		{ACCELReadContinue();ACCELReadContinue();ACCELReadContinue();
		ACCELReadContinue();ACCELReadContinue();ACCELReadLast();}
	else
	{
#ifdef ACCEL_8BIT_MODE // In 8 bit mode we just take the top data byte (left justified data)
volatile unsigned char dummy;
			dummy	 = 	ACCELReadContinue(); // XL
			*(ptr) = 	ACCELReadContinue(); // XH
			dummy = 	ACCELReadContinue(); // YL
			*(ptr+1) = 	ACCELReadContinue(); // YH
			dummy = 	ACCELReadContinue(); // ZL
			*(ptr+2) = 	ACCELReadContinue(); // ZH
#else
			*(ptr) 	 = 	ACCELReadContinue(); // XL
			*(ptr+1) = 	ACCELReadContinue(); // XH
			*(ptr+2) = 	ACCELReadContinue(); // YL
			*(ptr+3) = 	ACCELReadContinue(); // YH
			*(ptr+4) = 	ACCELReadContinue(); // ZL
			*(ptr+5) = 	ACCELReadContinue(); // ZH
#endif
	}
	ACCELClose();
	number_read++;

	return number_read;
}


// Read tap status
unsigned char AccelReadTapStatus(void)
{
	unsigned char tap_status_reg;
	// Blocks access if not detected
	if (!accelPresent) return 0;
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_CLICK_SRC);
	tap_status_reg = ACCELReadLast();
	ACCELClose();
	/*
	b7 - na
	b6 - asserted
	b5 - DCLICK ON
	b4 - SCLICK ON
	b3 - SIGN
	b2 - Z
	b1 - Y
	b0 - X
	*/
	return tap_status_reg;
}

// Read transient status or orientation result
unsigned char AccelReadTransientSource(void) {return(AccelReadOrientaion());}
unsigned char AccelReadOrientaion(void)
{
	unsigned char int1_source;
	// Blocks access if not detected
	if (!accelPresent) return 0;

	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_INT1_SOURCE);
	int1_source = ACCELReadLast();
	ACCELClose();
	/*
	b7 - na
	b6 - asserted
	b5 - ZH
	b4 - ZL
	b3 - YH
	b2 - YL
	b1 - XH
	b0 - XL
	*/
	return int1_source;
}

// Read interrupt source
unsigned char AccelReadIntSource(void)
{
	// Blocks access if not detected
	if (!accelPresent) return 0;	
	// Its up to the user to know this for this device	
	return 0;
}


// Debug dump registers
#if 0
void AccelDebugDumpRegisters(void)
{
	static unsigned char i,regs[128];
	for (i=0;i<127;i++)
	{
		ACCELOpen();	
		ACCELAddressRead(i|ACCEL_MASK_READ); 
		regs[i] = ACCELReadLast();
		ACCELClose();	
	}

	Nop();
	Nop();

	Nop();
	Nop();
 	return;
}
#endif

// 
void AccelPackData(short *input, unsigned char *output)
{
	// This function could be implemented if <14 bits were needed
	// For 14 bits, the best byte-aligned packing is: 4x 3-axis samples packed into 21 bytes instead of 24 bytes,
    // or word-aligned packing: 8x 3-axis samples packed into 42 bytes, instead of 48 bytes.
}



// Returns the setting code for the given values
unsigned short AccelSetting(int rate, int range)
{
    unsigned short value = 0x0000;
    switch (rate)
    {
		case 5000: value |= ACCEL_RATE_3200;  break;
        case 3200: value |= 0x8000 | ACCEL_RATE_3200;  break;       
        case 1600: value |= ACCEL_RATE_1600;  break;        
		case 1250: value |= ACCEL_RATE_800;  break;
        case  800: value |= 0x8000 | ACCEL_RATE_800;   break;
        case  400: value |= ACCEL_RATE_400;   break;
        case  200: value |= ACCEL_RATE_200;   break;
        case  100: value |= ACCEL_RATE_100;   break;
        case   50: value |= ACCEL_RATE_50;    break;
        case   25: value |= ACCEL_RATE_25;    break;        
        case   12: value |= 0x8000 | ACCEL_RATE_12_5;  break;
        case   10: value |= ACCEL_RATE_12_5;  break;
        case    6: value |= 0x8000 | ACCEL_RATE_6_25;  break;
        case    3: value |= 0x8000 | ACCEL_RATE_1_56; break;       
        case    1: value |= ACCEL_RATE_1_56;  break;
        default:   value |= ACCEL_RATE_100; value |= 0x8000; break;     // Mark as invalid
    }

    switch (range)
    {
        case 16:   value |= ACCEL_RANGE_16G; break;         
        case  8:   value |= ACCEL_RANGE_8G;  break;
        case  4:   value |= ACCEL_RANGE_4G;  break;
        case  2:   value |= ACCEL_RANGE_2G;  break;
        default: value |= ACCEL_RANGE_8G; value |= 0x8000; break;      // Mark as invalid
    }

    return value;
}


// ------ Uniform -ValidSettings() & -StartupSettings() functions ------

// Returns whether given settings are valid
char AccelValidSettings(unsigned short rateHz, unsigned short sensitivityG, unsigned long flags)
{
    unsigned short rateCode = AccelSetting(rateHz, sensitivityG);
    //flags;                                      // Unused
    if (rateCode & 0x8000) { return 0; }        // Invalid value
    return 1;
}

// Starts the device with the given settings
void AccelStartupSettings(unsigned short rateHz, unsigned short sensitivityG, unsigned long flags)
{
    unsigned char accelFlags = 0;
    unsigned char accelFlagsInt1 = 0;
	    
    unsigned short rateCode = AccelSetting(rateHz, sensitivityG);
    AccelStartup(rateCode);                     // Start accelerometer, interrupts off

	// Currently forces all to int2 except fifo (for compatibility)   
    if (flags & ACCEL_FLAG_FIFO_INTERRUPTS) 		{ accelFlags |= ACCEL_INT_SOURCE_WATERMARK; accelFlagsInt1 |= ACCEL_INT_SOURCE_WATERMARK; }    
    if (flags & ACCEL_FLAG_ORIENTATION_INTERRUPTS) 	{ accelFlags |= ACCEL_INT_SOURCE_ORIENTATION; }    
    if (flags & ACCEL_FLAG_TRANSIENT_INTERRUPTS) 	{ accelFlags |= ACCEL_INT_SOURCE_ACTIVITY; }    
    if (flags & ACCEL_FLAG_TAP_INTERRUPTS) 			{ accelFlags |= ACCEL_INT_SOURCE_SINGLE_TAP; }    
    if (flags & ACCEL_FLAG_TAP_INTERRUPTS) 			{ accelFlags |= ACCEL_INT_SOURCE_DOUBLE_TAP; }    
	
	if (accelFlags != 0)
	{
		// Default interrupts enabled and pin mapping
        AccelEnableInterrupts(accelFlags, accelFlagsInt1); // Last field diverts source to int1
    }
}
