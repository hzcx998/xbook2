#ifndef __GUISRV_DRIVER_MOUSE_H__
#define __GUISRV_DRIVER_MOUSE_H__

#include <gui/color.h>

typedef struct _gui_mouse {
    int            (*open)(void);
    int	           (*close)(void);
    int	           (*read)(void);
    int x, y;
    int             last_x; /* 上次坐标 */
    int             last_y;
    SCREEN_COLOR       old_color;  /* 鼠标原来的颜色 */
    void (*motion)(void);
    void (*button_down)(int);
    void (*button_up)(int);
    void (*show)(int , int );
} gui_mouse_t;

extern gui_mouse_t gui_mouse;

int gui_init_mouse();

void gui_mouse_show(int x, int y);
void gui_mouse_button_down(int btn);
void gui_mouse_button_up(int btn);
void gui_mouse_motion();

int init_mouse_layer();

#endif  /* __GUISRV_DRIVER_MOUSE_H__ */
