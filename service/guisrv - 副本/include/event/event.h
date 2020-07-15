#ifndef __GUISRV_EVENT_H__
#define __GUISRV_EVENT_H__

/* 图形事件状态 */
typedef enum _gui_event_state  {
    GUI_NOSTATE = 0,        /* 无状态 */
    GUI_PRESSED,            /* 按下状态 */
    GUI_RELEASED,           /* 释放状态 */
    GUI_ENTER,              /* 进入状态 */
    GUI_LEAVE,              /* 离开状态 */
} gui_event_state;

/* 图形键盘输入 */
typedef struct _gui_keyboard_event {
    unsigned char type;                /* 键盘事件类型：gui_KEY_DOWN, gui_KEY_UP */
    unsigned char state;               /* 按钮状态 */
    unsigned int code;
    unsigned int modify;
} gui_keyboard_event;

/* 图形鼠标移动 */
typedef struct _gui_mouse_motion_event {
    unsigned char type;                 /* 鼠标移动事件类型：gui_MOUSE_MOTION */
    unsigned char state;                /* 移动状态： */
    int x;                              /* x横坐标 */
    int y;                              /* y纵坐标 */
} gui_mouse_motion_event;

/* 图形鼠标按扭 */
typedef struct _gui_mouse_button_event {
    unsigned char type;                 /* 鼠标按钮事件类型：gui_MOUSE_DOWN, gui_MOUSE_UP */
    unsigned char state;                /* 按钮状态 */
    unsigned char button;               /* 按钮值 */
    int x;                              /* x横坐标 */
    int y;                              /* y纵坐标 */
} gui_mouse_button_event;

/* 图形的事件类型 */
typedef enum _gui_event_type {
    GUI_NOEVENT = 0,        /* 未使用 */
    GUI_EVENT_KEY,                /* 按键事件 */
    GUI_EVENT_MOUSE_MOTION,       /* 鼠标移动 */
    GUI_EVENT_MOUSE_BUTTON,       /* 鼠标按钮事件 */
    GUI_EVENT_QUIT,               /* 退出事件 */
    GUI_EVENT_NR,           /* 最大的事件数量 */
} gui_event_type;

typedef union _gui_event {
    unsigned char type;    /* 输入类型，由于下面每一个成员的第一个成员都是type，所以次type就是该成员的type */
    gui_keyboard_event key;
    gui_mouse_motion_event motion;
    gui_mouse_button_event button; 
} gui_event;

/* 事件数量 */
#define GUI_EVENT_NR    32

#define GUI_EVENT_POOL_EMPTY(pool) \
        ((pool)->head == (pool)->tail)

/* 由于事件池是在单个进程中使用，所以不需要互斥 */
typedef struct {
    gui_event events[GUI_EVENT_NR];
    int head;   /* 头位置 */
    int tail;   /* 尾位置 */
} gui_event_pool;

int gui_event_add(gui_event *ev);
int gui_event_del(gui_event *ev);
int gui_event_poll(gui_event *ev);
int init_event();

#endif  /* __GUISRV_EVENT_H__ */
