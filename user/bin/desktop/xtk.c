#include "xtk.h"
#include <uview.h>
#include <stdlib.h>
#include <assert.h>

#include <dotfont.h>

xtk_image_t *xtk_image_load(char *filename)
{
    xtk_image_t *img = malloc(sizeof(xtk_image_t));
    if (!img)
        return NULL;
    img->buf = uview_load_image(filename, &img->w, &img->h, &img->channels);
    if (!img->buf) {
        free(img);
        return NULL;
    }
    return img;
}

int xtk_image_destroy(xtk_image_t *img)
{
    if (!img)
        return -1;
    if (img->buf)
        free(img->buf);
    free(img);
    return 0;
}

int xtk_image_resize(xtk_image_t *img, int w, int h)
{
    if (!img)
        return -1;
    unsigned char *buf = malloc(w * h * img->channels);
    if (!buf)
        return -1;
    uview_resize_image(img->buf, img->w, img->h, buf, w, h, img->channels);
    free(img->buf);
    img->buf = buf;
    img->w = w;
    img->h = h;
    return 0;
}

xtk_image_t *xtk_image_load2(char *filename, int w, int h)
{
    xtk_image_t *img = xtk_image_load(filename);
    if (!img)
        return NULL;
    if (xtk_image_resize(img, w, h) < 0) {
        xtk_image_destroy(img);
        return NULL;
    }
    return img;
}

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

int xtk_spirit_set_background_image(xtk_spirit_t *spilit, char *filename)
{
    if (!spilit)
        return -1;
    if (filename == "") {
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

/* 将精灵渲染到bmp位图中 */
int xtk_spirit_to_bitmap(xtk_spirit_t *spilit, uview_bitmap_t *bmp)
{
    if (!spilit)
        return -1;
    /* 背景 */
    if (spilit->style.background_color != UVIEW_NONE) {
        uview_bitmap_rectfill(bmp, spilit->x, spilit->y, spilit->width, spilit->height, spilit->style.background_color);
    }
    if (spilit->background_image) {
        uview_bitmap_t srcbmp = {spilit->background_image->w, 
            spilit->background_image->h, spilit->background_image->buf};
        uview_bitmap_bitblt(bmp, spilit->x, spilit->y, &srcbmp, 0, 0, srcbmp.width, srcbmp.height);
    }
    if (spilit->style.border_color != UVIEW_NONE) {
        uview_bitmap_rect(bmp, spilit->x, spilit->y, spilit->width, spilit->height, spilit->style.border_color);
    }
    
    /* 前景 */
    if (spilit->text && spilit->style.color != UVIEW_NONE) {
        xtk_text_to_bitmap(spilit->text, spilit->style.color, DOTF_STANDARD_NAME, bmp, spilit->x, spilit->y);
    }
    return 0;
}

void xtk_test(int fd, uview_bitmap_t *wbmp)
{
    xtk_image_t *img = xtk_image_load2("/res/cursor.png", 32, 32);
    assert(img);
    
    uview_bitmap_t *bmp = uview_bitmap_create(img->w, img->h);
    assert(bmp);
    
    uview_bitmap_t srcbmp = {img->w, img->h, img->buf};
    uview_bitmap_bitblt(bmp, 0, 0, &srcbmp, 0, 0, srcbmp.width, srcbmp.height);
    uview_bitblt_update(fd, 0, 0, bmp);
    
    uview_bitmap_destroy(bmp);
    xtk_image_destroy(img);

    xtk_spirit_t *spirit = xtk_spirit_create(100, 100, 50, 24);
    assert(spirit);
    xtk_spirit_set_text(spirit, "abcdef!asdasdasd");
    xtk_spirit_set_background_image(spirit, "/res/cursor.png");
    xtk_spirit_to_bitmap(spirit, wbmp);
    uview_bitblt_update(fd, 0, 0, wbmp);

    spirit->style.background_color = UVIEW_GREEN;
    spirit->style.border_color = UVIEW_YELLOW;
    spirit->style.color = UVIEW_WHITE;
    xtk_spirit_set_pos(spirit, 100, 200);
    xtk_spirit_set_text(spirit, "hello, world!\n");
    xtk_spirit_set_background_image(spirit, "");
    xtk_spirit_to_bitmap(spirit, wbmp);
    uview_bitblt_update(fd, 0, 0, wbmp);
    
    while (1) {
        /* co de */
    }
    
}