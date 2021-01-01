#include "xbrower_hal.h"
#include "xbrower_view.h"
#include <stdint.h>
#include <stdio.h>

#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/mousewheel.h"

static xbrower_mouse_t xbrower_mouse;

static void mouse_motion(void)
{
    if (xbrower_mouse.x < 0)
        xbrower_mouse.x = 0;
    if (xbrower_mouse.y < 0)
        xbrower_mouse.y = 0;
    if (xbrower_mouse.x > xbrower_screen.width - 1)
        xbrower_mouse.x = xbrower_screen.width - 1;
    if (xbrower_mouse.y > xbrower_screen.height - 1)
        xbrower_mouse.y = xbrower_screen.height - 1;
    xbrower_msg_t msg;
    xbrower_msg_set(&msg, XGUI_MSG_MOUSE_MOTION, xbrower_mouse.x, xbrower_mouse.y, 0, 0);
    lv_mouse_handler(&msg);
}

static void mouse_wheel(int wheel)
{
    //printf("mouse wheel: %d\n", wheel);
    int id = (wheel == 0) ? XGUI_MSG_MOUSE_WHEEL_UP : XGUI_MSG_MOUSE_WHEEL_DOWN;
    xbrower_msg_t msg;
    xbrower_msg_set(&msg, id, xbrower_mouse.x, xbrower_mouse.y, 0, 0);
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

    xbrower_msg_t msg;
    xbrower_msg_set(&msg, id, xbrower_mouse.x, xbrower_mouse.y, 0, 0);
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

    xbrower_msg_t msg;
    xbrower_msg_set(&msg, id, xbrower_mouse.x, xbrower_mouse.y, 0, 0);
    lv_mouse_handler(&msg);
}

int xbrower_mouse_init()
{
    if (xbrower_mouse_open(&xbrower_mouse) < 0) {
        return -1;
    }
    xbrower_mouse.motion = mouse_motion;
    xbrower_mouse.button_down = mouse_button_down;
    xbrower_mouse.button_up = mouse_button_up;
    xbrower_mouse.wheel = mouse_wheel;
    xbrower_mouse.x = xbrower_screen.width / 2;
    xbrower_mouse.y = xbrower_screen.height / 2;
    return 0;
}

int xbrower_mouse_exit()
{
    xbrower_mouse_close(&xbrower_mouse);
    return 0;
}

int xbrower_mouse_poll()
{
    return xbrower_mouse_read(&xbrower_mouse);
}