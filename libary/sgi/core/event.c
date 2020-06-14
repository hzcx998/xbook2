#include <sgi/sgi.h>
#include <sgi/sgie.h>
#include <sgi/sgii.h>
#include <sys/res.h>
#include <sys/ipc.h>
#include <string.h>
#include <stdio.h>

int SGI_NextEvent(SGI_Display *display, SGI_Event *event)
{
    if (!display)
        return -1;
    
    if (!display->connected)
        return -1;
    memset(event, 0, sizeof(*event));
    if (display->event_msgid >= 0) {
        SGI_EventMsg msg;
        msg.type = 0;
        /* 接收一个消息 */
        if (res_read(display->event_msgid, 0, &msg, sizeof(SGI_Event)) < 0) {
            return -1;
        }
        /* 接收事件的窗口 */
        display->event_window = SGI_DisplayWindowHandleFindByWinid(display, msg.type);
        if (display->event_window < 0)  /* 正确的事件窗口 */
            return -1;
        /* 复制消息 */
        *event = msg.event;
        return 0;
    }
    return -1;
}

int SGI_PollEvent(SGI_Display *display, SGI_Event *event)
{
    if (!display)
        return -1;
    
    if (!display->connected)
        return -1;
    memset(event, 0, sizeof(*event));
    if (display->event_msgid >= 0) {
        SGI_EventMsg msg;
        msg.type = 0;
        /* 接收一个消息，如果没有消息就返回，不阻塞 */
        if (res_read(display->event_msgid, IPC_NOWAIT, &msg, sizeof(SGI_Event)) < 0) {
            return -1;
        }
        /* 接收事件的窗口 */
        display->event_window = SGI_DisplayWindowHandleFindByWinid(display, msg.type);
        if (display->event_window < 0)  /* 正确的事件窗口 */
            return -1;
        /* 复制消息 */
        *event = msg.event;
        return 0;
    }
    return -1;
}


