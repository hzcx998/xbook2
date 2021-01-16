#include "xtk_spirit.h"
#include "xtk_text.h"
#include <uview.h>
#include <stdlib.h>
#include <assert.h>
#include <dotfont.h>

extern dotfont_library_t __xtk_dotflib;

void xtk_spirit_init(xtk_spirit_t *spirit, int x, int y, int width, int height)
{
    spirit->x = x;
    spirit->y = y;
    spirit->width = width;
    spirit->height = height;
    
    spirit->style.border_color = UVIEW_NONE;
    
    spirit->style.color = UVIEW_BLACK;
    spirit->style.align = XTK_ALIGN_LEFT;
    spirit->text = NULL;
    spirit->image = NULL;
    spirit->bitmap = NULL;
    
    spirit->style.background_color = UVIEW_NONE;
    spirit->style.background_align = XTK_ALIGN_LEFT;
    spirit->background_image = NULL;

    spirit->collision = NULL;
    return spirit;
}

xtk_spirit_t *xtk_spirit_create(int x, int y, int width, int height)
{
    xtk_spirit_t *spirit = malloc(sizeof(xtk_spirit_t));
    if (!spirit)
        return NULL;
    xtk_spirit_init(spirit, x, y, width, height);
    return spirit;
}

int xtk_spirit_cleanup(xtk_spirit_t *spirit)
{
    if (!spirit)
        return -1;
    if (spirit->background_image) {
        xtk_image_destroy(spirit->background_image);
        spirit->background_image = NULL;
    }
    if (spirit->image) {
        xtk_image_destroy(spirit->image);
        spirit->image = NULL;
    }
    if (spirit->bitmap) {
        uview_bitmap_destroy(spirit->bitmap);
        spirit->bitmap = NULL;
    }
    if (spirit->text) {
        free(spirit->text);
        spirit->text = NULL;
    }
    return 0;
}

int xtk_spirit_destroy(xtk_spirit_t *spirit)
{
    if (!spirit)
        return -1;
    if (xtk_spirit_cleanup(spirit) < 0)
        return -1;
    free(spirit);
    return 0;
}

int xtk_spirit_set_pos(xtk_spirit_t *spirit, int x, int y)
{
    if (!spirit)
        return -1;
    spirit->x = x;
    spirit->y = y;
    return 0;
}

int xtk_spirit_set_size(xtk_spirit_t *spirit, int width, int height)
{
    if (!spirit)
        return -1;
    spirit->width = width;
    spirit->height = height;
    return 0;
}

int xtk_spirit_set_text(xtk_spirit_t *spirit, char *text)
{
    if (!spirit)
        return -1;
    int new_len = strlen(text);
    if (!spirit->text) {
        spirit->text = malloc(new_len + 1);
        memset(spirit->text, 0, new_len + 1);
    } else {
        int old_len = strlen(spirit->text);
        if (old_len < new_len) {
            spirit->text = realloc(spirit->text, new_len + 1);
            assert(spirit->text);
            memset(spirit->text, 0, new_len + 1);
        } else {
            memset(spirit->text, 0, old_len);
        }
    }
    assert(spirit->text);
    strcpy(spirit->text, text);
    spirit->text[strlen(spirit->text)] = '\0';
    return 0;
}

int xtk_spirit_auto_size(xtk_spirit_t *spirit)
{
    if (!spirit)
        return -1;
    if (spirit->text && spirit->style.color != UVIEW_NONE) {
        int len = strlen(spirit->text);
        dotfont_t *dotfont = dotfont_find(&__xtk_dotflib, DOTF_STANDARD_NAME);
        assert(dotfont);
        int w = dotfont_get_char_width(dotfont) * len;
        int h = dotfont_get_char_height(dotfont);
        if (w > spirit->width)
            spirit->width = w + 4;
        if (h > spirit->height)
            spirit->height = h + 4;
    }
    return 0;
}

int xtk_spirit_set_background_image(xtk_spirit_t *spirit, char *filename)
{
    if (!spirit)
        return -1;
    if (filename == NULL) {
        if (spirit->background_image)
            xtk_image_destroy(spirit->background_image);
        spirit->background_image = NULL;
        return 0;
    }
    xtk_image_t *img = xtk_image_load(filename);
    if (!img)
        return -1;
    if (spirit->background_image) {
        xtk_image_destroy(spirit->background_image);
        spirit->background_image = NULL;
    }
    spirit->background_image = img;
    return 0;
}

int xtk_spirit_set_image(xtk_spirit_t *spirit, char *filename)
{
    if (!spirit)
        return -1;
    if (filename == NULL) {
        if (spirit->image)
            xtk_image_destroy(spirit->image);
        spirit->image = NULL;
        return 0;
    }
    xtk_image_t *img = xtk_image_load(filename);
    if (!img)
        return -1;
    if (spirit->image) {
        xtk_image_destroy(spirit->image);
        spirit->image = NULL;
    }
    spirit->image = img;
    return 0;
}

int xtk_spirit_set_bitmap(xtk_spirit_t *spirit, uview_bitmap_t *bmp)
{
    if (!spirit)
        return -1;
    if (bmp == NULL) {
        if (spirit->bitmap)
            uview_bitmap_destroy(spirit->bitmap);
        spirit->bitmap = NULL;
        return 0;
    }
    if (spirit->bitmap) {
        uview_bitmap_destroy(spirit->bitmap);
        spirit->bitmap = NULL;
    }
    spirit->bitmap = bmp;
    return 0;
}

int xtk_spirit_set_collision(xtk_spirit_t *spirit, xtk_collision_t *collision)
{
    if (!spirit)
        return -1;
    if (collision == NULL) {
        if (spirit->collision)
            xtk_collision_destroy(spirit->collision);
        spirit->collision = NULL;
        return 0;
    }
    if (spirit->collision) {
        xtk_collision_destroy(spirit->collision);
        spirit->collision = NULL;
    }
    spirit->collision = collision;
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
int xtk_spirit_to_bitmap(xtk_spirit_t *spirit, uview_bitmap_t *bmp)
{
    if (!spirit)
        return -1;

    int start_x = spirit->x;
    int start_y = spirit->y;

    int off_x = 0, off_y = 0;
    /* 背景 */
    if (spirit->style.background_color != UVIEW_NONE) {
        uview_bitmap_rectfill(bmp, start_x, start_y, spirit->width, spirit->height,
            spirit->style.background_color);
    }
    if (spirit->background_image) {
        // 根据对齐方式设置显示位置
        __xtk_calc_aligin_pos(spirit->style.background_align, spirit->width, spirit->height, spirit->background_image->w,
            spirit->background_image->h, &off_x, &off_y);
        uview_bitmap_t srcbmp;
        uview_bitmap_init(&srcbmp, 
            (unsigned int) spirit->background_image->w,
            (unsigned int) spirit->background_image->h,
            (uview_color_t *) spirit->background_image->buf);

        uview_bitmap_bitblt(bmp, start_x + off_x, start_y + off_y, &srcbmp, 0, 0, srcbmp.width, srcbmp.height);
    }
    
    /* 前景 */
    if (spirit->bitmap) {
        // 根据对齐方式设置显示位置
        __xtk_calc_aligin_pos(spirit->style.align, spirit->width, spirit->height, 
            spirit->bitmap->width, spirit->bitmap->height, &off_x, &off_y);
        uview_bitmap_bitblt(bmp, start_x + off_x, start_y + off_y, spirit->bitmap, 0, 0,
            spirit->bitmap->width, spirit->bitmap->height);
    }
    if (spirit->image) {
        // 根据对齐方式设置显示位置
        __xtk_calc_aligin_pos(spirit->style.align, spirit->width, spirit->height, spirit->image->w,
            spirit->image->h, &off_x, &off_y);
        uview_bitmap_t srcbmp;
        uview_bitmap_init(&srcbmp, 
            (unsigned int) spirit->image->w,
            (unsigned int) spirit->image->h,
            (uview_color_t *) spirit->image->buf);
        uview_bitmap_bitblt(bmp, start_x + off_x, start_y + off_y, &srcbmp, 0, 0, srcbmp.width, srcbmp.height);
    }

    if (spirit->text && spirit->style.color != UVIEW_NONE) {
        dotfont_t *dotfont = dotfont_find(&__xtk_dotflib, DOTF_STANDARD_NAME);
        assert(dotfont);

        __xtk_calc_aligin_pos(spirit->style.align, spirit->width, spirit->height, 
            dotfont_get_char_width(dotfont) * strlen(spirit->text),
            dotfont_get_char_height(dotfont), &off_x, &off_y);
        xtk_text_to_bitmap(spirit->text, spirit->style.color, DOTF_STANDARD_NAME,
            bmp, start_x + off_x, start_y + off_y);
    }

    /* 边框 */
    if (spirit->style.border_color != UVIEW_NONE) {
        uview_bitmap_rect(bmp, start_x, start_y, spirit->width, spirit->height,
            spirit->style.border_color);
    }

    xtk_spirit_show_collision(spirit, bmp);

    return 0;
}

int xtk_spirit_show_collision(xtk_spirit_t *spirit, uview_bitmap_t *bmp)
{
    if (!spirit)
        return -1;
    /* 包围盒 */
    if (spirit->collision) {
        xtk_collision_show(spirit->collision, bmp, spirit->x, spirit->y);
    }
    return 0;
}