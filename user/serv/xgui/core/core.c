#include "xgui_hal.h"
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
    int x, y;
    for (y = 0; y < 600; y++) {
        for (x = 0; x < 800; x++) {
            xgui_draw_point(x, y, 0xffffff);
        }
    }
    return 0;
}

int xgui_loop()
{
    while (1) {
        xgui_mouse_poll();
        xgui_keyboard_poll();
    }
    
}