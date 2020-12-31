#include "xgui_hal.h"
#include "xgui_view.h"
#include <stdint.h>
#include <stdio.h>

#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/mousewheel.h"

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
    xgui_msg_t msg;
    xgui_msg_set(&msg, XGUI_MSG_MOUSE_MOTION, xgui_mouse.x, xgui_mouse.y, 0, 0);
    lv_mouse_handler(&msg);
}

static void mouse_wheel(int wheel)
{
    //printf("mouse wheel: %d\n", wheel);
    int id = (wheel == 0) ? XGUI_MSG_MOUSE_WHEEL_UP : XGUI_MSG_MOUSE_WHEEL_DOWN;
    xgui_msg_t msg;
    xgui_msg_set(&msg, id, xgui_mouse.x, xgui_mouse.y, 0, 0);
    lv_mousewheel_handler(&msg);
}

static void mouse_button_down(int button)
{
    //printf("mouse button: %d down\n", button);

    int id = -1;
    switch (button) {
    case 0:
        id = XGUI_MSG_MOUSE_LBTN_DOWN;
        break;
    case 1:
        id = XGUI_MSG_MOUSE_MBTN_DOWN;
        break;
    case 2:
        id = XGUI_MSG_MOUSE_RBTN_DOWN;
        break;
    default:
        break;
    }

    xgui_msg_t msg;
    xgui_msg_set(&msg, id, xgui_mouse.x, xgui_mouse.y, 0, 0);
    lv_mouse_handler(&msg);
}

static void mouse_button_up(int button)
{
    //printf("mouse button: %d up\n", button);
    
    int id = -1;
    switch (button) {
    case 0:
        id = XGUI_MSG_MOUSE_LBTN_UP;
        break;
    case 1:
        id = XGUI_MSG_MOUSE_MBTN_UP;
        break;
    case 2:
        id = XGUI_MSG_MOUSE_RBTN_UP;
        break;
    default:
        break;
    }

    xgui_msg_t msg;
    xgui_msg_set(&msg, id, xgui_mouse.x, xgui_mouse.y, 0, 0);
    lv_mouse_handler(&msg);
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