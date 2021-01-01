#include "xbrower_hal.h"
#include "xbrower_view.h"
#include "xbrower_core.h"
#include <unistd.h>
#include <sys/ioctl.h>

int xbrower_init()
{
    if (xbrower_screen_init() < 0) {
        return -1;
    }
    if (xbrower_mouse_init() < 0) {
        xbrower_screen_exit();
        return -1;
    }
    if (xbrower_keyboard_init() < 0) {
        xbrower_screen_exit();
        xbrower_mouse_exit();
        return -1;
    }

    if (xbrower_section_init() < 0) {
        xbrower_screen_exit();
        xbrower_mouse_exit();
        xbrower_keyboard_exit();
        return -1;
    }
    if (xbrower_view_init() < 0) {
        xbrower_screen_exit();
        xbrower_mouse_exit();
        xbrower_keyboard_exit();
        xbrower_section_exit();
        return -1;
    }
    return 0;
}

int xbrower_exit()
{
    xbrower_screen_exit();
    xbrower_mouse_exit();
    xbrower_keyboard_exit();
    xbrower_section_exit();
    xbrower_view_exit();
    return 0;
}

/* 输入的获取 */
int xbrower_loop()
{
    int i = 0;
    int has_event;
    while (1) {
        has_event = 0;
        if (!xbrower_mouse_poll()) {
            has_event = 1;
            i = 0;
        }
        if (!xbrower_keyboard_poll()) {
            has_event = 1;
            i = 0;
        }
        i++;
        if (i > 300 && !has_event) {
            sched_yeild();
        }
    }
}
