#ifndef _LIB_x86_XCHG_H
#define _LIB_x86_XCHG_H

char mem_xchg8(char *ptr, char value);
short mem_xchg16(short *ptr, short value);
int mem_xchg32(int *ptr, int value);

/**
 * __Xchg: 交换一个内存地址和一个数值的值
 * @x: 数值
 * @ptr: 内存指针
 * @size: 地址值的字节大小
 * 
 * 返回交换前地址中的值
 */
static inline unsigned int  mem_xchg(unsigned int x, 
        volatile void * ptr, int size)
{
    int old;
    switch (size) {
        case 1:
            old = mem_xchg8((char *)ptr, x);
            break;
        case 2:
            old = mem_xchg16((short *)ptr, x);
            break;
        case 4:
            old = mem_xchg32((int *)ptr, x);
            break;
    }
    return old;
}

#define xchg(ptr,v) ((__typeof__(*(ptr)))mem_xchg((unsigned int) \
        (v),(ptr),sizeof(*(ptr))))


/* 定义出口 */
#define __test_and_set(v, new) (xchg((v), new))

#endif  /* _LIB_x86_XCHG_H */
