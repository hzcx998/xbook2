#pragma once

#define idiv255(x) ((((int)(x) + 1) * 257) >> 16)

typedef struct color_t
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
} color_t;

//https://en.wikipedia.org/wiki/Alpha_compositing
static inline void blend(color_t *restrict d, color_t *restrict s)
{
    //TODO: perf: d->a == 255
    if (s->a == 255 || d->a == 0)
    {
        *(int *)d = *(int *)s;
    }
    else if (s->a != 0)
    {
        float a2 = s->a;
        float a1 = d->a;
        float f = a1 + a2 - a1 * a2 / 255.0;
        float f1 = a2 / f;

        d->b += (s->b - d->b) * f1;
        d->g += (s->g - d->g) * f1;
        d->r += (s->r - d->r) * f1;
        d->a = f;
    }
}

static inline void blend_with_opacity(color_t *restrict d, color_t *restrict s, int a)
{
    a = idiv255(a * s->a);
    if (a == 255)
    {
        *(int *)d = *(int *)s;
    }
    else if (a != 0)
    {
        float a2 = a;
        float a1 = d->a;
        float f = a1 + a2 - a1 * a2 / 255.0;
        float f1 = a2 / f;

        d->b += (s->b - d->b) * f1;
        d->g += (s->g - d->g) * f1;
        d->r += (s->r - d->r) * f1;
        d->a = f;
    }
}

static inline color_t RGB(unsigned color)
{
    color |= 0xff000000;
    return *(color_t *)&color;
}

static inline color_t ARGB(unsigned color)
{
    return *(color_t *)&color;
}