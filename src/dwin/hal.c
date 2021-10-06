#include <dwin/dwin.h>
#include <dwin/hal.h>
#include <dwin/objects.h>

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
            dwin_keyboard_init(&hal->keyboard->parent);
            if (hal->keyboard->init(&hal->keyboard->parent) < 0)
            {
                goto _bad_keyboard;
            }
        }
    }
    if (hal->mouse != NULL)
    {
        if (hal->mouse->init != NULL)
        {
            dwin_mouse_init(&hal->mouse->parent);
            if (hal->mouse->init(&hal->mouse->parent) < 0)
            {
                goto _bad_mouse;
            }
        }
    }
    if (hal->lcd != NULL)
    {
        if (hal->lcd->init != NULL)
        {
            dwin_lcd_init(&hal->lcd->parent);
            if (hal->lcd->init(&hal->lcd->parent) < 0)
            {
                goto _bad_lcd;
            }
            if (dwin_lcd_map(&hal->lcd->parent) < 0)
            {
                hal->lcd->exit(&hal->lcd->parent);
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
            hal->mouse->exit(&hal->mouse->parent);
        }
    }
_bad_mouse:
    if (hal->keyboard != NULL)
    {
        if (hal->keyboard->exit != NULL)
        {
            hal->keyboard->exit(&hal->keyboard->parent);
        }
    }
_bad_keyboard:
    return -1;
}

int dwin_hal_unregister(struct dwin_hal *hal)
{
    if (dwin_hal != hal)
    {
        return -1;
    }

    if (hal->lcd != NULL)
    {
        dwin_lcd_unmap(&hal->lcd->parent);
        if (hal->lcd->exit != NULL)
        {
            hal->lcd->exit(&hal->lcd->parent);
        }
    }
    if (hal->mouse != NULL)
    {
        if (hal->mouse->exit != NULL)
        {
            hal->mouse->exit(&hal->mouse->parent);
        }
    }
    if (hal->keyboard != NULL)
    {
        if (hal->keyboard->exit != NULL)
        {
            hal->keyboard->exit(&hal->keyboard->parent);
        }
    }
    dwin_hal = NULL;
    return 0;
}
