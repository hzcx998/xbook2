#include "xgui_hal.h"
#include "xgui_view.h"
#include <stdint.h>
#include <stdio.h>

static xgui_mouse_t xgui_mouse;

static void mouse_motion(void)
{
    if (xgui_mouse.x < 0)
        xgui_mouse.x = 0;
    if (xgui_mouse.y < 0)
        xgui_mouse.y = 0;
    if (xgui_mouse.x > xgui_screen.width - 1)
        xgui_mouse.x = xgui_screen.width - 1;
    if (xgui_mouse.y > xgui_screen.height - 1)
        xgui_mouse.y = xgui_screen.height - 1;
    //printf("mouse motion: %d, %d\n", xgui_mouse.x, xgui_mouse.y);
    xgui_view_set_xy(xgui_view_get_top(), xgui_mouse.x, xgui_mouse.y);
}

static void mouse_wheel(int wheel)
{
    printf("mouse wheel: %d\n", wheel);
}

static void mouse_button_down(int button)
{
    printf("mouse button: %d down\n", button);

}

static void mouse_button_up(int button)
{
    printf("mouse button: %d up\n", button);
}

int xgui_mouse_init()
{
    if (xgui_mouse_open(&xgui_mouse) < 0) {
        return -1;
    }
    xgui_mouse.motion = mouse_motion;
    xgui_mouse.button_down = mouse_button_down;
    xgui_mouse.button_up = mouse_button_up;
    xgui_mouse.wheel = mouse_wheel;
    xgui_mouse.x = xgui_screen.width / 2;
    xgui_mouse.y = xgui_screen.height / 2;
    return 0;
}

int xgui_mouse_poll()
{
    return xgui_mouse_read(&xgui_mouse);
}