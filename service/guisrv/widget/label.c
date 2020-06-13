
#include <guisrv.h>
#include <string.h>

#include <widget/label.h>
#include <layer/draw.h>

/**
 * __set_location - 设置标签的位置
 * @label: 标签
 * @x: 横坐标
 * @y: 纵坐标
 */
static void __set_location(gui_label_t *label, int x, int y)
{
    label->widget.set_location(&label->widget, x, y);
}

/**
 * __set_size - 设置标签的大小
 * @label: 标签
 * @width: 宽度
 * @height: 高度
 */
static void __set_size(gui_label_t *label, int width, int height)
{
    label->widget.set_size(&label->widget, width, height);
}

/**
 * __set_color - 设置标签的颜色
 * @label: 标签
 * @back: 背景色
 * @font: 字体色
 */
static void __set_color(gui_label_t *label, GUI_COLOR back, GUI_COLOR font)
{
    label->back_color = back;

    switch (label->content.type) {
    case GUI_LABEL_TEXT:
        label->content.text.font_color = font;
        break;
    case GUI_LABEL_PIXMAP:
        break;
    default:
        break;
    }
}

/**
 * __set_text_len - 设置标签的最大长度
 * @label: 标签
 * @length: 最大长度
 * 
 * 设置长文本前必须调用一次
 */
static int __set_text_len(gui_label_t *label, int length)
{
    if (label->content.type != GUI_LABEL_TEXT)
        return -1;

    /* 对长度进行限制 */
    if (length > GUI_MAX_LABEL_TEXT_LEN - 1)
        length = GUI_MAX_LABEL_TEXT_LEN - 1;
    
    char *old = NULL;
    /* 如果已经有数据，那么保存旧数据地址 */
    if (label->content.text.text != NULL) {
        old = label->content.text.text;
    }
    
    /* 分配文本 */
    label->content.text.text = gui_malloc(length);
    if (label->content.text.text == NULL) {
        /* 如果分配失败，恢复原来的数据 */
        label->content.text.text = old;
        return -1;
    }
    /* 分配新数据成功，并且原来也有数据，那么就把原来的数据释放掉 */
    if (old != NULL) {
        gui_free(old);
    }

    label->content.text.text_len_max = length;
    /* 数据清空 */
    memset(label->content.text.text, 0, length);
    return 0;
}

static void __set_align(gui_label_t *label, gui_widget_align_t align)
{
    label->align = align;
}

/**
 * __set_text - 设置标签的文本
 * @label: 标签
 * @text: 文本
 */
static void __set_text(gui_label_t *label, char *text)
{
    if (label->content.type != GUI_LABEL_TEXT)
        return;

    /* 文本类型 */
    label->content.type = GUI_LABEL_TEXT;

    int length = strlen(text);
    /* 修复长度 */
    if (length > label->content.text.text_len_max - 1)
        length = label->content.text.text_len_max - 1;
    
    /* 设置文本 */
    memcpy(label->content.text.text, text, length);
    /* 末尾填'\0' */
    label->content.text.text[length] = '\0';
    label->content.text.text_len = length;
    
}

/**
 * __set_font - 设置标签的字体
 * @label: 标签
 * @fontName: 字体名
 */
static int __set_font(gui_label_t *label, char *font_name)
{
    if (label->content.type != GUI_LABEL_TEXT)
        return -1;

    gui_font_t *font = gui_get_font(font_name);
    
    /* 找到才设置，没找到就不设置 */
    if (!font)
        return -1;

    label->content.text.font = font;
    return 0;
}

static void __set_name(gui_label_t *label, char *name)
{
    label->widget.set_name(&label->widget, name);
}

static void __set_pixmap(
    gui_label_t *label,
    unsigned int width,
    unsigned int height,
    GUI_COLOR *data
) {
    if (label->content.type != GUI_LABEL_PIXMAP)
        return;

    label->content.pixmap.width = width;
    label->content.pixmap.height = height;
    label->content.pixmap.data = data;
}

/**
 * __add - 添加标签
 * @label: 标签
 * @layer: 容器
 * 
 * 添加标签到容器上
 */
static void __add(gui_label_t *label, layer_t *layer)
{
    label->widget.add(&label->widget, layer);
}

/**
 * __del - 删除标签
 * @label: 标签
 * 
 * 从图层上删除标签
 */
static void __del(gui_label_t *label)
{
    label->widget.del(&label->widget);
}

/**
 * __widget_show - 显示标签
 * @label: 标签
 */
static void __widget_show(gui_widget_t *widget)
{
    if (!widget->layer)
        return;

    /* 转换成标签 */
    gui_label_t *label = (gui_label_t *)widget;
    /* 先绘制背景 */
    layer_draw_rect_fill(widget->layer,
        widget->x, widget->y, widget->width, widget->height, label->back_color);
    
    /* 可见才绘制 */
    if (label->visable) {
        int x, y;
        if (label->content.type == GUI_LABEL_TEXT) {
            y = widget->y + widget->height / 2 - label->content.text.font->height / 2;
            
            switch (label->align) {
            case GUI_WIDGET_ALIGN_LEFT:
                x = widget->x;
                break;
            case GUI_WIDGET_ALIGN_CENTER:
                x = widget->x + widget->width / 2 - (label->content.text.text_len * label->content.text.font->width) / 2 ;
                break;
            case GUI_WIDGET_ALIGN_RIGHT:
                x = widget->x + widget->width - (label->content.text.text_len * label->content.text.font->width);
                break;
            default:
                break;
            }
            GUI_COLOR color;
            /* 选择显示颜色 */
            if (!label->disabel) {
                color = label->content.text.font_color;
            } else {
                color = label->content.text.disable_color;
            }
            /* 再绘制文本 */
            layer_draw_text_ex(widget->layer, x, y, label->content.text.text, color, label->content.text.font);
        } else if (label->content.type == GUI_LABEL_PIXMAP) {
            y = widget->y + widget->height / 2 - label->content.pixmap.height / 2;
            
            switch (label->align) {
            case GUI_WIDGET_ALIGN_LEFT:
                x = widget->x;
                break;
            case GUI_WIDGET_ALIGN_CENTER:
                x = widget->x + widget->width / 2 - label->content.pixmap.width / 2;
                break;
            case GUI_WIDGET_ALIGN_RIGHT:
                x = widget->x + widget->width - label->content.pixmap.width;
                break;
            default:
                break;
            }

            /* 绘制数据 */
            layer_draw_bitmap(widget->layer, x, y, 
                label->content.pixmap.width, label->content.pixmap.height, label->content.pixmap.data);
        }
    }
    /* 刷新标签 */
    layer_refresh(widget->layer, widget->x, widget->y,
        widget->x + widget->width, widget->y + widget->height);
}

static void __show(gui_label_t *label)
{
    __widget_show(&label->widget);
}

/**
 * __cleanup - 清除标签内容
 * 
 */
static void __cleanup(gui_label_t *label)
{
    switch (label->content.type) {
    case GUI_LABEL_TEXT:
        /* 释放文本内容 */
        if (label->content.text.text)
            gui_free(label->content.text.text);
        break;
    case GUI_LABEL_PIXMAP:
        break;
    default:
        break;
    }
}

/**
 * gui_label_destroy - 销毁一个标签
 * 
 */
void gui_label_destroy(gui_label_t *label)
{
    __cleanup(label);
    gui_free(label);
}

/**
 * gui_label_init - 初始化一个标签
 * @label: 标签
 * 成功返回标签，失败返回NULL
 */
int gui_label_init(
    gui_label_t *label,
    gui_label_types_t type,
    int x,
    int y,
    int width,
    int height
) {
    /* 进行默认的初始化 */
    label->visable = 1;
    label->disabel = 0;
    label->back_color = GUI_LABEL_BACK_COLOR;
    label->align = GUI_WIDGET_ALIGN_LEFT;   /* 默认左对齐 */

    label->content.type = type;
    switch (type) {
    case GUI_LABEL_TEXT:
        
        label->content.text.text_len_max = GUI_DEFAULT_LABEL_TEXT_LEN;
        label->content.text.text = gui_malloc(label->content.text.text_len_max);
        if (label->content.text.text == NULL) {
            return -1;
        }
        label->content.text.font_color = GUI_LABEL_FONT_COLOR;
        label->content.text.disable_color = GUI_LABEL_DISABEL_COLOR;
        /* 文本内容 */
        memset(label->content.text.text, 0, label->content.text.text_len_max);
        strcpy(label->content.text.text, " ");
        label->content.text.text_len = strlen(label->content.text.text);
        label->content.text.font = current_font; /* 设置系统字体 */
        break;
    case GUI_LABEL_PIXMAP:
        /* 初始化像素图 */
        label->content.pixmap.width = 0;
        label->content.pixmap.height = 0;
        label->content.pixmap.data = NULL;
        break;
    default:
        break;
    }
    
    /* 初始化widget */
    gui_widget_init(&label->widget, GUI_WIDGET_LABEL, "label");
    /* 设置控件属性 */
    label->widget.set_location(&label->widget, x, y);
    if (width > 0 && height > 0) {  
        label->widget.set_size(&label->widget, width, height);    
    }
    label->widget.set_draw(&label->widget, __widget_show);
    
    /* 初始化方法 */
    label->set_location = __set_location;
    label->set_size = __set_size;
    label->set_color = __set_color;
    label->set_text_len = __set_text_len;
    label->set_text = __set_text;
    label->set_font = __set_font;
    label->set_name = __set_name;
    label->set_align = __set_align;
    label->set_pixmap = __set_pixmap;

    label->add = __add;
    label->del = __del;
    label->show = __show;
    label->cleanup = __cleanup;
    label->destroy = gui_label_destroy;
    
    return 0;
}

/**
 * gui_create_label - 创建一个标签
 * 
 * 成功返回标签，失败返回NULL
 */
gui_label_t *gui_create_label(
    gui_label_types_t type,
    int x,
    int y,
    int width,
    int height
) {
    gui_label_t *label = gui_malloc(sizeof(gui_label_t));
    if (label == NULL)
        return NULL;
    if (gui_label_init(label, type, x, y, width, height)) {
        gui_free(label);
        return NULL;
    }
    return label;
}
