#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gapi.h>
#include "desktop.h"

int main(int argc, char *argv[])
{
    printf("[desktop]: begin.\n");
    /* 初始化 */
    if (g_init() < 0)
        return -1;

    if (init_desktop() < 0) {
        return -1;
    }
    if (init_taskbar() < 0) {
        return -1;
    }
    
    int win = g_new_window("hello", 100, 200, 400, 300);
    if (win < 0)
        printf("new win failed!\n");

    /* 设置窗口界面 */

    g_show_window(win);

    int win2 = g_new_window("hello2", 200, 300, 300, 200);
    if (win2 < 0)
        printf("new win failed!\n");

    /* 设置窗口界面 */

    g_show_window(win2);

    /* 注册消息回调函数 */
    //g_set_routine(win_proc);
    
    g_msg_t msg;
    while (1)
    {
        /* 获取消息，一般消息返回0，退出消息返回-1 */
        if (g_get_msg(&msg) < 0)
            continue;
            
        printf("umsg: target=%d id=%x data0=%x data1=%x data2=%x data3=%x\n", 
            msg.target, msg.id, msg.data0, msg.data1, msg.data2, msg.data3);
        
        /*if (g_is_quit_msg(&msg))
            break;*/
        /* 有外部消息则处理消息 */
        g_dispatch_msg(&msg);
    }
    g_quit();   /* 退出gui */
    return 0;    
}
