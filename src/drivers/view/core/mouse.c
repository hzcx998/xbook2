#include "drivers/view/hal.h"
#include "drivers/view/view.h"
#include "drivers/view/msg.h"
#include "drivers/view/env.h"
#include "drivers/view/render.h"
#include "drivers/view/bitmap.h"
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
    view_msg_header(&msg, VIEW_MSG_MOUSE_MOTION, VIEW_TARGET_NONE);
    view_msg_data(&msg, view_mouse.x, view_mouse.y, 0, 0);
    view_put_global_msg(&msg);
    // move mouse
    view_mouse_move_view();
}

static void mouse_wheel(int wheel)
{
    keprint("mouse wheel: %d\n", wheel);
    int id = (wheel == 0) ? VIEW_MSG_MOUSE_WHEEL_UP : VIEW_MSG_MOUSE_WHEEL_DOWN;
    view_msg_t msg;
    view_msg_header(&msg, id, VIEW_TARGET_NONE);
    view_msg_data(&msg, view_mouse.x, view_mouse.y, 0, 0);
    view_put_global_msg(&msg);
}

static void mouse_button_down(int button)
{
    //keprint("mouse button: %d down\n", button);

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
    view_msg_header(&msg, id, VIEW_TARGET_NONE);
    view_msg_data(&msg, view_mouse.x, view_mouse.y, 0, 0);
    view_put_global_msg(&msg);
}

static void mouse_button_up(int button)
{
    //keprint("mouse button: %d up\n", button);
    
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
    view_msg_header(&msg, id, VIEW_TARGET_NONE);
    view_msg_data(&msg, view_mouse.x, view_mouse.y, 0, 0);
    view_put_global_msg(&msg);
}


void view_mouse_set_state(view_mouse_state_t state)
{
    if (view_mouse.state == state)
        return;
    // keprint("mouse state %d -> %d\n", view_mouse.state, state);
    view_mouse.state = state;
    view_mouse_draw(state);
}

int view_mouse_is_state(view_mouse_state_t state)
{
    return view_mouse.state == state;
}

int view_mouse_show()
{
    if (!view_mouse.view)
        return -1;
    if (view_mouse.view->z < 0) {
        return view_move_upper_top(view_mouse.view);
    }
    return -1;
}

int view_mouse_hide()
{
    if (!view_mouse.view)
        return -1;
    if (view_mouse.view->z >= 0) {
        return view_hide(view_mouse.view);
    }
    return -1;
}

static void view_mouse_draw_default(view_t *view)
{
    // 中间白，四周黑
    view_render_vline(view, view->width / 2 + 1, 0, view->height-1, VIEW_BLACK);
    //view_render_vline(view, view->width / 2 - 1, 0, view->height-1, VIEW_BLACK);
    
    //view_render_hline(view, 0, view->height / 2 - 1, view->width-1, VIEW_BLACK);
    view_render_hline(view, 0, view->height / 2 + 1, view->width-1, VIEW_BLACK);

    view_render_vline(view, view->width / 2, 0, view->height-1, VIEW_WHITE);
    view_render_hline(view, 0, view->height / 2, view->width-1, VIEW_WHITE);
}

static void view_mouse_set_view_off(int x, int y)
{
    view_mouse.view_off_x = x;
    view_mouse.view_off_y = y;
}

void view_mouse_move_view()
{
    view_set_xy(view_mouse.view, view_mouse.x + view_mouse.view_off_x,
        view_mouse.y + view_mouse.view_off_y);
}

int view_mouse_set_state_info(view_mouse_state_info_t *info)
{
    view_mouse_state_t state = info->state;
    if (state >= VIEW_MOUSE_STATE_NR)
        return -1;
    if (!info->bmp)
        return -1;

    view_mouse_state_info_t *_info = &view_mouse.state_table[state];
    if (!_info->bmp) {
        // 没有就需要创建
        _info->bmp = view_bitmap_create(info->bmp->width, info->bmp->height);
        if (!_info->bmp)
            return -1;
    } else {
        view_bitmap_clear(_info->bmp); // 清空已有的内容
    }
    // 复制位图数据
    view_bitmap_blit(info->bmp, NULL, _info->bmp, NULL);
    _info->off_x = info->off_x;
    _info->off_y = info->off_y;
    return 0;
}

/**
 * 根据鼠标的状态绘制鼠标
 */
void view_mouse_draw(view_mouse_state_t state)
{
    if (state >= VIEW_MOUSE_STATE_NR)
        return;
    view_mouse_state_info_t *sinfo = &view_mouse.state_table[state];
    view_render_clear(view_mouse.view);
    if (sinfo->bmp) {
        // 将位图复制到视图中
        view_render_bitblt(view_mouse.view, 0, 0, sinfo->bmp, 0, 0, sinfo->bmp->width, sinfo->bmp->height);
        // 设置视图偏移位置
        view_mouse_set_view_off(sinfo->off_x, sinfo->off_y);
    } else {
        // 绘制默认视图
        view_mouse_draw_default(view_mouse.view);
        // 设置视图偏移位置
        view_mouse_set_view_off(- view_mouse.view->width / 2, - view_mouse.view->height / 2);
    }
    view_mouse_move_view();
}

int view_mouse_set_state_info_ex(view_mouse_state_info_t *info)
{
    if (view_mouse_set_state_info(info) < 0)
        return -1;
    view_mouse_draw(view_mouse.state);
    return 0;
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
    
    view_mouse.view_off_x = view_mouse.view_off_y = 0;
    view_mouse.local_x = view_mouse.local_y = 0;
    view_mouse.click_x = view_mouse.click_y = 0;
    view_mouse.state = VIEW_MOUSE_NORMAL;

    // 创建状态位图
    int i;
    for (i = 0; i < VIEW_MOUSE_STATE_NR; i++) {
        view_mouse.state_table[i].bmp = NULL;
        view_mouse.state_table[i].off_x = 0;
        view_mouse.state_table[i].off_y = 0;
    }

    /* 创建鼠标视图 */
    view_t *view = view_create(view_mouse.x, view_mouse.y, VIEW_MOUSE_SIZE, VIEW_MOUSE_SIZE, 0);
    assert(view);
    view_mouse.view = view;
    view_set_type(view, VIEW_TYPE_FIXED);

    view_mouse_draw(view_mouse.state);    // 绘制视图
    view_set_z(view, 0);    // 设置鼠标图层为0，最开始的最高图层

    // 最开始，中间图层就是鼠标图层
    view_env_set_middle(view);
    return 0;
}

int view_mouse_exit()
{
    if (view_mouse.view) {
        view_hide(view_mouse.view);
        view_destroy(view_mouse.view);
        view_mouse.view = NULL;
    }
    view_mouse_close(&view_mouse);
    return 0;
}

int view_mouse_poll()
{
    return view_mouse_read(&view_mouse);
}