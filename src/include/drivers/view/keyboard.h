#ifndef _XBOOK_DRIVERS_VIEW_KEYBOARD_H
#define _XBOOK_DRIVERS_VIEW_KEYBOARD_H

#define VIEW_KMOD_SHIFT_L    0x01
#define VIEW_KMOD_SHIFT_R    0x02
#define VIEW_KMOD_SHIFT      (VIEW_KMOD_SHIFT_L | VIEW_KMOD_SHIFT_R)
#define VIEW_KMOD_CTRL_L     0x04
#define VIEW_KMOD_CTRL_R     0x08
#define VIEW_KMOD_CTRL       (VIEW_KMOD_CTRL_L | VIEW_KMOD_CTRL_R)
#define VIEW_KMOD_ALT_L      0x10
#define VIEW_KMOD_ALT_R      0x20
#define VIEW_KMOD_ALT        (VIEW_KMOD_ALT_L | VIEW_KMOD_ALT_R)
#define VIEW_KMOD_PAD	     0x40
#define VIEW_KMOD_NUM	     0x80
#define VIEW_KMOD_CAPS	     0x100

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
} view_keyboard_t;
int view_keyboard_read(view_keyboard_t *keyboard);
int view_keyboard_init();
int view_keyboard_exit();
int view_keyboard_poll();

#endif /* _XBOOK_DRIVERS_VIEW_KEYBOARD_H */
