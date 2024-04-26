/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _INTEGER
#define _INTEGER

#ifdef _WIN32	/* FatFs development platform */

#include <windows.h>
//#include <tchar.h>


#elif defined(__18CXX) || defined(__C30__) || defined(__PIC32MX__)  // [dgj] Added this section for PIC platforms so types are guaranteed compatible

#include "GenericTypeDefs.h"
//typedef char			CHAR;
typedef unsigned short	USHORT;
typedef unsigned long	ULONG;
typedef unsigned short	WCHAR;

#else			/* Embedded platform */

/* These types must be 16-bit, 32-bit or larger integer */
typedef int				INT;
typedef unsigned int	UINT;

/* These types must be 8-bit integer */
typedef char			CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types must be 16-bit integer */
typedef short			SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;

/* These types must be 32-bit integer */
typedef long			LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

#endif

#endif
