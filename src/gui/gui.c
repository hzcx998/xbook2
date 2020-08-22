#include <gui/screen.h>
#include <gui/keyboard.h>
#include <gui/mouse.h>
#include <gui/event.h>
#include <gui/font.h>
#include <gui/text.h>
#include <gui/rect.h>
#include <gui/console/console.h>
#include <gui/layer.h>
#include <gui/message.h>

#include <xbook/debug.h>
#include <xbook/gui.h>
#include <xbook/task.h>
#include <string.h>

/* 主要是处理输入事件 */
void kgui_thread(void *arg)
{
    //con_loop();
    g_msg_t msg;
    memset(&msg, 0, sizeof(g_msg_t));
    while (1)
    {
        /* 读取事件 */
        gui_keyboard.read();
        gui_mouse.read();
        
        /* 获取系统消息 */
        if (gui_pop_msg(&msg) < 0) {
            continue;
        }
        switch (msg.id)
        {
        case GM_KEY_DOWN:
        case GM_KEY_UP:
            /* 键盘消息发送到聚焦的图层 */
            gui_dispatch_key_msg(&msg);
            break;
        case GM_MOUSE_MOTION:
        case GM_MOUSE_LBTN_DOWN:
        case GM_MOUSE_LBTN_UP:
        case GM_MOUSE_LBTN_DBLCLK:
        case GM_MOUSE_RBTN_DOWN:
        case GM_MOUSE_RBTN_UP:
        case GM_MOUSE_RBTN_DBLCLK:
        case GM_MOUSE_MBTN_DOWN:
        case GM_MOUSE_MBTN_UP:
        case GM_MOUSE_MBTN_DBLCLK:
        case GM_MOUSE_WHEEL:
            /* 鼠标消息发送到鼠标指针所在的图层 */
            gui_dispatch_mouse_msg(&msg);
            break;
        default:
            /* 默认派发方式，发送消息给指定的目标 */
            gui_dispatch_target_msg(&msg);
            break;
        }
        /* 根据图层信息选择对应的鼠标图层，键盘图层，并发送消息 */
        /*printk("msg: target=%d id=%x data0=%x data1=%x data2=%x data3=%x\n", 
            msg.target, msg.id, msg.data0, msg.data1, msg.data2, msg.data3);
        */
    }
}

void init_gui()
{
    if (gui_init_screen() < 0)
        panic("init gui screen failed!\n");
    if (gui_init_keyboard() < 0)
        panic("init gui keyboard failed!\n");
    if (gui_init_mouse() < 0)
        panic("init gui keyboard failed!\n");

    if (gui_init_event() < 0)
        panic("init gui event failed!\n");

    if (gui_init_msg() < 0)
        panic("init gui msg failed!\n");
    
    gui_init_font();

    if (gui_screen.open() < 0)
        panic("open gui screen failed!\n");
    if (gui_keyboard.open() < 0)
        panic("open gui keyboard failed!\n");
    if (gui_mouse.open() < 0)
        panic("open gui keyboard failed!\n");

    if (gui_init_layer() < 0)
        panic("init gui layer failed!\n");

    /*
    if (gui_init_console() < 0)
        panic("init gui console failed!\n");
    */

    /* 启动gui线程 */
    if (kthread_start("kgui", TASK_PRIO_USER, kgui_thread, NULL) == NULL)
        panic("start kgui thread failed!\n");
}

int sys_g_init(void)
{
    task_t *cur = current_task;
    if (cur->gmsgpool)
        return -1;
    cur->gmsgpool = msgpool_create(sizeof(g_msg_t), GUI_MSG_NR);
    if (!cur->gmsgpool)
        return -1;  /* create failed! */
    
    return 0;
}

int sys_g_quit(void)
{
    task_t *cur = current_task;
    if (cur->gmsgpool) {
        msgpool_destroy(cur->gmsgpool);
        cur->gmsgpool = NULL;
    }

    return 0;
}