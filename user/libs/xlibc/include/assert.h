
#ifndef _XLIBC_ASSERT_H
#define _XLIBC_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

//#define NDEBUG

#define ASSERT_VOID_CAST  (void)
#if 0
//version 1

//fprintf(stderr, "Assertion failed:%s, file %s, line %d\n", #expr, __FILE__,__LINE__);

#ifndef NDEBUG
#define assert(expr)  \
    if(expr)  \
        do{ \
            printf("Assertion failed:%s, file %s, line %d\n", #expr, __FILE__,__LINE__); \
            abort(); \
        }while(0)
#else
#define assert(expr)  ASSERT_VOID_CAST(0)
#endif
#endif

#if 1
////version 2
static inline void assert_fail(const char *expr, const char *file, int line)
{
    //fprintf(stderr, "Assertion failed:%s, file %s, line %d\n", expr, file, line);
    printf("Assertion failed:%s, file %s, line %d\n", expr, file, line);     
    abort();
}

#ifndef NDEBUG
#define assert(expr)  \
   ( (expr) \
     ? ASSERT_VOID_CAST(0)   \
     : assert_fail(#expr, __FILE__, __LINE__) \
    )
#else
#define assert(expr)  ASSERT_VOID_CAST(0)
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_ASSERT_H */
