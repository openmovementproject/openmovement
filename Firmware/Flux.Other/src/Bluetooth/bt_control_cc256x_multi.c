/*
 * Copyright (C) 2011 by Matthias Ringwald
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MATTHIAS RINGWALD AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 *  bt_control_cc256x.c
 *
 *  Adapter to use cc256x-based chipsets with BTstack
 *  
 *  Handles init script (a.k.a. Service Patch)
 *  Allows for non-standard UART baud rate
 *  Allows to configure transmit power
 *  Allows to activate eHCILL deep sleep mode
 *
 *  Issues with mspgcc LTS:
 *  - 20 bit support is not there yet -> .text cannot get bigger than 48 kb
 *  - arrays cannot have more than 32k entries
 * 
 *  workarounds:
 *  - store init script in .fartext and use assembly code to read from there 
 *  - split into two arrays
 *  
 *  Re worked for C30 compiler Karim Ladha 2011-12
 */

#include "Compiler.h"
#include "HardwareProfile.h"
#include "Bluetooth/Bluetooth.h"
#include "config.h"
#include "stdint.h"
#include "bt_control.h"
#include "Bluetooth/bt_control_cc256x.h"

// Globals
extern const uint32_t cc256x_init_script_size;
static unsigned int packets_sent = 0;
static uint32_t init_script_offset  = 0;
static int16_t  init_power_in_dB    = 13; // 13 dBm
static int      init_ehcill_enabled = 0;

#ifndef INIT_SCRIPT_PACKET_COUNT
	// Number of packets in the default script for the cc2560
	#define INIT_SCRIPT_PACKET_COUNT 300
#endif

#ifdef INIT_SCRIPT_IN_PROG_SPACE 
	extern __prog__ uint8_t __attribute__((space(prog))) cc256x_init_script[];
	unsigned char InitScriptOpen(void)
	{
		// No inititialisation needed for prog space
		return 1;
	}
	void InitScriptReadBytes(unsigned char* dest , unsigned short num)
	{
		ReadProgram(((unsigned long)cc256x_init_script + init_script_offset), dest, num);
	}
	void InitScriptClose(void)
	{
		// No closing proceduresfor prog space
	}
#elif defined INIT_SCRIPT_IN_FILE
	#include "MDD File System/FSIO.h"
	FSFILE* init_script = NULL; 
	unsigned char InitScriptOpen(void)
	{
		if(FSInit())
		{
			init_script = FSfopen ("BT_INIT.BIN",FS_READ);
			if (init_script==NULL)
			{
				#ifdef ENABLE_LOG_INFO
				printLine("No init script!");
				#endif
				bluetoothState = BT_FAIL;
				return(0); /*Give up on initialisation*/
			}
		}
		else
		{
			#ifdef ENABLE_LOG_INFO
			printLine("File sys fail!");
			#endif
			bluetoothState = BT_FAIL;
			return(0); /*Give up on initialisation*/
		}
		return 1;
	}
	void InitScriptReadBytes(unsigned char* dest , unsigned short num)
	{
		// The FSfread will keep track of the offset into the file for me
		FSfread(dest, 1, num, init_script); 			
	}
	void InitScriptClose(void)
	{
		FSfclose(init_script);
	}
#elif defined INIT_SCRIPT_IN_EXT_EEPROM
	#include "Peripherals/Eeprom.h"
	unsigned char InitScriptOpen(void)
	{
		return (I2C_eeprom_present());
	}
	void InitScriptReadBytes(unsigned char* dest , unsigned short num)
	{
		I2C_eeprom_read((INIT_SCRIPT_BASE_ADD+init_script_offset), dest, num);
	}
	void InitScriptClose(void)
	{
	}
#else
	#error "Where is the init script?"
#endif


static int bt_control_cc256x_on(void *config){
	init_script_offset = 0;
	packets_sent = 0;
	return 0;
}

// UART Baud Rate control from: http://e2e.ti.com/support/low_power_rf/f/660/p/134850/484763.aspx
static int cc256x_baudrate_cmd(void * config, uint32_t baudrate, uint8_t *hci_cmd_buffer){
    hci_cmd_buffer[0] = 0x36;
    hci_cmd_buffer[1] = 0xFF;
    hci_cmd_buffer[2] = 0x04;
    hci_cmd_buffer[3] =  baudrate        & 0xff;
    hci_cmd_buffer[4] = (baudrate >>  8) & 0xff;
    hci_cmd_buffer[5] = (baudrate >> 16) & 0xff;
    hci_cmd_buffer[6] = 0;
    return 0;
}

// Output Power control from: http://e2e.ti.com/support/low_power_rf/f/660/p/134853/484767.aspx
#define NUM_POWER_LEVELS 16
#define DB_MIN_LEVEL -35
#define DB_PER_LEVEL 5
#define DB_DYNAMIC_RANGE 30

static int get_max_power_for_modulation_type(int type){
    // a) limit max output power
    int power_db;
    switch (type){
        case 0:     // GFSK
            power_db = 12;
            break;
        default:    // EDRx
            power_db = 10;
            break;
    }
    if (power_db > init_power_in_dB) {
        power_db = init_power_in_dB;
    }
    return power_db;
}

static int get_highest_level_for_given_power(int power_db, int recommended_db){
    int i = NUM_POWER_LEVELS-1;
    while (i) {
        if (power_db < recommended_db) {
            return i;
        }
        power_db -= DB_PER_LEVEL;
        i--;
    }
    return 0;
}

static void update_set_power_vector(uint8_t *hci_cmd_buffer){
    int i;
    int power_db = get_max_power_for_modulation_type(hci_cmd_buffer[3]);
    int dynamic_range = 0;
    // f) don't touch level 0
    for ( i = (NUM_POWER_LEVELS-1) ; i >= 1 ; i--){
        hci_cmd_buffer[4+i] = 2 * power_db;

        if (dynamic_range + DB_PER_LEVEL > DB_DYNAMIC_RANGE) continue;  // e)

        power_db      -= DB_PER_LEVEL;   // d)
        dynamic_range += DB_PER_LEVEL;

        if (power_db > DB_MIN_LEVEL) continue;

        power_db = DB_MIN_LEVEL;    // b) 
    } 
}

static void update_set_class2_single_power(uint8_t * hci_cmd_buffer){
    int i = 0;
    for (i=0;i<3;i++){
        hci_cmd_buffer[3+i] = get_highest_level_for_given_power(get_max_power_for_modulation_type(i), 4);
    }
}

// eHCILL activate from http://e2e.ti.com/support/low_power_rf/f/660/p/134855/484776.aspx
static void update_sleep_mode_configurations(uint8_t * hci_cmd_buffer){
    if (init_ehcill_enabled) {
        hci_cmd_buffer[4] = 1;
    } else {
        hci_cmd_buffer[4] = 0;
    }
}

static void bt_control_cc256x_update_command(uint8_t *hci_cmd_buffer){

    uint16_t opcode = hci_cmd_buffer[0] | (hci_cmd_buffer[1] << 8);

    switch (opcode){
        case 0xFD87:
            update_set_class2_single_power(hci_cmd_buffer);
            break;
        case 0xFD82:
            update_set_power_vector(hci_cmd_buffer);
            break;
        case 0xFD0C:
            update_sleep_mode_configurations(hci_cmd_buffer);
            break;
        default:
            break;
    }
}

static int bt_control_cc256x_next_cmd(void *config, uint8_t *hci_cmd_buffer)
{

	if (init_script_offset == 0) // On first call
	{
		InitScriptOpen();
	}


    if (init_script_offset >= cc256x_init_script_size) 
	{
		InitScriptClose();
		if (packets_sent != INIT_SCRIPT_PACKET_COUNT)
		{
			#ifdef ENABLE_LOG_INFO
			printLine("Currupt script!");
			#endif
			bluetoothState = BT_FAIL;
			return(0);
		}
	    return 0; /*Initialisation over*/
    }
    
	uint8_t payload_len;
	InitScriptReadBytes(&hci_cmd_buffer[0],1); 	// Discarded, packet type always 1
	if(hci_cmd_buffer[0] != 1)
	{
		#ifdef ENABLE_LOG_INFO
		printLine("Currupt script!");
		#endif
		bluetoothState = BT_FAIL;
		return(0);
	}
	init_script_offset++;
	InitScriptReadBytes(&hci_cmd_buffer[0],3);	// Read in, 3rd byte is len
	init_script_offset +=3;
	payload_len = hci_cmd_buffer[2];			// Number of bytes
	InitScriptReadBytes(&hci_cmd_buffer[3],payload_len); // Read in payload
	init_script_offset += payload_len;

    // support for cc256x power commands and ehcill 
    bt_control_cc256x_update_command(hci_cmd_buffer);
	packets_sent++;

	// Indicate a packet is ready to send
    return 1; 
}

// MARK: const structs 

static const bt_control_t bt_control_cc256x = {
	.on = bt_control_cc256x_on,
    .next_cmd = bt_control_cc256x_next_cmd,
    .baudrate_cmd = cc256x_baudrate_cmd
};

static const hci_uart_config_t hci_uart_config_cc256x = {
    .baudrate_init = 57600,
    .baudrate_main = 1000000
};

// MARK: public API

void bt_control_cc256x_enable_ehcill(int on){
    init_ehcill_enabled = on;
}

int bt_control_cc256x_ehcill_enabled(void){
    return init_ehcill_enabled;
}
void bt_control_cc256x_set_power(int16_t power_in_dB){
    init_power_in_dB = power_in_dB;
}

bt_control_t *bt_control_cc256x_instance(void){
    return (bt_control_t*) &bt_control_cc256x;
}

hci_uart_config_t *hci_uart_config_cc256x_instance(void){
    return (hci_uart_config_t*) &hci_uart_config_cc256x;
}
