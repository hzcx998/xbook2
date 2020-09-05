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
//#define DESKTOP_LAUNCH_BOSH

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

#if 0
    /* 声音测试 */
    int beep = open("buzzer", O_DEVEX, 0);
    if (beep < 0) {
        printf("[desktop]: open buzzer failed!\n");
        return -1;
    }
    ioctl(beep, SNDIO_PLAY, 0);
    int i;
    for (i = 20; i < 20000; i+=5) {
        ioctl(beep, SNDIO_SETFREQ, i);
        mdelay(10);
    }
    ioctl(beep, SNDIO_STOP, 0);
#endif
#if 0    
    int win = g_new_window("hello", 100, 200, 400, 300);
    if (win < 0)
        printf("new win failed!\n");

    g_enable_window_resize(win, 200, 100);
    /* 设置窗口界面 */
    
    g_show_window(win);

    int win2 = g_new_window("hello2", 200, 300, 300, 200);
    if (win2 < 0)
        printf("new win failed!\n");

    /* 设置窗口界面 */

    g_show_window(win2);
#endif

    /* 注册消息回调函数 */
    g_set_msg_routine(layer_proc);

#ifdef DESKTOP_LAUNCH_BOSH
    desktop_launch_app("bosh");
#endif
    g_msg_t msg;
    while (1)
    {
        /* 获取消息，一般消息返回0，退出消息返回-1 */
        if (g_get_msg(&msg) < 0)
            continue;
        /*
        printf("umsg: target=%d id=%x data0=%x data1=%x data2=%x data3=%x\n", 
            msg.target, msg.id, msg.data0, msg.data1, msg.data2, msg.data3);
        */
        /* NOTE: no exit in desktop
        if (g_is_quit_msg(&msg))
            break;
        */
        /* 有外部消息则处理消息，dispatch只会处理窗口的消息，对于图层的不会过滤 */
        g_dispatch_msg(&msg);
    }

    /* 退出gui */
    return g_quit();
}

/**
 * 如果是自己获取/丢失了焦点，那么就调用自己获取/丢失焦点的处理。
 * 如果是某个绑定的窗口发送来的消息，就修改任务栏对应的窗口控制的状态。
 * 
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
            winctl_last = NULL; /* 有窗口关闭的时候需要清空 */
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
        if (layer > 0) {    /* 某个图层获得了焦点 */
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
        if (layer > 0) {    /* 某个图层丢失了焦点 */
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
        if (layer > 0) {    /* 某个图层获得了焦点 */
            winctl = winctl_find_by_layer(layer);
            if (winctl) {
                winctl->ishidden = true;
                winctl_paint(winctl);
            }
        }
        break;
    case GM_SHOW:
        layer = g_msg_get_sender(msg);
        if (layer > 0) {    /* 某个图层获得了焦点 */
            winctl = winctl_find_by_layer(layer);
            if (winctl) {
                winctl->ishidden = false;
                winctl_paint(winctl);
            }
        }
        break;
    case GM_MOUSE_MOTION:
        /* 触摸状态改变 */
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_touch_state_check_group(&taskbar.touch_list, &po);
        g_touch_state_check_group(&icon_man.touch_list, &po);
        
        break;
    /* 标题栏的按钮处理 */
    case GM_MOUSE_LBTN_DOWN:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_touch_click_check_group(&taskbar.touch_list, &po, 0);
        g_touch_click_check_group(&icon_man.touch_list, &po, 0);
        
        break;
    case GM_MOUSE_LBTN_UP:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_touch_click_check_group(&taskbar.touch_list, &po, 1);
        g_touch_click_check_group(&icon_man.touch_list, &po, 1);
        break;
    /* 标题栏的按钮处理 */
    case GM_MOUSE_RBTN_DOWN:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_touch_click_check_group(&taskbar.touch_list, &po, 4);
        break;
    case GM_MOUSE_RBTN_UP:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_touch_click_check_group(&taskbar.touch_list, &po, 5);
        break;
    default:
        break;
    }
    return 0;
}
