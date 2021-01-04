#ifndef _LIB_VIEW_MSG_H
#define _LIB_VIEW_MSG_H

#include <stdint.h>

/* view message */
enum {
    VIEW_MSG_NONE = 0,
    VIEW_MSG_KEY_DOWN,
    VIEW_MSG_KEY_UP,
    VIEW_MSG_MOUSE_MOTION,
    VIEW_MSG_MOUSE_LBTN_DOWN,
    VIEW_MSG_MOUSE_LBTN_UP,
    VIEW_MSG_MOUSE_LBTN_DBLCLK,
    VIEW_MSG_MOUSE_RBTN_DOWN,
    VIEW_MSG_MOUSE_RBTN_UP,
    VIEW_MSG_MOUSE_RBTN_DBLCLK,
    VIEW_MSG_MOUSE_MBTN_DOWN,
    VIEW_MSG_MOUSE_MBTN_UP,
    VIEW_MSG_MOUSE_MBTN_DBLCLK,
    VIEW_MSG_MOUSE_WHEEL_UP,
    VIEW_MSG_MOUSE_WHEEL_DOWN,
    VIEW_MSG_MOUSE_WHEEL_LEFT,
    VIEW_MSG_MOUSE_WHEEL_RIGHT,
    VIEW_MSG_TIMER,
    VIEW_MSG_QUIT,
    VIEW_MSG_ENTER,
    VIEW_MSG_LEAVE,
    VIEW_MSG_RESIZE,
    VIEW_MSG_ACTIVATE,
    VIEW_MSG_INACTIVATE,
    VIEW_MSG_MOVE,
    VIEW_MSG_CREATE,
    VIEW_MSG_CLOSE,
    VIEW_MSG_HIDE,
    VIEW_MSG_SHOW,
    VIEW_MSG_PAINT,
    VIEW_MSG_NR,
};

typedef struct {
    uint32_t id;        /* 消息id */
    int target;         /* 消息目标 */
    uint32_t data0;     
    uint32_t data1;     
    uint32_t data2;     
    uint32_t data3;
} view_msg_t;

#endif  /* _LIB_VIEW_MSG_H */