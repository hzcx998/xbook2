#ifndef _DWIN_HAL_H
#define _DWIN_HAL_H

#include <dwin/objects.h>

struct dwin_hal_keyboard 
{
    struct dwin_keyboard parent;

    int (*init)(struct dwin_keyboard *);
    int (*exit)(struct dwin_keyboard *);
    int (*read)(struct dwin_keyboard *);
    void *extension;
};

struct dwin_hal_mouse 
{
    struct dwin_mouse parent;

    int (*init)(struct dwin_mouse *);
    int (*exit)(struct dwin_mouse *);
    int (*read)(struct dwin_mouse *);
    void *extension;
};

struct dwin_hal_lcd 
{
    struct dwin_lcd parent;

    int (*init)(struct dwin_lcd *);
    int (*exit)(struct dwin_lcd *);
    int (*map)(struct dwin_lcd *);
    int (*unmap)(struct dwin_lcd *);
    void *extension;
};

struct dwin_hal_thread 
{
    struct dwin_thread parent;
    int (*start)(struct dwin_thread *, void (*)(void *), void *);
    int (*stop)(struct dwin_thread *, void *);
    void *extension;
};

struct dwin_hal 
{
    struct dwin_hal_keyboard *keyboard;
    struct dwin_hal_mouse *mouse;
    struct dwin_hal_lcd *lcd;
    struct dwin_hal_thread *thread;

    void *extension;
};

/* export dwin hal */
extern struct dwin_hal *dwin_hal;

/* hal overwrite this function */
int dwin_hal_init(void);
void dwin_hal_exit(void);

int dwin_hal_register(struct dwin_hal *hal);
int dwin_hal_unregister(struct dwin_hal *hal);

#endif   /* _DWIN_HAL_H */
