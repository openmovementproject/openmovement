// PCA9685 LED controller driver header
// Karim Ladha 2014

#include <Compiler.h>
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Peripherals/PCA9685.h"

// Globals
exLedState_t exLedState;	

// Set in HW profile and pins set to inputs in InitIO()
#if !defined(EX_LED_ALT_I2C) || (EX_LED_ALT_I2C == 1)
	#define EX_LED_I2CCON		I2C1CON
	#define EX_LED_I2CCONbits	I2C1CONbits
	#define EX_LED_I2CBRG 		I2C1BRG
	#define EX_LED_I2CSTATbits 	I2C1STATbits
	#define EX_LED_I2CTRN		I2C1TRN
	#define EX_LED_I2CRCV 		I2C1RCV
#elif (EX_LED_ALT_I2C == 2)
	#define EX_LED_I2CCON		I2C2CON
	#define EX_LED_I2CCONbits	I2C2CONbits
	#define EX_LED_I2CBRG 		I2C2BRG
	#define EX_LED_I2CSTATbits 	I2C2STATbits
	#define EX_LED_I2CTRN		I2C2TRN
	#define EX_LED_I2CRCV 		I2C2RCV
#else
	#error "Which I2C bus?"
#endif

// I2C baud rate
// Rate setting for pic24 devices funning at 8/32MHz
#define I2C_RATE_100kHZ 		((OSCCONbits.COSC==1)? 157 : 39)	
#define I2C_RATE_200kHZ 		((OSCCONbits.COSC==1)? 72 : 18)
#define I2C_RATE_400kHZ 		((OSCCONbits.COSC==1)? 37 : 9)	
#define I2C_RATE_1000kHZ 		((OSCCONbits.COSC==1)? 13 : 3)			
#define I2C_RATE_2000kHZ		((OSCCONbits.COSC==1)? 7 : 1)

#define EX_LED_BAUD 	I2C_RATE_1000kHZ

// PIC24 can not opperate I2C with PLL on without these Nop()s
#define PLL_DELAY_FIX()	{if(OSCCONbits.COSC==0b001){Nop();}}

// Local copy of functions - do not include myI2C.h as it does not support dynamic baud changes
#define I2C_READ_MASK		0x1
#define myI2COpen(baud)		{EX_LED_I2CCON = 0x0000; EX_LED_I2CBRG = baud; EX_LED_I2CCON = 0xF200;}
#define myI2CStart()		{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();EX_LED_I2CCONbits.SEN=1;while(EX_LED_I2CCONbits.SEN && --timeout);LOCAL_CHECK_TIMEOUT(timeout);}
#define myI2CStop()			{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();EX_LED_I2CCONbits.PEN=1;while(EX_LED_I2CCONbits.PEN&& --timeout);LOCAL_CHECK_TIMEOUT(timeout);}
#define myI2CRestart()		{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();EX_LED_I2CCONbits.RSEN=1;while(EX_LED_I2CCONbits.RSEN&& --timeout);LOCAL_CHECK_TIMEOUT(timeout);}
#define myI2CAck()			{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();EX_LED_I2CCONbits.ACKDT = 0;EX_LED_I2CCONbits.ACKEN=1;while(EX_LED_I2CCONbits.ACKEN&& --timeout);LOCAL_CHECK_TIMEOUT(timeout);}
#define myI2CNack()			{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();EX_LED_I2CCONbits.ACKDT = 1;EX_LED_I2CCONbits.ACKEN=1;while(EX_LED_I2CCONbits.ACKEN&& --timeout);LOCAL_CHECK_TIMEOUT(timeout);}
#define myI2CClose()		{EX_LED_I2CCON = 0x0000;}
#define myI2Cputc(_x)		{unsigned short timeout = MY_I2C_TIMEOUT;PLL_DELAY_FIX();EX_LED_I2CTRN=_x;while(EX_LED_I2CSTATbits.TBF&& --timeout);while(EX_LED_I2CSTATbits.TRSTAT&& --timeout);LOCAL_CHECK_TIMEOUT(timeout);} 
#define myAckStat()			(!EX_LED_I2CSTATbits.ACKSTAT) /*TRUE or 1 if slave acked*/
static unsigned char myI2Cgetc(void)  {PLL_DELAY_FIX();EX_LED_I2CCONbits.RCEN = 1; while(EX_LED_I2CCONbits.RCEN); EX_LED_I2CSTATbits.I2COV = 0; return(EX_LED_I2CRCV);}

// Private bus functions
static unsigned char ReadData(unsigned char busAddress, unsigned char subAddress, unsigned char* dest, unsigned char num, unsigned short baud);
static unsigned char WriteData(unsigned char busAddress, unsigned char subAddress, unsigned char* source, unsigned char num, unsigned short baud);
static void LOCAL_CHECK_TIMEOUT(unsigned short timeout);

unsigned char ExLedVerifyDeviceId(void)
{
	unsigned char mode1;
	// Attempt read first
	exLedState.present = ReadData(PCA9685_ADDRESS, 0, &mode1, 1, EX_LED_BAUD);
	return exLedState.present;
}

void ExLedStartup(unsigned char doSleep)
{
	unsigned char mode1, mode2, pre_scale, i;
	unsigned short ontime;
	if(!exLedState.present) return;
	// Read register mode1
	ReadData(PCA9685_ADDRESS, 0, &mode1, 1, EX_LED_BAUD);
	// If currently asleep
	if(mode1 & 0x10) 
	{
		// Mode1 register - internal clock, auto inc, awake mode (to latch outputs off)
		mode1 = 0b00100000;
		WriteData(PCA9685_ADDRESS, 0x00, &mode1, 1, EX_LED_BAUD);
		DelayMs(2); /*Must wait for sleep exit*/
		// Clear flag - device is awake
		exLedState.asleep = FALSE;
	}
	// Mode 2 register - Invert, outputs change on stop, open drain, all off
	mode2 = 0b00010010;
	WriteData(PCA9685_ADDRESS, 0x01, &mode2, 1, EX_LED_BAUD);	
	// Disable outputs - device is on but LEDs are off
	PCA9685_nOE = 1;

	// Mode1 register - internal clock, auto inc, sleep mode to allow rate change
	mode1 = 0b00110000;
	WriteData(PCA9685_ADDRESS, 0x00, &mode1, 1, EX_LED_BAUD);
	DelayMs(2); /*Must wait for sleep entry*/
	exLedState.asleep = TRUE;
	// Set prescale register setting
	pre_scale = PCA9685_PRESCALE_VAL;
	WriteData(PCA9685_ADDRESS, 0xFE, &pre_scale, 1, EX_LED_BAUD);

	// If not setting sleep - wake up here
	if(!doSleep)
	{
		WriteData(PCA9685_ADDRESS, 0x00, &mode1, 1, EX_LED_BAUD);
		DelayMs(2); /*Must wait for sleep exit*/
		// Clear flag - device is awake
		exLedState.asleep = FALSE;
	}

	// Init 8 bit led parts
	memset(exLedState.led,0,sizeof(exLedState.led));
	exLedState.gScale = 0x0F;
	// Init 16 bit on/off times (surge reduction)
	ontime = 0;
	for(i=0;i<16;i++)
	{
		exLedState.ledVal[i].led_on = ontime;
		exLedState.ledVal[i].led_off = ontime;
		ontime += 256; /*Equally staggered and off*/
	}

	// Update the I2C regs to reflect globals
	ExLedUpdate();
	// Enable outputs
	PCA9685_nOE = 0;
	return;
}

void ExLedUpdate(void)
{
	unsigned short i, temp;
	if(!exLedState.present) return;
	// 8 bit led settings first
	for(i=0;i<16;i++)
	{
		// Scale temp using gScale and clamp
		temp = exLedState.led[i];
		temp *= exLedState.gScale;
		if(temp > 0x0FFF) temp = 0x0FFF;
		// Add to on time and mask
		temp += exLedState.ledVal[i].led_on;
		temp &= 0x0FFF;
		// Set off time
		exLedState.ledVal[i].led_off = temp;
	}
	// Burst update LEDs
	ExLedUpdateRaw();
}

void ExLedUpdateRaw(void)
{
	if(!exLedState.present) return;
	// Write registers to device
	WriteData(PCA9685_ADDRESS, 0x06, (void*)exLedState.ledVal, sizeof(exLedState.ledVal), EX_LED_BAUD);
	return;
}

void ExLedSleep(void)
{
	unsigned char mode1;
	if(!exLedState.present) return;
	// Disable outputs - device is on but LEDs are off
	PCA9685_nOE = 1;
	if(exLedState.asleep == FALSE)
	{
		// Mode1 register - internal clock, auto inc, sleep mode 
		mode1 = 0b00110000;
		WriteData(PCA9685_ADDRESS, 0x00, &mode1, 1, EX_LED_BAUD);
		//DelayMs(2); /*Note:Must wait for sleep exit before update leds*/
		exLedState.asleep = TRUE;
	}
	return;
}

void ExLedWake(void)
{
	unsigned char mode1;
	if(!exLedState.present) return;
	// Enable outputs 
	PCA9685_nOE = 0;
	if(exLedState.asleep == TRUE)
	{
		// Mode1 register - internal clock, auto inc, awake mode 
		mode1 = 0b00100000;
		WriteData(PCA9685_ADDRESS, 0x00, &mode1, 1, EX_LED_BAUD);
		//DelayMs(2); /*Note:Must wait for sleep exit before update leds*/
		exLedState.asleep = FALSE;
	}
	return;
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
	myI2CRestart()
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
