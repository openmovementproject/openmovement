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
#include <Compiler.h>
#include <TimeDelay.h>
#include "HardwareProfile.h"
#include "Utils/Util.h"
// GSM tranceiver
#include "TelitModule.h"
#include "Peripherals/Uart_vdma.h"

// Globals
volatile gsm_state_t gsmState = {0};

unsigned char gsm_printf_debug = FALSE;
#ifdef GSM_DEBUG_ENABLED
	#include "Usb/USB_CDC_MSD.h"
#endif

static char imsi_string[32] = "not read";
static char iccid_string[32] = "not read";

// The module assumes we do
//#define NEED_SIM_IN_IO

// Commands
const char* gsm_cmd_ping = "AT\r\n";
const char* gsm_cmd_echo_off = "ate=0\r\n";
const char* gsm_cmd_set_simin_io = "at#simincfg=8\r\n";
const char* gsm_cmd_set_sms_mode = "at+cmgf=1\r\n";
const char* gsm_cmd_query_signal = "at+csq\r\n";
const char* gsm_cmd_query_status = "at+creg?\r\n";
const char* gsm_cmd_shut_down = "at#shdn\r\n";
const char* gsm_cmd_query_carrier = "at+cops?\r\n";
const char* gsm_cmd_sms_cmd_part1 = "at+cmgs=\"";
const char* gsm_cmd_sms_cmd_part2 = "\"\r\n";
const char* gsm_cmd_sms_cmd_part3 = "\032";		// [dgj] Removed trailing "\r"
const char* gsm_cmd_sms_cmd_cancel = "\x1B\r\n\0";
const char* gsm_cmd_query_imsi = "at+cimi\r\n";
const char* gsm_cmd_query_iccid = "at#ccid\r\n";

const char* gsm_cmd_make_call_part1 = "atd";
const char* gsm_cmd_make_call_part2 = ";\r\n";
const char* gsm_cmd_call_query =  "at+cpas\r\n";
const char* gsm_cmd_end_call = "ath\r\n";

#ifdef __DEBUG
const char* default_phone_number_string  = "07937405006";					// Debug only 
#else
extern char* FormDebugMessage(void);
const char* default_sms_text(void){return ((const char*)FormDebugMessage());}
#endif

// GSM Module stuff
void GsmInit(void)
{
	// Power up the module
	GSM_INIT_ON();
	DelayMs(250); // Allow power up
	// Init the comms - remap must be done elsewhere
	UartVdmaInit();
	// Set the call backs
	UartVdmaSetRxCallBackChar('\n');
	UartVdmaSetRxCallback(GsmModuleRxLFHandler);
	UartVdmaSetTxCallback(GsmModuleTxDoneHandler);
	// Fifo mode
	UartVdmaRxFifoInit();
	// Reset variables
 	GsmStateReset();
}

void GsmStateReset(void)
{
	// Variable init
	gsmState.present = 0;
	gsmState.initialised = 0;
	gsmState.subCriticalError = 0; 	// Not reset to allow error detection
	gsmState.criticalError = 0; 	// Not reset to allow user to detect cycling of the module power
	gsmState.cmsErrorCount = 0;
	gsmState.txBusy = 0;
	gsmState.rxWait = 0;
	gsmState.rssi = 0;
	gsmState.regState = 4;
	gsmState.cmgs = 0;
	gsmState.networkName = NULL;
	gsmState.imsi = NULL;
	gsmState.iccid = NULL;
	gsmState.phoneNumber = NULL;
	gsmState.smsText = NULL;
	gsmState.result = FALSE;
	// State machine init
	gsmState.machineState = INIT_STATE;
	gsmState.machineStateQueue = INIT_STATE;
	gsmState.txTimeout = 0;
	gsmState.rxTimeout = 0;
	gsmState.lockupDetect = 0;
	gsmState.localLoopCounter = 0; /*Used by individual states*/
	gsmState.callStatus = 0;
	gsmState.cmsError = 0;
}

void GsmOff(void)
{
	GsmStateReset();
	GSM_INIT_OFF();
	UartVdmaOff();
	return;
}

void GsmModuleRxLFHandler(void)
{
	if (gsmState.rxWait>0)gsmState.rxWait--;
}

void GsmModuleTxDoneHandler(void)
{
	gsmState.txBusy = 0;
}

unsigned char GsmTxBusy(void)
{
	return gsmState.txBusy;
}

unsigned char GsmRxWait(void)
{
	return gsmState.rxWait;
}

unsigned char GsmSendData(const void* source, unsigned short num)
{
	if (gsmState.txBusy) return FALSE; 	// Currently busy - tx fail
	gsmState.txBusy = TRUE;				// Set busy flag
	UartVdmaSendBlock((const unsigned char*) source, num);
	return TRUE;
}

unsigned char GsmSendCommand(const char* cmd, unsigned short txTime, unsigned char rxWaitCount, unsigned short rxTime )
{
	#ifdef GSM_DEBUG_ENABLED
	if(gsm_printf_debug){
		printf("%s",cmd);
		USBCDCWait();
	}
	#endif

	if (gsmState.txBusy) return FALSE; 	// Currently busy - tx fail
	gsmState.rxWait = rxWaitCount;		// Responce expected, set wait flag count
	gsmState.rxTimeout = rxTime;		// Time to wait for rx
	gsmState.txBusy = TRUE;				// Set busy flag
	gsmState.txTimeout = txTime;		// Time to transmit
	UartVdmaSendBlock((const unsigned char*) cmd, strlen(cmd));
	return TRUE;
}	

// Call after the reciever indicates it has received some kind of response to reduce errors. 
// Setting doWaitRx in the 'GsmSendCommand()' function will cause it to hang while it waits 
// for doWaitRx number of \n's 
// For standard responses just call it with NULL - it will update variables such as rssi and:
// For all OK terminated responces it returns TRUE.
// For all others it returns FALSE.
// For manual handling send it a pointer, it will set this pointer to its line buffer and NULL
// terminate the resulting string. The function will return TRUE each time extra chars are received. 
unsigned char GsmProcessResponse(char ** resp)
{
	#define LINE_BUFFER_SIZE 256
	static char lineBuffer[LINE_BUFFER_SIZE];
	static short index = 0; 	// WARNING: static
	char *ptr = lineBuffer;
	char retVal = FALSE;
 	short len; 

	if (resp == NULL)
	{
		index = 0; // Do not append new inChars to line
	}

	// Get current rx'ed length - this is how many new chars we have
	len = UartVdmaSafeFifoLength();

	// Dont over run the buffer check
	if(len+index > (LINE_BUFFER_SIZE-1)) {
		// This is a fail, we have recieved a big response we were not expecting
		// to handle large responces >256 chars you must make another handler 
		// or increase LINE_BUFFER_SIZE
		index = 0;
		Nop(); 
	}

	// Copy it to the line buffer
	UartVdmaSafeFifoPop(&lineBuffer[index], len);
	// Increment the index - above check will prevent overruns
	index+=len; 

#ifdef GSM_DEBUG_ENABLED
	if (gsm_printf_debug == TRUE)
		{usb_write(&lineBuffer[index-len], len);}
#endif

	// For manual responces just append received chars, null terminate and return if /cr/lf found
	// You can keep calling this function in a tight loop, the return pointer will be the same each time
	// and the new chars with a null will be added as they come in. Call with null to erase the buffer
	if (resp != NULL) 
	{
		lineBuffer[index] = '\0';
		*resp = lineBuffer;
		return (len>0)?TRUE:FALSE; // Return true means one or more chars have been appended
	}
	
	// For normal opperation look for an '\r\nOK\r\n' at the end of the received buffer 
	if(strncmp("\r\nOK\r\n",&lineBuffer[index-6],6)==0)
	{	
		retVal = TRUE; 					// Response with \r\nOK\r\n found - continue
		if (index <= 9) return  retVal;	// No other parts to response - just return
	}
	else 
	{
		if(strncmp("\r\n",&lineBuffer[index-2],2)==0) // Does it have a CRLF at the end
		{
			retVal = FALSE;
			// But process the line anyway incase it is an error
		}
		else
		{
			index = 0; 						// Response does not end in \r\nOK\r\n or \r\n
			return FALSE;
		}
	}		


	// \r\nOK\r\n received - now process rest of response

while(1){
	// Go to the first real char - ptr points to start of line
	// Note special case for '>' character
	while( (*ptr<=' ' || *ptr>'~' || *ptr=='>') && len>0 ) {ptr++,len--;} // Removes the  "\r\n" from the start
	if (len<=0) break; 					// No real chars in buffer - just return

	// Next remove any leading OK\r\n's from the line
	if (strncmp("OK\r\n",ptr,4)==0) 
	{
		// Carry on parsing
		retVal = TRUE;
		ptr += 4;len-=4;
		if (len<=0) break;
		continue; // May be other parts to response
	}

	// Next check for any errors
	if (strncmp("ERROR\r\n",ptr,4)==0) 
	{
		// Carry on parsing
		retVal = -1; // ERROR case is -1
		ptr += 7;len-=7;
		if (len<=0) break;
		continue; // May be other parts to response
	}

	// Next check for any other errors
	if (strncmp("NO CARRIER\r\n",ptr,10)==0) 
	{
		// Carry on parsing
		retVal = -1; // ERROR case is -1
		ptr += 7;len-=7;
		if (len<=0) break;
		continue; // May be other parts to response
	}

	// Registered status
	if (strncmp("+CREG:",ptr,6)==0) 
	{
		//Parse response state
		retVal = TRUE;
		while((*ptr++)!=',');
		gsmState.regState = my_atoi(ptr);
		break; // We know these are only followed by \r\nOK\r\n
	}

	// RSSI figure
	if (strncmp("+CSQ:",ptr,5)==0) 
	{
		// Parse response RSSI
		retVal = TRUE;
		while((*ptr++)!=':');
		gsmState.rssi = my_atoi(ptr);
		break; // We know these are only followed by \r\nOK\r\n
	}

	// System errors
	if (strncmp("+CMS ERROR:",ptr,11)==0)
	{
		// Parse response RSSI
		retVal = FALSE;
		while((*ptr++)!=':');
		gsmState.cmsError = my_atoi(ptr);

#ifdef GSM_DEBUG_ENABLED
		if (gsm_printf_debug == TRUE)
		{
			if(gsmState.cmsError == 310)
			{
				// ONLY for sim error, allow a bit more time for card to respond (uses cms error timout)
				gsmState.lockupDetect = 0;
				printf("NO SIM\r\n");
			}
		}
#endif
		// KL: SIM ERROR 310 may occur a number of times until the SIM is ready!!
		if (gsmState.cmsErrorCount++ > MAX_CMS_ERRORS_TO_TOLLERATE)
			gsmState.machineState = GSM_SHUT_DOWN;

		break; // We know these are only followed by \r\nOK\r\n
	}

	// Call status figures - only valid if call active
	// 0 - ready, 
	// 1 - unavailable 
	// 2 - unknown
	// 3 - ringing (seems to be 4 or zero only, never 3)
	// 4 - call in progress
	if (strncmp("+CPAS:",ptr,5)==0) 
	{
		// Parse response RSSI
		retVal = TRUE;
		while((*ptr++)!=':');
		gsmState.callStatus = my_atoi(ptr);
		break; // We know these are only followed by \r\nOK\r\n
	}
	
	// Network name
	if (strncmp("+COPS:",ptr,5)==0) 
	{
		// Parse response RSSI
		static char networkName[32] = {0};
		unsigned char i;
		retVal = TRUE;
		while((*ptr++)!='"');
		for(i=0;i<32;)
		{
			if (*ptr=='"'){networkName[i] = '\0';break;}
			networkName[i++] = *ptr++;
		}
		if(i>1)gsmState.networkName = networkName;
		break; // We know these are only followed by \r\nOK\r\n
	}

	// Last MSG status
	if (strncmp("+CMGS:",ptr,5)==0) 
	{
		// Parse response to sms
		//retVal = TRUE;
		while((*ptr++)!=':');
		gsmState.cmgs = my_atoi(ptr);
		break; // We know these are only followed by \r\nOK\r\n
	}

	// ICCID
	if (strncmp("#CCID",ptr,5)==0) 
	{
		// Parse response to iccid
		retVal = TRUE;
		char* ptr2 = iccid_string;
		ptr+=5;
		while(*ptr==':' || *ptr==' ')ptr++;
		for(;*ptr>='0' && *ptr<='9';ptr++,ptr2++){*ptr2 = *ptr;}
		*ptr2 = '\0';
		break; // We know these are only followed by \r\nOK\r\n
	}

	break;
}

	index = 0; 		// Reset the index for the next response
	return retVal; 	// Return true to indicate a reponse was encountered
}

void GsmStateMachine (void)
{
static unsigned char waitResponse;

// State machine will wait for each transmit here
if (GsmTxBusy()) 
{
	// The tx being blocked could only normally be cause my the module locking up
	if(gsmState.txTimeout-- <= 0)
	{ 
		gsmState.criticalError++;
		gsmState.txBusy = 0;				// We have timed out and are not waiting
		// Critical errors usually mean the module needs re-setting or we are calling the tasks too fast
Nop(); 	// Debug - ensure we dont hit this!
		gsmState.machineState = INIT_STATE; // Will power cycle the module
	}
	return; 
}
// State machine waits for receive here
if (GsmRxWait()) // If we are waiting for the response
{
	// The module has taken too long to respond, 
	if(gsmState.rxTimeout-- <= 0) 
	{
		gsmState.rxWait = 0;				// We have timed out and are not waiting
		gsmState.rxTimeout = 0;				// Prevent it overflowing
		gsmState.subCriticalError++;		// This error count represents total timeouts
		gsmState.result = FALSE;			// Incase we are waiting externally for a result
		if (gsmState.initialised)
			{gsmState.machineState = GSM_SUSPECT_RESET;
			waitResponse = FALSE;}
		else gsmState.machineState = RESET_STATE; // Try re-initing the module
	}
	// Otherwise return - we are still waiting
	else return;
}
// State machine checks for lockups here
if (gsmState.lockupDetect > MAX_LOCKOUT_THRESHOLD)
{
		// Debug
		#ifdef GSM_DEBUG_ENABLED
		if (gsm_printf_debug == TRUE){
			printf("Gsm lockup detected.\r\n");
			USBCDCWait();
		}
		#endif

		gsmState.lockupDetect = 0;
		gsmState.subCriticalError++;		// This error count represents total timeouts
		gsmState.rxWait = 0;				// A response was not detected in the alotted time, reset wait var 
		if (gsmState.initialised)
			{gsmState.machineState = GSM_SUSPECT_RESET;
			waitResponse = FALSE;}
		else gsmState.machineState = RESET_STATE; // Try re-initing the module
}

// Main state machine 
switch (gsmState.machineState) {

	// Use this buffer to make longer commands to send
	static char concatenated[32] = {0}; // Not safe to stack it

	// First state - device is reset
	case (INIT_STATE)	:	{
					GsmInit();	// Setup device, advance to next state
					waitResponse = FALSE;
					gsmState.machineState = RESET_STATE;
					break;
				}

	// Powered but no comms yet, state machine initialised
	case (RESET_STATE)	:	{
					// Reset variables
 					GsmStateReset();
					waitResponse = FALSE;
					gsmState.machineState = GSM_CHECK_COMMS;
					break;
				}

	// Initialisation stages
	case (GSM_CHECK_COMMS) : 	{
					if (waitResponse == FALSE)
					{
						if (GsmSendCommand(gsm_cmd_ping,GSM_100mS_TIMEOUT,2,GSM_100mS_TIMEOUT)) // Note: 2 or 3 CR depending on echo
							{waitResponse = TRUE;}
					}
					else
					{
						if (GsmProcessResponse(NULL))
							{waitResponse = FALSE;
							gsmState.present = TRUE;	// Indicate device responded
							gsmState.machineState = SET_ECHO_OFF;}
						else 
							{waitResponse = FALSE; // This will retry the same command
							gsmState.lockupDetect++;}
					}
					break;			
				}
	case (SET_ECHO_OFF) : 	{
					if (waitResponse == FALSE)
					{
						if (GsmSendCommand(gsm_cmd_echo_off,GSM_100mS_TIMEOUT,2,GSM_100mS_TIMEOUT))
							{waitResponse = TRUE;}
					}
					else
					{
						if (GsmProcessResponse(NULL))
							{waitResponse = FALSE;
							#ifdef NEED_SIM_IN_IO
								gsmState.machineState = SET_SIM_IO;}
							#else
								gsmState.machineState = SET_SMS_MODE;}
							#endif
						else 
							{waitResponse = FALSE; // This will retry the same command
							gsmState.lockupDetect++;}
					}
					break;			
				}
#ifdef NEED_SIM_IN_IO
	case (SET_SIM_IO) : 	{
					if (waitResponse == FALSE)
					{
						if (GsmSendCommand(gsm_cmd_set_simin_io,GSM_100mS_TIMEOUT,2,GSM_100mS_TIMEOUT))
							{waitResponse = TRUE;}
					}
					else
					{
						if (GsmProcessResponse(NULL))
							{waitResponse = FALSE;
							gsmState.machineState = SET_SMS_MODE;}
						else 
							{waitResponse = FALSE; // This will retry the same command
							gsmState.lockupDetect++;}
					}
					break;			
				}
#endif
	case (SET_SMS_MODE) : 	{
					if (waitResponse == FALSE)
					{
						if (GsmSendCommand(gsm_cmd_set_sms_mode,GSM_100mS_TIMEOUT,2,GSM_100mS_TIMEOUT))
							{waitResponse = TRUE;}
					}
					else
					{
						if (GsmProcessResponse(NULL))
							{gsmState.initialised = TRUE;
							waitResponse = FALSE;
							gsmState.machineState = DEFAULT_READY;}
						else 
							{waitResponse = FALSE; // This will retry the same command
							gsmState.lockupDetect++;}
					}
					break;			
				}

	// The module occasionally internally resets - possible hardware issue? Detected by echo being reset
	case (GSM_SUSPECT_RESET) : 	{
					if (waitResponse == FALSE)
					{
						#ifdef GSM_DEBUG_ENABLED
						if(gsm_printf_debug){
							printf("Gsm checking module ok.");
							USBCDCWait();
						}
						#endif
						GsmProcessResponse(NULL); // Reset reciever
						if (GsmSendCommand(gsm_cmd_ping,GSM_100mS_TIMEOUT,2,GSM_100mS_TIMEOUT))
							{waitResponse = TRUE;}
					}
					else
					{
						char* resp;
						if (GsmProcessResponse(&resp))
						{
							if (strncmp("\r\nOK\r\n",resp,6)!=0)  		// See if it echos back the ping, not just 'ok'
								{gsmState.machineState = RESET_STATE;}	// Yes, then reset states
							else 
								{waitResponse = FALSE;
								gsmState.machineState = DEFAULT_READY;}
						}
						else 
							{waitResponse = FALSE; // This will retry the same command
							gsmState.lockupDetect++;}
							
					}
					break;			
				}

	// Switch state, point where state can branch
	case (DEFAULT_READY) :	{
					// If no queued commands are present then query module variables	
					if (gsmState.machineStateQueue != INIT_STATE)
					{
						gsmState.machineState = gsmState.machineStateQueue;
						gsmState.machineStateQueue = INIT_STATE;
						waitResponse = FALSE;
						break;
					}
					else
					{
						gsmState.rxTimeout = 1; // In the case of a timeout we must discard 
						gsmState.txTimeout = 1; // any pending messages and assume the module
						gsmState.rxWait = 0;	// is never going to respond.
						gsmState.machineState = QUERY_STATUS;
						waitResponse = FALSE;
						break;
					}
				}	

	// Query module stages
	case (QUERY_STATUS) :	{
					if (waitResponse == FALSE)
					{
						if (GsmSendCommand(gsm_cmd_query_status,GSM_100mS_TIMEOUT,4,GSM_100mS_TIMEOUT))
							{waitResponse = TRUE;}
					}
					else
					{
						if (GsmProcessResponse(NULL))
						{	// We should not read rssi until it is connected
							if (gsmState.regState == 1 || gsmState.regState == 5) 
								{waitResponse = FALSE;
								gsmState.machineState = QUERY_RSSI;}
							else 
								{waitResponse = FALSE;
								gsmState.machineState = DEFAULT_READY;}
						}
						else 
							{waitResponse = FALSE; // This will retry the same command
							gsmState.lockupDetect++;}
					}
					break;			
				}
	case (QUERY_RSSI) :	{
					if (waitResponse == FALSE)
					{
						if (GsmSendCommand(gsm_cmd_query_signal,GSM_100mS_TIMEOUT,4,GSM_100mS_TIMEOUT))
							{waitResponse = TRUE;}
					}
					else
					{
						if (GsmProcessResponse(NULL))
						{	
							// After reading the RSSI - do each of these parameter reads ONCE
							if (gsmState.regState == 1 || gsmState.regState == 5) 
							{
								waitResponse = FALSE;
								if(gsmState.networkName == NULL) // First thing to read
								{
									gsmState.machineState = QUERY_NET_NAME;
								}
								else if (gsmState.imsi == NULL)
								{
									gsmState.machineState = QUERY_IMSI;
								}
								else if (gsmState.iccid == NULL)
								{
									gsmState.machineState = QUERY_ICCID;
								}
								else 							// All parameters read
								{
									gsmState.machineState = DEFAULT_READY;
								}
							}
						}
						else 
							{waitResponse = FALSE; // This will retry the same command
							gsmState.lockupDetect++;}
					}
					break;			
				}
	case (QUERY_NET_NAME) :	{
					if (waitResponse == FALSE)
					{
						if (GsmSendCommand(gsm_cmd_query_carrier,GSM_100mS_TIMEOUT,4,GSM_100mS_TIMEOUT))
							{waitResponse = TRUE;}
					}
					else
					{
						if (GsmProcessResponse(NULL))
						{
							if (gsmState.imsi == NULL && (gsmState.regState == 1 || gsmState.regState == 5)) 
								{waitResponse = FALSE;
								gsmState.machineState = QUERY_IMSI;}
							else 
								{waitResponse = FALSE;
								gsmState.machineState = DEFAULT_READY;}
						}
						else 
							{waitResponse = FALSE; // This will retry the same command
							gsmState.lockupDetect++;}
					}
					break;			
				}

	case (QUERY_IMSI) :{
					if (waitResponse == FALSE)
					{
						GsmProcessResponse(NULL); // Reset reciever
						if (GsmSendCommand(gsm_cmd_query_imsi,GSM_100mS_TIMEOUT,2,GSM_100mS_TIMEOUT))
							{waitResponse = TRUE;}
					}
					else
					{
						char *ptr,*ptr2;
						if (GsmProcessResponse(&ptr)){
							if(ptr){ // If we got a response then parse it and cpy to global string
								while((*ptr<'0' || *ptr>'9')&&*ptr!='\0')ptr++;
								ptr2 = imsi_string;
								while (*ptr!='\r'){*ptr2++ = *ptr++;}
								*ptr2='\0';
							} 
							gsmState.imsi = imsi_string;
							waitResponse = FALSE;
							gsmState.machineState = QUERY_ICCID;}
						else 
							{waitResponse = FALSE; // This will retry the same command
							gsmState.lockupDetect++;}
					}
					break;			
				}

	case (QUERY_ICCID) :{
					if (waitResponse == FALSE)
					{
						GsmProcessResponse(NULL); // Reset reciever
						if (GsmSendCommand(gsm_cmd_query_iccid,GSM_100mS_TIMEOUT,2,GSM_100mS_TIMEOUT))
							{waitResponse = TRUE;}
					}
					else
					{
						if (GsmProcessResponse(NULL)){
							gsmState.iccid = iccid_string;
							waitResponse = FALSE;
							gsmState.machineState = DEFAULT_READY;}
						else 
							{waitResponse = FALSE; // This will retry the same command
							gsmState.lockupDetect++;}
					}
					break;			
				}

	// Send SMS stages
	case (GSM_SEND_SMS) :{
					if (waitResponse == FALSE)
					{
						// Set the string ptr before triggering this state to avoid default being used
						if ((gsmState.phoneNumber == NULL) ||
							(gsmState.smsText == NULL))
						{
							gsmState.result = FALSE;
							waitResponse = FALSE;
							gsmState.machineState = DEFAULT_READY;
							break;
						}
						// Start SMS send routine
						strcpy(concatenated,gsm_cmd_sms_cmd_part1); 	// 9 Chars
						strcat(concatenated,gsmState.phoneNumber);		// upto 20 numbers
						strcat(concatenated,gsm_cmd_sms_cmd_part2);		// 2 Chars
						GsmProcessResponse(NULL); // Reset reciever
						if (GsmSendCommand(concatenated,GSM_100mS_TIMEOUT,0,5*GSM_1S_TIMEOUT)) // Wait for > prompt
						{
							waitResponse = TRUE;
							gsmState.lockupDetect = 0; // Give us a chance to receive "> " prompt
							gsmState.localLoopCounter = (5 * GSM_1S_TIMEOUT); // Overide timeout below for extra time
						}
					}
					else
					{
						//Parse response, we are expecting a "> "
						char* resp = NULL;
						
						if (GsmProcessResponse(&resp))		// If something was received
						{	
							// If the string is "\r\n> ", the module seems temperamental - using broad check
							if (resp[0] == '>' || resp[1] == '>' || resp[2] == '>' || resp[3] == '>')					
								{gsmState.machineState = GSM_SEND_SMS_BODY;}	
							else if (strncmp("\r\nERROR\r\n",resp,9)==0)		// If the string is an error		// [dgj] changed 13 to 9
								{gsmState.machineState = GSM_SEND_SMS_ERROR;}
							else  if (strlen(resp)>4)							// If the string is longer than expected
								{gsmState.machineState = GSM_SEND_SMS_ERROR;}
							else gsmState.lockupDetect++;
						}
						else 
						{
							if(gsmState.localLoopCounter > 0)
							{
								gsmState.localLoopCounter--;
							}
							else
							{
								gsmState.lockupDetect++;
							}
						}
					}
					break;			
				}

	case (GSM_SEND_SMS_BODY) :{
						GsmSendCommand(gsmState.smsText,GSM_100mS_TIMEOUT,0,0);
						waitResponse = FALSE;
						gsmState.machineState = GSM_SEND_SMS_SEND;
						break;			
				}
	case (GSM_SEND_SMS_SEND) :{
					if (waitResponse == FALSE)
					{
						if (GsmSendCommand(gsm_cmd_sms_cmd_part3,GSM_100mS_TIMEOUT,4,15*GSM_1S_TIMEOUT)) // Long timeout, 15s
							{waitResponse = TRUE;}
					}
					else
					{
						if (GsmProcessResponse(NULL))
						{
							// Can clear the wait counter since it worked
							gsmState.lockupDetect = 0;
							gsmState.result = TRUE;
							waitResponse = FALSE;
							gsmState.machineState = DEFAULT_READY;
						}
						else 
						{
							gsmState.lockupDetect++;
							gsmState.result = FALSE;
							#ifdef GSM_DEBUG_ENABLED
							if (gsm_printf_debug == TRUE){
								printf("Gsm SMS failed resp:%u\r\n",gsmState.cmsError);
								USBCDCWait();
							}
							#endif
							waitResponse = FALSE;
							gsmState.machineState = DEFAULT_READY;
						}
					}
					break;			
				}
	case (GSM_SEND_SMS_ERROR) :{						
					// Send cancel string to exit the SMS entry mode
					// Will stick here
					#ifdef GSM_DEBUG_ENABLED
					if (gsm_printf_debug == TRUE){
						printf("Gsm SMS fail detected.\r\n");
						USBCDCWait();
					}
					#endif
					GsmSendCommand(gsm_cmd_sms_cmd_cancel,GSM_100mS_TIMEOUT,0,0);
					gsmState.result = FALSE;
					gsmState.subCriticalError++; // The SMS failed
					gsmState.machineState = DEFAULT_READY;
					break;
				}

	case (GSM_MAKE_CALL) :{	
					if (gsmState.phoneNumber != NULL)
					{
						if (waitResponse == FALSE)
						{
							// Start call
							strcpy(concatenated, gsm_cmd_make_call_part1); 	// 9 Chars
							strcat(concatenated,gsmState.phoneNumber);		// upto 20 numbers
							strcat(concatenated, gsm_cmd_make_call_part2);	// 2 Chars
							if (GsmSendCommand(concatenated,GSM_100mS_TIMEOUT,2,(10 * GSM_1S_TIMEOUT))) // No carriage returns in resp
								{waitResponse = TRUE;}	
						}
						else		
						{
							char resp = GsmProcessResponse(NULL);
							if (resp == TRUE)			// OK was received
								{waitResponse = FALSE;
								gsmState.machineState = GSM_CALL_ACTIVE;} 
							else if (resp == -1)  // Call fail or timeout
								{gsmState.machineState = DEFAULT_READY;
								waitResponse = FALSE;}
						}
					}
					else gsmState.machineState = DEFAULT_READY; // Failed to define a number to call		
					break;
				}

	case (GSM_CALL_ACTIVE) :{						
					// Read call status reg
					if (waitResponse == FALSE)
						{GsmSendCommand(gsm_cmd_call_query ,GSM_100mS_TIMEOUT,4,5*GSM_100mS_TIMEOUT);
						waitResponse = TRUE;}
					else 
					{
							char resp = GsmProcessResponse(NULL);
							if (resp == -1)
								{gsmState.machineState = DEFAULT_READY; // Call fail / ended (reached on third party hang up too)
								waitResponse = FALSE;}	
							if (resp >= 0)
								{waitResponse = FALSE;}					// Repeat calls to query		
					}
					// Special case - requires machine to lookout for call end command
					if (gsmState.machineStateQueue == GSM_END_CALL)
					{
						gsmState.machineState = GSM_END_CALL;
						gsmState.machineStateQueue = INIT_STATE;
						waitResponse = FALSE;
					}
					break;
				}
	case (GSM_END_CALL) :{	
					static unsigned short localTimeout = 5 * GSM_1S_TIMEOUT; // Max hang up time allowed					
					if (waitResponse == FALSE)
					{
						// End call
						if (GsmSendCommand(gsm_cmd_end_call,GSM_100mS_TIMEOUT,2,GSM_100mS_TIMEOUT))
							{waitResponse = TRUE;}	
					}
					else		
					{
						char resp = GsmProcessResponse(NULL);
						if (resp == TRUE)		// an OK was received
							{waitResponse = FALSE;						
							gsmState.machineState = DEFAULT_READY;}
						else if ((resp == -1) || (!(localTimeout--)) ) // Call fail or timeout
							{gsmState.machineState = DEFAULT_READY;
							waitResponse = FALSE;}
					}
					break;
				}

	case (GSM_SHUT_DOWN) :{						
					// Disconnect from radio - should always respond successfully
					if (waitResponse == FALSE)
					{
						if (GsmSendCommand(gsm_cmd_shut_down,GSM_100mS_TIMEOUT,2,10*GSM_1S_TIMEOUT)) // Long timeout, 10s
							{waitResponse = TRUE;}
					}
					else
					{
						if (GsmProcessResponse(NULL))
						{
							gsmState.result = TRUE;
							waitResponse = FALSE;
							gsmState.machineState = GSM_OFF;
						}
						else 
						{
							gsmState.lockupDetect++;
							gsmState.result = FALSE; // Module failed to respond in allotted time
							waitResponse = FALSE;
							gsmState.machineState = GSM_OFF;
						}
					}
					break;	
				}
	case (GSM_OFF)	:{
					// Module has been powered down - turn off radio and stop executing run loop
					if (waitResponse == FALSE)
					{
						GsmOff();
						gsmState.machineState = GSM_OFF;
						waitResponse = TRUE;
					}
					else
					{
						// Will stick here
						#ifdef GSM_DEBUG_ENABLED
						if (gsm_printf_debug == TRUE){
							printf("Gsm state machine called but module is off.\r\n");
							USBCDCWait();
						}
						#endif
					}
					break;		
				}			
	
	default : 	break;	
}
return;
}

