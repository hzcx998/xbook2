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

/* 过滤窗口消息，成功返回0，失败返回-1 */
int xtk_window_filter_msg(xtk_spirit_t *spirit, uview_msg_t *msg)
{
    switch (uview_msg_get_type(msg)) {
    case UVIEW_MSG_MOUSE_MOTION:
        {
            int x = uview_msg_get_mouse_x(msg);
            int y = uview_msg_get_mouse_y(msg);
            if (!xtk_mouse_motion(spirit, x, y))
                return 0;
        }
        break;
    case UVIEW_MSG_MOUSE_LBTN_DOWN:
        {
            int x = uview_msg_get_mouse_x(msg);
            int y = uview_msg_get_mouse_y(msg);
            if (!xtk_mouse_lbtn_down(spirit, x, y))
                return 0;
        }
        break;
    case UVIEW_MSG_MOUSE_LBTN_UP:
        {
            int x = uview_msg_get_mouse_x(msg);
            int y = uview_msg_get_mouse_y(msg);
            if (!xtk_mouse_lbtn_up(spirit, x, y))
                return 0;
        }
        break;
    default:
        break;
    }
    return -1;
}

int xtk_window_user_msg(xtk_spirit_t *spirit, uview_msg_t *msg)
{

    return 0;
}

int xtk_window_main()
{
    xtk_spirit_t *spirit;
    uview_msg_t msg;

    xtk_view_t *pview;
    xtk_view_for_each (pview) {
        uview_set_wait(pview->view, 1);
        if (uview_get_msg(pview->view, &msg) < 0) {
            continue;
        }
        // 遍历每一个视图来获取上面的精灵
        list_for_each_owner (spirit, &pview->spirit_list_head, list) {
            // 处理内置消息
            if (!xtk_window_filter_msg(spirit, &msg))
                continue;
            // 处理用户消息
            xtk_window_user_msg(spirit, &msg);
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
    xtk_spirit_t *spirit = &window->window_spirit;
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
    #if 0
    xtk_window_navigation_t *navigation = &window->navigation;
    xtk_spirit_set_pos(navigation->title, spirit->width / 2 - (xtk_label_length(navigation->title) * 
        8) / 2, (navigation_bottom - 16) / 2);
    xtk_text_to_bitmap(navigation->title->text, text_c, DOTF_STANDARD_NAME, spirit->bitmap,
        navigation->title->x, navigation->title->y);
    #endif

    uview_bitblt(spirit->view, 0, 0, spirit->bitmap);
    if (redraw_bg) {
        uview_update(spirit->view, 0, 0, spirit->width, spirit->height);
    }

    // TODO: 刷新所有精灵
    xtk_spirit_show_all(&window->window_spirit);

    return 0;
}

static int xtk_window_create_navigation(xtk_window_t *window, char *title)
{
    xtk_window_navigation_t *navigation = &window->navigation;
    //navigation->title = xtk_label_create(title);
    // ...
    xtk_spirit_t *window_spirit = &window->window_spirit;
    
    xtk_spirit_t *_title = xtk_label_create(title);
    assert(_title);
    navigation->title = _title;
    
    xtk_spirit_t *btn0 = xtk_button_create_with_label("-");
    assert(btn0);
    xtk_spirit_t *btn1 = xtk_button_create_with_label("O");
    assert(btn1);
    xtk_spirit_t *btn2 = xtk_button_create_with_label("X");
    assert(btn2);

    xtk_spirit_set_pos(_title, 4, 4);
    xtk_spirit_set_pos(btn2, window_spirit->width - btn2->width - 4, 4);
    xtk_spirit_set_pos(btn0, window_spirit->width - (btn2->width + btn1->width) - 4, 4);
    xtk_spirit_set_pos(btn1, window_spirit->width - (btn2->width + btn1->width + btn0->width) - 4, 4);
    
    xtk_container_add(XTK_CONTAINER(window_spirit), _title);
    xtk_container_add(XTK_CONTAINER(window_spirit), btn0);
    xtk_container_add(XTK_CONTAINER(window_spirit), btn1);
    xtk_container_add(XTK_CONTAINER(window_spirit), btn2);
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
    uview_show(window->window_spirit.view);
    return 0;
}

xtk_spirit_t *xtk_window_create2(char *title, int x, int y, int width, int height, uint32_t flags)
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
    
    xtk_spirit_t *spirit = &window->window_spirit;
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
    
    xtk_spirit_t *window_spirit = &window->window_spirit;
    xtk_spirit_init(window_spirit, 0, 0, new_width, new_height);
    xtk_spirit_set_type(window_spirit, XTK_SPIRIT_TYPE_WINDOW);
    window_spirit->style.align = XTK_ALIGN_CENTER;

    xtk_spirit_t *spirit = &window->spirit;
    xtk_spirit_init(spirit, 0, 0, width, height);
    xtk_spirit_set_type(spirit, XTK_SPIRIT_TYPE_WINDOW);
    spirit->style.align = XTK_ALIGN_CENTER;

    // 创建窗口容器，只能容纳一个容器
    window_spirit->container = xtk_container_create(XTK_CONTAINER_SINGAL, window_spirit);
    if (!window_spirit->container) {
        xtk_spirit_cleanup(window_spirit);
        free(window);
        return NULL;
    }

    spirit->container = xtk_container_create(XTK_CONTAINER_SINGAL, spirit);
    if (!spirit->container) {
        xtk_spirit_cleanup(window_spirit);
        free(window);
        return NULL;
    }

    uview_bitmap_t *bmp = uview_bitmap_create(new_width, new_height);
    if (!bmp) {
        xtk_container_destroy(window_spirit->container);
        window_spirit->container = NULL;
        xtk_spirit_cleanup(window_spirit);
        free(window);
        return NULL;
    }
    xtk_spirit_set_bitmap(window_spirit, bmp);

    bmp = uview_bitmap_create(width, height);
    if (!bmp) {
        xtk_container_destroy(window_spirit->container);
        window_spirit->container = NULL;
        xtk_spirit_cleanup(window_spirit);
        free(window);
        return NULL;
    }
    xtk_spirit_set_bitmap(spirit, bmp);

    // 创建导航栏
    if (xtk_window_create_navigation(window, title) < 0) {
        xtk_container_destroy(window_spirit->container);
        window_spirit->container = NULL;
        xtk_spirit_cleanup(window_spirit);
        free(window);
        return NULL;
    }

    // 创建视图
    int view = uview_open(new_width, new_height);
    if (view < 0) {
        xtk_container_destroy(window_spirit->container);
        window_spirit->container = NULL;
        xtk_window_destroy_navigation(window);
        xtk_spirit_cleanup(window_spirit);
        free(window);
        return NULL;
    }
    uview_set_type(view, UVIEW_TYPE_WINDOW);
    uview_set_pos(view, x, y);
    // 绑定视图
    xtk_spirit_set_view(window_spirit, view);
    xtk_spirit_set_view(spirit, view);

    xtk_window_draw_border(window, 1, 1);
    
    xtk_view_t *pview = xtk_view_create();
    assert(pview);
    pview->view = view;
    list_add(&window_spirit->list, &pview->spirit_list_head);
    list_add(&spirit->list, &pview->spirit_list_head);    
    xtk_view_add(pview);
    
    if (flags & XTK_WINDOW_SHOW)
        uview_show(view);

    return spirit;
}