#ifndef __GUISRV_EVENT_MSGQUE_H__
#define __GUISRV_EVENT_MSGQUE_H__

#include <sgi/sgim.h>

#define gui_msg_t SGI_Msg
#define GUI_MSG_LEN SGI_MSG_LEN

typedef struct _event_msgque {
    int msgid;                  /* 消息队列id */
    int (*open) ();
    int (*close) ();
    int (*read) ();
} event_msgque_t;

extern event_msgque_t event_msgque;

int init_msgque_event();

#endif  /* __GUISRV_EVENT_MSGQUE_H__ */