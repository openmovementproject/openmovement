// KL 2011

// Headers
#include "Compiler.h"
#include "TimeDelay.h"
#include "HardwareProfile.h"
#include "Peripherals\Uart_vdma.h"

#include "config.h"
#include "btstack\hal_uart_dma.h"
#include "btstack\hal_cpu.h"
#include "btstack\hal_tick.h"

#ifdef USE_T1_FOR_TICK
	#include "Peripherals/Timer1.h"
#elif defined  USE_RTC_FOR_TICK
	#include "Peripherals/Rtc.h" // For timeout
#else
	#warning "Default behaviour"
	#include "Peripherals/Rtc.h" // For timeout
#endif

// Globals
volatile unsigned char uartNeededDuringSleep = TRUE;		// Don't use this, use getter() in bluetooth.h
static volatile unsigned short IPL_shadow = 4; 				// 4 is the reset value

static void (*cts_irq_handler)(void) = NULL;
static void (*tick_handler)(void) = NULL;
// Code
void hal_uart_dma_init(void)			
{
	// Initialise and reset module
	BT_INIT_PINS(); 
	// Turn on the modules slow clock
	SETUP_BT_SLW_CLK();
	// Power module
	BT_EN = 1;
	DelayMs(100);
	// Start comms
	UartVdmaInit();
}

void hal_uart_dma_init_off(void)			
{
	// Stop comms - quickly kill interrupts
	hal_cpu_disable_irqs();
	BT_eHCILL_ENABLE = 0;
	hal_tick_set_handler(NULL);
	UartVdmaOff();
	hal_cpu_enable_irqs();
	// Power off module
	BT_INIT_OFF();
	// Turn on the modules slow clock
	STOP_BT_SLW_CLK();
}

int hal_uart_dma_set_baud(uint32_t baud){return UartVdmaSetBaud(baud);}

void hal_uart_dma_shutdown(void) 		{UartVdmaOff();}

void hal_uart_dma_set_block_received( void (*the_block_handler)(void))
{
  	UartVdmaSetRxCallback(the_block_handler);
}

void hal_uart_dma_set_block_sent( void (*the_block_handler)(void))
{
    UartVdmaSetTxCallback(the_block_handler);
}

void hal_uart_dma_set_csr_irq_handler( void (*the_irq_handler)(void))
{
	// Enable eHCILL wake event interrupt - default is low to high transition
	BT_eHCILL_PRIORITY = BT_UART_CTS_INT_PRIORITY;	// Set priority
	BT_eHCILL_FLAG = 0;								// Clear the flag
	BT_eHCILL_ENABLE = 1;							// Enable int - the ISR handles the wake event
    cts_irq_handler = the_irq_handler; 				// Assign fptr
}

void hal_uart_dma_send_block(const uint8_t * data, uint16_t len)
{ 
	UartVdmaSendBlock(data, len);
}

// int used to indicate a request for more new data
void hal_uart_dma_receive_block(uint8_t *buffer, uint16_t len)
{
	UartVdmaReceiveBlock(buffer, len);
}

// Put uart to inactive mode - called on entry to eHCILL low power mode
void hal_uart_dma_set_sleep(uint8_t sleep)
{
    uartNeededDuringSleep = !sleep; // Indicate to loop that it can be off
}

// Support for eHCILL signalling
void __attribute__((interrupt,auto_psv)) BT_eHCILL_VECTOR(void)	//eHCILL wake interrupt
{
	BT_eHCILL_FLAG = 0;				// Clear flag
	if (!uartNeededDuringSleep)		// Assume we have woken from sleep if this is clear
	{
		if (OSCCONbits.COSC == 0b001)
			{while (OSCCONbits.LOCK != 1){;}} // Wait for HW to indicate pll lock
		DelayMs(3);					// Await clock switch stabilisation (will have woken from sleep)
	}
#ifdef DUMP
	printf("CTS\n");
#endif
	if(cts_irq_handler != NULL)		// Guard NULL pointer
		{(*cts_irq_handler)();}		// Call the wake handler - CLEARS RTS in handler
	return;
}

/*
	The following three functions set the low power operation options for BTstack.
	After each loop interation a flag (trigger_event_received) is set to indicate if
	the device can sleep or if it needs to loop again. This flag is still set if the 
	uart has transmissions to complete - The device must idle in these cases and run
	the loop again when they complete. 
*/
void hal_cpu_disable_irqs(){
	// Turn off interrupts that could call embedded trigger
	IPL_shadow = SRbits.IPL;
	SRbits.IPL = 7;
	
}

void hal_cpu_enable_irqs(){
	// Restore all interrupts that were disabled
	SRbits.IPL = IPL_shadow;	
}

//example - use if(BluetoothGetPowerState() <= BT_SLEEP)
#ifdef HAVE_BLE
void hal_cpu_enable_irqs_and_sleep(){
	hal_cpu_enable_irqs();
	// Determine most suitable low power mode
	if (uartNeededDuringSleep)
	{
		// We should idle to keep uart working
		Idle();				// Interrupts must be enabled to wake the device
	}
	else
	{
		// We can sleep, uart not needed
		Sleep();			// Expect a CTS pin interrupt or Tick interrupt
	}
	return;
}
#endif

// Timer code - placed here to unify the C code files
void hal_tick_init(void){
#ifndef BT_MANAGE_OWN_TICK_SETTING
	#if (MS_PER_TICK == 1000)
		#ifdef USE_RTC_FOR_TICK
		if (IEC3bits.RTCIE==0)	RtcInterruptOn(0);
		#elif defined(USE_T1_FOR_TICK) 
		TimerInterruptOn(0);
		#else
		#error "No tick"
		#endif
	#else
		#ifdef USE_RTC_FOR_TICK
		#warning "Not recommended, must move tick handler from RTC int to T1 int."
		RtcInterruptOn((1000/MS_PER_TICK));
		#elif defined(USE_T1_FOR_TICK) 
		TimerInterruptOn((1000/MS_PER_TICK));
		#else
		#error "No tick"
		#endif
	#endif
#endif
}

void hal_tick_set_handler(void (*handler)(void)){
    tick_handler = handler;
}

int  hal_tick_get_tick_period_in_ms(void){
    return MS_PER_TICK;
}

void BT_tick_handler(void)
{
    if (tick_handler != NULL)
		{tick_handler();}
}


// EOF
