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

    uview_bitblt(window->view, 0, 0, spirit->bitmap);
    if (redraw_bg) {
        uview_update(window->view, 0, 0, spirit->width, spirit->height);
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
    xtk_spirit_init(&window->spirit, x, y, new_width, new_height);

    uview_bitmap_t *bmp = uview_bitmap_create(new_width, new_height);
    if (!bmp) {
        xtk_spirit_cleanup(&window->spirit);
        free(window);
        return NULL;
    }
    xtk_spirit_set_bitmap(&window->spirit, bmp);

    if (xtk_window_create_navigation(window, title) < 0) {
        xtk_spirit_cleanup(&window->spirit);
        free(window);
        return NULL;
    }

    // 创建视图
    window->view = uview_open(new_width, new_height);
    if (window->view < 0) {
        xtk_window_destroy_navigation(window);
        xtk_spirit_cleanup(&window->spirit);
        free(window);
        return NULL;
    }
    uview_set_type(window->view, UVIEW_TYPE_WINDOW);
    uview_set_pos(window->view, x, y);
    
    xtk_window_draw_border(window, 1, 1);
    
    if (flags & XTK_WINDOW_SHOW)
        uview_show(window->view);

    return &window->spirit;
}
