/*
PeripheralBus.c/.h written by Karim Ladha, March 2015
This source code is written to simplify the access of external peripherals for embedded devices.
The primary goal is to provide a unified i2c (and other) register interface to meet the common peripheral requirements:
1) Perform a bus initialisation and close
2) Write a set of registers
3) Read a set of registers
4) Checking, time outs and fault handling
The interface is broken into compact scriptable commands and callbacks. 
A devices scripts are made by a device driver which will normally use them directly in device specific function.
Scripts are typically constant rom entries invoked at run time to provide a specific function e.g:
1) Device init
2) Device setup function
3) Device perform function
4) Device off
To accommodate multiple device types and interfaces, the available command set is kept simple and flexible. 
Unfortunately, for logical behaviour this requires the use of callback pointers which makes the code not very 8-bit friendly.
Hardware types each need a small set of functions for the peripheral bus access; hardware abstraction.
A timer must be available for timeouts; Uses SysTime module.
*/

// Includes
#include <stdint.h>
#include "Compiler.h"
#include "Peripherals/SysTime.h"
#include "Peripherals/PeripheralBus.h"
#include "HardwareProfile.h"
#include "Config.h"

// Debugging - not used
#if 0
#define DEBUG_LEVEL_LOCAL	DEBUG_PBS
#define DBG_FILE			"pbs.c"	
#include "debug.h"
#endif

#warning "TODO"
// Timouts from systime
// Testing

// Defaults
#ifndef PBS_DEFAULT_TIMEOUT
	#define PBS_DEFAULT_TIMEOUT (500000ul)
#endif

// Prototypes of handlers - Private
static inline int16_t pbs_open_bus(PBS_context* ctx);
static inline int16_t pbs_close_bus(PBS_context* ctx);
static inline int16_t pbs_set_ptr(PBS_context* ctx);
static inline int16_t pbs_read_reg(PBS_context* ctx);
static inline int16_t pbs_write_reg(PBS_context* ctx);
static inline int16_t pbs_write_list(PBS_context* ctx);
static inline int16_t pbs_callback(PBS_context* ctx);
static inline int16_t pbs_script(PBS_context* ctx);
static inline int16_t pbs_delay(PBS_context* ctx);

// Function table of handlers - aligned to command enum
static PBS_func_t pbs_handlers[] = {
	(PBS_func_t) NULL,			// No operation, minimum length delay
	(PBS_func_t) pbs_open_bus,	// Set up bus and settings to use
	(PBS_func_t) pbs_close_bus,	// Turn off the active bus	
	(PBS_func_t) pbs_set_ptr,	// Set the context pointer from the script
	(PBS_func_t) pbs_read_reg,	// Execute a reg read. Register x N -> ptr
	(PBS_func_t) pbs_write_reg,	// Execute a reg write. Register x N <- ptr
	(PBS_func_t) pbs_write_list,// Execute a list write. Register/value pairs
	(PBS_func_t) pbs_callback,	// Call the ptr as a function pointer
	(PBS_func_t) pbs_script,	// Run ptr as a nested script
	(PBS_func_t) pbs_delay,		// Delay for next uint16_t x10us
	(PBS_func_t) NULL			// Last item in a script or register list
};

// User source
// Initialise a context structure pointer
void PbsInitCtx(PBS_context* ctx)
{
	// Initialise a context ptr
	memset(ctx,0,sizeof(PBS_context));
	ctx->tout = PBS_DEFAULT_TIMEOUT;	
}
// Execute script without context using default time out
uint8_t PbsScript(uint8_t* script)
{
	// Make stack context
	PBS_context ctx;
	PbsInitCtx(&ctx);
	// Initialise blank context
	ctx.script = script;
	// Execute the script, return result
	return PbsScriptCtx(&ctx);
}
// Execute script with specific context - faster
uint8_t PbsScriptCtx(PBS_context* ctx)
{
	// Stack variables
	PBS_func_t handler;
	uint8_t cmd;
	
	// Setup timeout
	// TODO

	// Execute the script
	do{
		// Check timeout
		// TODO ctx->errno = PBS_ERR_TIMEOUT;

		// Get command number, check in range
		cmd = ctx->script[0];
		if(cmd > PBS_END)
			ctx->errno = PBS_ERR_BAD_CMD;

		// Check for errors, exit
		if(ctx->errno != PBS_ERR_NONE)
			break;
		
		// Call handler, go to next command
		handler = pbs_handlers[cmd];
		if(handler != NULL)
			ctx->script += handler(ctx);
	}while(cmd != PBS_END);

	// Return script result
	return (ctx->errno);
}

// Standard private functions - all hardware
static inline int16_t pbs_set_ptr(PBS_context* ctx)
{
	// Set ptr command pro-ceeded by ptr
	ctx->ptr = (void*)PBS_READ_PTR(&ctx->script[1]);
	return (1+sizeof(void*));
}
static inline int16_t pbs_callback(PBS_context* ctx)
{
	// Call back command, function must returns offset to next script command
	if(ctx->ptr != NULL){ctx->script += ((PBS_func_t)ctx->ptr)(ctx);}
	// The callback modifies the script ptr directly, return 0
	return 0; 
}
static inline int16_t pbs_script(PBS_context* ctx)
{
	// Save current position
	uint8_t* script_save = ctx->script;
	// Script command is pre-ceded by set ptr
	ctx->script = ctx->ptr;
	// Check and run nested script using current context
	if(ctx->script != NULL)
		PbsScriptCtx(ctx);
	// Restore and return
	ctx->script = script_save;
	return 1;
}
static inline int16_t pbs_delay(PBS_context* ctx)
{
	SysTimeDelayUs(PBS_READLE16(&ctx->script[1]));
	return (3); // Bytes in this command
}

// Hardware specific private functions
#ifdef __C30__
// Hardware abstraction for PIC24 device family
// I2C bus control
// I2C hardware abstraction using base ptr (all instances are the same, using I2C1 struct from device library)
#define I2C_READ_MASK			0x1 // OR mask to make a read address
#ifndef PDB_I2C_TIMEOUT_LEN			// May need overriding if a slave uses clock stretching
#define PDB_I2C_TIMEOUT_LEN			((I2CBRG + 1)<<5) // Num of cycles to send 16 bits (very conservative)
#endif
// PIC24 I2C peripheral registers
#define I2CRCV					(((uint16_t*)ctx->baseReg)[((&I2C1RCV - &I2C1RCV)>>1)])
#define I2CTRN					(((uint16_t*)ctx->baseReg)[((&I2C1TRN - &I2C1RCV)>>1)])
#define I2CBRG					(((uint16_t*)ctx->baseReg)[((&I2C1BRG - &I2C1RCV)>>1)])
#define I2CCON					(((uint16_t*)ctx->baseReg)[((&I2C1CON - &I2C1RCV)>>1)])
#define I2CCONbits				(*((I2C1CONBITS*)&I2CCON))
#define I2CSTAT					(((uint16_t*)ctx->baseReg)[((&I2C1STAT - &I2C1RCV)>>1)])
#define I2CSTATbits				(*((I2C1STATBITS*)&I2CSTAT))
#define I2CADD					(((uint16_t*)ctx->baseReg)[((&I2C1ADD - &I2C1RCV)>>1)])
// Macros to control I2C peripheral - written to only work inside loop allowing 'break' on error
#define PLL_DELAY_FIX()			{if(OSCCONbits.COSC==0b001){Nop();}} // PIC24 can not opperate I2C with PLL on without this delay
#define PDB_I2C_TIMEOUT(_v)		{if(_v<=0){ctx->errno=PBS_ERR_TIMEOUT;break;}}
#define PDBI2COpen(baud)		{I2CCON = 0x0000; I2CBRG = baud; I2CCON = 0xF200;}
#define PDBI2CClose()			{I2CCON = 0x0000;}
#define PDBI2CStart()			{uint16_t timeout = PDB_I2C_TIMEOUT_LEN;PLL_DELAY_FIX();I2CCONbits.SEN=1;while(I2CCONbits.SEN && --timeout);PDB_I2C_TIMEOUT(timeout);}
#define PDBI2CStop()			{uint16_t timeout = PDB_I2C_TIMEOUT_LEN;PLL_DELAY_FIX();I2CCONbits.PEN=1;while(I2CCONbits.PEN&& --timeout);PDB_I2C_TIMEOUT(timeout);}
#define PDBI2CRestart()			{uint16_t timeout = PDB_I2C_TIMEOUT_LEN;PLL_DELAY_FIX();I2CCONbits.RSEN=1;while(I2CCONbits.RSEN&& --timeout);PDB_I2C_TIMEOUT(timeout);}
#define PDBI2CAck()				{uint16_t timeout = PDB_I2C_TIMEOUT_LEN;PLL_DELAY_FIX();I2CCONbits.ACKDT = 0;I2CCONbits.ACKEN=1;while(I2CCONbits.ACKEN&& --timeout);PDB_I2C_TIMEOUT(timeout);}
#define PDBI2CNack()			{uint16_t timeout = PDB_I2C_TIMEOUT_LEN;PLL_DELAY_FIX();I2CCONbits.ACKDT = 1;I2CCONbits.ACKEN=1;while(I2CCONbits.ACKEN&& --timeout);PDB_I2C_TIMEOUT(timeout);}
#define PDBI2Cputc(_v)			{uint16_t timeout = PDB_I2C_TIMEOUT_LEN;PLL_DELAY_FIX();I2CTRN=_v;while(I2CSTATbits.TBF && --timeout);while(I2CSTATbits.TRSTAT && --timeout);PDB_I2C_TIMEOUT(timeout);} 
#define PDBI2Cgetc(_ptr)  		{PLL_DELAY_FIX();I2CCONbits.RCEN = 1; while(I2CCONbits.RCEN); I2CSTATbits.I2COV = 0; *(uint8_t*)_ptr = (I2CRCV);}
#define PDBAckStat()			(!I2CSTATbits.ACKSTAT) /* TRUE or 1 if slave acked */
#define PDBI2CCheckAck()		{if(ctx->errno != PBS_ERR_NONE)break; else if(!PDBAckStat()){ctx->errno = PBS_ERR_ACK;break;}}
// I2C baud rate register values (assumes PLL is 16MIPS, otherwise 4MIPS)
#define I2CBRG_100kHZ 			((OSCCONbits.COSC==1)? 157 : 39)	
#define I2CBRG_200kHZ 			((OSCCONbits.COSC==1)? 72 : 18)
#define I2CBRG_400kHZ 			((OSCCONbits.COSC==1)? 37 : 9)	
#define I2CBRG_1000kHZ 			((OSCCONbits.COSC==1)? 13 : 3)			
#define I2CBRG_2000kHZ			((OSCCONbits.COSC==1)? 7 : 1)
#define SET_I2CBRG(_v)			{	if(_v == I2C_RATE_2000kHZ)		I2CBRG = I2CBRG_2000kHZ;\
									else if(_v == I2C_RATE_1000kHZ)	I2CBRG = I2CBRG_1000kHZ;\
									else if(_v == I2C_RATE_4000kHZ)	I2CBRG = I2CBRG_400kHZ;\
									else if(_v == I2C_RATE_200kHZ)	I2CBRG = I2CBRG_200kHZ;\
									else I2CBRG = I2CBRG_100kHZ;}/*Default is 100kHz*/								
/*
Add other peripheral control bus abstraction/control definitions here.
e.g. SPI, UART, USART etc. 
*/

// Private functions for PIC24 instance
static inline int16_t pbs_open_bus(PBS_context* ctx)
{
	uint8_t baud;
	// Read bus number, set peripheral control base pointer (special function register)
	ctx->bus = ctx->script[1];
	switch(ctx->bus) {
		case (PBS_I2C1) : {ctx->baseReg = (void*)&I2C1RCV;break;}	// Address = 0x0200
		case (PBS_I2C2) : {ctx->baseReg = (void*)&I2C2RCV;break;}  	// Address = 0x020A
		case (PBS_I2C3) : {ctx->baseReg = (void*)&I2C3RCV;break;}  	// Address = 0x021A
		default : {
			// No other transports currently implemented
			ctx->errno = PBS_ERR_BUS;
			return 0;
		}
	}
	// Setup the selected peripheral bus 
	// PBS_OPEN_BUS command is proceeded by bus descriptor
	switch(ctx->bus) {
		case (PBS_I2C1) :
		case (PBS_I2C2) :
		case (PBS_I2C3) : {
			// I2C handlers
			baud = ctx->script[2];
			ctx->tout = PBS_READLE32(&ctx->script[3]);
			ctx->add = ctx->script[7];
			// Setup correct i2c peripheral for master mode			
			I2CCON = 0;
			//I2CBRG = SET_I2CBRG(baud);
			// Turn on I2C peripheral for master mode + 7 bit address
			I2CCON = 0xF200; 
			return (8); // Bytes in this command (1 + sizeof(PBS_I2C_settings))			
		}
	}
	// Must be an error if reaching here
	ctx->errno = PBS_ERR_BUS;
	return 0;	
}

static inline int16_t pbs_close_bus(PBS_context* ctx)
{
	// Turn off the contexts peripheral bus
	switch(ctx->bus) {
		case (PBS_I2C1) :
		case (PBS_I2C2) :
		case (PBS_I2C3) : {
			// I2C handlers, turn off correct i2c peripheral
			I2CCON = 0;
			return (1); // Bytes in this command			
		}
	}	
	// In error if reaching here
	ctx->errno = PBS_ERR_BUS;
	return 0;	
}

static inline int16_t pbs_read_reg(PBS_context* ctx)
{
	// Execute a reg read. Register x N -> ptr
	uint8_t reg = ctx->script[1], count = ctx->script[2];

	// Check command parameters
	if(count == 0) return 3;

	// Process command
	switch(ctx->bus) {
		case (PBS_RAM)	: {
			// RAM access handler, *ptr = *reg
			memcpy(ctx->ptr,(uint8_t*)reg,count);
			return 3;
		}
		case (PBS_I2C1) :
		case (PBS_I2C2) :
		case (PBS_I2C3) : {
			// I2C handlers, read routine
			// I2C start
			PDBI2CStart();
			for(;;)
			{
				// I2C write bus address, check for ack
				PDBI2Cputc(ctx->add);
				PDBI2CCheckAck();
				// Write device register address, check for ack
				PDBI2Cputc(reg);
				PDBI2CCheckAck();		
				// I2C Restart
				PDBI2CRestart();
				// I2C write bus address with read flag set
				PDBI2Cputc(ctx->add | I2C_READ_MASK);
				PDBI2CCheckAck();	
				// Read loop while no errors upto last value
				for(; count > 0; count--)
				{
					// Read in to the next context pointer and send ack/nack
					PDBI2Cgetc(ctx->ptr);
					*(uint8_t*)ctx->ptr++; 
					if(count>1)	{PDBI2CAck();}
					else		{PDBI2CNack();}
				}
				// Exit dummy for() loop
				break;
			}
			// I2C stop
			PDBI2CStop();
			// Return
			return (3); // Bytes in this command			
		}
	}		
	// In error if reaching here
	ctx->errno = PBS_ERR_BUS;
	return 0;	
}

static inline int16_t pbs_write_reg(PBS_context* ctx)
{
	// Execute a reg write. Register x N -> ptr
	uint8_t reg = ctx->script[1], count = ctx->script[2];
	
	// Check command parameters
	if(count == 0) return 3;

	// Process command
	switch(ctx->bus) {
		case (PBS_RAM)	: {
			// RAM access handler, *reg = *ptr
			memcpy((uint8_t*)reg,ctx->ptr,count);
			return 3;
		}
		case (PBS_I2C1) :
		case (PBS_I2C2) :
		case (PBS_I2C3) : {
			// I2C handlers, write routine
			// I2C start
			PDBI2CStart();
			for(;;)
			{
				// I2C write bus address, check for ack
				PDBI2Cputc(ctx->add);
				PDBI2CCheckAck();
				// Write device register address, check for ack
				PDBI2Cputc(reg);
				PDBI2CCheckAck();	
				// Write loop while acknowledged, no errors and all values
				for(count = ctx->script[2]; count > 0; count--)
				{
					// Write from the next context pointer
					PDBI2Cputc(*(uint8_t*)ctx->ptr++);
					// Check for acknowledge and errors
					PDBI2CCheckAck();
				}	
				// Exit dummy for() loop
				break;				
			}
			// I2C stop
			PDBI2CStop();
			// Return
			return (3); // Bytes in this command			
		}
	}	
	// In error if reaching here
	ctx->errno = PBS_ERR_BUS;
	return 0;	
}

static inline int16_t pbs_write_list(PBS_context* ctx)
{
	// Execute a reg write list in the script. Register + value pairs
	uint8_t count = ctx->script[1], *list = &ctx->script[2], reg, value;

	// Check for empty list, return bytes in command
	if(count == 0) return (2); 
				
	switch(ctx->bus) {
		case (PBS_I2C1) :
		case (PBS_I2C2) :
		case (PBS_I2C3) : {
			// I2C handlers, write list routine
			// I2C start
			PDBI2CStart();
			for(;;)
			{
				// I2C write bus address, check for ack
				PDBI2Cputc(ctx->add);
				PDBI2CCheckAck();
				// Write loop while acknowledged, no errors and for all values
				for(;count>0;count--)
				{
					// Get next register value pair
					reg = list[0];
					value = list[1];
					list += 2;	
					// Write device register address, check ack
					PDBI2Cputc(reg);
					PDBI2CCheckAck();	
					// Write device register value, check ack
					PDBI2Cputc(*(uint8_t*)ctx->ptr++);
					PDBI2CCheckAck();						
					// I2C Restart
					PDBI2CRestart();
				}
				// Exit dummy for() loop
				break;				
			}
			// I2C stop
			PDBI2CStop();
			// Return
			return (((int16_t)count<<1) + 2); // Bytes in this command			
		}
	}	
	// In error if reaching here
	ctx->errno = PBS_ERR_BUS;
	return 0;	
}
#else
// Add peripheral bus control functions for other hardware implementations
#error "Hardware selection failed"
static inline int16_t pbs_open_bus(PBS_context* ctx)	{return 0;}
static inline int16_t pbs_close_bus(PBS_context* ctx)	{return 0;}
static inline int16_t pbs_read_reg(PBS_context* ctx)	{return 0;}
static inline int16_t pbs_write_reg(PBS_context* ctx)	{return 0;}
static inline int16_t pbs_write_list(PBS_context* ctx)	{return 0;}
#endif

//EOF
