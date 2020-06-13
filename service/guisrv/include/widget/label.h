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
    GUI_LABEL_UNKNOWN  = 0, /* 未知标签 */
    GUI_LABEL_TEXT,         /* 文本标签 */
    GUI_LABEL_PIXMAP,       /* 图像标签 */
} gui_label_types_t;

/* 像素图 */
typedef struct _gui_pixmap {
    gui_label_types_t type;     /* 类型 */
    unsigned int width, height; /* 图像大小 */
    GUI_COLOR *data;            /* 图像数据 */
} gui_pixmap_t;

/* 文本 */
typedef struct _gui_text {
    gui_label_types_t type;     /* 类型 */
    GUI_COLOR font_color;       /* 字体色 */
    GUI_COLOR disable_color;    /* 禁止字体色 */
    gui_font_t *font;           /* 字体 */
    char *text;                 /* 文本 */
    uint8_t text_len;           /* 文本长度 */
    uint8_t text_len_max;       /* 文本最大长度 */
} gui_text_t;

typedef union _gui_label_content {
    gui_label_types_t type;     /* 类型 */
    gui_pixmap_t    pixmap;     /* 像素图 */
    gui_text_t      text;       /* 文本 */
} gui_label_content_t;

typedef struct _gui_label {

/// 成员
    gui_widget_t widget;            /* 继承控件：第一个成员 */
    char visable;                   /* 可见 */
    char disabel;                   /* 禁止 */
    GUI_COLOR back_color;               /* 背景色 */
    gui_widget_align_t align;    /* 内容对齐 */
    gui_label_content_t content;    /* 标签的内容 */

/// 共有操作
    void (*set_location) (struct _gui_label *, int , int );
    void (*set_size) (struct _gui_label *, int , int );
    void (*set_name) (struct _gui_label *, char *);
    void (*set_align) (struct _gui_label *, gui_widget_align_t );
    
    void (*add) (struct _gui_label *, layer_t *);
    void (*del) (struct _gui_label *);
    void (*show) (struct _gui_label *);
    void (*cleanup) (struct _gui_label *);
    void (*destroy) (struct _gui_label *);
    
/// 文本操作
    void (*set_color) (struct _gui_label *, GUI_COLOR , GUI_COLOR );
    int (*set_text_len) (struct _gui_label *, int );
    int (*set_font) (struct _gui_label *, char *);
    void (*set_text) (struct _gui_label *, char *);

/// 像素图操作
    void (*set_pixmap) (struct _gui_label *, unsigned int , unsigned int , GUI_COLOR *);

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
