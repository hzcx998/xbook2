#include "drivers/view/hal.h"
#include "drivers/view/view.h"
#include "drivers/view/core.h"
#include "drivers/view/msg.h"
#include "drivers/view/env.h"
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <assert.h>

static task_t *view_thread;
static int view_thread_exit;

static void view_thread_entry(void *arg)
{
    view_core_loop();
}

int view_core_init()
{
    view_thread_exit = 0;
    view_thread = NULL;
    if (view_section_init() < 0) {
        return -1;
    }
    if (view_init() < 0) {
        view_mouse_exit();        
        return -1;
    }
    if (view_mouse_init() < 0) {
        view_mouse_exit();
        view_keyboard_exit();
        return -1;
    }
    if (view_keyboard_init() < 0) {
        view_mouse_exit();
        view_keyboard_exit();
        view_section_exit();
        return -1;
    }
    if (view_global_msg_init() < 0) {
        view_mouse_exit();
        view_keyboard_exit();
        view_section_exit();
        view_keyboard_exit();
        return -1;
    }
    if (view_env_init() < 0) {
        view_global_msg_exit();
        view_mouse_exit();
        view_keyboard_exit();
        view_section_exit();
        view_keyboard_exit();
        return -1;
    }
    view_thread = kern_thread_start("driver-view", TASK_PRIO_LEVEL_NORMAL, view_thread_entry, NULL);
    if (!view_thread) {
        view_env_exit();
        view_global_msg_exit();
        view_keyboard_exit();
        view_mouse_exit();
        view_exit();
        view_section_exit();
        return -1;
    }
    return 0;
}

int view_core_exit()
{
    /* 先停掉线程,等待线程退出 */
    view_thread_exit = 1;
    while (view_thread_exit)
        task_yeild();
    view_thread = NULL;
    
    view_env_exit();
    view_global_msg_exit();
    view_keyboard_exit();
    view_mouse_exit();
    view_exit();
    view_section_exit();
    return 0;
}

/* 输入的获取 */
void view_core_loop()
{
    view_msg_t msg;
    while (!view_thread_exit) {        
        view_mouse_poll();
        view_keyboard_poll();
        view_msg_reset(&msg);
        if (view_get_global_msg(&msg) < 0) {
            task_yeild();
            continue;
        }
        if (is_view_msg_valid(&msg)) {
            /* 处理消息 */
            switch (view_msg_get_id(&msg)) {
            case VIEW_MSG_KEY_DOWN:
            case VIEW_MSG_KEY_UP:
                /* 键盘消息发送到聚焦的图层 */
                view_dispatch_keycode_msg(&msg);
                break;
            case VIEW_MSG_MOUSE_MOTION:
            case VIEW_MSG_MOUSE_LBTN_DOWN:
            case VIEW_MSG_MOUSE_LBTN_UP:
            case VIEW_MSG_MOUSE_LBTN_DBLCLK:
            case VIEW_MSG_MOUSE_RBTN_DOWN:
            case VIEW_MSG_MOUSE_RBTN_UP:
            case VIEW_MSG_MOUSE_RBTN_DBLCLK:
            case VIEW_MSG_MOUSE_MBTN_DOWN:
            case VIEW_MSG_MOUSE_MBTN_UP:
            case VIEW_MSG_MOUSE_MBTN_DBLCLK:
            case VIEW_MSG_MOUSE_WHEEL_UP:
            case VIEW_MSG_MOUSE_WHEEL_DOWN:
            case VIEW_MSG_MOUSE_WHEEL_LEFT:
            case VIEW_MSG_MOUSE_WHEEL_RIGHT:
                /* 鼠标消息发送到鼠标指针所在的图层 */
                view_dispatch_mouse_msg(&msg);
                break;
            default:
                /* 默认派发方式，发送消息给指定的目标 */
                view_dispatch_target_msg(&msg);
                break;
            }
        }
    }
    view_thread_exit = 0;
    kern_thread_exit(0);
}
