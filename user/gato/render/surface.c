#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "surface.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define clamp(v, a, b) min(max(a, v), b)
#define fclampf(v, a, b) fminf(fmaxf(a, v), b)

surface_t *surface_alloc(int width, int height)
{
    surface_t *s;
    void *pixels;
    int stride, pixlen;

    if (width < 0 || height < 0)
        return NULL;

    s = calloc(1, sizeof(surface_t));
    s->pixels = calloc(1, width * height * sizeof(color_t));
    s->width = width;
    s->height = height;

    return s;
}

void surface_free(surface_t *s)
{
    free(s->pixels);
    free(s);
    return;
}

void surface_wrap(surface_t *s, color_t *c, int w, int h)
{
    s->width = w;
    s->height = h;
    s->pixels = c;
}

surface_t *surface_alloc_wrap(color_t *c, int width, int height)
{
    surface_t *s;
    void *pixels;
    int stride, pixlen;

    if (width < 0 || height < 0)
        return NULL;

    s = calloc(1, sizeof(surface_t));
    s->width = width;
    s->height = height;
    s->pixels = c;
    return s;
}

void surface_clear(surface_t *s, color_t c, int x, int y, int w, int h)
{
    int xs = clamp(x, 0, s->width - 1);
    int xe = clamp(x + w - 1, 0, s->width - 1);
    int ys = clamp(y, 0, s->height - 1);
    int ye = clamp(y + h - 1, 0, s->height - 1);

    for (int i = ys; i <= ye; i++)
    {
        color_t *pixels = &s->pixels[i * s->width];

        for (int j = xs; j <= xe; j++)
        {
            pixels[j] = c;
        }
    }
}

void surface_pixel_set(surface_t *s, color_t color, int x, int y)
{
    if (x >= 0 && x < s->width && y >= 0 && y < s->height)
    {
        color_t *c = s->pixels + y * s->width + x;
        blend(c, &color);
    }
}

surface_t *surface_copy(surface_t *s)
{
    surface_t *d = surface_alloc(s->width, s->height);

    for (int i = 0; i < d->height; i++)
    {
        for (int j = 0; j < d->width; j++)
        {
            d->pixels[i * d->width + j] = s->pixels[i * d->width + j];
        }
    }
    return d;
}

surface_t *surface_clone(surface_t *s, int x, int y, int w, int h)
{
    surface_t *d = surface_alloc(w, h);
    surface_cover(d, s, x, y);
    return d;
}

void surface_blit(surface_t *d, surface_t *s, int x, int y)
{
    int xs = clamp(x, 0, d->width - 1);
    int xe = clamp(x + s->width - 1, 0, d->width - 1);
    int ys = clamp(y, 0, d->height - 1);
    int ye = clamp(y + s->height - 1, 0, d->height - 1);
    for (int i = ys; i <= ye; i++)
    {
        color_t *dl = d->pixels + i * d->width;
        color_t *sl = s->pixels + (i - y) * s->width - x;

        for (int j = xs; j <= xe; j++)
        {
            blend(&dl[j], &sl[j]);
        }
    }
}

void surface_blit_with_opacity(surface_t *d, surface_t *s, int x, int y, int a)
{
    if (a == 255)
    {
        surface_blit(d, s, x, y);
        return;
    }

    int xs = clamp(x, 0, d->width - 1);
    int xe = clamp(x + s->width - 1, 0, d->width - 1);
    int ys = clamp(y, 0, d->height - 1);
    int ye = clamp(y + s->height - 1, 0, d->height - 1);
    for (int i = ys; i <= ye; i++)
    {
        color_t *dl = d->pixels + i * d->width;
        color_t *sl = s->pixels + (i - y) * s->width;

        for (int j = xs; j <= xe; j++)
        {
            blend_with_opacity(&dl[j], &sl[j - x], a);
        }
    }
}

void surface_clip(surface_t *d, surface_t *s, int x, int y)
{
    int xs = clamp(x, 0, d->width - 1);
    int xe = clamp(x + s->width - 1, 0, d->width - 1);
    int ys = clamp(y, 0, d->height - 1);
    int ye = clamp(y + s->height - 1, 0, d->height - 1);

    surface_mono(d, RGB(0x000000));
    for (int i = ys; i <= ye; i++)
    {
        for (int j = xs; j <= xe; j++)
        {
            color_t *dc = &d->pixels[i * d->width + j];
            color_t *sc = &s->pixels[(i - y) * s->width + j - x];
            if (dc->a)
                *dc = *sc;
        }
    }

    for (int i = 0; i < ys; i++)
        for (int j = 0; j < d->width; j++)
            d->pixels[i * d->width + j].a = 0;
    for (int i = ye + 1; i < d->height; i++)
        for (int j = 0; j < d->width; j++)
            d->pixels[i * d->width + j].a = 0;
    for (int i = 0; i < d->height; i++)
        for (int j = 0; j < xs; j++)
            d->pixels[i * d->width + j].a = 0;
    for (int i = 0; i < d->height; i++)
        for (int j = xe + 1; j < d->width; j++)
            d->pixels[i * d->width + j].a = 0;
}

void surface_mask(surface_t *d, surface_t *s, int x, int y)
{
    int xs = clamp(x, 0, d->width - 1);
    int xe = clamp(x + s->width - 1, 0, d->width - 1);
    int ys = clamp(y, 0, d->height - 1);
    int ye = clamp(y + s->height - 1, 0, d->height - 1);

    surface_mono(d, RGB(0x000000));
    for (int i = ys; i <= ye; i++)
    {
        for (int j = xs; j <= xe; j++)
        {
            color_t *dc = &d->pixels[i * d->width + j];
            color_t *sc = &s->pixels[(i - y) * s->width + j - x];

            dc->r = sc->r;
            dc->g = sc->g;
            dc->b = sc->b;
            dc->a = dc->a * sc->a / 255.0; //TODO:
        }
    }

    for (int i = 0; i < ys; i++)
        for (int j = 0; j < d->width; j++)
            d->pixels[i * d->width + j].a = 0;
    for (int i = ye + 1; i < d->height; i++)
        for (int j = 0; j < d->width; j++)
            d->pixels[i * d->width + j].a = 0;
    for (int i = 0; i < d->height; i++)
        for (int j = 0; j < xs; j++)
            d->pixels[i * d->width + j].a = 0;
    for (int i = 0; i < d->height; i++)
        for (int j = xe + 1; j < d->width; j++)
            d->pixels[i * d->width + j].a = 0;
}

void surface_cover(surface_t *d, surface_t *s, int x, int y)
{
    int xs = clamp(x, 0, d->width - 1);
    int xe = clamp(x + s->width - 1, 0, d->width - 1);
    int ys = clamp(y, 0, d->height - 1);
    int ye = clamp(y + s->height - 1, 0, d->height - 1);
    for (int i = ys; i <= ye; i++)
    {
        color_t *dc = d->pixels + i * d->width;
        color_t *sc = s->pixels + (i - y) * s->width - x;

        for (int j = xs; j <= xe; j++)
        {
            dc[j] = sc[j];
        }
    }
}

void surface_composite_out(surface_t *d, surface_t *s, int x, int y)
{
    int xs = clamp(x, 0, d->width - 1);
    int xe = clamp(x + s->width - 1, 0, d->width - 1);
    int ys = clamp(y, 0, d->height - 1);
    int ye = clamp(y + s->height - 1, 0, d->height - 1);

    for (int i = ys; i <= ye; i++)
    {
        color_t *dc = d->pixels + i * d->width;
        color_t *sc = s->pixels + (i - y) * s->width - x;

        for (int j = xs; j <= xe; j++)
        {
            dc[j].a = idiv255(dc[j].a * (255 - sc[j].a));
        }
    }
}

void surface_mono(surface_t *s, color_t color)
{
    for (int i = 0; i < s->height; i++)
    {
        for (int j = 0; j < s->width; j++)
        {
            color_t *c = &s->pixels[i * s->width + j];
            // if (c->a != 0)
            {
                c->r = color.r;
                c->g = color.g;
                c->b = color.b;
            }
        }
    }
}

static inline void blurinner(unsigned char *restrict p, int *restrict zr, int *restrict zg, int *restrict zb, int *restrict za, int alpha)
{
    int r, g, b;
    unsigned char a;

    b = p[0];
    g = p[1];
    r = p[2];
    a = p[3];

    *zb += (alpha * ((b << 7) - *zb)) >> 16;
    *zg += (alpha * ((g << 7) - *zg)) >> 16;
    *zr += (alpha * ((r << 7) - *zr)) >> 16;
    *za += (alpha * ((a << 7) - *za)) >> 16;

    p[0] = *zb >> 7;
    p[1] = *zg >> 7;
    p[2] = *zr >> 7;
    p[3] = *za >> 7;
}

static inline void blurrow(unsigned char *pixel, int width, int height, int channel, int line, int alpha)
{
    unsigned char *p = &(pixel[line * width * channel]);
    int zr, zg, zb, za;
    int i;

    zb = p[0] << 7;
    zg = p[1] << 7;
    zr = p[2] << 7;
    za = p[3] << 7;

    for (i = 0; i < width; i++)
        blurinner(&p[i * channel], &zr, &zg, &zb, &za, alpha);
    for (i = width - 2; i >= 0; i--)
        blurinner(&p[i * channel], &zr, &zg, &zb, &za, alpha);
}

static inline void blurcol(unsigned char *pixel, int width, int height, int channel, int x, int alpha)
{
    unsigned char *p = pixel;
    int zr, zg, zb, za;
    int i;

    p += x * channel;
    zb = p[0] << 7;
    zg = p[1] << 7;
    zr = p[2] << 7;
    za = p[3] << 7;

    for (i = width; i < (height - 1) * width; i += width)
        blurinner(&p[i * channel], &zr, &zg, &zb, &za, alpha);
    for (i = (height - 2) * width; i >= 0; i -= width)
        blurinner(&p[i * channel], &zr, &zg, &zb, &za, alpha);
}

static inline void expblur(unsigned char *pixel, int width, int height, int channel, int radius)
{
    int alpha = (int)((1 << 16) * (1.0 - expf(-2.3 / (radius + 1.0))));
    int row, col;

    for (row = 0; row < height; row++)
        blurrow(pixel, width, height, channel, row, alpha);
    for (col = 0; col < width; col++)
        blurcol(pixel, width, height, channel, col, alpha);
}

void surface_filter_blur(struct surface_t *s, int radius)
{
    int width = s->width;
    int height = s->height;
    unsigned char *pixels = s->pixels;

    if (radius > 0)
        expblur(pixels, width, height, 4, radius);
}

void surface_filter_opacity(struct surface_t *s, int a)
{
    float fa = a / 255.0;
    for (int i = 0; i < s->height; i++)
    {
        for (int j = 0; j < s->width; j++)
        {
            color_t *c = &s->pixels[i * s->width + j];
            c->a = c->a * fa;
        }
    }
}