/* 
 * Copyright (c) 2013-2014, Newcastle University, UK.
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

// Bluetooth physical layer interface
// Karim Ladha, 2013-2014

#ifndef _HCI_UART_H_
#define _HCI_UART_H_

// GLOBALS

// DEFINITIONS
#define HCI_CMD_HEADER		0x01
#define HCI_ACL_HEADER		0x02
#define HCI_EVENT_HEADER 	0x04

// RETURN VALUES
#define HCI_PHY_SUCCESS 		0x0
#define HCI_PHY_BUSY 			0x1
#define HCI_PHY_ERROR 			0x2

// PROTOTYPES
// API to HCI interface
unsigned char HciPhyInitOn(void);
unsigned char HciPhyInitOff(void);
unsigned char HciSupportsLe(void);
unsigned char* HciPhyGetAclBuffer(void);
unsigned char HciPhyWriteAcl(const unsigned char * data, unsigned short len);
unsigned char* HciPhyGetCmdBuffer(void); 
unsigned char HciPhyWriteCmd(const unsigned char * data, unsigned short len); 
// API to allow clock changes and sleep/idle
void		  HciPhyPause(void);
void		  HciPhyPlay(void);	
void 		  HciPhyAdaptClock(void);
void 		  HciPhySafeSleep(void);
// Callback installers for higher levels		
void		  HciInstallAclCB	(void(*CallBack)(const unsigned char * data, unsigned short len)); 	// hci connection level
void		  HciInstallEventCB	(void(*CallBack)(const unsigned char * data, unsigned short len));	// hci configuration level
void 		  HciInstallDataTryCB (unsigned short (*CallBack)(void));								// user data output
void 		  HciInstallTimeOutCB (void  (*CallBack)(void ));										// connection timeouts
#endif
//EOF
