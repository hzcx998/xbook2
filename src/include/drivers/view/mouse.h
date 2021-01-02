#ifndef _XBOOK_DRIVERS_VIEW_MOUSE_H
#define _XBOOK_DRIVERS_VIEW_MOUSE_H

#include "view.h"

typedef struct {
    void (*motion)(void);
    void (*button_down)(int);
    void (*button_up)(int);
    void (*wheel)(int);
    int x, y;
    int handle;
    view_t *view;
} view_mouse_t;

extern view_mouse_t view_mouse;

int view_mouse_init();
int view_mouse_exit();

int view_mouse_poll();

#endif /* _XBOOK_DRIVERS_VIEW_MOUSE_H */
