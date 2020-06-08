/* 窗口控件之标签 */
#ifndef _GUISRV_WIDGET_LABEL_H
#define _GUISRV_WIDGET_LABEL_H

#include <types.h>
#include <stdint.h>
#include <layer/color.h>
#include <sys/list.h>
#include <widget/widget.h>
#include <font/font.h>
#include <layer/layer.h>

/* 默认文本长度 */
#define GUI_MAX_LABEL_TEXT_LEN     255
#define GUI_DEFAULT_LABEL_TEXT_LEN     32

/* 标签默认颜色 */
#define GUI_LABEL_BACK_COLOR    COLOR_RGB(50, 50, 50)
#define GUI_LABEL_FONT_COLOR    COLOR_WHITE
#define GUI_LABEL_DISABEL_COLOR COLOR_RGB(192, 192, 192)

typedef enum _gui_label_type {
    GUI_LABEL_TEXT  = 0,    /* 文本标签 */
    GUI_LABEL_IMAGE,        /* 图像标签 */
    GUI_LABEL_PICTURE,      /* 图片标签 */
    GUI_LABEL_GRAPH         /* 图形标签 */
} gui_label_types_t;

typedef struct _gui_label {
/// 成员
    gui_widget_t widget;            /* 继承控件：第一个成员 */
    char visable;                   /* 可见 */
    char disabel;                   /* 禁止 */
    
    /* 颜色 */
    GUI_COLOR back_color;               /* 背景色 */
    GUI_COLOR font_color;               /* 字体色 */
    GUI_COLOR disable_color;            /* 禁止字体色 */
    
    /* 字体 */
    gui_font_t *font;

    /* 标签的内容 */
    gui_label_types_t type;         /* 标签类型 */
    char *text;                     /* 文本 */
    uint8_t text_len;               /* 文本长度 */
    uint8_t text_len_max;           /* 文本最大长度 */
    gui_widget_align_t align;    /* 内容对齐 */

/// 函数指针
    /* 内部函数 */
    void (*set_location) (struct _gui_label *, int , int );
    void (*set_size) (struct _gui_label *, int , int );
    void (*set_color) (struct _gui_label *, GUI_COLOR , GUI_COLOR );
    int (*set_text_len) (struct _gui_label *, int );
    void (*set_text) (struct _gui_label *, char *);
    int (*set_font) (struct _gui_label *, char *);
    void (*set_name) (struct _gui_label *, char *);
    void (*set_align) (struct _gui_label *, gui_widget_align_t );
    void (*add) (struct _gui_label *, layer_t *);
    void (*del) (struct _gui_label *);
    void (*show) (struct _gui_label *);
    void (*cleanup) (struct _gui_label *);
    void (*destroy) (struct _gui_label *);
    
    /* 图形 */
} gui_label_t;

gui_label_t *gui_create_label(
    gui_label_types_t type,
    int x,
    int y,
    int width,
    int height
);

void gui_label_destroy(gui_label_t *label);

int gui_label_init(
    gui_label_t *label,
    gui_label_types_t type,
    int x,
    int y,
    int width,
    int height
);

#endif   /* _GUISRV_WIDGET_LABEL_H */
