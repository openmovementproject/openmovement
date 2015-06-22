// Configuration, definitions and constants
// Karim Ladha, 2015

// Create locally per project
#ifndef _CONFIG_
#define _CONFIG_

// Firmware version
#define FIRMWARE_VERSION 		"1.0"

// Debug settings

// Interrupt priority levels
// Bluetooth UART ints
#define BT_UART_RX_INT_PRIORITY			6 	/* This makes the uart appear to be a DMA device to the CPU if IPL is < 7*/
#define BT_UART_TX_INT_PRIORITY			5	/* This makes the uart appear to be a DMA device to the CPU if IPL is < 6*/
#define BT_UART_CTS_INT_PRIORITY		BT_UART_TX_INT_PRIORITY

// Highest - RTC, TIMER tasks
#define RTC_INT_PRIORITY				5	/*Prevents ms rollover during reads*/
#define T1_INT_PRIORITY					RTC_INT_PRIORITY 	

// Mid level - FIFO time stamps only
#define FIFO_INTERRUPT_PRIORITY 		4
#define BT_EVENT_HANDLER_PRIORITY		FIFO_INTERRUPT_PRIORITY /*If using bt i/o fifos*/
// Off chip peripherals
#define PERIPHERAL_INTERRUPT_PRIORITY	FIFO_INTERRUPT_PRIORITY
#define ONE_SECOND_TRIGGER_INT_PRIORITY PERIPHERAL_INTERRUPT_PRIORITY
#define SAMPLER_INT_PRIORITY			PERIPHERAL_INTERRUPT_PRIORITY
#define ACCEL_INT_PRIORITY				PERIPHERAL_INTERRUPT_PRIORITY
#define GYRO_INT_PRIORITY				PERIPHERAL_INTERRUPT_PRIORITY
#define MAG_INT_PRIORITY				PERIPHERAL_INTERRUPT_PRIORITY
#define CN_INTS_PROCESSOR_PRIORITY		PERIPHERAL_INTERRUPT_PRIORITY

// File system options - see FSconfig.h
#define USE_FAT_FS

// Volatile memory configuration
// USB serial + line buffer settings 
#define LOCK_PRINTF_USBCALLS_IN_INTERRUPTS_ON_WRITE
#define SERIAL_BUFFER_SIZE		64 
#define IN_BUFFER_CAPACITY 		128	
#define OUT_BUFFER_CAPACITY 	512

// Bluetooth serial + line fifo buffers
#define BT_LINE_BUFFER_SIZE		64
#define BT_IN_BUFFER_SIZE 		128
#define BT_OUT_BUFFER_SIZE		128

// Main data out fifo
#define STREAM_OUT_BUFFER_SIZE	2048	
	
// Non volatile memory usage
//#define PRODUCTION_BUILD				// Set to load production settings to .hex
// Definitions
#define WRITE_PAGE_SIZE			0x80	// Size of nvm page
#define ERASE_BLOCK_SIZE		0x400	// Size of nvm block
#define MIN_ALLOWED_WRITE_ADD	0x1400	// Write protect start of code (bootloader)
#define MAX_ALLOWED_WRITE_ADD	0x2A800	// Write protect end of code (scripts+config)
// Addresses
#define SETTINGS_ADDRESS		0x24C00 // Non volatile settings
#define BLUETOOTH_NVM_START 	0x25800 // Bluetooth nvm area
#define SELF_TEST_RESULTS		0x25000 // Self test results save address
#define SELF_TEST_SIZE			0x400	// Same as linker
#define SCRATCH_BLOCK_ADD		0x25400	// Use intermediary nvm block, stack size = page size

// File system application settings
#define FTL_INACTIVE_FLUSH_SECONDS 3

// System time module behaviour
#define SYS_TICK_RATE				32768ul
#define SYSTIME_NUM_SECOND_CBS		1	
#define SYSTIME_NUM_PERIODIC_CBS	1	/* Hardware limited to one */

// Software watchdog timer
#define RTC_SWWDT_TIMEOUT 		60 

// SamplerStreamer settings (WAX9 mode)
#define MINIMUM_T1_RATE			2
#define LOW_FREQ_TASKS_TICKS 	(65536UL/MINIMUM_T1_RATE)
#define USE_BAROMETER_TEMP
#define SAMPLE_LED				LED_B
#define SAMPLER_VECTOR			_IC1Interrupt	
#define SAMPLER_INTERRUPT_IE	IEC0bits.IC1IE
#define SAMPLER_INTERRUPT_IF	IFS0bits.IC1IF
#define SAMPLER_INTERRUPT_IP	IPC0bits.IC1IP	
#endif

