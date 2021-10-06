#ifndef _DWIN_MESSAGE_H
#define _DWIN_MESSAGE_H

#include <dwin/dwin_config.h>

enum {
    DWM_NONE = 0,
    DWM_KEY_DOWN,
    DWM_KEY_UP,
    DWM_MOUSE_MOTION,
    DWM_MOUSE_LBTN_DOWN,
    DWM_MOUSE_LBTN_UP,
    DWM_MOUSE_LBTN_DBLCLK,
    DWM_MOUSE_RBTN_DOWN,
    DWM_MOUSE_RBTN_UP,
    DWM_MOUSE_RBTN_DBLCLK,
    DWM_MOUSE_MBTN_DOWN,
    DWM_MOUSE_MBTN_UP,
    DWM_MOUSE_MBTN_DBLCLK,
    DWM_MOUSE_WHEEL_UP,
    DWM_MOUSE_WHEEL_DOWN,
    DWM_MOUSE_WHEEL_LEFT,
    DWM_MOUSE_WHEEL_RIGHT,
    DWM_TIMER,
    DWM_QUIT,
    DWM_ENTER,
    DWM_LEAVE,
    DWM_RESIZE,
    DWM_ACTIVATE,
    DWM_INACTIVATE,
    DWM_MOVE,
    DWM_CREATE,
    DWM_CLOSE,
    DWM_HIDE,
    DWM_SHOW,
    DWM_PAINT,
    DWM_NR,
};

#define DWIN_MSG_DATA_NR    4

struct dwin_message
{
    uint32_t mid;   /* message id */
    uint32_t lid;   /* layer id */
    uint32_t data[DWIN_MSG_DATA_NR];
};
typedef struct dwin_message dwin_message_t;

static inline void dwin_message_head(dwin_message_t *msg, uint32_t mid, uint32_t lid)
{
    msg->mid = mid;
    msg->lid = lid;
}

static inline void dwin_message_body(dwin_message_t *msg, uint32_t d0, uint32_t d1,
        uint32_t d2, uint32_t d3)
{
    msg->data[0] = d0;
    msg->data[1] = d1;
    msg->data[2] = d2;
    msg->data[3] = d3;
}

static inline void dwin_message_zero(dwin_message_t *msg)
{
    msg->mid = 0;
    msg->lid = 0;
    int i;
    for (i = 0; i < DWIN_MSG_DATA_NR; i++)
    {
        msg->data[i] = 0;
    }
}


#endif   /* _DWIN_MESSAGE_H */
