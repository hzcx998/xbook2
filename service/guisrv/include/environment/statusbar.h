#ifndef __GUISRV_ENVIRONMENT_STATUSBAR_H__
#define __GUISRV_ENVIRONMENT_STATUSBAR_H__

#include <widget/button.h>
#include <window/window.h>
#include <sys/list.h>

/* 状态栏默认图标宽高 */
#define GUI_STATUSBAR_ICON_SIZE       16
#define GUI_STATUSBAR_ITEM_SIZE       30
#define GUI_STATUSBAR_HEIGHT          30

#define GUI_STATUSBAR_BACK_COLOR    COLOR_RGB(0,32,64)
#define GUI_STATUSBAR_ACTIVE_COLOR  COLOR_RGB(255,32,64)
#define GUI_STATUSBAR_FONT_COLOR    COLOR_WHITE

#define GUI_STATUSBAR_ITEM_NR       32

#define GUI_STATUSBAR_USED          0x01
#define GUI_STATUSBAR_MENU          0x02
#define GUI_STATUSBAR_ICON          0x04

/* 状态栏元素 */
typedef struct _gui_statusbar_item {
    int                 count;          /* 计数 */
    int                 flags;          /* 元素标志 */
    gui_button_t        *button;        /* 按钮 */
    list_t              list;           /* 窗口控制链表 */
    int                 (*add) (struct _gui_statusbar_item *);
    int                 (*del) (struct _gui_statusbar_item *);
    int                 (*destroy) (struct _gui_statusbar_item *);
    int                 (*set_text) (struct _gui_statusbar_item *, char *text);
    
} gui_statusbar_item_t;

/* 状态栏管理器 */
typedef struct {
    struct _gui_window  *window;        /* 管理器所在窗口 */
    list_t              menu_list_head; /* 菜单链表头 */
    list_t              icon_list_head; /* 图标链表头 */
    GUI_COLOR           back_color;     /* 背景颜色 */
    GUI_COLOR           font_color;     /* 字体颜色 */
    GUI_COLOR           active_color;   /* 激活后的颜色 */
    gui_statusbar_item_t *time_item;    /* 时间元素 */
    void                (*read) ();     /* 读取操作 */
} gui_statusbar_manager_t;

extern gui_statusbar_manager_t statusbar_manager;

int init_statusbar_manager();

/* 状态栏控件 */
void statusbar_time_read();

#endif  /* __GUISRV_ENVIRONMENT_STATUSBAR_H__ */
