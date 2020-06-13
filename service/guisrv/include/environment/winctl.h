#ifndef __GUISRV_ENVIRONMENT_WINCTL_H__
#define __GUISRV_ENVIRONMENT_WINCTL_H__

#include <widget/button.h>
#include <window/window.h>
#include <sys/list.h>

/* 窗口控制图标宽高 */
#define GUI_WINCTL_ICON_SIZE       48

#define GUI_WINCTL_WIDTH       64

#define GUI_WINCTL_BACKCOLOR    COLOR_RGB(0,64,128)

typedef struct {

    gui_button_t        *button;        /* 按钮 */
    struct _gui_window  *window;  /* 对应的窗口，存在互相调用的时候，就用struct原型 */
    list_t              list;           /* 窗口控制链表 */
} gui_winctl_t;

/* 窗口控制管理器 */
typedef struct {
    struct _gui_window  *window;            /* 管理器所在窗口 */
    list_t              winctl_list_head;   /* 窗口控制链表头 */
    GUI_COLOR           backcolor;          /* 背景颜色 */
} gui_winctl_manager_t;

int init_winctl_manager();
gui_winctl_t *gui_create_winctl(struct _gui_window *window);
int gui_destroy_winctl(gui_winctl_t *winctl);
void gui_winctl_show();

int gui_winctl_add(gui_winctl_t *winctl);
int gui_winctl_del(gui_winctl_t *winctl);

#endif  /* __GUISRV_ENVIRONMENT_WINCTL_H__ */
