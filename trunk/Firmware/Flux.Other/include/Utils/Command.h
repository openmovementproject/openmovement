/* 
 * Copyright (c) 2012, Newcastle University, UK.
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

// Command Handler
// Dan Jackson, 2012

#ifndef COMMAND_H
#define COMMAND_H

// Pre-defined to remove circular dependency between the definitions
struct commandParserState_t_struct;

// Command handler type
typedef int (*CommandHandler)(struct commandParserState_t_struct *parserState);

// Command flags
#define COMMAND_DEFINITION_UNPARSED         0x01	// Do not parse
#define COMMAND_DEFINITION_READ_ONLY        0x02    // Value can only be read
#define COMMAND_DEFINITION_PREFIX           0x08    // Match on prefix only (don't require a separator)
#define COMMAND_DEFINITION_CASE_SENSITIVE   0x10	// Case-sensitive match (default is case insensitive)

// Command type
typedef struct command_definition_t_struct
{
    int id;
    const char *label;
    CommandHandler handler;
    void *pointer;
    int flags;
} command_definition_t;

// Command parser context type
typedef struct commandParserState_t_struct
{
    const command_definition_t *command;
    int flags;
    int argc;
    const char **argv;
} commandParserState_t;

// Standard command processors
int CommandHandlerChar(commandParserState_t *cmd);
int CommandHandlerUShort(commandParserState_t *cmd);
int CommandHandlerULong(commandParserState_t *cmd);
int CommandHandlerTime(commandParserState_t *cmd);
int CommandHandlerPrintString(commandParserState_t *cmd);

// Command parsing flags
#define COMMAND_FLAG_OUTPUT_PRIMARY     0x01    // e.g. USB interface
#define COMMAND_FLAG_OUTPUT_ALTERNATIVE 0x02    // e.g. Alternative UART interface
#define COMMAND_MASK_OUTPUT     		0x03	// Mask for output flags
#define COMMAND_FLAG_SCRIPT             0x04    // Command called from a script
#define COMMAND_FLAG_RESTRICTED         0x08    // e.g. Restricted with lock command (e.g. most values read-only)

// Return values
#define COMMAND_RETURN_NOT_HANDLED  0
#define COMMAND_RETURN_OK           1
#define COMMAND_RETURN_PARAMETER    -2
#define COMMAND_RETURN_RESTRICTED   -1

// Parse a command against the specified command list
int CommandParseList(const command_definition_t *commandList, const char *line, int flags);

// Manage an array of lists
#define COMMAND_MAX_LISTS 8
//extern const command_definition_t *commandLists[COMMAND_MAX_LISTS];
void CommandAddList(const command_definition_t *commandList);

// Parse a command against all registered command lists
int CommandParse(const char *line, int flags);

// Run a script file, using all registered command lists
int CommandParseScript(const char *filename);

#endif
