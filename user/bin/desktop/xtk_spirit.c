#include "xtk_spirit.h"
#include "xtk_text.h"
#include <uview.h>
#include <stdlib.h>
#include <assert.h>
#include <dotfont.h>

extern dotfont_library_t __xtk_dotflib;

xtk_spirit_t *xtk_spirit_create(int x, int y, int width, int height)
{
    xtk_spirit_t *spilit = malloc(sizeof(xtk_spirit_t));
    if (!spilit)
        return NULL;
    spilit->x = x;
    spilit->y = y;
    spilit->width = width;
    spilit->height = height;
    
    spilit->style.border_color = UVIEW_NONE;
    
    spilit->style.color = UVIEW_BLACK;
    spilit->style.align = XTK_ALIGN_LEFT;
    spilit->text = NULL;

    spilit->style.background_color = UVIEW_NONE;
    spilit->style.background_align = XTK_ALIGN_LEFT;
    spilit->background_image = NULL;
    return spilit;
}

int xtk_spirit_destroy(xtk_spirit_t *spilit)
{
    if (!spilit)
        return -1;
    if (spilit->background_image) {
        xtk_image_destroy(spilit->background_image);
        spilit->background_image = NULL;
    }
    free(spilit);
    return 0;
}

int xtk_spirit_set_pos(xtk_spirit_t *spilit, int x, int y)
{
    if (!spilit)
        return -1;
    spilit->x = x;
    spilit->y = y;
    return 0;
}

int xtk_spirit_set_size(xtk_spirit_t *spilit, int width, int height)
{
    if (!spilit)
        return -1;
    spilit->width = width;
    spilit->height = height;
    return 0;
}

int xtk_spirit_set_text(xtk_spirit_t *spilit, char *text)
{
    if (!spilit)
        return -1;
    int new_len = strlen(text);
    if (!spilit->text) {
        spilit->text = malloc(new_len + 1);
        memset(spilit->text, 0, new_len + 1);
    } else {
        int old_len = strlen(spilit->text);
        if (old_len < new_len) {
            spilit->text = realloc(spilit->text, new_len + 1);
            assert(spilit->text);
            memset(spilit->text, 0, new_len + 1);
        } else {
            memset(spilit->text, 0, old_len);
        }
    }
    assert(spilit->text);
    strcpy(spilit->text, text);
    spilit->text[strlen(spilit->text)] = '\0';
    return 0;
}

int xtk_spirit_auto(xtk_spirit_t *spilit)
{
    if (!spilit)
        return -1;
    if (spilit->text && spilit->style.color != UVIEW_NONE) {
        int len = strlen(spilit->text);
        dotfont_t *dotfont = dotfont_find(&__xtk_dotflib, DOTF_STANDARD_NAME);
        assert(dotfont);
        int w = dotfont_get_char_width(dotfont) * len;
        int h = dotfont_get_char_height(dotfont);
        if (w > spilit->width)
            spilit->width = w + 4;
        if (h > spilit->height)
            spilit->height = h + 4;
    }
    return 0;
}

int xtk_spirit_set_background_image(xtk_spirit_t *spilit, char *filename)
{
    if (!spilit)
        return -1;
    if (filename == NULL) {
        if (spilit->background_image)
            xtk_image_destroy(spilit->background_image);
        spilit->background_image = NULL;
        return 0;
    }
    xtk_image_t *img = xtk_image_load(filename);
    if (!img)
        return -1;
    spilit->background_image = img;
    return 0;
}

static void __xtk_calc_aligin_pos(xtk_align_t align, int box_width, int box_height, 
        int width, int height, int *out_x, int *out_y)
{
    int x, y;
    switch (align) {
    case XTK_ALIGN_LEFT:
        x = 0;
        y = box_height / 2 - height / 2;
        break;
    case XTK_ALIGN_RIGHT:
        x = box_width - width;
        y = box_height / 2 - height / 2;
        break;
    case XTK_ALIGN_TOP:
        x = box_width / 2 - width / 2;
        y = 0;
        break;
    case XTK_ALIGN_BOTTOM:
        x = box_width / 2 - width / 2;
        y = box_height - height;
        break;
    default:
        x = box_width / 2 - width / 2;
        y = box_height / 2 - height / 2;
        break;
    }
    *out_x = x;
    *out_y = y;
}


/* 将精灵渲染到bmp位图中 */
int xtk_spirit_to_bitmap(xtk_spirit_t *spilit, uview_bitmap_t *bmp)
{
    if (!spilit)
        return -1;

    int start_x = spilit->x;
    int start_y = spilit->y;

    int off_x = 0, off_y = 0;
    /* 背景 */
    if (spilit->style.background_color != UVIEW_NONE) {
        uview_bitmap_rectfill(bmp, start_x, start_y, spilit->width, spilit->height,
            spilit->style.background_color);
    }
    if (spilit->background_image) {
        // 根据对齐方式设置显示位置
        __xtk_calc_aligin_pos(spilit->style.background_align, spilit->width, spilit->height, spilit->background_image->w,
            spilit->background_image->h, &off_x, &off_y);
        uview_bitmap_t srcbmp;
        uview_bitmap_init(&srcbmp, 
            (unsigned int) spilit->background_image->w,
            (unsigned int) spilit->background_image->h,
            (uview_color_t *) spilit->background_image->buf);

        uview_bitmap_bitblt(bmp, start_x + off_x, start_y + off_y, &srcbmp, 0, 0, srcbmp.width, srcbmp.height);
    }
    if (spilit->style.border_color != UVIEW_NONE) {
        uview_bitmap_rect(bmp, start_x - 1, start_y - 1, spilit->width + 2, spilit->height + 2,
            spilit->style.border_color);
    }

    /* 前景 */
    if (spilit->text && spilit->style.color != UVIEW_NONE) {
        dotfont_t *dotfont = dotfont_find(&__xtk_dotflib, DOTF_STANDARD_NAME);
        assert(dotfont);

        __xtk_calc_aligin_pos(spilit->style.align, spilit->width, spilit->height, 
            dotfont_get_char_width(dotfont) * strlen(spilit->text),
            dotfont_get_char_height(dotfont), &off_x, &off_y);
        xtk_text_to_bitmap(spilit->text, spilit->style.color, DOTF_STANDARD_NAME,
            bmp, start_x + off_x, start_y + off_y);
    }
    return 0;
}
