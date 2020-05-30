#include <window/window.h>
#include <window/draw.h>
#include <layer/draw.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/* 管理窗口打开的位置 */


gui_window_t *gui_create_window(
    char *title,
    int x,
    int y,
    int width,
    int height,
    int attr,
    gui_window_t *parent
) {
    if (width <= 0 || height <= 0)
        return NULL;
    int w, h;   /* window width, height */
    int y_off = 0;
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
    gui_window_t *win = sbrk(0);
    if (win == (void *) -1) {
        destroy_layer(layer);
        return NULL;
    }
        
    if (sbrk(sizeof(gui_window_t)) == (void *) -1) {
        destroy_layer(layer);
        return NULL;
    }

    /* 设置窗口结构 */
    win->x = x;
    win->y = y;
    win->x_off = 0;
    win->y_off = y_off;
    win->width = width;
    win->height = height;
    win->attr = attr;
    win->layer = layer;
    win->parent = parent;
    memset(win->title, 0, GUIW_TITLE_LEN);
    if (title) {    /* 有标题才复制 */    
        strcpy(win->title, title);
    }
    init_list(&win->list);
    init_list(&win->child_list);
    
    /* 有父窗口就追加到父窗口的子窗口链表中 */
    if (parent) {
        list_add_tail(&win->list, &parent->child_list);
    }

    /* 绘制窗口本体 */
    if (!(attr & GUIW_NO_TITLE)) {  /* 有标题才绘制标题 */
        layer_draw_rect_fill(win->layer, 0, 0, win->layer->width, GUIW_TITLE_HEIGHT, COLOR_BLUE);
    }

    layer_set_xy(layer, x, y);
    layer_set_z(layer, layer_topest->z);    /* 位于顶层图层下面 */

    return win;
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

    /* 释放窗口占用的空间 */

    /* 释放图层 */
    if (destroy_layer(layer))
        return -1;
    
    return 0;
}

int gui_window_update(gui_window_t *win, int left, int top, int right, int buttom)
{
    layer_refresh(win->layer, win->x_off + left, win->y_off + top, 
        win->x_off + right, win->y_off + buttom);
}

int init_gui_window()
{
    printf("in gui window.\n");
    gui_window_t *win = gui_create_window("xbook2", 20, 20, 320, 240, 0, NULL);
    if (win == NULL) {
        printf("create window failed!\n");
        return -1;
    }
    
    gui_window_put_point(win, 10, 10, COLOR_BLUE);
    gui_window_put_point(win, 15, 13, COLOR_RED);
    gui_window_put_point(win, 13, 15, COLOR_GREEN);

    gui_window_update(win, 10, 10, 15, 15);
    sleep(3);   /* 休眠1s */

    gui_destroy_window(win);
    return 0;
}
