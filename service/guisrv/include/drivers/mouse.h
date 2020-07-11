#ifndef __GUISRV_DRIVER_MOUSE_H__
#define __GUISRV_DRIVER_MOUSE_H__

typedef struct _drv_mouse {
    int            (*open)(void);
    int	           (*close)(void);
    int	           (*read)(void);
} drv_mouse_t;

extern drv_mouse_t drv_mouse;

int init_mouse_driver();

#endif  /* __GUISRV_DRIVER_MOUSE_H__ */
