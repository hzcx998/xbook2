#ifndef __GUISRV_WINDOW_H__
#define __GUISRV_WINDOW_H__

#include <layer/layer.h>
#include <widget/button.h>
#include <widget/label.h>
#include <sys/list.h>


#include <environment/box.h>

#define GUIW_TITLE_LEN    128   /* 标题字符串长度 */

#define GUIW_NO_TITLE       0x01    /* 没有标题 */
#define GUIW_BTN_MINIM      0x02    /* 最小化按钮 */
#define GUIW_BTN_MAXIM      0x04    /* 最大化按钮 */
#define GUIW_BTN_CLOSE      0x08    /* 关闭按钮 */
#define GUIW_FIXED          0x10    /* 固定的窗口 */

#define GUIW_BTN_MASK       (GUIW_BTN_MINIM | GUIW_BTN_MAXIM | GUIW_BTN_CLOSE)

/* 默认的话，3个按钮都有 */
#define GUIW_BTN_DEFAULT   GUIW_BTN_MASK

#define GUIW_TITLE_HEIGHT    24     /* 窗口标题高度 */

#define GUIW_TITLE_BAR_ON_COLOR     COLOR_RGB(192, 192, 192)
#define GUIW_TITLE_BAR_OFF_COLOR    COLOR_RGB(128, 128, 128)

#define GUIW_TITLE_TEXT_ON_COLOR    COLOR_RGB(255, 255, 255)
#define GUIW_TITLE_TEXT_OFF_COLOR   COLOR_RGB(200, 200, 200)

/* 可以缓存16个窗口 */
#define GUIW_CACHE_TABLE_SIZE       16

typedef struct _gui_window {
    unsigned int id;            /* 窗口id */
    int x, y;                   /* 窗口的位置 */
    int width, height;          /* 窗口的宽高 */
    int x_off, y_off;           /* 窗口数据的偏移 */
    int attr;                   /* 窗口的属性 */
    layer_t *layer;             /* 窗口对应的图层 */
    list_t list;                /* 窗口链表 */
    list_t child_list;          /* 子窗口链表 */
    list_t window_list;         /* 所有窗口串连成一个链表 */
    struct _gui_window *parent; /* 父窗口指针 */
    GUI_COLOR title_bar_color;  /* 标题栏颜色 */
    GUI_COLOR title_color;      /* 标题颜色 */
    env_box_t title_box;        /* 标题盒子 */
    env_box_t body_box;         /* 窗体盒子 */
    
    gui_button_t *btn_close;    /* “关闭”按钮 */
    gui_button_t *btn_minim;    /* “最小化”按钮 */
    gui_button_t *btn_maxim;    /* “最大化”按钮 */
    gui_label_t  *text_title;   /* 标题文本 */
    
    void *winctl;               /* 窗口控制 */

/// 客户端交互信息
    int shmid;                  /* 共享内存的id，用来映射客户端窗口 */
    void *mapped_addr;          /* 共享内存映射后的地址 */
    size_t map_size;            /* 映射占用内存大小：页为单位 */
    unsigned short start_off;   /* 起始偏移 */
    unsigned int display_id;    /* 窗口所在的显示id */
    long input_mask;            /* 输入遮罩 */
} gui_window_t;

extern gui_window_t *window_current;

#define current_window window_current

int init_gui_window();

gui_window_t *gui_create_window(
    char *title,
    int x,
    int y,
    int width,
    int height,
    GUI_COLOR background,
    int attr,
    gui_window_t *parent
);

int gui_destroy_window(gui_window_t *win);

int gui_window_update(
    gui_window_t *win,
    int left,
    int top,
    int right,
    int buttom
);

void gui_window_switch(gui_window_t *window);
void gui_window_focus(gui_window_t *window);
int gui_window_hide(gui_window_t *win);
gui_window_t *gui_window_topest();
int gui_window_show(gui_window_t *win);
gui_window_t *gui_window_get_by_id(unsigned int wid);

int gui_window_cache_add(gui_window_t *win);
int gui_window_cache_del(gui_window_t *win);
gui_window_t *gui_window_cache_find(unsigned int wid);
void gui_window_hide_all();
void gui_window_show_all();

#endif  /* __GUISRV_WINDOW_H__ */
