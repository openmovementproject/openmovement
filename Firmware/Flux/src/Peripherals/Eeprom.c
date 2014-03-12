/*KL, standart I2C eeprom driver*/
#include "Compiler.h"
#include "HardwareProfile.h"
#include "Peripherals/Eeprom.h"

// Alternate I2C bus?
#ifdef EEROM_ALTERNATE_I2C
	#define MY_I2C_OVERIDE	EEROM_ALTERNATE_I2C
#endif
#include "Peripherals/myI2C.h"


// Address structure, used by internal functions
union 
{
	unsigned long longAddress; 
	struct
	{
		unsigned short wordAdd;
		unsigned short unused;
	};
	struct
	{
		unsigned char lowAdd;
		unsigned char highAdd;
		unsigned char unused1;
		unsigned char unused2;
	};
}current_address;

// Globals
unsigned char eepromPresent = 0;

// Address
#ifndef EEPROM_ADDRESS	
	#warning "Using default address for eeprom."
	#define	EEPROM_ADDRESS	0xA0 	/*I2C address*/
#endif

// Speed
#ifndef ACCEL_I2C_RATE
	#define LOCAL_I2C_RATE		I2C_RATE_400kHZ
#else
	#define LOCAL_I2C_RATE		ACCEL_I2C_RATE
#endif

// Page sizes
#ifndef WRITE_PAGE_SIZE
	#warning "Using default page size 128 bytes"
	#define WRITE_PAGE_SIZE		0x80
#endif


// I2C 
#define CUT_DOWN_I2C_CODE_SIZE
#ifndef CUT_DOWN_I2C_CODE_SIZE
	#define EeOpen()              myI2COpen();myI2CStart(); WaitStartmyI2C();
	#define EeAddressRead(_r)     myI2Cputc(EEPROM_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(EEPROM_ADDRESS | I2C_READ_MASK);
	#define EeAddressWrite(_r)    myI2Cputc(EEPROM_ADDRESS); myI2Cputc((_r)); 
	#define EeReadContinue()      myI2Cgetc(); myI2CAck()
	#define EeReadLast()          myI2Cgetc(); myI2CNack()
	#define EeWrite(_v)           myI2Cputc((_v));
	#define EeClose()             myI2CStop(); WaitStopmyI2C();myI2CClose();
	#define EeReopen()            myI2CRestart(); WaitRestartmyI2C();
#else
	void EeOpen(void)					{myI2COpen();myI2CStart(); WaitStartmyI2C();}
	void EeAddressRead(unsigned char _r){myI2Cputc(EEPROM_ADDRESS); myI2Cputc((_r)); myI2CRestart(); WaitRestartmyI2C(); myI2Cputc(EEPROM_ADDRESS | I2C_READ_MASK);}
	void EeAddressWrite(unsigned char _r){myI2Cputc(EEPROM_ADDRESS); myI2Cputc((_r)); }
	unsigned char EeReadContinue(void)  {unsigned char ret = myI2Cgetc(); myI2CAck();return ret;}
	unsigned char EeReadLast(void)      {unsigned char ret = myI2Cgetc(); myI2CNack();return ret;}
	void EeWrite(unsigned char _v)      {myI2Cputc((_v));}
	void EeClose(void )             	{myI2CStop(); WaitStopmyI2C();myI2CClose();}
	void EeReopen(void )            	{myI2CRestart(); WaitRestartmyI2C();}
#endif

// Functions
unsigned char I2C_eeprom_present(void)
{
	unsigned short timeout = 0x0fff; 		// Suitable timeout for this HW
	eepromPresent = 1;						// Must set it present to enable following function
	while(I2C_eeprom_busy() && --timeout){;}
	if(!timeout) eepromPresent = 0;			// If it timesout, its not there
	
	return eepromPresent;
}
unsigned char I2C_eeprom_busy(void)
{
	unsigned char busy;

	if (!eepromPresent)return 1; // Always busy if not present

	EeOpen();
	EeWrite(EEPROM_ADDRESS);
	busy = !myAckStat();
 	EeClose();
	return busy;
}

unsigned char I2C_eeprom_wait(void)
{
	unsigned short timeout = 0xffff;
	while (I2C_eeprom_busy() && --timeout) {;}
	return ((timeout>0)?1:0);
}

unsigned char I2C_eeprom_write(unsigned long destination, unsigned char* data,unsigned short len)
{
	// If busy, return failed
	if (I2C_eeprom_busy()) 	return 0;
	if (len == 0)			return 1;

	// Set global pointer
	current_address.longAddress = destination;

	do	// Allow multiple passes if required
	{
		// Cache current page	
		unsigned char currentPage = current_address.lowAdd & WRITE_PAGE_SIZE; // WRITE_PAGE_SIZE is a power of 2, this tests the bit governing the current page
	
		// Open bus
		EeOpen();
		EeWrite(EEPROM_ADDRESS);
		// Write sesination address
		EeWrite(current_address.highAdd);
		EeWrite(current_address.lowAdd);
		// Write bytes into page
		while(len > 0)
		{
			EeWrite(*data);		
			data++;
			len--;
			current_address.longAddress++;
			// Check pointer is on same page
			if (currentPage != (current_address.lowAdd & WRITE_PAGE_SIZE)) // Written off edge of page
			{
				break; // Out of the inner while
			}
		}
		// Close bus
	 	EeClose();

		// If there is more data to write still, wait for this write to complete
		if (len)	
		{
			while(I2C_eeprom_busy());
		}

	}while(len); // Repeat if not finished


	return 1;
}

unsigned char I2C_eeprom_read( unsigned long source, unsigned char* destination, unsigned short len)
{
	// If busy, return failed
	if (I2C_eeprom_busy()) 	return 0;
	if (len == 0)			return 1;

	// Set global pointer
	current_address.longAddress = source;

	// Open bus
	EeOpen();
	EeWrite(EEPROM_ADDRESS);
	// Write desination address
	EeWrite(current_address.highAdd);
	EeWrite(current_address.lowAdd);
	// Restart bus
	EeReopen();
	// Initiate read
	EeWrite(EEPROM_ADDRESS | I2C_READ_MASK);
	// Read bytes with Acks
	while(len > 1)
	{
		*destination = EeReadContinue();	
		destination++;
		len--;
		current_address.wordAdd++;
	}
	// Read last, len==1 - Nack
	*destination = EeReadLast();	
	current_address.wordAdd++;
	// Close bus
 	EeClose();

	return 1;
}

//EOF





