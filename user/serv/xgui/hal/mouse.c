#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/input.h>
#include <unistd.h>
#include "xgui_hal.h"
#ifndef   DEV_NAME
#define   DEV_NAME  "mouse"
#endif

int xgui_mouse_open(xgui_mouse_t *mouse)
{
    mouse->handle = opendev(DEV_NAME, 0);
    if (mouse->handle < 0)
        return -1;
    int flags = DEV_NOWAIT;   // no block
    ioctl(mouse->handle, EVENIO_SETFLG, (void *) &flags);
    return 0;
}

int xgui_mouse_close(xgui_mouse_t *mouse)
{
    return close(mouse->handle);
}

int xgui_mouse_read(xgui_mouse_t *mouse)
{
    static int x_rel = 0;
    static int y_rel = 0;
    static int z_rel = 0;
    static int flag_rel = 0;
    struct input_event event;
    int ret = 0;
need_next:
    memset(&event, 0, sizeof(event));
    ret = read(mouse->handle, &event, sizeof(event));
    if (ret < 1)
        return -1;
    switch (event.type) {        
    case EV_REL:
        if ((event.code) == REL_X) {
            x_rel += event.value; 
            flag_rel  = 1;
            goto  need_next;
        } else if ((event.code) == REL_Y) {
            y_rel    += event.value; 
            flag_rel  = 1;
            goto  need_next;
        } else if ( (event.code) == REL_WHEEL ) {
            z_rel = (int )event.value;
            mouse->wheel(z_rel < 0 ? 0 : 1);
            return  0;           
        } else {
            return  0;
        }
        break;
    case EV_KEY:
        if ((event.code) == BTN_LEFT){
            if (event.value > 0) {
                mouse->button_down(0);
            } else {
                mouse->button_up(0);
            }
            return  0;
        } else if ((event.code) == BTN_MIDDLE) {
            if (event.value > 0) {
                mouse->button_down(1);
            } else {
                mouse->button_up(1);
            }
            return  0;
        } else if ((event.code) == BTN_RIGHT) {
            if (event.value > 0) {
                mouse->button_down(2);
            } else {
                mouse->button_up(2);
            }
            return  0;
        } else {
            return  0;
        }
        break;
    case EV_MSC:
        return  0;
    case EV_SYN:
        mouse->x += x_rel;
        mouse->y += y_rel;
        x_rel = 0;
        y_rel = 0;
        if (flag_rel == 1) {
            mouse->motion();
            flag_rel = 0;
            return  0;
        }
        flag_rel = 0;
        break;
    default:
        break;
    }
    return  0;
}
