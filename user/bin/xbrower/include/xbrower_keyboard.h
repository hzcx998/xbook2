#ifndef _XGUI_KEYBOARD_H
#define _XGUI_KEYBOARD_H

#define XGUI_KMOD_SHIFT_L    0x01
#define XGUI_KMOD_SHIFT_R    0x02
#define XGUI_KMOD_SHIFT      (XGUI_KMOD_SHIFT_L | XGUI_KMOD_SHIFT_R)
#define XGUI_KMOD_CTRL_L     0x04
#define XGUI_KMOD_CTRL_R     0x08
#define XGUI_KMOD_CTRL       (XGUI_KMOD_CTRL_L | XGUI_KMOD_CTRL_R)
#define XGUI_KMOD_ALT_L      0x10
#define XGUI_KMOD_ALT_R      0x20
#define XGUI_KMOD_ALT        (XGUI_KMOD_ALT_L | XGUI_KMOD_ALT_R)
#define XGUI_KMOD_PAD	     0x40
#define XGUI_KMOD_NUM	     0x80
#define XGUI_KMOD_CAPS	     0x100

/* 图形键盘输入 */
typedef struct {
    unsigned char state;    /* 按钮状态 */
    int code;               /* 键值 */
    int modify;             /* 修饰按键 */
} keyevent_t;

typedef struct {
    int handle;
    int ledstate;                   /* 修饰按键 */
    int key_modify;                 /* 修饰按键 */
    keyevent_t keyevent;            /* 按键事件 */ 
    int (*key_down)(int);
    int (*key_up)(int);    
} xbrower_keyboard_t;
int xbrower_keyboard_read(xbrower_keyboard_t *keyboard);
int xbrower_keyboard_init();
int xbrower_keyboard_exit();
int xbrower_keyboard_poll();

#endif /* _XGUI_KEYBOARD_H */
