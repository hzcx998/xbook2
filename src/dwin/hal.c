#include <dwin/hal.h>
#include <stddef.h>

/* dwin hal interface */
struct dwin_hal *dwin_hal = NULL;

int dwin_hal_register(struct dwin_hal *hal)
{
    if (hal == NULL)
    {
        return -1;
    }

    if (hal->keyboard != NULL)
    {
        if (hal->keyboard->init != NULL)
        {
            if (hal->keyboard->init(hal->keyboard) < 0)
            {
                goto _bad_keyboard;
            }
        }
    }
    if (hal->mouse != NULL)
    {
        if (hal->mouse->init != NULL)
        {
            if (hal->mouse->init(hal->mouse) < 0)
            {
                goto _bad_mouse;
            }
        }
    }
    if (hal->lcd != NULL)
    {
        if (hal->lcd->init != NULL)
        {
            if (hal->lcd->init(hal->lcd) < 0)
            {
                goto _bad_lcd;
            }
        }
    }
    dwin_hal = hal;
    return 0;
    
_bad_lcd:
    if (hal->mouse != NULL)
    {
        if (hal->mouse->exit != NULL)
        {
            hal->mouse->exit(hal->mouse);
        }
    }
_bad_mouse:
    if (hal->keyboard != NULL)
    {
        if (hal->keyboard->exit != NULL)
        {
            hal->keyboard->exit(hal->keyboard);
        }
    }
_bad_keyboard:
    return -1;
}

int dwin_hal_unregister(struct dwin_hal *hal)
{
    if (dwin_hal == hal)
    {
        dwin_hal = NULL;
        return 0;
    }
    return -1;
}
