#ifndef _UVIEW_MOUSE_H
#define _UVIEW_MOUSE_H

#include "uview_bitmap.h"

/* 鼠标光标状态 */
typedef enum {
    UVIEW_MOUSE_NORMAL = 0,        /* 普通 */
    UVIEW_MOUSE_HOLD,              /* 抓取 */
    UVIEW_MOUSE_HELP,              /* 帮助 */
    UVIEW_MOUSE_PEN,               /* 画笔 */
    UVIEW_MOUSE_BUSY,              /* 繁忙 */
    UVIEW_MOUSE_ACCURATE,          /* 精确选择 */
    UVIEW_MOUSE_TEXT,              /* 文本选择 */
    UVIEW_MOUSE_UNUSABLE,          /* 不可用 */
    UVIEW_MOUSE_VRESIZE,           /* 垂直调整大小 */
    UVIEW_MOUSE_HRESIZE,           /* 水平调整大小 */
    UVIEW_MOUSE_DRESIZE1,          /* 对角线调整大小1(斜上) */
    UVIEW_MOUSE_DRESIZE2,          /* 对角线调整大小2(斜下) */
    UVIEW_MOUSE_RESIZEALL,         /* 移动 */
    UVIEW_MOUSE_HAND,              /* 手 */
    UVIEW_MOUSE_STATE_NR,
} uview_mouse_state_t;

typedef struct {
    uview_bitmap_t *bmp; // 状态位图
    int off_x;  // 视图偏移
    int off_y;  
    uview_mouse_state_t state;
} uview_mouse_state_info_t;

int uview_set_mouse_state(int vfd, uview_mouse_state_t state);
int uview_set_mouse_state_info(int vfd, uview_mouse_state_info_t *info);
int uview_get_mouse_state(int vfd, uview_mouse_state_t *state);
int uview_get_mouse_state_info(int vfd, uview_mouse_state_info_t *info);

int uview_set_mouse_state_noview(uview_mouse_state_t state);
int uview_set_mouse_state_info_noview(uview_mouse_state_info_t *info);
int uview_get_mouse_state_noview(uview_mouse_state_t *state);
int uview_get_mouse_state_info_noview(uview_mouse_state_info_t *info);

#endif /* _UVIEW_MOUSE_H */