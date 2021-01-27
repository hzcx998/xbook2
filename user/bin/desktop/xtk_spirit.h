#ifndef _LIB_XTK_SPIRIT_H
#define _LIB_XTK_SPIRIT_H

#include <uview.h>
#include <sys/list.h>
#include "xtk_image.h"
#include "xtk_container_struct.h"
#include "xtk_surface.h"

typedef unsigned int xtk_color_t;

typedef enum {
    XTK_ALIGN_LEFT = 0,
    XTK_ALIGN_RIGHT,
    XTK_ALIGN_TOP,
    XTK_ALIGN_BOTTOM,
    XTK_ALIGN_CENTER,
} xtk_align_t;

/* 鼠标光标状态,和uview_mouse保持一致 */
typedef enum {
    XTK_CURSOR_NORMAL = 0,        /* 普通 */
    XTK_CURSOR_HOLD,              /* 抓取 */
    XTK_CURSOR_HELP,              /* 帮助 */
    XTK_CURSOR_PEN,               /* 画笔 */
    XTK_CURSOR_BUSY,              /* 繁忙 */
    XTK_CURSOR_ACCURATE,          /* 精确选择 */
    XTK_CURSOR_TEXT,              /* 文本选择 */
    XTK_CURSOR_UNUSABLE,          /* 不可用 */
    XTK_CURSOR_VRESIZE,           /* 垂直调整大小 */
    XTK_CURSOR_HRESIZE,           /* 水平调整大小 */
    XTK_CURSOR_DRESIZE1,          /* 对角线调整大小1(斜上) */
    XTK_CURSOR_DRESIZE2,          /* 对角线调整大小2(斜下) */
    XTK_CURSOR_RESIZEALL,         /* 移动 */
    XTK_CURSOR_HAND,              /* 手 */
    XTK_CURSOR_STATE_NR,
} xtk_cursor_state_t;

typedef struct {
    /* border */
    xtk_color_t border_color;
    /* background */
    xtk_color_t background_color;
    xtk_align_t background_align;
    /* font */
    xtk_color_t color;
    xtk_align_t align;
    xtk_cursor_state_t cursor;
} xtk_style_t;

typedef enum {
    XTK_SPIRIT_TYPE_UNKNOWN = 0,
    XTK_SPIRIT_TYPE_WINDOW,
    XTK_SPIRIT_TYPE_LABEL,
    XTK_SPIRIT_TYPE_BUTTON,
    XTK_SPIRIT_TYPE_BOX,
    XTK_SPIRIT_TYPE_TABLE,
    XTK_SPIRIT_TYPE_FIXED,
} xtk_spirit_type_t;

typedef struct {
    list_t list;
    xtk_spirit_type_t type;
    int x;
    int y;
    int width;
    int height;
    int width_min;
    int height_min;
    int visible;        // 可见性
    int view;   // 精灵所在的视图
    xtk_style_t style;
    /* back */
    xtk_image_t *background_image;
    /* front */
    char *text;
    xtk_image_t *image;
    xtk_surface_t *surface;
    /* extension */
    xtk_container_t *container;    // 每个精灵对应一个容器
    xtk_container_t *attached_container;    // 每个精灵附加到的容器
} xtk_spirit_t;

#define XTK_IN_SPIRIT(spirit, _x, _y) \
        ((spirit)->x <= (_x) && (spirit)->y <= (_y) && \
        (_x) < (spirit)->x + (spirit)->width  && (_y) < (spirit)->y + (spirit)->height)

void xtk_spirit_init(xtk_spirit_t *spirit, int x, int y, int width, int height);
int xtk_spirit_cleanup(xtk_spirit_t *spirit);
xtk_spirit_t *xtk_spirit_create(int x, int y, int width, int height);
int xtk_spirit_destroy(xtk_spirit_t *spilit);
int xtk_spirit_destroy_all(xtk_spirit_t *spirit);
int xtk_spirit_set_pos(xtk_spirit_t *spilit, int x, int y);
int xtk_spirit_set_size(xtk_spirit_t *spilit, int width, int height);
int xtk_spirit_set_size_request(xtk_spirit_t *spirit, int width, int height);
int xtk_spirit_set_text(xtk_spirit_t *spilit, char *text);
int xtk_spirit_set_type(xtk_spirit_t *spirit, xtk_spirit_type_t type);
int xtk_spirit_auto_size(xtk_spirit_t *spilit);
int xtk_spirit_set_background_image(xtk_spirit_t *spilit, char *filename);
int xtk_spirit_to_surface(xtk_spirit_t *spilit, xtk_surface_t *surface);
int xtk_spirit_set_image(xtk_spirit_t *spilit, char *filename);
int xtk_spirit_set_surface(xtk_spirit_t *spilit, xtk_surface_t *surface);
int xtk_spirit_set_view(xtk_spirit_t *spirit, int view);
int xtk_spirit_set_container(xtk_spirit_t *spirit, xtk_container_t *container);
int xtk_spirit_reset_size(xtk_spirit_t *spirit, int width, int height);

int xtk_spirit_calc_aligin_pos(xtk_spirit_t *spirit, int width, int height, int *out_x, int *out_y);

int xtk_spirit_show_all(xtk_spirit_t *spirit);
int xtk_spirit_show(xtk_spirit_t *spirit);
int xtk_spirit_show_children(xtk_spirit_t *spirit);

int xtk_spirit_hide(xtk_spirit_t *spirit);
int xtk_spirit_hide_all(xtk_spirit_t *spirit);

#endif /* _LIB_XTK_SPIRIT_H */