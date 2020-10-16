#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gapi.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <desktop.h>
#include <winctl.h>
#include <taskbar.h>
#include <icon.h>

/* 桌面在创建的时候打开shell，助于调试开发 */
#define DESKTOP_LAUNCH_BOSH

int layer_proc(g_msg_t *msg);

int main(int argc, char *argv[])
{
    printf("[desktop]: begin.\n");
    /* 初始化 */
    if (g_init() < 0)
        return -1;
    if (init_desktop() < 0) {
        g_quit();
        return -1;
    }
    if (init_taskbar() < 0) {
        g_quit();
        return -1;
    }
    if (init_icon()) {
        g_quit();
        return -1;
    }

#ifdef DESKTOP_LAUNCH_BOSH
    desktop_launch_app("/usr/bosh");
#endif
    g_msg_t msg;
    while (1)
    {
        /* 获取消息，无消息返回0，退出消息返回-1，有消息返回1 */
        if (!g_get_msg(&msg))
            continue;
        if (g_is_quit_msg(&msg))
            break;
        
        layer_proc(&msg);
    }

    /* 退出gui */
    return g_quit();
}

/**
 * 如果是自己获取/丢失了焦点，那么就调用自己获取/丢失焦点的处理。
 * 如果是某个绑定的窗口发送来的消息，就修改任务栏对应的窗口控制的状态。
 */
int layer_proc(g_msg_t *msg)
{
    int layer;
    winctl_t *winctl;
    g_point_t po;
    switch (g_msg_get_type(msg))
    {
    case GM_WINDOW_CREATE:
        layer = g_msg_get_sender(msg);
        create_winctl(layer);
        winctl_paint(NULL);
        break;
    case GM_WINDOW_CLOSE:
        layer = g_msg_get_sender(msg);
        winctl = winctl_find_by_layer(layer);
        if (winctl) {
            destroy_winctl(winctl);
            winctl_paint(NULL);
            winctl_last = NULL;
        }
        break;
    case GM_WINDOW_ICON:
        layer = g_msg_get_sender(msg);
        winctl = winctl_find_by_layer(layer);
        if (winctl) {
            winctl_set_icon(winctl);
        }
        break;
    case GM_GET_FOCUS:
        layer = g_msg_get_sender(msg);
        if (layer > 0) {
            winctl = winctl_find_by_layer(layer);
            if (winctl) {
                /* 窗口控制获得焦点 */
                winctl_get_focus(winctl);
                winctl_paint(winctl);
            }
        }
        break;
    case GM_LOST_FOCUS:
        layer = g_msg_get_sender(msg);
        if (layer > 0) {
            winctl = winctl_find_by_layer(layer);
            if (winctl) {
                /* 窗口控制丢失焦点 */
                winctl_lost_focus(winctl);
                winctl_paint(winctl);
            }
        }
        break;
    
    case GM_HIDE:
        layer = g_msg_get_sender(msg);
        if (layer > 0) {
            winctl = winctl_find_by_layer(layer);
            if (winctl) {
                winctl->ishidden = true;
                winctl_paint(winctl);
            }
        }
        break;
    case GM_SHOW:
        layer = g_msg_get_sender(msg);
        if (layer > 0) {
            winctl = winctl_find_by_layer(layer);
            if (winctl) {
                winctl->ishidden = false;
                winctl_paint(winctl);
            }
        }
        break;
    case GM_MOUSE_MOTION:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_check_touch_state_group(&taskbar.touch_list, &po);
        g_check_touch_state_group(&icon_man.touch_list, &po);
        
        break;
    case GM_MOUSE_LBTN_DOWN:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_check_touch_click_group(&taskbar.touch_list, &po, 0);
        g_check_touch_click_group(&icon_man.touch_list, &po, 0);
        
        break;
    case GM_MOUSE_LBTN_UP:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_check_touch_click_group(&taskbar.touch_list, &po, 1);
        g_check_touch_click_group(&icon_man.touch_list, &po, 1);
        break;
    case GM_MOUSE_RBTN_DOWN:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_check_touch_click_group(&taskbar.touch_list, &po, 4);
        break;
    case GM_MOUSE_RBTN_UP:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_check_touch_click_group(&taskbar.touch_list, &po, 5);
        break;
    default:
        break;
    }
    return 0;
}
