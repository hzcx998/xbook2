#include "xgui_hal.h"
#include "xgui_view.h"
#include "xgui_core.h"
#include <xgui_dotfont.h>

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
    
    if (xgui_dotfont_init() < 0) {
        return -1;
    }
    
    xgui_view_init();

    return 0;
}

int xgui_loop()
{
    int i = 0;
    int has_event;
    while (1) {
        has_event = 0;
        if (!xgui_mouse_poll()) {
            has_event = 1;
            i = 0;
        }
        if (!xgui_keyboard_poll()) {
            has_event = 1;
            i = 0;
        }
        i++;
        if (i > 10 && !has_event) {
            sched_yeild();
        }
    }
}
