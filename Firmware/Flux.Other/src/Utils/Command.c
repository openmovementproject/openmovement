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

#include "Peripherals/SysTime.h"
#include "Utils/Command.h"
#include "Utils/filesystem.h"


// Compares two DateTime_t structures for equality:  (<0): a<b,  (>0): a>b,  (=0): a==b
int SysTimeEquals(const DateTime_t *a, const DateTime_t *b)
{
	const char *pa = (const char *)a;
	const char *pb = (const char *)b;
	int ofs;
	for (ofs = 5; ofs >= 0; ofs--)
	{
		if (pa[ofs] > pb[ofs]) { return 1; }
		if (pa[ofs] < pb[ofs]) { return -1; }
	}
	return 0;
}

// Minimum valid time
const DateTime_t sysTimeMin = {
	.seconds = 0,
	.minutes = 0,
	.hours = 0,
	.day = 1,
	.month = 1,
	.year = 0,
};

// Maximum valid time (when packed)
const DateTime_t sysTimeMax = {
	.seconds = 59,
	.minutes = 59,
	.hours = 23,
	.day = 31,
	.month = 12,
	.year = 63,
};

// Returns whether a datetime is (=0) at-or-below the minimum, (<0) at-or-below the maximum, or (>0) a standard time.
int SysTimeMinMax(const DateTime_t *tm)
{
	int compMin = SysTimeEquals(tm, &sysTimeMin);
	int compMax = SysTimeEquals(tm, &sysTimeMax);

	if (compMin == -1 || compMin == 0) { return 0; }	// at-or-below minimum
	if (compMax == 1 || compMax == 0) { return -1; }	// at-or-below maximum
	return 1;	// a standard time
}

// Convert a date/time number to a string ("yyYY/MM/DD,HH:MM:SS+00" -- AT+CCLK compatible for default format)
const char *SysTimeToString(const DateTime_t *tm)
{
    // "yyYY/MM/DD,HH:MM:SS+00"
	static char rtcString[23];
    char *c = rtcString;
    unsigned int v;
	int minMax = SysTimeMinMax(tm);

	if (minMax == 0) { *c++ = '0'; *c++ = '\0'; }				// "0"
	else if (minMax < 0) { *c++ = '-'; *c++ = '1'; *c++ = '\0'; }	// "-1"
	else
	{
	    v = 2000 + tm->year; *c++ = '0' + ((v / 1000) % 10); *c++ = '0' + ((v / 100) % 10); *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '/';
	    v = tm->month;       *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = '/';
	    v = tm->day;         *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ',';
	    v = tm->hours;       *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
	    v = tm->minutes;     *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10); *c++ = ':';
	    v = tm->seconds;     *c++ = '0' + ((v / 10) % 10); *c++ = '0' + (v % 10);
	    //*c++ = '+'; *c++ = '0'; *c++ = '0';
	    *c++ = '\0';
	}
    return rtcString;
}

// Convert a date/time number from a string ("YY/MM/DD,HH:MM:SS+00" -- AT+CCLK compatible for default format)
// Returns >0 for a valid time stored to the structure (<0 for infinitely future, 0 for infinitely past)
int SysTimeFromString(const char *value, DateTime_t *tm)
{
    unsigned short v = 0xffff;
    int index = 0;
    const char *c = value;

	if (tm == NULL || value == NULL) { return 0; }
	if (value[0] == '\0') { return 0; }
	if (value[0] == '0' && value[1] == '\0') { return 0; }
	if (value[0] == '-') { return -1; }
    for (;;)
    {
        if (*c >= '0' && *c <= '9') // Part of value
        {
            if (v == 0xffff) { v = 0; }
            v = (v * 10) + (*c - '0');
        }
        else
        {
            if (v != 0xffff)  // End of value
            {
                if      (index == 0) { tm->year = (unsigned char)((v >= 2000) ? (v - 2000) : v); }
                else if (index == 1) { tm->month = (unsigned char)v; }
                else if (index == 2) { tm->day = (unsigned char)v; }
                else if (index == 3) { tm->hours = (unsigned char)v; }
                else if (index == 4) { tm->minutes = (unsigned char)v; }
                else if (index == 5) { tm->seconds = (unsigned char)v; }
                else { break; }
                index++;
                v = 0xffff;
                if (index >= 6) { break; }
            }
            if (*c == '\0') { break; }
        }
        c++;
    }

    // Check if parsed six elements and check validity of date
	if (index != 6 || !SysTimeCheckTime(tm))
	{
        return 0;		// invalid
	}
	return 1;		// valid time
}

unsigned long PackedTimeFromString(const char *value)
{
	DateTime_t tm;
	int timeResult = SysTimeFromString(value, &tm);
	if (timeResult < 0) { return 0xFFFFFFFFul; }
	if (timeResult == 0) { return 0; }
	return SysTimeToPacked(&tm);
}

unsigned long EpochTimeFromString(const char *value)
{
	DateTime_t tm;
	int timeResult = SysTimeFromString(value, &tm);
	if (timeResult < 0) { return 0xFFFFFFFFul; }
	if (timeResult == 0) { return 0; }
	return SysTimeToEpoch(&tm);
}

const char *PackedTimeToString(unsigned long value)
{
	DateTime_t tm;
	return SysTimeToString(SysTimeFromPacked(value, &tm));
}

const char *EpochTimeToString(unsigned long value)
{
	DateTime_t tm;
	return SysTimeToString(SysTimeFromEpoch(value, &tm));
}



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


// Set/get packed time
int CommandHandlerTime(commandParserState_t *cmd)
{
    if (cmd->argc >= 2)
    {
	    unsigned long value;
        if (cmd->command->flags & COMMAND_DEFINITION_READ_ONLY) { return COMMAND_RETURN_RESTRICTED; }
        if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }
        value = PackedTimeFromString(cmd->argv[1]);        
        *((unsigned long *)(cmd->command->pointer)) = value;
    }
    printf("%s=%s\r\n", cmd->command->label, PackedTimeToString(*((unsigned long *)(cmd->command->pointer))));
    return COMMAND_RETURN_OK;
}


// Set/get epoch-based time
int CommandHandlerTimeEpoch(commandParserState_t *cmd)
{
    if (cmd->argc >= 2)
    {
	    unsigned long value;
        if (cmd->command->flags & COMMAND_DEFINITION_READ_ONLY) { return COMMAND_RETURN_RESTRICTED; }
        if (cmd->flags & COMMAND_FLAG_RESTRICTED) { return COMMAND_RETURN_RESTRICTED; }
        value = EpochTimeFromString(cmd->argv[1]);        
        *((unsigned long *)(cmd->command->pointer)) = value;
    }
    printf("%s=%s\r\n", cmd->command->label, EpochTimeToString(*((unsigned long *)(cmd->command->pointer))));
    return COMMAND_RETURN_OK;
}


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

int CommandParseListContext(const command_definition_t *commandList, const char *rawLine, int flags, void *context)
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
			state.context = context;
            ret = cmd->handler(&state);
        }

        break;
    }

    return ret;
}

int CommandParseList(const command_definition_t *commandList, const char *rawLine, int flags)
{
	return CommandParseListContext(commandList, rawLine, flags, NULL);
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
