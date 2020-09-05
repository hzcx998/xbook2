#ifndef _GUI_SCREEN_H
#define _GUI_SCREEN_H

#include <gui/color.h>
#include <gui/shape.h>

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int bits_per_pixel;
    
    gui_region_t window_region;       /* 窗口区域，窗口可以活动的区域 */
} g_screen_t;

typedef struct {
    int width;  
    int height;
    int bpp;        /* bits per pixel */
    /* 颜色转换 */
    SCREEN_COLOR   (*gui_to_screen_color)(GUI_COLOR  gui_color);
    GUI_COLOR      (*screen_to_gui_color)(SCREEN_COLOR  screen_color);
    g_screen_t screen;
    int            (*open)(void);
    int	           (*close)(void);

    int            (*output_pixel)(int x, int y, SCREEN_COLOR  color);
    int            (*input_pixel)(int x, int y, SCREEN_COLOR  *color);
    
    int            (*output_hline)(int left, int right, int top, SCREEN_COLOR  color);
    int            (*output_vline)(int left, int top, int bottom, SCREEN_COLOR  color);
    int            (*output_rect_fill)(int left, int top, int right, int bottom, SCREEN_COLOR  color);

} gui_screen_t;

extern gui_screen_t gui_screen;

int gui_init_screen();

int sys_screen_get(g_screen_t *screen);
int sys_screen_set_window_region(gui_region_t *region);

#endif  /* _GUI_SCREEN_H */
