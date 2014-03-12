// init script for cc2560 extracted from Stellaris Bluetopia SDK
#include <stdint.h>
#include "bt config.h"

#if 0
const unsigned char cc2560_init_script[] = "test"; // turn off script in prog space to measure prog total size

#elif defined( INIT_SCRIPT_IN_PROG_SPACE) || defined (INIT_SCRIPT_IN_EXT_EEPROM)

#if !defined (INIT_SCRIPT_BASE_ADD)
	#warning "Linker decides init script location"
	__prog__ uint8_t __attribute__((space(prog)))
#else
	__prog__ uint8_t __attribute__((space(prog),address((INIT_SCRIPT_BASE_ADD))))
#endif
cc2560_init_script[] =
{
#error "For licensing reasons, the TI CC2560 initialization script (part 1) is not included"
};

__prog__ uint8_t __attribute__((space(prog),address((INIT_SCRIPT_BASE_ADD)+sizeof(cc2560_init_script))))
cc2560_init_script_2[] =
{
#error "For licensing reasons, the TI CC2560 initialization script (part 2) is not included"
};
//const uint32_t cc2560_init_script_size = sizeof(cc2560_init_script) + sizeof(cc2560_init_script_2);

#endif
