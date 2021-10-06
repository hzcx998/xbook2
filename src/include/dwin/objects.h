/**
 * Direct Window (DWIN)
 */

#ifndef _DWIN_OBJECTS_H
#define _DWIN_OBJECTS_H

#define DWIN_KMOD_SHIFT_L    0x01
#define DWIN_KMOD_SHIFT_R    0x02
#define DWIN_KMOD_SHIFT      (DWIN_KMOD_SHIFT_L | DWIN_KMOD_SHIFT_R)
#define DWIN_KMOD_CTRL_L     0x04
#define DWIN_KMOD_CTRL_R     0x08
#define DWIN_KMOD_CTRL       (DWIN_KMOD_CTRL_L | DWIN_KMOD_CTRL_R)
#define DWIN_KMOD_ALT_L      0x10
#define DWIN_KMOD_ALT_R      0x20
#define DWIN_KMOD_ALT        (DWIN_KMOD_ALT_L | DWIN_KMOD_ALT_R)
#define DWIN_KMOD_PAD	     0x40
#define DWIN_KMOD_NUM	     0x80
#define DWIN_KMOD_CAPS	     0x100

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
    void *object;
};

struct dwin_msgpool
{
    void *object;   /* message pool object */
    int msgsz;
    int msgcnt;
};

void dwin_keyboard_init(struct dwin_keyboard *keyboard);
void dwin_mouse_init(struct dwin_mouse *mouse);

void dwin_lcd_init(struct dwin_lcd *lcd);
void dwin_thread_init(struct dwin_thread *thread);

void dwin_msgpool_init(struct dwin_msgpool *msgpool);

int dwin_lcd_map(struct dwin_lcd *lcd);
int dwin_lcd_unmap(struct dwin_lcd *lcd);
void dwin_lcd_draw_rect(struct dwin_lcd *lcd, int x, int y, int w, int h, unsigned int color);
void dwin_lcd_demo(struct dwin_lcd *lcd);

#endif   /* _DWIN_OBJECTS_H */
