#ifndef _XBOOK_STDDEF_H
#define _XBOOK_STDDEF_H

#include <xbook/config.h>

typedef unsigned long size_t;
typedef long ssize_t;
typedef unsigned long dma_addr_t;

typedef int wchar_t;

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
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#ifndef NULL
#ifdef __cplusplus
        #define NULL 0
#else
        #define NULL ((void *)0)
#endif
#endif

#ifndef __cplusplus
//#define bool _Bool      //C语言下实现Bool
#define bool char      

#define true 1
#define false 0
#endif

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

#endif  /*_XBOOK_STDDEF_H*/

