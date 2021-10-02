#include <dwin/dwin.h>
#include <dwin/hal.h>

/* xbook kernel header */
#include <xbook/driver.h>
#include <sys/ioctl.h>
#include <sys/input.h>
#include <unistd.h>

#ifndef  KDEVICE_MOUSE_NAME 
#define  KDEVICE_MOUSE_NAME "mouse"
#endif

static int mouse_init(struct dwin_mouse *mouse)
{
    mouse->handle = device_open(KDEVICE_MOUSE_NAME, O_RDWR);
    if (mouse->handle < 0)
    {
        return -1;
    }

    int flags = DEV_NOWAIT;   // no block
    device_devctl(mouse->handle, EVENIO_SETFLG, (unsigned long) &flags);
    return 0;
}

static int mouse_exit(struct dwin_mouse *mouse)
{
    if (!mouse)
    {
        return -1;
    }
    if (mouse->handle < 0)
    {
        return -1;
    }
    int flags;
    device_devctl(mouse->handle, EVENIO_GETFLG, (unsigned long) &flags);
    flags &= ~DEV_NOWAIT;
    device_devctl(mouse->handle, EVENIO_SETFLG, (unsigned long) &flags);
    device_close(mouse->handle);
    return 0;
}

static int mouse_read(struct dwin_mouse *mouse)
{
    static int x_rel = 0;
    static int y_rel = 0;
    static int z_rel = 0;
    static int flag_rel = 0;
    struct input_event event;
    int ret = 0;

need_next:
    memset(&event, 0, sizeof(event));
    ret = device_read(mouse->handle, &event, sizeof(event), 0);
    if (ret < 1)
    {
        return -1;
    }

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
            mouse->wheel(mouse, z_rel < 0 ? 0 : 1);
            return  0;           
        } else {
            return  0;
        }
        break;
    case EV_KEY:
        if ((event.code) == BTN_LEFT){
            if (event.value > 0) {
                mouse->button_down(mouse, 0);
            } else {
                mouse->button_up(mouse, 0);
            }
            return  0;
        } else if ((event.code) == BTN_MIDDLE) {
            if (event.value > 0) {
                mouse->button_down(mouse, 1);
            } else {
                mouse->button_up(mouse, 1);
            }
            return  0;
        } else if ((event.code) == BTN_RIGHT) {
            if (event.value > 0) {
                mouse->button_down(mouse, 2);
            } else {
                mouse->button_up(mouse, 2);
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
            mouse->motion(mouse);
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

struct dwin_hal_mouse __kdevice_mouse_hal = {
    .init = mouse_init,
    .exit = mouse_exit,
    .read = mouse_read,
    .extension = NULL,
};
