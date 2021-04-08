#include <stdlib.h>

typedef void (*atexit_func_t)(void);

/* 最多支持32个退出时检测的函数调用 */
#define __ATEXIT_FUNC_NR    32
/* 初始化为NULL */
atexit_func_t __at_exit_func_table[__ATEXIT_FUNC_NR] = {0,};

/**
 * 注册退出时执行的回调函数
 */
void atexit(void (*func)(void))
{
    int i;
    for (i = 0; i < __ATEXIT_FUNC_NR; i++) {
        if (!__at_exit_func_table[i]) {
            __at_exit_func_table[i] = func;
            break;
        }
    }
}

/**
 * 执行注册的回调函数
 */
void __atexit_callback()
{
    int i;
    atexit_func_t func;
    for (i = 0; i < __ATEXIT_FUNC_NR; i++) {
        func = __at_exit_func_table[i];
        if (func) {
            func();
        } else {    /* 没有就直接退出循环 */
            break;
        }
    }
}