#ifndef _XLIBC_STDDEF_H
#define _XLIBC_STDDEF_H

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned long long uint64_t;
typedef signed long long int64_t;
typedef unsigned int    uint32_t;
typedef signed int      int32_t;
typedef unsigned short  uint16_t;
typedef signed short    int16_t;
typedef unsigned char   uint8_t;
typedef signed char     int8_t;

typedef unsigned long long  uint64;
typedef signed long long int64;
typedef unsigned int    uint32;
typedef signed int      int32;
typedef unsigned short  uint16;
typedef signed short    int16;
typedef unsigned char   uint8;
typedef signed char     int8;

typedef unsigned long long u64;
typedef signed long long s64;
typedef unsigned int    u32;
typedef signed int      s32;
typedef unsigned short  u16;
typedef signed short    s16;
typedef unsigned char   u8;
typedef signed char     s8;

typedef unsigned long size_t;
typedef long ssize_t;

typedef unsigned long dma_addr_t;

#ifndef __cplusplus

#ifndef _WCHAR_T_DEFINED   
typedef unsigned short wchar_t;   
#define _WCHAR_T_DEFINED   
#endif  

#endif /* __cplusplus */

typedef long int ptrdiff_t;

typedef int id_t;

/*
 *这里是define类型的
 */
#define write_once(var, val) \
        (*((volatile typeof(val) *)(&(var))) = (val))

/*
 * 我只能把大名鼎鼎的container_of宏直接学习过来了。 (*^.^*)
 */

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({ \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - offsetof(type,member) ); \
})

/* 
这里的__built_expect()函数是gcc(version >= 2.96)的内建函数,提供给程序员使用的，目的是将"分支转移"的信息提供给编译器，这样编译器对代码进行优化，以减少指令跳转带来的性能下降。
__buildin_expect((x), 1)表示x的值为真的可能性更大.
__buildin_expect((x), 0)表示x的值为假的可能性更大. 
*/
#if 0 /* no likely in stddef */
#if defined (__GNUC__)
#  define unlikely(expr) __builtin_expect ((expr), 0)
#else
#  define unlikely(expr)  (expr)
#endif

#if defined (__GNUC__)
#  define likely(x) __builtin_expect(!!(x), 1)
#else
#  define likely(expr)  (expr)
#endif
#endif

#ifndef NULL
#ifdef __cplusplus
        #define NULL 0
#else
        #define NULL ((void *)0)
#endif
#endif
#include <stdbool.h>

#ifndef BOOLEAN
#ifndef __cplusplus
    #define BOOLEAN char     
#else
    #define BOOLEAN _Bool       //C语言下实现Bool
#endif
    #ifndef TRUE
    #define TRUE    1 
    #endif

    #ifndef FALSE
    #define FALSE    0 
    #endif
#endif


#ifndef MAX_PATH
#define MAX_PATH    256
#endif

#define ifloor(x)		((x) > 0 ? (int)(x) : (int)((x) - 0.9999999999))
#define iround(x)		((x) > 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))
#define iceil(x)		((x) > 0 ? (int)((x) + 0.9999999999) : (int)(x))
#define idiv255(x)		((((int)(x) + 1) * 257) >> 16)

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define clamp(v, a, b)	min(max(a, v), b)

// This macroshould not be used.A use like abs(a++) will make a wrong result.Use abs() and labs()
// #define abs(a)    ((a) > 0 ? (a) : -(a))

/* 除后上入 */
#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))

/* 除后下舍 */
#define DIV_ROUND_DOWN(X, STEP) ((X) / (STEP))

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_STDDEF_H */

