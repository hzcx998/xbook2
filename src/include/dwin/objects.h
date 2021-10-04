/**
 * Direct Window (DWIN)
 */

#ifndef _DWIN_OBJECTS_H
#define _DWIN_OBJECTS_H

struct dwin_keyboard 
{
    int handle;
    int ledstate;
    int key_modify;
    int (*key_down)(struct dwin_keyboard *, int);
    int (*key_up)(struct dwin_keyboard *, int);
};

struct dwin_mouse
{
    int handle;
    void (*motion)(struct dwin_mouse *);
    void (*button_down)(struct dwin_mouse *, int);
    void (*button_up)(struct dwin_mouse *, int);
    void (*wheel)(struct dwin_mouse *, int);
    int x, y;
    int local_x, local_y;
    int click_x, click_y;
    int view_off_x, view_off_y;
};

struct dwin_lcd
{
    int handle;
    int width;
    int height;
    int bpp;        /* bits per pixel */
    unsigned char *vram_start;
    int (*out_pixel)(struct dwin_lcd *, int x, int y, unsigned int color);
};

struct dwin_thread
{
    int reserved;
};

void dwin_keyboard_init(struct dwin_keyboard *keyboard);
void dwin_mouse_init(struct dwin_mouse *mouse);
void dwin_lcd_init(struct dwin_lcd *lcd);
void dwin_thread_init(struct dwin_thread *thread);

int dwin_lcd_map(struct dwin_lcd *lcd);
int dwin_lcd_unmap(struct dwin_lcd *lcd);
void dwin_lcd_draw_rect(struct dwin_lcd *lcd, int x, int y, int w, int h, unsigned int color);
void dwin_lcd_demo(struct dwin_lcd *lcd);

#endif   /* _DWIN_OBJECTS_H */
