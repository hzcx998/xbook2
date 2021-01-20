#include "xtk.h"
#include <stdlib.h>
#include <assert.h>

LIST_HEAD(xtk_window_list_head);

static xtk_window_style_t __xtk_window_style_defult = {
    4, 
    24,
    UVIEW_ARGB(225, 245, 245, 245), // background
    UVIEW_ARGB(225, 225, 225, 225),
    UVIEW_RGB(230, 230, 230), // front
    UVIEW_RGB(200, 200, 200),
    UVIEW_RGB(200, 200, 200), // border
    UVIEW_RGB(180, 180, 180),
    UVIEW_RGB(25, 25, 25),      // text
    UVIEW_RGB(118, 118, 118),
};

void xtk_mouse_motion(xtk_spirit_t *spirit, int x, int y)
{
    xtk_container_t *container = spirit->container;
    if (!container)
        return;
    xtk_spirit_t *tmp;
    list_for_each_owner (tmp, &container->children_list, list) {
        switch (tmp->type)
        {
        case XTK_SPIRIT_TYPE_LABEL:
            {

            }
            break;
        case XTK_SPIRIT_TYPE_BUTTON:
            {
                xtk_button_t *btn = XTK_BUTTON(tmp);
                if (XTK_IN_SPIRIT(tmp, x, y)) {
                    if (btn->state == XTK_BUTTON_IDLE) {
                        xtk_button_change_state(btn, XTK_BUTTON_TOUCH);
                    }
                } else {
                    xtk_button_change_state(btn, XTK_BUTTON_IDLE);
                }
                xtk_spirit_show(tmp);
            }
            break;
        default:
            break;
        }
    }
}


void xtk_mouse_lbtn_down(xtk_spirit_t *spirit, int x, int y)
{
    xtk_container_t *container = spirit->container;
    if (!container)
        return;
    xtk_spirit_t *tmp;
    list_for_each_owner (tmp, &container->children_list, list) {
        switch (tmp->type)
        {
        case XTK_SPIRIT_TYPE_LABEL:
            {

            }
            break;
        case XTK_SPIRIT_TYPE_BUTTON:
            {
                xtk_button_t *btn = XTK_BUTTON(tmp);
                if (XTK_IN_SPIRIT(tmp, x, y)) {
                    if (btn->state == XTK_BUTTON_TOUCH) {
                        xtk_button_change_state(btn, XTK_BUTTON_CLICK);
                        xtk_spirit_show(tmp);   
                    }
                }
            }
            break;
        default:
            break;
        }
    }
}

void xtk_mouse_lbtn_up(xtk_spirit_t *spirit, int x, int y)
{
    xtk_container_t *container = spirit->container;
    if (!container)
        return;
    xtk_spirit_t *tmp;
    list_for_each_owner (tmp, &container->children_list, list) {
        switch (tmp->type)
        {
        case XTK_SPIRIT_TYPE_LABEL:
            {

            }
            break;
        case XTK_SPIRIT_TYPE_BUTTON:
            {
                xtk_button_t *btn = XTK_BUTTON(tmp);
                if (XTK_IN_SPIRIT(tmp, x, y)) {
                    if (btn->state == XTK_BUTTON_CLICK) {
                        printf("mouse call signal: %d, %d\n", x, y);
                        xtk_button_change_state(btn, XTK_BUTTON_TOUCH);
                        xtk_spirit_show(tmp);
                    }
                }
            }
            break;
        default:
            break;
        }
    }
}

/* 过滤窗口消息，成功返回0，失败返回-1 */
int xtk_window_filter_msg(xtk_spirit_t *spirit, uview_msg_t *msg)
{
    switch (uview_msg_get_type(msg)) {
    case UVIEW_MSG_MOUSE_MOTION:
        {
            int x = uview_msg_get_mouse_x(msg);
            int y = uview_msg_get_mouse_y(msg);
            xtk_mouse_motion(spirit, x, y);
        }
        break;
    case UVIEW_MSG_MOUSE_LBTN_DOWN:
        {
            int x = uview_msg_get_mouse_x(msg);
            int y = uview_msg_get_mouse_y(msg);
            xtk_mouse_lbtn_down(spirit, x, y);
        }
        break;
    case UVIEW_MSG_MOUSE_LBTN_UP:
        {
            int x = uview_msg_get_mouse_x(msg);
            int y = uview_msg_get_mouse_y(msg);
            xtk_mouse_lbtn_up(spirit, x, y);
        }
        break;
    default:
        break;
    }
}

int xtk_window_main()
{
    xtk_spirit_t *tmp;
    uview_msg_t msg;
    while (1) {
        list_for_each_owner (tmp, &xtk_window_list_head, list) {
            uview_set_wait(tmp->view, 1);
            if (uview_get_msg(tmp->view, &msg) < 0) {
                continue;
            }
            // 处理内置消息
            xtk_window_filter_msg(tmp, &msg);
            // 处理用户消息
            // xtk_window_user_msg(&msg);
        }
    }
}

/**
 * 绘制窗口边框
 * @is_active: 是否为激活状态，1为激活，0为不激活
 * @redraw_bg: 是否重绘窗体背景：1为重绘，0为不重绘
 */
int xtk_window_draw_border(xtk_window_t *window, 
        int is_active, int redraw_bg)
{
    if (!window)
        return -1;
    uview_color_t back, border, text_c;
    if (is_active) {
        back = window->style->background_color_active;
        border = window->style->border_color_active;
        text_c = window->style->text_color_active;
        window->winflgs |= XTK_WINDOW_ACTIVE;
    } else {
        back = window->style->background_color_inactive;
        border = window->style->border_color_inactive;
        text_c = window->style->text_color_inactive;
        window->winflgs |= XTK_WINDOW_ACTIVE;
    }
    xtk_spirit_t *spirit = &window->spirit;
    assert(spirit->bitmap);

    /* 需要清空位图 */
    uview_bitmap_clear(spirit->bitmap);
    if (redraw_bg)
        uview_bitmap_rectfill(spirit->bitmap, 1, 1, spirit->width - 2, spirit->height - 2, back);
    
    int navigation_bottom = window->style->border_thick + window->style->navigation_height;

    // 绘制导航栏
    uview_bitmap_rectfill(spirit->bitmap, 1, 
        navigation_bottom - 1,
        spirit->width - 2, 1, border);
    
    /* 基础边框 */
    uview_bitmap_rectfill(spirit->bitmap, 0, 0, spirit->width, 1, border);
    uview_bitmap_rectfill(spirit->bitmap, 0, spirit->height - 1, spirit->width, 1, border);
    uview_bitmap_rectfill(spirit->bitmap, 0, 0, 1, spirit->height, border);
    uview_bitmap_rectfill(spirit->bitmap, spirit->width - 1, 0, 1, spirit->height, border);

    /* TODO: 绘制按钮，标题，图标等 */
    xtk_window_navigation_t *navigation = &window->navigation;
    xtk_spirit_set_pos(navigation->title, spirit->width / 2 - (xtk_label_length(navigation->title) * 
        8) / 2, (navigation_bottom - 16) / 2);
    xtk_text_to_bitmap(navigation->title->text, text_c, DOTF_STANDARD_NAME, spirit->bitmap,
        navigation->title->x, navigation->title->y);

    uview_bitblt(spirit->view, 0, 0, spirit->bitmap);
    if (redraw_bg) {
        uview_update(spirit->view, 0, 0, spirit->width, spirit->height);
    }
    return 0;
}

static int xtk_window_create_navigation(xtk_window_t *window, char *title)
{
    xtk_window_navigation_t *navigation = &window->navigation;
    navigation->title = xtk_label_create(title);
    if (!navigation->title)
        return -1;
    // ...
    return 0;
} 


static int xtk_window_destroy_navigation(xtk_window_t *window)
{
    xtk_window_navigation_t *navigation = &window->navigation;
    if (navigation->title) {
        if (xtk_spirit_destroy(navigation->title) < 0)
            return -1;
        navigation->title = NULL;
    }
    return 0;
} 

int xtk_window_show(xtk_window_t *window)
{
    if (!window)
        return -1;
    uview_show(window->spirit.view);
    return 0;
}

xtk_spirit_t *xtk_window_create(char *title, int x, int y, int width, int height, uint32_t flags)
{
    if (!title || width <= 0 || height <= 0)
        return NULL;
    if (strlen(title) <= 0)
        return NULL;
    xtk_window_t *window = malloc(sizeof(xtk_window_t));
    if (!window)
        return NULL;

    window->content_width = width;
    window->content_height = height;

    window->style = &__xtk_window_style_defult;

    // 初始化精灵
    int new_width = window->style->border_thick * 2 + width;
    int new_height = window->style->border_thick * 2 + height + window->style->navigation_height;
    
    xtk_spirit_t *spirit = &window->spirit;
    xtk_spirit_init(spirit, x, y, new_width, new_height);
    xtk_spirit_set_type(spirit, XTK_SPIRIT_TYPE_WINDOW);
    spirit->style.align = XTK_ALIGN_CENTER;

    // 创建窗口容器，只能容纳一个容器
    spirit->container = xtk_container_create(XTK_CONTAINER_SINGAL, spirit);
    if (!spirit->container) {
        xtk_spirit_cleanup(spirit);
        free(window);
        return NULL;
    }

    uview_bitmap_t *bmp = uview_bitmap_create(new_width, new_height);
    if (!bmp) {
        xtk_container_destroy(spirit->container);
        spirit->container = NULL;
        xtk_spirit_cleanup(spirit);
        free(window);
        return NULL;
    }
    xtk_spirit_set_bitmap(spirit, bmp);

    if (xtk_window_create_navigation(window, title) < 0) {
        xtk_container_destroy(spirit->container);
        spirit->container = NULL;
        xtk_spirit_cleanup(spirit);
        free(window);
        return NULL;
    }

    // 创建视图
    int view = uview_open(new_width, new_height);
    if (view < 0) {
        xtk_container_destroy(spirit->container);
        spirit->container = NULL;
        xtk_window_destroy_navigation(window);
        xtk_spirit_cleanup(spirit);
        free(window);
        return NULL;
    }
    uview_set_type(view, UVIEW_TYPE_WINDOW);
    uview_set_pos(view, x, y);
    xtk_spirit_set_view(spirit, view);

    xtk_window_draw_border(window, 1, 1);
    
    // 添加到窗口链表
    list_add(&spirit->list, &xtk_window_list_head);

    if (flags & XTK_WINDOW_SHOW)
        uview_show(view);

    return spirit;
}
