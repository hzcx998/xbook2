#include <widget/widget.h>
#include <string.h>

static void __set_draw(
    gui_widget_t *widget,
    gui_widget_draw_t draw
) {
    widget->draw_handler = draw;
} 

static void __set_mouse(gui_widget_t *widget,
    gui_widget_mouse_button_t btn_down,
    gui_widget_mouse_button_t btn_up,
    gui_widget_mouse_motion_t motion)
{
    widget->mouse_btn_down = btn_down;
    widget->mouse_btn_up = btn_up;
    widget->mouse_motion = motion;
} 

static void __set_location(
    gui_widget_t *widget,
    int x,
    int y
) {
    widget->x = x;
    widget->y = y;
} 

static void __set_size(
    gui_widget_t *widget,
    int width,
    int height
) {
    widget->width = width;
    widget->height = height;
} 

static void __set_name(
    gui_widget_t *widget,
    char *name
) {
    widget->name_len = strlen(name);
    if (widget->name_len > GUI_WIDGET_NAME_LEN - 1)
        widget->name_len = GUI_WIDGET_NAME_LEN - 1;

    memset(widget->name, 0, GUI_WIDGET_NAME_LEN);
    memcpy(widget->name, name, widget->name_len);
    /* 末尾填'\0' */
    widget->name[widget->name_len] = '\0';
}

static void __show(gui_widget_t *widget)
{
    if (!widget->draw_counter) {
        /* 调用绘图 */
        if (widget->draw_handler)
            widget->draw_handler(widget);
        widget->draw_counter++;
    }
}

static void __add(gui_widget_t *widget, layer_t *layer)
{
    /* 控件添加到图层 */
    list_add_tail(&widget->list, &layer->widget_list_head);
    widget->layer = layer;
}

static void __del(gui_widget_t *widget)
{
    /* 控件从图层删除 */
    list_del(&widget->list);
    widget->layer = NULL;
} 

int gui_widget_mouse_button_up(list_t *list_head, int button, int mx, int my)
{
    int retval = 0;
    /* 控件检测 */
    gui_widget_t *widget;
    list_for_each_owner (widget, list_head, list) {
        if (widget->type == GUI_WIDGET_BUTTON) {
            if (widget->mouse_btn_up)
                retval = widget->mouse_btn_up(widget, button, mx, my);
        }
        if (retval == GUI_WIDGET_EVENT_HANDLED) {
            __show(widget);
            return GUI_WIDGET_EVENT_HANDLED;
        }
    }
    return 0;
}

int gui_widget_mouse_button_down(list_t *list_head, int button, int mx, int my)
{
    int retval = 0;
    /* 控件检测 */
    gui_widget_t *widget;
    list_for_each_owner (widget, list_head, list) {
        if (widget->type == GUI_WIDGET_BUTTON) {
            if (widget->mouse_btn_down)
                retval = widget->mouse_btn_down(widget, button, mx, my);
        }
        if (retval == GUI_WIDGET_EVENT_HANDLED) {
            __show(widget);
            return GUI_WIDGET_EVENT_HANDLED;
        }
    }
    return 0;
}

int gui_widget_mouse_motion(list_t *list_head, int mx, int my)
{
    /* 控件检测 */
    gui_widget_t *widget;
    list_for_each_owner (widget, list_head, list) {
        if (widget->type == GUI_WIDGET_BUTTON) {
            if (widget->mouse_motion)
                widget->mouse_motion(widget, mx, my);
        }
        __show(widget);
    }
    return 0;
}

void gui_widget_init(
    gui_widget_t *widget,
    gui_widget_type_t type,
    char *name
) {
    /* 初始化属性 */
    init_list(&widget->list);
    widget->type = type;
    widget->x = 0;
    widget->y = 0;
    widget->width = GUI_WIDGET_DEFAULT_WIDTH;
    widget->height = GUI_WIDGET_DEFAULT_HEIGHT;
    
    widget->draw_handler = NULL;
    widget->draw_counter = 0;

    widget->mouse_btn_down = NULL;
    widget->mouse_btn_up = NULL;
    widget->mouse_motion = NULL;

    memset(widget->name, 0, GUI_WIDGET_NAME_LEN);
    strcpy(widget->name, name);

    /* 初始化方法，内部函数指针 */
    widget->set_draw = __set_draw;
    widget->set_mouse = __set_mouse;
    widget->set_location = __set_location;
    widget->set_size = __set_size;
    widget->set_name = __set_name;

    widget->show = __show;
    widget->add = __add;
    widget->del = __del;
    
}
