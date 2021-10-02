#include <dwin/dwin.h>
#include <dwin/hal.h>

/* xbook kernel header */
#include <xbook/driver.h>
#include <sys/ioctl.h>
#include <sys/input.h>
#include <unistd.h>

#ifndef  KDEVICE_KEYBOARD_NAME 
#define  KDEVICE_KEYBOARD_NAME "tty7"
#endif

static int keyboard_init(struct dwin_keyboard *keyboard)
{
    keyboard->handle = device_open(KDEVICE_KEYBOARD_NAME, O_RDWR);
    if (keyboard->handle < 0)
    {
        return -1;
    }

    keyboard->ledstate = 0;
    int flags = TTYFLG_NOWAIT;   // no block
    device_devctl(keyboard->handle, TIOCSFLGS, (unsigned long) &flags);
    
    /* switch to tty raw */
    flags = TTYIO_RAW;
    device_devctl(keyboard->handle, TTYIO_SELECT, (unsigned long) &flags);
    return 0;
}

static int keyboard_exit(struct dwin_keyboard *keyboard)
{
    if (!keyboard)
    {
        return -1;
    }
    if (keyboard->handle < 0)
    {    
        return -1;
    }
    int flags;
    device_devctl(keyboard->handle, EVENIO_GETFLG, (unsigned long) &flags);
    flags &= ~DEV_NOWAIT;
    device_devctl(keyboard->handle, EVENIO_SETFLG, (unsigned long) &flags);
    device_close(keyboard->handle);
    return 0;
}

static int keyboard_read(struct dwin_keyboard *keyboard)
{
    assert(keyboard->handle >= 0);
    unsigned char ttybuf[2] = {0,0};
    int ret = 0;
    ret = device_read(keyboard->handle, ttybuf, 2, 0);
    if (ret < 0)
    {
        return -1;
    }
    if (!ttybuf[1])
    {
        dwin_log("key down %d,%d\n", ttybuf[0], ttybuf[1]);
        return keyboard->key_down(keyboard, ttybuf[0]);   
    }
    else
    {
        dwin_log("key up %d,%d\n", ttybuf[0], ttybuf[1]);
        return keyboard->key_up(keyboard, ttybuf[0]);
    }
    return -1;
}

struct dwin_hal_keyboard __kdevice_keyboard_hal = {
    .init = keyboard_init,
    .exit = keyboard_exit,
    .read = keyboard_read,
    .extension = NULL,
};
