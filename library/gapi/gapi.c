#include <stdint.h>
#include <sys/syscall.h>

#include <gapi.h>
#include <gscreen.h>

int g_init(void)
{
    /* call ginit */
    int val = syscall0(int, SYS_GINIT);
    if (val < 0) 
        return -1;
    /* get screen info */
    if (g_screen_get(&_g_screen) < 0)
        return -1;
    
    g_init_msg();
    
    return val;
}

int g_quit(void)
{
    int val = syscall0(int, SYS_GQUIT);
    
    return val;
}
