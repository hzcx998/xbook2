#ifndef __GUISRV_DRIVER_MOUSE_H__
#define __GUISRV_DRIVER_MOUSE_H__

#include <gui_color.h>

typedef struct _gui_mouse {
    int            (*open)(void);
    int	           (*close)(void);
    int	           (*read)(void);
    
} gui_mouse_t;

extern gui_mouse_t mouse;

int init_mouse_driver();


#endif  /* __GUISRV_DRIVER_MOUSE_H__ */
