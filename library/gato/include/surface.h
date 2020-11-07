#pragma once

#include "color.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct surface_t
	{
		int width;
		int height;
		int stride;
		int pixlen;
		color_t *pixels;
	} surface_t;

	surface_t *surface_alloc(int width, int height);
	surface_t *surface_copy(surface_t *s);
	void surface_free(surface_t *s);
	void surface_wrap(surface_t *s, color_t *c, int w, int h);
	surface_t *surface_alloc_wrap(color_t *c, int width, int height);
	void surface_clear(surface_t *s, color_t c, int x, int y, int w, int h);
	void surface_blit(surface_t *d, surface_t *s, int x, int y);
	void surface_cover(surface_t *d, surface_t *s, int x, int y);
	surface_t *surface_clone(surface_t *s, int x, int y, int w, int h);
	void surface_mono(surface_t *s, color_t color);
	void surface_filter_blur(struct surface_t *s, int radius);
	void surface_filter_opacity(struct surface_t *s, int a);
	void surface_mask(surface_t *d, surface_t *s, int x, int y);
	void surface_blit_with_opacity(surface_t *d, surface_t *s, int x, int y, int a);
	void surface_pixel_set(surface_t *s, color_t color, int x, int y);
	void surface_composite_out(surface_t *d, surface_t *s, int x, int y);

#ifdef __cplusplus
}
#endif