/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
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

// Data Logger
// Dan Jackson, K Ladha, 2010-2012


#ifndef LOGGER_H
#define LOGGER_H

// Globals

// Data events

// Privates
// Start the data logger
void RunLogging(void);

// Called from the RTC interrupt at 1Hz
void LoggerTimerTasks(void);

// Called from the timer1 interrupt if enabled
void LoggerRTCTasks(void);

// Read metadata settings from a binary file
char LoggerReadMetadata(const char *filename);

// Write metadata settings to a binary file
char LoggerWriteMetadata(const char *filename);

/*

// Clear the data buffers
void LoggerClear(void);

// Initialise logging
char LoggerInit(const char *filename);

// Start logging
void LoggerStart(void);

// Check each stream for data and write it out to the log file
// 0 = no write, 1 = data written, -1 = write error
char LoggerPollAndWrite(dataStreamFlags_t forceWrite)

// Stop logging
void LoggerStop(void);

*/
#endif
