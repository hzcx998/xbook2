#ifndef __GUISRV_INPUT_MOUSE_H__
#define __GUISRV_INPUT_MOUSE_H__

#include <graph/color.h>

typedef struct _input_mouse {
    int x, y;
    int             last_x; /* 上次坐标 */
    int             last_y;
    SCREEN_COLOR       old_color;  /* 鼠标原来的颜色 */
    void (*motion)(void);
    void (*button_down)(int);
    void (*button_up)(int);
    void (*show)(int , int );
} input_mouse_t;

extern input_mouse_t input_mouse;

int init_mouse_input();

#endif  /* __GUISRV_INPUT_MOUSE_H__ */
