#include <environment/interface.h>
#include <window/event.h>
#include <stdio.h>
#include <guisrv.h>
#include <sys/res.h>
#include <sys/ipc.h>

int gui_window_send_event(gui_window_t *window, gui_event_t *event)
{
    //printf("[%s] send event to window.\n", SRV_NAME);
    if (window->display_id > 0) {   /* 有显示id */
        env_display_t *disp = env_display_find(window->display_id);
        if (disp) {
            gui_event_msg_t msg;
            msg.type = window->id;  // 窗口id
            msg.event = *event;
            /* 往消息队列发送消息 */
            if (res_write(disp->msgid, IPC_NOWAIT, &msg, sizeof(gui_event_t)) < 0)
                return -1;
            
            return 0;
        }
    } else {    /* 没有显示id */

    }
    return -1;
}
