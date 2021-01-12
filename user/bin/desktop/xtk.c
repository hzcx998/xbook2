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
    xtk_spirit_auto(spirit);
    xtk_spirit_to_bitmap(spirit, wbmp);
    uview_bitblt_update(fd, 0, 0, wbmp);

    spirit->style.background_color = UVIEW_GREEN;
    spirit->style.border_color = UVIEW_YELLOW;
    spirit->style.color = UVIEW_WHITE;
    spirit->style.align = XTK_ALIGN_CENTER;
    xtk_spirit_set_pos(spirit, 100, 200);
    xtk_spirit_set_text(spirit, "hello, world!\n");
    xtk_spirit_set_background_image(spirit, NULL);
    xtk_spirit_auto(spirit);
    xtk_spirit_to_bitmap(spirit, wbmp);
    uview_bitblt_update(fd, 0, 0, wbmp);
    
    while (1) {
        /* co de */
    }
}