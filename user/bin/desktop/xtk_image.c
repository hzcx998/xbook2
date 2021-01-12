#include "xtk_image.h"
#include <uview.h>
#include <stdlib.h>
#include <assert.h>

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
