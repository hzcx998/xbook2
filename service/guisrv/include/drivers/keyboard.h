#ifndef __GUISRV_DRIVER_KEYBOARD_H__
#define __GUISRV_DRIVER_KEYBOARD_H__

typedef struct _gui_keyboard {
    
    int            (*open)(void);
    int	           (*close)(void);
    int	           (*read)(void);
    
} gui_keyboard_t;

extern gui_keyboard_t keyboard;

int init_keyboard_driver();



#endif  /* __GUISRV_DRIVER_KEYBOARD_H__ */
