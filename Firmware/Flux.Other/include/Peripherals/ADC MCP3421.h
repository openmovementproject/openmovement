// Device driver for the MCP3421 18 bit I2C ADC 
// KL 20-01-2012

#ifndef ADC_MCP3421_H
#define ADC_MCP3421_H

// Device defines for init function, use logical OR
#define MCP3421_START_SAMPLE 			0x80
#define MCP3421_SAMPLE_nREADY 			0x80
#define MCP3421_CONTINUOUS_CONVERSION 	0x10
#define MCP3421_ONESHOT_CONVERSION 		0x00
#define MCP3421_SAMPLE_12BIT_4MS		0x00
#define MCP3421_SAMPLE_14BIT_17MS		0x04
#define MCP3421_SAMPLE_16BIT_67MS		0x08
#define MCP3421_SAMPLE_18BIT_267MS		0x0C
#define MCP3421_GAIN_1					0x00
#define MCP3421_GAIN_2					0x01
#define MCP3421_GAIN_4					0x02
#define MCP3421_GAIN_8					0x03

#define USE_THERMOCOUPLE_TYPE_K  /*Enables function in C file*/
#define MCP3421_DEFAULT_TYPE_K	(MCP3421_SAMPLE_18BIT_267MS | MCP3421_GAIN_8)

// Call to see if a defice acks / i.e. is present
unsigned char MCP3421Verify(void);
// Initialise the device - call first with defined arguments
void MCP3421Init(unsigned char setup);
// Shut down the device
void MCP3421Off(void);
// Initiate conversion
void MCP3421Sample(void);
// Returns true is complete - better to just wait though for lowest power
unsigned char MCP3421CheckComplete(void);
// Note: ssumes the current conversion is over
signed long MCP3421Read(void);
// Convert to 8bit+8bit signed fractional -128 to 127 degrees C (same as MCP9800)
signed short MCP3421CalcTemperatureFractional (signed long conversionResult);
// Calculate junction temperature in 0.1C steps from MCP3421 result and MCP9800 conversion (coldJunction is native MCP9800 format)
signed int MCP3421CalcTemperature (signed long conversionResult, signed int coldJunction);
#endif
//EOF
