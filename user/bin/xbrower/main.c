#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/lpc.h>
#include <sys/proc.h>
#include <xgui_core.h>
#include <xgui_view.h>
#include <xgui_bitmap.h>
#include <xgui_vrender.h>
#include <xgui_image.h>
#include <xgui_screen.h>

#include "lvgl_window.h"
#include  "lv_examples.h"

void *lvgl_window_thread()
{
    if (lv_window_init("lvgl", 0,0, xgui_screen.width, xgui_screen.height) < 0)
        return (void *) -1;
    
    #if LV_USE_DEMO_WIDGETS
    lv_demo_widgets();
    #endif
    #if LV_USE_DEMO_PRINTER
    lv_demo_printer();
    #endif
    #if LV_USE_DEMO_KEYPAD_AND_ENCODER
    lv_demo_keypad_encoder();
    #endif

    lv_window_loop();
    return NULL;
}

int main(int argc, char *argv[])
{
    printf("hello xgui!\n");
    xgui_init();
    
    pthread_t th;
    pthread_create(&th, NULL, lvgl_window_thread, NULL);
    xgui_loop();
    return 0;
}

/*
客户端进程作为独立的个体，管理客户端的图形交互，
管理消息，并分发给客户端的各个子窗口。

维护客户端方法：设定一个定时器，超时后，检测

服务端function，执行一些服务功能：
    图形合并、事件分发
服务端接口：负责和客户端的接口信息

客户端function，再客户端执行一些服务功能：
    图形绘制，事件处理
客户端接口：负责和服务端绑定通信信息。


客户端：
xgui_client_t: 用于保存整个客户端的信息
xgui_client_view_t: 用于保存客户端视图的信息
xgui_client_event_t: 用于保存客户端事件信息

接口设计：
xgui_client_open: 打开客户端，和服务端绑定一些基础信息
xgui_client_close: 关闭客户端，结束各种信息的绑定

xgui_client_view_xxx: 对视图的操作
xgui_client_event_xxx: 对事件的操作



*/
