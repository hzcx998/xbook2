#include "xgui_hal.h"
#include "xgui_view.h"
#include "xgui.h"

int xgui_init()
{
    if (xgui_screen_init() < 0) {
        return -1;
    }
    if (xgui_mouse_init() < 0) {
        return -1;
    }
    if (xgui_keyboard_init() < 0) {
        return -1;
    }
    
    xgui_section_init();
    
    xgui_view_init();

    return 0;
}

int xgui_loop()
{
    while (1) {
        xgui_mouse_poll();
        xgui_keyboard_poll();
    }
    
}