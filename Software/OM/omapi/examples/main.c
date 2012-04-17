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

/** 
 *  @file main.c
 *  @brief     Open Movement API Example: Command Line Interface test application.
 *  @author    Dan Jackson
 *  @date      2011-2012
 *  @copyright BSD 2-clause license. Copyright (c) 2009-2012, Newcastle University, UK. All rights reserved.
 *  @details
 *
 *  This is a command-line tool to call the provided example code files.
 */


#ifdef _WIN32
#pragma comment(lib, "omapi.lib")
#if defined(_DEBUG)
#include <windows.h>
#endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* Externally available main functions for each example */
extern int test_main(int argc, char *argv[]);
extern int deploy_main(int argc, char *argv[]);
extern int download_main(int argc, char *argv[]);
extern int clear_main(int argc, char *argv[]);
extern int convert_main(int argc, char *argv[]);
extern int verify_main(int argc, char *argv[]);


/* Main function just chains to the required example code */
int main(int argc, char *argv[])
{
    int ret;

    printf("OMAPI Example Code Command Line Interface\n");
    printf("\n");

    if      (argc > 1 && !strcmp(argv[1], "test"  ))   { ret =     test_main(argc - 1, argv + 1); }
    else if (argc > 1 && !strcmp(argv[1], "deploy"))   { ret =   deploy_main(argc - 1, argv + 1); }
    else if (argc > 1 && !strcmp(argv[1], "download")) { ret = download_main(argc - 1, argv + 1); }
    else if (argc > 1 && !strcmp(argv[1], "clear"))    { ret =    clear_main(argc - 1, argv + 1); }
    else if (argc > 1 && !strcmp(argv[1], "convert"))  { ret =  convert_main(argc - 1, argv + 1); }
    else if (argc > 1 && !strcmp(argv[1], "verify"))   { ret =   verify_main(argc - 1, argv + 1); }
    else
    {
        printf("Usage: omapi <test|deploy|download|clear|convert|verify> [parameters...]\n");
        printf("\n");
        ret = -1;
    }

#if defined(_WIN32) && defined(_DEBUG)
    if (IsDebuggerPresent()) { fprintf(stderr, "Press [enter] to exit..."); getc(stdin); }
#endif

    return ret;
}
