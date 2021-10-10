#ifndef _XBOOK_DRIVERS_VIEW_MOUSE_H
#define _XBOOK_DRIVERS_VIEW_MOUSE_H

#include "view.h"
#include "bitmap.h"

/* 鼠标光标状态 */
typedef enum {
    VIEW_MOUSE_NORMAL = 0,        /* 普通 */
    VIEW_MOUSE_HOLD,              /* 抓取 */
    VIEW_MOUSE_HELP,              /* 帮助 */
    VIEW_MOUSE_PEN,               /* 画笔 */
    VIEW_MOUSE_BUSY,              /* 繁忙 */
    VIEW_MOUSE_ACCURATE,          /* 精确选择 */
    VIEW_MOUSE_TEXT,              /* 文本选择 */
    VIEW_MOUSE_UNUSABLE,          /* 不可用 */
    VIEW_MOUSE_VRESIZE,           /* 垂直调整大小 */
    VIEW_MOUSE_HRESIZE,           /* 水平调整大小 */
    VIEW_MOUSE_DRESIZE1,          /* 对角线调整大小1(斜上) */
    VIEW_MOUSE_DRESIZE2,          /* 对角线调整大小2(斜下) */
    VIEW_MOUSE_RESIZEALL,         /* 全方向调整 */
    VIEW_MOUSE_HAND,              /* 手 */
    VIEW_MOUSE_INVISIBLE,         /* 不可见状态 */
    VIEW_MOUSE_STATE_NR,
} view_mouse_state_t;

typedef struct {
    view_bitmap_t *bmp; // 状态位图
    int off_x;  // 视图偏移
    int off_y;  
    view_mouse_state_t state;
} view_mouse_state_info_t;

#define VIEW_MOUSE_SIZE 32

/* 准备执行resize操作 */
#define VIEW_MOUSE_FLAG_RESIZE_READY 0X01

typedef struct {
    void (*motion)(void);
    void (*button_down)(int);
    void (*button_up)(int);
    void (*wheel)(int);
    int x, y;
    int local_x, local_y;
    int click_x, click_y;
    int view_off_x, view_off_y;
    int handle;
    int flags;
    view_t *view;
    view_mouse_state_t state;

    view_mouse_state_info_t state_table[VIEW_MOUSE_STATE_NR];
} view_mouse_t;

extern view_mouse_t view_mouse;

int view_mouse_init();
int view_mouse_exit();

int view_mouse_poll();
void view_mouse_set_state(view_mouse_state_t state);
int view_mouse_get_state(view_mouse_state_t *state);

int view_mouse_is_state(view_mouse_state_t state);
void view_mouse_draw(view_mouse_state_t state);

void view_mouse_move_view();
int view_mouse_set_state_info(view_mouse_state_info_t *info);
int view_mouse_set_state_info_ex(view_mouse_state_info_t *info);
int view_mouse_get_state_info(view_mouse_state_info_t *info);

#endif /* _XBOOK_DRIVERS_VIEW_MOUSE_H */
