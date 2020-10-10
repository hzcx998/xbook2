#pragma once
#include "surface.h"

#ifdef __cplusplus
extern "C"
{
#endif

    surface_t *surface_image_load(char const *filename, int w, int h);
    surface_t *surface_image_resize(surface_t *image, int w, int h);

#ifdef __cplusplus
}
#endif