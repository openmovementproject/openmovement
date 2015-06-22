/*
	Karim Ladha 12-06-2015
	Full overhaul of the buggy ADC driver routines
	Several bug fixes including removal of code that set reserved bits in ADC registers
	Added functional FVR code to allow reliable Vdd measurement for precharge detection
	Includes time based charge progress implementation that hinges on persistent sram values
	Added conversion of light sensor and thermistor used on AX9 hardware
	
*/
#ifndef ANALOG_H
#define ANALOG_H

// Headers
#include <stdint.h>

// Types
typedef struct {
	// Raw samples
	uint16_t fvr;
	uint16_t batt;
	uint16_t light;
	uint16_t temp;
	// Converted values
	uint16_t vdd_mv;
	uint16_t light_lux;
	int16_t temp_x10C;
	uint16_t batt_percent;
	// ADC reference mV
	uint16_t adc_vref_mv;
	// Battery charging state
	uint8_t	chargingState;
	uint8_t	charged_percent;
	uint16_t battSaveRaw;
	uint16_t battSaveNot;
	uint16_t chargingTimer;
} adc_results_t; 

// Global results
extern adc_results_t adcResult;

// Prototypes
// Fast scan excluding fvr, updates battery capacity
void UpdateAdc(void);
// Powers analogue sensors, fvr and converts samples
void UpdateAdcFull(void);
// Estimate the battery capacity as percentage 
uint8_t AdcBattToPercent(uint16_t adcVal);
// Conversion function - raw sample to mv
uint16_t AdcToMillivolt(uint16_t adcVal);



#endif

