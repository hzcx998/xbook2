#include <xboot.h>
#include <drivers/fb_sandbox.h>

struct sandbox_fb_context_t {
#if 0    
	struct fb_fix_screeninfo fi;
	struct fb_var_screeninfo vi;
#endif
    SGI_Display *dp;
    SGI_Window w;
    SGI_WindowInfo *wi;
    SGI_FontInfo *fi;
    int line_length;
    int bits_per_pixel;
	int vramsz;
	void * vram;
};

void * sandbox_fb_open(const char * dev)
{
	struct sandbox_fb_context_t * ctx;

	ctx = malloc(sizeof(struct sandbox_fb_context_t));
	if(!ctx)
		return NULL;

    ctx->dp = SGI_OpenDisplay();
    if (ctx->dp == NULL) {
        printf("[xui] ERROR: %s: failed to open display!\n", __func__);
        free(ctx);
        return NULL;
    }
    printf("[xui] open display ok!\n");

    ctx->w = SGI_CreateSimpleWindow(
        ctx->dp,
        ctx->dp->root_window,
        50,
        50,
        CONFIG_WIN_WIDTH,
        CONFIG_WIN_WIDTH,
        SGIC_BLACK);

    if (ctx->w < 0) {
        printf("[xui] ERROR: %s: failed to create window!\n", __func__);
        SGI_CloseDisplay(ctx->dp);
        free(ctx);
        return NULL;
    }

    printf("[xui] open window %d ok!\n", ctx->w);
    
    if (SGI_MapWindow(ctx->dp, ctx->w) < 0) {
        printf("[xui] ERROR: %s: failed to map window!\n", __func__);
        SGI_DestroyWindow(ctx->dp, ctx->w);
        SGI_CloseDisplay(ctx->dp);
        free(ctx);
        return NULL;
    }
    ctx->fi = SGI_LoadFont(ctx->dp, "standard-8*16");

    ctx->wi = SGI_DISPLAY_GET_WININFO(ctx->dp, ctx->w);
    if (ctx->wi == NULL) {
        printf("[xui] ERROR: %s: failed to get win info!\n", __func__);
        SGI_UnmapWindow(ctx->dp, ctx->w);
        SGI_DestroyWindow(ctx->dp, ctx->w);
        SGI_CloseDisplay(ctx->dp);
        free(ctx);
        return NULL;
    }

    if (SGI_UpdateWindow(ctx->dp, ctx->w, 0, 0, ctx->wi->width, ctx->wi->height) < 0) {
        printf("[xui] ERROR: %s: failed to update window!\n", __func__);
        SGI_UnmapWindow(ctx->dp, ctx->w);
        SGI_DestroyWindow(ctx->dp, ctx->w);
        SGI_CloseDisplay(ctx->dp);
        free(ctx);
        return NULL;
    }
    /* 选择接收的输入内容 */
    SGI_SelectInput(ctx->dp, ctx->w, SGI_ButtonPressMask | SGI_ButtonRleaseMask |
        SGI_KeyPressMask | SGI_KeyRleaseMask | SGI_PointerMotionMask);
    
    ctx->bits_per_pixel = 32;
    ctx->line_length = ctx->wi->width * 4;
	ctx->vramsz = ctx->line_length * ctx->wi->height;
	ctx->vram = ctx->wi->mapped_addr + ctx->wi->start_off;
	//memset(ctx->vram, 0, ctx->vramsz);

#if 0
	ctx->fd = open(dev, O_RDWR);
	if(ctx->fd < 0)
	{
		free(ctx);
		return NULL;
	}

	if(ioctl(ctx->fd, FBIOGET_FSCREENINFO, &ctx->fi) != 0)
	{
		close(ctx->fd);
		free(ctx);
		return NULL;
	}

	if(ioctl(ctx->fd, FBIOGET_VSCREENINFO, &ctx->vi) != 0)
	{
		close(ctx->fd);
		free(ctx);
		return NULL;
	}

	ctx->vi.red.offset = 0;
	ctx->vi.red.length = 8;
	ctx->vi.green.offset = 8;
	ctx->vi.green.length = 8;
	ctx->vi.blue.offset = 16;
	ctx->vi.blue.length = 8;
	ctx->vi.transp.offset = 24;
	ctx->vi.transp.length = 8;
	ctx->vi.bits_per_pixel = 32;
	ctx->vi.nonstd = 0;

	if(ioctl(ctx->fd, FBIOPUT_VSCREENINFO, &ctx->vi) != 0)
	{
		close(ctx->fd);
		free(ctx);
		return NULL;
	}

	ctx->vramsz = ctx->vi.yres_virtual * ctx->fi.line_length;
	ctx->vram = mmap(0, ctx->vramsz, PROT_READ | PROT_WRITE, MAP_SHARED, ctx->fd, 0);
	if(ctx->vram == (void *)(-1))
	{
		close(ctx->fd);
		free(ctx);
		return NULL;
	}
	memset(ctx->vram, 0, ctx->vramsz);
#endif
	return ctx;
}

void sandbox_fb_close(void * context)
{
	struct sandbox_fb_context_t * ctx = (struct sandbox_fb_context_t *)context;

	if(!ctx)
		return;
    SGI_UnmapWindow(ctx->dp, ctx->w);
    SGI_DestroyWindow(ctx->dp, ctx->w);
    SGI_CloseDisplay(ctx->dp);

#if 0
	if(ctx->vram != (void *)(-1))
		munmap(ctx->vram, ctx->vramsz);
	if(!(ctx->fd < 0))
		close(ctx->fd);
#endif
	free(ctx);
}

int sandbox_fb_get_width(void * context)
{
	struct sandbox_fb_context_t * ctx = (struct sandbox_fb_context_t *)context;
#if 1
	if(ctx)
		return ctx->fi->width;
#endif
	return 0;
}

int sandbox_fb_get_height(void * context)
{
	struct sandbox_fb_context_t * ctx = (struct sandbox_fb_context_t *)context;
#if 1
	if(ctx)
		return ctx->fi->height;
#endif
	return 0;
}

int sandbox_fb_get_pwidth(void * context)
{
	struct sandbox_fb_context_t * ctx = (struct sandbox_fb_context_t *)context;
#if 1
	if(ctx)
		return ctx->fi->width;
#endif        
	return 0;
}

int sandbox_fb_get_pheight(void * context)
{
	struct sandbox_fb_context_t * ctx = (struct sandbox_fb_context_t *)context;
#if 1
	if(ctx)
		return ctx->fi->height;
#endif        
	return 0;
}

int sandbox_fb_surface_create(void * context, struct sandbox_fb_surface_t * surface)
{
	struct sandbox_fb_context_t * ctx = (struct sandbox_fb_context_t *)context;
#if 1
	surface->width = ctx->fi->width;
	surface->height = ctx->fi->height;
	surface->stride = ctx->line_length;
#endif
	surface->pixlen = ctx->vramsz;
	surface->pixels = memalign(4, ctx->vramsz);
	return 1;
}

int sandbox_fb_surface_destroy(void * context, struct sandbox_fb_surface_t * surface)
{
	if(surface && surface->pixels)
		free(surface->pixels);
	return 1;
}

int sandbox_fb_surface_present(void * context, struct sandbox_fb_surface_t * surface, struct sandbox_fb_region_list_t * rl)
{
#if 1   
	struct sandbox_fb_context_t * ctx = (struct sandbox_fb_context_t *)context;
	struct sandbox_fb_region_t * r;
	unsigned char * p, * q;
 
	int pitch = ctx->line_length;
	int bytes = ctx->bits_per_pixel >> 3;
	int offset, line, height;
	int i, j;

	if(rl && (rl->count > 0))
	{
		for(i = 0; i < rl->count; i++)
		{
			r = &rl->region[i];
			offset = r->y * pitch + r->x * bytes;
			line = r->w * bytes;
			height = r->h;

			p = (unsigned char *)ctx->vram + offset;
			q = (unsigned char *)surface->pixels + offset;
			for(j = 0; j < height; j++, p += pitch, q += pitch)
				memcpy(p, q, line);
            /* 刷新区域 */
            SGI_UpdateWindow(ctx->dp, ctx->w, r->x, r->y, r->x + r->w, r->y + r->h);
		}
	}
	else
	{
		height = ctx->fi->height;
		p = (unsigned char *)ctx->vram;
		q = (unsigned char *)surface->pixels;
		for(j = 0; j < height; j++, p += pitch, q += pitch)
			memcpy(p, q, pitch);
        /* 刷新整个屏幕 */
        SGI_UpdateWindow(ctx->dp, ctx->w, 0, 0, ctx->fi->width, ctx->fi->height);
	}
#endif    
	return 1;
}

void sandbox_fb_set_backlight(void * context, int brightness)
{
}

int sandbox_fb_get_backlight(void * context)
{
	return 0;
}
