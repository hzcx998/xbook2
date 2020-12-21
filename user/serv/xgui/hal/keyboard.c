#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/input.h>
#include <unistd.h>
#include "xgui_hal.h"

#ifndef  DEV_NAME 
#define  DEV_NAME         "kbd"
#endif

int xgui_keyboard_open(xgui_keyboard_t *keyboard)
{
    keyboard->handle = opendev(DEV_NAME, 0);
    if (keyboard->handle < 0)
        return -1;

    keyboard->ledstate = 0;
    int ledstate;
    ioctl(keyboard->handle, EVENIO_GETLED, (void *) &ledstate);
    if ( ledstate&0x01 )
        keyboard->ledstate |= XGUI_KMOD_NUM;

    if ( ledstate & 0x02 )
        keyboard->ledstate |= XGUI_KMOD_CAPS;

    int flags = DEV_NOWAIT;   // no block
    ioctl(keyboard->handle, EVENIO_SETFLG, (void *) &flags);
    return 0;
}

int xgui_keyboard_close(xgui_keyboard_t *keyboard)
{
    return close(keyboard->handle);
}

int xgui_keyboard_read(xgui_keyboard_t *keyboard)
{
    struct input_event event;
    int ret = 0;
    memset(&event, 0, sizeof(event));
    ret = read(keyboard->handle, &event, sizeof(event));
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