#ifndef __SGI_EventT_H__    /* Eventt */
#define __SGI_EventT_H__

#include "sgik.h"

/* 图形事件状态 */
typedef enum _SGI_EventState  {
    SGI_PRESSED   = 1,      /* 按下状态 */
    SGI_RELEASED,           /* 释放状态 */
    SGI_ENTER,              /* 进入状态 */
    SGI_LEAVE,              /* 离开状态 */
} SGI_EventState;

/* 图形键盘输入 */
typedef struct _SGI_KeyboardEvent {
    unsigned char type;                /* 键盘事件类型：SGI_KEY_DOWN, SGI_KEY_UP */
    unsigned char state;               /* 按钮状态 */
    SGI_KeyInfo keycode;    /* 按键信息 */
} SGI_KeyboardEvent;

/* 图形鼠标移动 */
typedef struct _SGI_MouseMotionEvent {
    unsigned char type;                 /* 鼠标移动事件类型：SGI_MOUSE_MOTION */
    unsigned char state;                /* 移动状态： */
    int x;                              /* x横坐标 */
    int y;                              /* y纵坐标 */
} SGI_MouseMotionEvent;

/* 图形鼠标按扭 */
typedef struct _SGI_MouseButtonEvent {
    unsigned char type;                 /* 鼠标按钮事件类型：SGI_MOUSE_DOWN, SGI_MOUSE_UP */
    unsigned char state;                /* 按钮状态 */
    unsigned char button;               /* 按钮值 */
    int x;                              /* x横坐标 */
    int y;                              /* y纵坐标 */
} SGI_MouseButtonEvent;

/* 图形的事件类型 */
typedef enum _SGI_EventType {
    SGI_NOEVENT = 0,        /* 未使用 */
    SGI_KEY,                /* 按键事件 */
    SGI_MOUSE_MOTION,       /* 鼠标移动 */
    SGI_MOUSE_BUTTON,       /* 鼠标按钮事件 */
    SGI_QUIT,               /* 退出事件 */
    SGI_EVENT_NR,           /* 最大的事件数量 */
} SGI_EventType;

/* 图形输入
一个联合结构
 */
typedef union _SGI_Event {
    unsigned char type;    /* 输入类型，由于下面每一个成员的第一个成员都是type，所以次type就是该成员的type */
    SGI_KeyboardEvent key;
    SGI_MouseMotionEvent motion;
    SGI_MouseButtonEvent button; 
} SGI_Event;

typedef struct _SGI_EventMsg {
    long type;                  /* 消息队列固定格式 */
    SGI_Event event;
} SGI_EventMsg;

/* 窗口可以接收的事件
鼠标点击，鼠标弹起，鼠标移动
按键按下，按键弹起
鼠标进入窗口，鼠标离开窗口
 */
enum SGI_EventMask {
    SGI_ButtonPressMask     = 0x01,
    SGI_ButtonRleaseMask    = 0x02,
    SGI_PointerMotionMask   = 0x04,
    SGI_KeyPressMask        = 0x08,
    SGI_KeyRleaseMask       = 0x10,
    SGI_EnterWindow         = 0x20,
    SGI_LeaveWindow         = 0x40,
    SGI_EventMaskDefault    = (SGI_ButtonPressMask | SGI_ButtonRleaseMask |
                            SGI_KeyPressMask |  SGI_KeyRleaseMask | SGI_EnterWindow |
                            SGI_LeaveWindow),
};

#endif  /* __SGI_EventT_H__ */