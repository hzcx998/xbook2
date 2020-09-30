#include "surface.h"
#include <assert.h>
#include <gapi.h>
#include <string.h>

surface_t *surface_image_load(char const *filename, int w, int h)
{
    int iw, ih, channels_in_file;
    
    char *image =  g_load_image(filename, &iw, &ih, &channels_in_file);
    assert(image);

    surface_t *s = surface_alloc(w, h);
    assert(s);
    g_resize_image(image, iw, ih, (unsigned char *) s->pixels, w, h, 4, GRSZ_BILINEAR);

    if (image)
        free(image);
    return s;
}

surface_t *surface_image_resize(surface_t *image, int w, int h)
{
    assert(image && image->pixels);
    surface_t *s = surface_alloc(w, h);
    assert(s);
    g_resize_image((unsigned char *) image->pixels, image->width, image->height,
            (unsigned char *) s->pixels, w, h, 4, GRSZ_BILINEAR);
    return s;
}
