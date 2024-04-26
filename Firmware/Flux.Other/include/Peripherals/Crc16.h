/* 
	Karim Ladha 27-06-2015
	CRC16, CCITT 0x1D0F implementation
	Requires 16 zeros shifted in to complete calculation
	Hardware abstraction provided where supported (e.g. PIC24) 
	Provided API is:
	CrcStart()
	CrcAdd(-byte-)
	CrcResult()
*/
// Include 
#include <stdint.h>

// Definitions
#define CRC_POLYNOMIAL	0x1021	/* CRC16 CCITT polynomial */
#define CRC16_INIT		0xFFFF	/* Standard initialiser value */
#define CRC16_PRE_INIT	0xE2F0	/* Shifting this word in with 0xffff init gives value 0xffff */

#ifdef __PIC24F__
/*
	Hardware abstraction for using the PIC24 CRC engine.
	Calculates the CCITT '1D0F' CRC16 using the hardware module.
	Requires 4 Tcy per CRC add minimum or the FIFO will overflow (corrupting result). 
*/
// Includes
#include "Compiler.h"

// Add byte to CRC calculation
#define CrcAdd(_b)		{*(volatile uint8_t*)&CRCDAT = (uint8_t)(_b);}
#define CrcResult()		(CRCWDAT)

static inline void __attribute__((always_inline)) CrcStart(void)
{
	PMD3bits.CRCMD = 1;	Nop();	/*Hard reset of crc */
	PMD3bits.CRCMD = 0;	Nop();	/*module to clear it.*/

	CRCCON = 0x000f;			/*16 bit CRC		*/
	CRCXOR = CRC_POLYNOMIAL;	/*CCITT CRC16		*/
	CRCWDAT = CRC16_INIT;		/*CRC initialiser	*/

	CRCDAT = CRC16_PRE_INIT;	/*Load pre-init val	*/
	CRCCONbits.CRCGO = 1;		/*Then turn on		*/
	while(!CRCCONbits.CRCMPT);	/*After last byte	*/
	Nop();Nop();Nop();Nop();	/*wait another 16	*/
	Nop();Nop();Nop();Nop();	/*shift cycles.		*/

	#if 0
	if(CRCWDAT != CRC16_INIT)	/*Check result ok	*/
	{
		/* This indicates a hardware fault */
		DebugReset(MCU_ERR);
	}
	#endif
	return;
}

static inline void __attribute__((always_inline)) CrcEnd(void)
{
	if(!CRCCONbits.CRCGO)		/*Early out if off	*/
		return;
	/* Shift in 16 0's to complete the CRC calculation.	*/
	asm volatile(
	"clr w0 \n\t"
	"mov.w w0, _CRCDAT \n\t"
	:::"w0");
			
	while(CRCCONbits.VWORD);	/*After last byte	*/
	Nop();Nop();Nop();Nop();	/*wait another 16	*/
	Nop();Nop();Nop();Nop();	/*shift cycles.		*/

	CRCCONbits.CRCGO = 0;		/*Then turn off		*/
}

#else
#warning "None hardware CRC calculation used."
static uint16_t crcVal;

static inline void __attribute__((always_inline)) CrcStart(void)
{
	crcVal = CRC16_INIT;
}

static inline void __attribute__((always_inline)) CrcAdd(uint8_t val)
{
	// Polynomial add to crc (optimal for 32bit processor)
	uint8_t round;
	uint32_t temp = (((uint32_t)(crcVal))<<8) + val;
	for(round = 8; round > 0; round--)
	{
		temp <<= 1;
		if(temp & 0x01000000)
			temp ^= (((uint32_t)CRC_POLYNOMIAL) << 8);
	}	
	crcVal = temp >> 8;
}

static inline void __attribute__((always_inline)) CrcEnd(void)
{
	// Must shift in 2 zeros according to convention
	CrcAdd(0x00);
	CrcAdd(0x00);
}

static inline uint16_t __attribute__((always_inline)) CrcResult(void)
{
	return crcVal;
}

#endif

// Helper function
static inline uint16_t CrcCalc(const uint8_t* data, uint16_t len) {CrcStart(); while(len--)CrcAdd(*data++); CrcEnd(); return CrcResult();}

//EOF
