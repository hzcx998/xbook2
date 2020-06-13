#include <event/msgque.h>
#include <window/window.h>
#include <sys/res.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <guisrv.h>

event_msgque_t event_msgque;

static int __open()
{
    event_msgque.msgid = res_open("guisrv-msgque", RES_IPC | IPC_MSG | IPC_CREAT | IPC_EXCL, 0);
    if (event_msgque.msgid < 0) {
        printf("[%s] open srver msgque failed!", SRV_NAME);
        return -1;
    }
    return 0;
}

static int __close()
{
    res_close(event_msgque.msgid);
    
    return 0;
}

static int __read()
{
    gui_msg_t msg;
    msg.type = 0;    // 读取第一个消息
    if (res_read(event_msgque.msgid, IPC_NOWAIT, &msg, GUI_MSG_LEN) < 0)
        return -1;
    /* 处理消息 */
    //msg.type // 窗口id
    //printf("[%s] get a message.\n", SRV_NAME);
    //printf("[%s] winid=%d, msgid=%d\n", SRV_NAME, msg.type, msg.id);
    
    if (msg.type < 0)
        return -1;

    gui_window_t *win = gui_window_cache_find(msg.type);   
    if (win == NULL) {  /* 没有在缓存中找到窗口 */
        /*  尝试在窗口链表中寻找 */
        win = gui_window_get_by_id(msg.type);
        if (win == NULL) /* 在窗口链表中也没有找到，就出错 */
            return -1;
    }

    switch (msg.id)
    {
    case 1:
        gui_window_update(win, msg.arg0, msg.arg1, msg.arg2, msg.arg3);
        break;
    default:
        break;
    }

    return 0;
}


int init_msgque_event()
{
    event_msgque.open = __open;
    event_msgque.close = __close;
    event_msgque.read = __read;

    return 0;
}