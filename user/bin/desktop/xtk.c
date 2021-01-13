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
    xtk_spirit_set_pos(spirit, 0, 0);
    xtk_spirit_to_bitmap(spirit, bmp1);

    #if 0
    uview_bitmap_bitblt(wbmp, 100, 200, bmp1, 0, 0, 100, 24);
    uview_bitblt_update(fd, 0, 0, wbmp);
    #else
    uview_bitblt_update(fd, 100, 200, bmp1);
    #endif
    xtk_spirit_destroy(spirit);
    while (1) {
        /* co de */
    }
}