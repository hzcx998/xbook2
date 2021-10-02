#include <dwin/objects.h>
#include <dwin/dwin.h>

static int keyboard_key_down(struct dwin_keyboard *keyboard, int code)
{
    dwin_log("keyboard key: %d down\n", code);
    return 0;
}

static int keyboard_key_up(struct dwin_keyboard *keyboard, int code)
{
    dwin_log("keyboard key: %d up\n", code);
    
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
