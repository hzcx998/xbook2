#include <xbook/debug.h>

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

void print_backtrace(void)
{
    void *buf[BACKTRACE_CNT] = {0};
    int cnt = backtrace(buf, BACKTRACE_CNT);
    dbgprint("[!] Call backtrace:\n");
    int i;
    for (i = 0; i < cnt; i++)
    {
        dbgprint("%d: call %p\n", i, buf[i]);
    }
    dbgprint("[!] Call backtrace done.\n");
}
