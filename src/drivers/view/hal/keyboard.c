#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/input.h>
#include <xbook/driver.h>
#include <unistd.h>
#include "drivers/view/hal.h"

#ifndef  DEV_NAME 
#define  DEV_NAME         "tty7"
#endif

int view_keyboard_open(view_keyboard_t *keyboard)
{
    keyboard->handle = device_open(DEV_NAME, 0);
    if (keyboard->handle < 0)
        return -1;
    keyboard->ledstate = 0;
    int flags = TTYFLG_NOWAIT;   // no block
    device_devctl(keyboard->handle, TIOCSFLGS, (unsigned long) &flags);
    /* 选择当前tty为当前tty */
    flags = TTYIO_RAW;
    device_devctl(keyboard->handle, TTYIO_SELECT, (unsigned long) &flags);
    return 0;
}

int view_keyboard_close(view_keyboard_t *keyboard)
{
    #if 0   /* 需要取消原来的设置，但是目前打开了 */
    int flags;
    device_devctl(keyboard->handle, EVENIO_GETFLG, (unsigned long) &flags);
    flags &= ~DEV_NOWAIT;
    device_devctl(keyboard->handle, EVENIO_SETFLG, (unsigned long) &flags);
    #endif
    return device_close(keyboard->handle);
}

int view_keyboard_read(view_keyboard_t *keyboard)
{
    unsigned char ttybuf[2] = {0,0};
    int ret = 0;
    ret = device_read(keyboard->handle, ttybuf, 2, 0);
    if (ret < 0) {
        return -1;
    }
    if (!ttybuf[1]) {
        // warnprint("view: key down %d,%d\n", ttybuf[0], ttybuf[1]);
        return keyboard->key_down(ttybuf[0]);   
    } else {
        // warnprint("view: key up %d,%d\n", ttybuf[0], ttybuf[1]);
        return keyboard->key_up(ttybuf[0]);
    }
    return -1;
}
