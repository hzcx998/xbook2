#ifndef __GUISRV_DRIVER_KEYBOARD_H__
#define __GUISRV_DRIVER_KEYBOARD_H__

typedef struct _gui_keyboard {
    int            (*open)(void);
    int	           (*close)(void);
    int	           (*read)(void);
    int ledstate;                 /* 修饰按键 */
} drv_keyboard_t;

extern drv_keyboard_t drv_keyboard;

int init_keyboard_driver();


#endif  /* __GUISRV_DRIVER_KEYBOARD_H__ */
