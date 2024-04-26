// Karim Ladha, 18-03-2016
// Generic digital altimeter API
#ifndef __ALT_H__
#define __ALT_H__

#include <stdint.h>

/* 
Required definitions for future device drivers
#define ALTIMETER_???
#define ALT_ALTERNATE_I2C	1
#define ALT_INIT_PINS()  	{}
*/

// Globals
extern uint8_t altPresent;

// Prototypes
/* Call first to detect the device, returns true for success */
uint8_t AltVerifyDeviceId(void);
/* Initialise altimeter device and read calibration */
uint8_t AltInit(void);
/* Put device into lowest power state */
void AltStandby(void);
/* Call to convert and return the last temp reading in 0.1^C */
int16_t AltCalcTemp(void); 
/* Call to convert and return the last pressure reading in Pa */
int32_t AltCalcPressure(void);
/* Begin the pressure (and temp if combined) conversion */
void AltSample(void);
/* Read the last pressure (and temp if combined) conversion into the global struct */
uint8_t AltReadLast(void);
/* Begin the temperature conversion (if separate) */
void AltSampleTemp(void);
/* Read the last temperature (if separate) conversion into the global struct */
uint8_t AltReadLastTemp(void);
/* Call to convert pressure to altitude*/
int32_t CalculateAltitude(int32_t pressure, int32_t pressure_sea_level);


#endif   // __ALT_H__
