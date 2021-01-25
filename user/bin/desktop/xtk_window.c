#include "xtk.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static xtk_window_style_t __xtk_window_style_defult = {
    1, 
    24,
    XTK_ARGB(225, 245, 245, 245), // background
    XTK_ARGB(225, 225, 225, 225),
    XTK_RGB(230, 230, 230), // front
    XTK_RGB(200, 200, 200),
    XTK_RGB(200, 200, 200), // border
    XTK_RGB(180, 180, 180),
    XTK_RGB(25, 25, 25),      // text
    XTK_RGB(118, 118, 118),
};

static int xtk_window_change_size(xtk_window_t *window, int width, int height)
{
    if (!window)
        return -1;
    xtk_spirit_t *spirit = &window->spirit;
    printf("xtk change size: from (%d, %d) to (%d, %d)", spirit->width, spirit->height, width, height);
    return 0;
}

/**
 * 
 * 
 */
void xtk_window_filter_msg(xtk_window_t *window, uview_msg_t *msg)
{
    xtk_spirit_t *spirit = &window->spirit;
    // 转换鼠标坐标位置为窗口内容的坐标
    switch (uview_msg_get_type(msg)) {
    case UVIEW_MSG_MOUSE_MOTION:
    case UVIEW_MSG_MOUSE_LBTN_DBLCLK:
    case UVIEW_MSG_MOUSE_LBTN_DOWN:
    case UVIEW_MSG_MOUSE_LBTN_UP:
    case UVIEW_MSG_MOUSE_MBTN_DBLCLK:
    case UVIEW_MSG_MOUSE_MBTN_DOWN:
    case UVIEW_MSG_MOUSE_MBTN_UP:
    case UVIEW_MSG_MOUSE_RBTN_DBLCLK:
    case UVIEW_MSG_MOUSE_RBTN_DOWN:
    case UVIEW_MSG_MOUSE_RBTN_UP:
    case UVIEW_MSG_MOUSE_WHEEL_UP:
    case UVIEW_MSG_MOUSE_WHEEL_DOWN:
    case UVIEW_MSG_MOUSE_WHEEL_LEFT:
    case UVIEW_MSG_MOUSE_WHEEL_RIGHT:
        {
            uview_msg_get_mouse_x(msg) = uview_msg_get_mouse_x(msg) - spirit->x;
            uview_msg_get_mouse_y(msg) = uview_msg_get_mouse_y(msg) - spirit->y;

            int x = uview_msg_get_mouse_x(msg);
            int y = uview_msg_get_mouse_y(msg);
            // 超出返回，就不传递过去
            if (x < 0 || y < 0 || x >= spirit->width || y >= spirit->height) {
                return;
            }
        }
        break;
    case UVIEW_MSG_HIDE:
        xtk_mouse_motion(spirit, -1, -1);
        xtk_spirit_hide(spirit);
        return;
    case UVIEW_MSG_SHOW:
        xtk_mouse_motion(spirit, -1, -1);
        xtk_spirit_show(spirit);
        return;
    case UVIEW_MSG_ACTIVATE:
        xtk_window_set_active(window, true);
        return;    
    case UVIEW_MSG_INACTIVATE:
        xtk_window_set_active(window, false);
        return;    
    case UVIEW_MSG_RESIZE:
        /* 响应大小调整 */
        xtk_window_change_size(XTK_WINDOW(spirit), uview_msg_get_resize_width(msg),
            uview_msg_get_resize_height(msg));
        /* 调整窗口后，鼠标位置发生了改变，需要做一次位置检测 */
        xtk_mouse_motion(spirit, -1, -1);
        return;
    default:
        break;
    }
    // 调用用户处理函数
    if (window->routine)
        window->routine(window, msg);
}

/**
 * 窗口逻辑
 * 返回0表示消息被截断，返回-1表示消息还可以继续传递
 */
int xtk_window_main(xtk_spirit_t *spirit, uview_msg_t *msg)
{
    switch (uview_msg_get_type(msg)) {
    case UVIEW_MSG_LEAVE:
    case UVIEW_MSG_ENTER:
    case UVIEW_MSG_MOUSE_MOTION:
        {
            int x = uview_msg_get_mouse_x(msg) - spirit->x;
            int y = uview_msg_get_mouse_y(msg) - spirit->y;
            if (!xtk_mouse_motion(spirit, x, y))
                return 0;
        }
        break;
    
    case UVIEW_MSG_MOUSE_LBTN_DOWN:
        {
            int x = uview_msg_get_mouse_x(msg) - spirit->x;
            int y = uview_msg_get_mouse_y(msg) - spirit->y;
            if (!xtk_mouse_lbtn_down(spirit, x, y))
                return 0;
        }
        break;
    case UVIEW_MSG_MOUSE_LBTN_UP:
        {
            int x = uview_msg_get_mouse_x(msg) - spirit->x;
            int y = uview_msg_get_mouse_y(msg) - spirit->y;
            if (!xtk_mouse_lbtn_up(spirit, x, y))
                return 0;
        }
        break;
    default:
        break;
    }
    return -1;  // 需要进一步处理
}

int xtk_window_quit(xtk_spirit_t *spirit)
{
    return xtk_spirit_destroy_all(spirit);
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
    assert(spirit->surface);
    int border_thick = window->style->border_thick;
    /* 需要清空位图 */
    xtk_surface_clear(spirit->surface);
    if (redraw_bg)
        xtk_surface_rectfill(spirit->surface, border_thick, border_thick, 
            spirit->width - border_thick * 2, spirit->height - border_thick * 2, back);
    
    int navigation_bottom = border_thick + window->style->navigation_height;

    // 绘制导航栏
    xtk_surface_rectfill(spirit->surface, border_thick, border_thick, 
        spirit->width - border_thick * 2, window->style->navigation_height, back);
    xtk_surface_rectfill(spirit->surface, border_thick, 
        navigation_bottom - border_thick,
        spirit->width - border_thick * 2, border_thick, border);
    
    /* 基础边框 */
    xtk_surface_rectfill(spirit->surface, 0, 0, spirit->width, border_thick, border);
    xtk_surface_rectfill(spirit->surface, 0, spirit->height - border_thick, spirit->width,
        border_thick, border);
    xtk_surface_rectfill(spirit->surface, 0, 0, border_thick, spirit->height, border);
    xtk_surface_rectfill(spirit->surface, spirit->width - border_thick, 0, 
        border_thick, spirit->height, border);

    /* TODO: 修改按钮，标题，图标颜色 */
    xtk_window_navigation_t *navigation = &window->navigation;
    // 有才设置颜色
    if (navigation->title)
        navigation->title->style.color = text_c;
    
    // 刷新到屏幕上
    uview_bitmap_t bmp;
    uview_bitmap_init(&bmp, spirit->surface->w, spirit->surface->h, (uview_color_t *) spirit->surface->pixels);
    uview_bitblt(spirit->view, 0, 0, &bmp);
    uview_update(spirit->view, 0, 0, spirit->width, spirit->height);
    
    // TODO: 刷新所有精灵
    xtk_spirit_show_children(&window->window_spirit);
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
    uview_show(window->spirit.view);
    return 0;
}

int xtk_window_spirit_setup(xtk_window_t *window, xtk_spirit_t *spirit, int x, int y, int width, int height)
{
    xtk_spirit_init(spirit, x, y, width, height);
    xtk_spirit_set_type(spirit, XTK_SPIRIT_TYPE_WINDOW);
    spirit->style.align = XTK_ALIGN_CENTER;
    spirit->style.background_color = window->style->background_color_inactive;
    
    xtk_container_t *container = xtk_container_create(spirit);
    if (!container) {
        return -1;
    }
    xtk_spirit_set_container(spirit, container);
    
    xtk_surface_t *bmp = xtk_surface_create(width, height);
    if (!bmp) {
        xtk_spirit_set_container(spirit, NULL);
        return -1;
    }
    xtk_spirit_set_surface(spirit, bmp);
    return 0;
}

int xtk_window_spirit_setdown(xtk_spirit_t *spirit)
{
    if (!spirit)
        return -1;
    xtk_spirit_set_surface(spirit, NULL);
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
    pview->spirit = &window->spirit;
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
    pview->spirit = NULL;
    xtk_view_remove(pview);
    uview_close(pview->view);
    xtk_view_destroy(pview);
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
    xtk_window_spirit_setup(window, window_spirit, 0, 0, new_width, new_height);
    
    xtk_spirit_t *spirit = &window->spirit;
    if (xtk_window_spirit_setup(window, spirit, window->style->border_thick, 
        window->style->border_thick + window->style->navigation_height,
        width, height) < 0)
    {
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
    if (xtk_window_spirit_setup(window, spirit, 0, 0, width, height) < 0) {
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
    window->routine = NULL;
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

int xtk_window_destroy(xtk_window_t *window)
{
    if (!window)
        return -1;

    if (xtk_window_view_setdown(window) < 0)
        return -1;
    if (window->type == XTK_WINDOW_TOPLEVEL) {
        xtk_window_destroy_navigation(window);
        xtk_window_spirit_setdown(&window->window_spirit);
    }
    xtk_window_spirit_setdown(&window->spirit);
    free(window);
    return 0;
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
        navigation->title->style.background_color = UVIEW_NONE_COLOR;
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

xtk_surface_t *xtk_window_get_surface(xtk_window_t *window)
{
    if (!window)
        return -1;
    return window->spirit.surface;
}

int xtk_window_flip(xtk_window_t *window)
{
    if (!window)
        return -1;
    xtk_spirit_t *spirit = &window->spirit;
    uview_bitmap_t bmp;
    uview_bitmap_init(&bmp, spirit->surface->w, spirit->surface->h,
        spirit->surface->pixels);
    return uview_bitblt_update(spirit->view, spirit->x, spirit->y,
            &bmp);
}

int xtk_window_update(xtk_window_t *window, int x, int y, int w, int h)
{
    if (!window)
        return -1;
    xtk_spirit_t *spirit = &window->spirit;
    
    if (x >= spirit->width)
        return 0;
    if (y >= spirit->height)
        return 0;
    
    int ex = x + w, ey = y + h;
    if (ex > spirit->width)
        ex = spirit->width;
    if (ey > spirit->height)
        ey = spirit->height;
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    uview_bitmap_t bmp;
    uview_bitmap_init(&bmp, spirit->surface->w, spirit->surface->h,
        spirit->surface->pixels);
    return uview_bitblt_update_ex(spirit->view, spirit->x + x, spirit->y + y,
            &bmp, x, y, ex - x, ey - y);
}

int xtk_window_set_position(xtk_window_t *window, xtk_window_position_t pos)
{
    if (!window)
        return -1;
    xtk_spirit_t *spirit = &window->window_spirit;
    if (spirit->view < 0)
        return -1;
    switch (pos) {
    case XTK_WIN_POS_NONE:
        {
            int vx = 0, vy = 0;
            uview_get_lastpos(spirit->view, &vx, &vy);
            uview_set_pos(spirit->view, vx, vy);
        }
        break;
    case XTK_WIN_POS_CENTER_ALWAYS:
        uview_set_unmoveable(spirit->view);
    case XTK_WIN_POS_CENTER:
        {
            int w = 0, h = 0;
            uview_get_screensize(spirit->view, &w, &h);
            uview_set_pos(spirit->view, w / 2 - spirit->width / 2,
                h / 2 - spirit->height / 2);
        }
        break;    
    case XTK_WIN_POS_MOUSE:
        {
            int mx = 0, my = 0;
            uview_get_mousepos(spirit->view, &mx, &my);
            uview_set_pos(spirit->view, mx, my);
        }
        break;  
    default:
        break;
    }
    return 0;
}

int xtk_window_set_default_size(xtk_window_t *window, int width, int height)
{
    if (!window)
        return -1;
    window->content_width = width;
    window->content_height = height;
    // 调整精灵的大小

    return 0;
}

int xtk_window_set_routine(xtk_window_t *window, xtk_window_routine_t routine)
{
    if (!window)
        return -1;
    window->routine = routine;
    return 0;
}

int xtk_window_set_active(xtk_window_t *window, bool is_active)
{
    if (!window)
        return -1;
    xtk_window_draw_border(window, is_active, 0);
    return 0;
}
