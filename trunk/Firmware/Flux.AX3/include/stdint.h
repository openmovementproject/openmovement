// Partial stdint.h
// Dan Jackson, 2011

#ifndef _STDINT
#define _STDINT

// 8-, 16-, 32-, 64-bit types
#ifndef int8_t
#define int8_t int8_t
typedef signed char int8_t;
#endif

#ifndef uint8_t
#define uint8_t uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef int16_t
#define int16_t int16_t
typedef signed short int16_t;
#endif

#ifndef uint16_t
#define uint16_t uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef int32_t
#define int32_t int32_t
typedef signed long int32_t;
#endif

#ifndef uint32_t
#define uint32_t uint32_t
typedef unsigned long uint32_t;
#endif

#ifndef int64_t
#define int64_t int64_t
typedef signed long long int64_t;
#endif

#ifndef uint64_t
#define uint64_t uint64_t
typedef unsigned long long uint64_t;
#endif

// Pointers are 16-bits
#ifndef intptr_t
#define intptr_t intptr_t
typedef int intptr_t;
#endif

#ifndef uintptr_t
#define uintptr_t uintptr_t
typedef unsigned int uintptr_t;
#endif

#endif
