#ifndef __SANDBOX_H__
#define __SANDBOX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * Framebuffer interface
 */
struct sandbox_fb_surface_t {
	int width;
	int height;
	int stride;
	int pixlen;
	void * pixels;
	void * priv;
};

struct sandbox_fb_region_t {
	int x, y;
	int w, h;
	int area;
};

struct sandbox_fb_region_list_t {
	struct sandbox_fb_region_t * region;
	unsigned int size;
	unsigned int count;
};

/* Framebuffer device */
void * sandbox_fb_open(const char * dev);
void sandbox_fb_close(void * context);
int sandbox_fb_get_width(void * context);
int sandbox_fb_get_height(void * context);
int sandbox_fb_get_pwidth(void * context);
int sandbox_fb_get_pheight(void * context);
int sandbox_fb_surface_create(void * context, struct sandbox_fb_surface_t * surface);
int sandbox_fb_surface_destroy(void * context, struct sandbox_fb_surface_t * surface);
int sandbox_fb_surface_present(void * context, struct sandbox_fb_surface_t * surface, struct sandbox_fb_region_list_t * rl);
void sandbox_fb_set_backlight(void * context, int brightness);
int sandbox_fb_get_backlight(void * context);

int fb_sandbox_probe();


#ifdef __cplusplus
}
#endif

#endif /* __SANDBOX_H__ */
