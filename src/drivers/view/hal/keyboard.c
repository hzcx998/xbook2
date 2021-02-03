#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/input.h>
#include <xbook/driver.h>
#include <unistd.h>
#include "drivers/view/hal.h"

#if 0

#ifndef  DEV_NAME 
#define  DEV_NAME         "kbd"
#endif

int view_keyboard_open(view_keyboard_t *keyboard)
{
    keyboard->handle = device_open(DEV_NAME, 0);
    if (keyboard->handle < 0)
        return -1;
    keyboard->ledstate = 0;
    int ledstate;
    device_devctl(keyboard->handle, EVENIO_GETLED, (unsigned long) &ledstate);
    if ( ledstate&0x01 )
        keyboard->ledstate |= VIEW_KMOD_NUM;

    if ( ledstate & 0x02 )
        keyboard->ledstate |= VIEW_KMOD_CAPS;
    int flags = DEV_NOWAIT;   // no block
    device_devctl(keyboard->handle, EVENIO_SETFLG, (unsigned long) &flags);
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
    struct input_event event;
    int ret = 0;
    memset(&event, 0, sizeof(event));
    ret = device_read(keyboard->handle, &event, sizeof(event), 0);
    if (ret < 0)
        return -1;
    switch (event.type) {
    case EV_KEY:
        if ( (event.value) > 0 ) {
            return keyboard->key_down(event.code);   
        } else {
            return keyboard->key_up(event.code);
        }
    default:
        break;
    }
    return -1;
}
#else

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
#endif