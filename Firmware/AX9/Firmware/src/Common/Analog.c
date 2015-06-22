/*
	Karim Ladha 12-06-2015
	Full overhaul of the buggy ADC driver routines
	Several bug fixes including removal of code that set reserved bits in ADC registers
	Added functional FVR code to allow reliable Vdd measurement for precharge detection
	Includes time based charge progress implementation that hinges on persistent sram values
	Added conversion of light sensor and thermistor used on AX9 hardware
	
*/
// Headers
#include <stdint.h>
#include "Compiler.h"
#include "Peripherals/SysTime.h"
#include "Analog.h"
#include "HardwareProfile.h"

// Globals - persistent to save last battery voltage through soft resets
__attribute__((persistent, address(0x1000))) adc_results_t adcResult;
const uint8_t battCapacity[]; /* Battery capacity look-up data */

// Prototypes
void AdcUpdateBattery(void);
void AdcUpdateSensors(void);
void AdcSampleFvr(void);
uint8_t BatteryPercentTimeBased(void);
uint8_t BatteryPercentCurveFit(uint16_t sample);
uint8_t BatteryPercentLookup(uint16_t sample);

// Source
void UpdateAdc(void)
{
	// Turn on the module
	PMD1bits.ADC1MD = 0;    // See device errata

	// Multi sample enabled channels
	AD1CON1bits.ADON = 0;
	AD1CON1 = 0x00E4; 	// Scan using Vdd as ref
	AD1CON2 = 0x0408; 	// <5-2> scan count = 3 (2)
	AD1CON3 = 0x8100; 	// Tad=250ns,Tsamp=Tad

	AD1CSSL = ~ADC_SELECT_L;
	AD1PCFGL = ADC_SELECT_L;
	AD1PCFGH = ADC_SELECT_H;
	AD1CON1bits.ADON = 1;
	
	// Perform conversion of scan inputs
	if (AD1CON1bits.ADON == 0) return; // Otherwise this code will hang forever
	IFS0bits.AD1IF = 0;		// Clear interrupt flag
	AD1CON1bits.ASAM = 1;	// Begin auto sampling
	while (!IFS0bits.AD1IF);// Wait for the allotted number of conversions set by ADC_INTR_?_CONV in config2
	AD1CON1bits.ASAM = 0;	// Stop auto sampling
	while(!AD1CON1bits.DONE);// Wait for partially complete samples
	
	// Save results
    adcResult.batt = *(&ADC1BUF0 + ADC_INDEX_BATT); 	// Battery
    adcResult.light  = *(&ADC1BUF0 + ADC_INDEX_LIGHT);	// Light
    adcResult.temp = *(&ADC1BUF0 + ADC_INDEX_TEMP); 	// Temp
	
	AD1CON1bits.ADON = 0;
	PMD1bits.ADC1MD = 1; // See device errata
}

// Update the ADC including light and thermistor
void UpdateAdcFull(void)
{
	// Turn on, wait, sample and turn off
	ANALOG_EN = 1;
	LED_SUSPEND();
	DelayMs(ANALOGUE_SETTLING_WAIT);
	UpdateAdc();
	ANALOG_EN = 0;
	LED_RESUME();
}

void AdcUpdateBattery(void)
{
	return;
}

void AdcUpdateSensors(void)
{
	return;
}

// The fixed bandgap reference is sampled to the results
void AdcSampleFvr(void)
{
	uint16_t i;
	PMD1bits.ADC1MD = 0;    // See device errata
	AD1CON1bits.ADON = 0;	// ADC off

	AD1CON1 = 0x00E0; 	// Sample using Vdd as ref
	AD1CON2 = 0x0000; 	// Sample channel A
	AD1CON3 = 0x1414; 	// Tad=Tcy/20,Tsamp=20Tad
	AD1CHS = 0x1111;	// Set channel A to band gap
	AD1CON1bits.ADON = 1;

	// Wait for ADC to power up
	for(i=100;i>0;i--)Nop(); 

	// FVR on and > 1ms delay for FVR settle on
	AD1PCFGH = ADC_SELECT_H & 0xfffd; 
	AD1PCFGL = ADC_SELECT_L;
	for(i=10000;i>0;i--)Nop(); 

	// Sample
	IFS0bits.AD1IF = 0;	
	AD1CON1bits.SAMP = 1;
	while (!IFS0bits.AD1IF);
	adcResult.fvr = ADC1BUF0;

	// Fvr off, undocumented errata, fvr settle off
	AD1PCFGH = ADC_SELECT_H; 
	AD1PCFGL = ADC_SELECT_L;
	for(i=100;i>0;i--)Nop(); 

	AD1CON1bits.ADON = 0;
	PMD1bits.ADC1MD = 1; // See device errata

}

// Convert adc reading to units of 0.001 V
uint16_t AdcToMillivolt(uint16_t adcVal)
{
    // Conversion to millivolts
    // Vbat = Vref * value / 1024, Vref = Vdd  
	uint32_t temp = adcVal;
	temp *= ANALOGUE_VREF_MV;
	temp >>= 10;
	return temp;
}


// Battery functions - Karim Ladha 13-06-2015
// The following code provides an approximate battery capacity 

// Time based charge approximation. Chargind at 1C yields approximately 2 hours charge time
#define CHARGE_SECONDS_MAX (3600ul*2) 

// Analogue conversions based on measured discharge curves
#if (ANALOGUE_VREF_MV == 3000)
	// Lookup table for battery capacity
	#define BATT_TABLE_OS 529																
	#define BATT_TABLE_MAX 708																
	const uint8_t battCapacity[180] = { /* Capacity = Table[(sample - 529)], for sample range 529 up to 708 */																
		2,   	2,   	2,   	2,   	2,   	3,   	3,   	3,   	3,   	3,   	3,   	3,   	3,   	3,   	3,   	3,   
		3,   	3,   	3,   	3,   	3,   	3,   	3,   	3,   	4,   	4,   	4,   	4,   	4,   	4,   	4,   	4,   
		4,   	4,   	4,   	4,   	4,   	5,   	5,   	5,   	5,   	5,   	5,   	5,   	5,   	5,   	5,   	6,   
		6,   	6,   	6,   	6,   	6,   	6,   	6,   	7,   	7,   	7,   	7,   	8,   	8,   	9,   	10,   	11,   
		13,   	14,   	15,   	16,   	17,   	18,   	19,   	21,   	22,   	24,   	27,   	29,   	32,   	35,   	38,   	40,   
		43,   	45,   	47,   	49,   	51,   	52,   	54,   	55,   	57,   	58,   	59,   	60,   	62,   	63,   	64,   	65,   
		66,   	67,   	68,   	69,   	69,   	70,   	71,   	72,   	73,   	74,   	75,   	76,   	77,   	78,   	78,   	79,   
		80,   	81,   	82,   	83,   	83,   	84,   	85,   	86,   	86,   	87,   	88,   	88,   	89,   	89,   	90,   	91,   
		91,   	92,   	93,   	93,   	94,   	95,   	95,   	96,   	96,   	96,   	97,   	97,   	98,   	98,   	98,   	98,   
		99,   	99,   	99,   	99,   												
	}; /* Battery capacity lookup table, 10bit ADC, 50% input divider and reference of 3000 mV */

	// Constants for curve fit													
	#define 	BATT_C1	704	// End of range 1		
	#define 	BATT_C2	187	// M value of range 1		
	#define 	BATT_C3	65	// C value of range 1		
	#define 	BATT_C4	656	// Start of range 1, end range 2		
	#define 	BATT_C5	401	// M value of range 2		
	#define 	BATT_C6	7	// C value of range 2		
	#define 	BATT_C7	619	// Start of range 2	
	
#elif (ANALOGUE_VREF_MV == 3300)

	// Lookup table for battery capacity
	#define BATT_TABLE_OS 514																
	#define BATT_TABLE_MAX 643																
	const uint8_t battCapacity[130] = { /* Capacity = Table[(sample - 514)], for sample range 514 up to 643 */																
		5,   	5,   	5,   	5,   	5,   	5,   	5,   	6,   	6,   	6,   	6,   	6,   	6,   	6,   	7,   	7,   
		7,   	7,   	8,   	8,   	9,   	11,   	13,   	14,   	15,   	16,   	17,   	19,   	20,   	22,   	24,   	26,   
		29,   	32,   	35,   	38,   	41,   	44,   	46,   	48,   	50,   	52,   	54,   	55,   	57,   	58,   	60,   	61,   
		62,   	63,   	64,   	65,   	66,   	68,   	69,   	70,   	71,   	72,   	73,   	74,   	75,   	76,   	77,   	77,   
		78,   	79,   	80,   	81,   	82,   	83,   	84,   	85,   	85,   	86,   	87,   	88,   	88,   	89,   	90,   	90,   
		91,   	92,   	93,   	93,   	94,   	95,   	95,   	96,   	96,   	97,   	97,   	97,   	98,   	98,   	98,   	99,   
		99,   	99,   														
	}; /* Battery capacity lookup table, 10bit ADC, 50% input divider and reference of 3300 mV */															

	// Constants for curve fit													
	#define 	BATT_C1	640	// End of range 1	
	#define 	BATT_C2	208	// M value of range 1	
	#define 	BATT_C3	65	// C value of range 1	
	#define 	BATT_C4	597	// Start of range 1, end range 2	
	#define 	BATT_C5	437	// M value of range 2	
	#define 	BATT_C6	7	// C value of range 2	
	#define 	BATT_C7	563	// Start of range 2	

#else
	#error "No battery data for this selection."
#endif

/* 	Battery capacity using time based charge model
	This removed the over prediction caused while charging
	The over prediction is caused by the battery resistance
	The resistance and error increases with battery age
	The model requires the total expected charge time for the hardware
	The battery level before charging begins is the start point
	A persistent ram variable is used to avoid frequent NVM writes
 */
uint8_t BatteryPercentTimeBased(void)
{
	// Estimate as a fraction of charging time
	// Division performed by multiplicative inverse to save instruction cycles
	uint32_t temp;
	temp = 100 - ((100ul * adcResult.chargingTimer) / CHARGE_SECONDS_MAX);
	return temp & 0xff;
}

/* 	Battery capacity from an ADC conversion
	Expects 10 bit ADC value with 50% divided battery level
	Constants depend upon the ADC reference voltage
	See spread sheet for making constants */
uint8_t BatteryPercentCurveFit(uint16_t sample)
{
	uint32_t temp;
	uint8_t capacity;
	if (sample > BATT_C1)
	{
		capacity = 100;
	}
	else if (sample >= BATT_C4)
	{
		temp = sample - BATT_C4;
		temp = ((temp * BATT_C2) >> 8) + BATT_C3;
		capacity = temp & 0xff;
	}
	else if (sample >= BATT_C7)
	{
		temp = sample - BATT_C7;
		temp = ((temp * BATT_C5) >> 8) + BATT_C6;
		capacity = temp & 0xff;
	}
    	else // sample < BATT_C7 
	{
		capacity = 0;
	}
	return capacity;
}

/* 	Battery capacity from an ADC conversion
	Uses a look up table covering a limited ADC range
	The data is for a 50% divided battery voltage and 10 bit ADC
	The data is for a specific ADC reference voltage
	See spread sheet to generate data for other references */
uint8_t BatteryPercentLookup(uint16_t sample)
{
	if (sample > BATT_TABLE_MAX) return 100;
	if (sample < BATT_TABLE_OS) return 0;	
	return battCapacity[(sample - BATT_TABLE_OS)];
}

//EOF
