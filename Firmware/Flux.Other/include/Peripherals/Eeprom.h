/*I2C code for reading or writing eeprom*/

#ifndef _EEPROM_H_
#define _EEPROM_H_

// Defines
extern unsigned char eepromPresent;

// Function Prototypes
unsigned char I2C_eeprom_present(void);
unsigned char I2C_eeprom_busy(void);
unsigned char I2C_eeprom_wait(void);
unsigned char I2C_eeprom_write(unsigned long destination, unsigned char* data,unsigned short len);
unsigned char I2C_eeprom_read( unsigned long source, unsigned char* destination, unsigned short len);

#endif
//EOF
