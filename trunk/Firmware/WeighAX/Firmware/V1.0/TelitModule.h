/* 
 * Copyright (c) 2009-2013, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */
 
// Includes
#ifndef TELITMODULE_H
#define TELITMODULE_H

// This generates code required to printf everything going to and from the module
#define GSM_DEBUG_ENABLED

// Set this to the tollerance level required (SIM startup takes 1-5s -> ~3 'sim not ready errors' )
#define MAX_CMS_ERRORS_TO_TOLLERATE		25

// Timing
// You must tell the statre machine how fast it 
// will be called so its timeouts are appropriate.
#define GSM_STATE_MACHINE_RATE_HZ	10UL
#define GSM_1S_TIMEOUT		(GSM_STATE_MACHINE_RATE_HZ)
#define GSM_100mS_TIMEOUT	(GSM_STATE_MACHINE_RATE_HZ/10)
#define MAX_LOCKOUT_THRESHOLD (GSM_1S_TIMEOUT)
// Types

// The enum for the main telit state machine
typedef enum {
// Initialization
INIT_STATE = 0,
RESET_STATE,		// 1
GSM_CHECK_COMMS,	// 2
SET_ECHO_OFF,		// 3
SET_SIM_IO,			// 4
SET_SMS_MODE,		// 5

// Errors
GSM_SUSPECT_RESET,	// +1

// Query commands
QUERY_RSSI,			// +1
QUERY_STATUS,		// +1
QUERY_NET_NAME,		// +1
QUERY_IMSI,			// +1
QUERY_ICCID,		// +1

// Branching condition
DEFAULT_READY,		// +1

// SMS state machine
GSM_SEND_SMS,		// +1
GSM_SEND_SMS_CMD,	// +1
GSM_SEND_SMS_BODY,	// +1
GSM_SEND_SMS_SEND,	// +1
GSM_SEND_SMS_ERROR,	// +1

// Turning off
GSM_SHUT_DOWN,		// +1
GSM_OFF,			// +1

// Others
GSM_MAKE_CALL,		// +1
GSM_CALL_ACTIVE,	// +1		
GSM_END_CALL		// +1

}gsmMachineState_t;


typedef struct {
	unsigned char present;
	unsigned char initialised;
	unsigned char subCriticalError;
	unsigned char criticalError;
	unsigned char cmsErrorCount;
	unsigned char txBusy;
	unsigned char rxWait;
	unsigned char rssi;
	unsigned char regState;
	unsigned char cmgs; // Don't know what this means - its the response to an sms
	char* networkName;
	char* imsi;
	char* iccid;
	char* phoneNumber;
	char* smsText;
	char result;
	gsmMachineState_t machineStateQueue;
	gsmMachineState_t machineState;
	unsigned short txTimeout;
	unsigned short rxTimeout;
	unsigned short lockupDetect;
	unsigned short localLoopCounter;
	unsigned char callStatus; 	// Only valid during call active
	unsigned short cmsError;	// 310 = NO SIM, full list -> http://www.developershome.com/sms/resultCodes2.asp 
}gsm_state_t;
	

// Globals
extern volatile gsm_state_t gsmState ;
extern unsigned char gsm_printf_debug;

// Defines
extern const char* default_phone_number_string;
extern const char* default_sms_text(void);

// Prototypes
void GsmInit(void);
void GsmStateReset(void);
void GsmOff(void);
void GsmStateMachine (void);

// Privates - not for user
unsigned char GsmSendData(const void* source, unsigned short num);
void GsmModuleRxLFHandler(void);
void GsmModuleTxDoneHandler(void);
unsigned char GsmTxBusy(void);
unsigned char GsmRxWait(void);
unsigned char GsmSendCommand(const char* cmd, unsigned short txTime, unsigned char rxWaitCount, unsigned short rxTime );
unsigned char GsmProcessResponse(char ** line);
unsigned char GsmCommInit(void);
#endif
