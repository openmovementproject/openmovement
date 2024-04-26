/*	Multi-function generic time/timing module - bare bones implementation
	Used to keep the time valid without using much code space. e.g. If stuck inside bootloader for a portion of time
	This is a SysTime driver for ultra minimal implementations:
		Load/read epoch
		Write epoch
		Increment second epoch
	Karim Ladha, 2017 Revisions:	
	2017-02-28: First written
	
*/
// Includes
#include <stdint.h>
#include <string.h>
#include "Peripherals/SysTime.h"
#include "app_config.h"
#include "nrf51.h"

// Persistent epoch value - create in persistent memory section
extern EpochTime_t rtcEpochTriplicate[3];

/* Load/read triplicate values */
uint8_t SysTimeInit(void)
{
	/* Read with triplicate correction */
	/* val[0] agrees with either val[1] or val[2]*/
	if(	(rtcEpochTriplicate[1] == rtcEpochTriplicate[0]) || 
		(rtcEpochTriplicate[2] == rtcEpochTriplicate[0]) )
		{;}
	/* val[0] disagrees, but val[1] matches val[2] */
	else if(rtcEpochTriplicate[2] == rtcEpochTriplicate[1])
		rtcEpochTriplicate[0] = rtcEpochTriplicate[1];
	/* All values disagree */
	else  
		rtcEpochTriplicate[0] = 0;
	// Set all to equal val[0]
	rtcEpochTriplicate[1] = rtcEpochTriplicate[0];
	rtcEpochTriplicate[2] = rtcEpochTriplicate[0];	
}
EpochTime_t SysTimeEpoch(void)
{
	return rtcEpochTriplicate[0];
}

/* Write epoch value */
uint8_t SysTimeSetEpoch(uint32_t epoch)
{
	/* Write with triplicate redundancy */
	rtcEpochTriplicate[0] = epoch;
	rtcEpochTriplicate[1] = rtcEpochTriplicate[0];
	rtcEpochTriplicate[2] = rtcEpochTriplicate[0];	
}

/* Epoch tick increment - unprotected */
void SysTimeEpochInc(void)
{
	/* Triplicate update of epoch*/
	rtcEpochTriplicate[0]++;
	rtcEpochTriplicate[1] = rtcEpochTriplicate[0];
	rtcEpochTriplicate[2] = rtcEpochTriplicate[0];
}

// Restart WDT if running - minimalist
#ifdef NRF51
inline __attribute__((always_inline)) void ClrWdt(void)
{
	// If the WDT is running, service enabled channels
	if(NRF_WDT->RUNSTATUS)
	{
		uint32_t channel, mask;
		// Write special value to all enabled channels, check with bit mask
		for(channel = 0, mask = 1; channel < NRF_WDT_CHANNEL_NUMBER; channel++, mask <<= 1)
		{
			// For all enable reset reload registers, write special reset value
			if( NRF_WDT->RREN & mask ) 
				NRF_WDT->RR[channel] = NRF_WDT_RR_VALUE;
		}
	}
	// All channels request WDT reset, countdown is restarted
}
#endif
// EOF

