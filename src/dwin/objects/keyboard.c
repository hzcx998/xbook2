#include <dwin/objects.h>
#include <dwin/dwin.h>
#include <dwin/workstation.h>

static int keyboard_key_down(struct dwin_keyboard *keyboard, int code)
{
    dwin_log("keyboard key: %d down\n", code);

    /* special process */

    if (dwin_current_workstation->focus_layer == NULL)
    {
        dwin_current_workstation->focus_layer = dwin_workstation_get_lowest_layer(dwin_current_workstation);        
        if (dwin_current_workstation->focus_layer == NULL)
        {
            return -1;
        }
    }
    
    /* send msg to focus layer */
    dwin_log(DWIN_TAG "keyboard down on layer %d\n", dwin_current_workstation->focus_layer->id);

    return 0;
}

static int keyboard_key_up(struct dwin_keyboard *keyboard, int code)
{
    dwin_log("keyboard key: %d up\n", code);
    
    /* special process */

    if (dwin_current_workstation->focus_layer == NULL)
    {
        dwin_current_workstation->focus_layer = dwin_workstation_get_lowest_layer(dwin_current_workstation);        
        if (dwin_current_workstation->focus_layer == NULL)
        {
            return -1;
        }
    }

    /* send msg to focus layer */
    dwin_log(DWIN_TAG "keyboard up on layer %d\n", dwin_current_workstation->focus_layer->id);

    return 0;
}

void dwin_keyboard_init(struct dwin_keyboard *keyboard)
{
    keyboard->handle = -1;
    keyboard->ledstate = 0;
    keyboard->key_modify = 0;
    keyboard->key_down = keyboard_key_down;
    keyboard->key_up = keyboard_key_up;
}
