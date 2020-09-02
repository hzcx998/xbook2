#include <gtouch.h>
#include <glayer.h>
#include <gbitmap.h>
#include <malloc.h>

g_touch_t *g_new_touch(unsigned int width, unsigned int height)
{
    g_touch_t *tch = (g_touch_t *)malloc(sizeof(g_touch_t));
    if (tch == NULL)
        return NULL;
    tch->rect.width = width;
    tch->rect.height = height;
    tch->rect.x = 0;
    tch->rect.y = 0;
    tch->state = G_TOUCH_IDLE;
    tch->shape = G_TOUCH_SHAPE_FILL;    /* 默认是填充形状 */
    int i;
    for (i = 0; i < 3; i++)
        tch->handler[i] = NULL;
    
    tch->layer = -1; 
    for (i = 0; i < 3; i++)
        tch->hold_on[i] = 0;
    
    tch->color_idle = GC_GRAY;
    tch->color_on = GC_BLUE;
    tch->extension = NULL;
    init_list(&tch->list);
    return tch;
}

int g_del_touch(g_touch_t *tch)
{
    if (!tch)
        return -1;
    list_del(&tch->list);
    free(tch);
    return 0;
}

int g_touch_paint(g_touch_t *tch)
{
    if (!tch)
        return -1;
    g_color_t color;
    if (tch->state == G_TOUCH_IDLE) {
        color = tch->color_idle;
    } else if (tch->state == G_TOUCH_ON) {
        color = tch->color_on;
    }
    #if 0
    g_layer_rect_fill(tch->layer, tch->rect.x, tch->rect.y, 
        tch->rect.width, tch->rect.height, color);
    g_layer_refresh_rect(tch->layer, tch->rect.x, tch->rect.y, 
        tch->rect.width, tch->rect.height);
    #else
    g_bitmap_t *bmp = g_new_bitmap(tch->rect.width, tch->rect.height);
    if (!bmp)
        return -1;
    if (tch->shape == G_TOUCH_SHAPE_FILL)
        g_rectfill(bmp, 0, 0, tch->rect.width, tch->rect.height, color);
    else
        g_rect(bmp, 0, 0, tch->rect.width, tch->rect.height, color);
        
    g_bitmap_sync(bmp, tch->layer, tch->rect.x, tch->rect.y);
    g_del_bitmap(bmp);
    #endif
    return 0;
}

int g_touch_paint_group(list_t *list_head)
{
    if (!list_head)
        return -1;
    g_touch_t *tch;
    list_for_each_owner (tch, list_head, list) {
        if (g_touch_paint(tch) < 0)
            return -1;
    }
    return 0;
}

int g_touch_set_idel_color_group(list_t *list_head, g_color_t color)
{
    if (!list_head)
        return -1;
    g_touch_t *tch;
    list_for_each_owner (tch, list_head, list) {
        tch->color_idle = color;
    }
    return 0;
}

int g_touch_del_group(list_t *list_head)
{
    if (!list_head)
        return -1;
    g_touch_t *tch, *next;
    list_for_each_owner_safe (tch, next, list_head, list) {
        if (g_del_touch(tch) < 0)
            return -1;
    }
    return 0;
}

/**
 * 状态改变检测，返回0表示已经改变，-1表示没有改变
 */
int g_touch_state_check(g_touch_t *tch, g_point_t *po)
{
    if (!tch)
        return -1;
    char old_state = tch->state;
    if (g_rect_in(&tch->rect, po->x, po->y)) {
        tch->state = G_TOUCH_ON;
    } else {
        tch->state = G_TOUCH_IDLE;
        /* 不在区域内，那么弹起准备取消 */
        int i;
        for (i = 0; i < 3; i++)
            tch->hold_on[i] = 0;
    }
    if (tch->state != old_state) {  /* 新旧状态不一样才重绘 */
        g_touch_paint(tch);
        return 0;
    }
    return -1;
}

int g_touch_state_check_group(list_t *list_head, g_point_t *po)
{
    if (!list_head)
        return -1;
    g_touch_t *tch;
    list_for_each_owner (tch, list_head, list) {
        if (!g_touch_state_check(tch, po))
            return 0;
    }
    return -1;
}

/**
 * 点击检测，返回0表示已经点击，-1表示未能点击
 * btn表示按键：
 *      0：鼠标左键按键按下
 *      1：鼠标左键按键弹起
 *      2：鼠标中键按键按下
 *      3：鼠标中键按键弹起
 *      4：鼠标右键按键按下
 *      5：鼠标右键按键弹起
 */
int g_touch_click_check(g_touch_t *tch, g_point_t *po, int btn)
{ 
    if (!tch)
        return -1;
    int retval = -1;
    g_touch_handler_t func;
    if (g_rect_in(&tch->rect, po->x, po->y)) {
        switch (btn)
        {
        case 0:
            tch->hold_on[0] = 1;
            break;
        case 1:
            func = tch->handler[0];
            if (tch->hold_on[0]) { /* 准备弹起就绪 */
                if (func)
                    retval = func(tch); /* 执行操作 */
                
            }
            break;
            
        case 2:
            tch->hold_on[1] = 1;
            
            break;
        case 3:
            func = tch->handler[1];
            if (tch->hold_on[1]) { /* 准备弹起就绪 */
                if (func)
                    retval = func(tch); /* 执行操作 */
                
            }
            break;
        case 4:
            tch->hold_on[2] = 1;
            break;
        case 5:
            func = tch->handler[2];
            if (tch->hold_on[2]) { /* 准备弹起就绪 */
                if (func)
                    retval = func(tch); /* 执行操作 */
                
            }
            break;
        default:
            break;
        }
    } else {
        int i;
        for (i = 0; i < 3; i++)
            tch->hold_on[i] = 0;
    
    }
    return retval;
}

int g_touch_click_check_group(list_t *list_head, g_point_t *po, int btn)
{
    if (!list_head)
        return -1;
    g_touch_t *tch;
    list_for_each_owner (tch, list_head, list) {
        if (!g_touch_click_check(tch, po, btn))
            return 0;
    }
    return -1;
}