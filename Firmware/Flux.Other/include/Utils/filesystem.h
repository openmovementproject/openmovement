// Stdio Filesystem Abstraction
// Dan Jackson, 2015

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "config.h"

// Used on a PIC?
#if (defined(USE_FAT_FS) || defined(USE_FS) || defined(USE_FF) || defined(__18CXX) || defined(__C30__) || defined(__PIC32MX__))

	// TODO: Remov ethis
	#include "HardwareProfile.h"

	#ifdef USE_FF					// Use the FatFS API directly

		#include "FatFs/ff.h"		// FatFS
		#include "FatFs/xff.h"		// Stdio-compatible functions
		
		// Remove any existing defines from stdio
		#ifdef FILE
			#undef FILE
		#endif
		#ifdef feof
			#undef feof
		#endif
		
		// Map stdio functions to MDD equivalents
		#define FILE FIL
		#define fopen f_fopen
		#define fclose f_fclose
		#define fseek f_fseek
		#define ftell f_tell
		#define feof f_eof
		#define fread f_fread
		#define fwrite f_fwrite
		#define fgetc f_fgetc
		#define fputc f_putc
		#define remove f_unlink
		#if !defined(_USE_STRFUNC) || _USE_STRFUNC < 1
			#define fgets f_fgets
		#else
			#define fgets f_gets
		#endif
		
	#elif defined(USE_FAT_FS) || defined(USE_FS)	// Using the Microchip Filesystem API

		#if defined(USE_FAT_FS) 					// FatFS-backed compatibility API as the Microchip FS API
			#include "FatFs/FatFsIo.h"
		#elif defined(USE_FS)						// Using the actual Microchip FS API
			#include "MDD File System/FSIO.h"
		#else
			#error "Unknown Microchip FS implementation"
		#endif
		
		// Remove any existing defines from stdio
		#ifdef FILE
			#undef FILE
		#endif
		#ifdef feof
			#undef feof
		#endif
		
		// Map stdio functions to MDD equivalents
		#define FILE FSFILE
		#define fopen FSfopen
		#define fclose FSfclose
		#define fseek FSfseek
		#define ftell FSftell
		#define feof FSfeof
		#define fread FSfread
		#define fwrite FSfwrite
		#define fgetc FSfgetc
		#define fgets FSfgets
		#define fputc FSfputc
		#define remove FSremove
		
		//#define SEEK_SET 0
		//#define SEEK_CUR 1
		//#define SEEK_END 2
		
		// fgetc/fputc implementation
		//static unsigned char FSfgetc(FILE *fp) { unsigned char c; if (fread(&c, 1, 1, fp) != 1) { return 0; } else { return c; } }
		//static unsigned char FSfputc(unsigned char c, FILE *fp) { if (fwrite(&c, 1, 1, fp) != 1) { return 0; } else { return 1; } }

	#else

		#warning "Using stdio.h for file system access on a PIC - this will not work."
		#include <stdio.h>

	#endif
		
#else

	// STDIO
	#include <stdio.h>

#endif


#endif
