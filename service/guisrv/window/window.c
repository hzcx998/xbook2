#include <window/window.h>
#include <window/draw.h>
#include <layer/draw.h>
#include <drivers/screen.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <guisrv.h>

#include <environment/desktop.h>
#include <environment/mouse.h>

/* 图层显示链表 */
extern list_t layer_show_list_head;

/* 当前活动的窗口 */
gui_window_t *window_current;

/* 下一个新窗口的id */
volatile unsigned int next_window_id;

/* 窗口链表 */
list_t window_list_head;

/* 窗口高速缓存表 */
gui_window_t *window_cache_table[GUIW_CACHE_TABLE_SIZE];

/**
 * 添加窗口到窗口缓存表 
 */
int gui_window_cache_add(gui_window_t *win)
{
    int i;

    /* 如果已经在缓存表中则返回 */
    for (i = 0; i < GUIW_CACHE_TABLE_SIZE; i++)
        if (window_cache_table[i] == win)
            return 0;

    for (i = 0; i < GUIW_CACHE_TABLE_SIZE; i++)
        if (window_cache_table[i] == NULL)
            break;

    if (i >= GUIW_CACHE_TABLE_SIZE)
        return -1;
         
    window_cache_table[i] = win;
    return 0;
}

/**
 * 从窗口缓存表删除窗口 
 */
int gui_window_cache_del(gui_window_t *win)
{
    int i;
    for (i = 0; i < GUIW_CACHE_TABLE_SIZE; i++) {
        if (window_cache_table[i] == win) {
            window_cache_table[i] = NULL;
            return 0;
        }
    }
    return -1;    
}

/**
 * 从窗口缓存表中查找窗口 
 */
gui_window_t *gui_window_cache_find(unsigned int wid)
{
    int i;
    for (i = 0; i < GUIW_CACHE_TABLE_SIZE; i++) {
        if (window_cache_table[i] && window_cache_table[i]->id == wid) {
            return window_cache_table[i];
        }
    }
    return NULL;    
}

void window_btn_down_handler(gui_button_t *button, int btn, int local_mx, int local_my)
{
    printf("[button] down handler %d, %d, %d\n", btn, local_mx, local_my);

    switch (button->tag)
    {
    case 1:
        printf("[button] tag close\n");

        break;
    case 2:
        printf("[button] tag maxim\n");
        
        break;
    case 3:
        printf("[button] tag minim\n");
        
        break;
    
    default:
        break;
    }

}

void window_btn_up_handler(gui_button_t *button, int btn, int local_mx, int local_my)
{
    printf("[button] up handler %d, %d, %d\n", btn, local_mx, local_my);
    gui_label_t *label = &button->label;
    gui_widget_t *widget = &label->widget;


    switch (button->tag)
    {
    case 1:
        printf("[button] tag close\n");
        /* 发送关闭窗口信息给窗口 */

        break;
    case 2:
        printf("[button] tag maxim\n");
        /* 对窗口最大化 */

        break;
    case 3:
        printf("[button] tag minim\n");
        if (widget->layer->extension) {
            gui_window_hide(widget->layer->extension);
        }
        break;
    
    default:
        break;
    }
}

int window_create_title_bar(gui_window_t *win, int width, char *title)
{
///
    /* 创建标题文本 */
    win->text_title = gui_create_label(GUI_LABEL_TEXT, 0, 0, width, GUIW_TITLE_HEIGHT);
    if (win->text_title == NULL) {
        return -1;
    }
    /* 设置标题内容 */
    if (title)
        win->text_title->set_text(win->text_title, title);
    else
        win->text_title->set_text(win->text_title, "window");
    win->text_title->set_align(win->text_title, GUI_WIDGET_ALIGN_CENTER); /* 居中对齐 */
    win->text_title->add(win->text_title, win->layer); /* 添加到图层 */
///
    /* 计算一下按钮的数量 */
    int btn_pos = 24;
    if (win->attr & GUIW_BTN_CLOSE) { /* 创建“关闭”按钮 */

        win->btn_close = gui_create_button(GUI_LABEL_TEXT, width - btn_pos, 0, 24, 24);
        if (win->btn_close == NULL) {
            goto wctb_free_title;
        }
        win->btn_close->tag = 1;

        win->btn_close->set_handler(win->btn_close, window_btn_down_handler,
            window_btn_up_handler);
        win->btn_close->set_align(win->btn_close, GUI_WIDGET_ALIGN_CENTER);
        win->btn_close->set_text(win->btn_close, "X");
        win->btn_close->set_color3(win->btn_close, win->title_bar_color,
            win->title_bar_color + 0x202020, win->title_bar_color + 0x101010);
        win->btn_close->add(win->btn_close, win->layer);
        btn_pos += 24;
    }
///
     if (win->attr & GUIW_BTN_MAXIM) { /* 创建“最大化”按钮 */
        win->btn_maxim = gui_create_button(GUI_LABEL_TEXT, width - btn_pos, 0, 24, 24);
        if (win->btn_maxim == NULL) {
            goto wctb_free_close;
        }
        win->btn_maxim->tag = 2;
        
        win->btn_maxim->set_handler(win->btn_maxim, window_btn_down_handler,
            window_btn_up_handler);
        win->btn_maxim->set_align(win->btn_maxim, GUI_WIDGET_ALIGN_CENTER);
        win->btn_maxim->set_color3(win->btn_maxim, win->title_bar_color,
            win->title_bar_color + 0x202020, win->title_bar_color + 0x101010);
        win->btn_maxim->set_text(win->btn_maxim, "O");
        win->btn_maxim->add(win->btn_maxim, win->layer);

        btn_pos += 24;
    }
///
    if (win->attr & GUIW_BTN_MINIM) { /* 创建“最小化”按钮 */
        win->btn_minim = gui_create_button(GUI_LABEL_TEXT, width - btn_pos, 0, 24, 24);
        if (win->btn_minim == NULL) {
            goto wctb_free_maxim;
        }
        win->btn_minim->tag = 3;
        
        win->btn_minim->set_handler(win->btn_minim, window_btn_down_handler,
            window_btn_up_handler);
        win->btn_minim->set_align(win->btn_minim, GUI_WIDGET_ALIGN_CENTER);
        win->btn_minim->set_color3(win->btn_minim, win->title_bar_color,
            win->title_bar_color + 0x202020, win->title_bar_color + 0x101010);
        win->btn_minim->set_text(win->btn_minim, "_");
        win->btn_minim->add(win->btn_minim, win->layer);
        btn_pos += 24;
    }
    
    /* 设置标题盒子 */
    ENV_BOX_INIT(win->title_box, 0, 0, width - btn_pos, 24);

///
    return 0;
wctb_free_title:
    win->text_title->del(win->text_title);
    win->text_title->destroy(win->text_title);
wctb_free_close:
    win->btn_close->del(win->btn_close);
    win->btn_close->destroy(win->btn_close);
wctb_free_maxim:
    win->btn_maxim->del(win->btn_maxim);
    win->btn_maxim->destroy(win->btn_maxim);
    return -1;
}

gui_window_t *gui_create_window(
    char *title,
    int x,
    int y,
    int width,
    int height,
    GUI_COLOR background,
    int attr,
    gui_window_t *parent
) {
    if (width <= 0 || height <= 0)
        return NULL;
    int w, h;   /* window width, height */
    int y_off = 0;

    if (!attr)
        attr = GUIW_BTN_DEFAULT;

    /* 自动添加关闭按钮 */
    attr |= GUIW_BTN_CLOSE;

    /* 根据属性设置窗口内容 */
    if (attr & GUIW_NO_TITLE) { /* 没有标题 */
        attr &= ~GUIW_BTN_MASK; /* 清除按钮控制 */
        
        /* 窗口=窗体 */
        w = width;
        h = height;
        
    } else {    
        /* 窗口=窗体+标题 */
        w = width;
        h = height + GUIW_TITLE_HEIGHT;
        y_off = GUIW_TITLE_HEIGHT;
    }

    /* alloc a new layer first */
    layer_t *layer = create_layer(w, h);
    if (layer == NULL) {
        return NULL;
    }
    
    /* 创建一个窗口 */
    gui_window_t *win = gui_malloc(sizeof(gui_window_t));
    if (win == NULL) {
        destroy_layer(layer);
        return NULL;
    }

    /* 设置窗口结构 */
    win->id = next_window_id++;
    win->x = x;
    win->y = y;
    win->x_off = 0;
    win->y_off = y_off;
    win->width = width;
    win->height = height;
    win->window_size = width * height * sizeof(GUI_COLOR);
    win->attr = attr;
    win->layer = layer;
    win->parent = parent;
    win->title_color = GUIW_TITLE_TEXT_ON_COLOR;
    win->title_bar_color = GUIW_TITLE_BAR_ON_COLOR;
    win->shmid = -1;
    win->mapped_addr = NULL;
    win->start_off = 0;
    win->display_id = 0;

    layer->extension = (void *) win;

    if (!(attr & GUIW_NO_TITLE)) { /* 有标题才创建 */
        if (window_create_title_bar(win, width, title)) {
            gui_free(win);
            destroy_layer(layer);
            return NULL;
        }
    }

    init_list(&win->list);
    init_list(&win->child_list);
    
    /* 有父窗口就追加到父窗口的子窗口链表中 */
    if (parent) {
        list_add_tail(&win->list, &parent->child_list);
    }

    /* 添加到窗口链表 */
    list_add_tail(&win->window_list, &window_list_head);
    
    /* 设置标题盒子 */
    ENV_BOX_INIT(win->body_box, win->x_off, win->y_off,
        win->x_off + win->layer->width, win->height + win->y_off);

    /* 绘制窗口本体 */
    layer_draw_rect_fill(win->layer, win->x_off, win->y_off, win->layer->width, win->height, background);

    layer_set_xy(layer, x, y);
    
    return win;
}

/*
 * 把一个隐藏的窗口显示出来
 */
int gui_window_show(gui_window_t *win)
{
    if (!win)
        return -1;
    layer_set_z(win->layer, layer_topest->z);    /* 位于顶层图层下面 */
    gui_window_focus(win);
    return 0;
}

int gui_destroy_window(gui_window_t *win)
{
    if (!win)
        return -1;
    layer_t *layer = win->layer;
    /* 隐藏图层 */
    layer_set_z(layer, -1);
    /* 关闭所有子窗口 */
    gui_window_t *child, *next;
    list_for_each_owner_safe (child, next, &win->child_list, list) {
        gui_destroy_window(child);  /* 递归销毁子窗口 */
    }

    /* 脱离父窗口 */
    if (win->parent) {
        list_del_init(&win->list);
        win->parent = NULL;
    }
    /* 脱离窗口链表 */
    list_del(&win->window_list);

    /* 释放窗口占用的空间 */

    /* 释放图层 */
    if (destroy_layer(layer))
        return -1;

    /* 切换到顶层窗口 */
    gui_window_switch(gui_window_topest());

    return 0;
}

int gui_window_hide(gui_window_t *win)
{
    if (!win)
        return -1;
    
    /* 隐藏当前窗口 */
    layer_set_z(win->layer, -1);

    /* 切换到顶层窗口 */
    gui_window_switch(gui_window_topest());
    return -1;
}

int gui_window_update(gui_window_t *win, int left, int top, int right, int buttom)
{
    layer_refresh(win->layer, win->x_off + left, win->y_off + top, 
        win->x_off + right, win->y_off + buttom);
    return 0;
}

void gui_window_title_switch(gui_window_t *window, bool on)
{
    if (on) {
        window->title_bar_color = GUIW_TITLE_BAR_ON_COLOR;
        window->title_color = GUIW_TITLE_TEXT_ON_COLOR;
    } else {
        window->title_bar_color = GUIW_TITLE_BAR_OFF_COLOR;
        window->title_color = GUIW_TITLE_TEXT_OFF_COLOR;
    }
    /* 绘制标题栏背景 */

    /* 设置颜色，并显示 */
    window->text_title->set_color(window->text_title, window->title_bar_color, window->title_color);
    
    window->text_title->show(window->text_title);

    if (window->attr & GUIW_BTN_CLOSE) { /* 有“关闭”按钮 */
        if (window->btn_close) {
            window->btn_close->set_color(window->btn_close, window->title_bar_color, window->title_color);
            window->btn_close->set_color3(window->btn_close, window->title_bar_color,
                window->title_bar_color + 0x202020, window->title_bar_color + 0x101010);
            window->btn_close->show(window->btn_close);    
        }
    }
    if (window->attr & GUIW_BTN_MAXIM) { /* 有“最大化”按钮 */
        window->btn_maxim->set_color(window->btn_maxim, window->title_bar_color, window->title_color);

        window->btn_maxim->set_color3(window->btn_maxim, window->title_bar_color,
                window->title_bar_color + 0x202020, window->title_bar_color + 0x101010);
        window->btn_maxim->show(window->btn_maxim);
    }
    if (window->attr & GUIW_BTN_MINIM) { /* 有“最小化”按钮 */
        window->btn_minim->set_color(window->btn_minim, window->title_bar_color, window->title_color);
        window->btn_minim->set_color3(window->btn_minim, window->title_bar_color,
                window->title_bar_color + 0x202020, window->title_bar_color + 0x101010);
        window->btn_minim->show(window->btn_minim);
    }
}

/**
 * gui_window_switch - 切换图形窗口
 * @window: 窗口
 * 
 */
void gui_window_focus(gui_window_t *window)
{
    if (window == current_window) {
        return;
    }
    if (window) {   /* 切换到指定窗口 */
        /* 修改聚焦窗口 */
        
        /* 移除聚焦 */
        if (current_window && !(current_window->attr & GUIW_NO_TITLE)) {  /* 有标题，就修改标题栏颜色 */
            /* 修改标题栏颜色 */
            gui_window_title_switch(current_window, false);
        }

        /* 绑定聚焦 */
        if (!(window->attr & GUIW_NO_TITLE)) {  /* 有标题，就修改标题栏颜色 */
            /* 修改标题栏颜色 */
            gui_window_title_switch(window, true);
        }

        /* 刷新一下窗体部分 */
        gui_window_update(window, 0, 0 , window->width, window->height);

        current_window = window;
    }
}

gui_window_t *gui_window_get_by_id(unsigned int wid)
{
    gui_window_t *win;
    list_for_each_owner (win, &window_list_head, window_list) {
        if (win->id == wid) {
            return win;
        }
    }
    return NULL;
}


/**
 * 查找最顶层的窗口 
 * 
 */
gui_window_t *gui_window_topest()
{
    layer_t *layer;
    /* 查看点击的位置，看是否是一个 */
    list_for_each_owner_reverse (layer, &layer_show_list_head, list) {
        if (layer == layer_topest)
            continue;
        if (layer->extension == NULL)
            continue;
        return (gui_window_t *) layer->extension;
    }
    return NULL;
}

/**
 * gui_window_switch - 切换图形窗口
 * @window: 窗口
 * 
 */
void gui_window_switch(gui_window_t *window)
{
    if (window == current_window) {
        return;
    }
    if (window) {   /* 切换到指定窗口 */
        /* 修改窗口的高度为最高 */
        if (window->layer) {
            /* 切换到鼠标下面 */
            layer_set_z(window->layer, layer_topest->z - 1);
            
            gui_window_focus(window);
        }
    }
}

int init_gui_window()
{
    window_current = NULL;
    next_window_id = 0;
    init_list(&window_list_head);

    /* 初始化窗口缓存表 */
    int i;
    for (i = 0; i < GUIW_CACHE_TABLE_SIZE; i++)
        window_cache_table[i] = NULL;

    if (init_env_desktop()) {
        printf("[desktop ] %s: init desktop environment failed!\n", SRV_NAME);
        return -1;
    }
    
    gui_window_draw_text(env_desktop.window, 0,0,"abcdefghijklmnopqrstuvwxyz0123456789",
        COLOR_RED);

    gui_window_draw_text_ex(env_desktop.window, 0,16,"abcdefghijklmnopqrstuvwxyz0123456789",
        COLOR_RED, gui_get_font("simsun"));
    
    gui_window_update(env_desktop.window, 0,0, 400, 16*2);

    /* 创建测试窗口 */
    gui_window_t *win = gui_create_window("xbook2", 20, 20, 320, 240, COLOR_WHITE, 0, NULL);
    if (win == NULL) {
        printf("create window failed!\n");
        return -1;
    }
    
    gui_window_put_point(win, 10, 10, COLOR_BLUE);
    gui_window_put_point(win, 15, 13, COLOR_RED);
    gui_window_put_point(win, 13, 15, COLOR_GREEN);

    gui_window_update(win, 10, 10, 15, 15);

    gui_window_draw_line(win, 20, 20, 100, 150, COLOR_YELLOW);
    
    gui_window_draw_rect(win, 20, 20, 100, 150, COLOR_BLUE);
    gui_window_draw_rect_fill(win, 100, 20, 100, 150, COLOR_GREEN);
    
    gui_window_show(win);

    gui_window_update(win, 20, 20, 300, 200);

    win = gui_create_window("xbook3", 500, 100, 240, 360, COLOR_BLACK, 0, NULL);
    if (win == NULL) {
        printf("create window failed!\n");
        return -1;
    }
    
    gui_window_put_point(win, 10, 10, COLOR_BLUE);
    gui_window_put_point(win, 15, 13, COLOR_RED);
    gui_window_put_point(win, 13, 15, COLOR_GREEN);

    gui_window_update(win, 10, 10, 15, 15);

    gui_window_draw_line(win, 20, 20, 100, 150, COLOR_YELLOW);
    
    gui_window_draw_rect(win, 20, 20, 100, 150, COLOR_BLUE);
    gui_window_draw_rect_fill(win, 100, 20, 100, 150, COLOR_GREEN);
    
    gui_window_draw_rect_fill(win, 100, 20, 100, 150, COLOR_GREEN);
    
    gui_window_draw_text(win, 100, 200, "hello, world!", COLOR_RED);

    gui_window_draw_text_ex(win, 100, 220, "hello, world!", COLOR_RED, gui_get_font("Simsun"));
    
    gui_window_show(win);
    gui_window_update(win, 20, 20, 300, 300);
    
    //sleep(3);   /* 休眠1s */

    //gui_destroy_window(win);
    return 0;
}
