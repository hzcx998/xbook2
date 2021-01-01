#ifndef _XGUI_MOUSE_H
#define _XGUI_MOUSE_H

typedef struct {
    void (*motion)(void);
    void (*button_down)(int);
    void (*button_up)(int);
    void (*wheel)(int);
    int x, y;
    int handle;
} xbrower_mouse_t;

int xbrower_mouse_init();
int xbrower_mouse_exit();

int xbrower_mouse_poll();

#endif /* _XGUI_MOUSE_H */
