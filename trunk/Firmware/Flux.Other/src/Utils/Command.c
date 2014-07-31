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

//#define COMMAND_DEBUG

#ifdef _WIN32
//#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#define NO_TIME
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Microchip File System I/O defines
#if defined(__18CXX) || defined(__C30__) || defined( __C32__ ) || defined(__PIC32MX__)
    #include "MDD File System/FSIO.h"
    #include "Utils/FSutils.h"
    #ifdef FILE
        #undef FILE
    #endif
    #ifdef fopen
        #undef fopen
    #endif
    #ifdef fgets
        #undef fgets
    #endif
    #ifdef fclose
        #undef fclose
    #endif
    #define FILE   FSFILE
    #define fopen  FSfopen
    #define fgets  FSfgets
    #define fclose FSfclose
#endif

#ifndef NO_TIME
#include "Peripherals/Rtc.h"
#endif

#include "Utils/Command.h"


// Set/get char
int CommandHandlerChar(commandParserState_t *cmd)
{
    if (cmd->argc >= 2)
    {
        char value;
        if (cmd->command->flags & COMMAND_DEFINITION_READ_ONLY) { return COMMAND_RETURN_RESTRICTED; }
        if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }
        //
        {
	        int ivalue = atoi(cmd->argv[1]);
	        if (ivalue > 127) { ivalue = 0xff; }
	        if (ivalue < -128) { ivalue = 0xff; }
	        value = (char)ivalue;
        }
        value = (char)atoi(cmd->argv[1]);
        *((char *)(cmd->command->pointer)) = value;
    }
    printf("%s=%d\r\n", cmd->command->label, *((char *)(cmd->command->pointer)));
    return COMMAND_RETURN_OK;
}


// Set/get an unsigned short
int CommandHandlerUShort(commandParserState_t *cmd)
{
    if (cmd->argc >= 2)
    {
        unsigned short value;
        if (cmd->command->flags & COMMAND_DEFINITION_READ_ONLY) { return COMMAND_RETURN_RESTRICTED; }
        if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }
        //
        { 
            unsigned long lvalue = (unsigned short)strtoul(cmd->argv[1], NULL, 10);
            if (lvalue > 0xffff) { lvalue = 0xffff; }
            value = (unsigned short)lvalue;
        }
        *((unsigned short *)(cmd->command->pointer)) = value;
    }
    printf("%s=%u\r\n", cmd->command->label, *((unsigned short *)(cmd->command->pointer)));
    return COMMAND_RETURN_OK;
}


// Set/get an unsigned long integer
int CommandHandlerULong(commandParserState_t *cmd)
{
    if (cmd->argc >= 2)
    {
        unsigned long value;
        if (cmd->command->flags & COMMAND_DEFINITION_READ_ONLY) { return COMMAND_RETURN_RESTRICTED; }
        if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }
        value = strtoul(cmd->argv[1], NULL, 10);
        *((unsigned long *)(cmd->command->pointer)) = value;
    }
    printf("%s=%lu\r\n", cmd->command->label, *((unsigned long *)(cmd->command->pointer)));
    return COMMAND_RETURN_OK;
}


#ifndef NO_TIME
// Set/get time
int CommandHandlerTime(commandParserState_t *cmd)
{
    if (cmd->argc >= 2)
    {
	    unsigned long value;
        if (cmd->command->flags & COMMAND_DEFINITION_READ_ONLY) { return COMMAND_RETURN_RESTRICTED; }
        if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }
        value = RtcFromString(cmd->argv[1]);        
        *((unsigned long *)(cmd->command->pointer)) = value;
    }
    printf("%s=%s\r\n", cmd->command->label, RtcToString(*((unsigned long *)(cmd->command->pointer))));
    return COMMAND_RETURN_OK;
}
#endif


// Print a fixed string
int CommandHandlerPrintString(commandParserState_t *cmd)
{
    //if (cmd->argc >= 2) { return COMMAND_RETURN_RESTRICTED; }
    if (cmd->command->pointer != NULL) 
    { 
        printf("%s\r\n", (const char *)(cmd->command->pointer)); 
    }
    return COMMAND_RETURN_OK;
}


/*
unsigned long SetBits(const char *string, unsigned long existing, int bitOffset)
{
    // Left-trim
    while (*string == ' ' || *string == '\t') { string++; }

    // Check for direct binary assignment
    if (*string == '=' || *string == '@')
    {
        int bitpos = 0;

        // Find the end
        while (string[bitpos] == '0' || string[bitpos] == '1' || string[bitpos] == '^' || string[bitpos] == '-' || string[bitpos] == '_') { bitpos++; }

        // Work back to the front, setting bits as appropriate
        for (; bitpos >= 0; bitpos--)
        {
            if (string[bitpos] == '0') { existing &= ~(1ul << bitpos); }
            else if (string[bitpos] == '1') { existing |= (1ul << bitpos); }
            else if (string[bitpos] == '^') { existing ^= (1ul << bitpos); }
            else { ; }  // unchanged
        }
    }
    else
    {
    	const char *p;
        int bitpos = -1;
        int type = 0;

        // Otherwise we're setting bits by index
	    for (p = string; ; p++)
	    {
            // TODO: Add range, e.g. "4-7"
            if (bitpos == -1 && *p == '+') { type = 1; }
            else if (bitpos == -1 && *p == '-') { type = -1; }
		    else if (*p >= '0' && *p <= '9')
		    {
			    if (bitpos < 0) { bitpos = 0; }
			    bitpos = bitpos * 10 + (*p - '0');
		    }
		    else if (bitpos >= 0)
			{
                if (type == -1) { existing &= ~(1ul << bitpos); }
                else if (type == 0 || type == 1) { existing |= (1ul << bitpos); }
                bitpos = -1;
				type = 0;
			}
			if (*p == '\0') { break; }
	    }
    }
}
*/

int CommandParseList(const command_definition_t *commandList, const char *rawLine, int flags)
{
    const char *sourceLine = rawLine;
    char ret = COMMAND_RETURN_NOT_HANDLED;   // "ERROR: Unknown command: %s\r\n"
    int linelen;
    #define MAX_LINE 128
    static char line[MAX_LINE];
    int numParams = 0;
    #define MAX_PARAMS 10
    const char *params[MAX_PARAMS];
    const command_definition_t *cmd = commandList;

    // Ignore NULL or completely empty commands
    if (sourceLine == NULL || sourceLine[0] == '\0') { return 0; }

    // Remove leading white space
    while (*sourceLine != '\0' && (*sourceLine == ' ' || *sourceLine == '\t' || *sourceLine == '\r' || *sourceLine == '\n')) { sourceLine++; }

    // Ignore comment lines (script files)
    if (sourceLine[0] == ';') { return 0; }

    // Ignore empty lines
    linelen = strlen(sourceLine);
    if (linelen == 0) { return 0; }

    // Truncate line length
    if (linelen > MAX_LINE - 1) { linelen = MAX_LINE - 1; }

    // Copy the source line to a mutable buffer for tokenization
    memcpy(line, sourceLine, linelen);
    line[linelen] = '\0';   // ensure NULL-terminated (if truncated)

    // Iterate through the supplied command list
    for (cmd = commandList; cmd != NULL && cmd->label != NULL; cmd++)
    {
        int i;
        char match;
        int cmdlen = strlen(cmd->label);
        char *paramStart;
        char hasParam = 0;  // to make "VALUE=" or "VALUE:" have a single empty parameter

        // Fail to match if command is too small or longer than the line
        if (cmdlen <= 0 || cmdlen > linelen) { continue; }

        // For non-prefix only commands
        if (!(cmd->flags & COMMAND_DEFINITION_PREFIX))
        {
            // Fail to match if the line does not end, or is an alpha-numeric character immediately after the command label
            if (line[cmdlen] != '\0' && ((line[cmdlen] >= 'A' && line[cmdlen] <= 'Z') || (line[cmdlen] >= 'a' && line[cmdlen] <= 'z') || (line[cmdlen] >= '0' && line[cmdlen] <= '9') || line[cmdlen] == '_')) { continue; }
        }

        // Perform case-insensitive match with wildcards
        match = 1;
        for (i = 0; i < cmdlen; i++)
        {
            char c = cmd->label[i];
            char d = line[i];

            if (!(cmd->flags & COMMAND_DEFINITION_CASE_SENSITIVE))
            {
                if (c >= 'a' && c <= 'z') { c = (c - 'a') + 'A'; }      // Normalize case
                if (d >= 'a' && d <= 'z') { d = (d - 'a') + 'A'; }      // Normalize case
            }
            if (c == '*' && ((d >= 'A' && d <= 'Z') || (d >= '0' && d <= '9') || (d == '_'))) { continue; }     // Wildcard
            if (c != d) { match = 0; break; }                       // Mismatch
        }

        // Non-matching
        if (!match) { continue; }
        
        // Find start of parameters
        paramStart = line + cmdlen;
        // Skip one standard name/value separator
        if (*paramStart == ':' || *paramStart == '=' || *paramStart == ',' || *paramStart == ' ' || *paramStart == '\t')
        { 
            if (*paramStart == ':' || *paramStart == '=') { hasParam = 1; }
            line[cmdlen] = '\0';        // Standard separators are replaced with end-of-string (any other non-alpha-numeric characters are left attached)
            paramStart++;
        }

        // Zero all params (called functions can just check against NULL without checking count)
        for (i = 0; i < MAX_PARAMS; i++) { params[i] = NULL; }
        numParams = 1;
        params[0] = line;           // First 'parameter' is the command string (like standard C argc/argv)

        // Check the type of processing we need to do
        if ((cmd->flags & COMMAND_DEFINITION_UNPARSED) != 0)    // No further processing
        {
            // Add the command parameters, if non empty
            if (*paramStart != '\0' || (paramStart != line + cmdlen)) { params[numParams++] = paramStart; }
        }
        else
        {
            char *param;
            char inQuote;
            char *p;

            // Skip whitespace
            while (*paramStart == ' ' || *paramStart == '\t') { paramStart++; }

            // Process parameters
            p = paramStart;
            param = NULL;
            inQuote = 0;
            params[numParams] = NULL;       // Ensure the param after the last one is always NULL (to allow iteration)
            for (p = paramStart; ; p++)
            {
                if (param == NULL)  // Not in a parameter
                {
                    if (*p == '\0') { if (numParams == 1 && hasParam) { params[numParams++] = p; } break; }                                          // End of command line
                    else if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') { ; } // Skip white space
                    else if (*p == '\"') { param = p + 1; inQuote = 1; }                // Start quoted parameter
                    else { param = p; inQuote = 0; }                                    // Start normal parameter
                }
                else                // In parameter
                {
                    // If end of token (end of string, unquote, or out-of-quote token separator)
                    if (*p == '\0' || *p == '\r' || *p == '\n' || (inQuote && *p == '\"') || (!inQuote && (*p == ' ' || *p == ',' || *p == '\t')))
                    {
                        if (numParams < MAX_PARAMS - 1)
                        {
                            params[numParams] = param;
                            params[numParams + 1] = NULL;
                        }
                        param = NULL;
                        numParams++;
                        if (*p == '\0') { break; }                                  // End of command line
                        *p = '\0';
                    }
                    else
                    {
                        if (*p == '\0') { break; }                                  // End of command line
                    }
                }
            }
        }

#ifdef COMMAND_DEBUG
        for (i = 0;i < numParams; i++)
        {
            printf("COMMAND: #%d=\"%s\"\n", i, params[i]);
        }
#endif

        if (cmd->handler == NULL)
        {
            printf("ERROR: Command without handler.");
        }
        else
        {
	        commandParserState_t state;
	        state.command = cmd;
	        state.argc = numParams;
	        state.argv = params;
	        state.flags = flags;
            ret = cmd->handler(&state);
        }

        break;
    }

    return ret;
}


// Array of lists
const command_definition_t *commandLists[COMMAND_MAX_LISTS] = {0};

// Manage an array of lists
void CommandAddList(const command_definition_t *commandList)
{
    int i;
    for (i = 0; i < COMMAND_MAX_LISTS; i++)
    {
        if (commandLists[i] == NULL)
        {
            commandLists[i] = commandList;
            break;
        }
    }
    return;
}


// Parse a command against all registered lists
int CommandParse(const char *line, int flags)
{
    int i;
    int ret = 0;
    for (i = 0; i < COMMAND_MAX_LISTS; i++)
    {
        const command_definition_t *commandList = commandLists[i];
        if (commandList != NULL)
        {
            ret = CommandParseList(commandList, line, flags);
            if (ret != 0) { break; }
        }
    }
    return ret;
}


// Run a script file, using all registered command lists
int CommandParseScript(const char *filename)
{
    FILE *fp;
    static char lineBuffer[64];
    char *line;

    fp = fopen(filename, "r");
    if (fp == NULL) { return 0; }
    for (;;)
    {
        line = fgets(lineBuffer, 64, fp);
        if (line == NULL) { break; }
        CommandParse(line, COMMAND_FLAG_SCRIPT);
    }
    fclose(fp);

    return 1;
}
