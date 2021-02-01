#ifndef _LIB_XTK_EVENT_H
#define _LIB_XTK_EVENT_H

#include <stdint.h>
#include <uview_keycode.h>

enum {
    XTK_PRESSED   = 1,    /* 按下状态 */
    XTK_RELEASED,         /* 释放状态 */
};

typedef struct {
    int code;           /* 键值 */
    int modify;         /* 修饰按键 */
} xtk_key_info_t;

/* 图形键盘输入 */
typedef struct {
    uint8_t type;                /* 键盘事件类型：XTK_KEY_DOWN, XTK_KEY_UP */
    uint8_t state;               /* 按钮状态 */
    xtk_key_info_t keycode;      /* 按键信息 */
} xtk_key_event_t;

typedef struct {
    uint8_t type;                /* 鼠标移动事件类型：XTK_MOUSE_MOTION */
    int32_t x;              /* x偏移 */
    int32_t y;              /* y偏移 */
} xtk_mouse_motion_even_t;

enum {
    XTK_WHEEL_UP   = 1,
    XTK_WHEEL_DOWN, 
    XTK_WHEEL_LEFT,
    XTK_WHEEL_RIGHT,
};

typedef struct {
    uint8_t type;                /* 鼠标移动事件类型：XTK_MOUSE_MOTION */
    int32_t wheel;          /* 滚轮方向 */ 
    int32_t x;              /* x偏移 */
    int32_t y;              /* y偏移 */
} xtk_mouse_wheel_even_t;

typedef struct {
    uint8_t type;                /* 鼠标按钮事件类型：XTK_MOUSE_DOWN, XTK_MOUSE_UP */
    uint8_t state;               /* 按钮状态 */
    uint8_t button;              /* 按钮值 */
    int32_t x;                  /* x偏移 */
    int32_t y;                  /* y偏移 */
} xtk_mouse_button_even_t;

/* 图形的事件类型 */
typedef enum {
    XTK_NOEVENT = 0,      /* 未使用 */
    XTK_KEY_DOWN,         /* 按键按下 */
    XTK_KEY_UP,           /* 按键弹起 */
    XTK_MOUSE_MOTION,     /* 鼠标移动 */
    XTK_MOUSE_BUTTON_DOWN,/* 鼠标按钮按下 */
    XTK_MOUSE_BUTTON_UP,  /* 鼠标按钮弹起 */
    XTK_MOUSE_WHEEL,   /* 鼠标滚轮 */
    XTK_TIMER_EVEN,       /* 时钟事件 */
    MAX_XTK_EVENT_NR,     /* 最大的事件数量 */
} xtk_event_type_t;

typedef union {
    uint8_t type;    /* 输入类型，由于下面每一个成员的第一个成员都是type，所以此type就是该成员的type */
    xtk_key_event_t key;
    xtk_mouse_motion_even_t motion;
    xtk_mouse_button_even_t button;
    xtk_mouse_wheel_even_t wheel;
} xtk_event_t;

#endif /* _LIB_XTK_EVENT_H */