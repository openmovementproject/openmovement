// Written to conglomerate a multi axis sensor platform to one .c file
// The main need for this is to save space and simplify the way we use
// the sensors by making a sampled sensor without fifo capability.

#include <Compiler.h>
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Config.h"
#include "Peripherals/MultiSensor.h"
#include "Settings.h"

// Bool types
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Globals
verify_id_t multiPresent;	

// PIC24 can not opperate I2C with PLL on without these Nop()s
#define PLL_DELAY_FIX()	{if(OSCCONbits.COSC==0b001){Nop();}}


// Local copy of functions - do not include myI2C.h as it does not support dynamic baud changes
#define I2C_READ_MASK		0x1
#define myI2COpen(baud)		{MULTI_SENSOR_I2CCON = 0x0000; MULTI_SENSOR_I2CBRG = baud; MULTI_SENSOR_I2CCON = 0xF200;}
#define myI2CStart()		{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();MULTI_SENSOR_I2CCONbits.SEN=1;while(MULTI_SENSOR_I2CCONbits.SEN && --timeout);LOCAL_CHECK_TIMEOUT(timeout);}
#define myI2CStop()			{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();MULTI_SENSOR_I2CCONbits.PEN=1;while(MULTI_SENSOR_I2CCONbits.PEN&& --timeout);LOCAL_CHECK_TIMEOUT(timeout);}
#define myI2CRestart()		{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();MULTI_SENSOR_I2CCONbits.RSEN=1;while(MULTI_SENSOR_I2CCONbits.RSEN&& --timeout);LOCAL_CHECK_TIMEOUT(timeout);}
#define myI2CAck()			{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();MULTI_SENSOR_I2CCONbits.ACKDT = 0;MULTI_SENSOR_I2CCONbits.ACKEN=1;while(MULTI_SENSOR_I2CCONbits.ACKEN&& --timeout);LOCAL_CHECK_TIMEOUT(timeout);}
#define myI2CNack()			{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();MULTI_SENSOR_I2CCONbits.ACKDT = 1;MULTI_SENSOR_I2CCONbits.ACKEN=1;while(MULTI_SENSOR_I2CCONbits.ACKEN&& --timeout);LOCAL_CHECK_TIMEOUT(timeout);}
#define myI2CClose()		{MULTI_SENSOR_I2CCON = 0x0000;}
#define myI2Cputc(_x)		{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();MULTI_SENSOR_I2CTRN=_x;while(MULTI_SENSOR_I2CSTATbits.TBF&& --timeout);while(MULTI_SENSOR_I2CSTATbits.TRSTAT&& --timeout);LOCAL_CHECK_TIMEOUT(timeout);} 
#define myAckStat()			(!MULTI_SENSOR_I2CSTATbits.ACKSTAT) /*TRUE or 1 if slave acked*/
unsigned char myI2Cgetc(void)  {PLL_DELAY_FIX();MULTI_SENSOR_I2CCONbits.RCEN = 1; while(MULTI_SENSOR_I2CCONbits.RCEN); MULTI_SENSOR_I2CSTATbits.I2COV = 0; return(MULTI_SENSOR_I2CRCV);}

unsigned char MultiVerifyDeviceId(void)
{
	multiPresent.all = 0;
	unsigned char id;
	// Accel
	if (ReadData(ACCEL_ADDRESS, ACCEL_ADDR_WHO_AM_I, &id, 1, ACCEL_BAUD))
	{
		if (id == ACCEL_DEVICE_ID) multiPresent.accel = TRUE;
	}
	// Gyro
	if (ReadData(GYRO_ADDRESS, GYRO_WHO_AM_I, &id, 1, GYRO_BAUD))
	{
		if (id == GYRO_DEVICE_ID) multiPresent.gyro= TRUE;
	}
	// Mag
	if (ReadData(MAG_ADDRESS, MAG_ADDR_WHO_AM_I, &id, 1, MAG_BAUD))
	{
		if (id == MAG_DEVICE_ID) multiPresent.mag = TRUE;
	}
	return multiPresent.all;
}

void MultiStartup(settings_t* set)
{
	unsigned char buffer[16];
	// Accel
	if((multiPresent.accel == TRUE)&&((set->accelOn && set->accelRate) || set->accelInts))
	{
		buffer[0] = 0;
		WriteData(ACCEL_ADDRESS, ACCEL_ADDR_CTRL_REG1, buffer, 1, ACCEL_BAUD); // Device off
		switch (set->accelRange){
			case (8) : {buffer[0] = 0b00000010;break;}
			case (4) : {buffer[0] = 0b00000001;break;}
			case (2) : {buffer[0] = 0b00000000;break;}
			default :  {buffer[0] = 0b00000010;break;} /*Default is 8g*/
		}/*	XYZ_DATA_CFG */
		buffer[1] = 0b00100011;	/*	HP_FILTER_CUTOFF*/
		WriteData(ACCEL_ADDRESS, ACCEL_ADDR_XYZ_DATA_CFG, buffer, 2, ACCEL_BAUD);
		buffer[0]=0b11000000;	/*	PL_CFG*/
		buffer[1]=0b01000100;	/*	PL_BF_ZCOMP*/
		buffer[2]=0b10000100;	/*	P_L_THS_REG*/
		buffer[3]=0b01111000;	/*	FF_MT_CFG*/
		WriteData(ACCEL_ADDRESS, ACCEL_ADDR_PL_CFG, buffer, 4, ACCEL_BAUD);
		buffer[0]=0b00000011;	/*	FF_MT_THS */
		buffer[1]=0b00000011;	/*	FF_MT_COUNT */
		WriteData(ACCEL_ADDRESS, ACCEL_ADDR_FF_MT_THS, buffer, 2, ACCEL_BAUD);
		buffer[0] = 0b00011110;	/*	TRANSIENT_CFG*/
		WriteData(ACCEL_ADDRESS, ACCEL_ADDR_TRANSIENT_CFG, buffer, 1, ACCEL_BAUD);
		buffer[0] = 0b10000010;	/*	TRANSIENT_THS*/
		buffer[1] = 0b00000010;	/*	TRANSIENT_COUNT*/
		buffer[2] = 0b01111111;	/*	PULSE_CFG*/
		WriteData(ACCEL_ADDRESS, ACCEL_ADDR_TRANSIENT_THS, buffer, 3, ACCEL_BAUD);

		buffer[0] = 32;	/*	PULSE_THSX Pulse threshold x*/
		buffer[1] = 32;	/*	PULSE_THSY Pulse threshold y*/
		buffer[2] = 32;	/*	PULSE_THSZ Pulse threshold z*/
		buffer[3] = 2;	/*	PULSE_TMLT TMLT[7:0] Pulse Time Limit. Default value: 0000_0000.*/
		buffer[4] = 2;	/*	PULSE_LTCY LTCY[7:0] Latency Time Limit. Default value: 0000_0000 */
		buffer[5] = 25;	/*	PULSE_WIND WIND[7:0] Second Pulse Time Window. Default value: 0000_0000.*/
		buffer[6] = 0b00000000;	/*	ASLP_COUNT D[7:0] Duration value. Default value: 0000_0000 */
		switch (set->accelRate) { /*CTRL_REG1*/
			case (800) :	{buffer[7] =0b00000000;break;}
			case (400) :	{buffer[7] =0b00001000;break;}
			case (200) :	{buffer[7] =0b00010000;break;}
			case (100) :	{buffer[7] =0b00011000;break;}
			case (50) : 	{buffer[7] =0b00100000;break;}
			case (12) :		{buffer[7] =0b00101000;break;}
			default	:		{buffer[7] =0b00011000;break;}/*100Hz default*/
		}
		buffer[8] = 0b00000000;	/*	CTRL_REG2*/
		buffer[9] = 0b00000010;	/*	CTRL_REG3*/
		switch(set->accelInts){/*CTRL_REG4*/
			case(0)	: {buffer[10] = 0b00000000;break;}	/*	Off*/
			case(1)	: {buffer[10] = 0b00100000;break;}	/*	Transients*/
			case(2)	: {buffer[10] = 0b00010000;break;}	/*	Orientation*/
			case(3)	: {buffer[10] = 0b00001000;break;}	/*	Taps*/
			default	: {buffer[10] = 0b00000000;break;}
		}
		/*	CTRL_REG4
		b7		:	INT_EN_ASLP		0: Auto-SLEEP/WAKE interrupt disabled; 1: Auto-SLEEP/WAKE interrupt enabled.
		B6		: 	INT_EN_FIFO		0: FIFO interrupt off
		b5		:	INT_EN_TRANS	0: Transient interrupt disabled; 1: Transient interrupt enabled.
		b4		:	INT_EN_LNDPRT	0: Orientation (Landscape/Portrait) interrupt disabled.
		b3		:	INT_EN_PULSE	0: Pulse Detection interrupt disabled; 1: Pulse Detection interrupt enabled
		b2		:	INT_EN_FF_MT	0: Freefall/Motion interrupt disabled; 1: Freefall/Motion interrupt enabled
		b0		:	INT_EN_DRDY		0: Data Ready interrupt disabled; 1: Data Ready interrupt enabled*/
		buffer[11] = buffer[10];	/*	CTRL_REG5, all ints on int 1*/
		buffer[12] = 0b00000000;	/*	OFF_X Offset Correction X Register */
		buffer[13] = 0b00000000;	/*	OFF_Y Offset Correction Y Register */
		buffer[14] = 0b00000000;	/*	OFF_Z Offset Correction Z Register */
		WriteData(ACCEL_ADDRESS, ACCEL_ADDR_PULSE_THSX, buffer, 15, ACCEL_BAUD);
		if(set->accelRate != 0)
		{
			buffer[7] |= 0b1; /*CTRL_REG1*/
		}
		WriteData(ACCEL_ADDRESS, ACCEL_ADDR_CTRL_REG1, &buffer[7], 1, ACCEL_BAUD);
		// Brief pause then read interrupts to arm them
		if(set->accelInts)
		{
			DelayMs(50);
			ACCEL_INT1_IP = ACCEL_INT_PRIORITY;
			ACCEL_INT2_IP = ACCEL_INT_PRIORITY;
			ReadData(ACCEL_ADDRESS,ACCEL_ADDR_INT_SOURCE, buffer, 1, ACCEL_BAUD);
			ReadData(ACCEL_ADDRESS, ACCEL_ADDR_PULSE_SRC, buffer, 1, ACCEL_BAUD);
			ReadData(ACCEL_ADDRESS, ACCEL_ADDR_PL_STATUS, buffer, 1, ACCEL_BAUD);
			ReadData(ACCEL_ADDRESS, ACCEL_ADDR_TRANSIENT_SRC, buffer, 1, ACCEL_BAUD);
			ACCEL_INT1_IF = 0;
			ACCEL_INT1_IE = 1;
		}
	}
	else if (multiPresent.accel == TRUE) 
	{
		unsigned char val = 0; // CTRL_REG1
		WriteData(ACCEL_ADDRESS, ACCEL_ADDR_PULSE_THSX, &val, 1, ACCEL_BAUD);
	}	// end accel setup

	// Gyro
	if((multiPresent.gyro == TRUE)&&(set->gyroRate)&& (set->gyroOn))
	{
		buffer[0] = 0xf0;	// GYRO_CTRL_REG1
		buffer[1] = 0x23;	// GYRO_CTRL_REG2
		buffer[2] = 0;		// GYRO_CTRL_REG3
		switch(set->gyroRange){
			case (2000)	:	buffer[3] = 0x80+0x20;break;	// GYRO_CTRL_REG4 (BDU is on)
			case (500)	:	buffer[3] = 0x80+0x10;break;	// GYRO_CTRL_REG4 (BDU is on)
			case (250)	:	buffer[3] = 0x80+0x00;break;	// GYRO_CTRL_REG4 (BDU is on)
			default	:		buffer[3] = 0x80+0x20;break;} 	// GYRO_CTRL_REG4 (BDU is on)
		buffer[4] = 0;		// GYRO_CTRL_REG5
		WriteData(GYRO_ADDRESS, GYRO_CTRL_REG1| GYRO_MASK_BURST, buffer, 5, GYRO_BAUD);
		DelayMs(10);
		if(set->gyroRate != 0)
		{
			switch (set->gyroRate) {
				case (800) : {buffer[0]=0xff; break;}// GYRO_CTRL_REG1
				case (400) : {buffer[0]=0xbf; break;}// GYRO_CTRL_REG1
				case (200) : {buffer[0]=0x7f; break;}// GYRO_CTRL_REG1
				case (100) : {buffer[0]=0x3f; break;}// GYRO_CTRL_REG1
				default : 	{buffer[0]= 0x3f; break;}}// GYRO_CTRL_REG1
			WriteData(GYRO_ADDRESS, GYRO_CTRL_REG1, buffer, 1, GYRO_BAUD);
		}
	}
	else if(multiPresent.gyro == TRUE)
	{
		unsigned char val = 0;// GYRO_CTRL_REG1
		WriteData(GYRO_ADDRESS, GYRO_CTRL_REG1, &val, 1, GYRO_BAUD);
	}

	// Mag
	if((multiPresent.mag == TRUE)&&(set->magRate)&&(set->magOn))
	{
		buffer[0] = 0; /*MAG_ADDR_CTRL_REG1*/
		buffer[1] = 0x80; /*MAG_ADDR_CTRL_REG2 - setting AUTO_MRST_EN */
		WriteData(MAG_ADDRESS, MAG_ADDR_CTRL_REG1, buffer, 2, MAG_BAUD);
		buffer[0] = 0; /*	MAG_ADDR_OFF_X_MSB */
		buffer[1] = 0; /*	MAG_ADDR_OFF_X_LSB */
		buffer[2] = 0; /*	MAG_ADDR_OFF_Y_MSB */
		buffer[3] = 0; /*	MAG_ADDR_OFF_Y_LSB */
		buffer[4] = 0; /*	MAG_ADDR_OFF_Z_MSB */
		buffer[5] = 0; /*	MAG_ADDR_OFF_Z_LSB */
		WriteData(MAG_ADDRESS, MAG_ADDR_OFF_X_MSB, buffer, 6, MAG_BAUD);
		if(set->magRate != 0)
		{
			switch (set->magRate){
			case(80):buffer[0]=0b00000001;break; /*MAG_ADDR_CTRL_REG1*/
			case(40):buffer[0]=0b00100001;break;
			case(20):buffer[0]=0b01000001;break;
			case(10):buffer[0]=0b01100001;break;
			case(5):buffer[0]=0b10000001; break;
			default:buffer[0]=0b01100001;break; } /*Default is 10Hz*/
			WriteData(MAG_ADDRESS, MAG_ADDR_CTRL_REG1, buffer, 1, MAG_BAUD);
		}
	}
	else if(multiPresent.mag == TRUE)
	{
		unsigned char val = 0;// CTRL_REG1
		WriteData(MAG_ADDRESS, MAG_ADDR_CTRL_REG1, &val, 1, MAG_BAUD);
	}

}


void MultiStandby(void)
{
	unsigned char val;

	// Accel
	if (settings.accelInts != 0)
	{
		// Interrupts are needed - Setup the accelerometer
		settings_t temp;
		temp.accelOn	= 1;
		temp.accelRate	= 50;
		temp.accelRange	= 8;
		temp.accelInts = settings.accelInts;
		temp.gyroOn	= 0;
		temp.magOn	= 0;
		MultiStartup(&temp);
	}
	else if (multiPresent.accel == TRUE) 
	{
		val = 0; // CTRL_REG1
		WriteData(ACCEL_ADDRESS, ACCEL_ADDR_PULSE_THSX, &val, 1, ACCEL_BAUD);
	}

	// Gyro
	if(multiPresent.gyro == TRUE)
	{
		val = 0;// GYRO_CTRL_REG1
		WriteData(GYRO_ADDRESS, GYRO_CTRL_REG1, &val, 1, GYRO_BAUD);
	}
	// Mag
	if(multiPresent.mag == TRUE)
	{
		val = 0;// CTRL_REG1
		WriteData(MAG_ADDRESS, MAG_ADDR_CTRL_REG1, &val, 1, MAG_BAUD);
	}
}


void MultiSingleSample(sensor_t *sample)
{
	unsigned char buffer[6];
	// Accel
	if(multiPresent.accel == TRUE)
	{
		ReadData(ACCEL_ADDRESS, ACCEL_ADDR_OUT_X_MSB, buffer, 6, ACCEL_BAUD);
#ifdef MULTI_SENSOR_ROTATE_XY_MINUS_90
		sample->accel.yh = buffer[0];
		sample->accel.yl = buffer[1];
		sample->accel.xh = buffer[2];
		sample->accel.xl = buffer[3];
		sample->accel.zh = buffer[4];
		sample->accel.zl = buffer[5];
		sample->accel.x = -sample->accel.x;
#else
		sample->accel.xh = buffer[0];
		sample->accel.xl = buffer[1];
		sample->accel.yh = buffer[2];
		sample->accel.yl = buffer[3];
		sample->accel.zh = buffer[4];
		sample->accel.zl = buffer[5];
#endif
	}
	// Mag
	if(multiPresent.mag == TRUE)
	{
		ReadData(MAG_ADDRESS, MAG_ADDR_OUT_X_MSB, buffer, 6, MAG_BAUD);
#ifdef MULTI_SENSOR_ROTATE_XY_MINUS_90
		sample->mag.yh = buffer[0];
		sample->mag.yl = buffer[1];
		sample->mag.xh = buffer[2];
		sample->mag.xl = buffer[3];
		sample->mag.zh = buffer[4];
		sample->mag.zl = buffer[5];

		sample->mag.x = -sample->mag.x;
#else
		sample->mag.xh = buffer[0];
		sample->mag.xl = buffer[1];
		sample->mag.yh = buffer[2];
		sample->mag.yl = buffer[3];
		sample->mag.zh = buffer[4];
		sample->mag.zl = buffer[5];
#endif
	}
	// Gyro
	if(multiPresent.gyro == TRUE)
	{
		// Ctrlreg4 is set to msb first, swaps not needed
		ReadData(GYRO_ADDRESS, GYRO_OUT_X_L | GYRO_MASK_BURST, &sample->gyro.xl, 6, GYRO_BAUD);
#ifdef MULTI_SENSOR_ROTATE_XY_MINUS_90
		{
			// Rotate about Z
			unsigned short temp;
			temp = sample->gyro.x;
			sample->gyro.x = -sample->gyro.y;
			sample->gyro.y = temp;
		}
#else
		// No rotation
#endif
	}
}

// Trys to clear interrupts - will try three times
unsigned char AccelReadIntSource(void)
{
	unsigned char val, retval, retrys = 2;
	do{
		ReadData(ACCEL_ADDRESS,ACCEL_ADDR_INT_SOURCE, &retval, 1, ACCEL_BAUD);
		if(retval & 0x01){	ReadData(ACCEL_ADDRESS, ACCEL_ADDR_PULSE_SRC, &val, 1, ACCEL_BAUD);}
		if(retval & 0x10){	ReadData(ACCEL_ADDRESS, ACCEL_ADDR_PL_STATUS, &val, 1, ACCEL_BAUD);}
		if(retval & 0x20){	ReadData(ACCEL_ADDRESS, ACCEL_ADDR_TRANSIENT_SRC, &val, 1, ACCEL_BAUD);}
		ReadData(ACCEL_ADDRESS,ACCEL_ADDR_INT_SOURCE, &val, 1, ACCEL_BAUD); // Check it cleared
	}while(retrys-- && (val&0x31));
	if (retrys == 0) return 0; // Unable to clear int
	else return retval;
}

unsigned char ReadData(unsigned char busAddress, unsigned char subAddress, unsigned char* dest, unsigned char num, unsigned short baud)
{
	myI2COpen(baud);
	myI2CStart();
	myI2Cputc(busAddress);
	if(!myAckStat())
	{
		//No ack = no device
		myI2CStop();
		myI2CClose();
		return FALSE;
	}	
	myI2Cputc(subAddress);
	myI2CRestart();
 	myI2Cputc(busAddress | I2C_READ_MASK);
	for(;num>1;num--)
	{
		*dest++ = myI2Cgetc();
		myI2CAck();
	}
	*dest++ = myI2Cgetc();	
	myI2CNack();
	myI2CStop();	
	myI2CClose();
	return TRUE;
}

unsigned char WriteData(unsigned char busAddress, unsigned char subAddress, unsigned char* source, unsigned char num, unsigned short baud)
{
	myI2COpen(baud);
	myI2CStart();
	myI2Cputc(busAddress);
	if(!myAckStat())
	{
		//No ack = no device
		myI2CStop();
		myI2CClose();
		return FALSE;
	}	
	myI2Cputc(subAddress);
	for(;num>0;num--)
	{
		myI2Cputc(*source++);
	}
	myI2CStop();	
	myI2CClose();
	return TRUE;
}

void LOCAL_CHECK_TIMEOUT(unsigned short timeout)	   
{
	if (timeout == 0)
	{
		#ifdef __DEBUG
		LED_SET(LED_MAGENTA);
		while(1);
		#endif
		DebugReset(I2C_ERR);
	}
}

//EOF
