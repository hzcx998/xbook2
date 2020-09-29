#include <winctl.h>
#include <icon.h>
#include <taskbar.h>
#include <desktop.h>
#include <stdio.h>
#include <malloc.h>

#define DEBUG_WINCTL
#if 0
/* 默认的图标数据 */
static g_color_t winctl_icon_data[12 * 12] = {
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};
#endif

winctl_manager_t winctl_manager;
winctl_t *winctl_last;  /* 上一个窗口控制 */

int winctl_lbtn_handler(void *arg)
{
    g_touch_t *gtch = (g_touch_t *) arg;
    winctl_t *winctl = (winctl_t *) gtch->extension;

    g_msg_t m;
    /* 当有窗口创建的时候，就会有聚焦消息，那么winctl_last肯定存在。 */

    if (winctl_last == winctl) { 
        /* 隐藏/显示 */
        if (winctl->ishidden) { /* 是一个隐藏的窗口，就显示 */
            m.id = GM_SHOW;
            m.target = winctl->layer;
            m.data0 = 0;
            g_send_msg(&m);
        } else {    /* 已经是聚焦的窗口，那么再次点击就隐藏 */
            m.id = GM_HIDE;
            m.target = winctl->layer;
            m.data0 = 0;
            g_send_msg(&m);
        }
    } else {
        /* 显示 */
        if (winctl->ishidden) { /* 是一个隐藏的窗口，就显示 */
            m.id = GM_SHOW;
            m.target = winctl->layer;
            m.data0 = 0;
            g_send_msg(&m);
        } else { /* 需要切换到一个新的窗口 */
            g_layer_focus(winctl->layer); /* 直接聚焦图层 */
        }
    }
    return 0;
}

int winctl_rbtn_handler(void *arg)
{
    g_touch_t *gtch = (g_touch_t *) arg;
    winctl_t *winctl = (winctl_t *) gtch->extension;

    /* 发送退出消息给窗口 */
    g_msg_t m;
    m.id = GM_QUIT;
    m.target = winctl->layer;
    g_post_msg(&m);
    winctl_last = NULL;

    return 0;
}

winctl_t *create_winctl(g_layer_t layer)
{
    if (layer < 0)
        return NULL;

    winctl_t *winctl = malloc(sizeof(winctl_t));
    if (winctl == NULL)
        return NULL;

    winctl->button = g_new_touch(WINCTL_ICON_SIZE + 4, WINCTL_ICON_SIZE + 4);

    if (winctl->button == NULL) {
        free(winctl);
        return NULL;
    }
    
    /* 创建图标位图 */
    winctl->button->bmp = g_new_bitmap(ICON_SIZE, ICON_SIZE);
    if (winctl->button->bmp == NULL) {
        g_del_touch(winctl->button);
        free(winctl);
        return NULL;
    }

    g_touch_set_color(winctl->button, winctl_manager.back_color, winctl_manager.back_color + 0x00404040);
    g_touch_set_handler(winctl->button, 0, winctl_lbtn_handler);
    g_touch_set_handler(winctl->button, 2, winctl_rbtn_handler);
    
    g_touch_set_layer(winctl->button, taskbar.layer, &taskbar.touch_list);
    winctl->button->extension = winctl;
    
    winctl->layer = layer;
    winctl->ishidden = false;   /* 默认是显示 */
    winctl->isfocus = false; /* 默认不聚焦 */
    list_add_tail(&winctl->list, &winctl_manager.winctl_list_head);

    /* 使用默认的位图 */
    g_rectfill(winctl->button->bmp, 0, 0, ICON_SIZE/2, ICON_SIZE/2, GC_RED);
    g_rectfill(winctl->button->bmp, ICON_SIZE/2, 0, ICON_SIZE/2, ICON_SIZE/2, GC_YELLOW);
    g_rectfill(winctl->button->bmp, 0, ICON_SIZE/2, ICON_SIZE/2, ICON_SIZE/2, GC_BLUE);
    g_rectfill(winctl->button->bmp, ICON_SIZE/2, ICON_SIZE/2, ICON_SIZE/2, ICON_SIZE/2, GC_GREEN);

    return winctl;
}

int destroy_winctl(winctl_t *winctl)
{
    if (!winctl)
        return -1;
    g_del_bitmap(winctl->button->bmp);
    winctl->button->bmp = NULL;

    list_del(&winctl->list);
    if (g_del_touch(winctl->button) < 0)
        return -1;
    free(winctl);
    return 0;
}

winctl_t *winctl_find_by_layer(g_layer_t layer)
{
    winctl_t *winctl;
    list_for_each_owner (winctl, &winctl_manager.winctl_list_head, list) {
        if (winctl->layer == layer)
            return winctl;
    }
    return NULL;
}

void winctl_get_focus(winctl_t *winctl)
{
    winctl_last = winctl;
    winctl->isfocus = true;
    /* draw */
    winctl->button->color_idle = winctl_manager.active_color;
}

int winctl_set_icon(winctl_t *winctl)
{
    char icon_path[MAX_PATH] = {0,};
    /* 获取图标路径 */
    if (g_get_icon_path(winctl->layer, icon_path, MAX_PATH) < 0) {
        return -1;
    }
    /* 追加到窗口控制上 */
    printf("[winctl]: set icon path:%s\n", icon_path);
    
    g_bitmap_clear(winctl->button->bmp);

    /* 如果加载图片失败，就使用默认位图 */
    if (png_load_bitmap(icon_path, winctl->button->bmp->buffer) < 0) {
        /* 使用默认的位图 */
        g_rectfill(winctl->button->bmp, 0, 0, ICON_SIZE/2, ICON_SIZE/2, GC_RED);
        g_rectfill(winctl->button->bmp, ICON_SIZE/2, 0, ICON_SIZE/2, ICON_SIZE/2, GC_YELLOW);
        g_rectfill(winctl->button->bmp, 0, ICON_SIZE/2, ICON_SIZE/2, ICON_SIZE/2, GC_BLUE);
        g_rectfill(winctl->button->bmp, ICON_SIZE/2, ICON_SIZE/2, ICON_SIZE/2, ICON_SIZE/2, GC_GREEN);
    }
    
    /* 绘制 */
    g_touch_paint(winctl->button);
    return 0; 
}

    
void winctl_lost_focus(winctl_t *winctl)
{
    winctl->isfocus = false;
    /* draw */
    winctl->button->color_idle = winctl_manager.back_color;
}

/**
 * 当窗口控制为NULL时，则重绘整个窗口控制
 * 不然就绘制指定的窗口控制
 */
void winctl_paint(winctl_t *winctl)
{
    if (winctl) {
        g_touch_paint(winctl->button);
    } else {

        /* 先刷新背景，再显示 */
        g_rectfill(taskbar.render, 0, 0, taskbar.width, taskbar.height, taskbar.color);
        g_bitmap_sync(taskbar.render, winctl_manager.layer, 0, 0);

        int x = 4, y = 4;
        g_touch_t *gtch;
        list_for_each_owner (gtch, &taskbar.touch_list, list) {
            /* 设置控件位置 */
            g_touch_set_location(gtch, x, y);

            g_touch_paint(gtch);
            
            x += WINCTL_ICON_SIZE + 4 + 2;
            
            /* 显示到最低端就不显示 */
            if (x >= taskbar.width - WINCTL_ICON_SIZE - 4)
                break;
        }
    }
}

int init_winctl_manager(g_layer_t layer)
{
    winctl_manager.layer = layer;
    winctl_manager.back_color = WINCTL_BACK_COLOR;
    winctl_manager.active_color = WINCTL_ACTIVE_COLOR;
    init_list(&winctl_manager.winctl_list_head);

    winctl_last = NULL;
    winctl_paint(NULL);

    return 0;    
}