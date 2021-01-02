#include "drivers/view/hal.h"
#include "drivers/view/view.h"
#include "drivers/view/msg.h"
#include <stdint.h>
#include <stdio.h>

view_mouse_t view_mouse;

static void mouse_motion(void)
{
    if (view_mouse.x < 0)
        view_mouse.x = 0;
    if (view_mouse.y < 0)
        view_mouse.y = 0;
    if (view_mouse.x > view_screen.width - 1)
        view_mouse.x = view_screen.width - 1;
    if (view_mouse.y > view_screen.height - 1)
        view_mouse.y = view_screen.height - 1;
    //keprint("mouse pos: %d, %d\n", view_mouse.x, view_mouse.y);
    view_msg_t msg;
    view_msg_header(&msg, VIEW_MSG_MOUSE_MOTION, -1);
    view_msg_data(&msg, view_mouse.x, view_mouse.y, 0, 0);
    view_put_global_msg(&msg);
    // move mouse
    view_set_xy(view_mouse.view, view_mouse.x, view_mouse.y);
}

static void mouse_wheel(int wheel)
{
    keprint("mouse wheel: %d\n", wheel);
    int id = (wheel == 0) ? VIEW_MSG_MOUSE_WHEEL_UP : VIEW_MSG_MOUSE_WHEEL_DOWN;
    view_msg_t msg;
    view_msg_header(&msg, id, -1);
    view_msg_data(&msg, view_mouse.x, view_mouse.y, 0, 0);
    view_put_global_msg(&msg);
}

static void mouse_button_down(int button)
{
    keprint("mouse button: %d down\n", button);

    int id = -1;
    switch (button) {
    case 0:
        id = VIEW_MSG_MOUSE_LBTN_DOWN;
        break;
    case 1:
        id = VIEW_MSG_MOUSE_MBTN_DOWN;
        break;
    case 2:
        id = VIEW_MSG_MOUSE_RBTN_DOWN;
        break;
    default:
        break;
    }

    view_msg_t msg;
    view_msg_header(&msg, id, -1);
    view_msg_data(&msg, view_mouse.x, view_mouse.y, 0, 0);
    view_put_global_msg(&msg);
}

static void mouse_button_up(int button)
{
    keprint("mouse button: %d up\n", button);
    
    int id = -1;
    switch (button) {
    case 0:
        id = VIEW_MSG_MOUSE_LBTN_UP;
        break;
    case 1:
        id = VIEW_MSG_MOUSE_MBTN_UP;
        break;
    case 2:
        id = VIEW_MSG_MOUSE_RBTN_UP;
        break;
    default:
        break;
    }

    view_msg_t msg;
    view_msg_header(&msg, id, -1);
    view_msg_data(&msg, view_mouse.x, view_mouse.y, 0, 0);
    view_put_global_msg(&msg);
}

int view_mouse_init()
{
    if (view_mouse_open(&view_mouse) < 0) {
        return -1;
    }
    view_mouse.motion = mouse_motion;
    view_mouse.button_down = mouse_button_down;
    view_mouse.button_up = mouse_button_up;
    view_mouse.wheel = mouse_wheel;
    view_mouse.x = view_screen.width / 2;
    view_mouse.y = view_screen.height / 2;

    /* 创建鼠标视图 */
    view_t *view = view_create(view_mouse.x, view_mouse.y, 32, 32);
    //assert(view);
    view_render_rectfill(view, 0, 0, view->width, view->height, VIEW_RED);
    view_move_upper_top(view);
    view_mouse.view = view;

    return 0;
}

int view_mouse_exit()
{
    view_mouse_close(&view_mouse);
    return 0;
}

int view_mouse_poll()
{
    return view_mouse_read(&view_mouse);
}