#include <arch/memory.h>

int backtrace(void **buffer, int size)
{
    int i = 0;
    int n = 0;
    unsigned int _ebp = 0;
    unsigned int _eip = 0;
    __asm__ __volatile__(" movl %%ebp, %0" :"=g" (_ebp)::"memory");
    for(i = 0; i < size && _ebp != 0 && *(unsigned int*)_ebp != 0 &&
            *(unsigned int *)_ebp != _ebp; i++) {
        _eip = (unsigned int)((unsigned int*)_ebp + 1);
        _eip = *(unsigned int*)_eip;
        _ebp = *(unsigned int*)_ebp;
        buffer[i] = (void*)_eip;
        n++;
    }
    return n;
}