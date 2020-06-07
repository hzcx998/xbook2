/* 窗口控件之按钮 */
#ifndef _GUISRV_WIDGET_BUTTON_H
#define _GUISRV_WIDGET_BUTTON_H

#include <types.h>
#include <stdint.h>
#include <layer/color.h>
#include <sys/list.h>
#include <font/font.h>
#include <widget/label.h>

#define GUI_BUTTON_DEFAULT      0   /* 按钮默认状态 */
#define GUI_BUTTON_FOCUS        1   /* 按钮聚焦状态 */
#define GUI_BUTTON_SELECTED     2   /* 按钮选择状态 */

/* 按钮默认颜色 */
#define GUI_BUTTON_DEFAULT_COLOR    COLOR_ARGB(255, 50, 50, 50)
#define GUI_BUTTON_FOCUS_COLOR      COLOR_ARGB(255, 200, 200, 200)
#define GUI_BUTTON_SELECTED_COLOR   COLOR_ARGB(255, 100, 100, 100)

typedef struct _gui_button {
    gui_label_t label;      /* 继承标签：第一个成员 */
    int state;              /* 按钮状态：默认，聚焦，点击 */
    GUI_COLOR default_color;    /* 默认颜色 */
    GUI_COLOR focus_color;      /* 聚焦颜色 */
    GUI_COLOR selected_color;   /* 选择颜色 */
    void (*handler) (struct _gui_button *button);

    /* 内部函数 */
    void (*set_location) (struct _gui_button *, int , int );
    void (*set_size) (struct _gui_button *, int , int );
    void (*set_color) (struct _gui_button *, GUI_COLOR , GUI_COLOR );
    int (*set_text_len) (struct _gui_button *, int );
    void (*set_text) (struct _gui_button *, char *);
    int (*set_font) (struct _gui_button *, char *);
    void (*set_name) (struct _gui_button *, char *);
    void (*add) (struct _gui_button *, layer_t *);
    void (*del) (struct _gui_button *);
    void (*show) (struct _gui_button *);
    void (*cleanup) (struct _gui_button *);

} gui_button_t;

/* 处理回调函数 */
typedef void (*gui_button_handler_t) (struct _gui_button *);

gui_button_t *gui_create_button(
    gui_label_types_t type,
    int x,
    int y,
    int width,
    int height
);

int gui_button_init(
    gui_button_t *button,
    gui_label_types_t type,
    int x,
    int y,
    int width,
    int height
);

void gui_button_destroy(gui_button_t *button);

#endif   /* _GUISRV_WIDGET_BUTTON_H */
