#ifndef _GUI_INPUT_MOUSE_H
#define _GUI_INPUT_MOUSE_H

#include <gui/color.h>
#include <gui/message.h>
#include <gui/layer.h>
#include <xbook/config.h>
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
    #ifdef CONFIG_WAKER_LAYER
    layer_t *walker;        /* 拖拽时显示图层（开启快速窗口时） */
    #endif /* CONFIG_WAKER_LAYER */
    uint32_t width;
    uint32_t height;
    void (*motion)(void);
    void (*button_down)(int);
    void (*button_up)(int);
    void (*show)(int , int );
} gui_mouse_t;

extern gui_mouse_t gui_mouse;

int gui_init_mouse();

void gui_mouse_show(int x, int y);
void gui_mouse_button_down(int btn);
void gui_mouse_button_up(int btn);
void gui_mouse_motion();

int init_mouse_layer();
void gui_mouse_set_state(mouse_state_t state);
void gui_mouse_set_lyoff(int x, int y);
void gui_mouse_layer_move();

#endif  /* _GUI_INPUT_MOUSE_H */
