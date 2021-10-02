#ifndef _DWIN_HAL_H
#define _DWIN_HAL_H

struct dwin_hal_keyboard 
{
    int (*init)(struct dwin_hal_keyboard *);
    void (*exit)(struct dwin_hal_keyboard *);
    void *extension;
};

struct dwin_hal_mouse 
{
    int (*init)(struct dwin_hal_mouse *);
    void (*exit)(struct dwin_hal_mouse *);
    void *extension;
};

struct dwin_hal_lcd 
{
    int (*init)(struct dwin_hal_lcd *);
    void (*exit)(struct dwin_hal_lcd *);
    void *extension;
};

struct dwin_hal 
{
    struct dwin_hal_keyboard *keyboard;
    struct dwin_hal_mouse *mouse;
    struct dwin_hal_lcd *lcd;

    void *extension;
};

/* export dwin hal */
extern struct dwin_hal *dwin_hal;

/* hal overwrite this function */
int dwin_hal_init(void);

int dwin_hal_register(struct dwin_hal *hal);
int dwin_hal_unregister(struct dwin_hal *hal);


#endif   /* _DWIN_HAL_H */
