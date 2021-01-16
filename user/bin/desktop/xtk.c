#include "xtk.h"
#include <uview.h>
#include <stdlib.h>
#include <assert.h>

#include <dotfont.h>

void xtk_test(int fd, uview_bitmap_t *wbmp)
{
    xtk_image_t *img = xtk_image_load2("/res/cursor.png", 32, 32);
    assert(img);
    
    uview_bitmap_t *bmp = uview_bitmap_create(img->w, img->h);
    assert(bmp);
    
    uview_bitmap_t srcbmp;
    uview_bitmap_init(&srcbmp, img->w, img->h, (uview_color_t *) img->buf);
    
    uview_bitmap_bitblt(bmp, 0, 0, &srcbmp, 0, 0, srcbmp.width, srcbmp.height);
    uview_bitblt_update(fd, 0, 0, bmp);
    
    uview_bitmap_destroy(bmp);
    xtk_image_destroy(img);

    xtk_spirit_t *spirit = xtk_spirit_create(100, 100, 100, 24);
    assert(spirit);
    spirit->style.background_align = XTK_ALIGN_CENTER;
    spirit->style.background_color = UVIEW_BLUE;
    xtk_spirit_set_text(spirit, "abcdef!asdasdasd");
    xtk_spirit_set_background_image(spirit, "/res/cursor.png");
    xtk_spirit_auto_size(spirit);
    xtk_spirit_to_bitmap(spirit, wbmp);
    uview_bitblt_update(fd, 0, 0, wbmp);
    uview_bitmap_t *bmp0 = uview_bitmap_create(32, 32);
    assert(bmp0);
    uview_bitmap_rectfill(bmp0, 0, 0, 32, 32, UVIEW_BLACK);
    spirit->style.background_color = UVIEW_GREEN;
    spirit->style.border_color = UVIEW_YELLOW;
    spirit->style.color = UVIEW_WHITE;
    spirit->style.align = XTK_ALIGN_CENTER;
    xtk_spirit_set_bitmap(spirit, bmp0);
    
    xtk_spirit_set_text(spirit, "hello, world!\n");
    xtk_spirit_set_background_image(spirit, NULL);
    xtk_spirit_auto_size(spirit);
    
    uview_bitmap_t *bmp1 = uview_bitmap_create(spirit->width, spirit->height);
    assert(bmp1);
    
    xtk_collision_t *collision = xtk_collision_create(0, 0, spirit->width, spirit->height);
    assert(collision);
    xtk_spirit_set_collision(spirit, collision);
    xtk_collision_set_visible(collision, XTK_COLLISION_VISIBLE);
    xtk_spirit_set_pos(spirit, 0, 0);
    xtk_spirit_to_bitmap(spirit, bmp1);

    #if 0
    uview_bitmap_bitblt(wbmp, 100, 200, bmp1, 0, 0, 100, 24);
    uview_bitblt_update(fd, 0, 0, wbmp);
    #else
    uview_bitblt_update(fd, 100, 200, bmp1);
    #endif

    xtk_spirit_destroy(spirit);
    uview_bitmap_destroy(bmp1);


    xtk_spirit_t *label0 = xtk_label_create("hello");
    xtk_spirit_set_pos(label0, 20, 150);
    xtk_spirit_to_bitmap(label0, wbmp);
    
    xtk_spirit_t *label1 = xtk_label_create("world");
    xtk_spirit_set_pos(label1, 20, 200);
    xtk_spirit_to_bitmap(label1, wbmp);
    
    xtk_spirit_destroy(label0);
    xtk_spirit_destroy(label1);

    uview_bitblt_update(fd, 0, 0, wbmp);

    xtk_spirit_t *win = xtk_window_create("test", 400, 300, 200, 300, XTK_WINDOW_SHOW);
    assert(win);
    
    xtk_spirit_t *win1 = xtk_window_create("win1", 100, 100, 640, 480, XTK_WINDOW_SHOW);
    assert(win1);
    
    #if 0
    xtk_window_t *pwin = XTK_WINDOW(win);
    uview_bitmap_rectfill(win->bitmap, 0, 0, win->width, win->height, UVIEW_GRAY);
    uview_bitmap_rectfill(win->bitmap, pwin->style->border_width, pwin->style->border_width + pwin->style->navigation_height,
        pwin->content_width, pwin->content_height, UVIEW_WHITE);
    
    uview_bitblt(pwin->view, 0, 0, win->bitmap);

    uview_show(pwin->view);
    #endif

    while (1) {
        /* co de */
    }
}