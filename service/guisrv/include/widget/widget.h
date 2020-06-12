/* 窗口控件之标签 */
#ifndef _GUISRV_WIDGET_H
#define _GUISRV_WIDGET_H

#include <types.h>
#include <stdint.h>
#include <sys/list.h>
#include <layer/color.h>
#include <layer/layer.h>

/* 控件的类型 */
typedef enum _gui_widget_type {
    GUI_WIDGET_LABEL = 0,       /* 标签控件 */
    GUI_WIDGET_BUTTON,          /* 按钮控件 */
} gui_widget_type_t;

/* 控件对齐方式 */
typedef enum _gui_widget_align {
    GUI_WIDGET_ALIGN_LEFT = 0,  /* 左对齐 */
    GUI_WIDGET_ALIGN_CENTER,    /* 居中对齐 */
    GUI_WIDGET_ALIGN_RIGHT,     /* 右对齐 */
} gui_widget_align_t;

/* 标签名长度 */
#define GUI_WIDGET_NAME_LEN     24

/* 默认宽高 */
#define GUI_WIDGET_DEFAULT_WIDTH         40
#define GUI_WIDGET_DEFAULT_HEIGHT        20

/* 事件已经捕捉 */
#define GUI_WIDGET_EVENT_HANDLED    1

struct _gui_widget;

/* 控件绘图指针 */
typedef void (*gui_widget_draw_t) (struct _gui_widget *widget);
typedef int (*gui_widget_mouse_button_t) (struct _gui_widget *widget, int button, int mx, int my);
typedef void (*gui_widget_mouse_motion_t) (struct _gui_widget *widget, int mx, int my);

/* 控件结构体 */
typedef struct _gui_widget {
    list_t list;                        /* 控件组成的链表 */
    uint8_t type;                       /* 控件类型 */
    int x, y;                           /* 位置属性 */
    int width, height;                  /* 大小属性 */
    char name[GUI_WIDGET_NAME_LEN];     /* 控件名 */
    uint8_t name_len;                 /* 控件名字长度 */

    layer_t *layer;                     /* 控件所在的图层 */
    uint8_t draw_counter;               /* 计数器：为0时才绘图，不然则不会图 */
    
    /* 操作方法 */
    

    /* 控件绘图回调 */
    void (*draw_handler) (struct _gui_widget *widget);   
    
    /* 鼠标事件 */
    int (*mouse_btn_down) (struct _gui_widget *widget, int button, int mx, int my);
    int (*mouse_btn_up) (struct _gui_widget *widget,  int button, int mx, int my);
    void (*mouse_motion) (struct _gui_widget *widget, int mx, int my);

    /* 内部方法 */
    void (*set_draw) (  /* 设置绘制函数 */
        struct _gui_widget *,
        gui_widget_draw_t 
    );

    void (*set_mouse) ( /* 设置鼠标事件 */
        struct _gui_widget *,
        gui_widget_mouse_button_t ,
        gui_widget_mouse_button_t ,
        gui_widget_mouse_motion_t
    );

    void (*set_location) ( /* 设置控件位置 */
        struct _gui_widget *,
        int ,
        int 
    );

    void (*set_size) ( /* 设置控件大小 */
        struct _gui_widget *,
        int ,
        int 
    );

    void (*set_name) ( /* 设置控件大小 */
        struct _gui_widget *,
        char *
    );

/// 操作方法
    void (*show) (struct _gui_widget *);

    void (*add) (struct _gui_widget *, layer_t *);
    void (*del) (struct _gui_widget *);

} gui_widget_t;

void gui_widget_init(
    gui_widget_t *widget,
    gui_widget_type_t type,
    char *name
);

int gui_widget_mouse_button_down(list_t *list_head, int button, int mx, int my);
int gui_widget_mouse_button_up(list_t *list_head, int button, int mx, int my);
int gui_widget_mouse_motion(list_t *list_head, int mx, int my);

#endif   /* _GUISRV_WIDGET_H */
