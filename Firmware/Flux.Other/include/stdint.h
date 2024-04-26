// Partial stdint.h
// Dan Jackson, 2011
// Karim Ladha, 2016: Expanded for limit defs

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

#define UINT8_MAX   255

#define INT8_MAX	127
#define INT8_MIN	(-128)

#define UINT16_MAX  65535

#define INT16_MIN   (-32767-1)
#define INT16_MAX   32767

#define UINT32_MAX  4294967295UL
#define INT32_MAX   2147483647L
#define INT32_MIN   (-2147483647L-1)

#define INT64_MIN   (-9223372036854775807LL-1)
#define INT64_MAX   9223372036854775807LL
#define UINT64_MAX  18446744073709551615ULL

#define INTMAX_MIN  (-9223372036854775807LL-1)
#define INTMAX_MAX  9223372036854775807LL
#define UINTMAX_MAX 18446744073709551615ULL

// KL: To fix IDE bug ignoring search prescedent ordering angled brackets
#if defined(__CROSSWORKS_ARM) 

typedef int8_t int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;

typedef int32_t int_fast8_t;
typedef int32_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;
    
typedef uint32_t uint_fast8_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;


#endif

#endif
