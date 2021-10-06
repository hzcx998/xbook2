#include <dwin/hal.h>

extern struct dwin_hal_keyboard __kdevice_keyboard_hal;
extern struct dwin_hal_mouse __kdevice_mouse_hal;
extern struct dwin_hal_lcd __kdevice_lcd_hal;
extern struct dwin_hal_thread __kdevice_thread_hal;
extern struct dwin_hal_msgpool __kdevice_msgpool_hal;

struct dwin_hal __kdevice_hal = {
    .keyboard   = &__kdevice_keyboard_hal,
    .mouse      = &__kdevice_mouse_hal,
    .lcd        = &__kdevice_lcd_hal,
    .thread     = &__kdevice_thread_hal,
    .msgpool    = &__kdevice_msgpool_hal,
};

/**
 * init hal for device
 */
int dwin_hal_init(void)
{
    if (dwin_hal_register(&__kdevice_hal) < 0)
    {
        return -1;
    }
    return 0;
}

/**
 * exit hal
 */
void dwin_hal_exit(void)
{
    dwin_hal_unregister(&__kdevice_hal);
}
