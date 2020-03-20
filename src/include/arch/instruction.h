#ifndef _ARCH_INSTRUCTION_H
#define _ARCH_INSTRUCTION_H

#include "general.h"

#define xchg(ptr,v) ((__typeof__(*(ptr)))__xchg((unsigned int) \
        (v),(ptr),sizeof(*(ptr))))

/**
 * __Xchg: 交换一个内存地址和一个数值的值
 * @x: 数值
 * @ptr: 内存指针
 * @size: 地址值的字节大小
 * 
 * 返回交换前地址中的值
 */
static inline unsigned int  __xchg(unsigned int x, 
        volatile void * ptr, int size)
{
    int old;
    switch (size) {
        case 1:
            old = __xchg8((char *)ptr, x);
            break;
        case 2:
            old = __xchg16((short *)ptr, x);
            break;
        case 4:
            old = __xchg32((int *)ptr, x);
            break;
    }
    return old;
}

#endif  /* _ARCH_INSTRUCTION_H */
