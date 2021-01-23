#include "xtk.h"
#include <stdlib.h>
#include <assert.h>

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

int xtk_window_main(xtk_spirit_t *spirit, uview_msg_t *msg)
{
    // 处理内置消息
    if (!xtk_window_filter_msg(spirit, msg))
        return 0;
    // 处理用户消息
    xtk_window_user_msg(spirit, msg);
    return -1;
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

static int xtk_window_create_navigation(xtk_window_t *window)
{
    xtk_window_navigation_t *navigation = &window->navigation;
    navigation->title = NULL;
    xtk_spirit_t *window_spirit = &window->window_spirit;

    xtk_spirit_t *btn_minim = xtk_button_create_with_label("-");
    assert(btn_minim);
    xtk_spirit_t *btn_maxim = xtk_button_create_with_label("O");
    assert(btn_maxim);
    xtk_spirit_t *btn_close = xtk_button_create_with_label("X");
    assert(btn_close);

    int x = window->window_spirit.width - window->style->border_thick - btn_close->width;
    int y = window->style->border_thick + window->style->navigation_height / 2;
    xtk_spirit_set_pos(btn_close, x, y - btn_close->height / 2);
    x -= btn_maxim->width;
    xtk_spirit_set_pos(btn_maxim, x, y - btn_maxim->height / 2);
    x -= btn_close->width;
    xtk_spirit_set_pos(btn_minim, x, y - btn_minim->height / 2);
    
    xtk_container_add(XTK_CONTAINER(window_spirit), btn_minim);
    xtk_container_add(XTK_CONTAINER(window_spirit), btn_maxim);
    xtk_container_add(XTK_CONTAINER(window_spirit), btn_close);
    return 0;
}

static int xtk_window_destroy_navigation(xtk_window_t *window)
{
    return xtk_container_remove_and_destroy_all(window->window_spirit.container);    
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
    
    xtk_spirit_t *window_spirit = &window->window_spirit;
    xtk_spirit_init(window_spirit, 0, 0, new_width, new_height);
    xtk_spirit_set_type(window_spirit, XTK_SPIRIT_TYPE_WINDOW);
    window_spirit->style.align = XTK_ALIGN_CENTER;

    xtk_spirit_t *spirit = &window->spirit;
    xtk_spirit_init(spirit, 0, 0, width, height);
    xtk_spirit_set_type(spirit, XTK_SPIRIT_TYPE_WINDOW);
    spirit->style.align = XTK_ALIGN_CENTER;

    // 创建窗口容器，只能容纳一个容器
    window_spirit->container = xtk_container_create(window_spirit);
    if (!window_spirit->container) {
        xtk_spirit_cleanup(window_spirit);
        free(window);
        return NULL;
    }

    spirit->container = xtk_container_create(spirit);
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
    if (xtk_window_create_navigation(window) < 0) {
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

int xtk_window_spirit_setup(xtk_spirit_t *spirit, int x, int y, int width, int height)
{
    xtk_spirit_init(spirit, 0, 0, width, height);
    xtk_spirit_set_type(spirit, XTK_SPIRIT_TYPE_WINDOW);
    spirit->style.align = XTK_ALIGN_CENTER;

    xtk_container_t *container = xtk_container_create(spirit);
    if (!container) {
        return -1;
    }
    xtk_spirit_set_container(spirit, container);
    
    uview_bitmap_t *bmp = uview_bitmap_create(width, height);
    if (!bmp) {
        xtk_spirit_set_container(spirit, NULL);
        return -1;
    }
    xtk_spirit_set_bitmap(spirit, bmp);
    return 0;
}

int xtk_window_spirit_setdown(xtk_spirit_t *spirit)
{
    if (!spirit)
        return -1;
    xtk_spirit_set_bitmap(spirit, NULL);
    xtk_spirit_set_container(spirit, NULL);
    xtk_spirit_cleanup(spirit);
    return 0;
}

int xtk_window_view_setup(xtk_window_t *window, int x, int y, int width, int height)
{
    // 创建视图
    int view = uview_open(width, height);
    if (view < 0) {
        return -1;
    }
    uview_set_type(view, UVIEW_TYPE_WINDOW);
    uview_set_pos(view, x, y);
    // 绑定视图
    xtk_spirit_set_view(&window->spirit, view);
    if (window->type == XTK_WINDOW_TOPLEVEL) {
        xtk_spirit_set_view(&window->window_spirit, view);
    }
    
    xtk_view_t *pview = xtk_view_create();
    assert(pview);
    pview->view = view;
    list_add(&window->spirit.list, &pview->spirit_list_head);    
    if (window->type == XTK_WINDOW_TOPLEVEL) {
        list_add(&window->window_spirit.list, &pview->spirit_list_head);
    }
    xtk_view_add(pview);
    return 0;
}

int xtk_window_view_setdown(xtk_window_t *window)
{
    if (!window)
        return -1;
    xtk_view_t *pview = xtk_view_find(window->spirit.view);
    if (!pview)
        return -1;
    list_del(&window->spirit.list);
    if (window->type == XTK_WINDOW_TOPLEVEL) {
        list_del(&window->window_spirit.list);
    }
    xtk_view_remove(pview);
    uview_close(pview->view);
    return 0;
}

static xtk_spirit_t *xtk_window_create_toplevel(xtk_window_t *window)
{
    int width = XTK_WINDOW_WIDTH_DEFAULT;
    int height = XTK_WINDOW_HEIGHT_DEFAULT;
    
    window->content_width = width;
    window->content_height = height;

    window->style = &__xtk_window_style_defult;

    // 初始化精灵
    int new_width = window->style->border_thick * 2 + width;
    int new_height = window->style->border_thick * 2 + height + window->style->navigation_height;
    
    xtk_spirit_t *window_spirit = &window->window_spirit;
    xtk_window_spirit_setup(window_spirit, 0, 0, new_width, new_height);
    
    xtk_spirit_t *spirit = &window->spirit;
    if (xtk_window_spirit_setup(spirit, 0, 0, width, height) < 0) {
        xtk_window_spirit_setdown(window_spirit);
        return NULL;
    }

    // 创建导航栏
    if (xtk_window_create_navigation(window) < 0) {
        xtk_window_spirit_setdown(window_spirit);
        xtk_window_spirit_setdown(spirit);
        return NULL;
    }

    if (xtk_window_view_setup(window, 0, 0, new_width, new_height) < 0) {
        xtk_window_destroy_navigation(window);
        xtk_window_spirit_setdown(window_spirit);
        xtk_window_spirit_setdown(spirit);
        return NULL;
    }
    
    // 窗口需要绘制
    xtk_window_draw_border(window, 1, 1);
    return spirit;
}

static xtk_spirit_t *xtk_window_create_popup(xtk_window_t *window)
{
    int width = XTK_WINDOW_WIDTH_DEFAULT;
    int height = XTK_WINDOW_HEIGHT_DEFAULT;
    window->content_width = width;
    window->content_height = height;
    window->style = NULL;
    xtk_spirit_t *spirit = &window->spirit;
    if (xtk_window_spirit_setup(spirit, 0, 0, width, height) < 0) {
        return NULL;
    }
        
    if (xtk_window_view_setup(window, 0, 0, width, height) < 0) {
        xtk_window_view_setdown(window);
        return NULL;
    }
    return spirit;
}

xtk_spirit_t *xtk_window_create(xtk_window_type_t type)
{
    xtk_window_t *window = malloc(sizeof(xtk_window_t));
    if (!window)
        return NULL;
    memset(window, 0, sizeof(xtk_window_t));
    window->type = type;
    xtk_spirit_t *spirit = NULL;
    if (type == XTK_WINDOW_TOPLEVEL) {
        spirit = xtk_window_create_toplevel(window);
    } else if (type == XTK_WINDOW_POPUP) {
        spirit = xtk_window_create_popup(window);
    }
    if (!spirit)
        free(window);
    return spirit;
}

int xtk_window_set_title(xtk_window_t *window, char *title)
{
    if (!window || !title)
        return -1;
    
    if (window->type != XTK_WINDOW_TOPLEVEL)
        return -1;

    xtk_window_navigation_t *navigation = &window->navigation;
    if (!navigation->title) {
        navigation->title = xtk_label_create(title);
        if (!navigation->title)
            return -1;
        navigation->title->style.background_color = UVIEW_NONE;
        // 第一次创建需要添加到容器中
        xtk_container_add(XTK_CONTAINER(&window->window_spirit), navigation->title);
    } else {
        xtk_label_set_text(navigation->title, title);
    }
    // 调整位置
    int x = window->style->border_thick;
    int y = window->style->border_thick + window->style->navigation_height / 2  - \
        navigation->title->height / 2;
    xtk_spirit_set_pos(navigation->title, x, y);
    // 设置标题后需要重绘边框
    xtk_window_draw_border(window, window->winflgs & XTK_WINDOW_ACTIVE, 1);
    return 0;
}

int xtk_window_set_resizable(xtk_window_t *window, bool resizable)
{
    if (!window)
        return -1;
    xtk_spirit_t *spirit = &window->window_spirit;
    if (resizable)
        uview_set_resizable(spirit->view);
    else
        uview_set_unresizable(spirit->view);
    return 0;
}