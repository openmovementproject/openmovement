// Device driver for the MCP3421 18 bit I2C ADC 
// KL 20-01-2012

#ifndef ADC_MCP3421_H
#define ADC_MCP3421_H

// Device defines for init function, use logical OR
#define MCP3421_START_SAMPLE 			0x80
#define MCP3421_SAMPLE_nREADY 			0x80
#define MCP3421_CONTINUOUS_CONVERSION 	0x10
#define MCP3421_SAMPLE_12BIT_4MS		0x00
#define MCP3421_SAMPLE_14BIT_17MS		0x04
#define MCP3421_SAMPLE_16BIT_67MS		0x08
#define MCP3421_SAMPLE_18BIT_267MS		0x0C
#define MCP3421_GAIN_1					0x00
#define MCP3421_GAIN_2					0x01
#define MCP3421_GAIN_4					0x02
#define MCP3421_GAIN_8					0x03

// Default: Useful for thermocouple measurements
#define MCP3421_DEFAULT	(MCP3421_SAMPLE_18BIT_267MS | MCP3421_GAIN_8)

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

#endif
//EOF
