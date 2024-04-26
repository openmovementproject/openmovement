
/**
  RTCC Generated Driver API Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    rtcc.c

  @Summary:
    This is the generated header file for the RTCC driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description:
    This header file provides APIs for driver for RTCC.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - pic24-dspic-pic32mm : 1.53.0.1
        Device            :  PIC24FJ1024GB606
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.33
        MPLAB 	          :  MPLAB X v4.05
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/


/**
 Section: Included Files
*/

#include <xc.h>
#include <time.h>
#include "stdint.h"
#include "stdbool.h"
#include "Peripherals/Rtc.h"

/**
// Section: Static function
*/

void RTCC_LockOff(void);
void RTCC_LockOn(void);


//static bool rtccTimeInitialized;
//static bool RTCCTimeInitialized(void);
static uint8_t ConvertHexToBCD(uint8_t hexvalue);
static uint8_t ConvertBCDToHex(uint8_t bcdvalue);

/**
// Section: Driver Interface Function Definitions
*/

//void RTCC_Initialize(void)
//{
//
//   RTCCON1Lbits.RTCEN = 0;
//   
//   __builtin_write_RTCC_WRLOCK();
//   
//   
//   if(!RTCCTimeInitialized())
//   {
//       // set 2018-04-26 16-17-43
//       DATEH = 0x1804;    // Year/Month
//       DATEL = 0x2604;    // Date/Wday
//       TIMEH = 0x1617;    // hours/minutes
//       TIMEL = 0x4300;    // seconds
//   }
//
//   // PWCPS 1:1; PS 1:1; CLKSEL SOSC; FDIV 0; 
//   RTCCON2L = 0x0000;
//   // DIV 1999999; 
//   RTCCON2H = 16384 - 1;;
//   // PWCSTAB 0; PWCSAMP 0; 
//   RTCCON3L = 0x0000;
//
//   // RTCEN enabled; OUTSEL Alarm Event; PWCPOE disabled; PWCEN disabled; WRLOCK disabled; PWCPOL disabled; TSAEN disabled; RTCOE disabled; 
//   RTCCON1L = 0x8000; 
//   
//   RTCC_LockOff();
//
//   IEC3bits.RTCIE = 1;
//}

void RTCC_LockOn(void)
{
    asm volatile("BSET RTCCON1L, #11");
}
void RTCC_LockOff(void)
{
    asm volatile("DISI #16");
    asm volatile("MOV #NVMKEY, W1");
    asm volatile("MOV #0x55, W2");
    asm volatile("MOV W2, [W1]");
    asm volatile("MOV #0xAA, W3");
    asm volatile("MOV W3, [W1]");
    asm volatile("BCLR RTCCON1L, #11");
}

void RtccReadTimeDate(rtccTimeDate* pTD)
{
	uint16_t dateh, datel, timeh, timel, retry = 3;
	
	// Copy registers under roll-over lock
    do{
    timel = TIMEL;
    dateh = DATEH;
	datel = DATEL;
	timeh = TIMEH;
	}while(timel != TIMEL && retry--);

	// Conversion to old hardware format - Since datasheet guarantees read-as-zero, skip masks
/*
	// No Dan, they don't line up at all with the old struct so I can use ANY non-byte copies to achieve this
	pTD->f.year = (dateh & 0xff00) >> 8;
	pTD->f.rsvd = 0;
	pTD->f.mday = (datel & 0x3f00) >> 8;
	pTD->f.mon = (dateh & 0x1f);
	pTD->f.hour = (timeh & 0x3f00) >> 8;
	pTD->f.wday = (datel & 0x7);
	pTD->f.sec = (timel & 0x7f00) >> 8;
	pTD->f.min = (timeh & 0x7F);
*/
	pTD->f.year = dateh >> 8;
	pTD->f.rsvd = 0;
	pTD->f.mday = datel >> 8;
	pTD->f.mon = dateh;
	pTD->f.hour = timeh >> 8;
	pTD->f.wday = datel;
	pTD->f.sec = timel >> 8;
	pTD->f.min = timeh;	
	return;
}

bool RTCC_TimeGet(struct tm *currentTime)
{
    uint16_t register_value;
    if(RTCSTATLbits.SYNC){
        return false;
    }
 
    register_value = DATEH;
    currentTime->tm_year = ConvertBCDToHex((register_value & 0xFF00) >> 8);
    currentTime->tm_mon = ConvertBCDToHex(register_value & 0x00FF);
    
    register_value = DATEL;
    currentTime->tm_mday = ConvertBCDToHex((register_value & 0xFF00) >> 8);
    currentTime->tm_wday = ConvertBCDToHex(register_value & 0x00FF);
    
    register_value = TIMEH;
    currentTime->tm_hour = ConvertBCDToHex((register_value & 0xFF00) >> 8);
    currentTime->tm_min = ConvertBCDToHex(register_value & 0x00FF);
    
    register_value = TIMEL;
    currentTime->tm_sec = ConvertBCDToHex((register_value & 0xFF00) >> 8);
   
    return true;
}

void RTCC_TimeSet(struct tm *initialTime)
{

    RTCC_LockOff();

   RTCCON1Lbits.RTCEN = 0;
   
   IFS3bits.RTCIF = false;
   IEC3bits.RTCIE = 0;

   // set RTCC initial time
   DATEH = (uint16_t)((uint16_t)ConvertHexToBCD(initialTime->tm_year) << 8) | ConvertHexToBCD(initialTime->tm_mon) ;  // YEAR/MONTH-1
   DATEL = (uint16_t)((uint16_t)ConvertHexToBCD(initialTime->tm_mday) << 8) | ConvertHexToBCD(initialTime->tm_wday) ;  // /DAY-1/WEEKDAY
   TIMEH = (uint16_t)((uint16_t)ConvertHexToBCD(initialTime->tm_hour) << 8)  | ConvertHexToBCD(initialTime->tm_min); // /HOURS/MINUTES
   TIMEL = (uint16_t)((uint16_t)ConvertHexToBCD(initialTime->tm_sec) << 8) ;   // SECOND
           
   // Enable RTCC, clear RTCWREN         
   RTCCON1Lbits.RTCEN = 1;  
   
   RTCC_LockOn();

   //Enable RTCC interrupt
   IEC3bits.RTCIE = 1;
}

//bool RTCC_BCDTimeGet(bcdTime_t *currentTime)
//{
//    uint16_t register_value;
//    if(RTCSTATLbits.SYNC){
//        return false;
//    }
//
//
//    __builtin_write_RTCC_WRLOCK();
//   
//    register_value = DATEH;
//    currentTime->tm_year = (register_value & 0xFF00) >> 8;
//    currentTime->tm_mon = register_value & 0x00FF;
//    
//    register_value = DATEL;
//    currentTime->tm_mday = (register_value & 0xFF00) >> 8;
//    currentTime->tm_wday = register_value & 0x00FF;
//    
//    register_value = TIMEH;
//    currentTime->tm_hour = (register_value & 0xFF00) >> 8;
//    currentTime->tm_min = register_value & 0x00FF;
//   
//    register_value = TIMEL;
//    currentTime->tm_sec = (register_value & 0xFF00) >> 8;
//   
//    RTCC_LockOff();
//
//    return true;
//}
//
//void RTCC_BCDTimeSet(bcdTime_t *initialTime)
//{
//
//   __builtin_write_RTCC_WRLOCK();
//
//   RTCCON1Lbits.RTCEN = 0;
//   
//   IFS3bits.RTCIF = false;
//   IEC3bits.RTCIE = 0;
//
//   // set RTCC initial time
//   DATEH = (initialTime->tm_year << 8) | (initialTime->tm_mon) ;  // YEAR/MONTH-1
//   DATEL = (initialTime->tm_mday << 8) | (initialTime->tm_wday) ;  // /DAY-1/WEEKDAY
//   TIMEH = (initialTime->tm_hour << 8) | (initialTime->tm_min); // /HOURS/MINUTES
//   TIMEL = (initialTime->tm_sec << 8);   // SECONDS   
//           
//   // Enable RTCC, clear RTCWREN         
//   RTCCON1Lbits.RTCEN = 1;  
//   RTCC_LockOff();
//
//   //Enable RTCC interrupt
//   IEC3bits.RTCIE = 1;
//}
//
///**
// This function implements RTCC_TimeReset.This function is used to
// used by application to reset the RTCC value and reinitialize RTCC value.
//*/
//void RTCC_TimeReset(bool reset)
//{
//    rtccTimeInitialized = reset;
//}
//
//static bool RTCCTimeInitialized(void)
//{
//    return(rtccTimeInitialized);
//}
//
//void RTCC_TimestampAEventManualSet(void)
//{
//    RTCSTATLbits.TSAEVT = 1;
//}
//
//bool RTCC_TimestampADataGet(struct tm *currentTime)
//{
//    uint16_t register_value;
//    if(!RTCSTATLbits.TSAEVT){
//        return false;
//    }
//  
//    register_value = TSADATEH;
//    currentTime->tm_year = ConvertBCDToHex((register_value & 0xFF00) >> 8);
//    currentTime->tm_mon = ConvertBCDToHex(register_value & 0x00FF);
//    
//    register_value = TSADATEL;
//    currentTime->tm_mday = ConvertBCDToHex((register_value & 0xFF00) >> 8);
//    currentTime->tm_wday = ConvertBCDToHex(register_value & 0x00FF);
//    
//    register_value = TSATIMEH;
//    currentTime->tm_hour = ConvertBCDToHex((register_value & 0xFF00) >> 8);
//    currentTime->tm_min = ConvertBCDToHex(register_value & 0x00FF);
//    
//    register_value = TSATIMEL;
//    currentTime->tm_sec = ConvertBCDToHex((register_value & 0xFF00) >> 8);
//   
//    RTCSTATLbits.TSAEVT = 0;
//
//    return true;
//}
//
//
//
//bool RTCC_TimestampA_BCDDataGet(bcdTime_t *currentTime)
//{
//    uint16_t register_value;
//    if(!RTCSTATLbits.TSAEVT){
//        return false;
//    }
//  
//    register_value = TSADATEH;
//    currentTime->tm_year = (register_value & 0xFF00) >> 8;
//    currentTime->tm_mon = (register_value & 0x00FF);
//    
//    register_value = TSADATEL;
//    currentTime->tm_mday = (register_value & 0xFF00) >> 8;
//    currentTime->tm_wday = (register_value & 0x00FF);
//    
//    register_value = TSATIMEH;
//    currentTime->tm_hour = (register_value & 0xFF00) >> 8;
//    currentTime->tm_min = (register_value & 0x00FF);
//    
//    register_value = TSATIMEL;
//    currentTime->tm_sec = (register_value & 0xFF00) >> 8;
//   
//    RTCSTATLbits.TSAEVT = 0;
//
//    return true;
//}
//

//
static uint8_t ConvertHexToBCD(uint8_t hexvalue)
{
    uint8_t bcdvalue;
    bcdvalue = (hexvalue / 10) << 4;
    bcdvalue = bcdvalue | (hexvalue % 10);
    return (bcdvalue);
}

static uint8_t ConvertBCDToHex(uint8_t bcdvalue)
{
    uint8_t hexvalue;
    hexvalue = (((bcdvalue & 0xF0) >> 4)* 10) + (bcdvalue & 0x0F);
    return hexvalue;
}
//
//
///* Function:
//  void __attribute__ ( ( interrupt, no_auto_psv ) ) _ISR _RTCCInterrupt( void )
//
//  Summary:
//    Interrupt Service Routine for the RTCC Peripheral
//
//  Description:
//    This is the interrupt service routine for the RTCC peripheral. Add in code if 
//    required in the ISR. 
//*/
//void __attribute__ ( ( interrupt, no_auto_psv ) ) _ISR _RTCCInterrupt( void )
//{
//    /* TODO : Add interrupt handling code */
//    IFS3bits.RTCIF = false;
//}
/**
 End of File
*/
