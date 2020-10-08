#include <assert.h>
#include <gapi.h>
#include <unistd.h>
#include <stdio.h>

#include "renderer.h"
#include "atlas.inl"

#define MU_RENDER_BUFFER_SIZE 16384

static mu_Rect mu_render_tex_buf[MU_RENDER_BUFFER_SIZE];
static mu_Rect mu_render_vert_buf[MU_RENDER_BUFFER_SIZE];
static mu_Color mu_render_color_buf[MU_RENDER_BUFFER_SIZE];
static mu_Rect mu_render_clip_rect;

static int mu_render_win;
static g_bitmap_t * mu_render;
static int mu_render_width;
static int mu_render_height;
static int mu_render_buf_idx;

/**
 * init microui render
 * render used for show microui on the screen
 */
mu_Context *mu_render_init(const char *title, int x, int y, int width, int height) 
{
    if (g_init() < 0) {
        printf("[mu render]: init failed!\n");
        return NULL;
    }
    mu_render_win = g_new_window((char *) title, x, y, width, height, GW_SHOW);
    if (mu_render_win < 0){
        printf("[mu render]: new widnow failed!\n");
        g_quit();
        return NULL;
    }

	mu_render = g_new_bitmap(width, height);
	if (mu_render == NULL) {
        printf("[mu render]: new bitmap failed!\n");
        g_del_window(mu_render_win);
        g_quit();
        return NULL;
    }
	mu_render_clip_rect = (mu_Rect) { 0, 0, width, height};
    mu_render_width = width;
    mu_render_height = height;
    
    /* init microui */
	mu_Context *ctx = malloc(sizeof(mu_Context));
    if (ctx == NULL) {
        printf("[mu render]: malloc for context failed!\n");
        g_del_bitmap(mu_render);
        g_del_window(mu_render_win);
        g_quit();
        return NULL;
    }
	mu_init(ctx);
    return ctx;
}

#define BIT_GET(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] & (1<<((p) % (sizeof(char) * 8))))

#if 0
static u32_t neon(u32_t src, u32_t color, u32_t factor)
{
	u32_t ret;
	asm (
		"vmov d0,%1,%2\n\t"
		"vmovl.u8 q0,d0\n\t"
		"vsub.u16 d1,d1,d0\n\t"
		"vmov d2,%3,%3\n\t"
		"vmov.u16 d2[1],%3\n\t"
		"vmov.u16 d2[3],%3\n\t"
		"vmull.u16 q1,d2,d1\n\t"
		"vshrn.u32 d2,q1,#16\n\t"
		"vadd.u16 d0,d2,d0\n\t"
		"vmovn.u16 d0,q0\n\t"
		"vmov.u32 %0,d0[0]\n\t"
		: "+r" (ret) 
		: "r" (src), "r" (color), "r" (factor)
		: "memory", "d0", "d1", "d2", "d3"
	);
	return ret;
}
#endif

static void disp_fill(mu_Rect dst, mu_Rect src, mu_Color color)
{
    int32_t x;
    int32_t y;
	// float factor_x = ((float)src.w) / (dst.w);
	// float factor_y = ((float)src.h) / (dst.h);

	int sx, sy, ex, ey;
	if(dst.x < mu_render_clip_rect.x) sx = mu_render_clip_rect.x;
	else if(dst.x >= mu_render_clip_rect.x + mu_render_clip_rect.w) return; else sx = dst.x;
	if(dst.y < mu_render_clip_rect.y) sy = mu_render_clip_rect.y;
	else if(dst.y >= mu_render_clip_rect.y + mu_render_clip_rect.h) return; else sy = dst.y;
	if(dst.x + dst.w > mu_render_clip_rect.x + mu_render_clip_rect.w) ex = mu_render_clip_rect.x + mu_render_clip_rect.w;
	else if(dst.x + dst.w <= mu_render_clip_rect.x) return; else ex = dst.x + dst.w;
	if(dst.y + dst.h > mu_render_clip_rect.y + mu_render_clip_rect.h) ey = mu_render_clip_rect.y + mu_render_clip_rect.h;
	else if(dst.y + dst.h <= mu_render_clip_rect.y) return; else ey = dst.y + dst.h;

	if(color.a == 0xff) {
		for(y = sy; y < ey; y++) {
			mu_Color *base = (mu_Color *)&((u32_t *)mu_render->buffer)[y * mu_render_width];
			for(x = sx; x < ex; x++) base[x] = color;
		}
	} else {
	    for(y = sy; y < ey; y++) {
			mu_Color *base = (mu_Color *)&((u32_t *)mu_render->buffer)[y * mu_render_width];
			for(x = sx; x < ex; x++) {
				mu_Color *c = &base[x];
				int a = color.a;
				c->r = ((int) (a * (color.r - c->r)) >> 8) + c->r;
				c->g = ((int) (a * (color.g - c->g)) >> 8) + c->g;
				c->b = ((int) (a * (color.b - c->b)) >> 8) + c->b;
			}
		}
	}
}

static void disp_texture(mu_Rect dst, mu_Rect src, mu_Color color)
{
    int32_t x;
    int32_t y;
	float factor_x = ((float)src.w) / (dst.w);
	float factor_y = ((float)src.h) / (dst.h);

	int sx, sy, ex, ey;
	if(dst.x < mu_render_clip_rect.x) sx = mu_render_clip_rect.x;
	else if(dst.x >= mu_render_clip_rect.x + mu_render_clip_rect.w) return; else sx = dst.x;
	if(dst.y < mu_render_clip_rect.y) sy = mu_render_clip_rect.y;
	else if(dst.y >= mu_render_clip_rect.y + mu_render_clip_rect.h) return; else sy = dst.y;
	if(dst.x + dst.w > mu_render_clip_rect.x + mu_render_clip_rect.w) ex = mu_render_clip_rect.x + mu_render_clip_rect.w;
	else if(dst.x + dst.w <= mu_render_clip_rect.x) return; else ex = dst.x + dst.w;
	if(dst.y + dst.h > mu_render_clip_rect.y + mu_render_clip_rect.h) ey = mu_render_clip_rect.y + mu_render_clip_rect.h;
	else if(dst.y + dst.h <= mu_render_clip_rect.y) return; else ey = dst.y + dst.h;

    for(y = sy; y < ey; y++) {
		mu_Color *base = (mu_Color *)&((u32_t *)mu_render->buffer)[y * mu_render_width];
        for(x = sx; x < ex; x++) {
			mu_Color *c = &base[x];
			int src_x = src.x + ((int)((x - dst.x) * factor_x));
			int src_y = src.y + ((int)((y - dst.y) * factor_y));
			int a = color.a * atlas_texture[src_y * ATLAS_HEIGHT + src_x];
#if 1
			c->r = ((int) (a * (color.r - c->r)) >> 16) + c->r;
			c->g = ((int) (a * (color.g - c->g)) >> 16) + c->g;
			c->b = ((int) (a * (color.b - c->b)) >> 16) + c->b;
#else
			*(u32_t *)c = neon(*(u32_t *)c, *(u32_t *)&color, a);
#endif
        }
    }
}

static void disp_custom(mu_Rect dst, mu_Color *color)
{
    int32_t x;
    int32_t y;
	int sx, sy, ex, ey;
	if(dst.x < mu_render_clip_rect.x) sx = mu_render_clip_rect.x;
	else if(dst.x >= mu_render_clip_rect.x + mu_render_clip_rect.w) return; else sx = dst.x;
	if(dst.y < mu_render_clip_rect.y) sy = mu_render_clip_rect.y;
	else if(dst.y >= mu_render_clip_rect.y + mu_render_clip_rect.h) return; else sy = dst.y;
	if(dst.x + dst.w > mu_render_clip_rect.x + mu_render_clip_rect.w) ex = mu_render_clip_rect.x + mu_render_clip_rect.w;
	else if(dst.x + dst.w <= mu_render_clip_rect.x) return; else ex = dst.x + dst.w;
	if(dst.y + dst.h > mu_render_clip_rect.y + mu_render_clip_rect.h) ey = mu_render_clip_rect.y + mu_render_clip_rect.h;
	else if(dst.y + dst.h <= mu_render_clip_rect.y) return; else ey = dst.y + dst.h;

	for(y = sy; y < ey; y++) {
		mu_Color *base = (mu_Color *)&((u32_t *)mu_render->buffer)[y * mu_render_width];
		for(x = sx; x < ex; x++) {
			mu_Color *c = &base[x];
			int src_x = x - dst.x;
			int src_y = y - dst.y;
			mu_Color *cc = &color[src_y * dst.w + src_x];
			int a = cc->a;
			c->r = ((int) (a * (cc->r - c->r)) >> 8) + c->r;
			c->g = ((int) (a * (cc->g - c->g)) >> 8) + c->g;
			c->b = ((int) (a * (cc->b - c->b)) >> 8) + c->b;
		}
	}
}

static void disp_clear(mu_Color color)
{
    int32_t x;
    int32_t y;
    for(y = 0; y < mu_render_height; y++) {
        for(x = 0; x < mu_render_width; x++) {
			((u32_t *)mu_render->buffer)[y * mu_render_width + x] = *(int *)&color;
        }
    }
}

static void flush(void) {
	if (mu_render_buf_idx == 0) { return; }

	for(int i = 0; i < mu_render_buf_idx; i++){
		if(mu_render_tex_buf[i].x == 125)
			disp_fill(mu_render_vert_buf[i], mu_render_tex_buf[i], mu_render_color_buf[i]);
		else
			disp_texture(mu_render_vert_buf[i], mu_render_tex_buf[i], mu_render_color_buf[i]);
	}
	mu_render_buf_idx = 0;
}


static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
	if (mu_render_buf_idx == MU_RENDER_BUFFER_SIZE) { flush(); }
	/* update texture buffer */
	mu_render_tex_buf[mu_render_buf_idx] = src;
	/* update vertex buffer */
	mu_render_vert_buf[mu_render_buf_idx] = dst;
	/* update color buffer */
	mu_render_color_buf[mu_render_buf_idx] = color;
	mu_render_buf_idx++;
}


void mu_render_draw_rect(mu_Rect rect, mu_Color color) {
	push_quad(rect, atlas[ATLAS_WHITE], color);
}


void mu_render_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
	mu_Rect dst = { pos.x, pos.y, 0, 0 };
	for (const char *p = text; *p; p++) {
		if ((*p & 0xc0) == 0x80) { continue; }
		int chr = mu_min((unsigned char) *p, 127);
		mu_Rect src = atlas[ATLAS_FONT + chr];
		dst.w = src.w;
		dst.h = src.h;
		push_quad(dst, src, color);
		dst.x += dst.w;
	}
}


void mu_render_draw_icon(int id, mu_Rect rect, mu_Color color) {
	mu_Rect src = atlas[id];
	int x = rect.x + (rect.w - src.w) / 2;
	int y = rect.y + (rect.h - src.h) / 2;
	push_quad(mu_rect(x, y, src.w, src.h), src, color);
}

void mu_render_draw_custom(mu_Rect rect, mu_Color *color) {
	flush();
	disp_custom(rect, color);
}

int mu_render_get_text_width(const char *text, int len) {
	int res = 0;
	for (const char *p = text; *p && len--; p++) {
		if ((*p & 0xc0) == 0x80) { continue; }
		int chr = mu_min((unsigned char) *p, 127);
		res += atlas[ATLAS_FONT + chr].w;
	}
	return res;
}


int mu_render_get_text_height(void) {
	return 18;
}


void mu_render_set_clip_rect(mu_Rect rect) {
	flush();

	if(rect.x < 0) rect.x = 0;
	if(rect.y < 0) rect.y = 0;
	if(rect.x + rect.w > mu_render_width) rect.w = mu_render_width - rect.x;
	if(rect.y + rect.h > mu_render_height) rect.h = mu_render_height - rect.y;
	mu_render_clip_rect = rect; 	// 可以注释这条语句，检查是否有组件进行了错误的裁剪
}


void mu_render_clear(mu_Color clr) {
	flush();
	disp_clear(clr);
}

void mu_render_present(void) {
	flush();
    g_paint_window(mu_render_win, 0, 0, mu_render);
}
