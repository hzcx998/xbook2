#ifndef _GAPI_TOUCH_H
#define _GAPI_TOUCH_H

#include "gshape.h"
#include "gcolor.h"
#include "gmessage.h"
#include "glayer.h"

#include <sys/list.h>

typedef int (*g_touch_handler_t) (void *arg);

enum {
    G_TOUCH_IDLE = 0,
    G_TOUCH_ON,
    G_TOUCH_SHAPE_FILL,
    G_TOUCH_SHAPE_BORDER,
};

/* 触碰块，类似于按钮机制，绑定在窗口上 */
typedef struct {
    list_t list;
    g_rect_t rect;      /* 位置区域 */
    char state;         /* 状态：默认，触碰 */
    char shape;         /* 形状 */
    char hold_on[3];       /* 鼠标已经点击，等待弹起完成事件 */
    g_color_t color_idle;
    g_color_t color_on;
    g_touch_handler_t handler[3]; /* 鼠标按键触发事件后执行的函数 */
    g_layer_t layer; /* 所在图层 */
    void *extension;    /* 扩展内容 */
} g_touch_t;

g_touch_t *g_new_touch(unsigned int width, unsigned int height);
int g_del_touch(g_touch_t *tch);

static inline void g_touch_set_location(g_touch_t *tch, int x, int y)
{
    tch->rect.x = x;
    tch->rect.y = y;
}

static inline void g_touch_set_color(g_touch_t *tch, g_color_t idle, g_color_t on)
{
    tch->color_idle = idle;
    tch->color_on = on;
}

static inline void g_touch_set_handler(g_touch_t *tch, int btn, g_touch_handler_t handler)
{
    if (btn >= 0 && btn < 3)
        tch->handler[btn] = handler;
}

static inline int g_touch_set_layer(g_touch_t *tch, g_layer_t layer, list_t *touch_list)
{
    if (!tch || layer < 0 || !touch_list)
        return -1;
    tch->layer = layer;
    list_add_tail(&tch->list, touch_list);
    return 0;
}

static inline void g_touch_set_shape(g_touch_t *tch, char shape)
{
    tch->shape = shape;
}

static inline void g_touch_set_extension(g_touch_t *tch, void *ext)
{
    tch->extension = ext;
}
int g_touch_paint(g_touch_t *tch);
int g_touch_paint_group(list_t *list_head);
int g_touch_set_idel_color_group(list_t *list_head, g_color_t color);
int g_touch_del_group(list_t *list_head);

int g_touch_state_check(g_touch_t *tch, g_point_t *po);
int g_touch_state_check_group(list_t *list_head, g_point_t *po);

int g_touch_click_check(g_touch_t *tch, g_point_t *po, int btn);
int g_touch_click_check_group(list_t *list_head, g_point_t *po, int btn);

#endif /* _GAPI_TOUCH_H */