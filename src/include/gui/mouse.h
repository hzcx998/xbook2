#ifndef _GUI_INPUT_MOUSE_H
#define _GUI_INPUT_MOUSE_H

#include <gui/color.h>
#include <gui/message.h>
#include <gui/layer.h>
#include <xbook/config.h>

/**
 * 图层遮罩机制：
 * 当移动图层或者调整图层大小的时候。调整整个图层位置或者大小的效率是
 * 十分低下的，于是，采取图层遮罩机制来实现这个中间过程。
 * 图层遮罩机制，最开始，会创建一个全屏的图层，并且设置为透明，然后隐藏它。
 * 打开遮罩时会把这个图层提升到窗口最高层，这样，在这个遮罩图层上绘制的内容，
 * 就可以在窗口之上显示了。
 * 核心点在于，我们时在这个遮罩图层上绘制要移动或者要调整大小的图层的边框。
 * 于是，仅仅绘制一个边框，其绘制以及刷新量都很少，速度很快。这样就起到了
 * 障眼法的效果。
 * 当关闭图层遮罩的时候，图层会隐藏，那么就恢复到之前没有进行窗口移动或者调整
 * 时的状态。
 * 
 * 激活流程：
 * 窗口移动：鼠标在标题栏中->按住左键->鼠标移动->开启图层遮罩机制->根据要移动的图层
 * 绘制图层遮罩图层里面的边框->鼠标左键弹起->关闭图层遮罩机制->将窗口移动到目标位置
 * 
 * 窗口调整：鼠标在窗口边框->按住鼠标左键->开启图层遮罩机制->鼠标移动->根据要调整的图层
 * 的大小绘制图层遮罩图层里面的边框->鼠标左键弹起->关闭图层遮罩机制->将窗口移动到目标位置
 * 并调整大小
 * 
 */

/* 鼠标光标状态 */
typedef enum {
    MOUSE_NORMAL = 0,        /* 普通 */
    MOUSE_HOLD,              /* 抓取 */
    MOUSE_HELP,              /* 帮助 */
    MOUSE_BACKGROUND,        /* 后台运行 */
    MOUSE_BUSY,              /* 繁忙 */
    MOUSE_ACCURATE,          /* 精确选择 */
    MOUSE_TEXT,              /* 文本选择 */
    MOUSE_UNUSABLE,          /* 不可用 */
    MOUSE_VRESIZE,           /* 垂直调整大小 */
    MOUSE_HRESIZE,           /* 水平调整大小 */
    MOUSE_DRESIZE1,          /* 对角线调整大小1(斜上) */
    MOUSE_DRESIZE2,          /* 对角线调整大小2(斜下) */
    MOUSE_MOVE,              /* 移动 */
    MOUSE_LINK,              /* 链接选择 */
} mouse_state_t;

typedef struct _gui_mouse {
    int            (*open)(void);
    int	           (*close)(void);
    int	           (*read)(void);
    int x, y;
    int             last_x; /* 上次坐标 */
    int             last_y;
    int             local_x; /* 在图层中的局部位置 */
    int             local_y;
    gui_point_t     click_point;    /* 点击点 */
    gui_point_t     lyoff;    /* 图层偏移，图层在鼠标位置的偏移量 */
    SCREEN_COLOR    old_color;  /* 鼠标原来的颜色 */
    mouse_state_t   state;      /* 鼠标状态 */
    layer_t *layer;         /* 鼠标的图层 */
    layer_t *drag_layer;    /* 拖拽的图层 */
    layer_t *resize_layer;  /* 调整大小的图层 */
    layer_t *hover;         /* 悬停的图层 */
    #ifdef CONFIG_SHADE_LAYER
    layer_t *shade;        /* 遮罩图层 */
    gui_rect_t shade_rect; /* 遮罩图层矩形区域 */
    #endif /* CONFIG_SHADE_LAYER */
    uint32_t width;     /* 鼠标图层宽度 */
    uint32_t height;    /* 鼠标图层高度 */
    void (*motion)(void);
    void (*button_down)(int);
    void (*button_up)(int);
} gui_mouse_t;

extern gui_mouse_t gui_mouse;

int gui_init_mouse();

void gui_mouse_show(int x, int y);
void gui_mouse_button_down(int btn);
void gui_mouse_button_up(int btn);
void gui_mouse_motion();
void gui_mouse_wheel(int val);

int init_mouse_layer();
void gui_mouse_set_state(mouse_state_t state);
void gui_mouse_set_lyoff(int x, int y);
void gui_mouse_layer_move();
void gui_draw_shade_layer(layer_t *shade, gui_rect_t *rect, int draw);

#endif  /* _GUI_INPUT_MOUSE_H */
