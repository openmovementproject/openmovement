// stdbool.h
// Dan Jackson, 2011

#ifndef __bool_true_and_false_are_defined

#ifndef __cplusplus

// stdbool defines
#define __bool_true_and_false_are_defined 1
#define bool _Bool
#define true 1
#define false 0

// C99 _Bool type
#ifdef _MSC_VER
typedef char _Bool;
#endif

#endif

#endif
