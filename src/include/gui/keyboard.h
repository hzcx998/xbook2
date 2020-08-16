#ifndef __GUISRV_DRIVER_KEYBOARD_H__
#define __GUISRV_DRIVER_KEYBOARD_H__

#define GUI_KMOD_SHIFT_L    0x01
#define GUI_KMOD_SHIFT_R    0x02
#define GUI_KMOD_SHIFT      (GUI_KMOD_SHIFT_L | GUI_KMOD_SHIFT_R)
#define GUI_KMOD_CTRL_L     0x04
#define GUI_KMOD_CTRL_R     0x08
#define GUI_KMOD_CTRL       (GUI_KMOD_CTRL_L | GUI_KMOD_CTRL_R)
#define GUI_KMOD_ALT_L      0x10
#define GUI_KMOD_ALT_R      0x20
#define GUI_KMOD_ALT        (GUI_KMOD_ALT_L | GUI_KMOD_ALT_R)
#define GUI_KMOD_PAD	    0x40
#define GUI_KMOD_NUM	    0x80
#define GUI_KMOD_CAPS	    0x100

/* 图形键盘输入 */
typedef struct _keyevent {
    unsigned char state;    /* 按钮状态 */
    int code;               /* 键值 */
    int modify;             /* 修饰按键 */
} keyevent_t;

typedef struct _gui_keyboard {
    int            (*open)(void);
    int	           (*close)(void);
    int	           (*read)(void);
    int ledstate;                 /* 修饰按键 */
    int key_modify;                 /* 修饰按键 */
    keyevent_t keyevent;            /* 按键事件 */      
} gui_keyboard_t;

extern gui_keyboard_t gui_keyboard;

int gui_init_keyboard();

int gui_key_pressed(int keycode);
int gui_key_released(int keycode);

#endif  /* __GUISRV_DRIVER_KEYBOARD_H__ */
