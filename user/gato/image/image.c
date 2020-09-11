#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize.h"
#include "surface.h"
#include <assert.h>
#if 1
surface_t *surface_image_load(char const *filename, int w, int h)
{
    int iw, ih, channels_in_file;
    char *image = stbi_load(filename, &iw, &ih, &channels_in_file, STBI_rgb_alpha);
    assert(image);
    surface_t *s = surface_alloc(w, h);
    assert(s);
    stbir_resize_uint8(image, iw, ih, 0, s->pixels, w, h, 0, STBI_rgb_alpha);

    for (int i = 0; i < s->height; i++)
    {
        for (int j = 0; j < s->width; j++)
        {
            color_t c = s->pixels[i * s->width + j];
            s->pixels[i * s->width + j] = (color_t){c.r, c.g, c.b, c.a};
        }
    }
    return s;
}

surface_t *surface_image_resize(surface_t *image, int w, int h)
{
    assert(image && image->pixels);
    surface_t *s = surface_alloc(w, h);
    assert(s);
    stbir_resize_uint8(image->pixels, image->width, image->height, 0, s->pixels, w, h, 0, STBI_rgb_alpha);
    return s;
}
#else

surface_t *surface_image_load(char const *filename, int w, int h)
{
    int iw, ih, channels_in_file;
    surface_t *s = surface_alloc(w, h);
    assert(s);
    for (int i = 0; i < s->height; i++)
    {
        for (int j = 0; j < s->width; j++)
        {
            s->pixels[i * s->width + j] = (color_t){255, 128, 128, 128};
        }
    }
    return s;
}

surface_t *surface_image_resize(surface_t *image, int w, int h)
{
    assert(image && image->pixels);
    surface_t *s = surface_alloc(w, h);
    assert(s);
    //stbir_resize_uint8(image->pixels, image->width, image->height, 0, s->pixels, w, h, 0, STBI_rgb_alpha);
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            s->pixels[i * w + j] = (color_t){255, 192, 192, 192};
        }
    }
    return s;
}

#endif