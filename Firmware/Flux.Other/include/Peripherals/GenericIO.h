/* 	
	Karim Ladha 24-06-2015
	Located in: Flux/Include/Peripherals/GenericIO.h
	Written to allow gpio pin control across hardware platforms
	Required to allow simpler cross platform peripheral drivers
	
	USAGE NOTES:
	The IO_pin_t structure requires proper initialisation to function properly.
	The port registers must have been properly initialised also.
	
	PIC24 & dsPIC30/33:
	For the PIC and dsPIC devices, the read and write functions can use either
	the PORT or LAT registers. However, read access must use PORT if the pin
	has a TRIS of 1 and LAT must be used to allow reliable writes to loaded pins. 
	
*/
#ifndef GENERIC_IO_H
#define GENERIC_IO_H

// Include
#include <stdint.h>

/* This type is sufficient for reading and writing pins on virtually all devices */
typedef struct {
	void* 	port_address;	// Can be a readable or writeable address
	int 	port_mask;		// Ansi C guarantees bit count >= address bits
} IO_pin_t;

#define C30_USE_ASM_VOLATILES_IN_GENERIC_IO_H			// [dgj] Think this won't work until the order of the "#elif"s below are swapped with the "#if __C30__"


/* In-line function implementations are hardware platform specific */
#ifdef __C30__
static int inline __attribute__((always_inline)) ReadIO(IO_pin_t pin)
{
	volatile uint16_t* latch = (void*)pin.port_address;
	uint16_t mask = pin.port_mask;
	if((*latch) & mask)return 1;
	return 0;
}
#elif defined(C30_USE_ASM_VOLATILES_IN_GENERIC_IO_H)
static int inline __attribute__((always_inline)) ReadIO(IO_pin_t pin)
{
	asm volatile (
	"mov.w [%[port]], w2 \n\t"
	"mov.w %[mask], w3 \n\t"
	"and w2, w3, %[port] \n\t"
	:"=r"(pin.port_address)
	:[port]"r"(pin.port_address),[mask]"r"(pin.port_mask)
	:"w2", "w3", "cc");
	return (int)pin.port_address;
}
#endif

#ifdef __C30__
static void inline __attribute__((always_inline)) SetIO(IO_pin_t pin)
{
	volatile uint16_t* latch = (void*)pin.port_address;
	uint16_t mask = pin.port_mask;
	*latch = (*latch) | mask;
}
#elif defined(C30_USE_ASM_VOLATILES_IN_GENERIC_IO_H)
static void inline __attribute__((always_inline)) SetIO(IO_pin_t pin)
{
	asm volatile (
	"mov.w %[latch], w2 \n\t"
	"mov.w %[mask], w3 \n\t"
	"ior.w w3, [w2], [w2] \n\t"
	:"=r"(pin.port_address)
	:[latch]"r"(pin.port_address),[mask]"r"(pin.port_mask)
	:"w2", "w3", "cc");
}
#endif

#ifdef __C30__
static void inline __attribute__((always_inline)) ClearIO(IO_pin_t pin)
{
	volatile uint16_t* latch = (void*)pin.port_address;
	uint16_t mask = ~(pin.port_mask);
	*latch = (*latch) & mask;
}
#elif defined(C30_USE_ASM_VOLATILES_IN_GENERIC_IO_H)
static void inline __attribute__((always_inline)) ClearIO(IO_pin_t pin)
{
	asm volatile (
	"mov.w [%[latch]], w2 \n\t"
	"mov.w %[mask], w3 \n\t"
	"xor.w w2, w3, w3 \n\t"
	"and.w w3, [%[latch]], [%[latch]] \n\t"
	:"=r"(pin.port_address)
	:[latch]"r"(pin.port_address),[mask]"r"(pin.port_mask)
	:"w2", "w3", "cc");
}
#endif



#ifndef __C30__
/* Tested on PIC24 & dsPIC33/30 devices */
/* Check the this works for the hardware platform used, or	*/
/* add hardware specific functions that do work.		 	*/
#error "Test compatibility with this hardware platform."	
/* Example functions */
static int inline __attribute__((always_inline)) ReadIO(IO_pin_t pin)
{
	return ((*(int*)pin.port_address & pin.port_mask) != 0) ? 1 : 0;
}
static void inline __attribute__((always_inline)) SetIO(IO_pin_t pin)
{
	*(int*)pin.port_address |= pin.port_mask;
}
static void inline __attribute__((always_inline)) ClearIO(IO_pin_t pin)
{
	*(int*)pin.port_address &= ~pin.port_mask;
}

#endif

#endif
