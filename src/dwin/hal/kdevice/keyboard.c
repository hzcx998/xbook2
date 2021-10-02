#include <dwin/hal.h>
#include <stddef.h>

#include <xbook/debug.h>

static int keyboard_init(struct dwin_hal_keyboard *keyboard)
{
    dbgprint("kdevice keyboard init\n");

    return 0;
}

static void keyboard_exit(struct dwin_hal_keyboard *keyboard)
{
    dbgprint("kdevice keyboard exit\n");
}

struct dwin_hal_keyboard __kdevice_keyboard_hal = {
    .init = &keyboard_init,
    .exit = &keyboard_exit,
    .extension = NULL,
};
