#include <sys/syscall.h>
#include <gscreen.h>

g_screen_t _g_screen;

int g_get_screen(g_screen_t *screen)
{
    return syscall1(int, SYS_GSCREENGET, screen);
}

int g_set_screen_window_region(g_region_t *region)
{
    return syscall1(int, SYS_GSCREENSETWINRG, region);
}
