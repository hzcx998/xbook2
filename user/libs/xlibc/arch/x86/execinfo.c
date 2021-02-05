#include <execinfo.h>

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

int backtrace2(void** buffer, int size)
{
    int n = 0;
    int* p = &n;
    int i = 0;
    int ebp = p[5]; /* 5是局部变量的数量 */
    int eip = p[5 + 1]; /* eip在ebp的上面 */
    for(i = 0; i < size && ebp != 0 && *(unsigned int*)ebp != 0 &&
            *(unsigned int *)ebp != ebp; i++) {
        buffer[i] = (void*)eip;
        p = (int*)ebp;
        ebp = p[0];
        eip = p[1];
        n++;
    }
    return n;
}
