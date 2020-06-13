#include <widget/label.h>
#include <widget/button.h>
#include <guisrv.h>
#include <string.h>
#include <stdio.h>

/**
 * gui_ButtonSetLocation - 设置按钮的位置
 * @Button: 按钮
 * @x: 横坐标
 * @y: 纵坐标
 */
static void __set_location(gui_button_t *button, int x, int y)
{
    button->label.set_location(&button->label, x, y);
}

/**
 * gui_ButtonSetSize - 设置按钮的大小
 * @Button: 按钮
 * 
 */
static void __set_size(gui_button_t *button, int width, int height)
{
    button->label.set_size(&button->label, width, height);
}

/**
 * gui_ButtonSetName - 设置按钮的名字
 * @Button: 按钮
 * @back: 背景色
 * @font: 字体色
 */
static void __set_name(gui_button_t *button, char *name)
{
    button->label.set_name(&button->label, name);
}

static void __set_color(gui_button_t *button, GUI_COLOR back, GUI_COLOR font)
{
    button->label.set_color(&button->label, back, font);
}

static void __set_color3(
    gui_button_t *button,
    GUI_COLOR _default,
    GUI_COLOR _focus,
    GUI_COLOR _select
) {
    button->default_color = _default;
    button->focus_color = _focus;
    button->selected_color = _select;
}

static void __set_align(gui_button_t *button, gui_widget_align_t align)
{
    button->label.set_align(&button->label, align);
}

/**
 * __set_text_len - 设置按钮的最大长度
 * @Button: 按钮
 * @length: 最大长度
 * 
 * 设置文本前必须调用一次
 */
static int __set_text_len(gui_button_t *button, int length)
{
    return button->label.set_text_len(&button->label, length);
}

static void __set_text(gui_button_t *button, char *text)
{
    button->label.set_text(&button->label, text);
}

static void __set_pixmap(
    gui_button_t *button,
    unsigned int width,
    unsigned int height,
    GUI_COLOR *pixmap
) {
    button->label.set_pixmap(&button->label, width, height, pixmap);
}

static void __set_data(gui_button_t *button, void *data)
{
    button->data = data;
}

static void __set_handler(gui_button_t *button, btn_handler_t down, btn_handler_t up)
{
    button->btn_down_handler = down;
    button->btn_up_handler = up;
}

static void __set_image(gui_button_t *button, int width,
    int height, uint8_t *data, GUI_COLOR border, GUI_COLOR fill)
{
    //button->label.set_image(&button->label, width, height, data, border, fill);
}

static void __set_image_align(gui_button_t *button, gui_widget_align_t align)
{
    //button->label.set_image_align(&button->label, align);
}

static void __add(gui_button_t *button, layer_t *layer)
{
    button->label.add(&button->label, layer);
}

/**
 * gui_ButtonDestroySub - 销毁一个按钮，子操作
 * 
 */
static void __cleanup(gui_button_t *button)
{
    /* 先销毁标签 */
    button->label.cleanup(&button->label);
    /* 销毁自己的内容 */
}

/**
 * gui_ButtonDel - 删除按钮
 * @button: 按钮
 * 
 */
static void __del(gui_button_t *button)
{
    button->label.del(&button->label);
}

static void __show(gui_button_t *button)
{
    button->label.show(&button->label);
}

/**
 * gui_button_mouse_down - 鼠标按下事件
 * @widget: 控件
 * @button: 按钮
 * @local_mx: 鼠标横坐标
 * @local_my: 鼠标纵坐标
 */
static int __button_mouse_down(gui_widget_t *widget, int btn, int local_mx, int local_my)
{
    gui_button_t *button = (gui_button_t *) widget;
    gui_label_t *label = &button->label;
    
    if (widget->x <= local_mx && local_mx < widget->x + widget->width &&
        widget->y <= local_my && local_my < widget->y + widget->height &&
        local_mx >= 0 && local_my >= 0) {
        /* 必须是聚焦了才能点击，点击后变成选择状态 */
        if (button->state == GUI_BUTTON_FOCUS) {
            /* 聚焦 */
            button->state = GUI_BUTTON_SELECTED;
            label->back_color = button->selected_color;
            /* 重绘 */
            label->widget.draw_counter = 0;  

            /* 调用处理函数 */
            if (button->btn_down_handler)
                return button->btn_down_handler(button, btn, local_mx, local_my);  
        }
    }
    return 0;
}

/**
 * gui_button_mouse_Down - 鼠标弹起事件
 * @widget: 控件
 * @button: 按钮
 * @local_mx: 鼠标横坐标
 * @local_my: 鼠标纵坐标
 */
static int __button_mouse_up(gui_widget_t *widget, int btn, int local_mx, int local_my)
{
    gui_button_t *button = (gui_button_t *) widget;
    gui_label_t *label = &button->label;
    if (widget->x <= local_mx && local_mx < widget->x + widget->width &&
        widget->y <= local_my && local_my < widget->y + widget->height &&
        local_mx >= 0 && local_my >= 0) {
        /* 如果已经是选择状态，那么弹起后，就会去处理调用函数。由于还在按钮内，所以状态设置为聚焦 */
        if (button->state == GUI_BUTTON_SELECTED) {
            /* 聚焦 */
            button->state = GUI_BUTTON_FOCUS;
            label->back_color = button->focus_color;
            /* 重绘 */
            label->widget.draw_counter = 0;      
            /* 调用处理函数 */
            if (button->btn_up_handler)
                return button->btn_up_handler(button, btn, local_mx, local_my);
        }
    } else {
        /* 如果弹起的时候没在按钮内，只有选择状态才会设置成默认状态。 */
        if (button->state == GUI_BUTTON_SELECTED) {
            /* 弹起的时候没有在按钮内，就设置成默认 */
            /* 默认 */
            button->state = GUI_BUTTON_DEFAULT;
            label->back_color = button->default_color;
            /* 重绘 */
            label->widget.draw_counter = 0;
        }
    }
    return 0;
}

static void __button_mouse_motion(gui_widget_t *widget, int local_mx, int local_my)
{
    gui_button_t *button = (gui_button_t *) widget;
    gui_label_t *label = &button->label;
    if (widget->x <= local_mx && local_mx < widget->x + widget->width &&
        widget->y <= local_my && local_my < widget->y + widget->height &&
        local_mx >= 0 && local_my >= 0) {   /* 鼠标必须为正 */
        
        /* 在移动的时候，如果是默认状态，并且碰撞到，那么就设置成聚焦状态 */
        if (button->state == GUI_BUTTON_DEFAULT) {
            /* 聚焦 */
            button->state = GUI_BUTTON_FOCUS;
            label->back_color = button->focus_color;
            /* 重绘 */
            label->widget.draw_counter = 0;    
        }
    } else {
        /* 没有碰撞，当是聚焦的时候，移动出来了才设置成默认，如果是选择状态，
        就先保持，说不定后面会移动回来，那样就可以继续处理弹起。 */
        if (button->state == GUI_BUTTON_FOCUS) {
            /* 默认 */
            button->state = GUI_BUTTON_DEFAULT;
            label->back_color = button->default_color;
            /* 重绘 */
            label->widget.draw_counter = 0;
        } 
    }
}

/**
 * gui_ButtonDestroy - 销毁一个按钮
 * 
 */
void gui_button_destroy(gui_button_t *button)
{
    __cleanup(button);
    gui_free(button);
}

int gui_button_init(
    gui_button_t *button,
    gui_label_types_t type,
    int x,
    int y,
    int width,
    int height
) {
    /* 初始化label */
    if (gui_label_init(&button->label, type, x, y, width, height)) {
        return -1;
    }
    /* 重载组件信息 */
    button->label.widget.type = GUI_WIDGET_BUTTON;
    button->label.widget.set_name(&button->label.widget, "button");

    /* 初始化按钮自己的信息 */
    button->state = GUI_BUTTON_DEFAULT;
    button->tag = 0;
    button->default_color = GUI_BUTTON_DEFAULT_COLOR;
    button->focus_color = GUI_BUTTON_FOCUS_COLOR;
    button->selected_color = GUI_BUTTON_SELECTED_COLOR;
    button->btn_down_handler = NULL;
    button->btn_up_handler = NULL;
    button->data = NULL;

    /* 设置事件 */
    button->label.widget.set_mouse(&button->label.widget, __button_mouse_down,
        __button_mouse_up, __button_mouse_motion);

    button->set_location = __set_location;
    button->set_size = __set_size;
    button->set_text_len = __set_text_len;
    button->set_text = __set_text;
    button->set_name = __set_name;
    button->set_color = __set_color;
    button->set_color3 = __set_color3;
    button->set_align = __set_align;
    button->set_handler = __set_handler;
    button->set_pixmap = __set_pixmap;
    button->set_data = __set_data;

    button->add = __add;
    button->del = __del;
    button->show = __show;
    button->cleanup = __cleanup;
    button->destroy = gui_button_destroy;

    return 0;
}

/**
 * gui_CreateButton - 创建一个按钮
 * 
 * 成功返回按钮，失败返回NULL
 */
gui_button_t *gui_create_button(
    gui_label_types_t type,
    int x,
    int y,
    int width,
    int height
) {
    gui_button_t *button = gui_malloc(sizeof(gui_button_t));
    if (button == NULL) {
        return NULL;
    }
    
    if (gui_button_init(button, type, x, y, width, height)) {
        gui_free(button);
        return NULL;
    }
    return button;
}
