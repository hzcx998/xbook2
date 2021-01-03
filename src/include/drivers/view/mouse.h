#ifndef _XBOOK_DRIVERS_VIEW_MOUSE_H
#define _XBOOK_DRIVERS_VIEW_MOUSE_H

#include "view.h"

/* 鼠标光标状态 */
typedef enum {
    VIEW_MOUSE_NORMAL = 0,        /* 普通 */
    VIEW_MOUSE_HOLD,              /* 抓取 */
    VIEW_MOUSE_HELP,              /* 帮助 */
    VIEW_MOUSE_BACKGROUND,        /* 后台运行 */
    VIEW_MOUSE_BUSY,              /* 繁忙 */
    VIEW_MOUSE_ACCURATE,          /* 精确选择 */
    VIEW_MOUSE_TEXT,              /* 文本选择 */
    VIEW_MOUSE_UNUSABLE,          /* 不可用 */
    VIEW_MOUSE_VRESIZE,           /* 垂直调整大小 */
    VIEW_MOUSE_HRESIZE,           /* 水平调整大小 */
    VIEW_MOUSE_DRESIZE1,          /* 对角线调整大小1(斜上) */
    VIEW_MOUSE_DRESIZE2,          /* 对角线调整大小2(斜下) */
    VIEW_MOUSE_MOVE,              /* 移动 */
    VIEW_MOUSE_LINK,              /* 链接选择 */
} view_mouse_state_t;

typedef struct {
    void (*motion)(void);
    void (*button_down)(int);
    void (*button_up)(int);
    void (*wheel)(int);
    int x, y;
    int local_x, local_y;
    int click_x, click_y;
    int handle;
    view_t *view;
    view_mouse_state_t state;
} view_mouse_t;

extern view_mouse_t view_mouse;

int view_mouse_init();
int view_mouse_exit();

int view_mouse_poll();
void view_mouse_set_state(view_mouse_state_t state);
int view_mouse_is_state(view_mouse_state_t state);

#endif /* _XBOOK_DRIVERS_VIEW_MOUSE_H */
