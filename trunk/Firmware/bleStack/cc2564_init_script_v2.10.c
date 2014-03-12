// init script created from bluetooth_init_cc2564_2.10.bts
#include <stdint.h>
#include "bt config.h"
#if 0
const unsigned char cc2564_init_script[] = "test"; // turn off script in prog space to measure prog total size

#elif defined( INIT_SCRIPT_IN_PROG_SPACE) || defined (INIT_SCRIPT_IN_EXT_EEPROM)

#if !defined (INIT_SCRIPT_BASE_ADD)
	#warning "Linker decides init script location"
	__prog__ unsigned char __attribute__((space(prog)))
#else
	__prog__ unsigned char __attribute__((space(prog),address((INIT_SCRIPT_BASE_ADD))))
#endif
cc2564_init_script[] =
{

#error "For licensing reasons, the TI CC2564 initialization script is not included"

};

const unsigned long cc2564_init_script_size = sizeof(cc2564_init_script);
#endif
